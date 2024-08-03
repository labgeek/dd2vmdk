
                              dd2vmdk v0.1
                             labgeek@gmail.com


               Licensed under the GNU General Public License v2

General
---------- 
dd2vmdk is a *nix-based program that allows you to mount raw disk images (created by dd, dcfldd, dc3dd, ftk imager, etc) by taking the
 raw image, analyzing the master boot record (physical sector 0), and getting specific information that is need to create a vmdk file.
 Version 0.1 has the ability to extract data from the master boot record of any raw image, extracted information such as heads
 cylinders, sectors per track, etc. and create a working vmdk file (monolithic flat disk).  
 
 There are two other tools, Liveview (https://sourceforge.net/projects/liveview/) and dd2vmdk (written by zapotek) which is a front end driver for the 
 primary java files used to do the MBR and partition entry analysis.  Don't know if I will add any other functionality
 to this tool but thought it might help someone.
 
 Usage:
 ------
 root@redbox:/data/projects/C/dd2vmdk# ./dd2vmdk -h

Program: dd2vmdk v0.1
Author:  labgeek@gmail.com

Description:  dd2vmdk is a command line tool to convert dd images into vmdk files.
Usage: ./dd2vmdk -i <path to dd image> -v <output path of vmdk>...

OPTIONS:
   -i image file name that you will be analyzing
   -v VMDK metadata file that you will be creating for your dd2vmdk conversion
   
Example Usage:
--------------

root@redbox:/data/projects/C/dd2vmdk-0.1.1# ./dd2vmdk -i /data/images/vmpersonal.dd -v output.vmdk|more

Author:   labgeek@gmail.com
Program:  dd2vmdk
Website:  http://vmforensics.org
Version:  0.1.1

Starting the MBR analysis...
Finding which partition is bootable
Getting cylinders value from partition1...
Getting ending cylinders value from partition1...
Getting sectors per track value from partition1...
Getting number of heads value from partition1...

Image Geometry specifications:
================================
1.  Image location:  /data/images/vmpersonal.dd
1.  VMDK destination:  output.vmdk
2.  Number of sectors = 62914560
3.  Number of cylinders:  1023
4.  Number of heads per track:  255
5.  Number of sectors per track: 63
6.  File size in bytes: 32212254720 (30.00 GB)
7.  Size of each sector: 512
8.  Bootable partition:  Partition 1

Two byte signature word:  55AA (End of MBR)
 
Compile:
--------
1.  tar zxvf dd2vmdk-0.1.tar.gz
2.  cd dd2vmdk
3.  make
4.  run -> ./dd2vmdk -h

Acknowledgments
----------------
Tim Vidas and Brian Kaplan
Zapotek (http://trainofthought.segfault.gr/)

Limitations:
-----------
- assumption that the active partition is the first one (0x80)

