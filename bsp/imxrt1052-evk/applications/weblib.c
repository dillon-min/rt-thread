#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//#include <termios.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>  
//#include <arpa/inet.h>
#include <netdb.h>  
#include <string.h>  
#include "cJSON.h"
#include <errno.h>
#include "weblib.h"
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#define PORT 9517
#define BUFFER_SIZE 1024  
#define HTTP_POST "POST /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n"\
	"Content-Type:application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s"  
#define HTTP_GET "GET /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n\r\n"  
#define MY_HTTP_DEFAULT_PORT 80 

static int http_tcpclient_create(const char *host, int port,int timeout){  
	struct hostent *he;  
	struct sockaddr_in server_addr;
	struct sockaddr_in addr; 
	int socket_fd,nOptval=1; 
	socklen_t lon; 
	int error;
	fd_set rset, wset;
	unsigned int len;
	struct timeval tv; 
	int valopt,ret; 
	int imode=1,i=0;
	char *ptr, **pptr;  
	char str[32] = {0};  
	if((he = gethostbyname(host))==NULL){  
		rt_kprintf(LOG_PREFX"gethostbyname failed\n");
		return -1;  
	}  
#if 0
	rt_kprintf("official hostname:%s\n", he->h_name);    

	for(pptr = he->h_aliases; *pptr != NULL; pptr++)  
		rt_kprintf("alias: %s\n", *pptr);  

	switch(he->h_addrtype)  
	{  
		case AF_INET:  
		case AF_INET6:  
			pptr = he->h_addr_list;  

			for(; *pptr != NULL; pptr++)   
			{
				rt_kprintf("address: %s\n", inet_ntop(he->h_addrtype, *pptr, str, sizeof(str)));
				rt_kprintf("first address: %s\n", inet_ntop(he->h_addrtype, he->h_addr, str, sizeof(str)));  
			}  
			break;  
		default:  
			rt_kprintf("unkown address type\n");  
			break;  
	}  
#endif
	server_addr.sin_family = AF_INET;  
	server_addr.sin_port = htons(port);  
	server_addr.sin_addr.s_addr = inet_addr("121.42.202.99"); //*((struct in_addr *)he->h_addr);  

	if((socket_fd = socket(AF_INET,SOCK_STREAM,0))==-1){  
		rt_kprintf("create socket failed\r\n");
		return -1;  
	}  
	tv.tv_usec=0;
	tv.tv_sec = timeout;
	fcntl(socket_fd, F_SETFL, O_NONBLOCK);
	if(connect(socket_fd, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) <0)
	{
		if(errno!=EINPROGRESS)
		{
			rt_kprintf(LOG_PREFX"connect failed\n");
			close(socket_fd);
			return -1;
		}
	}
	else
	{
		fcntl(socket_fd, F_SETFL, 0);
		return socket_fd;
	}


	/* in progress */
	FD_ZERO(&rset);
	FD_SET(socket_fd, &rset);
	wset = rset;

	ret = select(socket_fd + 1, &rset, &wset, NULL, &tv);
	if (ret == 0) {
		/* no readable or writable socket, timeout */
		close(socket_fd);
		errno = ETIMEDOUT;
		rt_kprintf(LOG_PREFX"Error no readable or writable socket, timeout\n");
		return -1;
	}
	else if (ret < 0) {
		/* select error */
		close(socket_fd);
		rt_kprintf(LOG_PREFX"Error select error\n");
		return -1;
	}
	if (FD_ISSET(socket_fd, &rset) || FD_ISSET(socket_fd, &wset)) {
		/* check is there any error */
		len = sizeof(error);
		getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &error, &len);
	}

	if (error) {
		/* connect error */
		close(socket_fd);
		errno = error;
		rt_kprintf(LOG_PREFX"Error %s\n",strerror(errno));
		return -1;
	}
	return socket_fd;  
}  

static void http_tcpclient_close(int socket){  
	close(socket);  
}  

static int http_tcpclient_recv(int socket,char *lpbuff,int timeout){	
	int recvnum = 0;	
	fd_set rset;
	struct timeval tv; 
	tv.tv_usec=0;
	tv.tv_sec = timeout;

	FD_ZERO(&rset); 
	FD_SET(socket, &rset);
	if(select(socket+1, &rset, NULL, NULL, &tv) > 0) 
	{
		recvnum = recv(socket, lpbuff,BUFFER_SIZE*4,0);  
	}
	else
	{
		rt_kprintf(LOG_PREFX"recv time out\n");
		return -1;
	}
	return recvnum;  
}

static int http_tcpclient_send(int socket,char *buff,int size,int timeout){  
	int sent=0,tmpres=0;	
	fd_set wset;
	struct timeval tv; 
	tv.tv_usec=0;
	tv.tv_sec = timeout;

	while(sent < size)
	{  
		FD_ZERO(&wset);
		FD_SET(socket, &wset);
		if(select(socket+1,NULL, &wset,  NULL, &tv) > 0) 
		{ 
			tmpres = send(socket,buff+sent,size-sent,0);	
			if(tmpres == -1)
			{  
				return -1;  
			}  
			sent += tmpres; 
		}
		else
		{
			rt_kprintf(LOG_PREFX"sent time out\n");
			return -1;
		}
	}  
	return sent;	
}  
static int http_parse_url(const char *url,char *host,char *file,int *port)  
{  
	char *ptr1,*ptr2;  
	int len = 0;  
	if(!url || !host || !file || !port){  
		return -1;  
	}  

	ptr1 = (char *)url;  

	if(!strncmp(ptr1,"http://",strlen("http://"))){  
		ptr1 += strlen("http://");  
	}else{  
		return -1;  
	}  

	ptr2 = strchr(ptr1,'/');  
	if(ptr2){  
		len = strlen(ptr1) - strlen(ptr2);  
		memcpy(host,ptr1,len);  
		host[len] = '\0';  
		if(*(ptr2 + 1)){  
			memcpy(file,ptr2 + 1,strlen(ptr2) - 1 );  
			file[strlen(ptr2) - 1] = '\0';  
		}  
	}else{  
		memcpy(host,ptr1,strlen(ptr1));  
		host[strlen(ptr1)] = '\0';  
	}  
	//get host and ip  
	ptr1 = strchr(host,':');  
	if(ptr1){  
		*ptr1++ = '\0';  
		*port = atoi(ptr1);  
	}else{  
		*port = MY_HTTP_DEFAULT_PORT;  
	}  

	return 0;  
}  



static char *http_parse_result(const char*lpbuf)  
{  
	char *ptmp = NULL;   
	char *response = NULL;  
	ptmp = (char*)strstr(lpbuf,"HTTP/1.1");  
	if(!ptmp){  
		rt_kprintf(LOG_PREFX"http/1.1 not faind\n%s\n",lpbuf);  
		return NULL;  
	}  
	//rt_kprintf(LOG_PREFX"%s",lpbuf);
	if(atoi(ptmp + 9)!=200)
	{  
		if(strstr(lpbuf,"ok")==NULL)
		{
			rt_kprintf(LOG_PREFX"result:\n%s\n",lpbuf);  
			return NULL;  
		}
		else
		{
			char *tmp=malloc(10);
			strcpy(tmp,"ok");
			return tmp;
		}
	}  

	ptmp = (char*)strstr(lpbuf,"\r\n\r\n");  
	if(!ptmp){  
		rt_kprintf(LOG_PREFX"ptmp is NULL\n");  
		return NULL;  
	}  
	response = (char *)malloc(strlen(ptmp)+1);  
	if(!response){  
		rt_kprintf(LOG_PREFX"malloc failed \n");  
		return NULL;  
	}  
	strcpy(response,ptmp+4);  
	return response;  
}  

char * http_post(const char *url,const char *post_str,int timeout){  

	char post[BUFFER_SIZE] = {'\0'};  
	int socket_fd = -1;  
	char lpbuf[BUFFER_SIZE*4] = {'\0'};  
	char *ptmp;  
	char host_addr[BUFFER_SIZE] = {'\0'};  
	char file[BUFFER_SIZE] = {'\0'};  
	int port = 0;  
	int len=0;  
	char *response = NULL;  

	if(!url || !post_str){  
		rt_kprintf(LOG_PREFX"      failed!\n");  
		return NULL;  
	}  

	if(http_parse_url(url,host_addr,file,&port)){  
		rt_kprintf(LOG_PREFX"http_parse_url failed!\n");  
		return NULL;  
	}  
	//rt_kprintf(LOG_PREFX"host_addr : %s\tfile:%s\t,%d\n",host_addr,file,port);  

	socket_fd = http_tcpclient_create(host_addr,port,timeout);  
	if(socket_fd < 0){  
		rt_kprintf(LOG_PREFX"http_tcpclient_create failed\n");  
		return NULL;  
	}  

	srt_kprintf(lpbuf,HTTP_POST,file,host_addr,port,(int)strlen(post_str),post_str);  

	if(http_tcpclient_send(socket_fd,lpbuf,strlen(lpbuf),timeout) < 0){  
		rt_kprintf(LOG_PREFX"http_tcpclient_send failed..\n");  
		return NULL;  
	}  
	//rt_kprintf(LOG_PREFX"POST Sent:\n%s\n",lpbuf);  
	memset(lpbuf,0,BUFFER_SIZE*4);
	/*it's time to recv from server*/  
	if((len=http_tcpclient_recv(socket_fd,lpbuf,timeout)) <= 0){  
		rt_kprintf(LOG_PREFX"http_tcpclient_recv failed\n");  
		return NULL;  
	}

	//	http_tcpclient_recv(socket_fd,lpbuf+len,timeout);

	http_tcpclient_close(socket_fd);  

	return http_parse_result(lpbuf);  
}  

char * http_get(const char *url,int timeout)  
{  

	char post[BUFFER_SIZE] = {'\0'};  
	int socket_fd = -1;  
	char lpbuf[BUFFER_SIZE*4] = {'\0'};  
	char *ptmp;  
	char host_addr[BUFFER_SIZE] = {'\0'};  
	char file[BUFFER_SIZE] = {'\0'};  
	int port = 0;  
	int len=0;  

	if(!url){  
		rt_kprintf(LOG_PREFX"      failed!\n");  
		return NULL;  
	}  

	if(http_parse_url(url,host_addr,file,&port)){  
		rt_kprintf(LOG_PREFX"http_parse_url failed!\n");  
		return NULL;  
	}  
	rt_kprintf(LOG_PREFX"host_addr : %s\tfile:%s\t,%d\n",host_addr,file,port);  

	socket_fd =  http_tcpclient_create(host_addr,port,timeout);  
	if(socket_fd < 0){  
		rt_kprintf(LOG_PREFX"http_tcpclient_create failed\n");  
		return NULL;  
	}  

	sprintf(lpbuf,HTTP_GET,file,host_addr,port);  
	rt_kprintf("lpbuf %s", lpbuf);
	if(http_tcpclient_send(socket_fd,lpbuf,strlen(lpbuf),timeout) < 0){  
		rt_kprintf(LOG_PREFX"http_tcpclient_send failed..\n");  
	http_tcpclient_close(socket_fd);  
		return NULL;  
	}  
	//rt_kprintf(LOG_PREFX"GET Sent:\n%s\n",lpbuf);  
	memset(lpbuf,0,BUFFER_SIZE*4);
	if((len=http_tcpclient_recv(socket_fd,lpbuf,timeout)) <= 0){  
		rt_kprintf(LOG_PREFX"http_tcpclient_recv failed\n");  
	http_tcpclient_close(socket_fd);  
		return NULL;  
	}
	http_tcpclient_close(socket_fd);  

	return http_parse_result(lpbuf);  
}  
char doit_ack(char *text,const char *item_str)
{
	char result=0;cJSON *json,*item_json;

	json=cJSON_Parse(text);
	if (!json) {rt_kprintf(LOG_PREFX"Error ack before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{
		item_json=cJSON_GetObjectItem(json,item_str);
		if((item_json->type & 255) ==cJSON_True)
		{
			result=1;
		}
		cJSON_Delete(json);
	}
	return result;
}
char *doit(char *text,const char *item_str)
{
	char *out=NULL;cJSON *json,*item_json;

	json=cJSON_Parse(text);
	if (!json) {rt_kprintf(LOG_PREFX"Error it before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{
		//out=cJSON_Print(json);
		item_json = cJSON_GetObjectItem(json, item_str);
		if (item_json)
		{
			cJSON *data;
			data=cJSON_GetObjectItem(item_json,item_str);
			if(data)
			{
				int nLen = strlen(data->valuestring);
				out=(char *)malloc(nLen+1);
				memset(out,'\0',nLen+1);
				memcpy(out,data->valuestring,nLen);
			}			
			//rt_kprintf(LOG_PREFX"%s ,%d %s\n",item_str,nLen,item_json->valuestring);
		}
		//else
		//	rt_kprintf(LOG_PREFX"get %s failed\n",item_str);
		cJSON_Delete(json);
		//rt_kprintf(LOG_PREFX"%s\n",out);
		//free(out);
	}
	return out;
}
char *doit_data(char *text,char *item_str)
{
	char *out=NULL;cJSON *item_json;

	item_json=cJSON_Parse(text);
	if (!item_json) {rt_kprintf(LOG_PREFX"Error data before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{

		cJSON *data;
		data=cJSON_GetObjectItem(item_json,item_str);
		if(data)
		{
			int nLen = strlen(data->valuestring);
			//rt_kprintf("%s ,%d %s\n",item_str,nLen,data->valuestring);					
			out=(char *)malloc(nLen+1);
			memset(out,'\0',nLen+1);
			memcpy(out,data->valuestring,nLen);
		}
		//else
		//	rt_kprintf(LOG_PREFX"get %s failed\n",item_str);
		cJSON_Delete(item_json);	
	}
	return out;
}

char *add_item(char *old,char *id,char *text)
{
	cJSON *root;
	char *out;
	if(old!=NULL)
		root=cJSON_Parse(old);
	else
		root=cJSON_CreateObject();	
	cJSON_AddItemToObject(root, id, cJSON_CreateString(text));
	//out=cJSON_Print(root);	
	out=cJSON_PrintUnformatted(root);	
	cJSON_Delete(root);
	if(old)
		free(old);
	return out;
}
char *rm_item(char *old,char *id)
{
	cJSON *root;
	char *out;
	if(old!=NULL)
		root=cJSON_Parse(old);
	else
		root=cJSON_CreateObject();	
	cJSON *data;
	data=cJSON_GetObjectItem(root,id);
	if(data)
	{
		cJSON_DeleteItemFromObject(root, id);
	}
	out=cJSON_PrintUnformatted(root);	
	cJSON_Delete(root);
	if(old)
		free(old);
	return out;
}
char *add_obj(char *old,char *id,char *pad)
{
	cJSON *root,*fmt;
	char *out;
	root=cJSON_Parse(old);
	fmt=cJSON_Parse(pad);
	cJSON_AddItemToObject(root, id, fmt);
	//out=cJSON_Print(root);
	out=cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	cJSON_Delete(fmt);
	free(pad);
	return out;
}
