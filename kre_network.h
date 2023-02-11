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
 * 	kREproxy network interface.
 *	@file kre_network.h
 */
#ifndef KRE_NETWORK_H
#define KRE_NETWORK_H


#include <linux/module.h>	/* memcpy, memset, htonl, htons, ... */
#include <linux/net.h>		/* struct socket, connect, accept, listen ... */
#include <asm/uaccess.h>	/* get_fs, set_fs */ 
#include <linux/in.h>		/* struct sockaddr_in */
#include "kre_debug.h"

/**	Set addres to 0.0.0.0 and port to port.
 *	@param a	Addres to set.
 *	@param port	Port number. 
 */
extern void kre_net_set_local( struct sockaddr_in *a, int port );

/** Set addres to ip and port
 *	@param a	Pointer to addres which we set.
 *	@param ip	IP addres to set.
 *	@param port	Port number to set.
 */
extern void kre_net_set_addr( struct sockaddr_in *a, u8 ip[], int port );

/**	Check if not null and release socket.  
 */
extern void kre_net_sock_release( struct socket **s );

/** Creating ( alocating ) new socket.
 */
extern int kre_net_sock_create( struct socket **s );

/**	Set one of the socket options.
 *	REUSEADDR = 1
 *	this is not work and need recognition.
 */
extern int kre_net_sock_setopt_reuse( struct socket *s );

/**	Binding addres structure with socket. 
 *	@param s	Socket to bind.
 *	@param a	Addres structure to bind.
 */
extern int kre_net_sock_bind( struct socket *s, struct sockaddr_in *a );

/**	Turn on listening on socket.
 *	@param s	Socket which should listen.
 */
extern int kre_net_sock_listen( struct socket *s );

/**	Accepting client TCP request.
 *	@param srv	Pointer to server socket.
 *	@param cli	Pointer to pointer client socket.
 */
extern int kre_net_sock_accept( struct socket *srv, struct socket **cli );

/**	Connect to server with addres a using socket s.
 *	@param s	Socket 
 *	@param a	Addres
 *	@return 	error code or 0
 */
extern int kre_net_connect( struct socket *s, struct sockaddr_in *a );

/**	Receive len bytes from client socket s.
 *	@param s	Client socket.
 *	@param a	Client addres.
 *	@param buf	Buffer to handle data from client.
 *	@param len	Max bytes to get from client.
 */
extern int kre_net_sock_receive(struct socket *s, struct sockaddr_in *a, unsigned char *buf, int len); 

/**	Sending data to client socket
 *	@param s	Client socket.
 *	@param a	Cut addres to send to the client.
 *	@param buf	Data buffer which will be send to the client.
 *	@param len	Count of byte from data buffer which will be send to the client.  
 */
extern int kre_net_sock_send(struct socket *s, struct sockaddr_in *a, unsigned char *buf, int len);

#endif
