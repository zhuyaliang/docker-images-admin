#include "docker-images-remote.h"
#include "docker-images-list.h"
GtkWidget *LoadRemoteImages(DockerImagesManege *dm)
{
    GtkWidget *Hbox;
    GtkWidget *Scrolled;
    GtkWidget *ImagesList;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    int i;

    Hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    Scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_box_pack_start(GTK_BOX(Hbox),Scrolled, TRUE, TRUE,0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (Scrolled),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (Scrolled),
                                        GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (Scrolled), 6);
    
	ImagesList= gtk_tree_view_new_with_model(ListModelCreate(dm));

    /* init user list */
    ListViewInit(ImagesList);
    dm->RemoteImagesList = ImagesList;

    for( i = 0; i < 10; i ++)
    {
        ImagesListAppend(ImagesList,
                         "i am zhouyan",
						 "latest",
						 "8dhjsgsq92idq",
						  "438M",
						  "green",
						 i,
                         &dm->dtl[i].Iter);
    }
	gtk_container_add (GTK_CONTAINER (Scrolled), ImagesList);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ImagesList));
    gtk_tree_selection_set_mode(selection,GTK_SELECTION_SINGLE);
    model=gtk_tree_view_get_model(GTK_TREE_VIEW(ImagesList));
    dm->RemoteModel = model;

	return Hbox;
}		
