#include "docker-images-local.h"
#include "docker-images-list.h"
#include "docker-images-utils.h"

static GtkListStore *ListStore = NULL;
static int LocalImagesCount;
static int DerivedFlag;
static int GetImagesInfo(char *data,DockerImagesManege *dm);
static void json_parse_array( json_object *jobj, const char *key,DockerImagesManege *dm);
static void RefreshImagesList( DockerImagesManege *dm )
{
    int i;
	int len = 0;
	CURLcode response;
    if(ListStore != NULL)
    {    
        gtk_list_store_clear(ListStore);
    }    
	response = DockerGet(dm->dc, "http://v1.25/images/json",DOCKERSOCK);
	if (response == CURLE_OK) 
	{
        LocalImagesCount = 0;
    	len = GetImagesInfo(GetBuffer(dm->dc),dm);
    }
	else
	{
		MessageReport(_("Get Images Fail"),_("Curl GET Error"),ERROR);
	}		
    for( i = 0; i < LocalImagesCount; i ++)
    {
        ImagesListAppend(dm->LocalImagesList,
						 dm->dll[i].ImagesName,
						 dm->dll[i].ImagesTag,
						 dm->dll[i].ImagesId,
						 dm->dll[i].ImagesSize,
						  "blue",
						 i,
                         &dm->dll[i].Iter,
                         &ListStore);
    }

}    

static void RemoveImages (GtkWidget *widget, gpointer data)
{
    GtkTreeIter iter;
    gint i;
    GtkTreePath *path;
    CURLcode response;
    char MsgBuf[128] = { 0 };

    DockerImagesManege *dm = (DockerImagesManege *)data;

    if (gtk_tree_selection_get_selected (dm->LocalImagesSelect, NULL, &iter))
    {
        path = gtk_tree_model_get_path (dm->LocalModel, &iter);
        i = gtk_tree_path_get_indices (path)[0];
        sprintf(MsgBuf,"%s/%s", "http://v1.25/images",dm->dll[i].ImagesId);
	    response = DockerDelete(dm->dc, MsgBuf);
	    if (response == CURLE_OK)
        {
            gtk_list_store_remove (GTK_LIST_STORE (dm->LocalModel), &iter);
        }
        gtk_tree_path_free (path);
    }
}

static void RunImages (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  DockerImagesManege *dm = (DockerImagesManege *)data;

  if (gtk_tree_selection_get_selected (dm->LocalImagesSelect, NULL, &iter))
    {
      //gint i;
      GtkTreePath *path;

      path = gtk_tree_model_get_path (dm->LocalModel, &iter);
      //i = gtk_tree_path_get_indices (path)[0];
	  gtk_tree_path_free (path);
    }
}

static void RefreshImages (GtkWidget *widget, gpointer data)
{
    DockerImagesManege *dm = (DockerImagesManege *)data;
    RefreshImagesList(dm);
}

static void CreateOperateWin(DockerImagesManege *dm)
{
    GtkWidget *OpreateWindow;
    GtkWidget *MainVbox;
    GtkWidget *Hbox;
    GtkWidget *FrameHorz;
    GtkWidget *PushButton;
    GtkWidget *TagButton;
    GtkWidget *LoadButton;
    GtkWidget *SaveButton;

    OpreateWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(OpreateWindow),_("Opreate Images"));
    gtk_window_set_default_size(GTK_WINDOW(OpreateWindow), 230, 150);
    gtk_window_set_position(GTK_WINDOW(OpreateWindow), GTK_WIN_POS_MOUSE); 
    g_signal_connect (OpreateWindow, "destroy",
                      G_CALLBACK (gtk_widget_destroyed),
                      &OpreateWindow);

    gtk_container_set_border_width (GTK_CONTAINER (OpreateWindow), 10);

    MainVbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (OpreateWindow),MainVbox);

    FrameHorz = gtk_frame_new (_("Supported operations"));
    gtk_frame_set_label_align(GTK_FRAME(FrameHorz),0.5,0.3);
    Hbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_container_set_border_width (GTK_CONTAINER (Hbox), 5);
    gtk_container_add (GTK_CONTAINER (FrameHorz), Hbox);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (Hbox),GTK_BUTTONBOX_SPREAD);
    gtk_box_set_spacing (GTK_BOX (Hbox),40);

    PushButton = gtk_button_new_with_label (_("Push"));
    gtk_container_add (GTK_CONTAINER (Hbox), PushButton);

    TagButton = gtk_button_new_with_label (_("Rename"));
    gtk_container_add (GTK_CONTAINER (Hbox), TagButton);

    LoadButton = gtk_button_new_with_label (_("Load"));
    gtk_container_add (GTK_CONTAINER (Hbox), LoadButton);

    SaveButton = gtk_button_new_with_label (_("Save"));
    gtk_container_add (GTK_CONTAINER (Hbox), SaveButton);
    
    gtk_box_pack_start (GTK_BOX (MainVbox),
                        FrameHorz,
                        TRUE, TRUE, 0);
    gtk_widget_show_all (OpreateWindow);
    //gtk_widget_destroy (window);
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
	    dm->SelectIndex = i;
        CreateOperateWin(dm);
        gtk_tree_path_free (path);
    }
}
static int ExtractNameTag(int index,char *data,DockerImagesManege *dm)
{
    char  **ImageInfo;
    int len = 0;
    const char *tag;
    
    ImageInfo = g_strsplit(data,":",-1);
    len = g_strv_length(ImageInfo);
    if(len == 1)
    {
	    memset(dm->dll[index].ImagesName,'\0',strlen(dm->dll[index].ImagesName));
        memcpy(dm->dll[index].ImagesName,"<none>",6);
	    memset(dm->dll[index].ImagesTag,'\0',strlen(dm->dll[index].ImagesTag));
        memcpy(dm->dll[index].ImagesTag,"<none>",6);
    }
    else if(len < 1)
    {
        g_strfreev(ImageInfo);
        return len;
    }
    else
    {
        tag = g_strrstr(data,":");
	    memset(dm->dll[index].ImagesTag,'\0',strlen(dm->dll[index].ImagesTag));
        memcpy(dm->dll[index].ImagesTag,&tag[1],(strlen(tag)-1));
        memset(dm->dll[index].ImagesName,'\0',strlen(dm->dll[index].ImagesName));
        memcpy(dm->dll[index].ImagesName,
               data,
              (strlen(data)-strlen(tag)));
    }
    g_strfreev(ImageInfo);
	return len;
}
static int ExtractId(int index,char *data,DockerImagesManege *dm)
{
    if(strlen(data) < 71 || strstr(data,"sha256:") == NULL)
        return -1;
    memset(dm->dll[index].ImagesId,'\0',strlen(dm->dll[index].ImagesId));
    memcpy(dm->dll[index].ImagesId,&data[7],12);
	return 0;
}
static int ExtractSzie(int index,int size,DockerImagesManege *dm)
{
    char Sizebuf[30] = { 0 };
    float _size = 0.0;
    if(size <= 0)
        return -1;
    
    if(size <= 1000)
    {
        sprintf(Sizebuf,"%d %s",size, "K");
        memset(dm->dll[index].ImagesSize,'\0',strlen(dm->dll[index].ImagesSize));
        memcpy(dm->dll[index].ImagesSize,Sizebuf,strlen(Sizebuf));
    }    
    else if(size > 1000 && size <=1000000)
    {
        _size = size / 1000.00;
        sprintf(Sizebuf,"%.2f %s",_size, "KB");
        memset(dm->dll[index].ImagesSize,'\0',strlen(dm->dll[index].ImagesSize));
        memcpy(dm->dll[index].ImagesSize,Sizebuf,strlen(Sizebuf));
    }
    else if(size > 1000000 && size <= 1000000000)
    {
        _size = size / (1000.00 * 1000.00);
        sprintf(Sizebuf,"%.2f %s",_size, "MB");
        memset(dm->dll[index].ImagesSize,'\0',strlen(dm->dll[index].ImagesSize));
        memcpy(dm->dll[index].ImagesSize,Sizebuf,strlen(Sizebuf));
    }    
    else 
    {
        _size = size / (1000.00 * 1000.00 *1000.00);
        sprintf(Sizebuf,"%.2f %s",_size, "GB");
        memset(dm->dll[index].ImagesSize,'\0',strlen(dm->dll[index].ImagesSize));
        memcpy(dm->dll[index].ImagesSize,Sizebuf,strlen(Sizebuf));
    
    } 
    
	return 0;
}
 /*printing the value corresponding to boolean, double, integer and strings*/
static void print_json_value(json_object *jobj,const char *key,DockerImagesManege *dm)
{
    char *RepoDigests; 
    char *RepoTags;
    char **ImageInfo;
    int len = 0;

    if(strcmp(key,"Id") == 0)
    {
        ExtractId(LocalImagesCount, json_object_get_string(jobj),dm);         
    }
    else if(strcmp(key,"Size") == 0)
    {
        ExtractSzie(LocalImagesCount,json_object_get_int(jobj),dm); 
        if(DerivedFlag == 1)
        {   
            LocalImagesCount++;
            ExtractSzie(LocalImagesCount,json_object_get_int(jobj),dm);    
            DerivedFlag = 0;
        }
    }    
    else if(strcmp(key,"RepoDigests") == 0)
    {
        memset(dm->dll[LocalImagesCount].RepoDigests,
              '\0',
              strlen(dm->dll[LocalImagesCount].RepoDigests));
        RepoDigests = json_object_get_string(jobj);
        ImageInfo = g_strsplit(RepoDigests,"@sha256" ,-1);
        len = g_strv_length(ImageInfo);
        if(len <= 1)
        {    
            memcpy(dm->dll[LocalImagesCount].RepoDigests,"none:none",9);    
        }
        else
        {    
            sprintf(dm->dll[LocalImagesCount].RepoDigests,"%s:%s",ImageInfo[0],"none");
        }
        g_strfreev(ImageInfo); 
    }    
    else if(strcmp(key,"RepoTags") == 0)
    {
        RepoTags = json_object_get_string(jobj);
        if(RepoTags == NULL)
        {
            ExtractNameTag(LocalImagesCount,dm->dll[LocalImagesCount].RepoDigests,dm);
        }    
        else
            ExtractNameTag(LocalImagesCount,RepoTags,dm); 
    } 
}
static void SetJsonArray(json_object *jobj,const char *key,DockerImagesManege *dm)
{
    memset(dm->dll[LocalImagesCount + 1].ImagesId,
          '\0',
          strlen(dm->dll[LocalImagesCount + 1].ImagesId));
    memcpy(dm->dll[LocalImagesCount + 1].ImagesId,
           dm->dll[LocalImagesCount ].ImagesId,
           strlen(dm->dll[LocalImagesCount].ImagesId));
    DerivedFlag = 1;
    ExtractNameTag(LocalImagesCount + 1,json_object_get_string(jobj),dm);
}    
void json_parse(json_object *jobj,DockerImagesManege *dm); /*Forward Declaration*/
static void json_parse_array( json_object *jobj, const char *key,DockerImagesManege *dm)
{
    
    enum json_type type;
    json_object *jarray; /*Simply get the array*/
    int arraylen;
    int i;
    json_object * jvalue;

    jarray = jobj;
    if(key)
    {
        jarray = json_object_object_get(jobj, key); /*Getting the array if it is a key value pair*/
    }
    arraylen = json_object_array_length(jarray); /*Getting the length of the array*/

    for (i=0; i< arraylen; i++)
    {
        jvalue = json_object_array_get_idx(jarray, i); /*Getting the array element at position i*/
        type = json_object_get_type(jvalue);
        if (type == json_type_array) 
        {
            json_parse_array(jvalue, NULL,dm);
        }   
        else if (type != json_type_object)
        {
            if(strcmp(key,"RepoTags") == 0 && i >=1)
            {
                SetJsonArray(jvalue,key,dm);
            }   
            else
            {    
                print_json_value(jvalue,key,dm);
            }
        }    
        else 
        {
            json_parse(jvalue,dm);
        }   
  }
}
static void JsonSplit(char *data,DockerImagesManege *dm)
{
    json_object * jobj = json_tokener_parse(data);
    json_parse(jobj,dm);
}

/*Parsing the json object*/
void json_parse(json_object * jobj,DockerImagesManege *dm)
{
    enum json_type type;
    json_object_object_foreach(jobj, key, val)  /*Passing through every array element*/
    {
        type = json_object_get_type(val);
        if(strcmp(key,"Labels") == 0)
        {
            continue;
        }    
        switch (type)
        {
            case json_type_boolean:
            case json_type_double:
            case json_type_int:
            case json_type_string:
                print_json_value(val,key,dm);
                break;
            case json_type_object:
                jobj = json_object_object_get(jobj, key);
                json_parse(jobj,dm);
                break;
            case json_type_array:
                json_parse_array(jobj, key,dm);
                break;
            case json_type_null:
                if(strcmp(key,"RepoTags") == 0)
                {
                    print_json_value(val,key,dm);
                }    
                break;
            default:
                break;
        }
    }

}

static int GetDelimiter(const char *data,char *Delimit)
{
    int i = 0;

    while(1)
    {
        if(data[i+1] != ':')
        {    
            Delimit[i] = data[i+1];
            i++;
        }
        else
            break;
    } 
    
    return i;
}    
static int GetImagesInfo(char *data,DockerImagesManege *dm)
{
    char **ImageInfo;
    char Delimit[20] = { 0 };
    char paragraph[512] = { 0 };
    int len = 0;
    int j;
    int ll = 0;

    
    data[strlen(data)-2] = '\0';
    GetDelimiter(data,Delimit);
    ImageInfo = g_strsplit(data,Delimit ,-1);
    len = g_strv_length(ImageInfo);
    
    for(j = 1 ; j < len ; j++)
    {
        ll = strlen(ImageInfo[j]);
        if(ImageInfo[j][ll -1] != '}')
            ImageInfo[j][ll -1] = '\0';
        sprintf(paragraph,"%s%s",Delimit,ImageInfo[j]);
        JsonSplit(paragraph,dm);
        LocalImagesCount ++; 
        memset(paragraph,'\0',strlen(paragraph));
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
	GtkWidget *ButtonRefresh;
	GtkWidget *ButtonOperation;
	GtkWidget *ButtonRun;

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

    RefreshImagesList(dm);

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

	ButtonRefresh =  gtk_button_new_with_label(_("  Refresh "));
    gtk_box_pack_start(GTK_BOX(Hbox),ButtonRefresh, FALSE, FALSE,0);
	g_signal_connect (ButtonRefresh, 
					 "clicked",
                      G_CALLBACK (RefreshImages), 
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
