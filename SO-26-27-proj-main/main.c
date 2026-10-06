#include <stdio.h>
#include <stdlib.h>
#include <String.h>
#include <unistd.h>
#include <dirent.h>
#include "parser.h"
#include "datacenter.h"
#include "constants.h"

int verifica_extensão(char str, char ext) {
	int l1, l2, l3;
	l1=strlen(str);
	l2=strlen(ext);
	l3=l1-l2;
	if (l3<0) {
		return -1;
	}
	else {
		return strcmp(str+l3,l2)==0;
	}
} 

int comparar_nomes(const void *a, const void *b) {
	const char *const *pa=a; //conversão mplícita de void*
	const char *const *pb=b;
	return strcmp(*pa,*pb);
}

int main(int argc, char **argv){
	DataCenter dc;
	datacenter_init(&dc);

	if (argc != 6) {
    fprintf(stderr, "Usage: %s <servers> <ram> <disk> <cpus> <INPUT_DIR>\n", argv[0]);
    return 1;
  }

	size_t servers;
	size_t ram;
	size_t disk;
	double cpu;
	size_t input_dir;

	if (parse_size_t_arg(argv[1], &servers) != 0 ||
			parse_size_t_arg(argv[2], &ram) != 0 ||
			parse_size_t_arg(argv[3], &disk) != 0 ||
			parse_double_arg(argv[4], &cpu) != 0 || 
			parse_size_t_arg(argv[5], &input_dir)!=0 ) {
		fprintf(stderr, "Invalid command line arguments.\n");
		return 1;
	}

	DIR *dirp;
	struct dirent *dp;
	dirp = opendir(argv[5]);
	if (dirp == NULL) {
		perror("opendir failed");
		return 1;
	}
	size_t count=0;
	while ((dp = readdir(dirp)) != NULL) {
		if (verifica_extensao(dp->d_name, ".conf")) {
			lista[count]=strdup(dp->d_name);
			count++;
		}
	}

	qsort(lista, count, sizeof(char*), comparar_nomes);
	// falta a cópia recursiva
	Resources resources = {
    .ram = ram,
    .disk = disk,
    .cpu = cpu
	};

	if(datacenter_configure(&dc, servers, &resources) != 0){
		fprintf(stderr, "Failed to configure Data Center.\n");
		return 1;
	}

	while(1){
		switch (get_next_command(STDIN_FILENO)){
			case CMD_DEFINE: {
				VMType vmtype;

				if (parse_define(STDIN_FILENO, &vmtype) != 0) {
					fprintf(stderr, "Invalid define command. See H (help) for usage.\n");
					continue;
				}

				if(datacenter_define_VM(&dc, &vmtype) != 0){
					fprintf(stderr, "Failed to define VM.\n");
					continue;
				}

				printf("VM successfully defined!\n");

				break;
			}

			case CMD_RESERVE: {
				Reservation reservation = {0};

				size_t num_items = parse_reserve(STDIN_FILENO, &reservation, MAX_RESERVATIONS_ITEMS);

				if (num_items == 0) {
					fprintf(stderr, "Invalid reserve command. See H (help) for usage.\n");
					continue;
				}

				if (datacenter_reserve(&dc, &reservation) != 0) {
					fprintf(stderr, "Failed to reserve VMs.\n");
					continue;
				}

				printf("Reservation made successfully!\n");

				break;
			}

			case CMD_EXECUTE:
				char id[MAX_STRING_SIZE];

				if(parse_execute(STDIN_FILENO, id) != 0){
					fprintf(stderr, "Invalid execute command. See H (help) for usage.\n");
					continue;
				}

				if (datacenter_execute(&dc, id) != 0) {
					fprintf(stderr, "Failed to execute reservation.\n");
					continue;
				}

				printf("Finished reservation execution!\n");

				break;

			case CMD_LIST:
				if (datacenter_list(&dc) != 0) {
					fprintf(stderr, "Failed to list VMs.\n");
					continue;
				}

				break;

			case CMD_WAIT:
				unsigned int delay;

				if(parse_wait(STDIN_FILENO, &delay) != 0){
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
				datacenter_destroy(&dc);
				return 0;
		}
	}
}