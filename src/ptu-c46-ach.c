/* -*-	indent-tabs-mode:t; tab-width: 8; c-basic-offset: 8  -*- */
/*
Copyright (c) 2012, Daniel M. Lofaro
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its contributors may
      be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <time.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <string.h>
#include <stdio.h>

// for timer
#include <time.h>
#include <sched.h>
#include <sys/io.h>
#include <unistd.h>

// for RT
#include <stdlib.h>
#include <sys/mman.h>

// For PTU
#include "ptu-c46-ach.h"
#include <curses.h>
//#include <conio.h>

// for hubo
// #include <hubo.h>

// for serial
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

/* At time of writing, these constants are not defined in the headers */
#ifndef PF_CAN
#define PF_CAN 29
#endif

#ifndef AF_CAN
#define AF_CAN PF_CAN
#endif

/* ... */

/* Somewhere in your app */

// Priority
#define MY_PRIORITY (49)/* we use 49 as the PRREMPT_RT use 50
			    as the priority of kernel tasklets
			    and interrupt handler by default */

#define MAX_SAFE_STACK (1024*1024) /* The maximum stack size which is
				   guaranteed safe to access without
				   faulting */


// Timing info
#define NSEC_PER_SEC    1000000000


struct timeb {
	time_t   time;
	unsigned short millitm;
	short    timezone;
	short    dstflag;
};



/* functions */
void stack_prefault(void);
static inline void tsnorm(struct timespec *ts);
void getMotorPosFrame(int motor, struct can_frame *frame);
//void huboLoop(struct hubo_param *h);
void huboLoop();
int ftime(struct timeb *tp);










// ach message type
//typedef struct hubo h[1];

// ach channels
ach_channel_t chan_ref;      // hubo-ach
ach_channel_t chan_state;    // hubo-ach-state

int debug = 0;
int hubo_debug = 0;










char getch2() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}









int getTicks(double p) {
	double r = 185.1428;
	double n = r / (60.0 * 60.0);
	return (int)floor(p / M_PI * 180.0 / n );
}

//void huboLoop(struct hubo_param *H_param) {
void mainLoop( int fd ) {
	/* Open Serial */
	int n;
    	char c;
    	int bytes;

    	char buffer[10];
    	char *bufptr;
    	int nbytes;
    	int tries;
    	int x;
    	struct termios termios_p;


    	if(fd == -1)
    	{
        	perror("open_port: Unable to open:");
    	}
    	else
    	{
        //	fcntl(fd, F_SETFL, 0);
//        	printf("Port Open\n");
    	}

    	tcgetattr(fd, &termios_p);

	cfsetispeed(&termios_p, B9600);
	cfsetospeed(&termios_p, B9600);



           termios_p.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                           | INLCR | IGNCR | ICRNL | IXON);
           termios_p.c_oflag &= ~OPOST;
           termios_p.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
           termios_p.c_cflag &= ~(CSIZE | PARENB);
           termios_p.c_cflag |= CS8;


/*
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	//options.c_cflag &= CS8;
	options.c_cflag &= ~( ICANON | ECHO | ECHOE |ISIG );
	options.c_iflag &= ~(IXON | IXOFF | IXANY );
	options.c_oflag &= ~OPOST;
*/
	tcsetattr(fd, TCSANOW, &termios_p);

	//write(fd, "LD\r\n",4); // turn off lim
	write(fd, "LU\r\n",4);
	write(fd, "PX2000\r\n",8);
	write(fd, "PN-2000\r\n",9);
	write(fd, "TX200\r\n",7);
	write(fd, "TN-1500\r\n",9);
	write(fd, "LU\r\n",4);
	write(fd, "LD\r\n",4); // turn off lim
	sleep(1);



	// get initial values for hubo



	struct ptu_ref H_ref;
	struct ptu_state H_state;
	memset( &H_ref,   0, sizeof(H_ref));
	memset( &H_state, 0, sizeof(H_state));

	size_t fs;
	//int r = ach_get( &chan_hubo_ref, &H, sizeof(H), &fs, NULL, ACH_O_LAST );
	//assert( sizeof(H) == fs );
	int r = ach_get( &chan_ref, &H_ref, sizeof(H_ref), &fs, NULL, ACH_O_COPY );
	if(ACH_OK != r) {
		if(hubo_debug) {
			printf("Ref ini r = %s\n",ach_result_to_string(r));}
		}
	else{   assert( sizeof(H_ref) == fs ); }
/*
	r = ach_get( &chan_state, &H_state, sizeof(H_state), &fs, NULL, ACH_O_COPY );
	if(ACH_OK != r) {
		if(hubo_debug) {
			printf("State ini r = %s\n",ach_result_to_string(r));}
		}
	else{
		assert( sizeof(H_state) == fs );
	 }

*/
	int L = 15;
	while(1) {

		/* Get latest ACH message */
		r = ach_get( &chan_ref, &H_ref, sizeof(H_ref), &fs, NULL, ACH_O_WAIT );
		if(ACH_OK != r) {
			if(hubo_debug) {
				printf("Ref r = %s\n",ach_result_to_string(r));}
			}
		else{   assert( sizeof(H_ref) == fs ); }
/*
		r = ach_get( &chan_state, &H_state, sizeof(H_state), &fs, NULL, ACH_O_COPY );
		if(ACH_OK != r) {
			if(hubo_debug) {
				printf("State r = %s\n",ach_result_to_string(r));}
			}
		else{   assert( sizeof(H_state) == fs ); }
*/
// ------------------------------------------------------------------------------
// ---------------[ DO NOT EDIT AVBOE THIS LINE]---------------------------------
// ------------------------------------------------------------------------------

		int pan = getTicks(H_ref.pan);
		int tilt = getTicks(H_ref.tilt);
		
		char strPan[L];
		char strTilt[L];

		memset( &strPan,   0, sizeof(strPan));
		memset( &strTilt,   0, sizeof(strTilt));

		sprintf(strPan, "PP%d\r\n",pan);
		sprintf(strTilt, "TP%d\r\n",tilt);
		strPan[L-1] = 13;
		strPan[L-2] = 10;

		strTilt[L-1] = 13;
		strTilt[L-2] = 10;

		write(fd, strPan,L);
		write(fd, strTilt,L);
//		printf(strTilt);
//		printf(strPan);

// ------------------------------------------------------------------------------
// ---------------[ DO NOT EDIT BELOW THIS LINE]---------------------------------
// ------------------------------------------------------------------------------
//		ach_put( &chan_hubo_ref, &H_ref, sizeof(H_ref));
	}


}


int doRead(int fd) {
	int x;
	int c;
	char buffer[10];
        char *bufptr;
        int nbytes;
	int bytes;

    fcntl(fd, F_SETFL, FNDELAY);
     bytes = read(fd, bufptr, sizeof(buffer));
    printf("number of bytes read is %d\n", bytes);
    perror ("read error:");

    for (x = 0; x < 10 ; x++)
    {
        c = buffer[x];
        printf("%d  ",c);
    }
    close(fd);

    //puts(buffer[0]);
    printf("\nshould of put something out \n");

    return (0);
}
int main(int argc, char **argv) {

	/* open ach channel */
	//int r = ach_open(&chan_hubo_ref, HUBO_CHAN_REF_NAME , NULL);
	int r = ach_open(&chan_ref, PTU_C46_CHAN_REF , NULL);
	assert( ACH_OK == r );

	r = ach_open(&chan_state, PTU_C46_CHAN_STATE , NULL);
	assert( ACH_OK == r );

	struct ptu_ref H_ref;
	struct ptu_state H_state;
	memset( &H_ref,   0, sizeof(H_ref));
	memset( &H_state, 0, sizeof(H_state));

	size_t fs;
	//int r = ach_get( &chan_hubo_ref, &H, sizeof(H), &fs, NULL, ACH_O_LAST );
	//assert( sizeof(H) == fs );
	r = ach_get( &chan_ref, &H_ref, sizeof(H_ref), &fs, NULL, ACH_O_LAST );
	if(ACH_OK != r) {
		if(hubo_debug) {
			printf("Ref ini r = %s\n",ach_result_to_string(r));}
		}
	else{   assert( sizeof(H_ref) == fs ); }


	/* check arguements */
	int i = 1;
	while(argc > i) {
		if(strcmp(argv[i], "-t") == 0) {
			H_ref.tilt = strtod(argv[i+1], NULL);
		//	printf("set to %f\n", H_ref.tilt);
			ach_put( &chan_ref, &H_ref, sizeof(H_ref));		
			return 0;
		}
		if(strcmp(argv[i], "-p") == 0) {
			H_ref.pan = strtod(argv[i+1], NULL);
			ach_put( &chan_ref, &H_ref, sizeof(H_ref));		
			return 0;
		}

		
		if(strcmp(argv[i], "-k") == 0) {
  		char opt;
		printf("Keyboard input mode Use W-S-D-F keys:\n");
		printf("Pan delta  = %f (rad) \n",DELTA_PAN);
		printf("Tilt delta = %f (rad) \n",DELTA_TILT);
		while(1) {
//			printf("Please enter either 'a' or 'b'");

			/* wait for keyboard */
			opt = getch2();

			r = ach_get( &chan_ref, &H_ref, sizeof(H_ref), &fs, NULL, ACH_O_LAST );
			if(ACH_OK != r) {
				if(hubo_debug) {
					printf("Ref ini r = %s\n",ach_result_to_string(r));}
				}
			else{   assert( sizeof(H_ref) == fs ); }


			if( tup == opt ) { 
				H_ref.tilt = H_ref.tilt+DELTA_TILT; 
				ach_put( &chan_ref, &H_ref, sizeof(H_ref));	
			}	

			if( tdown == opt ) { 
				H_ref.tilt = H_ref.tilt - DELTA_TILT; 
				ach_put( &chan_ref, &H_ref, sizeof(H_ref));	
			}	


			if( pleft == opt ) { 
				H_ref.pan = H_ref.pan+DELTA_PAN; 
				ach_put( &chan_ref, &H_ref, sizeof(H_ref));	
			}	
			
			if( pright == opt ) { 
				H_ref.pan = H_ref.pan-DELTA_PAN; 
				ach_put( &chan_ref, &H_ref, sizeof(H_ref));	
			}	



//			if( 119==opt )
//I				printf("here %d\n",opt);

//while((opt=getchar())!='\n' && opt !=EOF);

		}

		

		}
		i++;
	}





	int fd = 0;

	if(argc > 1) {
		//printf("%s\n",argv[1]);
		fd = open(argv[1], O_RDWR | O_NOCTTY | O_NDELAY);
	}
	else {
		printf("exit\n");
		return 0;
	}


        
	mainLoop(fd);

	pause();
	return 0;

}
