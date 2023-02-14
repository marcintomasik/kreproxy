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


#include "kre_utils.h"

/* power x to y  ok work fine*/
int kre_util_pow( int x, int y )
{
	size_t i=0;
	int tmp = x;
	if( y == 0 ) return 1;
	for(i=0; i<y-1; ++i ) {
		tmp *= x;
	}
	return tmp;
}

/* only one char digit to int conversion work fine */ 
int kre_util_chartoint( char *c ) 
{
	size_t i=0;
	for(i=48; i<58; ++i ) {
		if( (int)(*c) == i )
			return (int)(i)-48;
	}
	return 0;	
}

/* not work but should */
int kre_util_atoi( char *number, size_t s )
{
	size_t i=0;
	size_t k=0;
	int tmp = 0;
	if( s == 0 && number )
		i = strlen(number);
	else if( s > 0 && number ) {
		i = s;
	} else {
		KRE_DBG("kre_util_atoi: bad parmeter number or size < 0\n");
		return -1;
	}

	for(i=i-1, k=0; i>=0; --i, ++k ) {
		tmp += kre_util_chartoint( &number[i] ) * kre_util_pow( 10, k );
	}
	return tmp;
}

/* not work but should */
void kre_util_strreverse( char *rev, const char *str, size_t s )
{
	char tmp[15];
	size_t size=0, i=0, k=0;
	memset( tmp, 0, 15 );
	if( s != 0 ) {
		size = s;
	} else {
		size = strlen( str );
	}
	
	if( size <15 ) {
		k=0;
		for(i=size-1; i>=0; --i ) {
			rev[k] = str[i]; 
			++k;
		}
	} else {
		KRE_DBG("kre_util_strreverse: size to large");
	}
}
