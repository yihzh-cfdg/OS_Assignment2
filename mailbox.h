#ifndef MAILBOX_H
#define MAILBOX_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#define mNum 8
#define mCap 256

union semun
{
    int val;                   /* value for cmd SETVAL */
    struct semid_ds *buf;      /* buffer for IPC_STAT & IPC_SET */
    unsigned short int *array; /* array for GETALL & SETALL */
    struct seminfo *__buf;     /* buffer for IPC_INFO */
};

struct sembuf
{
    unsigned short int sem_num; /* semaphore number */
    short int sem_op;           /* semaphore operation */
    short int sem_flg;          /* operation flag */
};

int getSem(key_t key)
{
    return semget(key, 1, IPC_CREAT | 0666)
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
    int head, tail;       // head <= tail
    char *buffer;         // 2D vector, buffer[x] == [buffer + x * capacity], maxlen: buffer + containerCnt * capacity
    int *length;          // 1D vector, length of nth message, maxlen: length + containerCnt * sizeof(int)
} Mailbox;

Mailbox *InitMailBox(int id, int containerCnt, int capacity)
{
    Mailbox *mailbox = (Mailbox *)malloc(sizeof(Mailbox));
    int shmid;
    key_t key, k1, k2, k3, k4;
    key = ftok(".", id);
    shmid = shmget(key, containerCnt * capacity + containerCnt * sizeof(int), IPC_CREAT | 0666);
    k1 = ftok(".", id + 1 + shmid);
    k2 = ftok(".", id + 2 + shmid);
    k3 = ftok(".", id + 3 + shmid);
    k4 = ftok(".", id + 4 + shmid);
    if (shmid == -1)
    {
        perror("shmget error");
        return NULL;
    }
    system("ipcs -m");
    mailbox->shm = shmat(shmid, NULL, 0);
    mailbox->buffer = (char *)(mailbox->shm);
    mailbox->length = (int *)(mailbox->shm + containerCnt * capacity);
    mailbox->id = id;
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
    mailbox->head = mailbox->tail = 0;
    return mailbox;
}

void send(Mailbox *target, char *str, int len)
{
    int t = target->tail, c = target->containerCnt, cap = target->capacity;
    if (strlen(target->buffer) + len > c * cap)
    {
        perror("out of range");
        return;
    }
    if (target->tail + 1 > target->containerCnt)
    {
        perror("Mailbox is full");
        return;
    }
    P(target->wm);
    P(target->freeCnt);
    printf("Send to mailbox: %s\n", str);
    // buffer中每条消息的长度都是capacity
    strncpy(&target->buffer[t * cap], str, cap);
    target->length[t] = len;
    target->tail++;
    V(target->mailCnt);
    V(target->wm);
}

void receive(Mailbox *from, char *str)
{
    int h = from->head, c = from->containerCnt, cap = from->capacity;
    if (from->head == from->tail)
    {
        perror("Mailbox is empty");
        return;
    }
    P(from->rm);
    P(from->mailCnt);
    strncpy(str, &from->buffer[h * cap], from->length[h]);
    memset(&from->buffer[h * cap], 0, cap);
    from->head++;
    V(from->freeCnt);
    V(from->rm);
}

void printMailbox(Mailbox *mailbox)
{
    int i;
    printf("Mailbox %d:\n", mailbox->id);
    printf("head: %d, tail: %d\n", mailbox->head, mailbox->tail);
    printf("buffer: %s\n", mailbox->buffer);
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
    mailbox->tail--;
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