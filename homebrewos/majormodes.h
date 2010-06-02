// majormodes.h
//
//	This file defines the major operating modes of the robot.  Each mode
//	is implemented in its own .c file, but they can all include this one
//	in order to switch from mode to mode (by assigning to the gNextMode
//	global variable).
//
//----------------------------------------------------------------------

extern int gNextMode;

enum {
	kIdleMode = 0,
	kExperimentalMode,
	kChargeMode,
	kPoseMode,
	kSerialSlaveMode,
	kClassicMode,
	kBinMode
};
