#include "docker-images-remote.h"
#include "docker-images-list.h"
#include "docker-images-utils.h"
#include "docker-images-json.h"

static GtkListStore *RemoteListStore = NULL;
static void RemoveRemoteImages(DockerImagesManege *dm)
{
    char MsgBuf[1256] = { 0 };
    int i;
    CURLcode response;
     
    i = dm->SelectIndex;
    sprintf(MsgBuf,"http://%s:%s/api/repositories/%s/tags/%s",
                              dm->Address,
                              dm->Port,
                              dm->dtl[i].ImagesName,
                              dm->dtl[i].ImagesTag);
    response = DockerDeleteRepos(dm->dc,MsgBuf,dm->Name,dm->Pass);
    if (response == CURLE_OK)
    {
        printf("GetBuffer(dm->dc) = %s\r\n",GetBuffer(dm->dc));
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
static void WriteData(gpointer tag,gpointer data)
{
    DockerImagesManege *dm = (DockerImagesManege *)data;

    memset(dm->dtl[ImagesCount].ImagesTag,
          '\0',strlen(dm->dtl[ImagesCount].ImagesTag));
    memcpy(dm->dtl[ImagesCount].ImagesTag,tag,strlen(tag));
    ImagesCount++;
    memset(dm->dtl[ImagesCount].ImagesName,
           '\0',
          strlen(dm->dtl[ImagesCount].ImagesName));
    
    memcpy(dm->dtl[ImagesCount].ImagesName,
           dm->dtl[ImagesCount-1].ImagesName,    
           strlen(dm->dtl[ImagesCount-1].ImagesName));

}    
static int GetImagesTag(char *ImagesName,DockerImagesManege *dm)
{
    CURLcode response;
    GHashTable *hash;
    char tmp[2048] = { 0 };
    char MsgBuf[228] = { 0 };

    sprintf(MsgBuf,"http://%s:%s/api/repositories/%s/tags/",
           dm->Address,dm->Port,ImagesName);
    response = DockerAuthentication(dm->dc,MsgBuf,dm->Name,dm->Pass);
    if (response == CURLE_OK)
    {
        memset(dm->dtl[ImagesCount].ImagesName,
               '\0',
              strlen(dm->dtl[ImagesCount].ImagesName));
        memcpy(dm->dtl[ImagesCount].ImagesName,ImagesName,strlen(ImagesName));
        sprintf(tmp,"{\"\":%s}",GetBuffer(dm->dc));
        hash = AnalyticJson(tmp);
        g_slist_foreach((GSList*)g_hash_table_lookup(hash,"name"),
                        (GFunc)WriteData, dm);
    }
    else
    {
        MessageReport(_("Connect Repository"),
                      curl_easy_strerror(response),
                      ERROR);
        return -1;
    }
    
    return 0;
}    
static void GetValue(gpointer value, gpointer data)
{
    DockerImagesManege *dm = (DockerImagesManege *)data;
    
    GetImagesTag(value,dm);

}
static int GetRemoteImagesInfo (char *data,DockerImagesManege *dm)
{
    GHashTable *hash;
    hash = AnalyticJson(data);
    g_slist_foreach((GSList*)g_hash_table_lookup(hash,"repository_name"),
                    (GFunc)GetValue, dm);
   // g_hash_table_destroy(hash);
}    
static int GetRepositoryImages (GtkWidget *widget, gpointer data)
{
    DockerImagesManege *dm = (DockerImagesManege *)data;
    int i;
    CURLcode response;
    const char *Address;
    const char *Name;
    const char *Pass;
    const char *Port;
    char MsgBuf[128] = { 0 };

    Address = gtk_entry_get_text(GTK_ENTRY(dm->EntryAddress));
    if(IsEmpty(Address) < 0)
        return -1;    
    memset(dm->Address,'\0',sizeof(dm->Address));
    memcpy(dm->Address,Address,strlen(Address));
    
    Port    = gtk_entry_get_text(GTK_ENTRY(dm->EntryPort)); 
    memset(dm->Port,'\0',sizeof(dm->Port));
    memcpy(dm->Port,Port,strlen(Port));
    
    Pass    = gtk_entry_get_text(GTK_ENTRY(dm->EntryPass)); 
    if(IsEmpty(Pass) < 0)
        return -1;    
    memset(dm->Pass,'\0',sizeof(dm->Pass));
    memcpy(dm->Pass,Pass,strlen(Pass));
  
    Name    = gtk_entry_get_text(GTK_ENTRY(dm->EntryName)); 
    if(IsEmpty(Name) < 0)
        return -1;    
    memset(dm->Name,'\0',sizeof(dm->Name));
    memcpy(dm->Name,Name,strlen(Name));
    
    
    if(RemoteListStore != NULL)
    {    
        gtk_list_store_clear(RemoteListStore);
    }    
    sprintf(MsgBuf,"http://%s:%s/api/search?q=",Address,Port);

    response = DockerAuthentication(dm->dc,MsgBuf,Name,Pass);
    if (response == CURLE_OK)
    {
        /* Start parsing data */
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
    GtkWidget *LabelName;
    GtkWidget *EntryName;
    GtkWidget *EntryAddress;
    GtkWidget *LabelPort;
    GtkWidget *LabelPass;
    GtkWidget *EntryPass;
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
	
    LabelName = gtk_label_new(NULL);
    SetLableFontType(LabelName,"red",13,_("Name"));
    gtk_grid_attach(GTK_GRID(Table) , LabelName, 0 , 1 , 1 , 1);

    EntryName = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(EntryName),24);
    dm->EntryName = EntryName;
    gtk_grid_attach(GTK_GRID(Table) ,EntryName , 1 , 1 , 1 , 1);

    LabelPass = gtk_label_new(NULL);
    SetLableFontType(LabelPass,"red",13,_("Pass"));
    gtk_grid_attach(GTK_GRID(Table) , LabelPass, 2 , 1 , 1 , 1);

    EntryPass = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(EntryPass),20);
    dm->EntryPass = EntryPass;
    gtk_grid_attach(GTK_GRID(Table) ,EntryPass , 3 , 1 , 1 , 1);

    ButtonConnect    = gtk_button_new_with_label(_("Connect"));
    gtk_grid_attach(GTK_GRID(Table) ,ButtonConnect, 0 , 2 , 1 , 1);
	
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
    gtk_widget_set_size_request (Vbox,-1,320);
	
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

	ButtonPull =    gtk_button_new_with_label(_(" Pull  "));
    gtk_box_pack_start(GTK_BOX(Hbox1),ButtonPull, FALSE, FALSE,0);
	g_signal_connect (ButtonPull, 
					 "clicked",
                      G_CALLBACK (PullImages), 
					  dm);
*/	return Vbox;
}		
