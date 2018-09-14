#include "docker-images-remote.h"
#include "docker-images-list.h"
#include "docker-images-utils.h"

static GtkListStore *RemoteListStore = NULL;
static void RemoveRemoteImages(DockerImagesManege *dm)
{
    const Digest[64] = { 0 };
    char MsgBuf[1256] = { 0 };
    int i;
    CURLcode response;
     
    i = dm->SelectIndex;
    sprintf(MsgBuf,"http://%s:%s/v2/%s/manifests/%s",
                              dm->Address,
                              dm->Port,
                              dm->dtl[i].ImagesName,
                              dm->dtl[i].ImagesTag);
    response = DockerQueryDigest(dm->dc,MsgBuf,Digest);
    if (response == CURLE_OK)
    {
        printf("GetBuffer(dm->dc) = %s\r\n",Digest);
    }
}    
static void RemoveImages (GtkWidget *widget, gpointer data)
{
    GtkTreeIter iter;
    DockerImagesManege *dm = (DockerImagesManege *)data;
    if (gtk_tree_selection_get_selected (dm->RemoteImagesSelect, NULL, &iter))
    {
        gint i;
        GtkTreePath *path;
        path = gtk_tree_model_get_path (dm->RemoteModel, &iter);
        i = gtk_tree_path_get_indices (path)[0];
        dm->SelectIndex = i;
        RemoveRemoteImages(dm);
	    gtk_list_store_remove (GTK_LIST_STORE (dm->RemoteModel), &iter);
        gtk_tree_path_free (path);
    }
}
static gpointer AsyncPullImages(gpointer data)
{
    CURLcode response;
    char MsgBuf[256] = { 0 };
    DockerImagesManege *dm = (DockerImagesManege *)data;
    int i;

    i = dm->SelectIndex;
    sprintf(MsgBuf,"fromImage=%s:%s/%s:%s",
                              dm->Address,
                              dm->Port,
                              dm->dtl[i].ImagesName,
                              dm->dtl[i].ImagesTag);
    response = DockerPost(dm->dc,"http:/v1.24/images/create?",MsgBuf,NULL);
    if (response == CURLE_OK)
    {
        MessageReport(_("Pull Images"),
                      _("Images pull successful,please refresh the local list."),
                      INFOR);
    }
    else
    {
        MessageReport(_("Pull Images"),
                      curl_easy_strerror(response),
                      ERROR);
    }   
    return FALSE;
}    
static void PullImages (GtkWidget *widget, gpointer data)
{
    GtkTreeIter iter;
    DockerImagesManege *dm = (DockerImagesManege *)data;

    if (gtk_tree_selection_get_selected (dm->RemoteImagesSelect, NULL, &iter))
    {
        gint i;
        GtkTreePath *path;
        path = gtk_tree_model_get_path (dm->RemoteModel, &iter);
        i = gtk_tree_path_get_indices (path)[0];
        dm->SelectIndex = i;
        g_thread_new("pullimages",(GThreadFunc)AsyncPullImages,(gpointer)data);
        gtk_tree_path_free (path);
    }
}
static int ImagesCount;
static void WriteData(const char *name,const char *tag,DockerImagesManege *dm)
{
    memset(dm->dtl[ImagesCount].ImagesName,
          '\0',
          strlen(dm->dtl[ImagesCount].ImagesName));
    memcpy(dm->dtl[ImagesCount].ImagesName,name,strlen(name));

    memset(dm->dtl[ImagesCount].ImagesTag,
          '\0',strlen(dm->dtl[ImagesCount].ImagesTag));
    memcpy(dm->dtl[ImagesCount].ImagesTag,tag,strlen(tag));
    ImagesCount++;

}    
static int AnalysisData(char *data,DockerImagesManege *dm)
{
    char **ImageInfo;
    int len = 0;
    int j,i;

    ImageInfo = g_strsplit(data,"\"" ,-1);
    len = g_strv_length(ImageInfo);
    if(len < 9 )
    {
        return -1;
    }    
    if(len % 2 == 0)
    {
        return -1;
    }    
    i = (len - 9) / 2 + 1;
    for(j = 0 ; j < i ; j++)
    {
        WriteData(ImageInfo[3],ImageInfo[7+j*2],dm);
    }
    return 0;
}    
static int GetImagesTag(char *data,DockerImagesManege *dm)
{
    char **ImageInfo;
    CURLcode response;
    int len = 0;
    char MsgBuf[228] = { 0 };

    ImageInfo = g_strsplit(data,"\"" ,-1);
    len = g_strv_length(ImageInfo);
    if(len <= 1)
    {
        return -1;
    }    
    sprintf(MsgBuf,"http://%s:%s/v2/%s/tags/list",dm->Address,dm->Port,ImageInfo[1]);
    response = DockerGet(dm->dc,MsgBuf,NULL);
    if (response == CURLE_OK)
    {
        AnalysisData(GetBuffer(dm->dc),dm);
    }
    else
    {
        MessageReport(_("Connect Repository"),
                      curl_easy_strerror(response),
                      ERROR);
        return -1;
    }
    g_strfreev(ImageInfo);
    
    return 0;
}    
static int GetRemoteImagesInfo (char *data,DockerImagesManege *dm)
{
    char **ImageInfo;
    int len = 0;
    int j;

    ImageInfo = g_strsplit(data,"," ,-1);
    len = g_strv_length(ImageInfo);
   
    ImagesCount = 0;
    for(j = 1 ; j < len ; j++)
    {
        GetImagesTag(ImageInfo[j],dm);
    }
    g_strfreev(ImageInfo);
    return len - 1;

}    
static int GetRepositoryImages (GtkWidget *widget, gpointer data)
{
    DockerImagesManege *dm = (DockerImagesManege *)data;
    int i;
    CURLcode response;
    const char *Address;
    const char *Port;
    char MsgBuf[128] = { 0 };

    Address = gtk_entry_get_text(GTK_ENTRY(dm->EntryAddress));
    memset(dm->Address,'\0',sizeof(dm->Address));
    memcpy(dm->Address,Address,strlen(Address));
    Port    = gtk_entry_get_text(GTK_ENTRY(dm->EntryPort)); 
    memset(dm->Port,'\0',sizeof(dm->Port));
    memcpy(dm->Port,Port,strlen(Port));
    if(strlen(Address) <= 0 || strlen(Port) <= 0)
    {
        MessageReport(_("Connect Repository"),
                      _("The address and port number can not be empty."),
                      ERROR);
        return -1;
    }   
    if(RemoteListStore != NULL)
    {    
        gtk_list_store_clear(RemoteListStore);
    }    
    sprintf(MsgBuf,"http://%s:%s/v2/_catalog",Address,Port);
    response = DockerGet(dm->dc,MsgBuf,NULL);
    if (response == CURLE_OK)
    {
        GetRemoteImagesInfo(GetBuffer(dm->dc),dm);
    }
    else
    {
        MessageReport(_("Connect Repository"),
                      curl_easy_strerror(response),
                      ERROR);
        return -1;
    }    
    
    for( i = 0; i < ImagesCount; i ++)
    {
        ImagesListAppend(dm->RemoteImagesList,
						 dm->dtl[i].ImagesName,
						 dm->dtl[i].ImagesTag,
						 dm->dtl[i].ImagesId,
						 dm->dtl[i].ImagesSize,
						  "blue",
						 i,
                         &dm->dtl[i].Iter,
                         &RemoteListStore);
    
    }
   
    return 0;
}    
static void ConnectRepository(GtkWidget *Box,DockerImagesManege *dm)
{
    GtkWidget *Table;
    GtkWidget *LabelAddress;
    GtkWidget *EntryAddress;
    GtkWidget *LabelPort;
    GtkWidget *EntryPort;
    GtkWidget *ButtonConnect;

    Table = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(Box),Table,FALSE,FALSE,6);
    gtk_grid_set_column_homogeneous(GTK_GRID(Table),FALSE);

    LabelAddress = gtk_label_new(NULL);
    SetLableFontType(LabelAddress,"red",13,_("Address"));
    gtk_grid_attach(GTK_GRID(Table) , LabelAddress, 0 , 0 , 1 , 1);

    EntryAddress = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(EntryAddress),48);
    dm->EntryAddress = EntryAddress;
    gtk_grid_attach(GTK_GRID(Table) ,EntryAddress , 1 , 0 , 1 , 1);
    
    LabelPort = gtk_label_new(NULL);
    SetLableFontType(LabelPort,"red",13,_("Port"));
    gtk_grid_attach(GTK_GRID(Table) , LabelPort, 2 , 0 , 1 , 1);

    EntryPort = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(EntryPort),7);
    dm->EntryPort = EntryPort;
    gtk_grid_attach(GTK_GRID(Table) ,EntryPort , 3 , 0 , 1 , 1);
	
    ButtonConnect    = gtk_button_new_with_label(_("Connect"));
    gtk_grid_attach(GTK_GRID(Table) ,ButtonConnect, 0 , 1 , 1 , 1);
	
	g_signal_connect (ButtonConnect, 
					 "clicked",
                      G_CALLBACK (GetRepositoryImages), 
					  dm);
    gtk_grid_set_row_spacing(GTK_GRID(Table), 10);
    gtk_grid_set_column_spacing(GTK_GRID(Table), 10);
}
static void InitList(GtkWidget *Box,DockerImagesManege *dm)
{
    GtkWidget *Scrolled;
    GtkWidget *ImagesList;
    GtkTreeModel *model;
    GtkTreeSelection *selection;

    Scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_box_pack_start(GTK_BOX(Box),Scrolled, TRUE, TRUE,0);
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
	gtk_container_add (GTK_CONTAINER (Scrolled), ImagesList);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ImagesList));
    gtk_tree_selection_set_mode(selection,GTK_SELECTION_SINGLE);
    model=gtk_tree_view_get_model(GTK_TREE_VIEW(ImagesList));
    dm->RemoteModel = model;
	dm->RemoteImagesSelect = selection;

}    
GtkWidget *LoadRemoteImages(DockerImagesManege *dm)
{
    GtkWidget *Vbox;
    GtkWidget *Hbox;
    GtkWidget *Hbox1;
    GtkWidget *ButtonRemove;
    GtkWidget *ButtonPull;
//    GtkWidget *ButtonPush;

    Vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_size_request (Vbox,-1,260);
	
    Hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
    gtk_box_pack_start(GTK_BOX(Vbox),Hbox,FALSE,FALSE,6);
   
    ConnectRepository(Hbox,dm);

    InitList(Vbox,dm);

	Hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(Vbox),Hbox1,FALSE,FALSE,0);
	
	ButtonRemove =    gtk_button_new_with_label(_(" Remove  "));
    gtk_box_pack_start(GTK_BOX(Hbox1),ButtonRemove, FALSE, FALSE,0);
	g_signal_connect (ButtonRemove, 
					 "clicked",
                      G_CALLBACK (RemoveImages), 
					  dm);
/*
	ButtonPush =    gtk_button_new_with_label(_(" Push  "));
    gtk_box_pack_start(GTK_BOX(Hbox1),ButtonPush, FALSE, FALSE,0);
	g_signal_connect (ButtonPush, 
					 "clicked",
                      G_CALLBACK (PushImages), 
					  dm);
*/
	ButtonPull =    gtk_button_new_with_label(_(" Pull  "));
    gtk_box_pack_start(GTK_BOX(Hbox1),ButtonPull, FALSE, FALSE,0);
	g_signal_connect (ButtonPull, 
					 "clicked",
                      G_CALLBACK (PullImages), 
					  dm);
	return Vbox;
}		
