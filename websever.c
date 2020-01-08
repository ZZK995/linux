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
		error_request(fd,method,"501","Not Implemented","Web does not implement this method");
		return;
	}
	read_requeshdrs(&rio);

	static_flag=is_static(uri);
	if(static_flag)
		parse_static_uri(uri,filename);
	else
		parse_dynamic_uri(uri,filename,cgisrgs);

	if(stat(filename,&sbuf)<0){
		error_request(fd,filename,"404","Not Found","Web could not find this find");
		return;
	}

	if(static_flag){
		if(!(S_ISREG(sbuf.st_mode))||!(S_IRUSR & sbuf.st_mode)){
			error_request(fd,filename,"403","Forbidden","Web is not permtted to read the file");
			return;
		}
		feed_static(fd,filename,sbuf.st_size);
	}else{
		if(!(S_ISREG(sbuf.st_mode))||!(S_IXUSR &sbuf.st_mode)){
			error_request(fd,filename,"403","Forbidden","Web could not run the CGI program");
			return;
		}
		feed_static(fd,filename,cgiargs);
	}
}

int is_static(char *uri)
{
	if(!strstr(uri,"cgi-bin"))
		return 1;
	else
		return 0;
}

void error_request(int fd,char *cause,char *errnum,char *shortmsg,char *description)
{
	char buf[MAXLINE],body[MAXLINE];
	sprintf(body,"<html><title>error request</title>");
	sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body);
	sprintf(body,"%s%s:%s\r\n",body,errnum,shortmsg);
	sprintf(body,"%s<p>%s:%s\r\n",body,description,cause);
	sprintf(body,"%s<hr><em> Web server</em>\r\n",body);

	sprintf(buf,"HTTP/1.0 %s %s \r\n",errnum,shortmsg);
	rio_writen(fd,buf,strlen(buf));
	sprintf(buf,"Content-type:text/html\r\n");
	rio_writen(fd,buf,strlen(buf));
	sprintf(buf,"COntent-length:%d\r\n\r\n",(int)strlen(body));
	rio_writen(fd,buf,strlen(buf));
	rio_writen(fd,buf,strlen(body));
}

void read_requesthdrs(rio_t *rp)
{
	char buf[MAXLINE];

	rio_readlineb(rp,buf,MAXLINE);
	while(strcmp(buf,"\r\n")){
		printf("%s",buf);
		rio_readlineb(rp,buf,MAXLINE);
	}
	return;
}
