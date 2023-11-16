#include <stdio.h>
#include "mailbox.h"
#define NUM 8
#define CAP 128
using namespace std;
int main()
{
    Mailbox *A = InitMailBox(1, NUM, CAP);
    Mailbox *B = InitMailBox(5, NUM, CAP);
    char input;
    char str[128];
    while (1)
    {
        printf("Please input the command:\n");
        memset(str, 0, sizeof(str));
        input = getchar();
        if (input == 's')
        {
            printf("Please input the message:\n");
            scanf("%s", str);
            send(B, str, strlen(str));
            printf("Send to mailbox: %s\n\n", str);
        }
        else if (input == 'r')
        {
            receive(A, str);
            printf("Receive from mailbox: %s\n\n", str);
        }
        else if (input == 'p')
        {
            printf("A:\n");
            printMailbox(A);
            printf("B:\n");
            printMailbox(B);
        }
        else if (input == 'w')
        {
            withdraw(B);
        }
        else if (input == 'd')
        {
            deleteMailbox(A);
            break;
        }
        getchar();
    }
    return 0;
}