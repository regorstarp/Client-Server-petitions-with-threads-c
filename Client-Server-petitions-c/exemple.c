#include <pthread.h>
#include <stdio.h>
#define NUM_THREADS 5

// (RAW) void* pthread_function(void* thread_number)
void* pthread_function(int thread_id) {
	printf("I'm thread number %d\n",thread_id);
	pthread_exit(NULL);
}
int main(int argc,char** argv) {
// Create threads
	long int i;
	pthread_t thread[NUM_THREADS];
	for (i=0;i<NUM_THREADS;++i) {
	// (RAW) pthread_create(thread+i,NULL,pthread_function,i);
		pthread_create(thread+i,NULL,(void* (*)(void*))pthread_function,(void*)(i));
	}
	// Wait for threads
	for (i=0;i<NUM_THREADS;++i) {
		pthread_join(thread[i],NULL);
	}
	return 0;
}