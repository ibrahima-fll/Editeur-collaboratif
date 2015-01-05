#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#define PORT atoi(argv[1])
#define BUFFER_SIZE 1

/*fonction de verrouillage d'un fichier */
void lock(char fichier[]) 
{ 
  char buff[256];
  sprintf(buff,"touch ./lock/%s",fichier);
  system(buff);
} 
/* fonction pour tester si un fichier est verrouillé */
int test_lock(char fichier[])
{
  int ret;
  char buf[256];
  sprintf(buf,"./lock/%s",fichier);
  ret=open(buf,O_RDWR);
  close(ret);
  return ret; 
}
/* fonction pour deverouiller le fichier */
void unlock(char fichier[]) 
{ 
  char buff[256];
  sprintf(buff,"rm ./lock/%s",fichier);
  system(buff);
} 

void handler(int sig) 
{while(waitpid(-1, NULL, WNOHANG)>0);}

int main(int argc, char ** argv)
{
  char buf_lock[1000];
  char buf[3]="";
  struct sockaddr_in sin;
  int sfd;
  struct sockaddr_in csin;
  int csfd;
  socklen_t crecsize = sizeof(struct sockaddr_in);
  int x, nboct, fd,er;
  struct sigaction act;
  //unsigned char *buffer=NULL;
  unsigned char *buffer;  
  char *file_name; 
  if(argc != 2)
    {
      printf("usage : %s <port>\n", argv[0]);
      exit(1);
    } 

  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  act.sa_handler = handler;
  sigaction(SIGCHLD, &act, NULL);
  sfd = socket(AF_INET,SOCK_STREAM,0);
  /* test de l'ouverture du socket */
  if(sfd == -1)
    {
      perror("socket: erreur de creation de socket");
      exit(1);
    }

  printf("Le socket %d est maintenant ouvert en mode TCP/IP...\n", sfd);
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  sin.sin_family = AF_INET;
  sin.sin_port = htons(PORT);

  if(bind(sfd,(struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1)
    {
      perror("bind");
      exit(1);
    }
  printf("Socket lie au port %d...\n", PORT);

  if(listen(sfd, 5) == -1)
    {
      perror("listen: erreur d ecoute");
      exit(1);
    }

  printf("Le socket %d est maintenant en mode ecoute...\n", sfd);
  


  while(1)
    {
      printf("En attente de la tentative de connexion d'un client sur le port %d...\n", PORT);
      csfd = accept(sfd, (struct sockaddr*)&csin, &crecsize);
      if(csfd == -1)
	{
	  perror("accept : erreur de redirection vers le socket client");
	  exit(1);
	}
      printf("Nouveau client, connecte au socket %d depuis %s:%d...\n", csfd, inet_ntoa(csin.sin_addr), htons(csin.sin_port));
      x=fork();
      if(x<0)
	{
	  perror("fork: erreur de creation de processus");
	  close(sfd);
	  exit(1);
	}
      else if(x>0)
	{
	  close(csfd);
	  continue;
	}
      else
	{   
	  close(sfd);
	  if(recv(csfd,&er,sizeof(er),0)==-1)
	    {
	      perror("recv:echec lecture du type d echange");
	      close(csfd);
	      exit(1);
	    }
	  


	  printf("type envoirecep %d\n",er);

	  if(er==0)
	    {
	      /* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ si er==0 mode envoi pour le serveur @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
	      
	      printf("envoi du serveur au client\n");
	      printf("Debut envoi du fichier...\n");
	      // Il faut allouer le buffer d'envoi de données
	      //buffer = calloc(BUFFER_SIZE, sizeof(unsigned char));
	      // Puis faire la boucle d'envoi du fichier
	      buffer = (unsigned char*)calloc(BUFFER_SIZE, sizeof(unsigned char)); 
	      if(recv(csfd, &nboct, sizeof(nboct),0) == -1)
		{
		  perror("recv:echec reception du taille de fichier");
		  close(csfd);
		  exit(1);
		}
  
	      file_name = calloc(nboct+1, sizeof(char));
	      nboct = recv(csfd,file_name,nboct,0);
	      printf("fichier recu\n");
	      if(nboct == -1)
		{
		  perror("recv: echec reception du nom de fichier");
		  close(csfd);
		  exit(1);
		}
	      /*  @@@@@@@@@@@@@@@@@@@@@                 test du lockage                @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
	      if (test_lock(file_name)<=0)
	      	{
		  sprintf(buf,"OK");
		  send(csfd, buf, strlen(buf)+1,0 );
		  lock(file_name);
		  printf("Ouverture/Creation du fichier : %s...\n", file_name);
		  fd = open(file_name,O_RDWR|O_CREAT,0666);
		  if(fd == -1)
		    {
		      perror("open:echec ouverture/creation du fichier");
		      close(csfd);
		      exit(1);
		    }
		  printf("Fichier \"%s\" ouvert...\n", file_name);
		  do
		    {
		      // On vide le buffer, comme toujours
		      bzero(buffer, BUFFER_SIZE);
		      // On lie les données du fichier
		      printf("Lecture des donnees dans le fichier...\n");
		      nboct = read(fd, buffer,BUFFER_SIZE);
		      if(nboct == -1)
			{
			  perror("read: echec lecture a partir du fichier");
			  close(csfd);
			  close(fd);
			  exit(1);
			}
		      // Puis on envoie les données
		      printf("Envoi de %d octets au serveur...\n", nboct);
		      x = send(csfd, buffer, nboct, 0);
		      if(x == -1)
			{
			  perror("send: echec envoi au client");
			  close(csfd);
			  close(fd);
			  exit(1);
			}
		    }while (nboct == BUFFER_SIZE);
		  
		  close(csfd);
		  close(fd);
		  sprintf(buf_lock,"rm %s",file_name);
		  system(buf_lock);
		}

	       else
		{
		  sprintf(buf,"KO");
		  send(csfd, buf, strlen(buf)+1,0 );
		}
	    }

	  else  if (er==1) 
	    {
	      //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@    er==1 c'est une reception    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	      printf("envoi du client au serveur \n debut de reception de fichier ...\n");
	      buffer = (unsigned char*)calloc(BUFFER_SIZE, sizeof(unsigned char)); 
	      if(recv(csfd, &nboct, sizeof(nboct),0) == -1)
		{
		  perror("recv:echec reception du taille de fichier");
		  close(csfd);
		  exit(1);
		}

	      file_name = calloc(nboct+1, sizeof(char));
	      nboct = recv(csfd,file_name,nboct,0);
	      if(nboct == -1)
		{
		  perror("recv: echec reception du nom de fichier");
		  close(csfd);
		  exit(1);
		}
		  printf("Ouverture/Creation du fichier : %s...\n", file_name);
		  
		  fd = open(file_name,O_RDWR|O_CREAT,0666);
		  if(fd == -1)
		    {
		      perror("open:echec ouverture_creation du fichier");
		      close(csfd);
		      exit(1);
		    }
		  printf("Fichier \"%s\" ouvert...\n", file_name);
		  do
		    {
		      // On vide le buffer avant chaque réception
		      bzero(buffer, BUFFER_SIZE);
		      // On reçoit les données
		      //nboct = recv(csfd, buffer, BUFFER_SIZE, 0);	
		      nboct = recv(csfd, buffer, BUFFER_SIZE,0);
		      if(nboct == -1)
			{
			  perror("recv: echec reception du buffer");
			  close(csfd);
			  close(fd);
			  exit(1);
			}
		      
		      printf("%d octets recus...\n", nboct);
		      // On écrit les données dans le fichier
		      x = write(fd, buffer, nboct);
		      if(x == -1)
			{
			  perror("write: echec d ecriture sur le fichier");
			  close(csfd);
			  close(fd);
			  exit(1);
			}
		      printf("%d octets ecrits dans le fichier...\n", x);
		      // Si le nombre d'octets reçus est inférieur à BUFFER_SIZE, on est à la fin.
		    } while(nboct == BUFFER_SIZE);
		  
		  // Message de fin
		  printf("Fin de reception du fichier %s.\n", file_name);
		  unlock(file_name);
		  close(csfd);
		  close(fd);
	    }
	  // sinon si le serveur est en mode envoi on fait
	  
	}        
    } 
  return 0; 
}
