#include <stdio.h>
#include <stdlib.h>
#include "lwp.h"

#define MAX_THREADS 50

void init() {

}

void shutdown() {

}

void admit(thread new) {

}

void remove(thread victim) {

}

static scheduler sched = {NULL, NULL, admit, remove, NULL};
static scheduler *RoundRoubin = &sched;


static int lwpIndex = 0;
static tid_t tidCount = 1;
static context lwpThreads[MAX_THREADS];

/*The job of lwp create() is to set up a thread’s context
so that when it is selected by the scheduler to run and one of
lwp_yield(),lwp_start(),lwp_exit(),returns 1
to it, it will start executing where you want it to. */
tid_t lwp_create(lwpfun function, void *argument, size_t stacksize) {
   if(tidCount > MAX_THREADS)
      return (tid_t) -1;
   
   lwpThreads[lwpIndex]->tid = tidCount++;
   lwpThreads[lwpIndex]->stack = malloc(stacksize * sizeof(unsigned long));
   
   if(lwpThreads[lwpIndex]->stack == NULL)
      return (tid_t) -1;
   
   lwpThreads[lwpIndex]->stacksize = stacksize;
   
   
   
}

/* terminates the calling LWP */
void lwp_exit(void) {
   
}

/* return thread ID of thecalling LWP */
tid_t lwp_gettid(void) {
   return lwpThreads[lwpIndex]->tid;
}

/* yield the CPU to another LWP */
void lwp_yield(void) {

}

/* start the LWP system */
void lwp_start(void) {
   
}

/* stop the LWP system */
void lwp_stop(void) {
   
}

/* install a new scheduling function */
void lwp_set_scheduler(scheduler fun) {
	for (thread t = fun.next; t; t = RoundRobin->next) {
		RoundRobin->remove(t);
		fun.admit(t);
	}
}

/* find out what the current scheduler is */
scheduler lwp_get_scheduler(void) {
   
}

/* map a thread id to a context */
thread tid2thread(tid_t tid) {
   
}
