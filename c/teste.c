#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char * argv[])
{
  char em[1024]="./prog ";
  char recep[1024]="./client ";
  char envoi[1024]="./client ";
  char  *nomfichier=argv[1];
  char  *adresse_ip=argv[2];
  char  *por=argv[3];
  char *e="1";
  char *r="0";
  char test[128];

  if (argc != 4)
 {
    perror("usage: <nom de fichier> <host> <port>");
return 1;
}
  // ./client" " 
printf("message reception\n");
//concatener adresse_ip = ./client" "adresse_ip
 strcat(recep,adresse_ip);
 strcat(recep," ");
//concatener adresse_ip = ./client adresse_ip" "port 
 strcat(recep,por);
 strcat(recep," ");
//concatener adresse_ip = ./client adresse_ip port" "buffer_size(1) 
 strcat(recep,e);
 strcat(recep," ");
//concatener adresse_ip = ./client adresse_ip port buffer_size(1)" "nom_fichier
  strcat(recep,nomfichier);
  strcat(recep," ");
//concatener adresse_ip = ./client adresse_ip port buffer_size(1) nom_fichier 0(pour reception)
  strcat(recep,r);
  printf("le message de concatenation de recep\n %s\n",recep);
 

printf("message envoi\n");
//concatener adresse_ip = ./client adresse_ip
 strcat(envoi,adresse_ip);
 strcat(envoi," ");
//concatener avec adresse_ip = ./client adresse_ip port 
 strcat(envoi,por);
 strcat(envoi," ");
//concatener avec adresse_ip = ./client adresse_ip port buffer_size(1) 
 strcat(envoi,e);
 strcat(envoi," ");
//concatener avec adresse_ip = ./client adresse_ip port buffer_size(1) nom_fichier
  strcat(envoi,nomfichier);
  strcat(envoi," ");

//concatener avec adresse_ip = ./client adresse_ip port buffer_size(1) nom_fichier 1(pour envoi)
  strcat(envoi,e);
  printf("message concat envoi\n %s\n",envoi);


	printf("edition du fichier \n");
	strcat(em,nomfichier);
	printf("la commande edition %s\n",em);
	printf("debut \n");

	printf("reception fichier \n");
	system(recep);
	printf("fin reception fichier \n");

	sprintf(test,"./lock/%s",nomfichier);
    if(open(test,O_RDWR)==-1)
      {
	printf("Le fichier %s n'est pas locké. C'est bon faites vos modifications\n",nomfichier);
	printf("edition fichier \n");
	system(em);
	printf("fin fichier \n");
	
	printf("envoi fichier \n");
	system(envoi);
	printf("fin envoi fichier \n");
      }
    else
      printf("Le fichier %s est locké\n",nomfichier);
printf("Au revoir\n");

   return 0;

}
