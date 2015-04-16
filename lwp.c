/* Eric Yun & Justin Roll
 * CPE 453
 * Program 2
 */

#include <stdio.h>
#include <stdlib.h>
#include "lwp.h"

#define MAX_THREADS 50

/* GLOBALS */
static scheduler sched = {NULL, NULL, rr_admit, rr_remove, NULL};
static rfile returnContext; /* pointer to original return context */

static int lwpIndex = 1;
static tid_t tidCount = 1;
//static context lwpThreads[MAX_THREADS];
static context lwpThreads;
static void* returnSP; /* pointer to the return address */


void rr_init() {

}

void rr_shutdown() {

}

void rr_admit(thread new) {

}

void rr_remove(thread victim) {

}

/* The job of lwp create() is to set up a thread’s context
 * so that when it is selected by the scheduler to run and one of
 * lwp_yield(), lwp_start(), lwp_exit(), returns 1
 * to it, it will start executing where you want it to.
 */
tid_t lwp_create(lwpfun function, void *argument, size_t stacksize) {
   context *iter = lwpThreads;
   
   if(lwpIndex == 1) {
      iter->lib_one = NULL;
      iter->lib_two = NULL;
   }
   else {
      while(iter->lib_two != NULL)
         iter = iter->lib_two;
      
      iter->lib_two = malloc(sizeof(context));
      
      if(iter->lib_two == NULL)
         return (tid_t) -1;
      
      iter->lib_two->lib_one = iter;
      iter = iter->lib_two;
      iter->lib_two = NULL;
   }
   
   iter->tid = tidCount++;
   iter->stack = malloc(stacksize * sizeof(unsigned long));
  
   *(iter->stack) = function; /* setting return address here */ 
   if(iter->lib_two == NULL)
      return (tid_t) -1;
   
   iter->stacksize = stacksize;
   iter->state = returnContext;
   
   
   
   /*
   if(tidCount > MAX_THREADS)
      return (tid_t) -1;
   
   lwpThreads[lwpIndex]->tid = tidCount++;
   lwpThreads[lwpIndex]->stack = malloc(stacksize * sizeof(unsigned long));
   
   if(lwpThreads[lwpIndex]->stack == NULL)
      return (tid_t) -1;
   
   lwpThreads[lwpIndex]->stacksize = stacksize;
   
   //EXPERIMENTAL CODE
   lwpThreads[lwpIndex]->state = returnContext;
   
   if(tidCount - 1 == 1) {
      lwpThreads[lwpIndex]->lib_one = NULL;
      lwpThreads[lwpIndex]->lib_two = NULL;
   }
   else {
      
   }
   
   return lwpThreads[lwpIndex++]->tid;
   */
}

/* terminates the calling LWP */
void lwp_exit(void) {
	context ctx = sched.next();
	
	if (!ctx) {
		lwp_stop();
	}
	
}

/* return thread ID of the calling LWP */
tid_t lwp_gettid(void) {
   return lwpThreads[lwpIndex]->tid;
}

/* yield the CPU to another LWP 
 * save current stack 
 * make it look like the "next" thread is
 * going to be called by putting it on top of
 * the stack
 */
void lwp_yield(void) {
	thread *t = sched.next();
	rfile currentRegisters;

	save_context(currentRegisters); /* how to make this last? */

	if (t != NULL) {
		load_context(thread->state);

	}
	else 	
		lwp_stop();

}

/* start the LWP system
 * 1. save "real" where lwp can find it
 * 2. call scheduler, pick a lwp to run
 * 3. load the thread's context with swap_rfiles()
 */
void lwp_start(void) {
	save_context(returnContext); /* save the base pointer, instruction pointer, etc */
	thread *t = sched.next();
	load_context(t->context); 
}

/* stop the LWP system. 
 * RESTORE the initial system context by returning to the global
 */
void lwp_stop(void) {
	SetSP(returnSP);
	
 	load_context(returnContext); /* load the registers from the return context global */
}

/* install a new scheduling function */
void lwp_set_scheduler(scheduler fun) {
	for(thread t = fun.next; t; t = RoundRobin->next) {
		sched.remove(t);
		fun.admit(t);
	}
	
	if(sched.shutdown != NULL)
		sched.shutdown();
	
	sched = *(fun);

}

/* find out what the current scheduler is */
scheduler lwp_get_scheduler(void) {
   return sched; 
}

/* map a thread id to a context */
thread tid2thread(tid_t tid) {
	int i;

	thread* first = scheduler.next();
  	for(i = 0; i < MAX_THREADS; i++) {
		if(lwpThreads[i]->tid == tid)
			return *(lwpThreads[i]);
	}
	return NULL;
 
}
