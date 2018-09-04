#ifndef __DOCKER_IMAGES_LIST_H__
#define __DOCKER_IMAGES_LIST_H__

#include "docker-images-share.h"
enum
{
    INT_COLUMN = 0,
    LIST_NAME ,
    LIST_TAG,
    LIST_ID,
    LIST_SIZE,
    LIST_COLOR,
    LIST_FRONT,
    N_COLUMNS
};

GtkTreeModel  *ListModelCreate(DockerImagesManege *dm);
void ListViewInit(GtkWidget *list);
void ImagesListAppend(GtkWidget *list,
                      const gchar *ImagesName,
                      const gchar *ImagesTag,
                      const gchar *ImagesId,
                      const gchar *ImagesSize,
                      const gchar *Color,
                      int Index,
                      GtkTreeIter *Iter,
                      GtkListStore **ListStore);


#endif

