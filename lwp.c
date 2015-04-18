/* Eric Yun & Justin Roll
 * CPE 453
 * Program 2
 */

#include <stdio.h>
#include <stdlib.h>
#include "lwp.h"

#define prev lib_one
#define next lib_two

/* GLOBALS */
static scheduler sched = {NULL, NULL, rr_admit, rr_remove, rr_next};
static rfile returnContext; /* pointer to original return context */
static context runningThread = NULL; /* global for the currently running thread */
static context head;
static tid_t tidCount = 1;
static void* returnSP; /* pointer to the return address */


/* The job of lwp create() is to set up a thread’s context
 * so that when it is selected by the scheduler to run and one of
 * lwp_yield(), lwp_start(), lwp_exit(), returns 1
 * to it, it will start executing where you want it to.
 */
tid_t lwp_create(lwpfun function, void *argument, size_t stacksize) {
   unsigned long *tempSP;
   unsigned long *tempBP;
   context *iter = head;
   
   /* assign null value to thread pointers if only one thread created, 
    * else set up doubly linked list of threads.
    */
   if (tidCount == 1) {
      iter->prev = NULL; 
      iter->next = NULL;
	   head = iter; 
   }
   else {
      for ( ; iter->next; iter = iter->next)
         ;
      iter->next = malloc(sizeof (context));
      
      /* return error value if malloc fails */
      if (iter->next == NULL)
         return (tid_t) -1;
      
      /* set prev to prior last item in list, 
       * go to new last item and set next to NULL
       */
      iter->next->prev = iter;
      iter = iter->next;
      iter->next = NULL;
   }
   
   /* assign tid, stack, and stack size to thread */
   iter->tid = tidCount++;
   iter->stack = malloc(stacksize * sizeof (unsigned long));
   iter->stacksize = stacksize;
  
   /* return error value if stack malloc fails */
   if (iter->next == NULL)
      return (tid_t)-1;
   
   /* assign function parameters to stack, including return address */
   tempSP = iter->stack + stacksize;
   //*(--tempSP) = (unsigned long)argument;
   *(--tempSP) = (unsigned long) lwp_exit;
   *(--tempSP) = (unsigned long) function;
   tempBP = --tempSP;
   *(--tempSP) = (unsigned long) tempBP;
   
   iter->stack->rdi = (unsigned long) argument;
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
   int i = 1;	

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
		SetSP(t->stack);	
		load_context(t->state);
	}
	else 	
		lwp_stop();

}

/* start the LWP system
 * 1. save "real" context where lwp_stop() can find it
 * 2. call scheduler, pick next lwp to run
 * 3. load the thread's context with swap_rfiles()
 * 4. if no next lwp exists, restore original system context and return
 */
void lwp_start(void) {
   GetSP(returnSP);
   
   /* save the base pointer, instruction pointer, etc */
   save_context(returnContext);
   
   /* picks the next thread in the schedule, loads it's context */
   runningThread = sched.next();
   
   if (runningThread == NULL) {
      load_context(returnContext);
      return;
   }
   
   load_context(runningThread->context);
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
   thread t = head;

   for ( ; t; t = t->next()) {
      sched.remove(t);
      fun.admit(t);
   }

   if (sched.shutdown != NULL)
      sched.shutdown();

   sched = *fun;
}

/* find out what the current scheduler is */
scheduler lwp_get_scheduler(void) {
   return sched; 
}

/* map a thread id to a context */
thread tid2thread(tid_t tid) {
   context iter;
   context ctx = sched->next()

   if (ctx) {
      for (iter = head; iter && iter->tid != tid; iter = iter->next)
         ;
      if (iter->tid == tid)
         return iter;
   }

   return NULL; 
}

/* 
 * void init(void)
 * This is to be called before any threads are admitted to the scheduler. 
 * It’s to al
 * low
 * the scheduler to set up. This one is allowed, to be NULL, so don’t call it if it is.
 * void shutdown(void)
 * This is to be called when the lwp library is done with a scheduler to allow
 * it to clean up. This, too, is allowed, to be NULL, so don’t call it if it is.
 * void admit(thread new)
 * Add the passed context to the scheduler’s scheduling pool.
 * void remove(thread victim)
 * Remove the passed context from the scheduler’s scheduling pool.
 * thread next()
 * Return the thread ID of the next thread to be run or
 * NO
 * THREAD
 * if there isn’t one.
*/

void rr_init() {
   return;
}

void rr_shutdown() {
   return;
}

void rr_admit(thread new) {
   return;
}

void rr_remove(thread victim) {
   return;
}

/* find the  next thread that should run 
 * go through the linked list, starting at the currently 
 * running process's next. if the currently running process's
 * next is null, start at the head of the list.
 * */
context rr_next() {
   context iter = runningThread->next?runningThread->next:head;

   /* this is basically a check to see if there is only 1 entry 
    * in the linked list
    */
   if (runningThread == head && runningThread->next == NULL) {
      return runningThread;
   }	

   for (; iter != runningThread; iter = iter->next?iter->next:head)
      ;

   return iter;
}
