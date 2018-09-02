#ifndef __DOCKER_IMAGES_SHARE_H__
#define __DOCKER_IMAGES_SHARE_H__

#include <gtk/gtk.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include <libintl.h> // gettext 库支持  
#include <locale.h> // 本地化locale的翻译支持  

#define  _(STRING)  gettext(STRING)   
#define  PACKAGE    "docker-images-manage"   
#define  LOCALEDIR  "/usr/share/locale/" 
#define  LOCKFILE   "/tmp/docker-images.pid"
#define  TYPEMSG    "<span foreground='red'font_desc='13'>%s </span>"
#define  ERROR      1
#define  WARING     2
#define  INFOR      3
#define  QUESTION   4

typedef struct
{
    char ImagesName[128];
    char ImagesTag[20];
    char ImagesId[12];
    char ImagesSzie[20];
    GtkTreeIter Iter;
}ImagesInfoList;

typedef struct
{
    ImagesInfoList dll[200];
    ImagesInfoList dtl[200];
    GtkWidget     *MainWindow;
    GtkWidget     *NoteBook;
    GtkListStore  *ListSTore;
    GtkWidget     *LocalImagesList;
    GtkWidget     *RemoteImagesList;
    GtkTreeModel  *LocalModel;
    GtkTreeModel  *RemoteModel;
	GtkTreeSelection *LocalImagesSelect;
	GtkTreeSelection *RemoteImagesSelect;
}DockerImagesManege;

GtkWidget *WindowLogin;

gboolean on_window_quit (GtkWidget *widget,
                         GdkEvent *event,
                         gpointer user_data);

int MessageReport(const char *Title,const char *Msg,int nType);
void SetLableFontType(GtkWidget *Lable,
				      const char *Color,
					  int FontSzie,
					  const char *Word);



#endif
