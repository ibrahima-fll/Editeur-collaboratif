Editeur-collaboratif
====================

Petit éditeur collaboratif avec verrouillage de fichier

Le MAN de'utilisation :


I – Compilation :

// partie client



on compile l'interface graphique (dans le dossier ''c'') :
avec la commande :

gcc $(pkg-config --libs --cflags gtk+-2.0) main.c -o prog

on compile le client :
 gcc client.c -o client

apres on compile test (le fichier principale)

gcc teste.c -o test

on crée un dossier « LOCK » dans le dossier 'c' s'il n'existe pas.




// partie serveur

on compile le serveur (dans le dossier ''s'') :

gcc serveur.c -o serveur 

on crée un dossier « LOCK » dans le dossier 's' s'il n'existe pas.



II – Execution :

d'abord il faut connaître le hostname du serveur avec la commance 'hostname -i'
ou bien on suppose que le hostname(adresse ip) est connu par l'ensemble des clients
et elle est prédéfinie,


pour exécuter le serveur, il est nécessaire d'introduire un numéro de port,
on suppose que le serveur est toujours en éxécution



// partie client

on se rend au dossier 'c', et on suppose que le client connait bien l'adresse ip du serveur ainsi que le numéro de port et on lance sur le terminal l'exécutable qu'on a deja créé 'test' avec comme parametres : le nom de fichier, l'adresse ip du serveur, et le numéro de port.
L'éditeur de texte sera ainsi lancé en ouvrant le fichier qu'on a passé en parametre, le client aura le choix de modifier, enregistrer...




