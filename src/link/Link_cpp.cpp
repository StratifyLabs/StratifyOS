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

vector<string> Link::listDevices(link_transport_mdriver_t * d, int max){
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
	stdoutFd = -1;
	stdinFd = -1;
	lock = 0;
	isBoot = false;
	statusMessage = "";
	errMsg = "";
	lastsn = "";
	d = &drvr;
	link_load_default_driver(d);
}

Link::~Link(){}


int Link::lockDevice(void){
	return 0;
}

int Link::unlockDevice(void){
	return 0;
}

int Link::checkError(int err){
	switch(err){
	case LINK_PHY_ERROR:
		errMsg = "Physical Connection Error";
		this->exit();
		return LINK_PHY_ERROR;
	case LINK_PROT_ERROR:
		errMsg = "Protocol Error";
		return LINK_PROT_ERROR;
	}
	return err;
}

int Link::getProgress(void){
	return progress;
}

int Link::getProgessMax(void){
	return progressMax;
}

void Link::resetProgress(void){
	progress = 0;
	progressMax = 0;
}

string Link::getStatusMessage(void){
	return statusMessage;
}

int Link::init(string sn){
	int err;

	resetProgress();

	if ( d->dev.handle == LINK_PHY_OPEN_ERROR ){
		if( link_connect(d, sn.c_str()) < 0 ){
			errMsg = "Failed to Connect to Device";
			return -1;
		}

		lastsn = sn;

	} else {
		link_debug(LINK_DEBUG_MESSAGE, "Already connected");
		errMsg = genError("Already Connected", 1);
		return -1;
	}

	link_debug(LINK_DEBUG_MESSAGE, "Check for bootloader on 0x%llX", (uint64_t)d->dev.handle);

	err = link_isbootloader(d);

	if ( err > 0 ){
		link_debug(LINK_DEBUG_MESSAGE, "Bootloader connected");
		isBoot = true;
	} else if ( err == 0){
		isBoot = false;
	} else {
		errMsg = genError("Failed to check for Bootloader status", link_errno);
		return -1;
	}

	link_debug(LINK_DEBUG_MESSAGE, "Init complete with 0x%llX", (uint64_t)d->dev.handle);


	return 0;
}

int Link::open(string file, int flags, link_mode_t mode){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		link_debug(LINK_DEBUG_MESSAGE, "Open file %s on 0x%llX", file.c_str(), (uint64_t)d->dev.handle);

		err = link_open(d, file.c_str(), flags, mode);
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to open file: " + file, link_errno);
	}
	return checkError(err);
}

int Link::close(int fd){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_close(d, fd);
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to close file", link_errno);
	}
	return checkError(err);
}


int Link::read(int fd, void * buf, int nbyte){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err =  link_read(d, fd, buf, nbyte);
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to read", link_errno);
	}
	return checkError(err);
}

int Link::write(int fd, const void * buf, int nbyte){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err =  link_write(d, fd, buf, nbyte);
		if(err != LINK_PROT_ERROR) break;
	}
	if ( err < 0 ){
		errMsg = genError("Failed to write", link_errno);
	}
	unlockDevice();
	return checkError(err);

}

int Link::readFlash(int addr, void * buf, int nbyte){
	int err;
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err =  link_readflash(d, addr, buf, nbyte);
		if(err != LINK_PROT_ERROR) break;
	}
	if ( err < 0 ){
		errMsg = genError("Failed to read flash", link_errno);
	}
	unlockDevice();
	return checkError(err);

}

int Link::isExecuting(string name){
	sys_taskattr_t task;
	int id;
	int err;
	int fd;

	if ( this->connected() == false ){
		return -1;
	}

	fd = open("/dev/sys", LINK_O_RDWR);
	if( fd < 0 ){
		this->errMsg = "Failed to Open /dev/sys";
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

int Link::writeFlash(int addr, const void * buf, int nbyte){
	int err;
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err =  link_writeflash(d, addr, buf, nbyte);
		if(err != LINK_PROT_ERROR) break;
	}
	if ( err < 0 ){
		errMsg = genError("Failed to write flash", link_errno);
	}
	unlockDevice();
	return checkError(err);
}


int Link::lseek(int fd, int offset, int whence){
	int err;
	if ( isBoot ){
		return -1;
	}

	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_lseek(d, fd, offset, whence);
		if(err != LINK_PROT_ERROR) break;
	}
	if ( err < 0 ){
		errMsg = genError("Failed to lseek", link_errno);
	}
	unlockDevice();
	return checkError(err);

}


int Link::ioctl(int fd, int request, void * ctl){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_ioctl(d, fd, request, ctl);
		if(err != LINK_PROT_ERROR) break;
	}
	if ( err < 0 ){
		errMsg = genError("Failed to ioctl", link_errno);
	}
	unlockDevice();
	return checkError(err);

}

int Link::exit(void){
	lockDevice();
	if ( d->dev.handle != LINK_PHY_OPEN_ERROR ){
		link_disconnect(d);
		d->dev.handle = LINK_PHY_OPEN_ERROR;
	}
	unlockDevice();
	return 0;

}

bool Link::connected(void){

	if( d->status(d->dev.handle) == LINK_PHY_ERROR){
		d->dev.handle = LINK_PHY_OPEN_ERROR;
	}
	return (d->dev.handle != LINK_PHY_OPEN_ERROR);
}

int Link::openStdio(void){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	err = link_open_stdio(d);
	unlockDevice();
	if ( err < 0 ){
		if ( err == LINK_TRANSFER_ERR ){
			errMsg = "Connection Failed";
			this->exit();
			return -2;
		} else {
			errMsg = "Failed to Open Stdio ";
			return -1;
		}
	}
	return err;
}

int Link::closeStdio(void){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	err = link_close_stdio(d);
	unlockDevice();
	if ( err < 0 ){
		if ( err == LINK_TRANSFER_ERR ){
			errMsg = "Connection Failed";
			this->exit();
			return -2;
		} else {
			errMsg = genError("Failed to Close Stdio", link_errno);
			return -1;
		}
	}
	return err;
}

int Link::readStdout(void * buf, int nbyte, volatile bool * abort){
	int err;
	if ( isBoot ){
		return -1;
	}
	//lockDevice();
	link_errno = 0;
	err = link_read_stdout(d, buf, nbyte);
	//unlockDevice();
	if ( err < 0 ){
		if ( link_errno == 0 ){
			if ( abort == false ){
				errMsg = "Connection Failed";
				this->exit();
			}
			return -2;
		} else {
			errMsg = genError("Failed to Read Stdout", link_errno);
			return -1;
		}
	}
	return err;

}

int Link::writeStdin(const void * buf, int nbyte){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	err = link_write_stdin(d, buf, nbyte);
	unlockDevice();
	if ( err < 0 ){
		if ( err == LINK_TRANSFER_ERR ){
			errMsg = "Connection Failed";
			return -1;
		} else {
			errMsg = genError("Failed to Write Stdin", link_errno);
			return -1;
		}
	}
	return err;
}

int Link::stat(string path, struct link_stat * st){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_stat(d, path.c_str(), st);
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if( err < 0 ){
		errMsg = genError("Failed to Get Stat", link_errno);
	}
	return checkError(err);
}

int Link::getTime(struct tm * gt){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_gettime(d, gt);
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to Get Time", link_errno);
	}
	return checkError(err);
}

int Link::setTime(struct tm * gt){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_settime(d, gt);
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to Set Time", link_errno);
	}
	return checkError(err);
}

//Operations on the device
int Link::mkdir(string directory, link_mode_t mode){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_mkdir(d, directory.c_str(), mode);
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to create " + directory, link_errno);
	}
	return checkError(err);
}

int Link::rmdir(string directory){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_rmdir(d, directory.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to remove " + directory, link_errno);
	}
	return checkError(err);
}

int Link::opendir(string directory){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_opendir(d, directory.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to open " + directory, link_errno);
	}
	return checkError(err);
}

int Link::readdir_r(int dirp, struct link_dirent * entry, struct link_dirent ** result){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_readdir_r(d, dirp, entry, result);
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to read directory", link_errno);
		return -1;
	}
	return checkError(err);
}

int Link::closedir(int dirp){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_closedir(d, dirp);
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to close directory", link_errno);
	}
	return checkError(err);
}

int Link::symlink(string oldPath, string newPath){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_symlink(d, oldPath.c_str(), newPath.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to create symlink " + newPath, link_errno);
	}
	return checkError(err);
}

int Link::unlink(string filename){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_unlink(d, filename.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to remove " + filename, link_errno);
	}
	return checkError(err);
}

int Link::cp(string src, string dest, link_mode_t mode, bool toDevice, bool (*update)(void*, int, int), void * context){
	FILE * hostFile;
	int err;
	int deviceFile;
	int flags;
	int bytesRead;
	const int bufferSize = 512;
	char buffer[bufferSize];
	struct link_stat st;

	if ( isBoot ){
		return -1;
	}

	err = 0;

	if ( toDevice == true ){

		progressMax = 0;
		progress = 0;

		//Open the host file
		hostFile = fopen(src.c_str(), "rb");
		if ( hostFile == NULL ){
			errMsg = "Could not find file " + src + " on host.";
			return -1;
		}

		//Create the device file
		flags = LINK_O_TRUNC | LINK_O_CREAT | LINK_O_WRONLY; //The create new flag settings
		lockDevice();
		deviceFile = link_open(d, dest.c_str(), flags, mode);

		fseek(hostFile, 0, SEEK_END);
		progressMax = ftell(hostFile);
		rewind(hostFile);

		errMsg = "";

		if ( deviceFile > 0 ){
			while( (bytesRead = fread(buffer, 1, bufferSize, hostFile)) > 0 ){
				if ( (err = link_write(d, deviceFile, buffer, bytesRead)) != bytesRead ){
					errMsg = genError("Failed to write to Link device file", link_errno);
					if ( err > 0 ){
						err = -1;
					}
					break;
				} else {
					progress += bytesRead;
					if( update != 0 ){
						if( update(context, progress, progressMax) == true ){
							//update progress and check for abort
							break;
						}
					}
					err = 0;
				}
			}
		} else {
			unlockDevice();
			fclose(hostFile);

			if ( deviceFile == LINK_TRANSFER_ERR ){
				errMsg = "Connection Failed";
				this->exit();
				return -2;
			}

			errMsg = genError("Failed to create file " + dest + " on Link device", link_errno);
			return -1;
		}

		fclose(hostFile);

		if ( err == LINK_TRANSFER_ERR ){
			unlockDevice();
			errMsg = "Connection Failed";
			this->exit();
			return -2;
		}

		if ( link_close(d, deviceFile) ){
			errMsg = genError("Failed to close Link device file", link_errno);
			unlockDevice();
			return -1;
		}
		unlockDevice();

		return err;

	} else {


		if ( link_stat(d, src.c_str(), &st) < 0 ){
			errMsg = "Failed to get target file size";
			return -1;
		}

		progress = 0;
		progressMax = 0;
		//Copy the source file from the device to the host
		hostFile = fopen(dest.c_str(), "wb");
		if ( hostFile == NULL ){
			errMsg = "Failed to open file " + dest + " on host.";
			return -1;
		}


		//Open the device file
		flags = LINK_O_RDONLY; //Read the file only
		lockDevice();
		deviceFile = link_open(d, src.c_str(), flags, 0);

		if ( deviceFile > 0 ){
			progressMax = st.st_size;

			while( (bytesRead = link_read(d, deviceFile, buffer, bufferSize)) > 0 ){
				fwrite(buffer, 1, bytesRead, hostFile);
				progress += bytesRead;
				if( update != 0 ){
					if( update(context, progress, progressMax) == true ){
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
				errMsg = "Connection Failed";
				unlockDevice();
				this->exit();
				return -2;
			} else {
				errMsg = genError("Failed to open file " + src + " on Link device", link_errno);
				fclose(hostFile);
				unlockDevice();
				return -1;
			}
		}

		fclose(hostFile);

		if ( (err = link_close(d, deviceFile)) ){
			if ( err == LINK_TRANSFER_ERR ){
				errMsg = "Connection Failed";
				unlockDevice();
				this->exit();
				return -2;
			} else {
				errMsg = genError("Failed to close Link file", link_errno);
				unlockDevice();
				return -1;
			}
		}
		unlockDevice();
	}
	return 0;
}

int Link::runApp(string path){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_exec(d, path.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		if ( err == LINK_TRANSFER_ERR ){
			errMsg = "Connection Failed";
			this->exit();
			return -2;
		} else {
			errMsg = genError("Failed to run program: " + path, link_errno);
			return -1;
		}
	}
	return err;
}

int Link::format(string path){
	int err;
	if ( isBoot ){
		return -1;
	}
	errMsg = "";
	//Format the filesystem
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_mkfs(d, path.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to Format Filesystem", link_errno);
	}
	return checkError(err);
}

int Link::killPid(int pid, int signo){
	int err;
	stringstream ss;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_kill_pid(d, pid, signo);
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		ss << "Failed to kill process " << pid;
		errMsg = genError(ss.str(), link_errno);
	}
	return checkError(err);
}

int Link::reset(void){
	lockDevice();
	link_reset(d);
	unlockDevice();
	d->dev.handle = LINK_PHY_OPEN_ERROR;
	return 0;
}

int Link::resetBootloader(void){
	lockDevice();
	link_resetbootloader(d);
	unlockDevice();
	d->dev.handle = LINK_PHY_OPEN_ERROR;
	return 0;
}

int Link::trace_create(int pid, link_trace_id_t * id){
	int err;
	lockDevice();
	err = link_posix_trace_create(d, pid, id);
	if ( err < 0 ){
		errMsg = genError("Failed to create trace", link_errno);
	}
	unlockDevice();
	return err;
}

int Link::trace_tryget_events(link_trace_id_t id, void * data, size_t num_bytes){
	int err;
	lockDevice();
	err = link_posix_trace_tryget_events(d, id, data, num_bytes);
	if ( err < 0 ){
		errMsg = genError("Failed to get event", link_errno);
	}
	unlockDevice();
	return err;
}

int Link::trace_shutdown(link_trace_id_t id){
	int err;
	lockDevice();
	err = link_posix_trace_shutdown(d, id);
	if ( err < 0 ){
		errMsg = genError("Failed to shutdown trace", link_errno);
	}
	unlockDevice();
	return err;
}

bool Link::isBootloader(void){
	return isBoot;
}

int Link::getSecurityAddr(uint32_t * addr){
	return -1;
}

int Link::rename(string old_path, string new_path){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_rename(d, old_path.c_str(), new_path.c_str());
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if( err < 0 ){
		errMsg = genError("Failed to rename file", link_errno);
	}
	return checkError(err);
}

int Link::chown(string path, int owner, int group){
	int err;
	if ( isBoot ){
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_chown(d, path.c_str(), owner, group);
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to chown file", link_errno);
	}
	return checkError(err);
}

int Link::chmod(string path, int mode){
	int err;
	if ( isBoot ){
		errMsg = "Target is a bootloader";
		return -1;
	}
	lockDevice();
	for(int tries = 0; tries < MAX_TRIES; tries++){
		err = link_chmod(d, path.c_str(), mode);
		if(err != LINK_PROT_ERROR) break;
	}
	unlockDevice();
	if ( err < 0 ){
		errMsg = genError("Failed to chmod file", link_errno);
	}
	return checkError(err);

}

int Link::updateOS(string path, bool verify, bool (*update)(void*,int,int), void * context){
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

	if ( isBoot == false ){
		errMsg = "Target is not a bootloader";
		return -1;
	}

	//now write the OS to the device using link_writeflash()
	progressMax = 0;
	progress = 0;

	//Open the host file
	hostFile = fopen(path.c_str(), "rb");
	if ( hostFile == NULL ){
		errMsg = "Could not find file " + path + " on host.";
		unlockDevice();
		return -1;
	}

	fseek(hostFile, BOOTLOADER_HARDWARE_ID_OFFSET, SEEK_SET);
	fread(&image_id, 1, sizeof(uint32_t), hostFile);

	fseek(hostFile, 0, SEEK_END);
	progressMax = ftell(hostFile);
	rewind(hostFile);


	err = link_bootloader_attr(d, &attr, image_id);
	//err = link_ioctl(d, LINK_BOOTLOADER_FILDES, I_BOOTLOADER_GETATTR, &attr);
	if( err < 0 ){
		unlockDevice();
		errMsg = "Failed to read attributes";
		fclose(hostFile);
		return checkError(err);
	}

	startAddr = attr.startaddr;
	loc = startAddr;

	if( image_id != attr.hardware_id ){
		err = -1;
		sprintf(tmp,
				"Kernel Image ID (0x%X) does not match Bootloader ID (0x%X)",
				image_id,
				attr.hardware_id);
		errMsg = genError(tmp, link_errno);
		fclose(hostFile);
		return checkError(err);
	}

	lockDevice();
	//first erase the flash
	err = link_eraseflash(d);
	unlockDevice();

	if ( err < 0 ){
		fclose(hostFile);
		errMsg = "Failed to erase flash";
		unlockDevice();
		return checkError(err);
	}

	errMsg = "";
	statusMessage = "Writing OS to Target...";


	while( (bytesRead = fread(buffer, 1, bufferSize, hostFile)) > 0 ){

		if( loc == startAddr ){
			memcpy(stackaddr, buffer, 256);
			memset(buffer, 0xFF, 256);
		}

		if ( (err = link_writeflash(d, loc, buffer, bytesRead)) != bytesRead ){
			errMsg = genError("Failed to write to link flash", link_errno);
			if ( err < 0 ){
				err = -1;
			}
			break;
		}

		loc += bytesRead;
		progress += bytesRead;
		if( update(context, progress, progressMax) == true ){
			//update progress and check for abort
			break;
		}
		err = 0;
	}

	if ( err == 0 ){

		if ( verify == true ){

			rewind(hostFile);
			loc = startAddr;
			progress = 0;

			statusMessage = "Verifying...";


			while( (bytesRead = fread(buffer, 1, bufferSize, hostFile)) > 0 ){

				if ( (err = link_readflash(d, loc, cmpBuffer, bytesRead)) != bytesRead ){
					errMsg = genError("Failed to read flash memory", link_errno);
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
						errMsg = genError("Failed to verify program installation", link_errno);
						fclose(hostFile);

						//erase the flash
						//link_eraseflash(handle);

						return -1;
					}

					loc += bytesRead;
					progress += bytesRead;
					if( update(context, progress, progressMax) == true ){
						//update progress and check for abort
						break;
					}
					err = 0;
				}
			}
		}

		//write the stack address
		if( (err = link_writeflash(d, startAddr, stackaddr, 256)) != 256 ){
			errMsg = genError("Failed to write stack addr", err);
			return -1;
		}


		if( verify == true ){
			//verify the stack address
			if( (err = link_readflash(d, startAddr, buffer, 256)) != 256 ){
				errMsg = genError("Failed to write stack addr", err);
				fclose(hostFile);
				return -1;
			}

			if( memcmp(buffer, stackaddr, 256) != 0 ){
				link_eraseflash(d);
				errMsg = "Failed to verify stack address";
				fclose(hostFile);
				return -1;
			}
		}

		statusMessage = "Finalizing...";

	}

	statusMessage = "Done";

	fclose(hostFile);
	unlockDevice();

	if( err < 0 ){
		link_eraseflash(d);
	}

	return checkError(err);
}




