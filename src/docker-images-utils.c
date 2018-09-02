#include "docker-images-utils.h"

static size_t WriteFunction(void *data, size_t size, size_t nmemb, void *buf)
{
  	size_t realsize = size * nmemb;
  	struct buffer *mem = (struct buffer *)buf;

  	mem->DockerData = realloc(mem->DockerData, mem->DataSize + realsize + 1);
  	if(mem->DockerData == NULL)
   	{
    	return -1;
  	}
  	memcpy(&(mem->DockerData[mem->DataSize]), data, realsize);
  	mem->DataSize += realsize;
  	mem->DockerData[mem->DataSize] = 0;

  	return realsize;
}

static void InitCurl(DockerClient *dc) 
{
  	curl_easy_setopt(dc->curl, CURLOPT_UNIX_SOCKET_PATH, DOCKERSOCK);
  	curl_easy_setopt(dc->curl, CURLOPT_WRITEFUNCTION, WriteFunction);
  	curl_easy_setopt(dc->curl, CURLOPT_WRITEDATA, dc->Buffer);
}
DockerClient *InitDocker(void)
{
	DockerClient *dc;

	/*get curl version*/
	curl_version();
	
 	dc = (DockerClient *) malloc(sizeof(dc));
	if(dc == NULL)
	{
		return NULL;
	}			
  	dc->Buffer = (struct buffer *) malloc(sizeof(struct buffer));
	if(dc->Buffer == NULL)
	{
		free(dc);
	}
	dc->curl = curl_easy_init();

  	if (dc->curl) 
	{
    	InitCurl(dc);
    	return dc;
  	}

  	return NULL;
}

static void InitBuffer(DockerClient *dc) 
{
	dc->Buffer->DockerData = (char *) malloc(1);
  	dc->Buffer->DataSize = 0;
}

static CURLcode Perform(DockerClient *dc, char *url) 
{
	CURLcode response;
  
	InitBuffer(dc);
  	curl_easy_setopt(dc->curl, CURLOPT_URL, url);
  	response = curl_easy_perform(dc->curl);
  	curl_easy_reset(dc->curl);

  	return response;
}
CURLcode DockerGet(DockerClient *dc, char *url) 
{
  InitCurl(dc);
  return Perform(dc, url);
}
char *GetBuffer(DockerClient *dc) 
{
  return dc->Buffer->DockerData;
}
