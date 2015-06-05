# dd2vmdk
dd2vmdk is a *nix-based program that allows you to mount raw disk images (created by dd, dcfldd, dc3dd, ftk imager, etc) by taking the  raw image, analyzing the master boot record (physical sector 0), and getting specific information that is need to create a vmdk file.
