	.set noreorder

#include "pspstub.s"

	STUB_START "NetHostFS",0x40090000,0x00040005
	STUB_FUNC  0x1C88479A,netHostFSWaitDriverInited
	STUB_FUNC  0x67D062E6,netHostFSGetDriver
	STUB_FUNC  0xD4CAE579,netHostFSCleanup
	STUB_FUNC  0x2F75E93E,netHostGetKey
	STUB_END
