#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "functions.h"

int check_dig(const char *cadena){
	for (int i=0;i<strlen(cadena);i++){
		if (isdigit(cadena[i])==0)
			return 0;
	}
	return 1;
}

