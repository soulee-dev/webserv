/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"
#include <stdlib.h>
#define original_staticx

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize, char *method);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

/* doit에서 쓰이는 stat struct */
// struct stat{
//   dev_t st_dev; /* ID of device containing file */
// ino_t st_ino; /* inode number */
// mode_t st_mode; /* 파일의 종류 및 접근권한 */
// nlink_t st_nlink; /* hardlink 된 횟수 */
// uid_t st_uid; /* 파일의 owner */
// gid_t st_gid; /* group ID of owner */
// dev_t st_rdev; /* device ID (if special file) */
// off_t st_size; /* 파일의 크기(bytes) */
// blksize_t st_blksize; /* blocksize for file system I/O */
// blkcnt_t st_blocks; /* number of 512B blocks allocated */
// time_t st_atime; /* time of last access */
// time_t st_mtime; /* time of last modification */
// time_t st_ctime; /* time of last status change */ 
// }

// 응답을 해주는 함수
void doit(int fd)
{
  // 정적파일인지 아닌지를 판단해주기 위한 변수
  int is_static;
  // 파일에 대한 정보를 가지고 있는 구조체
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  /* Read request line and headers */
  // rio(robust I/O (Rio)) 초기화
  Rio_readinitb(&rio, fd);
  // buf에서 client request 읽어들이기
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  // request header 출력
  printf("%s", buf);
  // buf에 있는 데이터를 method, uri, version에 담기
  sscanf(buf, "%s %s %s", method, uri, version);
  // method가 GET이 아니라면 error message 출력
  // problem 11.11을 위해 HEAD 추가
  if (strcasecmp(method, "GET") != 0 || strcasecmp(method, "HEAD") != 0) {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }
  read_requesthdrs(&rio);

  /* Parse URI from GET request */
  is_static = parse_uri(uri, filename, cgiargs);
  // filename에 맞는 정보 조회를 하지 못했으면 error message 출력 (return 0 if success else -1)
  if (stat(filename, &sbuf) < 0) {
      clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
      return;
  }
  // request file이 static contents이면 실행
  if (is_static) {
    // file이 정규파일이 아니거나 사용자 읽기가 안되면 error message 출력
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
      return;
    }
    // response static file
    serve_static(fd, filename, sbuf.st_size, method);
  }
  // request file이 dynamic contents이면 실행
  else {
    // file이 정규파일이 아니거나 사용자 읽기가 안되면 error message 출력
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden",
      "Tiny couldn't run the CGI program");
      return;
  }
  // response dynamic files
    serve_dynamic(fd, filename, cgiargs, method);
  }
}

// error 발생 시, client에게 보내기 위한 response (error message)
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];
  // response body 쓰기 (HTML 형식)
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  //response 쓰기
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);        // 버전, 에러번호, 상태메시지
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));                          // body 입력
}

// request header를 읽기 위한 함수
void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];
  Rio_readlineb(rp, buf, MAXLINE);
  // 빈 텍스트 줄이 아닐 때까지 읽기
  while(strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

// uri parsing을 하여 static을 request하면 0, dynamic을 request하면 1반환
int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr;

  // parsing 결과, static file request인 경우 (uri에 cgi-bin이 포함이 되어 있지 않으면)
  if (!strstr(uri, "cgi-bin")) {
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    // request에서 특정 static contents를 요구하지 않은 경우 (home page 반환)
    if (uri[strlen(uri)-1] == '/') {
      strcat(filename, "home.html");
    }
    return 1;
  }
  // parsing 결과, dynamic file request인 경우
  else {
    // uri부분에서 file name과 args를 구분하는 ?위치 찾기
    ptr = index(uri, '?');
    // ?가 있으면
    if (ptr) {
      //cgiargs에 인자 넣어주기
      strcpy(cgiargs, ptr+1);
      // 포인터 ptr은 null처리
      *ptr = '\0';
    }
    // ?가 없으면
    else {
      strcpy(cgiargs, "");
    }
    // filename에 uri담기
    strcpy(filename, ".");
    strcat(filename, uri);
    return 0;
  }
}

#ifdef original_static
void serve_static(int fd, char *filename, int filesize)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  /* Send response body to client */
  srcfd = Open(filename, O_RDONLY, 0);
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  Close(srcfd);
  rio_writen(fd, srcp, filesize);
  Munmap(srcp, filesize);
}

#else
// HEAD method 처리를 위한 인자 추가
void serve_static(int fd, char *filename, int filesize, char *method)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  if(strcasecmp(method, "GET") == 0) {
    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);
    // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    // solved problem 11.9
    srcp = malloc(filesize);
    Rio_readn(srcfd, srcp, filesize);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    // Munmap(srcp, filesize);
    free(srcp);
  }
}
#endif
  /* * get_filetype - Derive file type from filename*/
  void get_filetype(char *filename, char *filetype)
  {
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else if (strstr(filename, ".mpg"))
    strcpy(filetype, "video/mpg");
  else
    strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs, char *method)
{
  char buf[MAXLINE], *emptylist[] = { NULL };

  /* Return first part of HTTP response */
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0) { /* Child */
  /* Real server would set all CGI vars here */
  setenv("QUERY_STRING", cgiargs, 1);
  // method를 cgi-bin/adder.c에 넘겨주기 위해 환경변수 set
  setenv("REQUEST_METHOD", method, 1);
  Dup2(fd, STDOUT_FILENO); /* Redirect stdout to client */
  Execve(filename, emptylist, environ); /* Run CGI program */
  }
  Wait(NULL); /* Parent waits for and reaps child */
}


int main(int argc, char **argv)             //argc: arguments count, argv: arguments vector
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  // client socket's length
  socklen_t clientlen;
  // client socket's addr
  struct sockaddr_storage clientaddr;

  // port number를 입력안했을 경우 error
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  //arg로 받은 port number로 listen socket으로 변경
  listenfd = open_listenfd(argv[1]);
  // 무한 서버 루프 실행
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);                       // SA: type of struct socketaddr
    // client socket에서 hostname과 port number를 스트링으로 변환
    getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);
    close(connfd);
  }
}
