// Upper processing routine of NOR, NAND flash.
//
// modified by Yeonwoo Sung on 2018.12.28

#ifndef _FLASH_HEADER_
#define _FLASH_HEADER_

#include <flash_29lvx.h>
#include <nand.h>

extern void CopyImage(void);
extern int ErasePartiotion(int argc, char **argv);

#endif // _FLASH_HEADER_
