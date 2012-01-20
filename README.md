# ARCADE "Airborne" Software

This repository includes all software components which run on the ARCADE UAV Linux system.

## Filesystem Structure

The brief file system structure summary of the airborne software looks like the following:


* __MOBICOM__ (MOBICOM\_PATH defines top-level path)
    * __common__ [MOBICOM common](http://github.com/MOBICOM/common) , aka __"level 1 common"__ 
        * __scl__: Signaling and Communication Link
        * __svctrl__: Service Control Utility
        * __scripts__: bashrc (sourced from user bashrc)
    * __ARCADE__ (MOBICOM\_PROJECT\_NAME = ARCADE):
        * __common__ [ARCADE network-level common](http://github.com/ARCADE-UAV/common), aka __"level 2 common"__
            * __messages__: messages formats exchanged between different ARCADE subprojects/machines
            * __config__: common configuration files for all subprojects
            * __scripts__: bashrc (sourced from upper-level bashrc)
        * __airborne__ (MOBICOM_SUBPROJECT_NAME = airborne)
            * __common__: airborne common for local IPC, aka __"level 3 common"__
                * __messages__: scl message formats for local system IPC
                * __config__: system.yaml, services.yaml, parameters-*.yaml
                * __scripts__: bashrc (sourced from upper-level bashrc)
            * __components__: programs connected through __common__ above

The key motivation behind this structure is to support code-reuse and repository-tracked configuration file sharing
on different levels on the software architecture.
It is inspired by the Unix filesystem structure, which has fixed file locations for system, user and local
binaries, includes files and libraries.
Currently, we use 3 hierarchy levels (1-3) with one ever-repeating directory: "common". Within common,
"scripts" is present on all three levels. "messages" and "config" can be found on level 2 and 3.
The specific purpose of each level is explained as follows:

* __level 1__: MOBICOM inter-project code reuse and tools (SCL, svctrl)
* __level 2__: code and configurations for heterogeneous machines (UAV, Notebook, Smartphone)
* __level 3__: code and configurations for multiple processes (components) on a single machine, e.g. UAV

In order to support this level concept we use another Unix concept: mounting filesystems.
Within git (and github), mounting filesystems is similar to defining submodules.
Currently, the "airborne" software defines the following submodules:

* common (MOBICOM common)
* ARCADE/common (ARCADE common)
* ARCADE/airborne/components/interfaces/mavlink/pymavlink (third-party library)


## Getting started

1. Clone the repo to your machine, make sure to call it MOBICOM (the actual project name will be a directory inside the repo)
    
    1.a. If you want a read-only clone, type:

    git clone git://github.com/ARCADE-UAV/airborne.git MOBICOM

    1.b. If you want a developer clone, type:
     
    git clone git:@github.com:ARCADE-UAV/airborne.git MOBICOM

2. Change into the new directory and initialize the submodules (i.e. the common part of MOBICOM)

    cd MOBICOM
    ./gitsub_init.sh

3. Copy the content of ```example.bashrc``` into your local bashrc and edit it if required, reinstalize your environment

    ```bash
    cat example.bashrc >> ~/.bashrc
    nano ~/.bashrc
    bash
    ```

4. Now, create a new build directory and build the source

    mkdir build && cd build
    
    4.a. If you are just compiling it on a regular PC for testing:

    cmake .. && make

    4.b If you are compiling on the UAV's ARM Cortex A8 system-on-chip:

    cmake -DCMAKE_TOOLCHAIN_FILE=../toolchains/cortex_a8.cmake .. && make

