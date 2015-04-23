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
void printSchedSize();

/* GLOBALS */
static struct scheduler sched_global = {NULL, NULL, rr_admit, 
 rr_remove, rr_next};
static scheduler sched = &sched_global;
static rfile returnContext; /* pointer to original return context */
static void* returnSP; /* pointer to the return address */
static context* runningThread = NULL; /* currently running thread */
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

   thread iter = head;
   /* assign null value to thread pointers if only one thread created, 
    * else set up doubly linked list of threads.
    */
	printf("CREATE\n");

    fflush(stdout); 

   if (tidCount == 1) {
      
      iter = malloc(sizeof (context));
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
   
   iter->stack = malloc(stacksize  * sizeof (unsigned long));
   iter->stacksize = stacksize;
  
   /* return error value if stack malloc fails */
   if (iter->stack == NULL)
      return (tid_t)-1;
   
   /* assign function parameters to stack, including return address */
   tempSP = iter->stack + stacksize;
   //*(--tempSP) = (unsigned long)argument;
   *(--tempSP) = (unsigned long) lwp_exit;
   *(--tempSP) = (unsigned long) function;
   tempBP = --tempSP;
//   *(--tempSP) = (unsigned long) tempBP;
   
   iter->state.rdi = (unsigned long) argument;
   iter->state.rsp = (unsigned long) tempSP;
   iter->state.rbp = (unsigned long) tempBP;
 	printf("CREATING THREAD %d\n", iter->tid);
    fflush(stdout); 
  
   sched->admit(iter);
   return iter->tid;
}

void printSchedSize() {
	thread temp;
	int i = 0;

	for (temp = shead; temp; temp=temp->snext, i++)
		;

	printf("Threads in scheduler:%d\n", i); 


}

void removeFromLL(thread victim) {
   
   /* special case if we're at the head */	
   if (victim == head) {
      if (victim->tnext) {
         victim->tnext->tprev = NULL;
         head = victim->tnext;
      }

      else {
         head = NULL;
      }

   }

   if (victim->tprev) {
      victim->tprev->tnext = victim->tnext;
   }
   if (victim->tnext) {
      victim->tnext->tprev = victim->tprev;

   }
   victim->tnext = NULL;
   victim->tprev = NULL;

   return;
}


/* terminates the calling LWP */
void lwp_exit(void) {
   unsigned long *safeStack = returnSP - (sizeof(unsigned long) * 4);
   unsigned long  *tempBP;
   unsigned long *stack;
   rfile safeRegisters;   
   
   save_context(&safeRegisters);

   *(--safeStack) = (unsigned long) lwp_exit;
   *(--safeStack) = (unsigned long) safeRegisters.rsp;
   tempBP = --safeStack;
//   *(--safeStack) = (unsigned long) tempBP;
   safeRegisters.rsp = (unsigned long) safeStack;
   safeRegisters.rbp = (unsigned long) tempBP;
   //printf("%p\n", returnSP);
   removeFromLL(runningThread);
   sched->remove(runningThread);

   thread next = sched->next();	
   SetSP(safeStack);
//   load_context(&safeRegisters);
   
//   free(runningThread);
   free(runningThread->stack);
 	printf("EXITING THREAD %d\n", runningThread->tid);
    fflush(stdout); 


   if (!next) {
	  runningThread = NULL;
      lwp_stop();
   }
   else {
	  runningThread = next;

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
   //GetSP(runningThread->state.rsp);
   
   /* picks the next thread in the schedule
    * loads new context if next thread exists, 
    * else restore previous contest
    */
   runningThread = sched->next();
   
   if (runningThread == NULL) {
      //SetSP(returnSP);
      load_context(&returnContext);
   }
   else {
      //SetSP(runningThread->state.rsp);
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
   printf("STARTING\n");
   fflush(stdout);   
   /* save previous context and stack pointer */
   save_context(&returnContext);
   returnSP = (unsigned long *) returnContext.rsp;
   /* picks the next thread in the schedule
    * loads new context if next thread exists, else restore previous contest
    */
   runningThread = sched->next();
   printf("GOT HERE\n");
   fflush(stdout);   


   if (runningThread == NULL) {
   printf("GO BACK\n");
   fflush(stdout);   

      load_context(&returnContext);
   }
   else {
   printf("load context\n");
   fflush(stdout);   

      load_context(&(runningThread->state));
   }
}

/* stop the LWP system. 
 * RESTORE the initial system context by returning to the global
 */
void lwp_stop(void) {

   //printf("Stop\n");
   /* stores current context and stack pointer before stopping */
   if (runningThread) {
   		save_context(&(runningThread->state));
   		GetSP(runningThread->state.rsp);
   }

   /* loads globally stored context and stack pointer */
   load_context(&returnContext);
}

/* install a new scheduling function */
void lwp_set_scheduler(scheduler fun) {
   thread t = shead;
   for ( ; t; t = shead) {
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
   iter->snext = new;
   new->snext = NULL;
   new->sprev = iter;
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
   thread prior = NULL; 
   /* If nothing is running and there's threads in the pool, 
 	  return the head. If nothing is running and there's no
	  threads left in the pool, return NULL */
   if (runningThread == NULL) {
   	  if (shead != NULL) {
	  	return shead;
	  }
   	  else {
	  	return NULL;
   	  }
   }

   thread iter = runningThread->snext?runningThread->snext:shead;

   /* this is basically a check to see if there is only 1 entry 
 	* in the linked list*/
   if (runningThread == shead && runningThread->snext == NULL) {
      return runningThread;
   }
  
   for (prior = iter; iter == runningThread; 
    iter = iter->snext?iter->snext:shead) {
		prior = iter;
   }

   return prior;
}
