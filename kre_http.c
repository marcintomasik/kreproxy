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
 *	kREproxy HTTP management interface implementation
 *	@file kre_http.c
 */
#include "kre_http.h"

int kre_http_get_request( char *bu, size_t s, struct kre_http_request *req, struct kre_thread *thr ) 
{
	size_t b = 2, i = 0;
	size_t size = 0;
	int er = 0;

	memset( bu, 0, s + 1 );
	bu[0] = '-';
	bu[1] = '-';
	bu[2] = '-';
	b = 2;
	do {
		if( b%100 == 0 && kre_threads_should_work( thr ) == 0 ) {
			return -KRE_HTTP_SHOULD_STOP;
			//goto should_stop;
        }
	   	++b;
		if ( b >= s ) {
			KRE_DBG("kre_client: header is bigger then maximal count of bytes = %d \n", KRE_HTTP_REQ_BUF_SIZE);
			return -KRE_HTTP_REQ_ENTITY_TOO_LARGE;	
		}
		if( thr->csock != NULL  && thr->csock->type == SOCK_STREAM && thr->csock->state == SS_CONNECTED ) {
			size = kre_net_sock_receive( thr->csock, NULL, &bu[b], 1);
		} else {
			if( b == 3 ) KRE_DBG_INF("kre_client: kre_net_sock_receive can\'t be execute\n");
		}
	} while ( !(bu[b-3]=='\r' && bu[b-2]=='\n' && bu[b-1]=='\r' && bu[b]=='\n') && size > 0 );

	if( b>3 && size > 0 ) {
		b -= 2;
		strncpy( bu, &bu[3], b );
		memset( &bu[b], 0, 4 );

		/* build kre interior request structure */
		if( (er=kre_http_get_req_from_buf( req, bu, b )) != 0 )
			return er;

		/* if POST and Content-length: is set receive this*/
		if( req->method == KRE_HTTP_METH_POST && req->content_length > 0 ) {
			for(i=0; i< req->content_length; ++i ) {
				if( (size= kre_net_sock_receive( thr->csock, NULL, &bu[b++], 1)) <= 0 ) {
					KRE_DBG("kre_client:http_get_request: I get less then content length\n");
					break;
				}
			}
		}

		KRE_DBG_INF("kre_client: [%d] bytes received\n",b );
		KRE_DBG_INF("kre_client: \n[%s]\n", bu );
	} else {
		KRE_DBG("kre_client: I cant receive HTTP request from client\n");
		return -KRE_HTTP_ERROR;
	}

	return b;
}

int kre_http_transfer_response( char *bu, size_t s, struct kre_thread *thr )
{
	size_t b = 2, i = 0;
	size_t size = 0, int_multiple = 0, int_rest = 0, actual = 0;
	struct kre_http_request req;

	memset( bu, 0, s + 1 );
	bu[0] = '-';
	bu[1] = '-';
	bu[2] = '-';
	b = 2;
	do {
		if( b%100 == 0 && kre_threads_should_work( thr ) == 0 ) {
			return -KRE_HTTP_SHOULD_STOP;
			//goto should_stop;
        }
	   	++b;
		if ( b >= s ) {
			KRE_DBG("kre_client: response hader is bigger then maximal count of bytes= [%d]\n", KRE_HTTP_REQ_BUF_SIZE);
			return -KRE_HTTP_REQ_ENTITY_TOO_LARGE;	
		}
		if( thr->dstsock != NULL  && thr->dstsock->type == SOCK_STREAM && thr->dstsock->state == SS_CONNECTED ) {
			size = kre_net_sock_receive( thr->dstsock, NULL, &bu[b], 1);
			kre_net_sock_send( thr->csock, NULL, &bu[b], 1 );
		} else {
			if( b == 3 ) KRE_DBG_INF("kre_client: kre_net_sock_receive can\'t be execute\n");
		}
	} while ( !(bu[b-3]=='\r' && bu[b-2]=='\n' && bu[b-1]=='\r' && bu[b]=='\n') && size > 0 );

	if( b>3 && size > 0 ) {
		b -= 2;
		strncpy( bu, &bu[3], b );
		memset( &bu[b], 0, 4 );

		KRE_DBG_INF("kre_client: [%d] bytes received in response headers\n",b );
		KRE_DBG_INF("kre_client: response header\n[%s]\n", bu );

		kre_http_get_content_length( &req, bu, b );

		/* if POST and Content-length: is set receive this*/
		if( req.content_length > 0 ) {
			int_multiple	= req.content_length / (s-1);
			int_rest		= req.content_length % (s-1);

			KRE_DBG_INF("kre_client: C-Length=[%d] int_multiple=[%d], int_rest=[%d] s-1=[%d]\n",
						req.content_length, int_multiple, int_rest, s-1);
	
			if( int_rest > 0 )
				++int_multiple;		
						
			for(i=0; actual<req.content_length &&  kre_threads_should_work( thr ) == 1 && i<2000000; ++i ) {
				memset(bu, 0, s);
				if( (size= kre_net_sock_receive( thr->dstsock, NULL, bu, s-1)) <= 0 ) {
					KRE_DBG("kre_client:http_transfer_response: I get less then content length size=[%d]\n",size);
					break;
				} else {
					KRE_DBG_INF("kre_client: http_transfer_response: multi size=[%d]\n",size);
					actual += size;
					kre_net_sock_send( thr->csock, NULL, bu, size);
				}
			}
			KRE_DBG_INF("kre_client: http_transfer: count trasfered data =[%d]\n",actual);
						
		} else {
			/*  there is no other implementation in this moment
			 *	only response with Content-Length may be
			 *	transfered.
			 * */	
			KRE_DBG_INF("kre_cilent: I dont have implementation of transfering response body without Content-Length header\n");
		}


	} else {
		KRE_DBG("kre_client: I cant receive HTTP response from server\n");
		return -KRE_HTTP_ERROR;
	}

	return b;

}

void kre_http_set_response( char *bu, size_t size, int code )
{
	char e400[] 	= 	"HTTP/1.1 400 Bad Request\r\n";	// 12 + 1 + 11 + 2 = 26
	char l400[]		=	"Content-Length: 68\r\n";	//42 + 26		

	char e404[] 	= 	"HTTP/1.1 404 Not Found\r\n"; // 12 + 1 + 9 + 2 = 24
	char l404[]		=	"Content-Length: 66\r\n";		//42 +24

	char e413[]		=	"HTTP/1.1 413 Request Entity Too Large\r\n"; // 24 + 15 = 39
	char l413[]		=	"Content-Length: 81\r\n";			// 42 + 39 

	char e500[]		=	"HTTP/1.1 500 Internal Server Error\r\n";  // 21 +.. = 36
	char l500[]		=	"Content-Length: 78\r\n";			//42 + 36 =78

	char o200[]		=	"HTTP/1.1 200 OK\r\n";		// 17
	


	char head[] = 	"Server: "KRE_SERVER_NAME"\r\n" \
					"Connection: close\r\n" \
					"Content-Type: text/html; charset=iso-8859-1\r\n" \
					"\r\n" \
					"<h1>"KRE_SERVER_NAME"</h1>";
					/* The e400, e404, ... is concatenate in this place 
					 * therefor Content-Length is variable */

	char kre0[]		=	"Destination server unrechable"; //29
	char lkre0[]	=	"Conten-Length: 71\r\n"; // 42+29 = 71

	char kre1[]		=	"kREproxy server is busy, please try later"; // 41
	char lkre1[]	=	"Content-Length: 83\r\n";	// 42 + 41 = 83
	
	if( bu ) {
		memset(bu,0,size);

		switch( code ) {
			case KRE_HTTP_BAD_REQUEST:				strcat( bu, e400); strcat(bu, l400); strcat(bu, head); strcat(bu, e400); break;
			case KRE_HTTP_NOT_FOUND:				strcat( bu, e404); strcat(bu, l404); strcat(bu, head); strcat(bu, e404); break;
			case KRE_HTTP_INTERNAL_SERVER_ERROR: 	strcat( bu, e500); strcat(bu, l500); strcat(bu, head); strcat(bu, e500); break;
			case KRE_HTTP_REQ_ENTITY_TOO_LARGE:		strcat( bu, e413); strcat(bu, l413); strcat(bu, head); strcat(bu, e413); break;

			case KRE_HTTP_SERVER_IS_BUSY:			strcat( bu, o200); strcat(bu, lkre1);strcat(bu, head); strcat(bu, kre1); break;
			case KRE_HTTP_DST_UNRECHABLE:			strcat( bu, o200); strcat(bu, lkre0);strcat(bu, head); strcat(bu, kre0); break;
			default:								strcat( bu, e400); strcat(bu, l400); strcat(bu, head); strcat(bu, e400); break;
		}	
	} else {
		KRE_DBG("kre_http_set_error: bad parameter bu is null\n");
	}
}


void kre_http_get_meth_from_buf( struct kre_http_request *req, char *b, size_t size )
{
	if( req && b  && size >= 7 ) {
	
		if( b[0]=='G' && b[1]=='E' && b[2]=='T' ) {					/* GET */
			req->method = KRE_HTTP_METH_GET;
		}
		else if( b[0]=='P' && b[1]=='O' && b[2]=='S' && b[3]=='T' ) {		/* POST */
			req->method = KRE_HTTP_METH_POST;
		}
		else if( b[0]=='H' && b[1]=='E' && b[2]=='A' && b[3]=='D' ) {		/* HEAD */
			req->method = KRE_HTTP_METH_HEAD;
		}
		else if( b[0]=='O' && b[1]=='P' && b[2]=='T' && b[3]=='I' && b[4]=='O' && b[5]=='N' && b[6]=='S' ) { /* OPTIONS */
			req->method	= KRE_HTTP_METH_OPTIONS;
		}
		else if( b[0]=='P' && b[1]=='U' && b[2]=='T' ) {					/* PUT */
			req->method = KRE_HTTP_METH_PUT;
		}
		else if( b[0]=='T' && b[1]=='R' && b[2]=='A' && b[3]=='C' && b[4]=='E' ) {	/* TRACE */
			req->method = KRE_HTTP_METH_TRACE;
		}
		else if( b[0]=='D' && b[1]=='E' && b[2]=='L' && b[3]=='E' && b[4]=='T' && b[5]=='E' ) {	/* DELETE */
			req->method = KRE_HTTP_METH_DELETE;
		}
		else if( b[0]=='C' && b[1]=='O' && b[2]=='N' && b[3]=='N' && b[4]=='E' && b[5]=='C' && b[6]=='T' ) {	/* CONNECT */
			req->method = KRE_HTTP_METH_DELETE;
		} else {
			req->method = KRE_HTTP_METH_NOT_IMPLEMENTED;	/* NOT IMPLEMENTED */
			req->state	= KRE_HTTP_NOT_IMPLEMENTED;
		}
	} else {
		KRE_DBG("kre_http_get_meth_from_buf: bad parameter req or b \n");
	}
}

void kre_http_get_content_length( struct kre_http_request *req, char *b, size_t size )
{
	/* "Content-Length: " 16 characters
	 *	this can be after request line so minimal size of request line is 16 
	 *	so 16 + 16 = 32 this is thing to change */
	size_t	i=15, l=0, retval=0, p=0;
	char 	len[i];
	memset( len, 0, i); 
	if( req && b && size > 16) {
		for(i=15; i<size; ++i) {
			if( b[i-15]=='C' && b[i-14]=='o' && b[i-13]=='n'&& b[i-12]=='t' &&
				b[i-11]=='e' && b[i-10]=='n' && b[i-9]=='t' && b[i-8]=='-'  &&
				b[i-7]=='L'  && b[i-6]=='e'  && b[i-5]=='n' && b[i-4]=='g'  &&
				b[i-3]=='t'  && b[i-2]=='h'  && b[i-1]==':' && b[i-0]==' ' ) {
					l=0;
					KRE_DBG_INF("kre_http_get_content_length: found length header on position i=[%d]\n",i);
					do {
						len[l++] = b[++i];
					} while( ( b[i+1]!='\r' || b[i+2]!='\n' ) && l < 15);				
	
					KRE_DBG_INF("kre_http_get_contetn_length: end of length position=[%d] value=[%s] count of digit=[%d]\n",i,len,l );
		
					/* conversion from string to int value */
					for(p=0; p<l; ++p ) {
						retval += kre_util_chartoint( &len[p] ) * kre_util_pow( 10, (l-1)-p );
					}					
					break;
			}
		}
		req->content_length =  retval;
	} else {
		KRE_DBG("kre_http_get_content_length: bad parameter req or b\n");
	}
}


void kre_http_get_uri( struct kre_http_request *req, char *b, size_t size )
{
	char *tmp = NULL;
	size_t i  = 0;
	if( req && b && size > 16) { /* minimal size 'GET / HTTP/1.1\r\n' 16 */
		memset( req->uri.field_value, 0, KRE_HTTP_MAX_FIELD_VALUE_SIZE );
		tmp = strchr( b, (int)(' ') );

		for( tmp +=1; *tmp != ' ' && i<KRE_HTTP_MAX_FIELD_VALUE_SIZE; ++i, ++tmp ) {
			req->uri.field_value[i] = *tmp;	
		}
		req->uri.fv_size = i;
	} else {
		req->state = KRE_HTTP_BAD_REQUEST;
		KRE_DBG("kre_http_get_uri: bad parameter req or b or size <=16\n");
		
	}
}

void kre_http_get_host( struct kre_http_request *req, char *b, size_t size )
{
	size_t i = 0, s=0;
	char *tmp = NULL;
	if( req && b ) {
		memset( req->host.field_value, 0, KRE_HTTP_MAX_FIELD_VALUE_SIZE );
		if( size > 27 ) {  /* minimal size 'GET / HTTP/1.1\r\n'=16 + 'Host: x\r\n\r\n'=11 = MIN 27 */
			/* minimal position is 16 + 'Host: '=6 = 22 counting from 0 is 21 but
			 * we want to b[i] will be the first char of host sa therefor is 22 */
			for( i=22; i<size; ++i ) {
				if( b[i-6]=='H' && b[i-5]=='o' && b[i-4]=='s'  &&
					b[i-3]=='t' && b[i-2]==':' && b[i-1]==' ' ) {

					/* 'Host: xxx.com\r\n'  host can be NULL this is posibble only when 
					 * uri consist of absolut_path http://xx.xx/path/to/resource */
					tmp = &b[i];
					for( s=0 ; *tmp!='\r'&& *(tmp+1)!='\n' && s < KRE_HTTP_MAX_FIELD_VALUE_SIZE; ++s, ++tmp ) {
						req->host.field_value[s] = *tmp;
					}
					req->host.fv_size = s;
					
					/* in b[i] is the first char of host */
					//break;
					return;
				} else if( 	b[i-4]=='\r' && b[i-3]=='\n' &&
							b[i-2]=='\r' && b[i-1]=='\n' ) {
					req->state = KRE_HTTP_BAD_REQUEST;
					return;
				}
			}
		
		} else {
			KRE_DBG("kre_http_get_host: bad size\n");
			req->state = KRE_HTTP_BAD_REQUEST;
			return;
		}
	} else {
		KRE_DBG("kre_http_get_host: bad parameter req or b\n");
	}
}

int kre_http_get_req_from_buf( struct kre_http_request *req, char *bu, size_t size )
{
	if( req ) {
		if( bu ) {
			req->state = 0;
			kre_http_get_meth_from_buf( req, bu, size );
			if( req->state != 0 ) return -(req->state);
			kre_http_get_content_length( req, bu, size );
			kre_http_get_uri( req, bu, size);
			if( req->state != 0) return -(req->state);
			kre_http_get_host( req, bu, size);
			return req->state;

		} else {
			KRE_DBG("kre_http_get_req_from_buf: bad parameter bu is NULL\n");
			return -EINVAL;
		}
	} else {
		KRE_DBG("kre_http_get_req_from_buf: bad parameter req is NULL\n");
		return -EINVAL;

	}
	return -1;
}
