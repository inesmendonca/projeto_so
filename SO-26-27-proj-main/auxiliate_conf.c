#include "auxiliate_conf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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