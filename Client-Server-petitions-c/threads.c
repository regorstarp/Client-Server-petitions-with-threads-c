#include <pthread.h> //threads
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h> //gettid()
#include <stdlib.h> //atoi() string -> int
#include <time.h>

#define TAM_BUFFER 5

pthread_mutex_t mutex;
pthread_cond_t buffer_semafor;
pthread_cond_t priority;


typedef struct {
	int id_cliente;
	int id_compra;
} peticion_t;

typedef struct {
	int pos_lectura;// Siguiente posicion a leer
	int pos_escritura;// Siguiente posicion a escribir
	int num_peticiones; // Número de peticiones
	peticion_t peticiones[TAM_BUFFER]; // Buffer
} buffer_peticiones_t;

buffer_peticiones_t buffer;
buffer_peticiones_t buffer_priority;

// Inicializa la estructura del buffer y sus centinelas @pos_lectura y @pos_escritura
void buffer_peticiones_inicializar(buffer_peticiones_t* buffer_peticiones){
	buffer_peticiones->pos_lectura = 0;
	buffer_peticiones->pos_escritura = 0;
	buffer_peticiones->num_peticiones = 0;
}

// Devuelve 1 si el buffer esta completamente lleno, 0 en caso contrario
int buffer_peticiones_lleno(buffer_peticiones_t* buffer_peticiones){
	if (buffer_peticiones->num_peticiones == TAM_BUFFER) return 1;
	return 0;
}

// Devuelve 1 si el buffer esta completamente vacio, 0 en caso contrario
int buffer_peticiones_vacio(buffer_peticiones_t* buffer_peticiones){
	if (buffer_peticiones->num_peticiones == 0) return 1;
	return 0;
}

// Encola @peticion en el buffer
void buffer_peticiones_encolar(buffer_peticiones_t* buffer_peticiones,peticion_t* peticion){
		buffer_peticiones->peticiones[buffer_peticiones->pos_escritura] = *peticion;
		buffer_peticiones->pos_escritura++;
		buffer_peticiones->pos_escritura %= TAM_BUFFER; 
		buffer_peticiones->num_peticiones++;
}

// Retira del buffer la peticion mas antigua (FIFO) y la copia en @peticion
void buffer_peticiones_atender(buffer_peticiones_t* buffer_peticiones,peticion_t* peticion){
		*peticion = buffer_peticiones->peticiones[buffer_peticiones->pos_lectura];
		buffer_peticiones->pos_lectura++;
		buffer_peticiones->pos_lectura %= TAM_BUFFER; 
		buffer_peticiones->num_peticiones--;
}

void servidor_atendre(buffer_peticiones_t* buffer_peticiones,peticion_t* peticion){
	pthread_mutex_lock(&mutex);
	while (buffer_peticiones_vacio(buffer_peticiones)){
		pthread_cond_wait(&buffer_semafor,&mutex);
	}
	buffer_peticiones_atender(buffer_peticiones, peticion);
	pthread_cond_signal(&buffer_semafor);
	pthread_mutex_unlock(&mutex);
}

void client_encolar(buffer_peticiones_t* buffer_peticiones,peticion_t* peticion){
	pthread_mutex_lock(&mutex);
	while (buffer_peticiones_lleno(buffer_peticiones)){
		pthread_cond_wait(&buffer_semafor,&mutex);
	}
	buffer_peticiones_encolar(buffer_peticiones, peticion);
	pthread_cond_signal(&buffer_semafor);
	pthread_mutex_unlock(&mutex);
}



void* servidor_thread(int thread_id) {
	FILE*  compres;
	compres = fopen("compres.txt", "w+");
	printf("Soc el thread servidor\nLa meva id és %d\nEl meu TID és %d\nEl meu PID és %d\n", thread_id, (int)syscall(SYS_gettid) ,getpid());    
	peticion_t peticio_server;
    while (1){
    	if (!buffer_peticiones_vacio(&buffer_priority)){
 			servidor_atendre(&buffer_priority, &peticio_server);
 			fprintf(compres, "$ Client VIP $: %d Compra: %d", peticio_server.id_cliente, peticio_server.id_compra);
			fprintf(compres, "\n"); 		
    	}
    	else {
    		servidor_atendre(&buffer, &peticio_server);
    		fprintf(compres, "Client Normal: %d Compra: %d", peticio_server.id_cliente, peticio_server.id_compra);
			fprintf(compres, "\n");
    	}
    	
    	
    	if (peticio_server.id_cliente == 0 && peticio_server.id_compra == 0){
    		break;
    	}
    }	    
	fclose(compres);		
	pthread_exit(NULL);
}

void* client_thread(int thread_id) {
	printf("Soc el thread client\nLa meva id és %d\nEl meu TID és %d\nEl meu PID és %d\n", thread_id, (int)syscall(SYS_gettid), getpid());
	/*creacio peticions*/
	peticion_t mi_peticion;
	srand(time(NULL)+ thread_id);
	for (int b = 1; b <= 100; b++){
		//creem la petició
		int rnd;
		mi_peticion.id_cliente = thread_id;
		mi_peticion.id_compra = b;
		rnd = rand()%2;
		if (rnd == 1){ //encolar aleatòriament les peticions en els buffers
			client_encolar(&buffer_priority, &mi_peticion);
		}	
		else {
			client_encolar(&buffer, &mi_peticion);
		}
	}
	pthread_exit(NULL);
}

int main(int argc, char** argv){
	if (argc != 2){
		printf("Error: %s num_clients", argv[0]);
		return 1;
	}
	int num_clients = atoi(argv[1]);
	long int i = 0;
	pthread_t * client = malloc(num_clients*sizeof(pthread_t));
	pthread_t servidor;
	//inicialització del buffer
	buffer_peticiones_inicializar(&buffer);
	buffer_peticiones_inicializar(&buffer_priority);
	/*inicialització del mutex*/
	pthread_mutex_init(&mutex,NULL);
	//inicialització variables condició
	pthread_cond_init(&buffer_semafor,NULL);
	pthread_cond_init(&priority,NULL);
	/*creació threads client i servidor*/
	pthread_create(&servidor, NULL, (void* (*) (void*)) servidor_thread, (void*)(i));
	for (i = 1; i <= num_clients; ++i){
		pthread_create(client +i, NULL, (void* (*) (void*)) client_thread, (void*)(i));
	}		
	/*wait threads*/
	for (i = 1; i <= num_clients; ++i){
		pthread_join(client[i],NULL);
	}
	peticion_t peticio_final;
	peticio_final.id_cliente = 0;
	peticio_final.id_compra = 0;
	client_encolar(&buffer, &peticio_final);
	pthread_join(servidor,NULL);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&buffer_semafor);
	pthread_cond_destroy(&priority);
	free(client);
	return 0;
}