# ARCADE airborne software

## Overview

ARCADE airborne includes all software components which run on the ARCADE UAV system.
The repository consists of two subrepositories, common and ARCADE/common.
The general folder structure is shown below:

* __MOBICOM__ (MOBICOM_PATH defines top-level path)
    * __common__ (MOBICOM common)
        * __scl__: signaling and communication link
        * __svctrl__: service control utility
        * __scripts__: bashrc (sourced from user bashrc)
    * __ARCADE__ (MOBICOM_PROJECT_NAME = ARCADE):
        * __common__ (ARCADE common)
            * __messages__: messages formats exchanged between different ARCADE subprojects
            * __config__: common configuration files for all subprojects
            * __scripts__: bashrc (sourced from upper-level bashrc)
        * __airborne__ (MOBICOM_SUBPROJECT_NAME = airborne)
            * __common__: network-level protobuf message definitions
                * __messages__: scl message formats for local system IPC
                * __config__: system.yaml, services.yaml, parameters-*.yaml
                * __scripts__: bashrc (sourced from upper-level bashrc)
            * __components__: programs connected through __common__ above


## Getting started

1. Clone the repo to your machine, make sure to call it MOBICOM (the actual project name will be a directory inside the repo)

    ```bash
$ git clone git://github.com/ARCADE-UAV/airborne.git MOBICOM
```
2. Change into the new directory and initialize the submodules (i.e. the common part of MOBICOM)
   
    ```bash
$ cd ./MOBICOM
$ ./gitsub_init.sh
```
    
3. Copy the content of ```example.bashrc``` into your local bashrc and edit it if required, reinstalize your environment

    ```bash
$ cat example.bashrc >> ~/.bashrc
$ nano ~/.bashrc
$ bash
```

4. Now, create a new build directory and build the source

    ```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```