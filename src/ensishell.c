/*****************************************************
 * Copyright Grégory Mounié 2008-2013                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "variante.h"
#include "readcmd.h"
#include "execcmd.h"
#include "jobs.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif


int main() {
    printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);
    //init_jobs();

	while (1) {
		struct cmdline *l;
		char *prompt = "ensishell>";
		update_list_of_jobs();
		l = readcmd(prompt);

		/* If input stream closed, normal termination */
		if (!l) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);
		if (l->bg) printf("background (&)\n");

		/* Display each command of the pipe */
		/*
		int i, j;
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			printf("seq[%d]: ", i);
                        for (j=0; cmd[j]!=0; j++) {
                                printf("'%s' ", cmd[j]);

                        }
			printf("\n");
		}
		*/
		/* Execute the command : */
		if(!execute_ligne_commande(l)) { 
			fprintf(stderr, "Failed to execute the command.");
			exit(EXIT_FAILURE);
		}
		// TODO : récuperer la mémoire
		//free_cmd(l); // TODO: à verifier
	}
}
