#include<stdio.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<sys/types.h>

#define SHMSIZE 1024 

int head, tail;

void errExit(char *);
void up(int semid, struct sembuf *sb);
void down(int semid, struct sembuf *sb);
void producer(int *buf, int full_slots, int empty_slots, struct sembuf *sb);
void consumer(int *buf, int shmid, int full_slots, int empty_slots, struct sembuf *sb);

main()
{
    // common key
    key_t key;

    // shared memory
    int shmid;
    int *buf;

    // semaphore
    struct sembuf sb;
    int full_slots, empty_slots;

    // getting key
    if((key = ftok("producer-consumer-problem2.c", 'b')) == -1)
        errExit("ftok");

    // creating shm
    if((shmid = shmget(key, SHMSIZE, IPC_CREAT|0600)) == -1)
        errExit("shmget");

    // attaching buf to shm
    buf = shmat(shmid, 0, 0);
    if(buf == (int *)(-1))
        errExit("shmat");

    // creating sem full_slots
    if((full_slots = semget(key, 1, IPC_CREAT|0600)) == -1)
        errExit("semget full_slots");

    // creating sem empty_slots
    if((empty_slots = semget(key, 1, IPC_CREAT|0600)) == -1)
        errExit("semget empty_slots");

    sb.sem_num = 0;
    sb.sem_flg = 0;

    // initialising full_slots to 0
    sb.sem_op = 0;
    if(semop(full_slots, &sb, 1) == -1)
        errExit("semop full_slots");

    // intiailising empty_slots to SHMSIZE
    sb.sem_op = SHMSIZE;
    if(semop(empty_slots, &sb, 1) == -1)
        errExit("semop empty_slots");

    // forking
    switch(fork()){
        case -1: // error
                errExit("fork");

        case 0: // producer
                producer(buf, full_slots, empty_slots, &sb);
                break;

        default: // consumer
                consumer(buf, shmid, full_slots, empty_slots, &sb);
                break;
    }
    printf("\n");
}

void consumer(int *buf, int shmid, int full_slots, int empty_slots, struct sembuf *sb)
{
    while(1){
        down(full_slots, sb);

        // critical section
        printf("%d\n", buf[tail]);
        tail = (tail + 1) % SHMSIZE;

        up(empty_slots, sb);
    }

    // consumer deattaching
    if(shmdt(buf) == -1)
        errExit("consumer: shmat");

    // closing shm
    if(shmctl(shmid, IPC_RMID, NULL) == -1)
        errExit("shmctl");

    // closing full_slots
    if(semctl(full_slots, 0, IPC_RMID) == -1)
        errExit("full_slots shmctl");

    // closing empty_slots
    if(semctl(empty_slots, 0, IPC_RMID) == -1)
        errExit("empty_slots shmctl");
}

void producer(int *buf, int full_slots, int empty_slots, struct sembuf *sb)
{
    int c = 0;
    while(1){
        down(empty_slots, sb);

        // critical section
        buf[head] = c;
        c += 1;
        head = (head + 1) % SHMSIZE;

        up(full_slots, sb);
    }

    // producer deattaching
    if(shmdt(buf) == -1)
        errExit("producer: shmat");
}

void up(int semid, struct sembuf *sb)
{
    sb->sem_op = 1;
    if(semop(semid, sb, 1) == -1)
        errExit("up: semop");
}

void down(int semid, struct sembuf *sb)
{
    sb->sem_op = -1;
    if(semop(semid, sb, 1) == -1)
        errExit("down: semop");
}

void errExit(char *errMsg)
{
    perror(errMsg);
    exit(1);
}
