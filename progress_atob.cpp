#include <stdio.h>
#include "mailbox.h"
#define NUM 8
#define CAP 128
using namespace std;
void menu()
{
    printf("-----------------------\n");
    printf("-       进程通信       -\n");
    printf("- 请输入指令:          -\n");
    printf("- s: 发送消息          -\n");
    printf("- r: 接收消息          -\n");
    printf("- p: 打印信箱          -\n");
    printf("- d: 撤销信箱并退出     -\n");
    printf("-----------------------\n");
}

int main()
{
    Mailbox *A = InitMailBox(1, NUM, CAP);
    Mailbox *B = InitMailBox(5, NUM, CAP);
    char input;
    char str[128];
    menu();
    while (1)
    {
        printf("Please input the comand:\n");
        memset(str, 0, sizeof(str));
        input = getchar();
        if (input == 's')
        {
            printf("Please input the message:\n");
            scanf("%s", str);
            send(A, str, strlen(str));
            printf("Send to mailbox: %s\n\n", str);
        }
        else if (input == 'r')
        {
            receive(B, str);
            printf("Receive from mailbox: %s\n", str);
        }
        else if (input == 'p')
        {
            printf("A:\n");
            printMailbox(A);
            printf("B:\n");
            printMailbox(B);
        }
        /*
        else if (input == 'w')
        {
            withdraw(A);
        }*/
        else if (input == 'd')
        {
            deleteMailbox(B);
            break;
        }
        getchar(); 
    }
    return 0;
}