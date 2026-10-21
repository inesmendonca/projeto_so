/**
 * Verifica se o ficheiro é do tipo indicado
 *
 * @param str ficheiro a analisar
 * @param ext tipo de extensão a analisar
 */
int verifica_extensao(const char * str, const char * ext); 

/**
 * Compara dois nomes de ficheiros com a finalidade de os ordenar alfabeticamente
 *
 * @param a primeiro ficheiro
 * @param b segundo ficheiro 
 */
int comparar_nomes(const void *a, const void *b) {
	const char *const *pa = a; //conversão implícita de void*
	const char *const *pb = b;
	return strcmp(*pa,*pb);
}