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
  struct stat fich_inf;
  if (stat(origem,&fich_inf)!=0) {
    perror("Erro em obter o stat do ficheiro");
    return 1;
  }
  int fd1= open(origem, O_RDONLY);
  int fd2= open(destino, O_CREAT | O_TRUNC | O_WRONLY, fich_inf.st_mode & 0777);
  if (fd1==-1 || fd2==-1) {
    perror ("Erro ao abrir ficheiro");
    return 1;
  }
  char buffer[MAX_PATH_SIZE];
  int bytes_read = 0;
  while ((bytes_read = read(fd1, buffer + done, MAX_PATH_SIZE ))>0) {
    int done = 0;
    while(done < bytes_read){
    bytes_written=write(fd2, buffer + done, bytes_read - done);
    if (bytes_written < 0) {
      perror("write error");
      return 1;
    }
    done += bytes_written;
    }
  }
  close(fd1);
  close(fd2);
  return 0;
}

int copiadiretoria(const char *origem, const char *destino) {
  if(mkdir(destino,0755) != 0 && errno != EEXIST){
    perror("erro ao criar diretoria de destino");
    return -1;
  }
	DIR *dp = opendir(origem);
  char nova_origem[MAX_PATH_SIZE];
  char novo_destino[MAX_PATH_SIZE];
  if (dp == NULL) {
		perror("opendir failed");
		return 1;
	}
	while((entry = readdir(dir)) > 0){
    if(strcmp(dp->d_name, ".") || strcmp(dp->d_name,".."))
      continue;
    snprintf(nova_origem, sizeof(nova_origem), "%s/%s", origem, dp->d_name);
    snprintf(novo_destino,sizeof(novo_destino),"%s/%s",destino, dp->d_name);
  
    struct stat st;
    if(lstat(nova_origem,&st) == -1){
      perror("erro ao executar stat");
      return 1;
      }
    if(S_ISDIR(st.st_mode))
      copiadiretoria(nova_origem,novo_destino);
    else if (S_ISREG(st.st_mode))
      copiaficheiro(nova_origem,novo_destino);
  }
  closedir(dp);
  return 0;

}