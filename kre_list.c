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
 *	kREproxy List interface implementation
 *	@file kre_list.c
 */
#include "kre_list.h"

void kre_list_init( kre_list_head *node )
{
	INIT_LIST_HEAD( node );
}

void kre_list_add_entry_tail( kre_list_head *new, kre_list_head *head ) 
{
	list_add_tail( new, head );
}


