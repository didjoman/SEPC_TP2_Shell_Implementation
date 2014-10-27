
#ifndef __JOBS_H
#define __JOBS_H

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <string.h>

// Structure servant à stocker la liste des processus lancés en background.
struct job{
    pid_t pid;
    char* cmd;
    struct timeval time;
    struct job* next;
};

void add_job(pid_t pid,char** cmd);
struct job* get_job(pid_t pid);
struct job* remove_job(pid_t pid);
void free_job(struct job** j);
void print_jobs(void);
void update_list_of_jobs(void);

#endif