#include "dyc.h"

#define MAXLINE 1024

int main(int argc, char **argv)
{
	int clientfd, i = 1;
	char *host, *port, buf[MAXLINE];
	rio_t rio;
	ssize_t n;
	fd_set read_set, ready_set;

//	if(argc < 3){
//		fprintf(stderr, "useage: %s <host> <port>\n", argv[0]);
//		exit(0);
//	}

	host = "192.168.100.2";
//	host = "127.0.0.1";
	port = "8888";

	clientfd = open_clientfd(host, port);
	if(clientfd < 0){
		fprintf(stderr, "连接服务器失败: %s\n", host);
		exit(0);
	}

	FD_ZERO(&read_set);
	FD_SET(clientfd, &read_set);
	FD_SET(STDIN_FILENO, &read_set);

	while (i){

//		fgets(buf, MAXLINE, stdin) != NULL

		ready_set = read_set;

		select(clientfd + 1, &ready_set, NULL, NULL, NULL);

		if(FD_ISSET(clientfd, &ready_set)){
			if((n = recv(clientfd, buf, MAXLINE, 0)) > 0){
				fputs(buf, stdout);
				fputs("\n", stdout);
			} else {
				printf("服务端连接已断开");
				exit(0);
			}
		}

		if(FD_ISSET(STDIN_FILENO, &ready_set)){
			fgets(buf, MAXLINE, stdin);
			send(clientfd, buf, strlen(buf), 0);
		}
	}

	close(clientfd);
	exit(0);
}