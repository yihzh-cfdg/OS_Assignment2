#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct
{
    long mtype;     /* message type, must be > 0 */
    char data[128]; /* message data */
} MSG;

int main()
{
    key_t key;
    int msgid;
    MSG s_msg, r_msg;

    key = ftok("progfile", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid < 0)
    {
        printf("msgget error\n");
        return 0;
    }
    else
        printf("msgget success msgid:%d\n", msgid);
    system("ipcs -q");
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        printf("fork error\n");
        return 0;
    }
    if (pid > 0)
    {
        s_msg.mtype = 1;
        memset(s_msg.data, 0, sizeof(s_msg.data));
        strncpy(s_msg.data, "msg queue data", strlen("msg queue data"));
        msgsnd(msgid, &s_msg, strlen(s_msg.data), 0);
        msgctl(msgid, IPC_RMID, NULL);
        system("ipcs -q");
        return 0;
    }
    if (pid == 0)
    {
        msgrcv(msgid, &r_msg, sizeof(r_msg.data), 0, 0);
        printf("recv msg data type: %ld, data: %s\n", r_msg.mtype, r_msg.data);
        return 0;
    }
    return 0;
}