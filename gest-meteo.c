/*
Gestionnaire Météo:

!note: possiblement trop de #include par rapport à ce qui est nécessaire
           vérifier le chemin des fichiers (différent Windows/Linux)
           vérifier system(" clear ") pour effacer l'écran
           utilse 2 getchar pour pause écran (car le buffer garde le premier ENTER et je ne sais plus comment éviter cela)
           

!programme: affiche menu 3 options
                      1: vérifie si ATIS existe ou sinon le crée et y écrit un string 
                      2: écrit un second string dans atis si read_lock n'existe pas sinon ne fait rien !! mode a+ pour écrire à la
                      suite du premier string (si w+ on écrase le fichier)
                      3: quitter
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

FILE * fpA;
FILE * fpW;
FILE * fpR;
char * fileA = "atis.txt";
char * fileR = "read_lock.txt";
char * fileW = "write_lock.txt";

void menu(){
 system("clear");
 printf("Options: \n");
 printf("1: Vérifier existence ATIS\n");
 printf("2: Modifier ATIS\n");
 printf("x: Quitter\n");
 printf("Choix: ");
}

void verifATIS() {
 fpA = fopen(fileA,"r");
 if(fpA == 0) { //atis pas trouvé => création et écriture
  fpA = fopen(fileA, "w");
  fprintf(fpA,"%s","EBLG 1803 00000KT 0600 FG OVC008 BKN040 PROB40 2024 0300 DZ FG OVC002 BKN040");
  fclose(fpA); 
  printf("Le fichier ATIS a été créé\n");
  getchar();
 }
 else {
  printf("Fichier ATIS trouvé");
  getchar();
 } 
}

void ecrireATIS() {
 fpR = fopen(fileR, "r");    
 if(fpR != 0) { //read_lock existe => atis est lu par serveur => retour menu
  printf("Le fichier ATIS est en cours de lecture\n");
  getchar();
 }
 else {
  fpW = fopen(fileW, "w"); //atis est libre => ecriture nouvelle ligne
  fclose(fpW);
  fpA = fopen(fileA, "a+"); //a+ = rajoute à la suite >< w+ = écraser fichier
  fprintf(fpA,"%s","EBBR 0615 20015KT 8000 RA SCT010 OVC015 TEMPO 0608 5000 RA BKN005 BECMG 0810 9999 NSW BKN025");
  fclose(fpA); 
  remove(fileW);
  printf("Le fichier ATIS a été modifié");
  getchar();
 }
}

int main() {
 char choix;
 menu();
 do{
  choix=getchar();

   switch(choix){
    case '1': {
               verifATIS();
               getchar();
               menu();
              }
    break;
 
    case '2': {
               ecrireATIS();
               getchar();
               menu();
              }
    break;
   }
 }
 while(choix != 'x');
 return 0; 
} 
