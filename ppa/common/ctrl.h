#ifndef __EXT_CTRL_H__
#define __EXT_CTRL_H__

#include <pspctrl.h>
#include <psphprm.h>
#include <psptypes.h>

#define CTRL_FORWARD 0x10000000
#define CTRL_BACK 0x20000000
#define CTRL_PLAYPAUSE 0x40000000
#define CTRL_ANALOG 0x80000000

#ifdef __cplusplus
extern "C" {
#endif

void ctrl_init();
void ctrl_destroy();
void ctrl_analog(int * x, int * y);
u32 ctrl_read_cont();
u32 ctrl_read();
u32 ctrl_hprm();
void ctrl_enablehprm(int enable);

#ifdef __cplusplus
}
#endif

#endif
