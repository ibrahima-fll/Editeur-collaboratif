#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib.h>
#include "barreoutils.h"
#include "callback.h"
#include "document.h"
#include "menu.h"
#include "raccourcis.h"

docs_t docs = {NULL, NULL, NULL, NULL, NULL, NULL};
static void open_file (const gchar *);
int main (int argc, char **argv)
{
  if (argc !=2) {perror("usage <nom de fichier >");return 1;}
  GtkWidget *p_window = NULL;
  GtkWidget *p_main_box = NULL;
  GtkWidget *p_hpaned = NULL;
GtkWidget *p_text_view = NULL;

  /* Initialisation de GTK+ */
  gtk_init (&argc, &argv);

  /* Creation de la fenetre principale de notre application */
  p_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  /* lancer l'application en plein écran*/
  gtk_window_maximize (GTK_WINDOW (p_window));
  /*titre de notre editeur */
  gtk_window_set_title (GTK_WINDOW (p_window), "Editeur de texte en GTK+");
  /* signal pour arreter cb_quit */
  g_signal_connect (G_OBJECT (p_window), "destroy", G_CALLBACK (cb_quit), NULL);
  docs.p_main_window = GTK_WINDOW (p_window);

  /* Creation du conteneur principal */
  p_main_box = gtk_vbox_new (FALSE, 0);
  /*Pour ajouter un widget*/
  gtk_container_add (GTK_CONTAINER (p_window), p_main_box);

 
  /* Creation du menu */
  gtk_box_pack_start (GTK_BOX (p_main_box), GTK_WIDGET (menu_new (p_text_view)), FALSE, FALSE, 0);

  /* Creation de la barre d'outils */
  gtk_box_pack_start (GTK_BOX (p_main_box), GTK_WIDGET (toolbar_new (p_text_view)), FALSE, FALSE, 0);

  /* Creation du separateur horizontal */
  p_hpaned = gtk_hpaned_new ();
  gtk_box_pack_start (GTK_BOX (p_main_box), p_hpaned, TRUE, TRUE, 0);

 
  /* Creation de la page d'onglets */
  {
    GtkWidget *p_notebook = NULL;

    p_notebook = gtk_notebook_new ();
    gtk_paned_add2 (GTK_PANED (p_hpaned), p_notebook);
    g_signal_connect (G_OBJECT (p_notebook), "switch-page", G_CALLBACK (cb_page_change), NULL);
    docs.p_notebook = GTK_NOTEBOOK (p_notebook);
  }

  /* Affichage de la fenetre principale */
  gtk_widget_show_all (p_window);
  gtk_paned_set_position (GTK_PANED (p_hpaned), 200);
  /* Lancement de la boucle principale */
  //open_file ("ouveau");
  open_file(argv[1]);
  gtk_main ();
  
  return EXIT_SUCCESS;
}












/* callback.c  la se trouvent toutes les fonctions*/












static void open_file (const gchar *);
static void set_title (void);
static gboolean close_all (void);

/* fonction créer nouveau */
void cb_new (GtkWidget *p_widget, gpointer user_data)
{
  document_t *nouveau = NULL;

  nouveau = g_malloc (sizeof (*nouveau));
  nouveau->chemin = NULL;
  /* Le document vient d'etre ouvert, il n'est donc pas modifie */
  nouveau->sauve = TRUE;
  docs.tous = g_list_append (docs.tous, nouveau);
  {
    gint index = 0;
    GtkWidget *p_scrolled_window = NULL;

    p_scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (p_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    nouveau->p_text_view = GTK_TEXT_VIEW (gtk_text_view_new ());
    {
      GtkTextBuffer *p_buffer = NULL;

      p_buffer = gtk_text_view_get_buffer (nouveau->p_text_view);
      g_signal_connect (G_OBJECT (p_buffer), "changed", G_CALLBACK (cb_modifie), NULL);
    }
    gtk_container_add (GTK_CONTAINER (p_scrolled_window), GTK_WIDGET (nouveau->p_text_view));
    index = gtk_notebook_append_page (docs.p_notebook, p_scrolled_window, GTK_WIDGET (gtk_label_new ("Nouveau document")));
    gtk_widget_show_all (p_scrolled_window);
    gtk_notebook_set_current_page (docs.p_notebook, index);
    set_title ();
  }

  /* parametres inutilises */
  (void)p_widget;
  (void)user_data;
}

/* fonction open */
void cb_open (GtkWidget *p_widget, gpointer user_data)
{
  GtkWidget *p_dialog = NULL;
  p_dialog = gtk_file_chooser_dialog_new ("Ouvrir un fichier", NULL,
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);
  if (gtk_dialog_run (GTK_DIALOG (p_dialog)) == GTK_RESPONSE_ACCEPT)
  {
    gchar *file_name = NULL;

    file_name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (p_dialog));
    open_file (file_name);
    g_free (file_name), file_name = NULL;
  }
  gtk_widget_destroy (p_dialog);

  /* Parametre inutilise */
  (void)p_widget;
  (void)user_data;
}


/* fonction pour enregistrer */
void cb_save (GtkWidget *p_widget, gpointer user_data)
{
  if (docs.actif)
  {
    if (!docs.actif->sauve)
    {
      /* Le fichier n'a pas encore ete enregistre */
      if (!docs.actif->chemin)
      {
        GtkWidget *p_dialog = NULL;

        p_dialog = gtk_file_chooser_dialog_new ("Sauvegarder le fichier", NULL,
                                                GTK_FILE_CHOOSER_ACTION_SAVE,
                                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                NULL);
        if (gtk_dialog_run (GTK_DIALOG (p_dialog)) == GTK_RESPONSE_ACCEPT)
        {
          docs.actif->chemin = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (p_dialog));
        }
        gtk_widget_destroy (p_dialog);
      }
      /* Soit le fichier a deja ete enregistre, soit l'utilisateur vient de nous
         fournir son nouvel emplacement, on peut donc l'enregistrer */
      if (docs.actif->chemin)
      {
        FILE *fichier = NULL;

        fichier = fopen (docs.actif->chemin, "w");
        if (fichier)
        {
          gchar *contents = NULL;
          gchar *locale = NULL;
          GtkTextIter start;
          GtkTextIter end;
          GtkTextBuffer *p_text_buffer = NULL;

          p_text_buffer = gtk_text_view_get_buffer (docs.actif->p_text_view);
          gtk_text_buffer_get_bounds (p_text_buffer, &start, &end);
          contents = gtk_text_buffer_get_text (p_text_buffer, &start, &end, FALSE);
          locale = g_locale_from_utf8 (contents, -1, NULL, NULL, NULL);
          g_free (contents), contents = NULL;
          fprintf (fichier, "%s", locale);
          g_free (locale), locale = NULL;
          fclose (fichier), fichier = NULL;
          docs.actif->sauve = TRUE;
          set_title ();
        }
        else
        {
          printf ("Impossible de sauvegarder le fichier %s", docs.actif->chemin);
        }
      }
    }
  }
  else
  {
    printf ("Aucun document ouvert");
  }

  /* Parametres inutilises */
  (void)p_widget;
  (void)user_data;
}

/* fonction enregistrer sous */
void cb_saveas (GtkWidget *p_widget, gpointer user_data)
{
  if (docs.actif)
  {
    document_t tmp = *docs.actif;

    docs.actif->chemin = NULL;
    docs.actif->sauve = FALSE;
    cb_save (p_widget, user_data);
    if (!docs.actif->sauve)
    {
      (*docs.actif) = tmp;
    }
  }
  else
  {
    printf ("Aucun document ouvert");
  }
}

/*fonction pour fermer */
void cb_close (GtkWidget *p_widget, gpointer user_data)
{
  /* Avant de fermer, il faut verifier qu'un document a bien ete ouvert */
  if (docs.actif)
  {
    if (!docs.actif->sauve)
    {
      GtkWidget *p_dialog = NULL;
      GtkWidget *p_label = NULL;

      p_dialog = gtk_dialog_new_with_buttons ("Fermer le document", docs.p_main_window, GTK_DIALOG_MODAL, GTK_STOCK_YES, GTK_RESPONSE_YES, GTK_STOCK_NO, GTK_RESPONSE_NO, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
      p_label = gtk_label_new ("Voulez-vous enregistrer les modifications ?");
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (p_dialog)->vbox), p_label, TRUE, FALSE, 10);
      gtk_widget_show_all (GTK_DIALOG (p_dialog)->vbox);
      switch (gtk_dialog_run (GTK_DIALOG (p_dialog)))
      {
        case GTK_RESPONSE_NO:
        break;
        case GTK_RESPONSE_YES:
          cb_save (p_widget, user_data);
        break;
        case GTK_RESPONSE_CANCEL:
          gtk_widget_destroy (p_dialog);
          return;
        break;
      }
      gtk_widget_destroy (p_dialog);
    }
    {
      docs.tous = g_list_remove (docs.tous, docs.actif);
      g_free (docs.actif->chemin), docs.actif->chemin = NULL;
      g_free (docs.actif), docs.actif = NULL;
      gtk_notebook_remove_page (docs.p_notebook, gtk_notebook_get_current_page (docs.p_notebook));
      if (gtk_notebook_get_n_pages (docs.p_notebook) > 0)
      {
        docs.actif = g_list_nth_data (docs.tous, gtk_notebook_get_current_page (docs.p_notebook));
      }
      else
      {
        docs.actif = NULL;
      }
    }
  }
  else
  {
    printf("Aucun document ouvert");
  }
}

void cb_quit (GtkWidget *p_widget, gpointer user_data)
{
  if (close_all ())
  {
    gtk_main_quit();
  }

  /* parametres inutilises */
  (void)p_widget;
  (void)user_data;
}

void cb_about (GtkWidget *p_widget, gpointer user_data)
{
  GtkWidget *p_about_dialog = NULL;

  p_about_dialog = gtk_about_dialog_new ();
  gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (p_about_dialog), "1.0");
  gtk_about_dialog_set_name (GTK_ABOUT_DIALOG (p_about_dialog), "Editeur de texte");

  {
    const gchar *authors[2] = {"titi206", NULL};

    gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (p_about_dialog), authors);
  }
  {
    gchar *contents = NULL;

    if (g_file_get_contents ("COPYING", &contents, NULL, NULL))
    {
      gchar *utf8 = NULL;

      utf8 = g_locale_to_utf8 (contents, -1, NULL, NULL, NULL);
      g_free (contents), contents = NULL;
      gtk_about_dialog_set_license (GTK_ABOUT_DIALOG (p_about_dialog), utf8);
      g_free (utf8), utf8 = NULL;
    }
  }
  gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (p_about_dialog), "http://www.gtk.org/");
  {
    GdkPixbuf *p_logo = NULL;

    p_logo = gdk_pixbuf_new_from_file ("logo.png", NULL);
    gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG (p_about_dialog), p_logo);
  }
  gtk_dialog_run (GTK_DIALOG (p_about_dialog));

  /* parametres inutilises */
  (void)p_widget;
  (void)user_data;
}

void cb_select (GtkTreeView *p_tree_view, GtkTreePath *arg1, GtkTreeViewColumn *arg2, gpointer user_data)
{
  gchar *str = NULL;
  GtkTreeIter iter;
  GtkTreeModel *p_tree_model = NULL;

  p_tree_model = gtk_tree_view_get_model (p_tree_view);
  gtk_tree_model_get_iter (p_tree_model, &iter, arg1);
  gtk_tree_model_get (p_tree_model, &iter, 1, &str, -1);
  {
    gchar *file_name = NULL;

    file_name = g_build_path (G_DIR_SEPARATOR_S, docs.dir_name, str, NULL);
    g_free (str), str = NULL;
    if (g_file_test (file_name, G_FILE_TEST_IS_DIR))
    {
      g_free (docs.dir_name), docs.dir_name = NULL;
      docs.dir_name = file_name;
      dir_list ();
    }
    else
    {
      open_file (file_name);
    }
  }

  /* parametres inutilises */
  (void)arg2;
  (void)user_data;
}

/*modifier */
void cb_modifie (GtkWidget *p_widget, gpointer user_data)
{
  if (docs.actif)
  {
    docs.actif->sauve = FALSE;
    set_title ();
  }

  /* Parametres inutilises */
  (void)p_widget;
  (void)user_data;
}

void cb_page_change (GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, gpointer user_data)
{
  docs.actif = g_list_nth_data (docs.tous, page_num);

  /* parametres inutilises */
  (void)notebook;
  (void)page;
  (void)user_data;
}

void dir_list (void)
{
  GDir *dir = NULL;

  dir = g_dir_open (docs.dir_name, 0, NULL);
  if (dir)
  {
    const gchar *read_name = NULL;
    GtkTreeIter iter;
    GdkPixbuf *p_file_image = NULL;

    gtk_list_store_clear (docs.p_list_store);
    p_file_image = gdk_pixbuf_new_from_file ("dossier.png", NULL);
    gtk_list_store_append (docs.p_list_store, &iter);
    gtk_list_store_set (docs.p_list_store, &iter, 0, p_file_image, 1, "..", -1);
    while ((read_name = g_dir_read_name (dir)))
    {
      gchar *file_name = NULL;

      file_name = g_build_path (G_DIR_SEPARATOR_S, docs.dir_name, read_name, NULL);
      if (g_file_test (file_name, G_FILE_TEST_IS_DIR))
      {
        p_file_image = gdk_pixbuf_new_from_file ("dossier.png", NULL);
      }
      else
      {
        p_file_image = gdk_pixbuf_new_from_file ("fichier.png", NULL);
      }
      /* Ajout d'une nouvelle entree au magasin */
      gtk_list_store_append (docs.p_list_store, &iter);
      gtk_list_store_set (docs.p_list_store, &iter, 0, p_file_image, 1, read_name, -1);
    }
    g_dir_close (dir), dir = NULL;
  }
}

static void open_file (const gchar *file_name)
{
  g_return_if_fail (file_name);
  {
    gchar *contents = NULL;

    if (g_file_get_contents (file_name, &contents, NULL, NULL))
    {
      /* Copie de contents dans le GtkTextView */
      gchar *utf8 = NULL;
      GtkTextIter iter;
      GtkTextBuffer *p_text_buffer = NULL;

      cb_new (NULL, NULL);
      docs.actif->chemin = g_strdup (file_name);
      p_text_buffer = gtk_text_view_get_buffer (docs.actif->p_text_view);
      gtk_text_buffer_get_iter_at_line (p_text_buffer, &iter, 0);
      utf8 = g_locale_to_utf8 (contents, -1, NULL, NULL, NULL);
      g_free (contents), contents = NULL;
      gtk_text_buffer_insert (p_text_buffer, &iter, utf8, -1);
      g_free (utf8), utf8 = NULL;
      /* Nous sommes obliges de remetre sauve a TRUE car l'insertion du contenu
         du fichier dans le GtkTextView a appele cb_modfie */
      docs.actif->sauve = TRUE;
      set_title ();
    }
    else
    {
    	printf("Impossible d'ouvrir le fichier %s\n", file_name);
    }
  }
}

static void set_title (void)
{
  if (docs.actif)
  {
    gchar *title = NULL;
    gchar *tmp = NULL;

    if (docs.actif->chemin)
    {
      tmp = g_path_get_basename (docs.actif->chemin);
    }
    else
    {
      tmp = g_strdup ("Nouveau document");
    }
    if (docs.actif->sauve)
    {
      title = g_strdup (tmp);
    }
    else
    {
      title = g_strdup_printf ("%s *", tmp);
    }
    g_free (tmp), tmp = NULL;
    {
      gint index = 0;
      GtkWidget *p_child = NULL;
      GtkLabel *p_label = NULL;

      index = gtk_notebook_get_current_page (docs.p_notebook);
      p_child = gtk_notebook_get_nth_page (docs.p_notebook, index);
      p_label = GTK_LABEL (gtk_notebook_get_tab_label (docs.p_notebook, p_child));
      gtk_label_set_text (p_label, title);
    }
    g_free (title), title = NULL;
  }
}

static gboolean close_all (void)
{
  gboolean ret = TRUE;

  while (docs.actif)
  {
    gint tmp = gtk_notebook_get_n_pages (docs.p_notebook);

    gtk_notebook_set_current_page (docs.p_notebook, 0);
    cb_close (NULL, NULL);
    if (gtk_notebook_get_n_pages (docs.p_notebook) >= tmp)
    {
      ret = FALSE;
      break;
    }
  }
  return ret;
}








/* menu.c */



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










/* barreoutils.c */










static void toolbar_item_new (GtkToolbar *, const gchar *, GCallback, gpointer);

GtkToolbar *toolbar_new (gpointer user_data)
{
  GtkWidget *p_toolbar = NULL;

  p_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (p_toolbar), GTK_TOOLBAR_ICONS);
  toolbar_item_new (GTK_TOOLBAR (p_toolbar), GTK_STOCK_NEW, G_CALLBACK (cb_new), user_data);
  toolbar_item_new (GTK_TOOLBAR (p_toolbar), GTK_STOCK_OPEN, G_CALLBACK (cb_open), user_data);
  toolbar_item_new (GTK_TOOLBAR (p_toolbar), GTK_STOCK_SAVE, G_CALLBACK (cb_save), user_data);
  toolbar_item_new (GTK_TOOLBAR (p_toolbar), GTK_STOCK_SAVE_AS, G_CALLBACK (cb_saveas), user_data);
  toolbar_item_new (GTK_TOOLBAR (p_toolbar), GTK_STOCK_CLOSE, G_CALLBACK (cb_close), user_data);
  toolbar_item_new (GTK_TOOLBAR (p_toolbar), GTK_STOCK_QUIT, G_CALLBACK (cb_quit), user_data);
  toolbar_item_new (GTK_TOOLBAR (p_toolbar), GTK_STOCK_ABOUT, G_CALLBACK (cb_about), user_data);
  return GTK_TOOLBAR (p_toolbar);
}

static void toolbar_item_new (GtkToolbar *p_toolbar, const gchar *stock_id, GCallback callback, gpointer user_data)
{
  GtkToolItem *p_tool_item = NULL;

  p_tool_item = gtk_tool_button_new_from_stock (stock_id);
  g_signal_connect (G_OBJECT (p_tool_item), "clicked", callback, user_data);
  gtk_toolbar_insert (p_toolbar, p_tool_item, -1);
}







/* raccourcis.c */







static void accelerator_new (GtkAccelGroup *, const gchar *, const gchar *, GCallback, gpointer);
static gboolean accel_new (GtkAccelGroup *, GObject *, guint keyval, GdkModifierType, gpointer);
static gboolean accel_open (GtkAccelGroup *, GObject *, guint keyval, GdkModifierType, gpointer);
static gboolean accel_save (GtkAccelGroup *, GObject *, guint keyval, GdkModifierType, gpointer);
static gboolean accel_saveas (GtkAccelGroup *, GObject *, guint keyval, GdkModifierType, gpointer);
static gboolean accel_close (GtkAccelGroup *, GObject *, guint keyval, GdkModifierType, gpointer);
static gboolean accel_quit (GtkAccelGroup *, GObject *, guint keyval, GdkModifierType, gpointer);

GtkAccelGroup *accel_group_new (gpointer user_data)
{
  GtkAccelGroup *p_accel_group = NULL;

  p_accel_group = gtk_accel_group_new ();
  accelerator_new (p_accel_group, "<Control>N", ACCEL_PATH_NEW, G_CALLBACK (accel_new), user_data);
  accelerator_new (p_accel_group, "<Control>O", ACCEL_PATH_OPEN, G_CALLBACK (accel_open), user_data);
  accelerator_new (p_accel_group, "<Control>S", ACCEL_PATH_SAVE, G_CALLBACK (accel_save), user_data);
  accelerator_new (p_accel_group, "<Control><Shift>S", ACCEL_PATH_SAVEAS, G_CALLBACK (accel_saveas), user_data);
  accelerator_new (p_accel_group, "<Control>W", ACCEL_PATH_CLOSE, G_CALLBACK (accel_close), user_data);
  accelerator_new (p_accel_group, "<Control>Q", ACCEL_PATH_QUIT, G_CALLBACK (accel_quit), user_data);
  gtk_window_add_accel_group (docs.p_main_window, p_accel_group);
  return p_accel_group;
}

static void accelerator_new (GtkAccelGroup *p_accel_group, const gchar *accelerator, const gchar *accel_path,
                             GCallback callback, gpointer user_data)
{
  guint key;
  GdkModifierType mods;
  GClosure *closure = NULL;

  gtk_accelerator_parse (accelerator, &key, &mods);
  closure = g_cclosure_new (callback, user_data, NULL);
  gtk_accel_group_connect (p_accel_group, key, mods, GTK_ACCEL_VISIBLE, closure);
  gtk_accel_map_add_entry (accel_path, key, mods);
}

static gboolean accel_new (GtkAccelGroup *accel_group, GObject *acceleratable, guint keyval, GdkModifierType modifier, gpointer user_data)
{
  cb_new (NULL, user_data);

  /* parametres inutilises */
  (void)accel_group;
  (void)acceleratable;
  (void)keyval;
  (void)modifier;
  (void)user_data;

  return TRUE;
}

static gboolean accel_open (GtkAccelGroup *accel_group, GObject *acceleratable, guint keyval, GdkModifierType modifier, gpointer user_data)
{
  cb_open (NULL, user_data);

  /* parametres inutilises */
  (void)accel_group;
  (void)acceleratable;
  (void)keyval;
  (void)modifier;
  (void)user_data;

  return TRUE;
}

static gboolean accel_save (GtkAccelGroup *accel_group, GObject *acceleratable, guint keyval, GdkModifierType modifier, gpointer user_data)
{
  cb_save (NULL, user_data);

  /* parametres inutilises */
  (void)accel_group;
  (void)acceleratable;
  (void)keyval;
  (void)modifier;
  (void)user_data;

  return TRUE;
}

static gboolean accel_saveas (GtkAccelGroup *accel_group, GObject *acceleratable, guint keyval, GdkModifierType modifier, gpointer user_data)
{
  cb_saveas (NULL, user_data);

  /* parametres inutilises */
  (void)accel_group;
  (void)acceleratable;
  (void)keyval;
  (void)modifier;
  (void)user_data;

  return TRUE;
}

static gboolean accel_close (GtkAccelGroup *accel_group, GObject *acceleratable, guint keyval, GdkModifierType modifier, gpointer user_data)
{
  cb_close (NULL, user_data);

  /* parametres inutilises */
  (void)accel_group;
  (void)acceleratable;
  (void)keyval;
  (void)modifier;
  (void)user_data;

  return TRUE;
}

static gboolean accel_quit (GtkAccelGroup *accel_group, GObject *acceleratable, guint keyval, GdkModifierType modifier, gpointer user_data)
{
  cb_quit (NULL, user_data);

  /* parametres inutilises */
  (void)accel_group;
  (void)acceleratable;
  (void)keyval;
  (void)modifier;
  (void)user_data;

  return TRUE;
}

