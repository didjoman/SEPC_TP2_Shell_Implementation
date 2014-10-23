#ifndef __EXECCMD_H
#define __EXECCMD_H

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

 #include "readcmd.h"

int execute_ligne_commande(struct cmdline* cmd);

#endif
