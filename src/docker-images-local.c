#include "docker-images-local.h"
#include "docker-images-list.h"

GtkWidget *LoadLocalImages(DockerImagesManege *dm)
{
    GtkWidget *Vbox;
    GtkWidget *Scrolled;
    GtkWidget *ImagesList;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkWidget *Hbox;
	GtkWidget *ButtonRemove;
	GtkWidget *ButtonRun;
	GtkWidget *ButtonOperation;
	GtkWidget *ButtonSave;
    int i;

    Vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_widget_set_size_request (Vbox,-1,200);

    Scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_box_pack_start(GTK_BOX(Vbox),Scrolled, TRUE, TRUE,0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (Scrolled),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (Scrolled),
                                        GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (Scrolled), 6);
    
	ImagesList= gtk_tree_view_new_with_model(ListModelCreate(dm));

    /* init user list */
    ListViewInit(ImagesList);
    dm->LocalImagesList = ImagesList;

    for( i = 0; i < 10; i ++)
    {
        ImagesListAppend(ImagesList,
                         "docker.isoft.zhcn.cc/zhuyaliang/shipyard",
						 "latest",
						 "jfh8w8eq92idq",
						  "458M",
						  "blue",
						 i,
                         &dm->dll[i].Iter);
    }
	gtk_container_add (GTK_CONTAINER (Scrolled), ImagesList);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ImagesList));
    gtk_tree_selection_set_mode(selection,GTK_SELECTION_SINGLE);
    model=gtk_tree_view_get_model(GTK_TREE_VIEW(ImagesList));
    dm->LocalModel = model;
	dm->LocalImagesSelect = selection;

	Hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(Vbox),Hbox,FALSE,FALSE,0);
	
	ButtonRemove =    gtk_button_new_with_label(_(" Remove  "));
    gtk_box_pack_start(GTK_BOX(Hbox),ButtonRemove, FALSE, FALSE,0);
	ButtonSave =      gtk_button_new_with_label(_("  Save      "));
    gtk_box_pack_start(GTK_BOX(Hbox),ButtonSave, FALSE, FALSE,0);
	ButtonOperation = gtk_button_new_with_label(_("Operation"));
    gtk_box_pack_start(GTK_BOX(Hbox),ButtonOperation, FALSE, FALSE,0);
	ButtonRun =       gtk_button_new_with_label(_("   Run      "));
    gtk_box_pack_start(GTK_BOX(Hbox),ButtonRun, FALSE, FALSE,0);

	g_signal_connect(selection, "changed", G_CALLBACK(Select), ua);

	return Vbox;
}		
