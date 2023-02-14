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
 * 	kREproxy HTTP protocol service
 *	@file kre_http.h
 */
#ifndef KRE_HTTP_H
#define KRE_HTTP_H

#include "kre_threads.h"
#include "kre_network.h"
#include "kre_utils.h"

#define KRE_SERVER_NAME					"kREproxy (author: Marcin Tomasik)"
#define KRE_SERVER_NAME_SIZE			33

#define KRE_HTTP_REQ_BUF_SIZE			2048
/* settings of kre_http_message_header */
#define KRE_HTTP_MAX_FIELD_SIZE			100
#define KRE_HTTP_MAX_FIELD_VALUE_SIZE	1000
#define KRE_HTTP_MAX_REQUEST_LINE_SIZE	1000

#define KRE_HTTP_ERROR					1
#define KRE_HTTP_SHOULD_STOP			2

/* HTTP ok 2** */
#define KRE_HTTP_OK						200
#define KRE_HTTP_NO_CONTENT				204

/* HTTP errors 4** */
#define KRE_HTTP_BAD_REQUEST			400
#define KRE_HTTP_NOT_FOUND				404
#define KRE_HTTP_METHOD_NOT_ALLOWD		405
#define KRE_HTTP_REQ_ENTITY_TOO_LARGE	413
#define KRE_HTTP_REQ_URI_TOO_LARGE		414

/* KRE status */
#define KRE_HTTP_DST_UNRECHABLE			900
#define KRE_HTTP_SERVER_IS_BUSY			901

/* HTTP server 5** */
#define KRE_HTTP_INTERNAL_SERVER_ERROR	500
#define KRE_HTTP_NOT_IMPLEMENTED		501
#define KRE_HTTP_SERVICE_UNAVALIBLE		503

/* HTTP methods */
#define KRE_HTTP_METH_GET				0
#define KRE_HTTP_METH_POST				1
#define KRE_HTTP_METH_HEAD				2
#define KRE_HTTP_METH_OPTIONS			3
#define KRE_HTTP_METH_PUT				4
#define KRE_HTTP_METH_DELETE			5
#define KRE_HTTP_METH_TRACE				6
#define KRE_HTTP_METH_CONNECT			7
#define KRE_HTTP_METH_NOT_IMPLEMENTED	8

/** Message header. The same as in HTTP protocol meaning.
 */
struct kre_http_message_header {
	/*char field_name[ KRE_HTTP_MAX_FIELD_SIZE ]; */
	char field_value[ KRE_HTTP_MAX_FIELD_VALUE_SIZE ]; /**< Field value. */
	size_t	fv_size;									/**<  Field value size. */
};

/** kREproxy HTTP request representation.
 */
struct kre_http_request {
	int								method;			/**< %method / HTTP/1.1 value */
	struct kre_http_message_header	uri;			/**< METHOD %uri HTTP/1.1 value */
	struct kre_http_message_header	host;			/**< Host: HTTP header value */
	size_t							content_length;	/**< Content-Length: HTTP header value */
	int								state;	/**< State of request for examp. KRE_HTTP_BAD_REQUEST, ...NOT_IMPLEMENTED,... */
};


/**	Get HTTP method from b parameter.
 *	@param req	Place to save http method.
 *	@param b	Buffor with http request.
 *	@param size	Size of buffer. 
 */
extern void kre_http_get_meth_from_buf( struct kre_http_request *req, char *b, size_t size );


/**	Get HTTP Content-Length if exsists value.
 *	@param req	Place where contetn length value is saved if not exsixts 0 value is saved.
 *	@param b	Buffor with http request.
 *	@param size	Size of buffer. 
 */
extern void kre_http_get_content_length( struct kre_http_request *req, char *b, size_t size );


/**	Get URI from HTTP request ( from buffer b ).
 *	@param req	Place to save uri.
 *	@param b	Buffer with request.
 *	@param size	Size of buffer.  
 */
extern void kre_http_get_uri( struct kre_http_request *req, char *b, size_t size );


/**	Get Host from HTTP request b.
 *	@param req	Place to save host.
 *	@param b	Buffer with request.
 *	@param size	Size of buffer b.
 */
extern void kre_http_get_host( struct kre_http_request *req, char *b, size_t size );


/**	Get all needed field from request buf and save to req structure.
 *	@param req	Place to save request field.
 *	@param bu	Source request buffor.
 *  @param size	Size of buffer.
 *	@return		0 if everything is ok or -EINVAL, -1, or req->state ( KRE_HTTP_BAD_REQUEST, ... ).
 */
extern int kre_http_get_req_from_buf( struct kre_http_request *req, char *bu, size_t size );


/**	Get HTTP request from client to buffer bu
 *	and return count of received bytes or error or should stop flag.
 *	@param bu	Buffer where http request will go.
 *	@param s	Size of buffer.
 *	@param req	Place to save HTTP request in specific kre format.
 *	@param cur	Curent kre thread with csock.
 *	@return		-KRE_HTTP_ERROR | -KRE_HTTP_SHOULD_STOP | count of received bytes. 
 */
extern int kre_http_get_request( char *bu, size_t s, struct kre_http_request *req, struct kre_thread *cur );


/**	Transfer response from dst host to client host using buffer bu.
 *	@param bu	Buffer using during transfering.
 *	@param s	Size of buffer bu.
 *	@param thr	Thread structure with alocated and connecting dstsock and csock socket.
 *	@return 	Error code or 0.
 */
extern int kre_http_transfer_response( char *bu, size_t s, struct kre_thread *thr );


/**	Build response in bu based on http_error number.
 *	@param bu	Buffer where new response is build.
 *	@param size	Max size of buffer.
 *	@param code	KRE_HTTP_( response code ) for example KRE_HTTP_BAD_REQUEST,  
 *				KRE_HTTP_INTERNAL_SERVER_ERROR, ... , KRE_HTTP_SERVER_IS_BUSY,.. 
 */
extern void kre_http_set_response( char * bu, size_t size, int code);


#endif

