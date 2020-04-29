---
date: "2019-06-27"
layout: post
title: Understanding Threads on Stratify OS
katex: true
slug: Guide-Threads
menu:
  sidebar:
    name: Threads
    parent: Guides
---

Stratify OS threads are designed according the POSIX specification. Stratify OS includes an embedded friendly C++ API called the Stratify API that makes it extremely easy to manage threads.

## POSIX Thread Concepts

If you are new to POSIX threads, this is a short explanation to understand the concepts.

- Processes
- Threads
- Mutexes
- Other Tools

### Processes

On a typical operating system (Linux, Windows, Mac), a process is a compiled program that can be stored on disk then loaded into its own virtual address space upon execution. The processor uses a memory management unit (MMU) to map the virtual address space to physical memory in real-time. Each process has at least one thread which defines a sequence of instructions that are executing. New threads created within the the process share the same virtual address space with an independent stack and shared heap.

Stratify OS can run processes [but it does so on hardware without an MMU](https://blog.stratifylabs.co/device/2014-05-03-Applications-without-MMU/). At compile time, the process is built to run in a faux address space. When it is copied to RAM or flash for execution, Stratify OS uses a code relocation algorithm and the hardware memory protection unit (MPU) to mimic the behavior of the MMU.

In short, processes are compiled programs whose memory is protected from access by other programs. Threads are execution sequences within a process.

### Threads

To create a thread, you need to specify a function to execute, an argument to pass to that function and the thread attributes (stack, priority, etc).

```c++
#include <pthread.h>

int pthread_create(pthread_t * thread, 
    const pthread_attr_t * attr,
    void *(*start_routine)(void*), void * arg);
```

The StratifyAPI has a thread class that sets default attributes and allows thread creation in a few lines of code.

```c++
#include <sapi/sys.hpp>

static void * start_function(void * arg){
   //this thread doesn't do much
   return arg;
}

int main(int argc, char * argv[]){
   Thread thread(4096); //4096 is the stack size
   thread.create(start_function); //this will add the thread to the scheduler

   while( thread.is_running() ){
      ;
   }

   return 0;
}

```

#### Detached or Joinable

A thread can be created as detached or joinable. If you want to know the return value of the thread or block until it completes, you will want to create a joinable thread. Otherwise, you will create a detached thread. The example above is a detached thread. The example below shows how to use a joinable thread.

```c++
#include <sapi/sys.hpp>

static void * start_function(void * arg){
   //this thread doesn't do much
   return arg;
}

int main(int argc, char * argv[]){
   void * arg = 0;
   Thread thread(4096, false); //4096 is the stack size, detached is false
   thread.create(start_function, arg); //this will add the thread to the scheduler
   void * arg_pointer;

   thread.join( &arg_pointer ); //this will block until the thread is done
  
   //arg_pointer will be zero because start_function returns 0

   return 0;
}
```

For more information, you can read the [sys::Thread documentation](../StratifyAPI/#classsys_1_1_thread)).

### Mutex

A mutex, short for mutual exclusion, is a software concept that allows only one thread to access a resource at a time.  When a thread "locks" a mutex, the thread will block until the mutex is available and then continue once it acquires the mutex. When the thread is done using the mutex, it "unlocks" the mutex allowing another thread to use it.

Consider a board that has an accelerometer and a fuel gauge on the I2C bus. An application might use one thread to monitor the accelerometer and one to monitor the fuel gauge. However, both sensors use the same I2C bus which can only address one sensor at a time. A mutex allows the each thread exclusive access to the I2C bus.

```c++
#include <sapi/hal.hpp> //I2C 
#include <sapi/sys.hpp> //Thread
#include <sapi/chrono.hpp> //Timer

static Mutex i2c_mutex;
static I2C i2c(0); //bus zero

static void * fuel_gauge_thread_function(void * args);
static void * accelerometer_thread_function(void * args);

int main(int argc, char * argv[]){
    Thread fuel_guage_thread;
    Thread accelerometer_thread;

    fuel_gauge_thread.create(fuel_gauge_thread_function);
    accelerometer_thread.create(accelerometer_thread_function);

    fuel_guage_thread.wait();
    accelerometer_thread.wait();

   return 0;
}


void * fuel_gauge_thread_function(void * args){
 while(1){
        fuel_guage_t fuel_guage_data;
        i2c_mutex.lock();
        i2c.prepare(FUEL_GUAGE_ADDR);
        //If the context changes here to accelerometer_thread_function(), 
        //it will lock the mutex and block until this thread unlocks it.
        i2c.read(0, &fuel_guage_data, sizeof(fuel_guage_data));
        i2c_mutex.unlock();
        //do some processing
        Timer::wait_milliseconds(1000); //sample at 1Hz
    }
    return 0;
}

void * accelerometer_thread_function(void * args){
    while(1){
        accel_t accel_data;
        i2c_mutex.lock();
        i2c.prepare(ACCEL_ADDR);
        i2c.read(0, &accel_data, sizeof(accel_data));
        i2c_mutex.unlock();
        //do some processing
        Timer::wait_milliseconds(100); //sample at 10Hz
    }
    return 0;
}
```

If you run into a situation where a high priority thread and a low priority thread need to share a resource, you can set the mutex priority ceiling to the value of the higher priority thread. This will cause any thread that locks the mutex to have an elevated priority until the mutex is unlocked. This is standard POSIX behavior and can be accessed using [sys::MutexAttr](../StratifyAPI/#classsys_1_1_mutex_attributes).

```c++
#include <sapi/sys.hpp>
MutexAttr attr;
attr.set_prio_ceiling(20);
Mutex mutex(attr); //creates a mutex with a priority ceiling of 20
```
### Other Tools

There are many other tools in the POSIX specification that are implemented in Stratify OS that allow you to work with multiple threads (and processes).

- [Semphores](../StratifyAPI/#classsys_1_1_sem)
- [Signals](../StratifyAPI/#classsys_1_1_signal)
- [Message Queues](../StratifyAPI/#classsys_1_1_mq)
- [Scheduler](../StratifyAPI/#classsys_1_1_sched)

The [sys::TaskManager class](../StratifyAPI/#classsys_1_1_task_manager) from the StratifyAPI uses the `/dev/sys` driver to see how much resources each task (process or thread) is using and comes in very handy while debugging resource constrained applications.

## Try the Demo

Once you have Stratify OS installed using the [web application tutorials](https://app.stratifylabs.co), you can install the thread-demo app from the cloud and check out the [source code on Github](https://github.com/StratifyLabs/thread-demo).

