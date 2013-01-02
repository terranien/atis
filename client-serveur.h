/*************************************************/
/* D�clarations communes au client et au serveur */
/*************************************************/

#define NOMPOINTACCESSERV "FiFo_Client-Serveur"  /* d�finir le nom de FIFO*/

/* Structure des messages utilis�s dans le sens client-->serveur */

typedef struct{
  char pointAccesClient[128];
  char contenu[256];
} requete_t;

/* Structure des messages utilis�s dans le sens serveur-->client */

typedef struct{
  char pointAccesPrive[128];
  char contenu[256];
} reponse_t;
