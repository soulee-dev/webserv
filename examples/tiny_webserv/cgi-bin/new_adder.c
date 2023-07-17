/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) 
{
  char *buf, *p, *arg1_p, *arg2_p, *method;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE], val1[MAXLINE], val2[MAXLINE];
  int n1=0, n2=0;

  /* Extract the two arguments */
  if ((buf = getenv("QUERY_STRING")) != NULL) {
    p = strchr(buf, '&');
    *p = '\0';
    strcpy(arg1, buf);
    strcpy(arg2, p+1);
    // fnum=value에서 value만 추출하기 위한 작업
    arg1_p = strchr(arg1, '=');
    *arg1_p = '\0';
    strcpy(val1, arg1_p+1);
    // snum=value에서 value만 추출하기 위한 작업
    arg2_p = strchr(arg2, '=');
    *arg2_p = '\0';
    strcpy(val2, arg2_p+1);
    //문자를 정수로 전환하는 atoi함수
    n1 = atoi(val1);
    n2 = atoi(val2);
  }
  method = getenv("REQUEST_METHOD");
  /* Make the response body */
  sprintf(content, "QUERY_STRING=%s", buf);
  sprintf(content, "Welcome to add.com: ");
  sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
  sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1 + n2);
  sprintf(content, "%sThanks for visiting!\r\n", content);
  /* Generate the HTTP response */
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");

  // method가 GET일 경우에만 response body보냄
  if (strcasecmp(method, "GET") == 0) {
    printf("%s", content);
  }
  fflush(stdout);

  exit(0);
}