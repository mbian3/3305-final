/******************************************************************************
*
* Name: 	Zaid Albirawi
* Email: 	zalbiraw@uwo.ca
*
* scheduler.c
*
******************************************************************************/

#include "scheduler.h"

job_t *get_next_job(int mode, d_linked_list_t* jobs) {
	if (mode == 0){
		job_t *j = dequeue(jobs);
		return j;
	}
	else if(mode == 1){
		job_t *j = pop(jobs);
		return j;
	}
	else if(mode == 2){
		int i;
		job_t *l;
		job_t *j = dequeue(jobs);
		for(i=0; i< jobs->size; i++){
			l = dequeue(jobs);
			if(l->required_time < j->required_time){
				enqueue(jobs, (void *) j);
				j = l;
			}
			else{
				enqueue(jobs, (void *) l);
			}
		}
		return j;
	}
	else{
		job_t *j = dequeue(jobs);
		return j;
	}
}
