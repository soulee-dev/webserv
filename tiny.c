#include "csapp.h"

void	doit(int fd);
void	read_requesthdrs(rio_t *rp);
int		parse_uri(char *uri, char *filename, char *cgiargs);
void	serve_static(int fd, char *filename, int filesize);
void	get_filetype(char *filename, char *filetype);
void	serve_dynamic(int fd, char *filename, char *cgiargs);
void	clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int	main(int argc, char **argv)
{
	int			listenfd;
	int			connfd;
	char		hostname[MAXLINE], port[MAXLINE];
	socklen_t	clientlen;
	struct		sockaddr_storage	clientaddr;

	if (argc != 2)
	{
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}

	listenfd = Open_listenfd(argv[1]);
	while (1)
	{
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
		printf("Accepted connection from (%s, %s)\n", hostname, port);
		doit(connfd);
		Close(connfd);
	}
}

void	doit(int fd)
{
	int			is_static;
	struct stat	sbuf;
	char		buf[MAXLINE];
	char		method[MAXLINE];
	char		uri[MAXLINE];
	char		version[MAXLINE];
	char		filename[MAXLINE];
	char		cgiargs[MAXLINE];
	rio_t		rio;

	Rio_readinitb(&rio, fd);
	Rio_readlineb(&rio, buf, MAXLINE);
	printf("Request headers:\n");
	printf("%s", buf);
	sscanf(buf, "%s %s %s", method, uri, version);
	if (strcasecmp(method, "GET"))
	{
		clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
		return ;
	}
	read_requesthdrs(&rio);

	is_static = parse_uri(uri, filename, cgiargs);
	if (stat(filename, &sbuf) < 0)
	{
		clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
		return ;
	}
	if (is_static)
	{
		// Serve static content
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
		{
			clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
			return ;
		}
	}
	else
	{
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
		{
			clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
			return ;
		}
		serve_dynamic(fd, filename, cgiargs);
	}
}

void	clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
	char	buf[MAXLINE];
	char	body[MAXBUF];
}