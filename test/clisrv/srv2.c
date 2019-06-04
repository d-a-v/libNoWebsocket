#include "../tcpserver.h"

int main (void)
{
    int srv = my_socket();
    
    my_bind_listen(srv, 1111);
    
    int cli = my_accept(srv);
    
    printf("cli has come, srv writes first\n");

    char hello[] = "hello";
    int ret = nowswrite(cli, hello, 5);
    printf("send %zi\n", 5);
    printret("sent", ret);

    nowsclose(cli);
    close(srv);
}
