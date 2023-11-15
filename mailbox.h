#ifndef _OS_MAILBOX_H
#define _OS_MAILBOX_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#define NUM 8
#define CAP 128

union semun
{
    int val;                   /* value for cmd SETVAL */
    struct semid_ds *buf;      /* buffer for IPC_STAT & IPC_SET */
    unsigned short int *array; /* array for GETALL & SETALL */
    struct seminfo *__buf;     /* buffer for IPC_INFO */
};

// struct sembuf
// {
//     unsigned short int sem_num; /* semaphore number */
//     short int sem_op;           /* semaphore operation */
//     short int sem_flg;          /* operation flag */
// };

int getSem(key_t key)
{
    return semget(key, 1, IPC_CREAT | 0666);
}

int setSem(int semid, int value)
{
    union semun tmp;
    tmp.val = value;
    return semctl(semid, 0, SETVAL, tmp) == -1 ? 0 : 1;
}

int delSem(int semid)
{
    union semun tmp;
    return semctl(semid, 0, IPC_RMID, tmp);
}

int P(int semid)
{
    struct sembuf tmp;
    tmp.sem_num = 0;
    tmp.sem_op = -1;
    tmp.sem_flg = SEM_UNDO;
    // DEBUG
    printf("Waiting sem %d\n", semid);
    if (semop(semid, &tmp, 1) == -1)
    {
        perror("P Failed\n");
        return 0;
    }
    return 1;
}

int V(int semid)
{
    struct sembuf tmp;
    tmp.sem_num = 0;
    tmp.sem_op = 1;
    tmp.sem_flg = SEM_UNDO;
    if (semop(semid, &tmp, 1) == -1)
    {
        perror("V Failed\n");
        return 0;
    }
    return 1;
}

typedef struct
{
    int id, containerCnt, capacity;
    int mailCnt, freeCnt; // 2 sem for control range
    int rm;               // "read mutex"
    int wm;               // "write mutex"
    void *shm;            // share memory
    int rNum, wNum;       // rNum：最新可读取的信格号, wNum:最新可写入的信格号
    char *buffer;         // 2D vector, buffer[x] == [buffer + x * capacity], maxlen: buffer + containerCnt * capacity
    int *length;          // 1D vector, length of nth message, maxlen: length + containerCnt * sizeof(int)
} Mailbox;

Mailbox *InitMailBox(int id, int containerCnt, int capacity)
{
    Mailbox *mailbox = (Mailbox *)malloc(sizeof(Mailbox));
    int shmid;
    key_t key, k1, k2, k3, k4;
    key = ftok(".", id);
    shmid = shmget(key, sizeof(char) * containerCnt * capacity + containerCnt * sizeof(int), IPC_CREAT | 0666);
    k1 = ftok(".", id + 1);
    k2 = ftok(".", id + 2);
    k3 = ftok(".", id + 3);
    k4 = ftok(".", id + 4);
    if (shmid == -1)
    {
        perror("shmget error");
        return NULL;
    }
    mailbox->buffer = (char *)shmat(shmid, NULL, 0);
    if (mailbox->buffer == (char *)-1)
    {
        perror("shmat"); // Print error message if shmat fails
        return NULL;
    }
    mailbox->length = (int *)(mailbox->buffer + containerCnt * capacity);
    mailbox->id = shmid;
    mailbox->containerCnt = containerCnt;
    mailbox->capacity = capacity;
    mailbox->mailCnt = getSem(k1);
    mailbox->freeCnt = getSem(k2);
    mailbox->rm = getSem(k3);
    mailbox->wm = getSem(k4);
    setSem(mailbox->mailCnt, 0);
    setSem(mailbox->freeCnt, capacity);
    setSem(mailbox->rm, 1);
    setSem(mailbox->wm, 1);
    mailbox->wNum = mailbox->rNum = 0;
    return mailbox;
}

void send(Mailbox *target, char *str, int len)
{
    int w = target->wNum, c = target->containerCnt, cap = target->capacity;
    if (strlen(target->buffer) + len > c * cap)
    {
        perror("out of range");
        return;
    }
    if (target->wNum + 1 > target->containerCnt)
    {
        perror("Mailbox is full");
        return;
    }
    P(target->wm);
    P(target->freeCnt);
    printf("Send to mailbox: %s\n", str);
    // buffer中每条消息的长度都是capacity
    strncpy(&target->buffer[w * cap], str, cap);
    target->length[w] = len;
    target->wNum++;
    V(target->mailCnt);
    V(target->wm);
}

void receive(Mailbox *from, char *str)
{
    int r = from->rNum, c = from->containerCnt, cap = from->capacity;
    P(from->rm);
    P(from->mailCnt);
    strncpy(str, &from->buffer[r * cap], from->length[r]);
    memset(&from->buffer[r * cap], 0, cap);
    from->rNum++;
    V(from->freeCnt);
    V(from->rm);
}

void printMailbox(Mailbox *mailbox)
{
    int i;
    char str[128];
    memset(str, 0, sizeof(str));
    printf("Mailbox %d:\n", mailbox->id);
    printf("wNum: %d, rNum: %d\n", mailbox->wNum, mailbox->rNum);
    printf("buffer:");
    for (i = 0; i < mailbox->containerCnt; ++i)
    {
        strncpy(str, &mailbox->buffer[i * mailbox->capacity], mailbox->length[i]);
        printf("%s ", str);
        memset(str, 0, sizeof(str));
    }
    printf("\n");
    printf("length: ");
    for (i = 0; i < mailbox->containerCnt; i++)
        printf("%d ", mailbox->length[i]);
    printf("\n");
}

void withdraw(Mailbox *mailbox)
{
    P(mailbox->mailCnt);
    P(mailbox->wm);
    P(mailbox->rm);
    if (mailbox->rNum > 0)
        mailbox->rNum--;
    V(mailbox->rm);
    V(mailbox->wm);
    V(mailbox->freeCnt);
}

void deleteMailbox(Mailbox *mailbox)
{
    delSem(mailbox->mailCnt);
    delSem(mailbox->freeCnt);
    delSem(mailbox->rm);
    delSem(mailbox->wm);
    shmdt(mailbox->shm);
    shmctl(mailbox->id, IPC_RMID, NULL);
    free(mailbox);
}

#endif