#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "docker-images-share.h"

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
    GdkRGBA Rgb;
    PangoFontDescription *font_desc;

    if(gdk_rgba_parse(&Rgb,Color) == TRUE)
        gtk_widget_override_color(Widget, GTK_STATE_NORMAL, &Rgb);

    font_desc = pango_font_description_from_string("Sans Blod 20");
    pango_font_description_set_size (font_desc, FontSize * PANGO_SCALE);
    gtk_widget_modify_font (Widget, font_desc);
    pango_font_description_free (font_desc);
}
