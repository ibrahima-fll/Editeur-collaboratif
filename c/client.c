#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#define FILE argv[4]

int main(int argc, char ** argv)
{
  char buf[3];
  int PORT;
  int BUFFER_SIZE;
  char *IP;
  int er;
  char buf_lock[100];
  struct sockaddr_in sin;
  int fd, sfd, nboct, x;
  unsigned long int ip; //int ip;	
  unsigned char *buffer;

  if(argc != 6) // on test le nombre d'argument
    {
      printf("usage : %s <ip> <port> <buffer size> <file name> <type envoi ou reception>\n", argv[0]);
      return 1;
    }

  /* initialisation des variables */
  PORT=atoi(argv[2]);
  BUFFER_SIZE=atoi(argv[3]);
  IP=argv[1];
  er=atoi(argv[5]);
  fd = open(FILE,O_RDWR|O_CREAT,0666);
  /* teste l'ouverture du fichier */
  if (fd == -1)
    {
      perror("open: erreur ouverture ou creation du fichier");
      exit(1);
    }
  printf("Fichier %s ouvert...\n", FILE);
  /* ouverture du socket */
  if((sfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
      perror("socket: erreur de creation ou ouverture du socket");
      close(fd);
      exit(1);
    }

  printf("Ouverture du socket %d...\n", sfd);
  
  if(!inet_pton(AF_INET, IP, &ip))
    {
      fprintf(stderr, "Le format de l'adresse IP donnee en argument est incorrect.\n");
            exit(1);
    }
  
  // Description du serveur sur lequel on veut se connecter
  sin.sin_addr.s_addr = ip;
  sin.sin_port = htons(PORT);
  sin.sin_family = AF_INET;
  
  // Connexion au serveur
  if(connect(sfd,(struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1)
    {
      perror("connect: erreur de l etablissement de connexion entre le client et le serveur");
      close(sfd);
      close(fd);
      exit(1);
    }
  printf("Connecte au serveur...\n");

  printf("le type d echange est : %d",er);
  if(send(sfd,&er,sizeof(er),0)== -1)
    {
      perror("send: erreur dans lenvoi du type d echange");
      close(sfd);
      close(fd);
      exit(1);
    }
  printf("Type d echange envoyé...\n");  
  buffer = calloc(BUFFER_SIZE, sizeof(unsigned char));  

  if(er==1)
    {
      printf("envoi du client au serveur\n");
      x = strlen(FILE);
      if(send(sfd, &x, sizeof(int), 0) == -1)
	{
	  perror("send : echec d envoi du longueur du fichier");
	  close(sfd);
	  close(fd);
	  exit(1);
	}
      printf("Debut envoi du nom de fichier...\n");
      nboct = send(sfd, FILE, x, 0);
      if(nboct == -1)
	{
	  perror("send: echec d envoi du nom du fichier ");
	  close(sfd);
	  close(fd);
	  exit(1);
	}
      printf("Nom fichier envoye...\n");
      printf("Debut envoi du fichier...\n");
      do
	{ 
	  bzero(buffer, BUFFER_SIZE);
	  printf("Lecture des donnees dans le fichier...\n");
	  nboct = read(fd,buffer,BUFFER_SIZE);
	  if(nboct == -1)
	    {
	      perror("read: echec de lecture a partir du fichier ouvert");
	      close(sfd);
	      close(fd);
	      exit(1);
	    }
	  // Puis on envoie les données
	  printf("Envoi de %d octets au serveur...\n", nboct);
	  if((send(sfd,buffer,nboct, 0)) == -1)
	    {
	      perror("send");
	      close(sfd);
	      close(fd);
	      exit(1);
	    }
	}while (nboct == BUFFER_SIZE);
      close(sfd);
      close(fd);
    }

  else if (er==0) 
    {


      printf("reception a partir du serveur%d\n",er);
      x = strlen(FILE);
      if(send(sfd, &x, sizeof(int), 0) == -1)
	{
	  perror("send : echec d envoi du longueur du fichier");
	  close(sfd);
	  close(fd);
	  exit(1);
	}
      printf("Debut envoi du nom de fichier...\n");
      nboct = send(sfd, FILE, x, 0);
      if(nboct == -1)
	{
	  perror("send: echec d envoi du nom du fichier ");
	  close(sfd);
	  close(fd);
	  exit(1);
	}
      printf("Nom fichier envoye...\n");

      recv(sfd,buf,3,0);
      printf("%s\n",buf);

      if(strncmp(buf,"OK",2)==0)
	{
	  sprintf(buf_lock,"rm ./lock/%s",FILE);
	  system(buf_lock);
   
	  do{
	    bzero(buffer,BUFFER_SIZE);
	    nboct = recv(sfd,buffer,BUFFER_SIZE,0);
	    if(nboct == -1)
	      {
		perror("recv: echec reception du buffer a partir du serveur");
		close(sfd);
		close(fd);
		exit(1);
	      }
	    printf("%d octets recus...\n", nboct);
	    x = write(fd, buffer, nboct);
	    if(x == -1)
	      {
		perror("write: echec d ecriture sur le fichier");
		close(sfd);
		close(fd);
		exit(1);
	      }
	    printf("%d octets ecrits dans le fichier...\n", x);
	  } while(nboct == BUFFER_SIZE);
	  printf("Fin de reception du fichier %s. \n",FILE);
	  close(sfd);
	  close(fd);    
	}
      else
	{
	  printf("Fichier locké\n");
	  sprintf(buf_lock,"touch ./lock/%s",FILE);
	  system(buf_lock);
	}
    }
  return 0;
}
