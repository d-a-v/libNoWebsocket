
#include "../tcpserver.h"

void talktowebgl (int sock)
{
	int i = 1;
	char bufout [128];

	while (1)
	{
		sleep(1);

		sprintf(bufout, "iz=%g;", 0.01 * i++);
			
		ssize_t ret = nowswrite(sock, bufout, strlen(bufout));

		if (ret == -1 && errno != EAGAIN)
		{
			perror("write");
			nowsclose(sock);
			return;
		}

		printf("nowswrite('%s')=%d\n", bufout, (int)ret);
	}

	nowsclose(sock);
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
		printf("please open 3js.html\n");
		printf("waiting on port %i\n", port);
		int clisock = my_accept(sock);
		talktowebgl(clisock);
	}

	return 0;
}
