#include "docker-images-local.h"
#include "docker-images-list.h"
#include "docker-images-utils.h"


static void RemoveImages (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  DockerImagesManege *dm = (DockerImagesManege *)data;

  if (gtk_tree_selection_get_selected (dm->LocalImagesSelect, NULL, &iter))
    {
      gint i;
      GtkTreePath *path;

      path = gtk_tree_model_get_path (dm->LocalModel, &iter);
      i = gtk_tree_path_get_indices (path)[0];
	  printf(" i = %d\r\n",i);
	  gtk_list_store_remove (GTK_LIST_STORE (dm->LocalModel), &iter);
      gtk_tree_path_free (path);
    }
}

static void RunImages (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  DockerImagesManege *dm = (DockerImagesManege *)data;

  if (gtk_tree_selection_get_selected (dm->LocalImagesSelect, NULL, &iter))
    {
      gint i;
      GtkTreePath *path;

      path = gtk_tree_model_get_path (dm->LocalModel, &iter);
      i = gtk_tree_path_get_indices (path)[0];
	  printf(" i = %d\r\n",i);
	  gtk_tree_path_free (path);
    }
}

static void SaveImages (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  DockerImagesManege *dm = (DockerImagesManege *)data;

  if (gtk_tree_selection_get_selected (dm->LocalImagesSelect, NULL, &iter))
    {
      gint i;
      GtkTreePath *path;

      path = gtk_tree_model_get_path (dm->LocalModel, &iter);
      i = gtk_tree_path_get_indices (path)[0];
	  printf(" i = %d\r\n",i);
	  gtk_tree_path_free (path);
    }
}

static void OperationImages (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  DockerImagesManege *dm = (DockerImagesManege *)data;

  if (gtk_tree_selection_get_selected (dm->LocalImagesSelect, NULL, &iter))
    {
      gint i;
      GtkTreePath *path;

      path = gtk_tree_model_get_path (dm->LocalModel, &iter);
      i = gtk_tree_path_get_indices (path)[0];
	  printf(" i = %d\r\n",i);
	  gtk_tree_path_free (path);
    }
}
static int ExtractNameTag(int index,char *data,DockerImagesManege *dm)
{
    char  **ImageInfo;
    int len = 0;
    
    ImageInfo = g_strsplit(data,":",-1);
    len = g_strv_length(ImageInfo);
    if(len == 2)
    {
	    memset(dm->dll[index].ImagesName,'\0',strlen(dm->dll[index].ImagesName));
        memcpy(dm->dll[index].ImagesName,"<none>",6);
	    memset(dm->dll[index].ImagesTag,'\0',strlen(dm->dll[index].ImagesTag));
        memcpy(dm->dll[index].ImagesTag,"<none>",6);
    }
    else if(len <= 1)
    {
        g_strfreev(ImageInfo);
        return len;
    }
    else
    {
	    memset(dm->dll[index].ImagesName,'\0',strlen(dm->dll[index].ImagesName));
        memcpy(dm->dll[index].ImagesName,&ImageInfo[1][2],(strlen(ImageInfo[1])-2));
	    memset(dm->dll[index].ImagesTag,'\0',strlen(dm->dll[index].ImagesTag));
        memcpy(dm->dll[index].ImagesTag,ImageInfo[2],(strlen(ImageInfo[2])-2));
    }
    g_strfreev(ImageInfo);
	return len;
}
static int ExtractId(int index,char *data,DockerImagesManege *dm)
{
	if(strlen(data) < 78)
        return -1;
    memset(dm->dll[index].ImagesId,'\0',strlen(dm->dll[index].ImagesId));
    memcpy(dm->dll[index].ImagesId,&data[13],12);

	return 0;
}
static int ExtractSzie(int index,char *data,DockerImagesManege *dm)
{
    float size = 0.0;
    char Sizebuf[24] = { 0 };
    if(strlen(data) < 7)
        return -1;
    size = atof(&data[7]);
    if(size <= 1000)
    {
        sprintf(Sizebuf,"%.2f %s",size, "K");
        memset(dm->dll[index].ImagesSzie,'\0',strlen(dm->dll[index].ImagesSzie));
        memcpy(dm->dll[index].ImagesSzie,Sizebuf,strlen(Sizebuf));
    }    
    else if(size > 1000 && size <=1000000)
    {
        sprintf(Sizebuf,"%.2f %s",size / 1000, "KB");
        memset(dm->dll[index].ImagesSzie,'\0',strlen(dm->dll[index].ImagesSzie));
        memcpy(dm->dll[index].ImagesSzie,Sizebuf,strlen(Sizebuf));
    }
    else if(size > 1000000 && size <= 1000000000)
    {
        sprintf(Sizebuf,"%.2f %s",(size / (1000 * 1000)), "MB");
        memset(dm->dll[index].ImagesSzie,'\0',strlen(dm->dll[index].ImagesSzie));
        memcpy(dm->dll[index].ImagesSzie,Sizebuf,strlen(Sizebuf));
    }    
    else 
    {
        sprintf(Sizebuf,"%.2f %s",(size / (1000 * 1000 *1000)), "GB");
        memset(dm->dll[index].ImagesSzie,'\0',strlen(dm->dll[index].ImagesSzie));
        memcpy(dm->dll[index].ImagesSzie,Sizebuf,strlen(Sizebuf));
    
    }    
	return 0;
}
static int JsonSplit(int index,char *data,DockerImagesManege *dm)
{
    char  **ImageInfo;
    int len = 0;
    
    ImageInfo = g_strsplit(data,",",-1);
    len = g_strv_length(ImageInfo);
    if(len < 10)
    {
        return len;
    }    
    ExtractNameTag(index,ImageInfo[6],dm);
    ExtractSzie(index,ImageInfo[8],dm);
    ExtractId(index,ImageInfo[2],dm);

	g_strfreev(ImageInfo); 
    return len;
}

static int GetImagesInfo(char *data,DockerImagesManege *dm)
{
    char **ImageInfo;
    int len = 0;
    int j;
    
    ImageInfo = g_strsplit(data,"\"Containers\"" ,-1);
    len = g_strv_length(ImageInfo);

    for(j = 1 ; j < len ; j++)
    {
        JsonSplit(j-1,ImageInfo[j],dm);
    }	
	g_strfreev(ImageInfo); 
    return len - 1;
}		
GtkWidget *LoadLocalImages(DockerImagesManege *dm)
{
    GtkWidget *Vbox;
    GtkWidget *Scrolled;
    GtkWidget *ImagesList;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkWidget *Hbox;
	GtkWidget *ButtonRemove;
	GtkWidget *ButtonRun ;
	GtkWidget *ButtonOperation;
	GtkWidget *ButtonSave;
    int i;
	int len = 0;
	CURLcode response;

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
	
	response = DockerGet(dm->dc, "http://v1.25/images/json",DOCKERSOCK);
	if (response == CURLE_OK) 
	{
    	len = GetImagesInfo(GetBuffer(dm->dc),dm);
    }
	else
	{
		MessageReport(_("Get Images Fail"),_("Curl GET Error"),ERROR);
	}			
    for( i = 0; i < len; i ++)
    {
        ImagesListAppend(ImagesList,
						 dm->dll[i].ImagesName,
						 dm->dll[i].ImagesTag,
						 dm->dll[i].ImagesId,
						 dm->dll[i].ImagesSzie,
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
	g_signal_connect (ButtonRemove, 
					 "clicked",
                      G_CALLBACK (RemoveImages), 
					  dm);

	ButtonSave =      gtk_button_new_with_label(_("  Save      "));
    gtk_box_pack_start(GTK_BOX(Hbox),ButtonSave, FALSE, FALSE,0);
	g_signal_connect (ButtonSave, 
					 "clicked",
                      G_CALLBACK (SaveImages), 
					  dm);

	ButtonOperation = gtk_button_new_with_label(_("Operation"));
    gtk_box_pack_start(GTK_BOX(Hbox),ButtonOperation, FALSE, FALSE,0);
	g_signal_connect (ButtonOperation, 
					 "clicked",
                      G_CALLBACK (OperationImages), 
					  dm);
	
	ButtonRun =       gtk_button_new_with_label(_("   Run      "));
    gtk_box_pack_start(GTK_BOX(Hbox),ButtonRun, FALSE, FALSE,0);
	g_signal_connect (ButtonRun, 
					 "clicked",
                      G_CALLBACK (RunImages), 
					  dm);

	//g_signal_connect(selection, "changed", G_CALLBACK(Select), ua);

	return Vbox;
}		
