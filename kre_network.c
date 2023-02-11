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
 *	kREproxy network interface implementation
 *	@file kre_network.c
 */
#include "kre_network.h"


void kre_net_set_local( struct sockaddr_in *a, int port  )
{
	unsigned char ip[] = {0,0,0,0};

	if( a ) {
        memset( a, 0, sizeof(struct sockaddr_in) );
		a->sin_family			= AF_INET;
		/* listen on any interface */
		memcpy( &(a->sin_addr), ip, 4 ); 
        a->sin_addr.s_addr      = htonl(INADDR_ANY);
		a->sin_port             = htons( port );

	} else {
		KRE_DBG("kre_net_set_local: bad parameter a\n");
	}
}

void kre_net_set_addr( struct sockaddr_in *a, u8 ip[], int port )
{
	if( a ) {
        memset( a, 0, sizeof(struct sockaddr_in) );
		a->sin_family			= AF_INET;
		/* listen on any interface */
		memcpy( &(a->sin_addr), ip, 4 ); 
       	a->sin_port             = htons( port );

	} else {
		KRE_DBG("kre_net_set_local: bad parameter a\n");
	}
}

void kre_net_sock_release( struct socket **s )
{
	if( *s ) {
		sock_release( *s );
		*s=NULL;
		KRE_DBG_INF("kre_net_sock_release: socket released\n");
	} else {
		KRE_DBG_INF("kre_net_sock_release: socket = NULL ok\n");
	}
}

int kre_net_sock_create( struct socket **s )
{
	int er = 0;
	if ( (er = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, s )) < 0 ) {
		KRE_DBG("kre_net_sock_create: I cant create socket error= [%d]\n", er);
    } else {
		KRE_DBG_INF("kre_net_sock_create: socket created\n");
	}
	return er;
}

int kre_net_sock_setopt_reuse( struct socket *s )
{
	int er = 0;
	if( s ) {
		/*	asm/socket.h asm-i386/socket.h linux/net.h */
		if( ( er= s->ops->setsockopt( s, SOL_SOCKET, SO_REUSEADDR, "1" , 1 ) ) < 0 ) {
			KRE_DBG("kre_net_sock_setopt: cant set options of socket error= [%d]\n", er);
			return er;
		} else {
			KRE_DBG_INF("kre_net_sock_setopt: setting socket options ok\n");
		}
	} else {
		KRE_DBG("kre_net_sock_setopt: bad parameter s\n");
		return -EINVAL;
	}
	return er;
}

int kre_net_sock_bind( struct socket *s, struct sockaddr_in *a )
{
	int er = 0;
	if( s ) {
		if( a ) {
															/* check what exactyly should be here */
			if( ( er = s->ops->bind( s, (struct sockaddr*)a, sizeof( *a ))) < 0 ) {
				KRE_DBG("kre_net_sock_bind: I cant bind socket with addres error= [%d]\n", er);
			} else {
				KRE_DBG_INF("kre_net_sock_bind: socket binded with addres\n");
			}
		} else {
			KRE_DBG("kre_net_sock_bind: bad parameter addres a \n");
			er = -EINVAL;
		}
	} else {
		KRE_DBG("kre_net_sock_bind: bad parameter socket s=NULL\n");
		er = -EINVAL;
	}
	return er;
}

int kre_net_sock_listen( struct socket *s )
{
	int er = 0;
	if( s ) {
		if( ( er = s->ops->listen( s, 5) ) < 0 ) {
			KRE_DBG("kre_net_sock_listen: with listening error= [%d]\n", er);
	    } else {
			KRE_DBG_INF("kre_net_sock_listen: listening turn on ...\n");
		}
	} else {
		KRE_DBG("kre_net_sock_listen: bad parameter socket s=NULL\n");
		er = -EINVAL;
	}
	return er;
}

int kre_net_sock_accept( struct socket *srv, struct socket **cli )
{
	int er = 0;
	if( srv ) {
		if( (*cli) ) {
			
			(*cli)->type = srv->type;
			(*cli)->ops = srv->ops;

			if(	/* declaration in  linux-2.6.*-src/include/linux/net.h */
				( er = srv->ops->accept( srv, (*cli), 0 ) ) < 0 ) {
				KRE_DBG("kre_net_sock_accept: problem while accepting connection error= [%d]\n", er);
			} else {
				KRE_DBG_INF("kre_net_sock_accept: new connection accepted\n");
			}

		} else {
			KRE_DBG("kre_net_sock_accept: bad parameter cli\n");
			return -EINVAL;
		}
	} else {
		KRE_DBG("kre_net_sock_accept: bad parameter srv\n");
		return -EINVAL;
	}

	return er;
}

int kre_net_connect( struct socket *s, struct sockaddr_in *a )
{
	int er = 0;
	if( s ) {
		if( a ) {
			//KRE_DBG_INF("kre_net_connect: addres")
			if( (er = s->ops->connect( s, (struct sockaddr *)a, sizeof( *a ), 0 ) ) < 0 ) {
				KRE_DBG("kre_net_connect: I cant connect to server error= [%d]\n", er);
				return er;
			} else {
				KRE_DBG_INF("kre_net_connect: I made connection with destination server\n");
				return er;
			}
		} else {
			KRE_DBG("kre_net_connect: bad parameter a ( addres is NULL )\n");
			return -EINVAL;
		}
	} else {
		KRE_DBG("kre_net_connect: bad parameter s ( socket is NULL)\n");
		return -EINVAL;
	}
	return er;
}

int kre_net_sock_receive(struct socket *s, struct sockaddr_in *a, unsigned char *buf, int len)
{
	struct iovec iov;
	struct msghdr msg;
	int size = 0;
	mm_segment_t oldfs;

	if( s ) {
		if( s->sk != NULL) {
					/* in out vector */
					iov.iov_base 		= buf; /* buffer  */
					iov.iov_len 		= len; /* length of bufer */

					/* Message */
					msg.msg_flags 		= 0;
					/*  msg.msg_flags |= MSG_WAITALL;*/  /* include/linux/socket.h */
					/*  msg.msg_flags |= MSG_FIN; */
					msg.msg_name 		= NULL; 	/* a ; */ 
					msg.msg_namelen		= 0; 	/* sizeof(struct sockaddr_in); */
					msg.msg_control		= NULL;
					msg.msg_controllen	= 0;
					msg.msg_iov			= &iov;  	/* data block io vector */
					msg.msg_iovlen 		= 1;		/* number of block */
					msg.msg_control		= NULL;

					oldfs = get_fs();	/* linux/uaccess.h  */
					set_fs(KERNEL_DS);  /* KERNEL_DS | USER_DS  to DS to jest od Data Segment */
								
						size = sock_recvmsg( s, &msg, len, msg.msg_flags);
								
					set_fs(oldfs);
		} else {
			KRE_DBG("kre_net_sock_receive: bad value of sk (NULL)\n");
			return size;
		}
	} else {
		KRE_DBG("kre_net_sock_receive: bad parameter s (NULL)\n");
		return -EINVAL;
	}
	return size;
}

int kre_net_sock_send(struct socket *s, struct sockaddr_in *a, unsigned char *buf, int len)
{
   	struct iovec iov;
	struct msghdr msg;
	int size = 0;
	mm_segment_t oldfs;
	
	if( s ) {
		if( s->sk != NULL ) {
				if( buf ) {

					iov.iov_base		= buf;
					iov.iov_len			= len;

					msg.msg_flags		= 0; 
					//msg.msg_flags 	|= MSG_FIN;
					msg.msg_name		= a;
					msg.msg_namelen		= sizeof(struct sockaddr_in);
					msg.msg_control		= NULL; /* tylko na BSD file descriptor passing */
					msg.msg_controllen	= 0;
					msg.msg_iov			= &iov;
					msg.msg_iovlen		= 1;
					msg.msg_control		= NULL; 

					oldfs = get_fs();
					set_fs(KERNEL_DS);
						size = sock_sendmsg( s, &msg, len);
					set_fs(oldfs);
				} else {
					KRE_DBG("kre_net_sock_send: bad parameter buf (NULL)\n");
					return -EINVAL;
				}
		} else {
			KRE_DBG("kre_net_sock_send: bad sk (NULL)\n");
			return size;
		}
	} else {
		KRE_DBG("kre_net_sock_send: bad parameter s (NULL)\n");
		return -EINVAL;
	}
	return size;
}

