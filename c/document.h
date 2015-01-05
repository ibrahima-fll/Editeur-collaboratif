#ifndef H_DOCUMENT
#define H_DOCUMENT

#include <gtk/gtk.h>

typedef struct
{
  gchar *chemin;
  gboolean sauve;
  GtkTextView *p_text_view;
} document_t;

typedef struct
{
  GList *tous;
  document_t *actif;
  GtkWindow *p_main_window;
  GtkNotebook *p_notebook;
  GtkListStore *p_list_store;
  gchar *dir_name;
} docs_t;

extern docs_t docs;

#endif /* not H_DOCUMENT */
