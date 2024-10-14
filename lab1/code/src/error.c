#include <unistd.h>
#include <stdlib.h>
#include "../include/functions.h"

#define STDERR 2

void error_and_exit(char *message, int error_code)
{
    write(STDERR, message, string_length(message));
    exit(error_code);
}

