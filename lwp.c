/* Eric Yun & Justin Roll
 * CPE 453
 * Program 2
 */

#include <stdio.h>
#include <stdlib.h>
#include "lwp.h"

#define tprev lib_one
#define tnext lib_two
#define sprev sched_one
#define snext sched_two 

context* rr_next();
void rr_remove(thread victim);
void rr_admit(thread new);

/* GLOBALS */
static struct scheduler sched_global = {NULL, NULL, rr_admit, rr_remove, rr_next};
static scheduler sched = &sched_global;
static rfile returnContext; /* pointer to original return context */
static void* returnSP; /* pointer to the return address */
static context* runningThread = NULL; /* global for the currently running thread */
static context* head = NULL;
static thread shead = NULL;
static tid_t tidCount = 1;


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
      iter->tprev = NULL; 
      iter->tnext = NULL;
	   head = iter; 
   }
   else {
      for ( ; iter->tnext; iter = iter->tnext)
         ;
      iter->tnext = malloc(sizeof (context));
      
      /* return error value if malloc fails */
      if (iter->tnext == NULL)
         return (tid_t) -1;
      
      /* set prev to prior last item in list, 
       * go to new last item and set next to NULL
       */
      iter->tnext->tprev = iter;
      iter = iter->tnext;
      iter->tnext = NULL;
   }
   
   /* assign tid, stack, and stack size to thread */
   iter->tid = tidCount++;
   iter->stack = malloc(stacksize * sizeof (unsigned long));
   iter->stacksize = stacksize;
  
   /* return error value if stack malloc fails */
   if (iter->tnext == NULL)
      return (tid_t)-1;
   
   /* assign function parameters to stack, including return address */
   tempSP = iter->stack + stacksize;
   //*(--tempSP) = (unsigned long)argument;
   *(--tempSP) = (unsigned long) lwp_exit;
   *(--tempSP) = (unsigned long) function;
   tempBP = --tempSP;
   *(--tempSP) = (unsigned long) tempBP;
   
   iter->state.rdi = (unsigned long) argument;
   iter->state.rsp = (unsigned long) tempSP;
   iter->state.rbp = (unsigned long) tempBP;
   
   sched->admit(iter);

   return iter->tid;
}

/* terminates the calling LWP */
void lwp_exit(void) {
   context *iter = head;
   
   while (iter != runningThread)
      iter = iter->tnext;
      
   iter->tprev->tnext = iter->tnext;
   iter->tprev = NULL;
   iter->tnext = NULL;
   
   sched->remove(iter);
   SetSP(returnSP);
   free(iter->stack);
   thread runningThread = sched->next();
   
   if (!runningThread) {
      lwp_stop();
   }
   else {
      SetSP(runningThread->state.rsp);
      load_context(&(runningThread->state));
   }
}

/* return thread ID of the calling LWP 
 *
 */
tid_t lwp_gettid(void) {
	if (runningThread != NULL)
		return runningThread->tid;
	else
		return NO_THREAD;
}

/* yield the CPU to another LWP 
 * save current stack 
 * make it look like the "next" thread is
 * going to be called by putting it on top of
 * the stack
 */
void lwp_yield(void) {
	/* save previous context and stack pointer */
   save_context(&(runningThread->state));
   GetSP(runningThread->state.rsp);
   
   /* picks the next thread in the schedule
    * loads new context if next thread exists, else restore previous contest
    */
   runningThread = sched->next();
   
   if (runningThread == NULL) {
      SetSP(returnSP);
      load_context(&returnContext);
   }
   else {
      SetSP(runningThread->state.rsp);
      load_context(&(runningThread->state));
   }
}

/* start the LWP system
 * 1. save "real" context where lwp_stop() can find it
 * 2. call scheduler, pick next lwp to run
 * 3. load the thread's context with swap_rfiles()
 * 4. if no next lwp exists, restore original system context and return
 */
void lwp_start(void) {
   /* exit if no threads to start */
   if (tidCount - 1 == 0)
      return;
   
   /* save previous context and stack pointer */
   save_context(&returnContext);
   GetSP(returnSP);
   
   /* picks the next thread in the schedule
    * loads new context if next thread exists, else restore previous contest
    */
   runningThread = sched->next();
   
   if (runningThread == NULL) {
      SetSP(returnSP);
      load_context(&returnContext);
   }
   else {
      SetSP(runningThread->state.rsp);
      load_context(&(runningThread->state));
   }
}

/* stop the LWP system. 
 * RESTORE the initial system context by returning to the global
 */
void lwp_stop(void) {
   /* stores current context and stack pointer before stopping */
   save_context(&(runningThread->state));
   GetSP(runningThread->state.rsp);
   
   /* loads globally stored context and stack pointer */
   SetSP(returnSP);
   load_context(&returnContext);
}

/* install a new scheduling function */
void lwp_set_scheduler(scheduler fun) {
   thread t = shead;

   for ( ; t; t = t->snext) {
      sched->remove(t);
      fun->admit(t);
   }

   if (sched->shutdown != NULL)
      sched->shutdown();

   sched = fun;
}

/* find out what the current scheduler is */
scheduler lwp_get_scheduler(void) {
   return sched; 
}

/* map a thread id to a context */
thread tid2thread(tid_t tid) {
   thread iter;

   for (iter = head; iter && iter->tid != tid; iter = iter->tnext)
      ;
   if (iter->tid == tid)
      return iter;

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
	thread iter;
	
	if (shead == NULL) {
		shead = new;
		new->snext = NULL;
		new->sprev = NULL;
		return;
	}
 	
	/* go to the last item of the list */
	for (iter = head; iter->snext; iter = iter->snext)
         ;
      
	      
/* set prev to prior last item in list, 
 * go to new last item and set next to NULL
 */
	if (iter->snext) {
		iter->snext->snext = new;
		new->sprev = iter->snext;
	}
	else {
		iter->snext = new;
		new->snext = NULL;
		new->sprev = iter;
	}

	return;
}

void rr_remove(thread victim) {

	/* special case if we're at the head */	
	if (victim == shead) {
		if (victim->snext) {
			victim->snext->sprev = NULL;
			shead = victim->snext;
		}
	
		else {
			shead = NULL;
		}
	
	}
	
	if (victim->sprev) {
		victim->sprev->snext = victim->snext;
	}
	if (victim->snext) {
		victim->snext->sprev = victim->sprev;

	}
	victim->snext = NULL;
	victim->sprev = NULL;

	return;
}


/* find the  next thread that should run 
 * go through the linked list, starting at the currently 
 * running process's next. if the currently running process's
 * next is null, start at the head of the list.
 * */
thread rr_next() {
   thread iter = runningThread->snext?runningThread->snext:shead;

	/* this is basically a check to see if there is only 1 entry 
 	* in the linked list*/
	if (runningThread == shead && runningThread->snext == NULL) {
		return runningThread;
	}	

   for (; iter != runningThread; iter = iter->snext?iter->snext:head)
      ;

   return iter;
}
