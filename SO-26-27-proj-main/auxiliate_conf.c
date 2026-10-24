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
	if (l1 < l2) { // se o nome do ficheiro é menor do que o comprimento da extensão, não é da extensão pretendida
		return 0;
	}
	else {
		return strcmp(str+ (l1 - l2),ext)==0; //compara a string com o sufixo, consumindo os carateres da string 
	}
} 

int comparar_nomes(const void *a, const void *b) {
    /** como o qsort passa ponteiros para elementos do array, cada elemento é char*.
    Assim, como *a e *b são do tipo char*, é necessário transformá-los em ponteiros de string (const char*) */
	const char *const *pa = a;
	const char *const *pb = b;
	return strcmp(*pa,*pb); //compara as duas strings
}

void *do_thread(void *arg){
	// criação de um ponteiro para a estrutura Thread
	Thread *args = (Thread *) arg;
	DataCenter *dc = args->dc;
	size_t file_idx = 0; //indice a percorrer o ficheiro inicializado a 0
	

	//processamento dos comandos de cada ficheiro
	while(1){
		//necessário fazer lock e unlock para que cada ficheiro seja atribuído a uma só thread (secção crítica)
		pthread_mutex_lock(&args->mutex_t);
		if(args->ind_curr < args->total_files){ //percorre todos os ficheiros
			// atualização do índice do ficheiro em situação de lock para que apenas o indice do ficheiro em questão seja alterado
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
		while(processing){ //se o ficheiro foi aberto com sucesso
			switch (get_next_command(fd)){ //lê o inicio do ficheiro
					case CMD_DEFINE: { //se a linha lida corresponder ao comando D
						VMType vmtype;

						if (parse_define(fd, &vmtype) != 0) { //erro no comando
							fprintf(stderr, "Invalid define command. See H (help) for usage.\n");
							continue;
						}

						if(datacenter_define_VM(dc, &vmtype) != 0){ //erro no comando
							fprintf(stderr, "Failed to define VM.\n");
							continue;
						}

						printf("VM successfully defined!\n"); //sucesso

						break;
					}

					case CMD_RESERVE: { //se a linha lida corresponder ao comando R
						Reservation reservation = {0}; //nova reserva

                         //realiza a reserva e retorna o número de itens na reserva
						size_t num_items = parse_reserve(fd, &reservation, MAX_RESERVATIONS_ITEMS); 

						if (num_items == 0) { //erro na reserva
							fprintf(stderr, "Invalid reserve command. See H (help) for usage.\n");
							continue;
						}

						if (datacenter_reserve(dc, &reservation) != 0) { //erro na reserva
							fprintf(stderr, "Failed to reserve VMs.\n");
							continue;
						}

						printf("Reservation made successfully!\n"); //sucesso

						break;
					}

					case CMD_EXECUTE: { // se a linha lida corresponder ao comando A
						char id[MAX_STRING_SIZE]; //novo id

						if(parse_execute(fd, id) != 0){ //erro na execução
							fprintf(stderr, "Invalid execute command. See H (help) for usage.\n");
							continue;
						}

						if (datacenter_execute(dc, id, args->cores) != 0) { //erro na execução
							fprintf(stderr, "Failed to execute reservation.\n");
							continue;
						}

						printf("Finished reservation execution!\n"); //sucesso

						break;
                    }
					case CMD_LIST: { //sea linha lida corresponder ao comando L
						if (datacenter_list(dc) != 0) { //erro ao listar a reserva
							fprintf(stderr, "Failed to list VMs.\n");
							continue;
						}

						break;
                    }
					case CMD_WAIT: { //se a linha lida corresponder ao comando E
						unsigned int delay; //tempo de espera

						if(parse_wait(fd, &delay) != 0){ //erro
							fprintf(stderr, "Invalid wait command. See H (help) for usage.\n");
							continue;
						}

						datacenter_wait(delay);
						break;
                    }
					case CMD_INVALID: { //se a linha lida não corresponder a nenhum comando
						fprintf(stderr, "Invalid Command. See H (help) for usage.\n");
						break;
                    }
					case CMD_HELP: { //se a linha lida corresponder ao comando H
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
                    }
					case CMD_EMPTY: //se a linha lida começar com '#' ou '\n'
						break;

					case EOC: //a leitura deu erro ou a leitura foi concluída
						processing = 0;
						break;
				}
			
		}

			close(fd);
		}
			return NULL;
	}