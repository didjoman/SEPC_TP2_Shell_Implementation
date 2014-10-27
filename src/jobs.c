#include "jobs.h"

struct job* jobs = NULL;

/*
Transforme un tableau de chaines de caractère en une chaine de caractères.
*/
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

/*
    Ajoute un job à la liste des jobs
    @param: pid = pid du job à ajouter
    @param: cmd = commande exécutée par le processus 
*/
void add_job(pid_t pid,char** cmd)
{
    struct job* new_job = (struct job*) malloc(sizeof(struct job));
    if(!new_job){
        errno = ENOMEM;
        perror(0);
        exit(EXIT_FAILURE);
    }

    new_job->pid = pid;
    new_job->next = jobs;
    gettimeofday(&new_job->time, NULL);

    // On fait une copie de cmd
    new_job->cmd = get_complete_cmd(cmd);


    jobs = new_job;
}

/*
    Recherche un job à partir de son pid, dans la liste de jobs.
    @param: pid = pid du job à rechercher dans la liste.
*/
struct job* get_job(pid_t pid)
{
    // On recherche l'élément
    struct job *tmp = jobs;
    while(tmp && tmp->pid != pid)
        tmp = tmp->next;

    return tmp;
}

/*
    Retire un job à la liste des jobs (sans libérer la mémoire affectée)
    @param: pid = pid du job à retirer de la liste.
*/
struct job* remove_job(pid_t pid)
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

/*
    Libère la mémoire d'un pointeur vers un job
    @param: j: le job à désallouer
*/
void free_job(struct job** j){
    free((*j)->cmd);
    // tmp->next MUST BE NULL
    free(*j);
}

/*
    Affiche la liste des jobs, avec pour chaque job :
    - son pid
    - la commande exécutée
*/
void print_jobs(void)
{
    struct job *tmp = jobs;
    while(tmp){
        printf("%d ", tmp->pid);
        fflush(stdout);
        //printf("%lld ", (long long)tmp->time.tv_sec);
        printf("%s\n", tmp->cmd);
        tmp = tmp->next;
    }   
}

/*
    Retire de la liste des jobs les processus qui se sont terminés.
*/
void update_list_of_jobs (void)
{
    int status;
    pid_t end_pid;

    do{
        end_pid = waitpid(WAIT_ANY, &status, WUNTRACED|WNOHANG);

        // Pas de processus fils à attendre
        if (end_pid == -1)
            return;

        // Processus fils non encore terminé
        if (end_pid == 0)
            return;

        // Affichage d'un message spécifique en fonction du statut d'arrêt 
        // du processus fils :
        if (WIFEXITED(status)) {
            printf("child process (%d) terminated normally\n", end_pid);                   
        } else if (WIFSIGNALED(status)) {
            printf("child process was terminated by a signal \
                   (%d)\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("Child process was stopped by delivery of \
                    a signal (%d)\n", WSTOPSIG(status));
        }

        // On retire le processus terminé de la liste des jobs
        struct job* job_removed = remove_job(end_pid);
        if(job_removed != NULL)
            free_job(&job_removed);

    }  while (!WIFEXITED(status) && !WIFSIGNALED(status) 
              && !WIFSTOPPED(status));
}
