#define LOG_PREFX "[WebSubSystem]:"
char *doit(char *text,const char *item_str);
char *add_item(char *old,char *id,char *text);
char *add_obj(char *old,char *id,char *pad);
char * http_get(const char *url,int timeout) ;
char * http_post(const char *url,const char *post_str,int timeout);
char *doit_data(char *text,char *item_str);
char *rm_item(char *old,char *id);
