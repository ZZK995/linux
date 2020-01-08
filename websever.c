#include "wrapper.h"

void process_trans(int fd);
void read_requesthdrs(rio_t *rp);
int is_static(char *uri);
void parse_static_uri(char *uri,char *filename);
void parse_dynamic_uri(char *uri,char *filename,char *cgiargs);
void feed_static(int fd,char *filename,int filesize);
void get_filetype(char *filename,char *filetype);
void feed_dynamic(int fd,char *filename,char *cgiargs);
void error_request(int fd,char *cause,char *errnum,char *shortmsg,char *desprition);

int main(int argc,char **argv)
{
	int listen_sock,conn_sock,port,clientlen;
	struct sockaddr_in clientaddr;

	if(argc!=2){
		fprintf(stderr,"usage:%s<port>\n",argc[0]);
		exit(0);
	}
	port = atoi(argc[1]);

	listen_sock=open_listen_sock(port);
	while(1){
		clientlen=sizeof(clientaddr);
		conn_sock=accpt(listen_sock,(SA *)&clientaddr,&clientlen);
		process_trans(conn_sock);
		close(conn_sock);
	}
}
