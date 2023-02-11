/*
 *	GPL vRelease licence
 *	
 *	kREproxy project (C) Marcin Tomasik 2009
 *
 *	<marcin.tomasik.priv[onserver]gmail.com>
 *	
 *	http://kreproxy.sourceforge.net/
 *
 */

/**
 * 	kREproxy configuration interface implementation.
 *	@file kre_config.c
 */
#include "kre_config.h"

void kre_config_register( kre_config *c ) 
{
	return kre_list_init( c );
}

int kre_config_unregister( kre_config *c ) 
{
	
	kre_list_head *ptr;
	struct kre_config_entry *entry;
	
	if( c ) {
/*		for( ptr= c->next; ptr != c; ptr= ptr->next ) {
			entry = kre_list_entry(	ptr, struct kre_config_entry, list);
			kfree( entry );
		}
*/		kre_list_for_each(ptr, c) {
			entry = kre_list_entry( ptr, struct kre_config_entry, list);
			KRE_DBG_INF(" before delete: [ %s, %s ] => [ %d.%d.%d.%d, %d ]\n",
					entry->req, entry->host, 
					(entry->dst_host[0]), (entry->dst_host[1]), 
					(entry->dst_host[2]), (entry->dst_host[3]),
					entry->dst_port);

			/* kfree can get NULL  */
			kfree( entry->host );
			kfree( entry->req );
			kfree( entry );
		}
		return 0;
	} 
	KRE_DBG("kre_config_unregister get NULL parameter c this is illegal\n");
	return -EINVAL; /* Invalid argument asm-generic/errno-base.h */
}



struct kre_config_entry *
kre_config_new_entry( 	const char *req,
						size_t		req_size,
						const char *host,
						size_t		host_size,
						u8 			dst_host[], 
						int 		dst_port )
{
	size_t i = 0;
	struct kre_config_entry *tmp = NULL;
	tmp = (struct kre_config_entry *)kmalloc( sizeof(struct kre_config_entry) ,GFP_KERNEL);

	if( tmp ) {
		KRE_DBG_INF(" kre_config_new_entry trying to copy datas\n"); 
	
		memset( tmp, 0 , sizeof( struct kre_config_entry) );
	
		/* allocate and copy data to http request */
		if(  (tmp->req = (char*)kmalloc( req_size+1 ,GFP_KERNEL))  == NULL ) {
			KRE_DBG("kre_config_new_entry cant allocate memory for req arg\n");
			goto req_err;
		}
		memset( tmp->req, 0, req_size+1);
		for(i=0; i<req_size; ++i) {
			tmp->req[i] = req[i];
									KRE_DBG_INF(" req[%d] = %c\n",i, tmp->req[i] );
			/*if( req[i] == 0 ) break;*/
		}

		/* allocate and copyt data to http host */
		if( (tmp->host = (char*)kmalloc( host_size+1, GFP_KERNEL)) == NULL ) {
			KRE_DBG("kre_config_new_entry cant allocate memory for host arg\n");
			goto host_err;	
		}
		memset( tmp->host, 0, host_size+1);
		for(i=0; i<host_size; ++i) {
			tmp->host[i] 		= host[i];
		}

		/* copy destination host 4 bytes addrees IP and destination port */
		memcpy( tmp->dst_host, dst_host, 4 );
		tmp->dst_port = dst_port;
	
		tmp->req_size = req_size;
		tmp->host_size= host_size;

		return tmp;
	} else {
		KRE_DBG(" kre_config_new_entry cant allocate memory\n");
		return NULL;
	}
	host_err:
		if(tmp && tmp->req) kfree(tmp->req);
	req_err: 
		if(tmp) kfree( tmp );
	return NULL;
}

int kre_config_add_entry_tail( struct kre_config_entry *new, kre_config *conf ) 
{
	if( new ) { 
		if( conf ) {
			kre_list_add_entry_tail( &(new->list), conf );
		} else {
			KRE_DBG(" Problem with argument of kre_config_add_entry_tail: conf is NULL or 0\n");
			return -EINVAL;
		}
	} else {
		KRE_DBG(" Problem with argument of kre_config_add_entry_tail: new is NULL or 0\n");
		return -EINVAL;
	}
	return 0;
}

int kre_config_add_entry( kre_config *c, const char *uri, size_t us, const char *host, size_t hs, u8 dst_host[], int dst_port)
{
	struct kre_config_entry	*tmp = NULL;
	int err;

	if( (tmp=kre_config_new_entry( uri, us, host, hs, dst_host, dst_port )) == NULL )
		return -ENOMEM;

	if( (err=kre_config_add_entry_tail( tmp, c )) != 0 )
		return err;
	
	return 0;
}

int kre_config_add_default_entry( kre_config *c, u8 dst_host[], int dst_port)
{	
	struct kre_config_entry *tmp = NULL;
	int err = 0;
	if( dst_port > 0 && dst_port < 65536 ) {
		if( (tmp=kre_config_new_entry( "", 0, "", 0, dst_host, dst_port )) == NULL )
	        return -ENOMEM;
		if( (err=kre_config_add_entry_tail( tmp, c )) != 0 )
			return err;
	} else {
		KRE_DBG("kre_config_add_default_entry: bad dst_port parameter, Range error\n");
		return -EINVAL;
	}
	return 0;
}


int kre_config_is_uri_equal( const char *u1, size_t s1, const char *u2, size_t s2)
{
	/* u1 is URI from configuration */
	/* u2 is URI from HTTP request */
	size_t i = 0;
	if( u1 ) {
		if( u2 ) {
			if( s1 > s2 ) { 
				if( u1[s1-1] == '*' && s1 == s2+1 ) {
					return kre_config_is_uri_equal(u1, s1-1, u2, s2 );
				}else 
					return 0;
			} else 
			if( s1 == s2 ) {
				for(i=0; i<s1; ++i ) {
					if( u1[i] == u2[i] )
						continue;
					else if( u1[i] == '*' )
						return 1;
					else 
						return 0;
				}	
			} else {
			/* s1 < s2 */
				if( u1[s1-1] != '*' ) return 0;
				for(i=0; i<s1-1; ++i) {
					if( u1[i] == u2[i] )
						continue;
					else
						return 0;
				}
			}

		} else {
			KRE_DBG("kre_config_is_uri_equal: Problem with argument u2 ( NULL )\n");
			return 0;
		}
	} else {
		KRE_DBG("kre_config_is_uri_equal: Problem with argument u1 ( NULL )\n ");
		return 0;
	}
	return 1;
}

int kre_config_is_host_equal( const char *h1, size_t s1, const char *h2, size_t s2)
{
	/* h1 - from configuration 
	 * h2 - from HTTP requst */
	size_t i = 0;
	size_t http = s2;
	if( h1 ) {
		if( h2 ) {
			if( s2 > 3 && h2[ http-3 ]==':' && h2[ http-2 ]=='8' && h2[ http-1 ]=='0' )
				http = s2-3;
			if( s1 == http ) {
				for(i=0; i<s1; ++i)
					if( h1[i] == h2[i] )
						continue;
					else
						return 0;
			} else {
				return 0;
			}
		} else {
			KRE_DBG("kre_config_is_host_equal: Problem with argument h2 (NULL)\n");
			return 0;
		}
	} else {
		KRE_DBG("kre_config_is_host_equal: Problem with argument h1 ( NULL )\n");
		return 0;
	}
	return 1;
}

struct kre_config_entry * kre_config_find( kre_config *c, const char *uri, size_t us, const char *host, size_t hs)
{
	kre_list_head *ptr = NULL;
	struct kre_config_entry *entry = NULL;
	struct kre_config_entry *last =  kre_list_entry( c->prev, struct kre_config_entry, list);
	if( c ) {
		kre_list_for_each(ptr, c) {
            entry = kre_list_entry( ptr, struct kre_config_entry, list);

			if(  entry == last ) {// entry == last ) {
				/* this is the last entry in configuration
				 * this always must be somethig like default destination  */
				KRE_DBG_INF("kre_config_find: last\n");
				return entry;
			} else {
				/* checking order must be: first host after uri */
				KRE_DBG_INF("kre_config_find: not last host [%s,%d] ? [%s,%d]\n", entry->host, entry->host_size, host, hs);
				KRE_DBG_INF("kre_config_find: not last uri  [%s,%d] ? [%s,%d]\n", entry->req,  entry->req_size, uri, us);
				if( kre_config_is_host_equal( entry->host, entry->host_size, host, hs ) == 1 &&
					kre_config_is_uri_equal( entry->req, entry->req_size, uri, us  ) == 1 )
				{
					KRE_DBG_INF("kre_config_find: found something\n");
					return entry;
				} 
			}
		}	
	} else {
		KRE_DBG("kre_config_find: Problem with argumetn c (NULL)\n");
	}
	return NULL;
}
