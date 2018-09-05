#include "docker-images-share.h"
#include "docker-images-local.h"
#include "docker-images-remote.h"
#include "docker-images-utils.h"


static GdkPixbuf * GetAppIcon(void)
{
    GdkPixbuf *Pixbuf;
    GError    *Error = NULL;

    Pixbuf = gdk_pixbuf_new_from_file("/usr/share/docker-admin/icon/docker-images-admin.jpg",&Error);
    if(!Pixbuf)
    {
        MessageReport(_("Get Icon Fail"),Error->message,ERROR);
        g_error_free(Error);
    }   
    
    return Pixbuf;
}    
static void InitMainWindow(DockerImagesManege *dm)
{
    GtkWidget *Window;
    GdkPixbuf *AppIcon;

    Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(Window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(Window), _("docker images admin"));
    gtk_container_set_border_width(GTK_CONTAINER(Window),10);
    
    AppIcon = GetAppIcon();
    if(AppIcon)
    {
        gtk_window_set_icon(GTK_WINDOW(Window),AppIcon);
        g_object_unref(AppIcon);
    }    
    dm->MainWindow = Window;
}

static void CreateInterface(DockerImagesManege *dm)
{
    GtkWidget *NoteBook;
   	GtkWidget *LocalBox;	
   	GtkWidget *RemoteBox;
	GtkWidget *LocalNoteName;
	GtkWidget *RemoteNoteName;
	
	NoteBook = gtk_notebook_new();   
 	gtk_container_add(GTK_CONTAINER(dm->MainWindow), NoteBook);
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK (NoteBook), GTK_POS_TOP);
	
	LocalNoteName = gtk_label_new(_("     Local Images   "));
	LocalBox = LoadLocalImages(dm);
	gtk_notebook_append_page(GTK_NOTEBOOK (NoteBook),LocalBox,LocalNoteName);
	
	RemoteNoteName = gtk_label_new(_("    Remote Images   "));
	RemoteBox = LoadRemoteImages(dm);
	gtk_notebook_append_page(GTK_NOTEBOOK (NoteBook),RemoteBox,RemoteNoteName);

}
static int RecordPid(void)
{
    int pid = 0;
    int fd;
    char WriteBuf[30] = { 0 };
    fd = open(LOCKFILE,O_WRONLY|O_CREAT|O_TRUNC,0777);
    if(fd < 0)
    {
         MessageReport(_("open file"),_("Create pid file failed"),ERROR);
         return -1;      
    }       
    chmod(LOCKFILE,0777); 
    pid = getpid();
    sprintf(WriteBuf,"%d",pid);
    write(fd,WriteBuf,strlen(WriteBuf));
    close(fd);

    return 0;
}        
/******************************************************************************
* Function:              ProcessRuning      
*        
* Explain: Check whether the process has been started,If the process is not started, 
*          record the current process ID =====>"/tmp/user-admin.pid"
*        
* Input:         
*        
*        
* Output:  start        :TRUE
*          not start    :FALSE
*        
* Author:  zhuyaliang  31/07/2018
******************************************************************************/
static gboolean ProcessRuning(void)
{
    int fd = 0;
    int pid = 0;
    gboolean Run = FALSE;
    char ReadBuf[30] = { 0 };

    if(access(LOCKFILE,F_OK) == 0)
    {
        fd = open(LOCKFILE,O_RDONLY);
        if(fd < 0)
        {
             MessageReport(_("open file"),_("open pid file failed"),ERROR);
             return TRUE;
        }        
        if(read(fd,ReadBuf,sizeof(ReadBuf)) <= 0)
        {
             MessageReport(_("read file"),_("read pid file failed"),ERROR);
             goto ERROREXIT;
        }        
        pid = atoi(ReadBuf);
        if(kill(pid,0) == 0)
        {        
             goto ERROREXIT;
        }
    }
    
    if(RecordPid() < 0)
        Run = TRUE;
    
    return Run;
ERROREXIT:
    close(fd);
    return TRUE;

}        

int main(int argc, char **argv)
{
    DockerImagesManege dm;
    
    bindtextdomain (PACKAGE, LOCALEDIR);   
    textdomain (PACKAGE); 
    
    gtk_init(&argc, &argv);
    
	/*Initialization docker API and Curl*/
	dm.dc = InitDocker();
	if(dm.dc == NULL)
	{
		exit(1);	
	}			
    /* Create the main window */
    InitMainWindow(&dm);
	g_signal_connect(G_OBJECT(dm.MainWindow), 
                    "delete-event",
                    G_CALLBACK(on_window_quit),
                    &dm);
    /* Check whether the process has been started */
    if(ProcessRuning() == TRUE)
        exit(0);        

    WindowLogin = dm.MainWindow;
   
    /* Create an interface */ 
    CreateInterface(&dm);

    gtk_widget_show_all(dm.MainWindow);
    gtk_main();

}
