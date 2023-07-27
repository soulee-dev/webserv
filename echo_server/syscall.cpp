#include "echo_server.hpp"

inline void exit_with_error(const std::string &msg)
{
	std::cerr << msg << '\n';
	exit(EXIT_FAILURE);
}

/** @brief socket descripter를 반환 **/ 

int __socket_init()
{
	const int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1)
		exit_with_error("socket error\n" + std::string(strerror(errno)));
	return (server_socket);
}

/** @brief server address 초기화 
 * struct sockaddr_in 
 * {
	__uint8_t       sin_len;
	sa_family_t     sin_family; -> AF_INET(ipv4)
	in_port_t       sin_port; // uint16_t (unsigned short)
	struct  in_addr sin_addr; // struct in_addr { in_addr_t	s_addr; // uint32_t (unsigned int)}
	char            sin_zero[8];
}; **/

void __init_server_addr(struct sockaddr_in &server_addr)
{
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(8080); // convert host byte order to network byte order
}

/** @brief 소켓에 IP 주소와 포트 번호를 할당 **/ 

void __bind_handling(int server_socket, struct sockaddr_in &server_addr)
{
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
		exit_with_error("bind error\n" + std::string(strerror(errno)));
}

/** @brief 서버 소켓에서 클라이언트의 접속을 대기 **/

void __listen_handling(int sd)
{
	const int count_wait_queue = 5;
	if (listen(sd, count_wait_queue) == -1)
		exit_with_error("listen error " + std::string(strerror(errno)));
}

void __fcntl_handling(int sd)
{
	if (fcntl(sd, F_SETFL, O_NONBLOCK) == -1)
		exit_with_error("fcntl error " + std::string(strerror(errno)));
}

int	__kqueue_handling()
{
	const int kq = kqueue();
	if (kq == -1)
		exit_with_error("kqueue error " + std::string(strerror(errno)));
	return (kq);
}

int	__accept_handling(int sd)
{
	const int client_socket = accept(sd, NULL, NULL);
	if (client_socket == -1)
		exit_with_error("accept error : " + std::string(strerror(errno)));
	std::cout << "accept new client : " << client_socket << '\n';
	return (client_socket); 
}