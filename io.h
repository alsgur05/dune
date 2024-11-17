/*
* raw(?) I/O
*/
#ifndef _IO_H_
#define _IO_H_

#define k_up_double (k_up + 10)
#define k_left_double (k_left + 10)
#define k_right_double (k_right + 10)
#define k_down_double (k_down + 10)
#define k_esc 27

#include "common.h"

KEY get_key(void);
void printc(POSITION pos, char ch, int color);
void gotoxy(POSITION pos);
void set_color(int color);

#endif

