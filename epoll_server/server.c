#include "dyc.h"
#include <sys/epoll.h>
#define MAX_EVENTS 20

int main() {
	int listenfd, connfd,  epollfd, result, wait_count;
	int n,i, w = 1;
	struct sockaddr_storage clientaddr;
	socklen_t clientlen;
	char port[MAXLINE], host[MAXLINE], server_port[] = "8888";
	struct epoll_event ev, events[MAX_EVENTS];

	listenfd = open_listenfd(server_port);
	if(listenfd == -1){
		printf("service start failed\n");
		exit(EXIT_FAILURE);
	}

	// 创建epoll实例
	epollfd = epoll_create1(0);
	if(epollfd == 1){
		printf("epoll_create1: epoll instance!\n");
		exit(0);
	}

	ev.data.fd = listenfd;
	ev.events = EPOLLIN | EPOLLET; // 边缘触发

	// 设置epoll的事件
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1){
		printf("Epoll add failed\n");
		exit(0);
	}

	while (w){

		wait_count = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if(wait_count == -1){
			printf("epoll_wait error!\n");
		}

		for (i = 0; i < wait_count; ++i) {

			char buf[MAXLINE] = {0};
			// 客户端新连接请求
			if(events[i].data.fd == listenfd){
				// 等待客户端连接到达
				clientlen = sizeof(struct sockaddr_storage);
				connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
				if(connfd == -1){
					printf("accept error!\n");
					exit(EXIT_FAILURE);
				}

				// 向epoll注册client_sockfd事件
				ev.events = EPOLLIN;// 边缘触发
				ev.data.fd = connfd;
				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) == -1){
					printf("epoll_ctl:client_sockfd EPOLL_CTL_ADD failed\n");
					exit(EXIT_FAILURE);
				}

				getnameinfo((struct sockaddr *)&clientaddr, clientlen, host, MAXLINE, port, MAXLINE, 0);
				printf("Accept connected from <%s %s>\n", host, port);

			}
			// 客户端新数据发送
			else {
				if((n = recv(events[i].data.fd, buf, MAXLINE, 0)) <= 0){
					if(n == 0){
						// 客户端断开连接
						printf("client closed\n");
					} else {
						printf("receive from client error\n");
					}
					close(ev.data.fd);
				} else {
					printf("server receive: %s\n", buf);
					send(events[i].data.fd, buf, MAXLINE, 0);
				}
			}
		}
	}
}