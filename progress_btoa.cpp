#include <stdio.h>
#include "mailbox.h"
#define NUM 8
#define CAP 128
using namespace std;
int main()
{
    Mailbox* A = InitMailBox(1, NUM, CAP);
    Mailbox* B = InitMailBox(5, NUM, CAP);
    //DEBUG
    system("ipcs");
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
            //DEBUG
            printMailbox(B);
        }
        else if (input == 'r')
        {
            printf("Receive from mailbox:");
            receive(A, str);
            printf("%s\n", str);
            //DEBUG
            printMailbox(A);
        }
        else if (input == 'p')
        {
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