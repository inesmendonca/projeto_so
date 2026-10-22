#include "auxiliate_conf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datacenter.h"
#include <fcntl.h>
#include <unistd.h>
#include "parser.h"
#include "constants.h"

int verifica_extensao(const char * str, const char * ext) {
	size_t l1, l2;
	l1=strlen(str);
	l2=strlen(ext);
	if (l1 < l2) {
		return 0;
	}
	else {
		return strcmp(str+ (l1 - l2),ext)==0;
	}
} 

int comparar_nomes(const void *a, const void *b) {
	const char *const *pa = a; //conversão implícita de void*
	const char *const *pb = b;
	return strcmp(*pa,*pb);
}

void *do_thread(void *arg){
	// criação de um ponteiro para a estrutura Thread
	Thread *args = (Thread *) arg;
	DataCenter *dc = args->dc;
	size_t file_idx = 0; 
	

	//processamento dos comandos de cada ficheiro
	while(1){
		//necessário fazer lock e unlock, já que nos encontramos numa secção crítica
		pthread_mutex_lock(&args->mutex_t);
		if(args->ind_curr < args->total_files){
			// atualização do índice do ficheiro-secção crítica
			file_idx = args->ind_curr ++;
			pthread_mutex_unlock(&args->mutex_t);
		}
		else{
			//não existem mais ficheiros para processar
			pthread_mutex_unlock(&args->mutex_t);
			break;
		}
		//construir o caminho completo do ficheiro.conf
		char file_path[MAX_PATH_SIZE];
		snprintf(file_path,sizeof(file_path),"%s/%s",args->input_dir,args->conf_files[file_idx]);
		int fd = open(file_path,O_RDONLY);
		if(fd == - 1){
			perror("Erro ao abrir ficheiro de extensão .conf");
			continue; //tenta processar outros ficheiros
		}
		int processing = 1;
		while(processing){
			switch (get_next_command(fd)){
					case CMD_DEFINE: {
						VMType vmtype;

						if (parse_define(fd, &vmtype) != 0) {
							fprintf(stderr, "Invalid define command. See H (help) for usage.\n");
							continue;
						}

						if(datacenter_define_VM(dc, &vmtype) != 0){
							fprintf(stderr, "Failed to define VM.\n");
							continue;
						}

						printf("VM successfully defined!\n");

						break;
					}

					case CMD_RESERVE: {
						Reservation reservation = {0};

						size_t num_items = parse_reserve(fd, &reservation, MAX_RESERVATIONS_ITEMS);

						if (num_items == 0) {
							fprintf(stderr, "Invalid reserve command. See H (help) for usage.\n");
							continue;
						}

						if (datacenter_reserve(dc, &reservation) != 0) {
							fprintf(stderr, "Failed to reserve VMs.\n");
							continue;
						}

						printf("Reservation made successfully!\n");

						break;
					}

					case CMD_EXECUTE:
						char id[MAX_STRING_SIZE];

						if(parse_execute(fd, id) != 0){
							fprintf(stderr, "Invalid execute command. See H (help) for usage.\n");
							continue;
						}

						if (datacenter_execute(dc, id, args->cores) != 0) { //alterei os argumentos
							fprintf(stderr, "Failed to execute reservation.\n");
							continue;
						}

						printf("Finished reservation execution!\n");

						break;

					case CMD_LIST:
						if (datacenter_list(dc) != 0) {
							fprintf(stderr, "Failed to list VMs.\n");
							continue;
						}

						break;

					case CMD_WAIT:
						unsigned int delay;

						if(parse_wait(fd, &delay) != 0){
							fprintf(stderr, "Invalid wait command. See H (help) for usage.\n");
							continue;
						}

						datacenter_wait(delay);
						break;

					case CMD_INVALID:
						fprintf(stderr, "Invalid Command. See H (help) for usage.\n");
						break;

					case CMD_HELP:
						printf(
							"Spaces between arguments are allowed, but not after command end.\n"
							"Available commands:\n"
							" D <VM_TYPE_ID> <INPUT_FOLDER> <EXECUTABLE_PATH> <RAM_NEEDED> <DISK_NEEDED> <VCPU_NEEDED_COUNT>\n"
							" R <RESERVATION_ID> [<VM_TYPE_ID> <COUNT> <SERVER_ID>]+\n"
							" A <RESERVATION_ID>\n"
							" L\n"
							" E <DELAY_MS>\n"
							" H\n"
						);
						break;

					case CMD_EMPTY:
						break;

					case EOC:
						processing = 0;
						break;
				}
			
		}

			close(fd);
		}
			return NULL;
	}