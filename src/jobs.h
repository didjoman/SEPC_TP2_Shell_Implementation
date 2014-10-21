
#ifndef __JOBS_H
#define __JOBS_H

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void add_job(pid_t pid,char** cmd);
struct job* remove_job(pid_t pid);
void free_job(struct job** j);
void print_jobs(void);
void update_list_of_jobs(void);

#endif