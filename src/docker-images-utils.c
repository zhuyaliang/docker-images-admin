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

static void InitCurl(DockerClient *dc,const char *Socket) 
{
  	curl_easy_setopt(dc->curl, CURLOPT_UNIX_SOCKET_PATH, Socket);
  	curl_easy_setopt(dc->curl, CURLOPT_WRITEFUNCTION, WriteFunction);
  	curl_easy_setopt(dc->curl, CURLOPT_WRITEDATA, dc->Buffer);
}
DockerClient *InitDocker(void)
{
	DockerClient *dc;
	int i = 0;
    int len = 0;
    int count = 16;
    char *p;
    char  **ImageInfo;
    
    /*get curl version*/
    
    p =strstr(curl_version(),"libcurl");
 	if(p == NULL)
    {
        return NULL;
    } 
    while(count--)
    {
        if(p[i] == ' ')
        {
            p[i] = '\0';
                break;
        }   
        i++;
    }    
    ImageInfo = g_strsplit(p,".",-1);
    len = g_strv_length(ImageInfo);
    if(len <= 1)
    {   
        return NULL;
    }    
    if(atoi(ImageInfo[1]) < 40)
    {
        MessageReport(_("Get Curl Version"),
                      _("Curl version needs more than 7.40.0"),
                      ERROR);
    }   
    if(access(DOCKERSOCK,F_OK) != 0)
    {
        
        MessageReport(_("Init Docker"),
                      _("Please start the docker service first."),
                      ERROR);
        return NULL;
    }    
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
CURLcode DockerGet(DockerClient *dc, const char *url,const char *Socket) 
{
  InitCurl(dc,Socket);
  return Perform(dc,url);
}
static size_t realsize;
static size_t HeaderCallback(char *buf, size_t size,
                            size_t nitems, void *data)
{
    size_t datasize;
  	datasize = size * nitems;
    
    if(strstr(buf,"Docker-Content-Digest") != NULL)
    {   
        memcpy((char *)data,&buf[30],strlen(buf)-30);
        realsize = 1;
    }    
  	return datasize;
}
CURLcode DockerQueryDigest(DockerClient *dc, const char *url,char *Return) 
{
	CURLcode response;
    InitCurl(dc,NULL);
    curl_easy_setopt(dc->curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(dc->curl,CURLOPT_HEADERFUNCTION,HeaderCallback);
    curl_easy_setopt(dc->curl, CURLOPT_HEADERDATA, Return);
    response = Perform(dc,url);
    
    return response;
}
char *GetBuffer(DockerClient *dc) 
{
  return dc->Buffer->DockerData;
}

CURLcode DockerPost(DockerClient *dc,
                    const char *url,
                    const char *data,
                    const char *head) 
{
    CURLcode response;
    struct curl_slist *headers = NULL;

    InitCurl(dc,DOCKERSOCK);
    if(head != NULL)
    {    
        headers = curl_slist_append(headers, head);
        curl_easy_setopt(dc->curl, CURLOPT_HTTPHEADER, headers);
    }
    curl_easy_setopt(dc->curl, CURLOPT_POSTFIELDS, (void *)data);
    response = Perform(dc, url);
    if(head != NULL)
        curl_slist_free_all(headers);

    return response;
}
CURLcode DockerDelete(DockerClient *dc, const char *url)
{
    CURLcode response;

    InitCurl(dc,DOCKERSOCK);
    curl_easy_setopt(dc->curl, CURLOPT_CUSTOMREQUEST,"DELETE");
    response = Perform(dc, url);

    return response;
}
