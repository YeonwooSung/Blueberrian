/************************************************************
 * The standard argument header file for the Blueberrian.   *
 * Author: Yeonwoo Sung                                     *
 ************************************************************/

#ifndef BLUEBERRIAN_STDARG_H
#define BLUEBERRIAN_STDARG_H

typedef char *va_list;

#  define va_start(ap, p)	(ap = (char *) (&(p)+1))
#  define va_arg(ap, type)	((type *) (ap += sizeof(type)))[-1]
#  define va_end(ap)		((void)0)

#endif //BLUEBERRIAN_STDARG_H
