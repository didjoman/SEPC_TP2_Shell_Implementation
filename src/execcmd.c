#include "execcmd.h"
#include "jobs.h"

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
                        return true;
                }

                // Execution sans &, on attend la terminaison du fils.
                /* Attention ! 0n attend dans deux cas :
                 *  - commande unique (sans pipe) lancée sans &
                 *  - commande pipelinée ET dernière commande du pipe
                 */
                if(!cmd->bg && cmd->seq[i + 1] == NULL)
                        wait(NULL);
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
