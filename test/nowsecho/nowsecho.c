
#include "../tcpserver.h"

int test (int sock)
{
	char recv [1024];
	while (1)
	{
		const char tosend[] = "hello from wsocket";
		ssize_t sent = nowswrite(sock, tosend, sizeof(tosend) - 1);
		printf("sent %d/%d bytes\n", (int)sent, (int)sizeof(tosend) - 1);

		ssize_t len = nowsread(sock, recv, sizeof(recv) - 1);
		
		if (!len)
		{
			printf("0=eof\n");
			break;
		}
		
		if (len == -1)
		{
			if (errno != EAGAIN)
			{
				nowsclose(sock);
				return -1;
			}
		}
		
		recv[len] = 0;
		printf("received %d bytes: '%s'\n", (int)len, recv);
	}
	
	return 0;
}

int main (void)
{
	int port = DEFAULTPORT;
	
	int sock = my_socket();
	my_bind_listen(sock, port);
	  
	while (1)
	{
		printf("please open wsecho.html\n");
		printf("waiting on port %i\n", port);
		int clisock = my_accept(sock);
		test(clisock);
	}
	close(sock);
}
