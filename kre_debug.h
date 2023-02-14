/*
 *  GPL v3.0 license
 *  
 *  kREproxy project (C) Marcin Tomasik 2009
 *
 *  <marcin[at]tomasik.pl>
 *  
 *  http://kreproxy.sourceforge.net/
 *
 */

/**
 * 	kREproxy debuging management.
 *	@file kre_debug.h
 */
#ifndef KRE_DEBUG_H
#define KRE_DEBUG_H

#	undef	KRE_DBG
#	undef	KRE_DBG_INF

#define KRE_DBG( format, args... ) \
        	printk( KERN_DEBUG "kre-error: " format, ## args )

#	ifdef KRE_DEBUG_MODE
    	/* debug mode for kreproxy is turn on */

#		define KRE_DBG_INF( format, args... ) \
			printk( KERN_INFO "kre-info: " format, ## args )

#	else
    	/* debug mode is turn off */
#		define KRE_DBG_INF( format, args...)
			/* nothing to do */
#	endif


#endif
