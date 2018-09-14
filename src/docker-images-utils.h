#ifndef __DOCKER_IMAGES_UTILS_H__
#define __DOCKER_IMAGES_UTILS_H__

#include "docker-images-share.h"

#define   DOCKER_API_VERSION    v1.25
#define   DOCKERSOCK            "/var/run/docker.sock" 

DockerClient *InitDocker(void);
CURLcode      DockerGet(DockerClient *dc, const char *url,const char *Socket);
CURLcode      DockerDelete(DockerClient *dc, const char *url);
CURLcode      DockerQueryDigest(DockerClient *dc, const char *url,char *Return);
CURLcode DockerPost(DockerClient *dc,
                    const char *url,
                    const char *data,
                    const char *head);
char         *GetBuffer(DockerClient *dc); 

#endif 
