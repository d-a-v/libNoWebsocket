
#include "../tcpserver.h"

void nowseval (int sock)
{

//XXXX make it work without:
//setcntl(sock, F_SETFL, O_NONBLOCK, "O_NONBLOCK");

	struct pollfd pollfd = { .fd = sock, .events = POLLIN | POLLOUT, };
	
	char bufin [1024];
	const size_t maxrecv = sizeof(bufin);
	
	char bufout [1024];
//	const size_t maxsend = sizeof(bufout);
	
	while (1)
	{
D("loop\n");
sleep(1);

		pollfd.events =  0;
		pollfd.events |= POLLIN;
		pollfd.events |= POLLOUT;
		D("poll: ");
		int ret = poll(&pollfd, 1, 1000 /*ms*/);
		D("%d\n", (int)ret);
		if (ret == -1)
		{
			perror("poll");
			nowsclose(sock);
			return;
		}

		if (pollfd.revents & POLLIN)
		{
			D("read:");
			ssize_t ret = nowsread(sock, bufin, maxrecv - 1);
			D("%d\n", (int)ret);
			if (ret == -1)
			{
				if (errno == EAGAIN)
					continue;
				perror("read");
				exit(EXIT_FAILURE);
			}
			if (ret == 0)
			{
				fprintf(stderr, "peer has closed\n");
				break;
			}
			
			bufin[ret] = 0;
			printf("received: '%s'\n", bufin);
		}
		
		if (pollfd.revents & POLLOUT)
		{
			D("poll: write ");
			
			static int i = 0;
			sprintf(bufout, "document.body.innerText+='%d\\n';", i++);
			
			ssize_t ret = nowswrite(sock, bufout, strlen(bufout));
			D("%d\n", (int)ret);
			if (ret == -1)
			{
				if (errno == EAGAIN)
					continue;
				perror("write");
				break;
			}
		}
		
		if (pollfd.revents & ~(POLLIN | POLLOUT))
		{
			fprintf(stderr, "unregular event occured\n");
			break;
		}
	}

	my_close(sock);
}

int main (int argc, char* argv[])
{
	int op;
	int port = DEFAULTPORT;
	
	while ((op = getopt(argc, argv, "hp:")) != EOF) switch(op)
	{
		case 'h':
			help();
			return 0;

		case 'p':
			port = atoi(optarg);			
			break;
	}
			
	int sock = my_socket();
	my_bind_listen(sock, port);
	while (1)
	{
		printf("please open wseval.html\n");
		printf("waiting on port %i\n", port);
		int clisock = my_accept(sock);
		nowseval(clisock);
	}

	return 0;
}
