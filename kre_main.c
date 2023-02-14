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
 * 	Main kREproxy Linux module implementation.
 *	@file kre_main.c
 */
#include <linux/kernel.h>   		/* printk() */
#include <linux/module.h>   		/* all modules */
#include <linux/init.h>				/* __init,__initdata ... */		


#include <linux/errno.h>			/* E*** */

#include <linux/net.h> 				/* for SS_flags SOCKET STATE */
#include <linux/netdevice.h>  		/* socket_create, lock/unlock_kernel... */
//#include <linux/ip.h>
//#include <linux/in.h>				/* IPPROTO_*, ... */

#include <linux/delay.h>			/* s/m sleep() */
//#include <linux/types.h>			/* u8,u16, lists... */
//#include <linux/smp.h> 
//#include <linux/smp_lock.h>		/* lock_kernel() unlock_kernel() */
//#include <linux/completion.h>
//#include <linux/signal.h>
//#include <linux/sched.h>			/* allow_signal, demonize */

#include "kre_list.h"
#include "kre_config.h"
#include "kre_threads.h"
#include "kre_network.h"
#include "kre_http.h"

#include "kre_options.h"

/* main kreproxy server thread */
static struct kre_thread kreproxy;

/* local configuration set */
static kre_config kre_main_conf;

/* client threads set */
static kre_threads kre_ts;

/* temporary thread pointer */
static struct kre_thread *tmpt = NULL;

//static struct kre_config_entry *tmp_entry;



/* main client service function */
static int kre_client( void *d )
{
	/* position in kre_ts set */
	int id = 0, b = 0, er = 0;
	/* current kre_thread */
	struct kre_thread *cur = NULL;

	/* data buffor */
	unsigned char buf[ KRE_HTTP_REQ_BUF_SIZE + 1 ];
	
	struct kre_http_request req;
	
	struct kre_config_entry *tmp_entry = NULL;

	/* Destination server addres */
	struct sockaddr_in	dstaddr;

    lock_kernel();
		id	= kre_threads_find_by_pid( current->pid, kre_ts );
		cur	= kre_threads_get_by_id( id, kre_ts );
		if( cur == NULL )	{
			unlock_kernel();
			return -1;
		}
		/* this is emergency solution */
		cur->state = KRE_RUNNING;
		allow_signal(SIGKILL);
		daemonize( MOD_CLIENT );
   	unlock_kernel();

	KRE_DBG_INF("kre_client: PID= %d\n", cur->task->pid);
	KRE_DBG_INF("kre_client: value of csocket passed from kre_server [%p]\n", cur->csock );

	memset( &req, 0, sizeof( struct kre_http_request) );
	switch( b=kre_http_get_request( buf, KRE_HTTP_REQ_BUF_SIZE, &req, cur) ) {
		case -KRE_HTTP_ERROR:	
		case -KRE_HTTP_SHOULD_STOP:
		case -EINVAL:
			//goto should_stop;
		break;
		case -KRE_HTTP_REQ_ENTITY_TOO_LARGE:
            kre_http_set_response( buf, KRE_HTTP_REQ_BUF_SIZE+1, KRE_HTTP_REQ_ENTITY_TOO_LARGE );
            kre_net_sock_send( cur->csock, NULL, buf, strlen(buf) );
		break;
		case -KRE_HTTP_BAD_REQUEST:
		case 0:
			kre_http_set_response( buf, KRE_HTTP_REQ_BUF_SIZE+1 ,KRE_HTTP_BAD_REQUEST );
			kre_net_sock_send( cur->csock, NULL, buf, strlen(buf) );
		break;
		case -KRE_HTTP_NOT_IMPLEMENTED:
		default:
			/* request service */	
			KRE_DBG_INF("kre_client: METHOD: %d\n", req.method );
			KRE_DBG_INF("kre_client: length: %d\n", req.content_length );
			KRE_DBG_INF("kre_client: uri.size: %d\n", req.uri.fv_size );
			KRE_DBG_INF("kre_client: uri: [%s]\n",  req.uri.field_value );
			KRE_DBG_INF("kre_client: host.size: %d\n", req.host.fv_size );
			KRE_DBG_INF("kre_client: host: [%s]\n", req.host.field_value );
			KRE_DBG_INF("kre_client: REQ.state: %d\n", req.state );
			switch( req.method ) {
				case KRE_HTTP_METH_GET:
				case KRE_HTTP_METH_POST:
					/* service GET/POST request */
						KRE_DBG_INF("kre_client: finding[%s size%d, %s size%d]\n",req.uri.field_value, req.uri.fv_size,req.host.field_value, req.host.fv_size);
						tmp_entry	=	kre_config_find(	&kre_main_conf, 
															req.uri.field_value, req.uri.fv_size, 
															req.host.field_value, req.host.fv_size
														);
						if( tmp_entry ) {
							/* We have destination host
							 * so we must send request to it
							 */
							KRE_DBG_INF("kre_client: found entry: [%s, %s] [%d.%d.%d.%d : %d]\n",
										tmp_entry->req, tmp_entry->host,
										tmp_entry->dst_host[0], tmp_entry->dst_host[1],
										tmp_entry->dst_host[2], tmp_entry->dst_host[3],
										tmp_entry->dst_port
							);
							kre_net_set_addr( &dstaddr, tmp_entry->dst_host, tmp_entry->dst_port );
							kre_net_sock_create( &(cur->dstsock) );
							if( (er=kre_net_connect( cur->dstsock, &dstaddr )) < 0 ) {	
								/* send to the client info about destination
								 * server is unrechable */
								kre_http_set_response( buf, KRE_HTTP_REQ_BUF_SIZE+1, KRE_HTTP_DST_UNRECHABLE );
								kre_net_sock_send( cur->csock, NULL, buf, strlen(buf) );

								goto exit;
								//kre_net_sock_release( &(cur->dstsock) );
							} else {
								/* we have connection with destination server */
								kre_net_sock_send( cur->dstsock, NULL, buf, strlen(buf) );

								kre_http_transfer_response( buf, KRE_HTTP_REQ_BUF_SIZE+1, cur );

								goto exit;
							}						
						} else {
							/* there is no configuration entry for this request */
							KRE_DBG_INF("kre_client: I cant find appropriate config entry\n");
							kre_http_set_response( buf, KRE_HTTP_REQ_BUF_SIZE+1 ,KRE_HTTP_BAD_REQUEST );
							kre_net_sock_send( cur->csock, NULL, buf, strlen(buf) );
						}							
				break;
				case KRE_HTTP_METH_TRACE:
					/* sending his request to client */
					/* this need change  HTTP/1.1 specyfication need 
					 * send response 200 OK with request as body */
					kre_net_sock_send( cur->csock, NULL, buf, strlen(buf) );
				break;
				case KRE_HTTP_METH_OPTIONS:
				case KRE_HTTP_METH_PUT:
				case KRE_HTTP_METH_CONNECT:
				case KRE_HTTP_METH_DELETE:
				case KRE_HTTP_METH_NOT_IMPLEMENTED:
				default:
					kre_http_set_response( buf, KRE_HTTP_REQ_BUF_SIZE+1 ,KRE_HTTP_NOT_IMPLEMENTED );
					kre_net_sock_send( cur->csock, NULL, buf, strlen(buf) );
				break;
			}
		break;
	}
	
	exit:
	//should_stop:
		kre_net_sock_release( &(cur->dstsock) );
		kre_net_sock_release( &(cur->csock) );	
		
		kre_threads_complete( cur );	
	return 0;
}

/* main kreproxy server thread function */
static int kre_server( void *d )
{ 
	struct sockaddr_in local;
    /* data buffor */
    unsigned char buf[ KRE_HTTP_REQ_BUF_SIZE + 1 ];

	lock_kernel();
		kreproxy.state = KRE_RUNNING;
		daemonize( MOD_NAME );
		allow_signal( SIGKILL );

		kre_net_set_local( &local, KRE_LISTEN_PORT );

		/* kreproxy.csock it is not so good name because
		 * structure kre_trhead its designed for client
		 * service, actually server socket is stored
		 * in csock in this specific case */
		kre_net_sock_create( &(kreproxy.csock) );
		kre_net_sock_setopt_reuse( kreproxy.csock );
	if( kre_net_sock_bind( kreproxy.csock, &local ) < 0 ) 	{ unlock_kernel(); goto csock; }
	if(	kre_net_sock_listen( kreproxy.csock ) < 0 ) 		{ unlock_kernel(); goto csock; }

		/* kre threads registration */
		kre_threads_register( &kre_ts );	
	unlock_kernel();

	/* client accepting loop */
	while( kre_threads_should_work( &kreproxy ) ) {

		/* searching for free thread 
		 * to service new connection 
		 */
		tmpt = NULL;
		tmpt = kre_threads_fff( kre_ts );

		if( tmpt ) {
			/* free thread found */

			kre_threads_init( tmpt, kre_client, NULL );
		
			kre_net_sock_create( &(tmpt->csock) );
			
			
			if(  kre_net_sock_accept( kreproxy.csock, &(tmpt->csock) ) > -1 ) { 
				/* new connection accepted */
				
				KRE_DBG_INF("kre_server: kre_thread csock pointer [%p]\n",tmpt->csock);

				if(  kre_threads_start( tmpt ) == 0  ) {
					/*	new client thread is working 
				 	*	and it is responsible for stop/cleaning/release
				 	*	connection in tmpt->csock so we can not do anything
					*	more in this point: */
					continue;
				} else {
					/* 	new client thread crushed 
				 	*	so we must inform accepted client 
				 	*	that server cant service it */
		            kre_http_set_response( buf, KRE_HTTP_REQ_BUF_SIZE+1 ,KRE_HTTP_INTERNAL_SERVER_ERROR );
		            kre_net_sock_send( tmpt->csock, NULL, buf, strlen(buf) );
					goto not_accepted;
				}
			} else {
				/* problem with accepting new coonection */
				goto not_accepted;
			}
		} else {
			/*	server cant find free thread to 
			 *	service accepted client, we must
			 *	inform client that server is busy
			 *	now and taht "he can try later" */
			continue;
		}

	
		not_accepted:
			kre_net_sock_release( &(tmpt->csock) );
		/*end_loop:*/
			KRE_DBG_INF("kre_server: end of accept iteratio\n");	
	}

	csock:
	kre_net_sock_release( &(kreproxy.csock) );
	
	/* killing client threads */
	kre_threads_unregister( &kre_ts );

	kre_threads_complete( &kreproxy );
	return 0;
}


static int __init kre_init(void) 
{
	int err=0;
 	u8	ip[] = {192,168,64,1};
	
	
	printk(KERN_INFO "\n"MOD_NAME": KREPROXY START SEQUENCE\n");
	
	/* creating main configuration */
	kre_config_register( &kre_main_conf );



	/* ******************************************************************************************** */
	/* ******************	kREproxy configuration ************************************************ */
	/* ******************************************************************************************** */
	if( (err=kre_config_add_entry( &kre_main_conf, "/bank/*",	7, "mgr", 		3, ip, 8080 )) != 0 ) 
		goto config_err;

	if( (err=kre_config_add_entry( &kre_main_conf, "/*",		2, "asadmin", 	7, ip, 4848 )) != 0 ) 
		goto config_err;

	if( (err=kre_config_add_entry( &kre_main_conf, "/*",		2, "mgr", 		3, ip, 80 )) != 0 ) 
		goto config_err;

	/* creating and add default configuration entry */
	if( (err=kre_config_add_default_entry( &kre_main_conf, 							ip, 80 ) ) != 0 )
		goto config_err;

	/* ******************************************************************************************** */



	/* starting main kreproxy thread */
	kre_threads_register_one_thread( &kreproxy );
	kre_threads_init( &kreproxy, kre_server, NULL);
	kre_threads_start( &kreproxy );
	

	printk(KERN_INFO MOD_NAME": STARTED\n");
	return  0; /* non-zero means module can't be loaded. */

	config_err:
		kre_config_unregister( &kre_main_conf );
		return err;
}

/* if module's unloading is disabled
 * it could be never called */
static void __exit kre_exit(void) 
{	
	KRE_DBG_INF("KREPROXY EXIT SEQUENCE\n");

	kre_config_unregister( &kre_main_conf );
	KRE_DBG_INF("KREPORXY unregistred conf\n");

	/* killing main kreproxy thread */
	kre_threads_stop( &kreproxy );
	kre_net_sock_release(	&(kreproxy.csock) );	

	printk(KERN_INFO MOD_NAME": STOPED\n");
}


/* module info */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marcin Tomasik <marcin[at]tomasik.pl>");
MODULE_DESCRIPTION("kREproxy - simple reverse proxy [kreproxy.sourceforge.net]");
MODULE_VERSION( MOD_VER );

module_init( kre_init );
module_exit( kre_exit );

