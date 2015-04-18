/* Eric Yun & Justin Roll
 * CPE 453
 * Program 2
 */

#include <stdio.h>
#include <stdlib.h>
#include "lwp.h"

#define prev lib_one
#define next lib_two
#define head sched_one


/* GLOBALS */
static scheduler sched = {NULL, NULL, rr_admit, rr_remove, NULL};
static rfile returnContext; /* pointer to original return context */

static context runningThread; /* global for the currently running thread */
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
   unsigned long *tempBP;
   context *iter = lwpThreads;
   
   /* assign null value to thread pointers if only one thread created, 
    * else set up doubly linked list of threads.
    */
   if(lwpIndex == 0) {
      iter->prev = NULL; 
      iter->next = NULL;
	   iter->head = iter; 
   }
   else {
      for(; iter->next; iter = iter->next)
         ;
      iter->next = malloc(sizeof(context));
      
      /* return error value if malloc fails */
      if(iter->next == NULL)
         return (tid_t)-1;
      
      /* set prev to prior last item in list, 
       * go to new last item and set next to NULL
       */
      iter->next->prev = iter;
      iter = iter->next;
      iter->next = NULL;
   }
   
   /* assign tid and stack to thread */
   iter->tid = tidCount++;
   iter->stack = malloc(stacksize * sizeof(unsigned long));
  
   /* return error value if malloc fails */
   if(iter->next == NULL)
      return (tid_t)-1;
   
   /* assign the size of the stack and registers to the thread */
   iter->stacksize = stacksize;
   iter->state = returnContext; //necessary? ask justin
   
   /* assign function parameters to stack, including return address */
   tempSP = iter->stack + stacksize;
   *(--tempSP) = (unsigned long)argument; //should this go into rdi instead?
   *(--tempSP) = (unsigned long)lwp_exit;
   *(--tempSP) = (unsigned long)function;
   tempBP = --tempSP;
   *(--tempSP) = (unsigned long)tempBP;
   
   iter->state->rsp = tempSP;
   iter->state->rbp = tempBP;
   
   return iter->tid;
}

/* terminates the calling LWP */
void lwp_exit(void) {
	context ctx = sched.next();
	
	if (!ctx) {
		lwp_stop();
	}
	
}

/* return thread ID of the calling LWP 
 *
 * */
tid_t lwp_gettid(void) {
   thread temp = sched.next();
   int i = 0;	

   for (; temp && temp != runningThread; i++, temp = temp->next)
      ;

   if (temp != NULL)
      return (tid_t) i;
   else
      return -1; /* error, couldn't find thread in linked list */
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

	save_context(runningThread->state); 

	if (t != NULL) {
		runningThread = t;
		load_context(t->state);
	
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
	runningThread = t;
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
	for(thread t = fun.next; t; t = RoundRobin->next()) {
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
	context temp;

	context ctx = sched->next()
	if (ctx) {
		for (temp = ctx->head, i = 0; temp && (tid_t) i != tid; temp = temp->next, i++)
			;
		if ((tid_t) i == tid)
			return temp;
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
