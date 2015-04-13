#include <stdio.h>
#include "lwp.h"

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

/*The job of lwp create() is to set up a thread’s context
so that when it is selected by the scheduler to run and one of
lwp_yield(),lwp_start(),lwp_exit(),returns 1
to it, it will start executing where you want it to. */
tid_t lwp_create(lwpfun,void *,size_t) {

}

/* terminates the calling LWP */
void  lwp_exit(void) {

}

/* return thread ID of thecalling LWP
 */
tid_t lwp_gettid(void) {

}

/* yield the CPU to another LWP */
void  lwp_yield(void) {

}

/* start the LWP system */
void  lwp_start(void) {


}

/* stop the LWP system */
void  lwp_stop(void) {

}

/* install a new scheduling function */
void  lwp_set_scheduler(scheduler fun) {
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

