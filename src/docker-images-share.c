#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "docker-images-share.h"
#include "docker-images-utils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
gboolean on_window_quit (GtkWidget *widget, 
                         GdkEvent *event, 
                         gpointer user_data)
{
	DockerImagesManege *dm = (DockerImagesManege *)user_data;
  	free(dm->dc->Buffer->DockerData);
  	free(dm->dc->Buffer);
    curl_easy_cleanup(dm->dc->curl);
  	free(dm->dc);

	gtk_main_quit();
    return TRUE;
}
/******************************************************************************
* Function:            MessageReport
*        
* Explain: Prompt information dialog
*          
* Input:  @Title           Message title
*         @Msg             Message content           
*         @nType           Message type
* Output: 
*        
* Author:  zhuyaliang  25/05/2018
******************************************************************************/
int MessageReport(const char *Title,const char *Msg,int nType)
{
    GtkWidget *dialog = NULL;
    int nRet;

    switch(nType)
    {
        case ERROR:
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(WindowLogin),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_ERROR,
                                            GTK_BUTTONS_OK,
                                            "%s",Title);
            break;
        }
        case WARING:
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(WindowLogin),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_WARNING,
                                            GTK_BUTTONS_OK,
                                            "%s",Title);
            break;
        }
        case INFOR:
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(WindowLogin),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_INFO,
                                            GTK_BUTTONS_OK,
                                            "%s",Title);
            break;
        }
        case QUESTION:
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(WindowLogin),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_MESSAGE_QUESTION,
                                            GTK_BUTTONS_YES_NO,
                                            "%s",Title);
            gtk_dialog_add_button (GTK_DIALOG (dialog),_("_Return"),
                                   GTK_RESPONSE_ACCEPT);
            break;
        }
        default :
            break;

    }
    gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),
                                               TYPEMSG,
                                               Msg);
    gtk_window_set_title(GTK_WINDOW(dialog),_("Message"));
    nRet =  gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return nRet;
}

/******************************************************************************
* Function:            SetLableFontType
*        
* Explain: Tips the style of the text
*          
* Input:  @GtkWidget       lable
*         @Color           
*         @FontSize
*         @Word            text
* Output: 
*        
* Author:  zhuyaliang  25/05/2018
******************************************************************************/
void SetLableFontType(GtkWidget *Lable ,
                      const char *Color,
                      int FontSzie,
                      const char *Word)        
{
    char LableTypeBuf[200] = { 0 };
    
    sprintf(LableTypeBuf,
           "<span foreground=\'%s\'weight=\'light\'font_desc=\'%d\'>%s</span>",
            Color,FontSzie,Word);
    gtk_label_set_markup(GTK_LABEL(Lable),LableTypeBuf);
}
void SetWidgetStyle(GtkWidget * Widget,const char *Color,int FontSize)
{
    /*
    GdkRGBA Rgb;
    PangoFontDescription *font_desc;

    if(gdk_rgba_parse(&Rgb,Color) == TRUE)
        gtk_widget_override_color(Widget, GTK_STATE_NORMAL, &Rgb);

    font_desc = pango_font_description_from_string("Sans Blod 20");
    pango_font_description_set_size (font_desc, FontSize * PANGO_SCALE);
    gtk_widget_modify_font (Widget, font_desc);
    pango_font_description_free (font_desc);
*/
}
int CheckEmpty(GtkWidget *EntryWidget)
{
    char *p = NULL;
    p = gtk_entry_get_text(GTK_ENTRY(EntryWidget));
    
    return strlen(p);
}

int CheckNetwork(const char *ip,const char *port)
{
    int Sockfd;
    struct sockaddr_in Addr;
    unsigned long  addnet; 
    struct hostent *host;

    addnet = inet_addr(ip);
    if (addnet == INADDR_NONE)
    {
        host = gethostbyname(ip);
        if(host == NULL)
        {
            return -1;
        }    
        ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);
    }
    if ((Sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        return -1;
    }
    Addr.sin_family = AF_INET;
    Addr.sin_port = htons(atoi(port));
    if(inet_pton(AF_INET, ip,&Addr.sin_addr) <= 0)
    {
        close(Sockfd);
        return -1;
    }    
    bzero(&(Addr.sin_zero),8);
    
    if(connect(Sockfd, 
              (struct sockaddr *)&Addr,  
              sizeof(struct sockaddr)) < 0)
    {
        close(Sockfd);
        return -1;
    }
    
    close(Sockfd);
    return 0;
}   
int ChangeTag(DockerClient *dc,
              const char *OldName,
              const char *OldTag,
              const char *NewName,
              const char *NewTag)
{
    char MsgBuf[128] = { 0 };
    CURLcode response;

    sprintf(MsgBuf,"http:/v1.25/images/%s:%s/tag?repo=%s:%s&tag",
                    OldName,
                    OldTag,
                    NewName,
                    NewTag);
    response = DockerPost(dc,MsgBuf,NULL,NULL);

    return response;
}
int IsEmpty(const char *str)
{
    if(strlen(str) <= 0)
    {
        MessageReport(_("Entry input"),
                      _("Input parameter can not be empty."),
                      ERROR);
        return -1;
    }
    return 0;
}

