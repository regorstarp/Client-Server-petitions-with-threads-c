#include <pthread.h> 
#include <stdio.h> 
#include <unistd.h>
#define NUM_THREADS 5
 // Mutex & CV
pthread_mutex_t mutex;
pthread_cond_t tickets_ready;
// Global counter
int tickets = 2;
// Thread function
void* pthread_function(int thread_id) {
	// IN (Get ticket)
	pthread_mutex_lock(&mutex);
	while (tickets==0) {
		pthread_cond_wait(&tickets_ready,&mutex);
    }
	--tickets;
	pthread_mutex_unlock(&mutex);
 	// DO STUFF ...
 	printf("Thread %d working\n",thread_id);
 	sleep(1);
 	// OUT (Release ticket)
 	pthread_mutex_lock(&mutex);
 	if (tickets >= 0){
 		pthread_cond_signal(&tickets_ready);
 	}
 	++tickets;
 	pthread_mutex_unlock(&mutex);
 	pthread_exit(NULL); // Exit
 }
 // Main
 int main(int argc,char** argv) {
 	// Init mutex & CV
 	pthread_mutex_init(&mutex,NULL);
 	pthread_cond_init(&tickets_ready,NULL);
 	// Create threads
 	long int i;
 	pthread_t thread[NUM_THREADS];
 	for (i=0;i<NUM_THREADS;++i) {
 		pthread_create(thread+i,NULL,(void* (*)(void*))pthread_function,(void*)(i));
 	}
 	// Wait for threads
 	for (i=0;i<NUM_THREADS;++i) {
 		pthread_join(thread[i],NULL);
 	}
 	pthread_mutex_destroy(&mutex);
 	pthread_cond_destroy(&tickets_ready);
 	return 0;
 }