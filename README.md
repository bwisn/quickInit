<img alt="quickInit" src="media/logo.png" width="300">

**Simple and fast init**

**Introduction**
------------
quickInit is a simple and fast init written in C (as for now). 

**Features**
--------
    * TTY spawning
    * Service spawning and killing

quickInit is in constant development. There will be more features.

**Compiling**
-------------
If you want to crosscompile, specify CC variable, otherwise skip it.

Example:

```
make clean
make CC="x86_64-linux-musl-gcc"
```

**Usage**
-----
### **TTY file**
Create /etc/quickinit/tty. Example file:
```
tty1::respawn:/sbin/getty 38400 tty1
tty2::askfirst:/sbin/getty 38400 tty2
tty3::askfirst:/sbin/getty 38400 tty3
```

The format of the file is:
```
tty[0, S0, USB0 or whatever]:[runlevel]:[respawn askfirst once]:[command]
```
 Runlevels are not supported as for now.

 ### **Services**
 Services are stored in:  
    ```/etc/quickinit/services/available```  - there are executable scripts or programs which must parse two parameters: start and stop. Other parameters are optional.  
    Example file: 
```bash
#!/bin/sh

case "$1" in
start)    
  ;;
stop) ;;

restart | reload)
  "$0" stop
  "$0" start
  ;;
*)
  echo $"Usage: $0 {start|stop|restart}"
  exit 1
  ;;
esac

exit $?
```

```/etc/quickinit/services/enabled```  - there are symlinks to the files from ```available``` directory.   
Listing of example directory: 
```lrwxrwxrwx 1 bartek users   21 2020-08-28  K001localnet -> ../available/localnet
lrwxrwxrwx 1 bartek users   21 2020-08-28  K002mountvfs -> ../available/mountvfs
lrwxrwxrwx 1 bartek users   21 2020-08-28  S001mountvfs -> ../available/mountvfs
lrwxrwxrwx 1 bartek users   21 2020-08-28  S002localnet -> ../available/localnet
```

Syntax of the filename is [S|K][priority][name] where:  
- S stands for start, K stands for kill
- Priority must be between 000-999 (three characters). If the number is lower, the service will be started/killed earlier
- name is the service name, same as the name of the file in the ```available``` folder

[![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://lbesson.mit-license.org/)
[![Open Source Love svg2](https://badges.frapsoft.com/os/v2/open-source.svg?v=103)](https://github.com/ellerbrock/open-source-badges/)
