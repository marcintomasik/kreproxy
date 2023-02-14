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
 * 	kREproxy list interface
 *	@file kre_list.h
 */
#ifndef KRE_LIST_H
#define KRE_LIST_H

#include <linux/list.h>
#include "kre_debug.h"

/**	kREproxy list head.
 */
#define kre_list_head \
		struct list_head


/**	kREproxy list initialization.
 */
extern void kre_list_init( kre_list_head * );


/**	Add new list entry tail.
 * 	@param new	New list head entry to add.
 * 	@param list	Destination list.
 */
extern void kre_list_add_entry_tail( kre_list_head *new, kre_list_head *list );

/**	Delete entry list_head from list.
 *	@param list_head	The entry to delete.
 */
#define kre_list_del( list_head ) \
			list_del( list_head )

/**	Macro for iteration of kre list entry.
 *	@param pos	The &struct list_head to use as a loop cursor.
 *	@param head	The head for your list.
 */
#define kre_list_for_each( pos, head ) \
		list_for_each( pos, head )

/**	Macro for geting entry structure from list
 * 	Look for <linux/list.h> for more detail.
 * 	@param ptr		Pointer to list head.
 * 	@param type		Type of entry.
 * 	@param member	list head member name.
 * 	@return 		Pointer to entry.
 */
#define kre_list_entry( ptr, type, member ) \
		list_entry( ptr, type, member )


/*extern void kre_list_clean( kre_list_head *list );*/

#endif 
