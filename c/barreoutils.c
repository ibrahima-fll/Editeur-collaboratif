#include <gtk/gtk.h>
#include "callback.h"
#include "barreoutils.h"

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
