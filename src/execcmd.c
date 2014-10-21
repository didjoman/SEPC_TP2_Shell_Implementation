#include "execcmd.h"
#include "readcmd.h"
#include <string.h>

struct job{
    pid_t pid;
    char* cmd;
    struct job* next;
};

struct job* jobs = NULL;

static char* get_complete_cmd(char ** cmd)
{
    char* complete_cmd = NULL;
    char** cmd_part = cmd;
    bool first_time = true;
    while(*cmd_part){
        // Definition de la nouvelle taille
        int size = (first_time) ? 
                strlen(*cmd_part) + 1 : 
                strlen(complete_cmd) + 1 + strlen(*cmd_part) + 1;
        // Allocation de la mémoire nécessaire
        char* tmp = realloc(complete_cmd, size);
        if(tmp)
            complete_cmd = tmp;
        // Copie du "morceau de commande" dans le nom de commande
        if(first_time)
            sprintf(complete_cmd, "%s", *cmd_part);
        else
            sprintf(complete_cmd, "%s %s", complete_cmd, *cmd_part);
        ++cmd_part;
        first_time = false;
    }

    return complete_cmd;

}

static void add_job(pid_t pid,char** cmd)
{
    struct job* new_job = (struct job*) malloc(sizeof(struct job));
    if(!new_job){
        errno = ENOMEM;
        perror(0);
        exit(EXIT_FAILURE);
    }

    new_job->pid = pid;
    new_job->next = jobs;

    // On fait une copie de cmd
    new_job->cmd = get_complete_cmd(cmd);


    jobs = new_job;
}

static struct job* remove_job(pid_t pid)
{
    // On recherche l'élément à supprimer
    struct job *last_one = NULL;
    struct job *tmp = jobs;
    while(tmp && tmp->pid != pid){
        last_one = tmp;
        tmp = tmp->next;
    }   

    // Si on l'a trouvé on l'enlève.
    if(tmp){
        if(last_one)
            last_one->next = tmp->next;
        else 
            jobs = tmp->next;
        
        tmp->next = NULL;   
    }

    return tmp;
}

static void free_job(struct job** j){
    free((*j)->cmd);
    // tmp->next MUST BE NULL
    free(*j);
}


static void print_jobs(void)
{
    struct job *tmp = jobs;
    while(tmp){
        printf("%d ", tmp->pid);
        fflush(stdout);
        printf("%s\n", tmp->cmd);
        tmp = tmp->next;
    }   
}


void update_list_of_jobs (void)
{
    int status;
    pid_t end_pid;

    do{
        end_pid = waitpid(-1, &status, WUNTRACED|WNOHANG);

        // No child to wait for.
        if (end_pid == -1)
            return;

        // Traitements spécifique en fonction du statut d'arrêt 
        // du processus fils :
        if (WIFEXITED(status)) {
            printf("child process (%d) terminated normally\n", end_pid);

            struct job* job_removed = remove_job(end_pid);
            if(job_removed != NULL)
                free_job(&job_removed);
                    
        } else if (WIFSIGNALED(status)) {
            printf("child process was terminated by a signal \
                   (%d)\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("Child process was stopped by delivery of \
                    a signal (%d)\n", WSTOPSIG(status));
        }

    }  while (!WIFEXITED(status) && !WIFSIGNALED(status));
}


static int execute_commande(char** cmd)
{ 
    if(cmd[0] == NULL)
        return false;

    execvp(cmd[0], cmd);
    perror("Fail of execvp");
    exit(EXIT_FAILURE);
}


int exec_cmd(struct cmdline * cmd)
{
        if(cmd->seq[0]== NULL)
            return true;

        pid_t pid = fork();  

        switch(pid){
        // Problème lors du fork
        case -1 :
            perror("problem while forking ...");
            exit(EXIT_FAILURE);
        
        // Processus fils (execute les commandes)
        case 0: 
            for (int i=0; cmd->seq[i]!=0; i++) {
                if(!strcmp(*cmd->seq[i], "jobs"))
                    print_jobs();
                else if(execute_commande(cmd->seq[i]) == -1){
                    //libere(result);
                    perror("command not found.");
                    return false; /*-1*/
                }
            }
        // Processus père        
        default:
            // Execution avec &, on n'attend pas la terminaison du fils.
            if (cmd->bg){
                add_job(pid, cmd->seq[0]);
                return true;
            }

            // Execution sans &, on attend la terminaison du fils.
            wait(NULL);
        }
        
        return true;
}