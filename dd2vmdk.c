/*
 * ##########################################################################################
 # Created on:  9 August 2010
 # File(s):     dd2vmdk
 # Version:	    0.1 beta
 #
 # @author: labgeek@gmail.com
 #
 # This program is free software; you can redistribute it and/or
 # modify it under the terms of the GNU General Public License
 # as published by the Free Software Foundation, using version 2
 # of the License.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program; if not, write to the Free Software
 # Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 # 02110-1301, USA.
 #
 ##########################################################################################
 */

/* SVN FILE: $Id: dd2vmdk.c 5 2010-08-14 17:40:43Z labgeek13 $*/
/*
 * Project Name : dd2vmdk
 * $Author: labgeek13 $
 * $Date: 2010-08-14 13:40:43 -0400 (Sat, 14 Aug 2010) $
 * $Revision: 5 $
 * $LastChangedBy: labgeek13 $
 * $URL: https://dd2vmdk.svn.sourceforge.net/svnroot/dd2vmdk/trunk/dd2vmdk_src/dd2vmdk.c $
 */

//TODO fix issue with first partition entry being only one we look at and assume active (0x80)

/* primary header */
#include "dd2vmdk.h"

int main(int argc, char** argv) {
	char *cmd = argv[0];
	char partition_name[15] = "partition";
	int j = 0;
	int mbrctr = 0;
	char outFileExt[] = ".vmdk";
	char outFilename[100];
	char *image = NULL;
	char *fileoutput = NULL;
	char *vmdkoutput = NULL;
	char *hexvalue = NULL;
	char *analysis_output = NULL;
	char opt = 0;
	float gb = 0.0;
	long long fSize = 0;
	long blocksize = 0;
	long totalsectors = 0;
	int i, bytecount, cylinders, sectorspertrack, numheads, endcylinder;
	unsigned char *mbr_buffer;
	int pecounter = 0;
	char active[] = "80";
	int isBootable = 0;
	int bootablePartition = 0;
	char pbuf[5];
	//char activePartition = "80";
	/*
	 * Series of arrays that hold the
	 * partition entry data for all four
	 * primary partitions
	 */

	int partition1[16] = { };
	int partition2[16] = { };
	int partition3[16] = { };
	int partition4[16] = { };

	/*
	 * flag switch for users to pick
	 * what options they want.  Right
	 * now, it is limited to just
	 * source and output flags..
	 */
	while (opt != -1) {
		opt = getopt(argc, argv, "hi:v:");
		switch (opt) {
		case '?':
			usage(cmd);
			break;
		case 'i':
			image = optarg;
			break;
		case 'v':
			vmdkoutput = optarg;
			break;
		case 'h':
			usage(cmd);
			return 0;
		case -1:
			break;
		default:
			usage(cmd);
			return (1);
		}
	}

	if ((image) && (vmdkoutput)) {

		FILE *fp = fopen(image, "rb");
		if (!fp) {
			printf("%s could not be opened - here\n", image);
			exit(0);
		}

		mbr_buffer = malloc(MBR);
		if (mbr_buffer == NULL) {
			printf("Error, malloc failed, wtf\n");
			exit(EXIT_FAILURE);
		}

		/*
		 * Total sectors is found by dividing the total
		 * number of bytes in the file by the sector size
		 * which in this case is 512.  It will be interesting
		 * to see what happens when we move to 2k or 4k
		 * sector sizes
		 */
		//printf("sizeof(off_t) = %d.\n", sizeof(off_t));

		off_t file_size = getfilesize(fp);
		fSize = file_size;

		if(fSize < 512)
		{
			fprintf(stderr, "File size is less than 512 bytes in size\n");
			fprintf(stderr, "The image must be at least 512 bytes in size\n");
			usage(cmd);
		}

		/* convert to GB */
		gb = (fSize / 1024) / 1024 / 1024;
		totalsectors = file_size / MBR;
		(fseek(fp, 0, SEEK_SET));

		//TODO bytecount error checking needed
		bytecount = fread(mbr_buffer, 1, MBR, fp);

		/*
		 * Partition table:  64 byte entries from byte offsets 446 - 509
		 * Partition 1:  446 - 461
		 * Partition 2:  462 - 477
		 * Partition 3:  478 - 493
		 * Partition 4:  494 - 509
		 *
		 * Disk Signature marker is from byte offset 510 and 511 (55AA for windows)
		 */

		/*
		 * bootcodesize is 0-445 or 446 bytes.  In the situation below
		 * we increment by 16 in order to get each of the 4 primary
		 * partition data entries.
		 */
		for (i = BOOTCODESIZE; i < BOOTCODESIZE + byte_jump_size; i++) {
			//printf("[%d]: %02X\n", i, mbr_buffer[i] & 0xff);
			partition1[pecounter] = mbr_buffer[i] & 0xff;
			partition2[pecounter] = mbr_buffer[i + 16] & 0xff;
			partition3[pecounter] = mbr_buffer[i + 32] & 0xff;
			partition4[pecounter] = mbr_buffer[i + 48] & 0xff;
			pecounter++;
		}
		/* stdout printing to the command console */
		printf("\nAuthor:   %s\n", AUTHOR);
		printf("Program:  %s\n", PROGRAM_NAME);
		printf("Website:  %s\n", WEBSITE);
		printf("Version:  %s\n", VERSION);

		printf("\nStarting the MBR analysis...\n");
		printf("Finding which partition is bootable\n");
		bootablePartition = getBootablePartition(mbr_buffer);
		if (bootablePartition < 0) {
			fprintf(stderr, "No active parition found - something is wrong\n");
			usage(cmd);
		}
		sprintf(pbuf, "%d", bootablePartition);
		strcat(partition_name, pbuf);

		if (bootablePartition == 1) {
			/*
			 * Gets the total number of cylinders
			 */
			printf("Getting cylinders value from %s...\n", partition_name);
			cylinders = getcylinders(partition1);
			printf("Getting ending cylinders value from %s...\n",
					partition_name);
			endcylinder = getendcylinder(partition1);

			/*
			 * gets the number of sectors per track
			 */
			printf("Getting sectors per track value from %s...\n",
					partition_name);
			sectorspertrack = getsectorspertrack(endcylinder);
			printf("Getting number of heads value from %s...\n", partition_name);
			numheads = getheadspertrack(partition1);
		} else if (bootablePartition == 2) {
			/*
			 * Gets the total number of cylinders
			 */
			printf("Getting cylinders value from %s...\n", partition_name);
			cylinders = getcylinders(partition2);
			printf("Getting ending cylinders value from %s...\n",
					partition_name);
			endcylinder = getendcylinder(partition2);

			/*
			 * gets the number of sectors per track
			 */
			printf("Getting sectors per track value from %s...\n",
					partition_name);
			sectorspertrack = getsectorspertrack(endcylinder);
			printf("Getting number of heads value from %s...\n", partition_name);
			numheads = getheadspertrack(partition2);
		} else if (bootablePartition == 3) {
			/*
			 * Gets the total number of cylinders
			 */
			printf("Getting cylinders value from %s...\n", partition_name);
			cylinders = getcylinders(partition3);
			printf("Getting ending cylinders value from %s...\n",
					partition_name);
			endcylinder = getendcylinder(partition3);

			/*
			 * gets the number of sectors per track
			 */
			printf("Getting sectors per track value from %s...\n",
					partition_name);
			sectorspertrack = getsectorspertrack(endcylinder);
			printf("Getting number of heads value from %s...\n", partition_name);
			numheads = getheadspertrack(partition3);
		} else if (bootablePartition == 4) {
			/*
			 * Gets the total number of cylinders
			 */
			printf("Getting cylinders value from %s...\n", partition_name);
			cylinders = getcylinders(partition4);
			printf("Getting ending cylinders value from %s...\n",
					partition_name);
			endcylinder = getendcylinder(partition4);

			/*
			 * gets the number of sectors per track
			 */
			printf("Getting sectors per track value from %s...\n",
					partition_name);
			sectorspertrack = getsectorspertrack(endcylinder);
			printf("Getting number of heads value from %s...\n", partition_name);
			numheads = getheadspertrack(partition4);
		} else {
			fprintf(stderr, "Bootable Partition = %d\n", bootablePartition);
			fprintf(stderr, "No bootable partition found\n");
		}

		printf("\nImage Geometry specifications:\n");
		printf("================================\n");

		printf("1.  Image location:  %s\n", image);
		printf("1.  VMDK destination:  %s\n", vmdkoutput);
		printf("2.  Number of sectors = %ld\n", totalsectors);
		printf("3.  Number of cylinders:  %d\n", cylinders);
		printf("4.  Number of heads per track:  %d\n", numheads);
		printf("5.  Number of sectors per track: %d\n", sectorspertrack);
		printf("6.  File size in bytes: %lld (%.2f GB)\n", fSize, gb);
		printf("7.  Size of each sector: %d\n", MBR);
		printf("8.  Bootable partition:  Partition %d\n", bootablePartition);
		end_of_sector_marker(mbr_buffer);

		/*
		 * prints out the vmdk file based on the -v flag
		 */
		printvmdkfile(image, vmdkoutput, totalsectors, cylinders, numheads,
				sectorspertrack);

		free(mbr_buffer);
		fclose(fp);
		return 0;

	} else {
		usage(cmd);
		return 0;
	}
}

/* Passes in pointer to master boot record code,
 * parses through starting at offset 446 to 510,
 * converts decimal value to hex, then trys to
 * find the 0x80 code for active partition.
 * Returns the partition index number that
 * is active.
 */

int getBootablePartition(char *buffer) {
	int j = 0;
	char *hexvalue = NULL;
	int mbrctr = 0;

	for (j = BOOTCODESIZE; j < mbr_signature; j += byte_jump_size) {
		mbrctr++;
		hexvalue = dec2hex(buffer[j] & 0xff);
		if (strcmp(hexvalue, "80") == 0) {

			return mbrctr;
		}
	}
	return -1;

}

/* simple usage function */
void usage(char *prog_name) {
	fprintf(stderr, "\nProgram: dd2vmdk v0.1.1\n");
	fprintf(stderr, "Author:  JD Durick <labgeek@gmail.com>\n");
	fprintf(stderr,
			"Description:  dd2vmdk is a command line tool to convert dd images into "
				"vmdk files.\n");
	fprintf(
			stderr,
			"Usage: %s -i <path to dd image> -v <output path of vmdk>...\n"
				"\nOPTIONS:\n"
				"   -i image file name that you will be analyzing\n"
				"   -v VMDK metadata file that you will be creating for your dd2vmdk conversion\n",
			prog_name);

}

/* gets the cylinders value */

int getcylinders(int activepartition[]) {

	int begincylinder, endcyclinder, cylinders;
	/*
	 * starting cylinder is a 10 bit value,
	 * taking 8 bits from partition entry
	 *  and 2 bits from partition entry 3
	 */
	begincylinder = ((activepartition[3] << 8) | activepartition[2]);
	endcyclinder = ((activepartition[7] << 8) | activepartition[6]);

	/*
	 * code from PartitionEntry.java
	 * in the Liveview source code
	 */
	cylinders = ((endcyclinder & 65280) >> 8);

	// if the 6th bit of the 16 bit
	// structure is not zero
	if ((endcyclinder & 64) != 0) {
		cylinders += 256;
	}
	// set bit 8 of the 10 bit cylinder value

	if ((endcyclinder & 128) != 0) { // if the 7th bit of the 16 bit
		// structure is not zero
		cylinders += MBR; // set bit 9 of the 10 bit cylinder value
	}

	return cylinders;

}

/* gets the end cyclinder value */
int getendcylinder(int partition[]) {
	int endcyl = 0;
	endcyl = ((partition[7] << 8) | partition[6]);
	return endcyl;
}

/* gets the sectors per track value */
int getsectorspertrack(int endcyl) {
	int sectors_per_track = 0;
	/* the ending sector is 6 bits which would be 00111111
	 * use the bitwise operator AND and compares in binary form
	 * in the case of 65535,  = 11111111 11111111
	 * Comparing the two would give us still 63 decimal
	 */
	sectors_per_track = endcyl & 63;
	return sectors_per_track;

}

/* gets the heads per track value */
int getheadspertrack(int activepart[]) {

	int nheads = 0;
	nheads = activepart[1] + activepart[5];
	return nheads;

}

/*
 * Compliments to zapotek's template for creating a
 * baseline VMDK metadata file
 *
 */

void printvmdkfile(char *src, char *output, long tsectors, int cyls, int heads,
		int sectpertrack) {

	FILE *outputfile;
	outputfile = fopen(output, "w");
	fprintf(outputfile, "\nversion=1\n");
	fprintf(outputfile, "encoding=\"UTF-8\"\n");
	fprintf(outputfile, "CID=fffffffe\n");
	fprintf(outputfile, "parentCID=ffffffff\n");
	fprintf(outputfile, "isNativeSnapshot=\"no\"\n");
	fprintf(outputfile, "createType=\"monolithicFlat\"\n\n");
	fprintf(outputfile, "RW %ld FLAT \"%s\" 0\n\n", tsectors, src);
	fprintf(outputfile, "ddb.virtualHWVersion = \"7\"\n");
	fprintf(outputfile,
			"ddb.longContentID = \"29075898903f9855853610dffffffffe\"\n");
	fprintf(outputfile,
			"ddb.uuid = \"60 00 C2 91 8e 73 27 62-43 58 3b f8 05 ae 2e a0\"\n");
	fprintf(outputfile, "ddb.geometry.cylinders = \"%d\"\n", cyls);
	fprintf(outputfile, "ddb.geometry.heads = \"%d\"\n", heads);
	fprintf(outputfile, "ddb.geometry.sectors = \"%d\"\n", sectpertrack);
	fprintf(outputfile, "ddb.adapterType = \"ide\"\n");
	fclose(outputfile);

}

/* gets the end of sector signature within the master boot record */
void end_of_sector_marker(char *mastboot) {
	/*
	 * gets byte offset 510 and 511 which hold
	 the two byte signature.  Without the end-of-sector
	 marker, this sector would not be interpreted as a
	 valid MBR.*/

	printf("\nTwo byte signature word:  %02X%02X (End of MBR)\n\n",
			mastboot[510] & 0xff, mastboot[511] & 0xff);
	return;

}

/* converts decimal value to hex value */
char* dec2hex(int decimal) {
	static char hx[256];
	sprintf(hx, "%02x", decimal);
	//printf("Hex value of Decimal value %d  is %s\n",decimal,hx);
	return hx;

}

/* gets the file size of the raw image opened */
off_t getfilesize(FILE *f) {
	struct stat xstat;
	fstat(fileno(f), &xstat);
	return xstat.st_size;
}

