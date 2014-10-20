#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

 #include "readcmd.h"

void init_jobs();
int exec_cmd(struct cmdline * cmd);