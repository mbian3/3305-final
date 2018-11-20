/******************************************************************************
*
* Name: 	Zaid Albirawi
* Email: 	zalbiraw@uwo.ca
*
* simulate.c
*
******************************************************************************/

#include "simulate.h"

int memory, max_memory, mode, time_quantum;
int flag =0;
FILE *fp;
d_linked_list_t *jobs;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;

void* run(void *j)
{
	job_t *job = get_next_job(mode, jobs);
	int number, required_memory;

	while (job != NULL)
	{
		number = job->number;
		required_memory = job->required_memory;

		/**********************************************************************
		* checks if the memory requested exceeds maximum memory
		**********************************************************************/
		if (required_memory > max_memory)
		{
			/******************************************************************
			* inform user that the job won't run, clean and terminate
			******************************************************************/
			print_exceed_memory(fp, number);
			free(job);
			return NULL;
		}

		/**********************************************************************
		* runs job
		**********************************************************************/
		if (required_memory <= memory) execute_job(job);

		/**********************************************************************
		* checks if the memory requested exceeds current available memory
		**********************************************************************/
		else
		{
			/******************************************************************
			* inform user that the job doesn't have enough resources at the
			* moment, add the job back to the list
			******************************************************************/
			print_insufficient_memory(fp, number);

			enqueue(jobs, job);
		}

		job = get_next_job(mode, jobs);
		sleep(time_quantum); //Ensures another thread gets a chance to enter the critical section before this one does again
	}
	return NULL;

}


/******************************************************************************
*
******************************************************************************/
void simulate(int memory_value, int mode_value, int time_quantum_value,
	d_linked_list_t *list)
{
	/**************************************************************************
	* opens output file
	**************************************************************************/
	fp = fopen(SYSTEM_OUTPUT, WRITE);

	/**************************************************************************
	* check if the system file is in the directory
	**************************************************************************/
	if (fp == NULL)
	{
		printf("Unable to open %s file\n", SYSTEM_OUTPUT);
		exit(FAILURE);
	}

	/**************************************************************************
	* set global variables
	**************************************************************************/
	memory = max_memory = memory_value;

	/**************************************************************************
	* set executing mode
	**************************************************************************/
	mode = mode_value;
	time_quantum = time_quantum_value;
	print_mode(fp, mode);
	jobs = list;

	/**************************************************************************
	* create threads and run jobs
	**************************************************************************/
	pthread_t threads[NUMBER_OF_THREADS];
	for (int i = 0; i < NUMBER_OF_THREADS; ++i)
	{
		pthread_mutex_lock( &mutex3); //added line
		if (pthread_create(&threads[i], NULL, run, NULL))
		{
			printf("Error: failed to create thread.\n");
			exit(FAILURE);
		}
	}

	/**********************************************************************
	* wait for the jobs to finish executing
	**********************************************************************/
	for (int i = 0; i < NUMBER_OF_THREADS; ++i)
		pthread_join(threads[i], NULL);

}

void execute_job(job_t *job) {
	int number = job->number,
		required_memory = job->required_memory;

	/******************************************************************
	* inform user that the job started executing and allocate mrmory
	******************************************************************/

	print_starting(fp, number);
	allocate_memory(required_memory);
	pthread_mutex_unlock( &mutex3 );	//Protects multiple memory checks above


	/******************************************************************
	* run the job
	******************************************************************/
	pthread_mutex_lock( &mutex1 );	//added line
	if(mode !=3 || (job->required_time <= time_quantum)){
		sleep(job->required_time);


		/******************************************************************
		* inform user that the job finished executing
		******************************************************************/
		print_completed(fp, number);
		free(job);

		/******************************************************************
		* deallocate memory
		******************************************************************/
		deallocate_memory(required_memory);
	}
	else{
		sleep(time_quantum);
		int difference = job->required_time - time_quantum;
		job_t *k = init_job(job->number, job->required_memory, difference);
		enqueue(jobs, (void *)k);
		free(job);
		deallocate_memory(required_memory);
	}
	pthread_mutex_unlock( &mutex1 );	//ensures only one simulated process can run at a time
}



void allocate_memory(int r) {
	memory -= r;
	print_allocate_memory(fp, memory, r);
}

void deallocate_memory(int r) {
	memory += r;
	print_deallocate_memory(fp, memory, r);
}
