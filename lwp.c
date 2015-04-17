/* Eric Yun & Justin Roll
 * CPE 453
 * Program 2
 */

#include <stdio.h>
#include <stdlib.h>
#include "lwp.h"

/* GLOBALS */
static scheduler sched = {NULL, NULL, rr_admit, rr_remove, NULL};
static rfile returnContext; /* pointer to original return context */

static int lwpIndex = 0;
static tid_t tidCount = 1;
static context lwpThreads;
static void* returnSP; /* pointer to the return address */

/* The job of lwp create() is to set up a thread’s context
 * so that when it is selected by the scheduler to run and one of
 * lwp_yield(), lwp_start(), lwp_exit(), returns 1
 * to it, it will start executing where you want it to.
 */
tid_t lwp_create(lwpfun function, void *argument, size_t stacksize) {
   unsigned long *tempSP;
   context *iter = lwpThreads;
   
   /* assign null value to thread pointers if only one thread created, 
    * else set up doubly linked list of threads.
    */
   if(lwpIndex == 0) {
      iter->lib_one = NULL;
      iter->lib_two = NULL;
   }
   else {
      while(iter->lib_two != NULL)
         iter = iter->lib_two;
      
      iter->lib_two = malloc(sizeof(context));
      
      /* return error value if malloc fails */
      if(iter->lib_two == NULL)
         return (tid_t) -1;
      
      iter->lib_two->lib_one = iter;
      iter = iter->lib_two;
      iter->lib_two = NULL;
   }
   
   /* assign tid and stack to thread */
   iter->tid = tidCount++;
   iter->stack = malloc(stacksize * sizeof(unsigned long));
  
   /* return error value if malloc fails */
   if(iter->lib_two == NULL)
      return (tid_t) -1;
   
   /* assign the size of the stack and registers to the thread */
   iter->stacksize = stacksize;
   iter->state = returnContext;
   
   /* assign function parameters to stack, including return address */
   tempSP = iter->stack;
   *(--tempSP) = argument;
   *(--tempSP) = lwp_exit(); // return address? TEST CODE
   *(--tempSP) = function;
   
   return lwpThreads[lwpIndex++]->tid;
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


void rr_init() {

}

void rr_shutdown() {

}

void rr_admit(thread new) {

}

void rr_remove(thread victim) {

}
