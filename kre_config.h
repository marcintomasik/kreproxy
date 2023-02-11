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
 * 	kREproxy configuration interface
 *	@file kre_config.h
 */
#ifndef KRE_CONFIG_H
#define KRE_CONFIG_H

#include <linux/types.h> /* u8, u16 ... */
#include <linux/slab.h> /* kmalloc, kfree ... */
#include "kre_list.h"
#include "kre_debug.h"

#include "kre_options.h"

/**	Main kREproxy configuration structure.
 * 	This should be declared once in global
 * 	scope.
 */
typedef kre_list_head kre_config;

/** Configuration entry.	
 */
struct kre_config_entry {
	kre_list_head 	list;		/**< Pointer to next/prev entry. */
	char 			*req;		/**< Configuration URI. */ 
	size_t			req_size;	/**< Size of req. */
	char 			*host;		/**< Configuration host. */
	size_t			host_size;	/**< Size of host. */
	u8			dst_host[4];	/**< Destination IP addres. */
	int			dst_port;		/**< Destination port number. */
};
/*typedef struct _kre_config_entry_ kre_config_entry;*/


/**	kREproxy configuration registration 
 * 	@param Pointer to main configuration structure.
 */
extern void kre_config_register( kre_config * );


/** kREproxy configuration unregistration
 * 	@param Pointer to main configuration structure.
 * 	@return error output or 0 if everythig ok
 */
extern int  kre_config_unregister( kre_config * );


/** Creating new config entry.
 *	This function create new configuration entry allocate
 *	memory for new entry copy data and return address of
 *	this entry.
 *	@param req		Http GET/POST request.
 *	@param host		Http Host: entry.
 *	@param dst_host	Destination address IP.
 *	@param dst_port	Destination port.
 *	@return		  	Pointer to new allocated entry.
 */
extern struct kre_config_entry * kre_config_new_entry( 	 
										const char *req,
										size_t		req_size,
										const char *host,
										size_t		host_size,
										u8 			dst_host[], 
										int dst_port );


/**	Adding new configuration entry in tail of configuration list.
 *	@param new 	New added entry.
 *	@param conf	Actual configuration - place to add new entry.
 *	@return 	Error code or 0.
 */
extern int kre_config_add_entry_tail( struct kre_config_entry *new, kre_config *conf );


/** Creating and adding configuration entry.
 * 	This is main configuration adding function which end user should use.
 *	@param c		Pointer to main kREproxy configuration.
 *	@param uri		HTTP URI.
 *	@param us		Size of URI.
 *	@param host		HTTP host.
 *	@param hs		Size of host.
 *	@param dst_host	Destination host.
 *	@param dst_port	Destination port.
 *	@return Error code or 0.
 */
extern int kre_config_add_entry( kre_config *c, const char *uri, size_t us, const char *host, size_t hs, u8 dst_host[], int dst_port);

/**	Adding default entry. This is default destination host to
 * 	service requst which not match to any other configuration entry.
 * 	YOU HAVE TO REMEMBER THAT THIS ENTRY MUST BE SET AS LAST 
 * 	CONFIGURATION ENTRY.
 *	@param dst_host	Destination host.
 *	@param dst_port Destination port.
 *	@return error code or 0
 */
extern int kre_config_add_default_entry( kre_config *c, u8 dst_host[], int dst_port);

/**	Equal test of two uri from HTTP request
 *  and from configuration.
 * 	This is specific kreproxy comparison this can interpret asterisk ('*')
 * 	symbol on 	The end of uri as string of any char or zero chars.
 * 	@param u1 	URI 1 to equal test, which have to come from kre configuration.
 * 	@param s1 	Size of uri 1.
 * 	@param u2 	URI 2 to equal test, which have to come from HTTP request.
 * 	@param s2 	Size of uri 2.
 * 	@return 	0 if not equal, if equal 1. 
 */
extern int kre_config_is_uri_equal( const char *u1, size_t s1, const char *u2, size_t s2);


/**	Equal test of two hosts from HTTP request
 *	and configuation. This is simple equal test. If all of chars are equal 
 *	this function return 1 in other hand 0.
 *	@param h1	Host 1 to equal test, which have to come from kre configuration.
 *	@param s1 	Size of host 1.
 *	@param h2 	Host 2 to equal test, which have to come from HTTP request.
 *	@param s2 	Size of host 2.
 *	@return 	0 if not equal or 1 if equal. 
 */
extern int kre_config_is_host_equal( const char *h1, size_t s1, const char *h2, size_t s2);


/**	Find configuration structure witch tell kreproxy where
 *	pass the request from client.
 *	@param c	Pointer to main kREproxy configuration.
 *	@param uri	URI from client HTTP request.
 *	@param us	Size of URI.
 *	@param host	Host from client HTTP request.
 *	@param hs	Size of host.
 *	@return		Pointer to right configuration entry.
 */
extern struct kre_config_entry * kre_config_find( kre_config *c, const char *uri, size_t us, const char *host, size_t hs);



#endif
