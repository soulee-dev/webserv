#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <vector>

#include <sys/time.h>

void exit_with_error(const std::string &msg);

int __socket_init();
void __init_server_addr(struct sockaddr_in &server_addr);
void __bind_handling(int server_socket, struct sockaddr_in &server_addr);
void __listen_handling(int sd);
void __fcntl_handling(int sd);
int __kqueue_handling();
int __accept_handling(int sd);
