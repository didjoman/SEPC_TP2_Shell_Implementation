 #include "execcmd.h"

static int execute_commande(char** cmd){ /* return -1 when false, 0 when true. */
    if(cmd[0] == NULL)
        return -1;
    
    execvp(cmd[0], cmd);
    perror("Fail of execvp");
    exit(EXIT_FAILURE);
}


int exec_cmd(struct cmdline * cmd)
{
    int nb = 1;
        pid_t pid = fork();  

        if(pid == -1){
            perror("problem while forking ...");
            exit(EXIT_FAILURE);
        }
    
        if (pid == 0) {  /* le programme fils exectue les commandes */
            for(int i=0; i<nb; ++i){ /* Dans notre cas, nb est toujours = 1 (car execution d'une commande)*/
                if(execute_commande(cmd->seq[i]) == -1){
                    //libere(result);
                    perror("command not found.");
                    return false; /*-1*/
                }
            }
        }
        else{ /* père */
            if (!cmd->bg){ /* Absence de &, donc on attend la réponse du fils. */
                wait(NULL);
            }
             /* Dans les autres cas (= présence de & dans ligne de commande) on n'attends pas le résulat. */
        }
        return true;
}