#include<stdio.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<sys/types.h>

#define SHMSIZE 1024 

void errExit(char *);

main()
{
    FILE *file;

    // shared memory
    int shmid;
    key_t key;
    char *data;

    // semaphore
    int semid;
    struct sembuf sb;

    // creating key
    if((key = ftok("file.txt", 'P')) == -1)
        errExit("ftok");

    // creating a shared memory
    if((shmid = shmget(key, SHMSIZE, IPC_CREAT | 0600)) == -1)
        errExit("shmget");

    // attaching to a pointer data
    data = shmat(shmid, 0, 0);
    if(data == (char *)(-1))
        errExit("shmat");

    // creating semaphore
    if((semid = semget(key, 1, IPC_CREAT | 0600)) == -1)
        errExit("semget");

    sb.sem_num = 0;
    sb.sem_flg = 0;

    // setting value to 1
    sb.sem_op = 1;
    if(semop(semid, &sb, 1) == -1)
        errExit("semop");

    switch(fork()){
        case -1:
                errExit("fork");

        case 0: // producer
                if((file = fopen("file.txt", "r")) == NULL)
                    errExit("fopen");

                while(1){
                    // decrementing semval, reading line from a file and storing to shm
                    sb.sem_op = -1;
                    if(semop(semid, &sb, 1) == -1)
                        errExit("semop1 producer");

                    // critical section
                    if(fgets(data, SHMSIZE, file) == NULL)
                        rewind(file);

                    printf("produced : %s\n", data);

                    // incrementing semval
                    sb.sem_op = 1;
                    if(semop(semid, &sb, 1) == -1)
                        errExit("semop2 producer");
                }

                // child deattaching
                if(shmdt(data) == -1)
                    errExit("shmdt producer");
                break;

        default: // consumer
                while(1){
                    // decrement semval, read data from shm and print to screen
                    sb.sem_op = -1;
                    if(semop(semid, &sb, 1) == -1)
                        errExit("semop1 consumer");

                    // critical section
                    printf("consumed : %s\n", data);

                    // incrementing semval
                    sb.sem_op = 1;
                    if(semop(semid, &sb, 1) == -1)
                        errExit("semop2 consumer");
                }

                // parent deattaching
                if(shmdt(data) == -1)
                    errExit("shmdt consumer");

                // closing shm
                if(shmctl(shmid, IPC_RMID, NULL) == -1)
                    errExit("shmctl consumer");

                // closing sem
                if(semctl(semid, 0, IPC_RMID) == -1)
                    errExit("semctl consumer");
                break;
    }

    printf("\n");
}

void errExit(char *errMsg)
{
    perror(errMsg);
    exit(1);
}
