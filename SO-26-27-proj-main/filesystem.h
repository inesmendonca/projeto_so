#ifndef FILESYSTEM__H
#define FILESYSTEM__H

#include <stddef.h>

/**
 * Checks whether a path exists and is a directory.
 *
 * @param path Directory path.
 *
 * @return 1 if it exists and is a directory, 0 otherwise.
 */
int path_exists(const char *path);

/**
 * Checks whether a path exists and is a regular file.
 *
 * @param path File path.
 *
 * @return 1 if it exists and is a regular file, 0 otherwise.
 */
int file_exists(const char *path);

/**
 * Converts the given path into an absolute path, resolving symbolic links,
 * relative components ('.' and '..'), and redundant separators. The resolved
 * path is copied into the provided buffer.
 *
 * @param path Path to resolve.
 * @param buffer Destination buffer where the absolute path will be stored.
 * @param size Size of the destination buffer, in bytes.
 *
 * @return 0 if the path was successfully resolved and copied to the buffer
 * @return 1 if the path could not be resolved or the buffer is too small.
 */
int absolute_path(const char *path, char *buffer, size_t size);

/**
 * Lê a diretoria de origem e abre subdiretorias até encontrar ficheiros.
 * Ao encontrar ficheiros, copia, através do copiaficheiro o seu conteúdo para a diretoria de destino.
 *
 * @param origem diretoria de origem
 * @param destino diretoria de destino
 *
 * @return 0 em caso de sucesso, 1 em caso de erro
 */
int copiadiretoria(const char *origem, const char *destino);


/**
 * Copia o conteúdo do ficheiro de origem para o ficheiro de destino.
 *
 * @param origem ficheiro de origem
 * @param destino fichiero de destino
 *
 * @return 0 em caso de sucesso, 1 em caso de erro
 */
int copiaficheiro(const char *origem, const char *destino);


#endif // FILESYSTEM__H