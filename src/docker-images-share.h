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
#include <curl/curl.h>
#include <json-c/json_tokener.h>
#include <json-c/json.h>

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

struct buffer
{
    char *DockerData;
    size_t DataSize;
};

typedef struct docker
{
    CURL *curl;
    struct buffer *Buffer;
}DockerClient;

typedef struct
{
    char ImagesName[148];
    char ImagesTag[24];
    char ImagesSize[34];
    char ImagesId[13];
    char RepoDigests[148];
    GtkTreeIter Iter;
}ImagesInfoList;

typedef struct
{
    GtkWidget *NewImagesName;
    GtkWidget *NewImagesTag;
}RenameImageInfo;

typedef struct
{
    ImagesInfoList   dll[200];
    ImagesInfoList   dtl[200];
    RenameImageInfo  ReImage;  
    char             Address[128];
    char             Name[128];
    char             Port[10];
    char             Pass[128];
    char             Tmp[256];
    int              SelectIndex;
	DockerClient     *dc;
    GtkWidget        *MainWindow;
    GtkWidget        *OpreateWindow;
    GtkWidget        *PushWindow;
    GtkWidget        *RenameWindow;
    GtkWidget        *NoteBook;
    GtkListStore     *ListSTore;
    GtkWidget        *LocalImagesList;
    GtkWidget        *RemoteImagesList;
    GtkTreeModel     *LocalModel;
    GtkTreeModel     *RemoteModel;
	GtkTreeSelection *LocalImagesSelect;
	GtkTreeSelection *RemoteImagesSelect;
    GtkWidget        *EntryAddress;
    GtkWidget        *EntryPort;
    GtkWidget        *EntryName;
    GtkWidget        *EntryPass;
    GtkWidget        *EntryProject;
}DockerImagesManege;

GtkWidget *WindowLogin;
GCond Cond;
gboolean on_window_quit (GtkWidget *widget,
                         GdkEvent *event,
                         gpointer user_data);

int MessageReport(const char *Title,const char *Msg,int nType);
void SetLableFontType(GtkWidget *Lable,
				      const char *Color,
					  int FontSzie,
					  const char *Word);

void SetWidgetStyle(GtkWidget * Widget,const char *Color,int FontSize);
int CheckEmpty(GtkWidget *EntryWidget);
int CheckNetwork(const char *ip,const char *port);
int ChangeTag(DockerClient *dc, 
              const char *ip,
              const char *layer,
              const char *name,
              const char *tag);
int IsEmpty(const char *str);
#endif
