#include "../tcpserver.h"

int main (void)
{
    int cli = my_socket();
    my_connect("localhost", 1111, cli);
    nows_simulate_client(cli);
    
    printf("cli waits\n");

    char buf[6];
    char* pbuf = buf;
    
    int need = 5;
    while (need)
    {
        int ret = nowsread(cli, pbuf, need);
        printret("read", ret);
        if (ret >= 0)
        {
            need -= ret;
            pbuf += ret;
        }
        else if (errno != EAGAIN)
            break;
    }
    
    buf[5] = 0;
    printf("received: '%s'\n", buf);

    
    nowsclose(cli);
}
