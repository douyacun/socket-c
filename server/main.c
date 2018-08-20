#include "dyc.h"
#include "sbuf.h"

#define NTHREADS 4
#define SBUFSIZE 16

void echo_cnt(int connfd);
void *thread(void *vargp);
static int byte_cnt;
static sem_t mutex;

sbuf_t sbuf;

int main(int argc, char **argv) {
	int i, listenfd, connfd, w = 1;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;
	pthread_t tid;
	char port[MAXLINE], host[MAXLINE];

	if(argc != 2){
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}

	listenfd = open_listenfd(argv[1]);

	sbuf_init(&sbuf, SBUFSIZE);

	for (i = 0; i < NTHREADS; ++i)
		pthread_create(&tid, NULL, thread, NULL);

	while (w){
		clientlen = sizeof(struct sockaddr_storage);
		connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
		getnameinfo((struct sockaddr *)&clientaddr, clientlen, host, MAXLINE, port, MAXLINE, 0);
		printf("Accept connected from <%s %s>\n", host, port);
		sbuf_insert(&sbuf, connfd);
	}

}

void *thread(void *vargp)
{
	// 分离自身, 终止之后回收它的内存资源
	pthread_detach(pthread_self());
	int w = 1;

	while(w){
		int connfd = sbuf_remove(&sbuf);
		echo_cnt(connfd);
		close(connfd);
	}
}

static void init_echo_cnt(void)
{
	sem_init(&mutex, 0, 1);
	byte_cnt = 0;
}

void echo_cnt(int connfd)
{
	int n;
	char buf[MAXLINE];
	rio_t rio;

	static pthread_once_t once = PTHREAD_ONCE_INIT;

	pthread_once(&once, init_echo_cnt);

	rio_readinitb(&rio, connfd);
	while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0){
		P(&mutex);
		byte_cnt += n;
		printf("Server receive %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
		V(&mutex);
		rio_writen(connfd, buf, MAXLINE);
	}
}