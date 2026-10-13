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
  int done=0;
  int bytes_read=0;
  while ((bytes_read=read(fd1, buffer + done, MAX_PATH_SIZE - done))>0) {
    bytes_written=write(fd2, buffer+done, MAX_PATH_SIZE -done);
    if (bytes_written<0) {
      perror("write error");
      return 1;
    }
    done+=bytes_written;
  }
  close(fd1);
  close(fd2);
  return 0;
}

int copiadiretoria(const char *origem, const char *destino) {
  DIR *d= mkdir(destino);
	DIR *dirp = opendir(origem);
  if (dirp == NULL) {
		perror("opendir failed");
		return -1;
	}
	dp=readdir(dirp);
  if (lstat(dp)==-1) {
    return copiadiretoria(const char* dp, const char* destino);
  }
  return copiaficheiro(const char* dp, const char* destino);
}
