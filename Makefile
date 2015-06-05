all:  dd2vmdk

bytelocator: Makefile dd2vmdk.c
	gcc -g -Wall -o dd2vmdk dd2vmdk.c

clean:
	rm dd2vmdk