#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define SOH 	0x1
#define EOT	0x4
#define ACK	0x6
#define NAK	0x15

void xmodem_sendblk(int ufd, uint8_t *bin, int itr, int ct)
{
	int i;
	uint8_t csum = 0;
	uint8_t msg[133];

	msg[0] = SOH;
	msg[1] = (uint8_t)((itr / 128) % 255) + 1;
	msg[2] = (uint8_t)(255 - ((itr / 128) % 255) - 1);
	for (i = 0; i < ct; i++){
		msg[3 + i] = bin[itr + i];
		csum += msg[3 + i];
	}

	msg[131] = csum;

	/* Silabs can't keep up with 115200 data being sent constantly
	 * So this is slowing it down a bit to what the silabs can manage.
	 * My testing shows this takes ~ 2 minutes 10 seconds.  Sending
	 * 128 byte chunks is about 1 minute 32 seconds */
	for (i = 0; i < 132; i+=33)
	{
		write(ufd, &msg[i], 33);
		tcdrain(ufd);
	}
	tcdrain(ufd);
}

/* Just copy all of u-boot into memory */
uint8_t* load_uboot(char *path,  ssize_t *binsz)
{
	uint8_t *bin = NULL;
	FILE *f = fopen(path, "r");
	if(fseek(f, 0L, SEEK_END) == 0) {
		ssize_t rd;
		*binsz = ftell(f);
  		rewind (f);
		if(*binsz == -1) {
			perror("Couldn't read filesize\n");
			goto fail;
		}

		bin = malloc(sizeof(uint8_t) * (*binsz) + 1);
		if(bin == NULL)
		{
			perror("bin");
			goto fail;
		}
		rd = fread(bin, sizeof(uint8_t), *binsz, f);
		if(ferror(f) != 0 || rd != (*binsz)) {
			fprintf(stderr, "Failed to read %s\n", path);
			goto fail;
		}
	} else {
		goto exit;
	}

fail:
	fclose(f);

exit:
	return bin;
}

uint8_t xmodem_get_response(int ufd, int sec_tout)
{
	struct timeval tval;
	fd_set rfds;
	tval.tv_sec = sec_tout;
	tval.tv_usec = 0;
	FD_ZERO(&rfds);
	FD_SET(ufd, &rfds);

	select(ufd + 1, &rfds, NULL, NULL, &tval);
	if(FD_ISSET(ufd, &rfds)) {
		uint8_t rbuf;
		read(ufd, &rbuf, 1);
		return rbuf;
	}
	/* 0 is not a valid response from xmodem, so
	 * in this case it will just mean a timeout */
	return 0;
}

void send_uartboot(int ufd)
{
	const uint8_t mvstop[] = {0xbb, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
	write(ufd, mvstop, 8);
	tcdrain(ufd);
}

int main(int argc, char **argv)
{
	int i;
	int resp;
	int ufd;
	ssize_t binsz = 0;
	struct termios newtio;
	uint8_t *bin = 0;
	uint8_t tmp;

	if(argc != 3) {
		printf("Usage: %s /dev/ttyUSB# /path/to/u-boot \n", argv[0]);
		return 1;
	}

	ufd = open(argv[1], O_RDWR | O_SYNC); 
	if (ufd <0) {
		perror(argv[1]); 
		return -1;
	}

	bin = load_uboot(argv[2], &binsz);
	if(bin == NULL){
		printf("Failed to read u-boot");
		return 1;
	}
	if(bin == NULL){
		printf("OOps\n");
		return 1;
	}

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = CS8 | CLOCAL | CREAD | B115200;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME]    = 0;
	newtio.c_cc[VMIN]     = 1; 
	tcflush(ufd, TCIFLUSH);
	tcsetattr(ufd, TCSANOW, &newtio);

	printf("Sending CPU uart boot mode command until we get a response\n");
	do {
		send_uartboot(ufd);
		printf(".");
		fflush(stdout);
	} while(xmodem_get_response(ufd, 1) != NAK);
	printf("\rSending u-boot over xmodem\n");

	for (i = 0; i < binsz; i+=128)
	{
		if(i + 128 > binsz) {
			xmodem_sendblk(ufd, bin, i, binsz % 128);
			break;
		} else {
			xmodem_sendblk(ufd, bin, i, 128);
		}
		resp = xmodem_get_response(ufd, 10);

		fflush(stdout);
		switch(resp) {
		case ACK:
			printf("Sent %d/%ld (%0.2f%%)\r", i, binsz, i*100 / (float)binsz);
			continue;
		case NAK:
			i-=128;
			break;
		default:
			printf("Got resp 0x%X\n", resp);
			break;
		}
	}
	puts("\n");

	/* Send end of transmit */
	tmp = EOT;
	write(ufd, &tmp, 1);
	resp = xmodem_get_response(ufd, 10);
	printf("Executed U-boot (resp %d)\n", resp);

	free(bin);

	return 0;
}
