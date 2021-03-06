#include "../tcpserver.h"

int main (void)
{
    int srv = my_socket();
    
    my_bind_listen(srv, 1111);
    
    int cli = my_accept(srv);
    
    printf("cli has come, srv waits\n");

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
    close(srv);
}
