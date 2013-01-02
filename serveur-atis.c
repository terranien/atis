/*serveur-atis.c :

- Lit les requettes du client sur son tube
- Il forke 3 enfants pour faciliter le traitement
- Transmet la requette a son enfant pour traiter
- Il se met en boucle
- Si l'atis est locké les enfants se mettent en sleep
- Ils trouvent l'info demandée dans le fichier ATIS et envoient au client*/

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

#define NBENFANTS 3  /*Nombre d'enfants maximum*/

int dansPool[2]; /* Tableau pour le pool*/
int pereEnfant[NBENFANTS][2];

void traiterRequete(requete_t *requete, FILE *f){
  reponse_t reponse;
  char ligne[256];
  int fdW;
  int nbWrite;

  /* Calcul reponse->pointAccesPrive */
  strcpy(reponse.pointAccesPrive, "");
           
  /* Calcul reponse.contenu. */
  while (feof(f) == 0) {
    if (fgets(ligne, sizeof(ligne), f) != NULL) {
      if (strncmp(ligne, requete->contenu, strlen(requete->contenu)) == 0){
	break;
      }
    }
  }
  if (feof(f) == 0) {
    strcpy(reponse.contenu, ligne);
    reponse.contenu[strlen(ligne)-1] = '\0'; /* Supprime le '\n' final */
  }
  else {
    strcpy(reponse.contenu,"Désolé, je n'ai pas trouvé cette demande");
  }
  
  /* En revanche, on se remet au début du fichier pour que la prochaine */
  /* recherche de ce fils se déroule correctement (NB : on n'a pas      */
  /* besoin de faire clearerr(f) car fseek efface l'indicateur de fin   */
  /* de fichier pour le flux si l'appel se déroule bien)                */

  if (fseek(f, 0, SEEK_SET) < 0) {
    perror("fseek");
    exit(EXIT_FAILURE);
  }

  /* Affichage */
  printf("L'enfant %d du serveur a recu \"%s\" et repond \"%s\"\n",
	 getpid(),
	 requete->contenu,
	 reponse.contenu );

  /* Connexion au point d'accès client et réponse */
  fdW = open(requete->pointAccesClient, O_WRONLY);
  if (fdW == -1) {
    perror("open(pointAccessClient)");
    exit(EXIT_FAILURE);
  }
  nbWrite = write(fdW, &reponse, sizeof(reponse));
  if (nbWrite < sizeof(reponse)) {
    perror("probleme d'ecriture sur le fifo");
  }
  /* Dans cette application, le client ne renvoie pas de requête ultérieure*/
  /* nécessitant une réponse ==> On peut fermer ce tube                    */
  close(fdW);

}

void attenteRequete(int monNum){
  requete_t requete;
  FILE *f;

  /* Ouverture du fichier ATIS par le serveur*/
  f = fopen("atis.txt", "r");
  if (f == NULL) {  /*Vérifier existence ATIS*/
    perror("open(atis.txt)");
    exit(EXIT_FAILURE);
  }

  /* Attente de requête */
  while(1){
    printf("L'enfant %d (monNum=%d) se met dans le pool\n", getpid(), monNum);

    /* On écrit qu'on est disponible pour recevoir une nouvelle requete */
    write(dansPool[1], &monNum, sizeof(monNum));

    /* On attend une requête */
    read(pereEnfant[monNum][0], &requete, sizeof(requete));
    printf("L'enfant %d a ete sorti du pool pour traiter une requete\n", getpid());

    /* On la traite */
    traiterRequete(&requete,f);
  }
}
    
int main() {
  requete_t requete;
  int fdR;
  int nbRead;
  int i;
  int enfant;

  /* Création de FIFO du serveur */
  if (mkfifo(NOMPOINTACCESSERV, S_IRUSR|S_IWUSR) < 0) {
    if (errno != EEXIST) {
      perror("mkfifo(fifo client");
      exit(EXIT_FAILURE);
    }
    else {
      printf("%s Existe deja : on suppose que c'est un fifo\n", NOMPOINTACCESSERV );
      printf(" et qu'on peut continuer le programme sans probleme\n");
      puts("");
    }
  }

  /* Création du pool de fils                                             */
  /* Le pool est représenté par un tube sur lequel le père lit les        */
  /* numéros des fils qui se considèrent prêts à traiter des requêtes     */
  pipe(dansPool);
  for (i=0 ; i<NBENFANTS ; i++){
    pipe(pereEnfant[i]);
  }
  for (i=0 ; i<NBENFANTS ; i++){
    if (fork() == 0) {
      attenteRequete(i);
    }
  }

  /* Ouverture de ce tube nommé (equivalent a une ouverture de connexion).
     NB : On l'ouvre en RDWR et pas en RDONLY,
     car (cf. Blaess) : si nous ouvrons le client en lecture seule, à chaque
     fois que le client se termine, la lecture du tube nomme va echouer
     car le noyau detecte une fermeture de fichier. En demandant un tube
     en lecture et en ecriture, nous evitons ce genre de situation, car il
     reste toujours au moins un descripteur ouvert sur la sortie
  */
  fdR = open(NOMPOINTACCESSERV, O_RDWR);
  if (fdR == -1) {
    perror("open(fifo)");
    exit(EXIT_FAILURE);
  }
  /* Attente de requêtes */
  do{
    nbRead = read(fdR, &requete, sizeof(requete));
    if (nbRead != sizeof(requete)) {
      perror("lire sur fifo NOK");
      return EXIT_FAILURE;
    }
    /* On choisit, dans le pool, l'enfant en mesure de traiter cette requête */
    /* S'il n'y en a pas, on en attend un                                    */
    /* Noter qu'une fois que le read est fini, l'enfant n'est plus dans le   */
    /* pool !                                                                */
    read(dansPool[0], &enfant, sizeof(int));

    /* On envoie la requête a l'enfant */
    write(pereEnfant[enfant][1], &requete, sizeof(requete));
  }while(1);
}


   
