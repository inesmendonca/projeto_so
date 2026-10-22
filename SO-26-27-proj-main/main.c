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
    fprintf(stderr, "Usage: %s <servers> <ram> <disk> <cpus> <INPUT_DIR> <cores-da-maquina-local-seleccionados-para-o-CloudIST> <MAX_THREADS>\n", argv[0]);
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
			parse_double_arg(argv[6], &cores) != 0 ||
			parse_size_t_arg(argv[7],&max_threads) != 0) {
		fprintf(stderr, "Invalid command line arguments.\n");
		return 1;
	}

	if(max_threads == 0){
		fprintf(stderr,"o máximo de threads deve ser um inteiro positivo\n");
		return 1;
	}
	DIR *dirp;
	struct dirent *dp;
	dirp = opendir(argv[5]);
	if (dirp == NULL) {
		fprintf(stderr,"opendir failed\n");
		return 1;
	}
	size_t count = 0;
	char **lista = NULL;
	while ((dp = readdir(dirp)) != NULL) {
		if (verifica_extensao(dp->d_name, ".conf")) {
			char **tmp = realloc(lista, (count + 1)*sizeof(char*));
			if(tmp == NULL)
				return 1;
			lista = tmp;
			lista[count]=strdup(dp->d_name);
			count++;
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

	Thread *args= malloc(sizeof(Thread));
	args->input_dir=argv[5];
	args->conf_files=lista;
	args->total_files=sizeof(lista);
	args->ind_curr=0;
	args->dc=&dc;
	pthread_mutex_init(&args->mutex_t,NULL);

	pthread_t threads[max_threads];
	for (size_t i=0; i<max_threads; i++) {
		if(pthread_create(&threads[i],0,do_thread,&args)!=0) {
			exit(1);
		};
	}
	for (size_t i=0; i<max_threads;i++) {
		pthread_join (threads[i], NULL);
	}
	pthread_mutex_destroy(&args->mutex_t);
	free(args);

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
	
	free(lista);
	datacenter_destroy(&dc);
	return 0;
}