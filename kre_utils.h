/*
 *  GPL vRelease licence
 *  
 *  kREproxy project (C) Marcin Tomasik 2009
 *
 *  <marcin.tomasik.priv[onserver]gmail.com>
 *  
 *  http://kreproxy.sourceforge.net/
 *
 */


/**
 *	kREproxy utilities
 *	@file kre_utils.h
 */
#ifndef KRE_UTILS_H
#define KRE_UTILS_H

#include <linux/kernel.h>
#include <linux/module.h>
#include "kre_debug.h"

/**	Power x to y. 
 *	@param x	 
 *	@param y
 */
extern int kre_util_pow( int x, int y);

/**	Convers char to integer.
 * 	@param c	Character to convers.
 * 	@return 	Integer value of digit from c.
 */
extern int kre_util_chartoint( char *c );

/**	Asci to int 
 * 	@number: table with number
 *	@s: size of number, can be 0 then strlen will be used
 */
extern int kre_util_atoi( char *number, size_t s );


/**	String reverse
 */
extern void kre_util_strreverse( char *rev, const char *str, size_t s );

#endif
