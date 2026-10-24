#define _XOPEN_SOURCE 700
#include <dirent.h>
#include "constants.h"
#include "filesystem.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int path_exists(const char *path){
  struct stat st;

  if (stat(path, &st) != 0)
    return 0;

  return S_ISDIR(st.st_mode);
}

int file_exists(const char *path){
  struct stat st;

  if (stat(path, &st) != 0)
    return 0;

  return S_ISREG(st.st_mode);
}

int absolute_path(const char *path, char *buffer, size_t size){
  char *resolved = realpath(path, NULL);

  if (resolved == NULL)
    return 1;

  if (strlen(resolved) >= size) {
    free(resolved);
    return 1;
  }

  strcpy(buffer, resolved);

  free(resolved);
  return 0;
}

int copiaficheiro(const char *origem, const char *destino) {
  struct stat fich_inf; // guarda informação do ficheiro

  // preenche o fich_inf com a informação no ficheiro origem
  if (stat(origem,&fich_inf) != 0) {
    perror("Erro em obter o stat do ficheiro");
    return 1;
  }
  int fd1 = open(origem, O_RDONLY);
  if(fd1 == -1){
    perror("Erro ao abrir o ficheiro de origem");
    return 1;
  }
  // operação bit a bit que otém as permissões do ficheiro
  int fd2 = open(destino, O_CREAT | O_TRUNC | O_WRONLY, fich_inf.st_mode & 0777);
  if (fd2 == -1) {
    perror ("Erro ao abrir ficheiro de destino");
    return 1;
  }
  char buffer[MAX_PATH_SIZE];
  long int bytes_read = 0;
  long int bytes_written = 0;
  while ((bytes_read = read(fd1, buffer, MAX_PATH_SIZE)) > 0) {
    long int done = 0;
    // em caso de o disco ficar sem espaço em que escreve menos do que lê 
    while(done < bytes_read){
    bytes_written = write(fd2, buffer + done, (size_t) (bytes_read - done));
    if (bytes_written < 0) {
      perror("erro de escrita");
      close(fd1);
      close(fd2);
      return 1;
    }
    done += bytes_written;
    }
  }
  // se a operação read falhar
  if(bytes_read < 0){
    perror("erro de leitura");
    close(fd1);
    close(fd2);
    return 1;
  }
  close(fd1);
  close(fd2);
  return 0;
}

int copiadiretoria(const char *origem, const char *destino) {
  // cria uma diretoria com as permissões adequadas
  if(mkdir(destino,0755) != 0 && errno != EEXIST){
    perror("erro ao criar diretoria de destino");
    return 1;
  }
  struct dirent *entry;
	DIR *dp = opendir(origem); // abertura da diretoria de origem
  char nova_origem[MAX_PATH_SIZE];
  char novo_destino[MAX_PATH_SIZE];
  if (dp == NULL) {
		perror("opendir failed");
		return 1;
	}
	while((entry = readdir(dp)) != NULL){
    if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name,"..") == 0) // ignora a diretoria do pai e do filho
      continue;
    snprintf(nova_origem, sizeof(nova_origem), "%s/%s", origem, entry->d_name); // cria novo caminho de origem  
    snprintf(novo_destino,sizeof(novo_destino),"%s/%s",destino, entry->d_name); // cria novo caminho de destino
  
    struct stat st;
    if(lstat(nova_origem,&st) == -1){
      perror("erro ao executar lstat");
      // fecha a diretoria uma vez que houve erro
      closedir(dp);
      return 1;
    }
    if(S_ISDIR(st.st_mode)) // se for uma diretoria chama a função recursivamente
      copiadiretoria(nova_origem,novo_destino);
    else if (S_ISREG(st.st_mode)) // se for um ficheiro chama a função copiaficheiro
      copiaficheiro(nova_origem,novo_destino);
  }
  closedir(dp); // fecha a diretoria em caso de sucesso
  return 0;

}