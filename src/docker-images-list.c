#include "docker-images-list.h"

GtkTreeModel  *ListModelCreate(DockerImagesManege *dm)
{

    GtkListStore *store;
    store = gtk_list_store_new(N_COLUMNS,
                               G_TYPE_INT,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_INT);
    dm->ListSTore = store;
    return GTK_TREE_MODEL(store);
}
void ListViewInit(GtkWidget *list)
{
    GtkCellRenderer *Renderer;
    GtkTreeViewColumn *column = NULL;

    Renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes (_("images name"),
                                                         Renderer,
                                                        "text",
                                                         LIST_NAME,
														 "foreground",
                                                         LIST_COLOR,
														 "weight",
														 LIST_FRONT,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (column,LIST_NAME);
    gtk_tree_view_append_column (GTK_TREE_VIEW(list), column);

    Renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes (_("images tag"),
                                                        Renderer,
                                                        "text",
                                                         LIST_TAG,
														 "foreground",
                                                         LIST_COLOR,
														 "weight",
														 LIST_FRONT,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (column,LIST_TAG);
    gtk_tree_view_append_column (GTK_TREE_VIEW(list), column);

    Renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes (_(" images id   "),
                                                        Renderer,
                                                        "text",
                                                         LIST_ID,
														 "foreground",
                                                         LIST_COLOR,
														 "weight",
														 LIST_FRONT,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (column,LIST_ID);
    gtk_tree_view_append_column (GTK_TREE_VIEW(list), column);

    Renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes (_("images size"),
                                                        Renderer,
                                                        "text",
                                                         LIST_SIZE,
														 "foreground",
                                                         LIST_COLOR,
														 "weight",
														 LIST_FRONT,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id (column,LIST_SIZE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(list), column);
	
}

void ImagesListAppend(GtkWidget *list,
                      const gchar *ImagesName,
                      const gchar *ImagesTag,
                      const gchar *ImagesId,
                      const gchar *ImagesSize,
                      const gchar *Color,
                      int Index,
                      GtkTreeIter *Iter,
                      GtkListStore **list_store)
{
    GtkListStore *store;
    GtkTreeIter iter;

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store,
                       &iter,
                       INT_COLUMN,Index,       //count
                       LIST_NAME, ImagesName,  //name
                       LIST_TAG,  ImagesTag,   //tag
                       LIST_ID,   ImagesId,
                       LIST_SIZE, ImagesSize,
                       LIST_COLOR,Color,
                       LIST_FRONT,300,
                       -1);
    *Iter = iter;
    if(list_store != NULL)
        *list_store = store;

}

