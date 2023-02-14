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
 *	kREproxy threads management interface
 *	@file kre_threads.h
 */
#ifndef KRE_THREADS_H
#define KRE_THREADS_H

#define KRE_NAME	"kreproxythread"

/* kREproxy thread states */
#define KRE_NOT_INITIALIZED	0
#define	KRE_INITIALIZED		1
#define KRE_RUNNING			2
#define KRE_SHOULD_STOP		3
#define KRE_COMPLETED		4 

#include <linux/kthread.h>
//#include <linux/smp_lock.h>

#include <linux/netdevice.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <asm/semaphore.h>
#include "kre_debug.h"
#include "kre_network.h"

#ifndef KRE_MAX_THREADS
#	define KRE_MAX_THREADS	10
#endif 

/**	kREproxy thread representation
 */
struct kre_thread {

	/** Struct of actual kernel thread. */
	struct task_struct *task;

	/** Thread function with pointer to this struct. */
	int (*fun)( void * );

	void *data;

	/** State can be KRE_INITIALIZED, KRE_RUNNING ... */
	int state;

	/** SEMAPHOR 
	 * for change state. */
	struct semaphore state_sem;

	/** Info about completion of this thread
	 * this use instaid of 'state' when kre_thread struct must be 
	 * clean.
	 */
	struct completion c;

	/** Accepted client socket */
	struct socket *csock;

	/** Proxy destination server socket */
	struct socket *dstsock;

};

/**	kREproxy thread group representation
 * 	This should be declared once in global scope.
 */
typedef	struct kre_thread kre_threads[ KRE_MAX_THREADS ];


/** Registration one thread. This can be used to register main
 * 	kREproxy server thread. 
 */
extern void kre_threads_register_one_thread( struct kre_thread *t );

/**	Registration group of thread.
 */
extern void kre_threads_register( kre_threads * );

/**	Unregistration group of thread.
 */
extern int kre_threads_unregister( kre_threads * );

/**	Initialization of thread. 
 *	@param t	Thread to initlize.
 *	@param f	Thread function.
 *	@param data	Pointer to external thread data. NOT WORK AT NOW.
 *	@return		Error report or 0 if ok.
 */
extern int kre_threads_init( 	struct kre_thread *t, 
								int (*f)( void * ), 
								void *data
						  );

/**	Starting the new thread.
 *	@param t	Kreporxy thread to start.
 *	@return		Error report.
 */ 
extern int kre_threads_start( struct kre_thread *t );

/**	Find Firs Free kre_thread structure.
 *	@param t	Set of threads to search.
 *	@return		Thread which was found or NULL. 
 */
extern struct kre_thread * kre_threads_fff( kre_threads t );

/*	kre_thread_add_tail - add new thread structure on the end of tset
 *	@new: new thread to add
 *	@tset: pointer to set of threads
 */	
//extern int kre_thread_add_tail( struct kre_thread *new, kre_threads *tset );


/*	kre_thread_count - count thread in thread set
 *	@t: set of kre threads
 *
 * 	@return: count of kre threads ( this function 
 * 	counting all states KRE_NOT_INI..., KRE_INITI...,.... ) 
 */
//extern int kre_thread_count( kre_threads *t );


/**	Find localization of kre_thread in kre_threads set by pid.
 *	@param p	Function search t set looking for pid p.
 *	@param t	Set of kre threads.
 *	@return		id of kre_thread struct in set of kre threads or -1 if not found.
 */
extern int kre_threads_find_by_pid( pid_t p, kre_threads t );


/**	Return kre thread by id.
 *	@param id	Identyficator returned for example by kre_threads_find_by_pid().
 *	@param t	Set of kre threads.
 */
extern struct kre_thread * kre_threads_get_by_id( int id, kre_threads t );

/**	Set things, and inform other task that
 *	actual task is completed.
 *	@param t	Current kre thread structure.
 */
extern void kre_threads_complete( struct kre_thread *t );

/**	It can tell whether kthread should work or finish all jobs
 *	@param t	Current thread structure.
 *	@return		0 - if should stop, 1 - if should work.
 */
extern int kre_threads_should_work( struct kre_thread *t );

/**	The main cause of exist this function is
 *	to kill main kreproxy server thread in clear way.
 *	@param t	kre_thread structure with running task.
 *	@return 	Error output.
 */
extern int kre_threads_stop( struct kre_thread *t);

#endif
