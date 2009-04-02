// idle_mode.c
//
//	Implements the mode where the robot is idle (as when first turned on).
//	In this mode, it should do very little, and most especially must not
//	activate the servos.

#include "majormodes.h"
#include "ir.h"
#include "uart.h"
#include "comm.h"
#include "rprintf.h"

extern void basic_run(int);


static void exitToExperimental(void) {
	// Go to basic pose, and then switch to experimental mode
	BasicPose();
	gNextMode = kExperimentalMode;
}

static void exitToSerialSlave(void) {
	// Switch to serial-slave mode
	gNextMode = kSerialSlaveMode;
}
	

static void handle_serial(int cmd) {
	if ('p' == cmd || 'P' == cmd) exitToExperimental();
	if ('s' == cmd || 'S' == cmd) exitToSerialSlave();
	if ('r' == cmd || 'R' == cmd) basic_run(0);
	if ('?' == cmd) {
		rprintf("\nIdle mode\n");
	}
}

static void handle_ir(int cmd) {
	if (0x07 == cmd) exitToExperimental(); // (square button)
}

void idle_mainloop(void) {

	int cmd;

	while (kIdleMode == gNextMode) {

		cmd = uartGetByte();
		if (cmd >= 0) handle_serial(cmd);

		cmd = irGetByte();
		if (cmd >= 0) handle_ir(cmd);
		
		// Here we should also check hardware buttons -- perhaps one
		// to enter pose mode, and both (held for a while) to enter
		// battery-charging mode.
	}
}
