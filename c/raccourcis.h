#ifndef H_RACCOURCIS
#define H_RACCOURCIS

#include <glib.h>

#define ACCEL_PATH_NEW "<EditeurGTK>/Fichier/Nouveau"
#define ACCEL_PATH_OPEN "<EditeurGTK>/Fichier/Ouvrir"
#define ACCEL_PATH_SAVE "<EditeurGTK>/Fichier/Enregistrer"
#define ACCEL_PATH_SAVEAS "<EditeurGTK>/Fichier/Enregistrer sous"
#define ACCEL_PATH_CLOSE "<EditeurGTK>/Fichier/Fermer"
#define ACCEL_PATH_QUIT "<EditeurGTK>/Fichier/Quitter"

GtkAccelGroup *accel_group_new (gpointer);

#endif /* not H_RACCOURCIS */
