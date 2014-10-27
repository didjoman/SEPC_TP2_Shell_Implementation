#include "execcmd.h"
#include "jobs.h"

/*  [OPTION : Temps de Calcul]
 *  Ce traitant est appelé à la réception d'un signal SIGCHLD par le processus 
 *  père.
 *  Il affiche la durée d'exécution du processus fils qui vient de se terminer.
 */
static void handler_sigchild (int sig, siginfo_t *siginfo, void *context)
{
        struct job* j = get_job(siginfo->si_pid);
        char time_string[15];

        // If the jobs is not found in the list of jobs : 
        if(!j)
        return;
        
        // We work out the elpased time between start and end of the process
        struct timeval time_interval;
        gettimeofday(&time_interval, NULL);
        
        time_interval.tv_sec -= j->time.tv_sec;
        time_interval.tv_usec -= (j->time.tv_usec / 1000000);
        
        // Formating of the string
        int ss = (int) (time_interval.tv_sec) % 60 ;
        int mm = (int) ((time_interval.tv_sec / 60) % 60);
        int hh = (int) ((time_interval.tv_sec / (3600)) % 24);
        // String containing the micro seconds :
        char tmp_micro_secs[19];
        sprintf(tmp_micro_secs, "%04lld", (long long) time_interval.tv_usec);
        char micro_secs[4];
        strncpy(micro_secs, tmp_micro_secs, 4);
        // String containing the whole elpased time on the format hh:mm:ss.ms
        sprintf(time_string, "%02d:%02d:%02d.%s", hh, mm, ss, micro_secs);
        time_string[14] = '\0';
        
        // Printing of the string : 
	printf ("%ld \"%s\", elapsed time: %s\n", 
                (long)j->pid, j->cmd, time_string);
        fflush(stdout);
        
        // Finally we remove the job from the list of jobs :
        /*
         ?? Why to do it here ??
         |-> The signal has been caught by this handler and will never be caught
         in the waitpid() of the function update_list_of_jobs() (called before 
         the prompt).
         The job will therefore never be removed from the list of jobs if we 
         don't do it now.
         */

        struct job* job_removed = remove_job(siginfo->si_pid);
        if(job_removed != NULL){
            free_job(&job_removed);
            print_jobs();
            printf("\n");
        }

        //while (waitpid((pid_t)(-1), 0, WNOHANG) > 0);
}

/*
   Execute la commande passée en paramètre, en appelant execvp.
   */
static int execute_commande(char** cmd)
{ 
        if(cmd[0] == NULL)
                return false;

        execvp(cmd[0], cmd);
        perror("Fail of execvp");
        exit(EXIT_FAILURE);
}

/*
   Créé un processus fils servant à lancer la commande passée en paramètre.
   Puis lance la commande dans le processus fils créé.
   Si !cmd->bg, Le processus père attend la fin de l'exécution du fils.
   */
int exec_cmd(int in, int out, struct cmdline * cmd, int i)
{

        pid_t pid = fork();  

        switch(pid) {
                // Problème lors du fork :
        case -1 :
                perror("problem while forking ...");
                exit(EXIT_FAILURE);
                break;
                // Processus fils (execute les commandes)
        case 0: 
                // Lance l'ensemble des commandes séparées par les pipes

                // Si il y a une autre commande avant
                if (in != 0) {
                        // Le file descriptor est maintenant une copie de in
                        dup2(in,0);
                        // On ferme in, car on en a fait une copie.
                        close(in);
                }

                // Si il y a une commande après
                if (out != 1) {
                        // Ferme "out" et le duplique sur le fd 1
                        dup2(out,1);
                        close(out);
                }

                // Cas particulier, "jobs" est exécutée par le shell
                if(!strcmp(*cmd->seq[i], "jobs"))
                        print_jobs();
                // Cas général
                else if(execute_commande(cmd->seq[i]) == -1){
                        perror("command not found.");
                        return false; /*-1*/
                }
                break;
                // Processus père (attends ou non la mort du fils)     
        default:

                //Si il y a eu une commande avant
                if (in != 0)
                        close(in);

                //Si il y a eu une commande après
                if (out != 1)
                        close(out);

                // Execution avec &, on n'attend pas la terminaison du fils.
                if (cmd->bg){
                        add_job(pid, cmd->seq[0]);
                        
                        // Temps de calcul d'un processus fils : 
                        struct sigaction sa;
                        sa.sa_sigaction = handler_sigchild;
                        sigemptyset(&sa.sa_mask);
                        sa.sa_flags = SA_SIGINFO; 
                        
                        if (sigaction(SIGCHLD, &sa, NULL) == -1){
                                perror ("sigaction");
                                exit(EXIT_FAILURE);
                        }
                        
                        return true;
                }
            
                // Execution sans &, on attend la terminaison du fils.
                /* Attention ! 0n attend dans deux cas :
                 *  - commande unique (sans pipe) lancée sans &
                 *  - commande pipelinée ET dernière commande du pipe
                 */
                if(!cmd->bg && cmd->seq[i + 1] == NULL)
                        wait(NULL);
                break;
        }

        return true;
}

int execute_ligne_commande(struct cmdline* cmd) 
{
        int myPipe[2];
        int in, out;

        if(cmd->seq[0]== NULL)
                return true;

        //Pour la première commande, l'entrée est l'entrée standard
        //ou un fichier spécifié entime -p sleep 3 | echo toto entrée.
        in = cmd->in ? open (cmd->in, O_RDONLY) : 0;

        for (int i = 0; cmd->seq[i] != 0; i ++) {

                /*Si il y a une commande après la commande courante on crée
                 * un pipe.
                 * 3 cas pour la sortie :
                 * - on écrit dans un pipe 
                 * - on écrit dans un fichier
                 * - on écrit sur la sortie standard
                 */
                if (cmd->seq[i+1] != 0) {
                        pipe(myPipe);
                        out = myPipe[1];
                }
                else if (cmd->out)
                        out = open(cmd->out, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP
                                        | S_IROTH);
                else
                        out = 1;

                if (exec_cmd(in, out, cmd, i) == -1) {
                        perror("error while launching command");
                        exit(EXIT_FAILURE);
                }
                in = myPipe[0];
        }
        
        // Si il y a un pipe, on attend la mort de tous les processus créés.
        if(!cmd->bg && cmd->seq[1] != NULL){
                pid_t pid;
                int status;        
                while ((pid = wait(&status)) != -1); 
        }
        
        return true;
}
