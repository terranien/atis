/*************************************************/
/* Déclarations communes au client et au serveur */
/*************************************************/

#define NOMPOINTACCESSERV "FiFo_Client-Serveur"  /* définir le nom de FIFO*/

/* Structure des messages utilisés dans le sens client-->serveur */

typedef struct{
  char pointAccesClient[128];
  char contenu[256];
} requete_t;

/* Structure des messages utilisés dans le sens serveur-->client */

typedef struct{
  char pointAccesPrive[128];
  char contenu[256];
} reponse_t;
