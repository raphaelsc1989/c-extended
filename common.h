#ifndef _COMMON_
#define _COMMON_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define STRING_TYPE "string\0"
#define DIRECTIVE_FOUND 1
#define OVERFLOW_DETECTED 2

struct symbols {
	char symbol_name[32];
	struct symbols *next;
};

#endif