#include "i_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

noreturn void
I_Error(char *error, ...) {
	//extern bool demorecording;
	va_list ap;

	va_start(ap, error);
	fprintf(stderr, "Error: ");
	vfprintf(stderr, error, ap);
	fputc('\n', stderr);
	va_end(ap);

/*
	if(demorecording)
		G_CheckDemoStatus();

	D_QuitNetGame();
*/

	exit(EXIT_FAILURE);
}
