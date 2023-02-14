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
 *	kREproxy threads management interface implementation
 *	@file kre_threads.c
 */
#include "kre_threads.h"

/*
struct kre_thread {
	list;
	*task;
	void (*fun);
	*data;
	state;
	c;
}; */
void kre_threads_register_one_thread( struct kre_thread *t ) 
{
		t->task	= NULL;
		t->fun	= NULL;
		t->data	= NULL;
		t->csock	= NULL;
		t->dstsock= NULL;
		init_MUTEX(  &( t->state_sem )  );
		t->state	= KRE_NOT_INITIALIZED;
}

void kre_threads_register( kre_threads *t )
{
	size_t	 i=0;
	for(i=0; i< KRE_MAX_THREADS; ++i) {
		kre_threads_register_one_thread( &((*t)[i]) );
	}
}

int kre_threads_unregister( kre_threads *t )
{
	size_t	i=0;
	for(i=0; i< KRE_MAX_THREADS; ++i ) {
		down( &( (*t)[i].state_sem ) );
		switch( (*t)[i].state ) {
			case KRE_NOT_INITIALIZED:
			case KRE_INITIALIZED:
			case KRE_COMPLETED:
				KRE_DBG_INF("kre_threads_unregister [%d]:  not_init or initialized or completed\n",i);
			break;
			case KRE_SHOULD_STOP:
				up( &( (*t)[i].state_sem ) );
				wake_up_process( (*t)[i].task );
				KRE_DBG_INF("kre_threads_unregister: should_stop wait for completion\n");
				wait_for_completion( &( (*t)[i].c ) );
				KRE_DBG_INF("kre_threads_unregister: thread completed\n");
			break;
			case KRE_RUNNING:
				(*t)[i].state = KRE_SHOULD_STOP;
				up( &( (*t)[i].state_sem ) );
				wake_up_process( (*t)[i].task );
				KRE_DBG_INF("kre_threads_unregister: should_stop wait for completion\n");
				wait_for_completion( &( (*t)[i].c) );
				kre_net_sock_release( &( (*t)[i].csock) );
				kre_net_sock_release( &( (*t)[i].dstsock) );
				KRE_DBG_INF("kre_threads_unregister: thread completed\n");			
			break;
			default:
				KRE_DBG("kre_threads_unregister: ERROR bad state !!!\n");
		}
		up( &( (*t)[i].state_sem ) );	
	}
	return 0;
}

struct kre_thread * kre_threads_fff( kre_threads t )
{
	size_t i=0, count=3;
	if( t ) 

	for(i=0; i< KRE_MAX_THREADS; ++i ) {
		down( &(t[i].state_sem) );
		switch( t[i].state ) {
			case KRE_NOT_INITIALIZED:
			case KRE_INITIALIZED:
			case KRE_COMPLETED:
				up( &(t[i].state_sem) );
				KRE_DBG_INF("kre_threads_fff: found free thread structure [%d]\n",i);
				return &t[i];
			break;
			default:
				up(&(t[i].state_sem));
				if( i == KRE_MAX_THREADS-1) {
					--count;
					if( count == 0 ) return NULL;
					i = -1;
					msleep(20);
				}
			break;
		}
	}

	else {
		KRE_DBG("kre_threads_fff: WARNING bad parameter\n");
	}
	return NULL;
}

int kre_threads_find_by_pid( pid_t p, kre_threads t ) 
{
	size_t i=0;
	if( t ) {
		for(i=0; i< KRE_MAX_THREADS; ++i ) {
			down( &(t[i].state_sem) );
			switch( t[i].state ) {
				case KRE_RUNNING:
				case KRE_SHOULD_STOP:
					if( t[i].task ) {
						if( t[i].task->pid == p ) {
							up( &(t[i].state_sem) );
							return i;
						}
					} else {
						KRE_DBG("kre_threads_find_by_pid: ERROR state is running or should stop but t->task is NULL\n");
					}
				break;
			}
			up( &(t[i].state_sem) );
		}
	} else {
		KRE_DBG("kre_threads_find_by_pid: WARNING bad parameter\n");
		return -EINVAL;
	}
	return -1;
}

struct kre_thread * kre_threads_get_by_id( int id, kre_threads t )
{
	if( id > -1 && id < KRE_MAX_THREADS ) {
		if( t )
			return &t[id];
		else {
			KRE_DBG("kre_threads_get_by_id: WARNING bad parameter t\n");
			return NULL;
		}
	} 
	return NULL;
}

int kre_threads_should_work( struct kre_thread *t )
{
	int er = 1;
	if( t ) {
		down( &(t->state_sem)); 
			if( t->state == KRE_SHOULD_STOP )
				er = 0;
			else 
				er = 1;
		up( &(t->state_sem) );
		return er;
	 } else {
		KRE_DBG("kre_threads_should_work: ERROR bad parameter t\n");
		return -EINVAL;
	}
}


void kre_threads_complete( struct kre_thread *t )
{
	if( t ) {
	    lock_kernel();
			down( &(t->state_sem) );
			t->state = KRE_COMPLETED;
			up( &(t->state_sem) );
			complete( &(t->c) );
    	unlock_kernel();
	} else {
		KRE_DBG("kre_threads_complete: ERROR cant complet kre thread - bad parameter\n");
	}
}

int kre_threads_init( struct kre_thread *t, int (*f)( void* ), void* data )
{
	if( t ) {
		down( &(t->state_sem) );
		switch( t->state ) {
			case KRE_NOT_INITIALIZED:
				t->fun	= f;
				t->task	= NULL;
				t->data	= data;
				t->csock= NULL;
				t->dstsock=NULL;
				init_completion( &(t->c) );
				t->state = KRE_INITIALIZED;
				up( &(t->state_sem) );
				KRE_DBG_INF("kre_threads_init: initialized ok\n");
				return 0;
			break;
			case KRE_INITIALIZED:
			case KRE_COMPLETED:
				t->fun	= f;
				t->task = NULL;
				t->data = data;
				INIT_COMPLETION( t->c );
				kre_net_sock_release( &(t->csock) );
				kre_net_sock_release( &(t->dstsock) );
				t->state = KRE_INITIALIZED;
				up( &(t->state_sem) );
				KRE_DBG_INF("kre_threads_init: reinitialized initialized or completed kre thread\n");
				return 0;
			break;
			case KRE_SHOULD_STOP:
			case KRE_RUNNING:
				KRE_DBG("kre_threads_init: cant initialize should stop or running kre thread\n");
				goto error;
			break;
			default:
				KRE_DBG("kre_threads_init: ERROR bad state\n");
				goto error;
			break;
		}

	} else {
		KRE_DBG("kre_threads_init: ERROR bad parameter t\n");
		return -EINVAL;
	}
	error:
		up( &(t->state_sem) );
		return -1;
}

int kre_threads_start( struct kre_thread *t )
{
	int er = 0;
	if( t ) {
		down( &(t->state_sem) );
		t->task =  kthread_run( t->fun, t->data, KRE_NAME );
	    if( IS_ERR( t->task ) ) {
			KRE_DBG("kre_threads_start: ERROR kre_thread not started\n");
			er = -1;
			goto error;
		} else {
			t->state = KRE_RUNNING;
			up( &(t->state_sem) );
			KRE_DBG_INF("kre_threads_start: end\n");
			return 0;
		}
	} else {
		KRE_DBG("kre_threads_start: ERROR bad parameter t\n");
		return -EINVAL;
	}
	error:
		up( &(t->state_sem) );
		return er;
}

int kre_threads_stop( struct kre_thread *t )
{
	int er = 0;
	if( t ) {
		down( &(t->state_sem) );
		if( t->state == KRE_RUNNING ) {
			t->state = KRE_SHOULD_STOP;
			up( &(t->state_sem) );
			wake_up_process( t->task );
				KRE_DBG_INF("kre_threads_stop: should_stop wait for completion\n");
			wait_for_completion( &(t->c) );
       			KRE_DBG_INF("kre_threads_stop: thread completed\n");
			kre_net_sock_release( &(t->csock) );
			kre_net_sock_release( &(t->dstsock) );
			msleep(100);
			return 0;
		} else {
			KRE_DBG("kre_threads_stop: ERROR bad state I cant stop thread with state other then running\n");
			er = -1;
			goto error;
		}
	} else {
		KRE_DBG("kre_threads_stop: ERROR bad parameter t\n");
		return -EINVAL;
	}
	error:
		up( &(t->state_sem) );
		return er;
}
