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

void process_trans(int fd)
{
	int static_flag;
	struct stat sbuf;
	char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
	char filename[MAXLINE],cgiargs[MAXLINE];
	rio_t rio;

	rio_readinitb(&rio,fd);
	rio_readlineb(&rio,buf,MAXLINE);
	sscanf(buf,"%s %s %s",method,uri,version);
	if(strcasecmp(method,"GET")){
		error_request(fd,method,"501","Not Implemented","web does not implement this method");
		return;
	}
	read_requeshdrs(&rio);

	static_flag=is_static(uri);
	if(static_flag)
		parse_static_uri(uri,filename);
	else
		parse_dynamic_uri(uri,filename,cgisrgs);

	if(stat(filename,&sbuf)<0){
		error_request(fd,filename,"404","Not Found","web could not find this find");
		return;
	}

	if(static_flag){
		if(!(S_ISREG(sbuf.st_mode))||!(S_IRUSR & sbuf.st_mode)){
			error_request(fd,filename,"403","Forbidden","web is not permtted to read the file");
			return;
		}
		feed_static(fd,filename,sbuf.st_size);
	}else{
		if(!(S_ISREG(sbuf.st_mode))||!(S_IXUSR &sbuf.st_mode)){
			error_request(fd,filename,"403","Forbidden","web could not run the CGI program");
			return;
		}
		feed_static(fd,filename,cgiargs);
	}
}



