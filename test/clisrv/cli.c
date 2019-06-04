#include "../tcpserver.h"

int main (void)
{
    int cli = my_socket();
    my_connect("localhost", 1111, cli);
    nows_simulate_client(cli);
    
    printf("cli writes first\n");

    char hello[] = "hello";
    int ret = nowswrite(cli, hello, 5);
    printret("write", ret);
    
    nowsclose(cli);
}
