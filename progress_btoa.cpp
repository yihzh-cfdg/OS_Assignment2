#include <iostream>
#include <string>
#include "mailbox.h"
#define NUM 8
#define CAP 128
using namespace std;
int main()
{
    Mailbox *A = InitMailBox(1, NUM, CAP);
    Mailbox *B = InitMailBox(2, NUM, CAP);
    string input;
    char str[128];
    while (1)
    {
        cout << "Please input the command:\n";
        input.clear();
        memset(str, 0, sizeof(str));
        cin >> input;
        if (input == "send")
        {
            cout << "Please input the message:\n";
            cin >> str;
            send(B, str, strlen(str));
        }
        else if (input == "receive")
        {
            cout << "Receive from mailbox:";
            receive(A, str);
            cout << str << endl;
        }
        else if (input == "print")
        {
            printMailbox(B);
        }
        else if (input == "withdraw")
        {
            withdraw(B);
        }
        else if (input == "delete")
        {
            deleteMailbox(A);
            break;
        }
        else
        {
            cout << "Invalid command!\n";
        }
    }
    return 0;
}