#include <gtk/gtk.h>
#include "callback.h"
#include "menu.h"
#include "raccourcis.h"

static void menu_item_new (GtkMenu *, const gchar *, const gchar *, GCallback, gpointer);

GtkMenuBar *menu_new (gpointer user_data)
{
  GtkWidget *p_menu_bar = NULL;

  p_menu_bar = gtk_menu_bar_new ();

  /* Menu "Fichier" */
  {
    GtkWidget *p_menu = NULL;
    GtkWidget *p_menu_item = NULL;

    p_menu = gtk_menu_new ();
    gtk_menu_set_accel_group (GTK_MENU (p_menu), accel_group_new (user_data));
    p_menu_item = gtk_menu_item_new_with_mnemonic ("_Fichier");
    menu_item_new (GTK_MENU (p_menu), "_Nouveau", ACCEL_PATH_NEW, G_CALLBACK (cb_new), user_data);
    menu_item_new (GTK_MENU (p_menu), "_Ouvrir", ACCEL_PATH_OPEN, G_CALLBACK (cb_open), user_data);
    menu_item_new (GTK_MENU (p_menu), "_Enregistrer", ACCEL_PATH_SAVE, G_CALLBACK (cb_save), user_data);
    menu_item_new (GTK_MENU (p_menu), "Enregistrer _sous", ACCEL_PATH_SAVEAS, G_CALLBACK (cb_saveas), user_data);
    menu_item_new (GTK_MENU (p_menu), "_Fermer", ACCEL_PATH_CLOSE, G_CALLBACK (cb_close), user_data);
    menu_item_new (GTK_MENU (p_menu), "_Quitter", ACCEL_PATH_QUIT, G_CALLBACK (cb_quit), user_data);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (p_menu_item), p_menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (p_menu_bar), p_menu_item);
  }
  /* Menu "Aide" */
  {
    GtkWidget *p_menu = NULL;
    GtkWidget *p_menu_item = NULL;

    p_menu = gtk_menu_new ();
    p_menu_item = gtk_menu_item_new_with_mnemonic ("_Aide");
    menu_item_new (GTK_MENU (p_menu), "A _propos", NULL, G_CALLBACK (cb_about), user_data);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (p_menu_item), p_menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (p_menu_bar), p_menu_item);
  }
  return GTK_MENU_BAR (p_menu_bar);
}

static void menu_item_new (GtkMenu *p_menu, const gchar *title, const gchar *accel_path, GCallback callback, gpointer user_data)
{
  GtkWidget *p_menu_item = NULL;

  p_menu_item = gtk_menu_item_new_with_mnemonic (title);
  gtk_menu_shell_append (GTK_MENU_SHELL (p_menu), p_menu_item);
  g_signal_connect (G_OBJECT (p_menu_item), "activate", callback, user_data);
  gtk_menu_item_set_accel_path (GTK_MENU_ITEM (p_menu_item), accel_path);
}
