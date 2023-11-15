
int main()
{
    Mailbox* A = InitMailBox(1, NUM, CAP);
    Mailbox* B = InitMailBox(5, NUM, CAP);
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
            send(A, str, strlen(str));
            printMailbox(A);
        }
        else if (input == 'r')
        {
            printf("Receive from mailbox:");
            receive(A, str);
            printf("%s\n", str);
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