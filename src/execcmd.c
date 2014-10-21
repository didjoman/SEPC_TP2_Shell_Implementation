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
int exec_cmd(struct cmdline * cmd)
{
        if(cmd->seq[0]== NULL)
            return true;

        pid_t pid = fork();  

        switch(pid){
        // Problème lors du fork :
        case -1 :
            perror("problem while forking ...");
            exit(EXIT_FAILURE);
        
        // Processus fils (execute les commandes)
        case 0: 
            // Lance l'ensemble des commandes séparées par les pipes
            // TODO : gérer les pipes ici.
            for (int i=0; cmd->seq[i]!=0; i++) {
                // Cas particulier, "jobs" est exécutée par le shell
                if(!strcmp(*cmd->seq[i], "jobs"))
                    print_jobs();
                // Cas général
                else if(execute_commande(cmd->seq[i]) == -1){
                    perror("command not found.");
                    return false; /*-1*/
                }
            }
        // Processus père (attends ou non la mort du fils)     
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