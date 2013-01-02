/*Pilote : client.c
		- connexion avec le serveur
		- fait une demande d'info ATIS sur le tube
		- s'il recoit la bonne info il envoie un ACK 
		- sinon un NACK pour redemander de la renvoyer
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "client-serveur.h"

int main(int argc, char *argv[]) {
  requete_t requete;
  reponse_t reponse;
  int i;
  int len;
  int fdW, fdR;  
  int nbRead, nbWrite;/* variables ecrire et lire sur le fifo*/

  if (argc != 2) {
    printf("Usage = ./Client code_OACI \nExemples :\nEBLG : Belgique,Liege.\nEBBR : Belgique,Bruxelles.\n EBAW : Antwerpen\n");
    exit(EXIT_FAILURE);
  }

  /* Préparation du champ pointAccesClient de la requête */
  sprintf(requete.pointAccesClient, "./serveur-client.%d", getpid());

  /* Préparation du champ contenu : On y recopie argv[1] en mettant en */
  /* majuscules                                                        */
  len = strlen(argv[1]);
  for (i=0 ; i < len ; i++) {
    requete.contenu[i] = toupper(argv[1][i]);
  }
  requete.contenu[len] = '\0';

  /* Création du tube nommé du client */
  if (mkfifo(requete.pointAccesClient, S_IRUSR|S_IWUSR) < 0) {
    if (errno != EEXIST) {
      perror("mkfifo(tube nommé client");
      exit(EXIT_FAILURE);
    }
    else {
      printf("%s existe deja : on suppose que c'est un fifo\n",
	     requete.pointAccesClient );
      printf("et qu'on peut continuer le programme sans probleme\n");
      puts("");
    }
  }

  /* Envoi de la requête vers le serveur */
  fdW = open(NOMPOINTACCESSERV, O_WRONLY);
  if (fdW == -1) {
    perror("open(NOMPOINTACCESSERV)");
    exit(-1);
  }
  nbWrite = write(fdW, &requete, sizeof(requete));
  if (nbWrite < sizeof(requete)) {
	perror("probleme d'ecriture sur FIFO");
  }

  /* On n'aura pas d'autres messages à envoyer au serveur. On peut donc    */
  /* fermer le tube                                                        */
  close(fdW);

  /* On ouvre seulement maintenant le tube pour écouter la réponse, sinon  */
  /* on serait bloqué jusqu'à ce que le serveur ouvre le pipe en écriture  */
  /* pour envoyer sa réponse. Si on ouvrait le tube avant d'envoyer la     */
  /* requête, le serveur ne recevrait jamais la requête et donc n'ouvrirait*/
  /* jamais le tube ==> On serait bloqué indéfiniment.                     */
  fdR = open(requete.pointAccesClient, O_RDONLY);
  if (fdR == -1) {
    perror("open(fifo)");
    exit(-1);
  }

  /* On lit la réponse */
  nbRead = read(fdR, &reponse, sizeof(reponse));
  if (nbRead != sizeof(reponse)) {
    printf("Communication avec le serveur probablement rompue et envoyé un NACK\n");
    exit(EXIT_FAILURE);
  }

  /* On affiche la réponse */
  printf("Reponse du serveur = \"%s\"\n", reponse.contenu);
  /*if (requete == reponse)*/  
	printf("\nEnvoyer un ACK au serveur\n");
  /*else
	printf("Envoyer un NACK au serveur");*/

  /* NB : Dans cette application, on utilise pas le point d'accès privé */
  /* vers le serveur                                                    */

  /* On ferme le tube côté réception */
  close(fdR);

  /* On détruit le tube nommé du client (puisque ce tube nommé ne servira */
  /* plus jamais.                                                         */
  if (unlink(requete.pointAccesClient) < 0) {
    perror("unlink");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}


   
