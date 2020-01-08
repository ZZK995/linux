#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include "lx_mt_server.h"
#include "lxtime.h"
#include "lx_http.h"
#include "lx_http_util.h"
#include "lxlog.h"

#define g_ctx (g_lxmt_server_ctx)

struct lxmt_hdarg
{
    lx_connection conn;
};
typedef struct lxmt_hdarg lxmt_hdarg; 

struct lxmt_server_ctx 
{
    int stimeo_milli;
    int rtimeo_milli;
    lx_bool_t is_nostub;
    struct lxlog log;
    struct lxlog_dailyas asarg;
};
typedef struct lxmt_server_ctx lxmt_server_ctx;

static lxmt_server_ctx *g_lxmt_server_ctx;

static char * g_home = "home";
static char * g_whome = "webhome";
static char * g_loghome = "logs";

int init_lxmt_server(lx_bool_t is_nostub,char * phome)
{
    char buff[1024];
    int ret = 0;
    struct lxmt_server_ctx * ctx;
    ctx = (lxmt_server_ctx *)malloc(sizeof(lxmt_server_ctx));
    if( ctx == NULL)
    {
        perror("malloc in init error\n");
        return -1;
    }
    ctx->stimeo_milli = 200000;
    ctx->rtimeo_milli = 200000;
    ctx->is_nostub = is_nostub;
   
    if(phome)
        g_home = phome;
  
    newlxlog( (&ctx->log));
    ctx->asarg.newhour = 18;
    ctx->log.arg = &ctx->asarg;
    if(snprintf(buff,1024,"%s/%s",g_home,g_loghome) <=0 || 
        (ret = lxlog_init(&ctx->log,buff,"access.log", LX_LOG_DEBUG)) ){
        printf("init log error,ret=%d",ret);
        return EXIT_FAILURE;
    }
    ctx->log.flushnow = 1;
    ctx->log.tlockflag = 1;
    ctx->log.plockflag = 0;
    ctx->log.showpid = 1;
   // ctx->log.showtid = 0;

    g_lxmt_server_ctx = ctx;

    return 0;
}
int cleanup_lxmt_server()
{
    if( g_ctx != NULL)
    {
        g_ctx->log.cleanup(&g_ctx->log);     
        free(&g_ctx); 
        g_ctx = NULL; 
    }    
    return 0;
}
static int handler(void *);
static int do_work(void *);

int start_lxmt_server(int port)
{
    int listen_fd;
    lxmt_hdarg arg;
    char buff[1024];

    if( (listen_fd = lx_listen(port)) == -1)
    {
        perror("lx_listen error\n");
        return -1;
    }

    if( getwidetime(time(NULL),buff,1024) <= 0){
       perror("get start time error");
       return -1;
    }
    g_ctx->log.loginfo(&g_ctx->log,"server start at %s",buff);

    if(lx_start_server(listen_fd,handler,&arg ))
    {
        perror("lx_start_server error");
        return -1;
    }

    return 0;
}

