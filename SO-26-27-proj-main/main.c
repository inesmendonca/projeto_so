#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "parser.h"
#include "datacenter.h"
#include "auxiliate_conf.h"
#include "constants.h"
#include <fcntl.h>
#include <pthread.h>

int main(int argc, char **argv){

	DataCenter dc;
	datacenter_init(&dc);

	if (argc != 8) {
    fprintf(stderr, "Usage: %s <servers> <ram> <disk> <cpus> <cores-da-maquina-local-seleccionados-para-o-CloudIST> <INPUT_DIR> <MAX_THREADS>\n", argv[0]);
    return 1;
  }

	size_t servers;
	size_t ram;
	size_t disk;
	double cpu;
	double cores;
	size_t max_threads;

	if (parse_size_t_arg(argv[1], &servers) != 0 ||
			parse_size_t_arg(argv[2], &ram) != 0 ||
			parse_size_t_arg(argv[3], &disk) != 0 ||
			parse_double_arg(argv[4], &cpu) != 0 ||
			parse_double_arg(argv[5], &cores) != 0 ||
			parse_size_t_arg(argv[7],&max_threads) != 0) {
		fprintf(stderr, "Invalid command line arguments.\n");
		return 1;
	}

	if(max_threads == 0){ // se o número de threads for 0 dá erro
		fprintf(stderr,"o máximo de threads deve ser um inteiro positivo\n");
		return 1;
	}
	DIR *dirp; 
	struct dirent *dp; 
	dirp = opendir(argv[6]); // abre o INPUT_DIR

	if (dirp == NULL) { // erro ao abrir a diretoria
		fprintf(stderr,"opendir failed\n");
		return 1;
	}

	// construção da lista de ficheiros de extensão .conf
	size_t count = 0; 
	char **lista = NULL; 

	while ((dp = readdir(dirp)) != NULL) { // enquanto for possível ler a diretoria
		if (verifica_extensao(dp->d_name, ".conf")) { // se o ficheiro lido foi da extensão .conf
			char **tmp = realloc(lista, (count + 1)*sizeof(char*)); // aumenta o tamanho da lista para mais uma posição
			if(tmp == NULL)
				return 1;
			lista = tmp; // lista passa a apontar para o novo bloco realocado
			lista[count]=strdup(dp->d_name); // a nova posição guarda uma cópia do nome do ficheiro
			count++; // aumenta o número de elementos na lista
		}
	}
	closedir(dirp);
	// se não existirem ficheiros do tipo pretendido, a função não deve processar nada 
	if(count == 0){
		fprintf(stderr, "Não há ficheiros .conf presentes na diretoria\n");
		return 0;
	}
	else{ //ordenação por ordem alfabética caso existam ficheiros .conf
		qsort(lista, count, sizeof(char*), comparar_nomes);
	}
	//limitar o número de threads caso sejam superiores ao número de ficheiros presentes
	if(max_threads > count){
		max_threads = count;
	} 
	Resources resources = {
    .ram = ram,
    .disk = disk,
    .cpu = cpu
	};

	if(datacenter_configure(&dc, servers, &resources) != 0){
			fprintf(stderr, "Failed to configure Data Center.\n");
			for(size_t i = 0; i < count; i++){
				free(lista[i]);
			}
			free(lista);
			return 1;
		}
	
	Thread *args= malloc(sizeof(Thread)); // aloca memória para os argumenntos partilhados pelas threads
	// define os argumentos partilhados
	args->input_dir=argv[6]; 
	args->conf_files=lista;
	args->total_files=count;
	args->ind_curr=0;
	args->dc=&dc;
	args->cores = cores;
	pthread_mutex_init(&args->mutex_t,NULL); // inicializa o mutex que protege o ind_curr

	pthread_t threads[max_threads];
	for (size_t i=0; i<max_threads; i++) { 
		if(pthread_create(&threads[i],0,do_thread,args)!=0) { // cria uma thread
			exit(1);
		};
	}
	for (size_t i=0; i<max_threads;i++) {
		pthread_join (threads[i], NULL); // espera que a threads[i] termine
	}
	pthread_mutex_destroy(&args->mutex_t); // "destrói" a mutex
	free(args); // liberta a memória do args

	for(size_t i = 0; i < count; i++){
		free(lista[i]);
	}
	
	free(lista);
	datacenter_destroy(&dc); // "destrói o datacenter"
	return 0;
}