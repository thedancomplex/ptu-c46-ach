// for ach
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>
#include "ach.h"

// for serial
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#define 	PTU_C46_CHAN_REF		"ptu-c46-ref"     ///> Channel ref for PTU C-46
#define 	PTU_C46_CHAN_STATE		"ptu-c46-state"   ///> Channel state for PTU C-46

#define 	DELTA_PAN			0.05		///> change in pan (rad)
#define 	DELTA_TILT			0.05		///> change in tilt (rad)

// pan and tilt up and down keys (in byte form)
#define 	tup		119
#define		tdown		115
#define		pleft		97
#define		pright		100

struct ptu_ref {
	double pan;	///> commanded pan
	double tilt;	///> commanded tilt
	double roll;	///> commanded roll 
}ptu_ref_t;

struct ptu_state {
	double pan;	///> actuial pan
	double tilt;	///> actuial tilt
	double roll;	///> actuial roll 
}ptu_state_t;
