
#define DEBUG 1
#define WRITEMIN 1

#if DEBUG
#define D(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); fflush(stderr); } while (0)
#else
#define D(...) do { (void)0; } while (0)
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <signal.h>

#include <nows.h>

#define DEFAULTPORT 1111

void getsetflag (int sock, int sock2, int level, int flag, int val, const char* name)
{
	static int ret[64];
	socklen_t retlen;
	int locval = val;
	
	retlen = 0;
	ret[0] = 0;
	if (getsockopt(sock, level, flag, ret, &retlen) == -1)
	{
		perror("getsockopt");
		exit(EXIT_FAILURE);
	}
	printf("flag = %s(%i): %i(len=%i) - set it to %i - ", name, flag, *(int*)ret, retlen, locval);
	fflush(stdout);
	if (setsockopt(sock, level, flag, &locval, sizeof(locval)) == -1)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	retlen = sizeof(int);
	*(int*)ret = 0;
	if (getsockopt(sock, level, flag, &ret, &retlen) == -1)
	{
		perror("getsockopt");
		exit(EXIT_FAILURE);
	}
	printf("re-read: %i(len=%i)\n", *(int*)ret, retlen);
	
	if (sock2 > 0 && sock2 != sock)
		getsetflag(sock2, -1, level, flag, val, name);
}
	
void setflag (int sock, int sock2, int level, int flag, int val, const char* name)
{
	int locval = val;

	printf("flag = %s(%i) - set it to %i\n", name, flag, locval);
	if (setsockopt(sock, level, flag, &locval, sizeof(locval)) == -1)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	if (sock2 > 0 && sock2 != sock)
		setflag(sock2, -1, level, flag, val, name);
}
	
int my_socket (void)
{
	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket()");
		exit(EXIT_FAILURE);
	}
	return sock;
}

void my_bind_listen (int srvsock,  int port)
{
	struct sockaddr_in server;

	getsetflag(srvsock, -1, SOL_SOCKET, SO_REUSEADDR, 1, "reuseaddr");

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(srvsock, (struct sockaddr*)&server, sizeof(server)) == -1)
	{
		perror("bind()");
		exit(EXIT_FAILURE);
	}
	
	if (listen(srvsock, 1) == -1)
	{
		perror("listen()");
		exit(EXIT_FAILURE);
	}
}

int my_accept (int srvsock)
{
	int clisock;
	socklen_t n;
	struct sockaddr_in client;

	n = sizeof(client);
	if ((clisock = accept(srvsock, (struct sockaddr*)&client, &n)) == -1)
	{
		perror("accept()");
		exit(EXIT_FAILURE);
	}
	
	printf("remote client arrived.\n");
	
	return clisock;
}

void my_connect (const char* servername,  int port,  int sock)
{
	struct sockaddr_in server;
	struct hostent *desc_server;
	
	if ((desc_server = gethostbyname(servername)) == NULL)
	{
		perror("gethostbyname()");
		exit(EXIT_FAILURE);
	}
	
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	bcopy(desc_server->h_addr, &server.sin_addr, desc_server->h_length);
	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == -1)
	{
		perror("connect()");
		exit(EXIT_FAILURE);
	}
	
	static int displayed = 0;
	if (!displayed)
	{
		displayed = 1;
		printf("connected to %s.\n", servername);
	}
}

void my_close (int sock)
{
	if (sock >= 0)
		nowsclose(sock);
}

void help (void)
{
	printf("** nows eval tester\n");
}

void setcntl (int fd, int cmd, int flags, const char* name)
{
	if (fcntl(fd, cmd, flags) == -1)
		perror(name);
}

void printret (const char* what, int ret)
{
    printf("%s: ret=%d", what, ret);
    if (ret == -1)
        printf(" (errno=%d EAGAIN=%d)", errno, EAGAIN);
    printf("\n");
}
