/* Copyright 2011-2016 Tyler Gilbert; 
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */


#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <string>

#define MAX_TRIES 3

#include "Link.h"
#include "link_flags.h"

/*
static int atoh(string s){
	stringstream ss;
	int x;
	ss << hex << s;
	ss >> x;
	return x;
}
 */


static string genError(string msg, int err_number){
	string s;
	char buffer[32];
	sprintf(buffer, " (%d)", err_number);
	s = msg + string(buffer);
	return s;
}

vector<string> Link::get_device_list(link_transport_mdriver_t * d, int max){
	vector<string> devices;
	char * snList;
	int i;
	char * sn;
	snList = link_new_device_list(d, max);
	devices.clear();
	if ( snList ){
		i = 0;
		while( ( strlen( (sn = link_device_list_entry(snList, i)) ) > 0 ) && (i < max) ){
			devices.push_back(sn);
			i++;
		}
		link_del_device_list(snList);
	}
	return devices;
}

Link::Link(){
	//check to see if the device is connected -- if it is not connected, connect to it
	m_stdout_fd = -1;
	m_stdin_fd = -1;
	m_lock = 0;
	m_boot = false;
	m_status_message = "";
	m_error_message = "";
	m_last_serialno = "";
	m_driver = &m_default_driver;
	link_load_default_driver(m_driver);
}

Link::~Link(){}


int Link::lock_device(){
	return driver()->lock( driver()->dev.handle );
}

int Link::unlock_device(){
	return driver()->unlock( driver()->dev.handle );
}

int Link::check_error(int err){
	switch(err){
	case LINK_PHY_ERROR:
		m_error_message = "Physical Connection Error";
		this->exit();
		return LINK_PHY_ERROR;
	case LINK_PROT_ERROR:
		m_error_message = "Protocol Error";
		return LINK_PROT_ERROR;
	}
	return err;
}


void Link::reset_progress(){
	m_progress = 0;
	m_progress_max = 0;
}


int Link::init(string sn){
	int err;

	reset_progress();

	if ( m_driver->dev.handle == LINK_PHY_OPEN_ERROR ){
		if( link_connect(m_driver, sn.c_str()) < 0 ){
			m_error_message = "Failed to Connect to Device";
			return -1;
		}

		m_last_serialno = sn;

	} else {
		link_debug(LINK_DEBUG_MESSAGE, "Already connected");
		m_error_message = genError("Already Connected", 1);
		return -1;
	}

	link_debug(LINK_DEBUG_MESSAGE, "Check for bootloader on 0x%llX", (uint64_t)m_driver->dev.handle);

	err = link_isbootloader(m_driver);

	if ( err > 0 ){
		link_debug(LINK_DEBUG_MESSAGE, "Bootloader connected");
		m_boot = true;
	} else if ( err == 0){
		m_boot = false;
	} else {
		m_error_message = genError("Failed to check for Bootloader status", link_errno);
		return -1;
	}

	link_debug(LINK_DEBUG_MESSAGE, "Init complete with 0x%llX", (uint64_t)m_driver->dev.handle);


	return 0;
}

int Link::open(string file, int flags, link_mode_t mode){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		link_debug(LINK_DEBUG_MESSAGE, "Open file %s on 0x%llX", file.c_str(), (uint64_t)m_driver->dev.handle);

		err = link_open(m_driver, file.c_str(), flags, mode);
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to open file: " + file, link_errno);
	}
	return check_error(err);
}

int Link::close(int fd){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_close(m_driver, fd);
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to close file", link_errno);
	}
	return check_error(err);
}


int Link::read(int fd, void * buf, int nbyte){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err =  link_read(m_driver, fd, buf, nbyte);
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to read", link_errno);
	}
	return check_error(err);
}

int Link::write(int fd, const void * buf, int nbyte){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err =  link_write(m_driver, fd, buf, nbyte);
		if(err != LINK_PROT_ERROR) break;
	}
	if ( err < 0 ){
		m_error_message = genError("Failed to write", link_errno);
	}
	unlock_device();
	return check_error(err);

}

int Link::read_flash(int addr, void * buf, int nbyte){
	int err;
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err =  link_readflash(m_driver, addr, buf, nbyte);
		if(err != LINK_PROT_ERROR) break;
	}
	if ( err < 0 ){
		m_error_message = genError("Failed to read flash", link_errno);
	}
	unlock_device();
	return check_error(err);

}

int Link::get_is_executing(string name){
	sys_taskattr_t task;
	int id;
	int err;
	int fd;

	if ( this->get_is_connected() == false ){
		return -1;
	}

	fd = open("/dev/sys", LINK_O_RDWR);
	if( fd < 0 ){
		this->m_error_message = "Failed to Open /dev/sys";
		return -1;
	}

	id = 0;

	do {
		task.is_enabled = 0;
		task.tid = id;
		memset(task.name, 0, 24);
		err = this->ioctl(fd, I_SYS_GETTASK, &task);

		if( err > 0 ){
			if( task.is_enabled != 0 ){
				if( string(task.name) == name ){
					close(fd);
					return task.pid;
				}
			}
		}
		id++;
	} while( err != -1 );

	close(fd);

	return -1;

}

int Link::write_flash(int addr, const void * buf, int nbyte){
	int err;
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err =  link_writeflash(m_driver, addr, buf, nbyte);
		if(err != LINK_PROT_ERROR) break;
	}
	if ( err < 0 ){
		m_error_message = genError("Failed to write flash", link_errno);
	}
	unlock_device();
	return check_error(err);
}


int Link::lseek(int fd, int offset, int whence){
	int err;
	if ( m_boot ){
		return -1;
	}

	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_lseek(m_driver, fd, offset, whence);
		if(err != LINK_PROT_ERROR) break;
	}
	if ( err < 0 ){
		m_error_message = genError("Failed to lseek", link_errno);
	}
	unlock_device();
	return check_error(err);

}


int Link::ioctl(int fd, int request, void * ctl){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_ioctl(m_driver, fd, request, ctl);
		if(err != LINK_PROT_ERROR) break;
	}
	if ( err < 0 ){
		m_error_message = genError("Failed to ioctl", link_errno);
	}
	unlock_device();
	return check_error(err);

}

int Link::exit(){
	lock_device();
	if ( m_driver->dev.handle != LINK_PHY_OPEN_ERROR ){
		link_disconnect(m_driver);

		if ( m_driver->dev.handle != LINK_PHY_OPEN_ERROR ){
			unlock_device(); //can't unlock the device because if it has been destroyed
		}
	}


	return 0;

}

bool Link::get_is_connected(){
	lock_device();
	if( m_driver->status(m_driver->dev.handle) == LINK_PHY_ERROR){
		m_driver->dev.handle = LINK_PHY_OPEN_ERROR;
	} else {
		unlock_device();
	}
	return (m_driver->dev.handle != LINK_PHY_OPEN_ERROR);
}

int Link::open_stdio(){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	err = link_open_stdio(m_driver);
	unlock_device();
	if ( err < 0 ){
		if ( err == LINK_TRANSFER_ERR ){
			m_error_message = "Connection Failed";
			this->exit();
			return -2;
		} else {
			m_error_message = "Failed to Open Stdio ";
			return -1;
		}
	}
	return err;
}

int Link::close_stdio(){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	err = link_close_stdio(m_driver);
	unlock_device();
	if ( err < 0 ){
		if ( err == LINK_TRANSFER_ERR ){
			m_error_message = "Connection Failed";
			this->exit();
			return -2;
		} else {
			m_error_message = genError("Failed to Close Stdio", link_errno);
			return -1;
		}
	}
	return err;
}

int Link::read_stdout(void * buf, int nbyte, volatile bool * abort){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	link_errno = 0;
	err = link_read_stdout(m_driver, buf, nbyte);
	unlock_device();
	if ( err < 0 ){
		if ( link_errno == 0 ){
			if ( abort == false ){
				m_error_message = "Connection Failed";
				this->exit();
			}
			return -2;
		} else {
			m_error_message = genError("Failed to Read Stdout", link_errno);
			return -1;
		}
	}
	return err;

}

int Link::write_stdin(const void * buf, int nbyte){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	err = link_write_stdin(m_driver, buf, nbyte);
	unlock_device();
	if ( err < 0 ){
		if ( err == LINK_TRANSFER_ERR ){
			m_error_message = "Connection Failed";
			return -1;
		} else {
			m_error_message = genError("Failed to Write Stdin", link_errno);
			return -1;
		}
	}
	return err;
}

int Link::stat(string path, struct link_stat * st){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_stat(m_driver, path.c_str(), st);
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if( err < 0 ){
		m_error_message = genError("Failed to Get Stat", link_errno);
	}
	return check_error(err);
}

int Link::get_time(struct tm * gt){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_gettime(m_driver, gt);
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to Get Time", link_errno);
	}
	return check_error(err);
}

int Link::set_time(struct tm * gt){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_settime(m_driver, gt);
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to Set Time", link_errno);
	}
	return check_error(err);
}

//Operations on the device
int Link::mkdir(string directory, link_mode_t mode){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_mkdir(m_driver, directory.c_str(), mode);
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to create " + directory, link_errno);
	}
	return check_error(err);
}

int Link::rmdir(string directory){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_rmdir(m_driver, directory.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to remove " + directory, link_errno);
	}
	return check_error(err);
}

vector<string> Link::get_dir_list(string directory){

	vector<string> list;
	int dirp;
	struct link_dirent entry;
	struct link_dirent * result;

	dirp = opendir(directory);

	if( dirp == 0 ){
		return list;
	}

	while( readdir_r(dirp, &entry, &result) == 0 ){
		list.push_back(entry.d_name);
	}

	closedir(dirp);

	return list;
}

int Link::opendir(string directory){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_opendir(m_driver, directory.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to open " + directory, link_errno);
	}
	return check_error(err);
}

int Link::readdir_r(int dirp, struct link_dirent * entry, struct link_dirent ** result){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_readdir_r(m_driver, dirp, entry, result);
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to read directory", link_errno);
		return -1;
	}
	return check_error(err);
}

int Link::closedir(int dirp){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_closedir(m_driver, dirp);
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to close directory", link_errno);
	}
	return check_error(err);
}

int Link::symlink(string oldPath, string newPath){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_symlink(m_driver, oldPath.c_str(), newPath.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to create symlink " + newPath, link_errno);
	}
	return check_error(err);
}

int Link::unlink(string filename){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_unlink(m_driver, filename.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to remove " + filename, link_errno);
	}
	return check_error(err);
}

int Link::copy(string src, string dest, link_mode_t mode, bool toDevice, bool (*update)(void*, int, int), void * context){
	FILE * hostFile;
	int err;
	int deviceFile;
	int flags;
	int bytesRead;
	const int bufferSize = 512;
	char buffer[bufferSize];
	struct link_stat st;

	if ( m_boot ){
		return -1;
	}

	err = 0;

	if ( toDevice == true ){

		m_progress_max = 0;
		m_progress = 0;

		//Open the host file
		hostFile = fopen(src.c_str(), "rb");
		if ( hostFile == NULL ){
			m_error_message = "Could not find file " + src + " on host.";
			return -1;
		}

		//Create the device file
		flags = LINK_O_TRUNC | LINK_O_CREAT | LINK_O_WRONLY; //The create new flag settings
		lock_device();
		deviceFile = link_open(m_driver, dest.c_str(), flags, mode);

		fseek(hostFile, 0, SEEK_END);
		m_progress_max = ftell(hostFile);
		rewind(hostFile);

		m_error_message = "";

		if ( deviceFile > 0 ){
			while( (bytesRead = fread(buffer, 1, bufferSize, hostFile)) > 0 ){
				if ( (err = link_write(m_driver, deviceFile, buffer, bytesRead)) != bytesRead ){
					m_error_message = genError("Failed to write to Link device file", link_errno);
					if ( err > 0 ){
						err = -1;
					}
					break;
				} else {
					m_progress += bytesRead;
					if( update != 0 ){
						if( update(context, m_progress, m_progress_max) == true ){
							//update progress and check for abort
							break;
						}
					}
					err = 0;
				}
			}
		} else {
			unlock_device();
			fclose(hostFile);

			if ( deviceFile == LINK_TRANSFER_ERR ){
				m_error_message = "Connection Failed";
				this->exit();
				return -2;
			}

			m_error_message = genError("Failed to create file " + dest + " on Link device", link_errno);
			return -1;
		}

		fclose(hostFile);

		if ( err == LINK_TRANSFER_ERR ){
			unlock_device();
			m_error_message = "Connection Failed";
			this->exit();
			return -2;
		}

		if ( link_close(m_driver, deviceFile) ){
			m_error_message = genError("Failed to close Link device file", link_errno);
			unlock_device();
			return -1;
		}
		unlock_device();

		return err;

	} else {


		if ( link_stat(m_driver, src.c_str(), &st) < 0 ){
			m_error_message = "Failed to get target file size";
			return -1;
		}

		m_progress = 0;
		m_progress_max = 0;
		//Copy the source file from the device to the host
		hostFile = fopen(dest.c_str(), "wb");
		if ( hostFile == NULL ){
			m_error_message = "Failed to open file " + dest + " on host.";
			return -1;
		}


		//Open the device file
		flags = LINK_O_RDONLY; //Read the file only
		lock_device();
		deviceFile = link_open(m_driver, src.c_str(), flags, 0);

		if ( deviceFile > 0 ){
			m_progress_max = st.st_size;

			while( (bytesRead = link_read(m_driver, deviceFile, buffer, bufferSize)) > 0 ){
				fwrite(buffer, 1, bytesRead, hostFile);
				m_progress += bytesRead;
				if( update != 0 ){
					if( update(context, m_progress, m_progress_max) == true ){
						//update progress and check for abort
						break;
					}
				}
				if ( bytesRead < bufferSize ){
					break;
				}
			}
		} else {
			if ( deviceFile == LINK_TRANSFER_ERR ){
				m_error_message = "Connection Failed";
				unlock_device();
				this->exit();
				return -2;
			} else {
				m_error_message = genError("Failed to open file " + src + " on Link device", link_errno);
				fclose(hostFile);
				unlock_device();
				return -1;
			}
		}

		fclose(hostFile);

		if ( (err = link_close(m_driver, deviceFile)) ){
			if ( err == LINK_TRANSFER_ERR ){
				m_error_message = "Connection Failed";
				unlock_device();
				this->exit();
				return -2;
			} else {
				m_error_message = genError("Failed to close Link file", link_errno);
				unlock_device();
				return -1;
			}
		}
		unlock_device();
	}
	return 0;
}

int Link::run_app(string path){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_exec(m_driver, path.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		if ( err == LINK_TRANSFER_ERR ){
			m_error_message = "Connection Failed";
			this->exit();
			return -2;
		} else {
			m_error_message = genError("Failed to run program: " + path, link_errno);
			return -1;
		}
	}
	return err;
}

int Link::format(string path){
	int err;
	if ( m_boot ){
		return -1;
	}
	m_error_message = "";
	//Format the filesystem
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_mkfs(m_driver, path.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to Format Filesystem", link_errno);
	}
	return check_error(err);
}

int Link::kill_pid(int pid, int signo){
	int err;
	stringstream ss;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_kill_pid(m_driver, pid, signo);
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		ss << "Failed to kill process " << pid;
		m_error_message = genError(ss.str(), link_errno);
	}
	return check_error(err);
}

int Link::reset(){
	lock_device();
	link_reset(m_driver);
	unlock_device();
	m_driver->dev.handle = LINK_PHY_OPEN_ERROR;
	return 0;
}

int Link::reset_bootloader(){
	lock_device();
	link_resetbootloader(m_driver);
	unlock_device();
	m_driver->dev.handle = LINK_PHY_OPEN_ERROR;
	return 0;
}

int Link::trace_create(int pid, link_trace_id_t * id){
	int err;
	lock_device();
	err = link_posix_trace_create(m_driver, pid, id);
	if ( err < 0 ){
		m_error_message = genError("Failed to create trace", link_errno);
	}
	unlock_device();
	return err;
}

int Link::trace_tryget_events(link_trace_id_t id, void * data, size_t num_bytes){
	int err;
	lock_device();
	err = link_posix_trace_tryget_events(m_driver, id, data, num_bytes);
	if ( err < 0 ){
		m_error_message = genError("Failed to get event", link_errno);
	}
	unlock_device();
	return err;
}

int Link::trace_shutdown(link_trace_id_t id){
	int err;
	lock_device();
	err = link_posix_trace_shutdown(m_driver, id);
	if ( err < 0 ){
		m_error_message = genError("Failed to shutdown trace", link_errno);
	}
	unlock_device();
	return err;
}

int Link::get_security_addr(uint32_t * addr){
	return -1;
}

int Link::rename(string old_path, string new_path){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_rename(m_driver, old_path.c_str(), new_path.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if( err < 0 ){
		m_error_message = genError("Failed to rename file", link_errno);
	}
	return check_error(err);
}

int Link::chown(string path, int owner, int group){
	int err;
	if ( m_boot ){
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_chown(m_driver, path.c_str(), owner, group);
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to chown file", link_errno);
	}
	return check_error(err);
}

int Link::chmod(string path, int mode){
	int err;
	if ( m_boot ){
		m_error_message = "Target is a bootloader";
		return -1;
	}
	lock_device();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_chmod(m_driver, path.c_str(), mode);
		if(err != LINK_PROT_ERROR) break;
	}
	unlock_device();
	if ( err < 0 ){
		m_error_message = genError("Failed to chmod file", link_errno);
	}
	return check_error(err);

}

int Link::update_os(string path, bool verify, bool (*update)(void*,int,int), void * context){
	int err;
	uint32_t loc;
	int bytesRead;
	FILE * hostFile;
	char stackaddr[256];
	const int bufferSize = 1024;
	unsigned char buffer[bufferSize];
	unsigned char cmpBuffer[bufferSize];
	int i;
	bootloader_attr_t attr;
	uint32_t startAddr;
	uint32_t image_id;
	char tmp[256];

	if ( m_boot == false ){
		m_error_message = "Target is not a bootloader";
		return -1;
	}

	//now write the OS to the device using link_writeflash()
	m_progress_max = 0;
	m_progress = 0;

	//Open the host file
	hostFile = fopen(path.c_str(), "rb");
	if ( hostFile == NULL ){
		m_error_message = "Could not find file " + path + " on host.";
		unlock_device();
		return -1;
	}

	fseek(hostFile, BOOTLOADER_HARDWARE_ID_OFFSET, SEEK_SET);
	fread(&image_id, 1, sizeof(uint32_t), hostFile);

	fseek(hostFile, 0, SEEK_END);
	m_progress_max = ftell(hostFile);
	rewind(hostFile);


	err = link_bootloader_attr(m_driver, &attr, image_id);
	//err = link_ioctl(d, LINK_BOOTLOADER_FILDES, I_BOOTLOADER_GETATTR, &attr);
	if( err < 0 ){
		unlock_device();
		m_error_message = "Failed to read attributes";
		fclose(hostFile);
		return check_error(err);
	}

	startAddr = attr.startaddr;
	loc = startAddr;

	if( image_id != attr.hardware_id ){
		err = -1;
		sprintf(tmp,
				"Kernel Image ID (0x%X) does not match Bootloader ID (0x%X)",
				image_id,
				attr.hardware_id);
		m_error_message = genError(tmp, link_errno);
		fclose(hostFile);
		return check_error(err);
	}

	lock_device();
	//first erase the flash
	err = link_eraseflash(m_driver);
	unlock_device();

	if ( err < 0 ){
		fclose(hostFile);
		m_error_message = "Failed to erase flash";
		unlock_device();
		return check_error(err);
	}

	m_error_message = "";
	m_status_message = "Writing OS to Target...";


	while( (bytesRead = fread(buffer, 1, bufferSize, hostFile)) > 0 ){

		if( loc == startAddr ){
			memcpy(stackaddr, buffer, 256);
			memset(buffer, 0xFF, 256);
		}

		if ( (err = link_writeflash(m_driver, loc, buffer, bytesRead)) != bytesRead ){
			m_error_message = genError("Failed to write to link flash", link_errno);
			if ( err < 0 ){
				err = -1;
			}
			break;
		}

		loc += bytesRead;
		m_progress += bytesRead;
		if( update(context, m_progress, m_progress_max) == true ){
			//update progress and check for abort
			break;
		}
		err = 0;
	}

	if ( err == 0 ){

		if ( verify == true ){

			rewind(hostFile);
			loc = startAddr;
			m_progress = 0;

			m_status_message = "Verifying...";


			while( (bytesRead = fread(buffer, 1, bufferSize, hostFile)) > 0 ){

				if ( (err = link_readflash(m_driver, loc, cmpBuffer, bytesRead)) != bytesRead ){
					m_error_message = genError("Failed to read flash memory", link_errno);
					if ( err > 0 ){
						err = -1;
					}
					break;
				} else {

					if( loc == startAddr ){
						memset(buffer, 0xFF, 256);
					}

					if ( memcmp((void*)cmpBuffer, buffer, bytesRead) != 0 ){
						for(i=0; i < bytesRead; i++){
							if( buffer[i] != cmpBuffer[i] ){
								//printf("0x%X targ:0x%02X actual:0x%02X", loc + i, buffer[i], cmpBuffer[i]);
							}
						}
						m_error_message = genError("Failed to verify program installation", link_errno);
						fclose(hostFile);

						//erase the flash
						//link_eraseflash(handle);

						return -1;
					}

					loc += bytesRead;
					m_progress += bytesRead;
					if( update(context, m_progress, m_progress_max) == true ){
						//update progress and check for abort
						break;
					}
					err = 0;
				}
			}
		}

		//write the stack address
		if( (err = link_writeflash(m_driver, startAddr, stackaddr, 256)) != 256 ){
			m_error_message = genError("Failed to write stack addr", err);
			return -1;
		}


		if( verify == true ){
			//verify the stack address
			if( (err = link_readflash(m_driver, startAddr, buffer, 256)) != 256 ){
				m_error_message = genError("Failed to write stack addr", err);
				fclose(hostFile);
				return -1;
			}

			if( memcmp(buffer, stackaddr, 256) != 0 ){
				link_eraseflash(m_driver);
				m_error_message = "Failed to verify stack address";
				fclose(hostFile);
				return -1;
			}
		}

		m_status_message = "Finalizing...";

	}

	m_status_message = "Done";

	fclose(hostFile);
	unlock_device();

	if( err < 0 ){
		link_eraseflash(m_driver);
	}

	return check_error(err);
}




