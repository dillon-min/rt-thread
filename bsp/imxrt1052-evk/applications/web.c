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
#include "weblib.h"
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

void dump_info(char *text)
{
	cJSON *item_json;
	int i;

	item_json=cJSON_Parse(text);
	if (!item_json) {rt_kprintf(LOG_PREFX"Error data before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{

		cJSON *data;
		data=cJSON_GetObjectItem(item_json,"img_height");
		if(data)
			rt_kprintf("img_height\t %d\r\n",data->valueint);					
		else
			rt_kprintf("can not find img_height\r\n");
		data=cJSON_GetObjectItem(item_json,"img_width");
		if(data)
			rt_kprintf("img_width\t %d\r\n",data->valueint);					
		else
			rt_kprintf("can not find img_width\r\n");
		data=cJSON_GetObjectItem(item_json,"img_id");
		if(data)
			rt_kprintf("img_id\t\t %s\r\n",data->valuestring);					
		else
			rt_kprintf("can not find img_id\r\n");
		data=cJSON_GetObjectItem(item_json,"url");
		if(data)
			rt_kprintf("url\t\t %s\r\n",data->valuestring);					
		else
			rt_kprintf("can not find url\r\n");
		data=cJSON_GetObjectItem(item_json,"face");
		if(data) {
			int face_cnt = cJSON_GetArraySize(data);
			for (i=0; i<face_cnt; i++) {
				rt_kprintf("FACE[%d]\r\n", i);
				cJSON *face = cJSON_GetArrayItem(data, i);
				
				cJSON *attribute = cJSON_GetObjectItem(face, "attribute");
				cJSON *tmp = cJSON_GetObjectItem(attribute, "age");
				rt_kprintf("\tage\t\t\t %d\r\n", tmp->valueint);
				tmp = cJSON_GetObjectItem(attribute, "gender");
				rt_kprintf("\tgender\t\t\t %s\r\n", tmp->valuestring);
				tmp = cJSON_GetObjectItem(attribute, "lefteye_opendegree");
				rt_kprintf("\tlefteye_opendegree\t %d\r\n", tmp->valueint);
				tmp = cJSON_GetObjectItem(attribute, "mouth_opendegree");
				rt_kprintf("\tmouth_opendegree\t %d\r\n", tmp->valueint);
				
				tmp = cJSON_GetObjectItem(attribute, "pose");
				cJSON *tmp1 = cJSON_GetObjectItem(tmp, "tilting");
				rt_kprintf("\ttilting\t\t\t %d\r\n", tmp1->valueint);
				tmp1 = cJSON_GetObjectItem(tmp, "raise");
				rt_kprintf("\traise\t\t\t %d\r\n", tmp1->valueint);
				tmp1 = cJSON_GetObjectItem(tmp, "turn");
				rt_kprintf("\tturn\t\t\t %d\r\n", tmp1->valueint);
				
				tmp = cJSON_GetObjectItem(attribute, "righteye_opendegree");
				rt_kprintf("\trighteye_opendegree\t %d\r\n", tmp->valueint);
				tmp = cJSON_GetObjectItem(face, "face_id");
				rt_kprintf("\tface_id\t\t\t %s\r\n", tmp->valuestring);
				
				tmp = cJSON_GetObjectItem(face, "position");
				tmp1 = cJSON_GetObjectItem(tmp, "center");
				cJSON *tmp2 = cJSON_GetObjectItem(tmp1, "x");
				rt_kprintf("\tcenter.x\t\t %d\r\n", tmp2->valueint);
				tmp2 = cJSON_GetObjectItem(tmp1, "y");
				rt_kprintf("\tcenter.y\t\t %d\r\n", tmp2->valueint);
				tmp1 = cJSON_GetObjectItem(tmp, "eye_left");
				tmp2 = cJSON_GetObjectItem(tmp1, "x");
				rt_kprintf("\teye_left.x\t\t %d\r\n", tmp2->valueint);
				tmp2 = cJSON_GetObjectItem(tmp1, "y");
				rt_kprintf("\teye_left.y\t\t %d\r\n", tmp2->valueint);
				tmp1 = cJSON_GetObjectItem(tmp, "eye_right");
				tmp2 = cJSON_GetObjectItem(tmp1, "x");
				rt_kprintf("\teye_right.x\t\t %d\r\n", tmp2->valueint);
				tmp2 = cJSON_GetObjectItem(tmp1, "y");
				rt_kprintf("\teye_right.y\t\t %d\r\n", tmp2->valueint);
				
				tmp1 = cJSON_GetObjectItem(tmp, "height");
				rt_kprintf("\theight\t\t\t %d\r\n", tmp1->valueint);
				tmp1 = cJSON_GetObjectItem(tmp, "width");
				rt_kprintf("\twidth\t\t\t %d\r\n", tmp1->valueint);
				tmp = cJSON_GetObjectItem(face, "tag");
				rt_kprintf("\ttag\t\t %s\r\n", tmp->valuestring);
			}
		}
		else
			rt_kprintf("can not find face\n");
		cJSON_Delete(item_json);	
	}
}
int upload_data(char *url,char *appid,char *appkey,char *url2, int timeout)
{
	int result=0;
	char *message=NULL,*rcv=NULL;
	message = (char *)rt_malloc(strlen(url)+strlen(appid)+strlen(appkey)
			+strlen(url2)+32);
	sprintf(message, "%s?app_id=%s&app_key=%s&url=%s",url,appid,appkey,url2);
	rt_kprintf("<GET> %s\r\n",message);
	rcv=http_get(message,timeout);

	rt_free(message);
	if(rcv!=NULL)
	{	
		rt_kprintf("<=== %s\r\n",rcv);
		char *res_code=NULL;
		char *res_message=NULL;
		res_code=doit_data(rcv,(char *)"res_code");
		res_message=doit_data(rcv,(char *)"message");
		rt_kprintf("res_code\t %s\r\n",res_code);
		if (atoi(res_code) == 0) {
			dump_info(rcv);	
		} else
			rt_kprintf("message\t\t %s\r\n", res_message);
		if(res_code!=NULL)
		{
			free(res_code);
		}
		if(res_message!=NULL)
		{
			free(res_message);
		}
		result=1;
		free(rcv);
	}
	return result;
}

/*get cmd http://api.eyekey.com/face/Check/checking?
  app_id=f89ae61fd63d4a63842277e9144a6bd2&
  amp;app_key=af1cd33549c54b27ae24aeb041865da2&
  url=http%3A%2F%2Fpicview01.baomihua.com%2Fphotos%2F20120713%2
  Fm_14_634778197959062500_40614445.jpg
  */
//ack {"message":"从 url [null] 获取图片出错","res_code":"1011"}
#define URL 	"http://api.eyekey.com/face/Check/checking"
#define APPID	"f89ae61fd63d4a63842277e9144a6bd2"
#define APPKEY	"af1cd33549c54b27ae24aeb041865da2"
//#define URL2 	"http://5b0988e595225.cdn.sohucs.com/images/20180525/c7ab772c69a24db6b6e8dcb702edaf28.jpeg"
#define URL2 	"http://06.imgmini.eastday.com/mobile/20180526/20180526031905_f8c6334daca13ce802e608d9b9c910ff_1.jpeg"
int web(int argc, void* argv[])
{
	rt_kprintf("url:\t\t %s\r\napp_id:\t\t %s\r\napp_key:\t %s\r\nurl2:\t\t %s\n\n",
			URL,APPID,APPKEY,argv[1]);		

	if(upload_data(URL,APPID,APPKEY,argv[1],99))
		rt_kprintf("xfer data ok\n");
	else
		rt_kprintf("xfer data failed\n");

	return ;
}
#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(web, test socket);
#endif
