#include<stdio.h>
#include<stdlib.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<sys/types.h>

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

main()
{
    int semid;
    struct sembuf sb;
    union semun sn;
    key_t key = IPC_PRIVATE;
    int semflg = 0666 | IPC_CREAT;

    semid = semget(key, 1, semflg);
    printf("sem created\n");
    printf("\tsemid : %d\n", semid);

    sn.val = 0;
    if(semctl(semid, 0, SETVAL, sn) == -1){
        fprintf(stderr, "semctl error");
        exit(0);
    }
    printf("sem initialised\n");


    sb.sem_num = 0;
    sb.sem_op = 1;
    sb.sem_flg = 0;

    semop(semid, &sb, 1);


    switch(fork()){
        case -1:
                fprintf(stderr, "fork error\n");
                exit(0);

        case 0: //child

                printf("child going to wait\n");
                sb.sem_op = -3;
                semop(semid, &sb, 1);
                printf("child wait over\n");
                break;

        default: // parent

                sleep(3);
                sb.sem_op = 2;
                printf("parent going to signal\n");
                semop(semid, &sb, 1);

                wait(NULL);
                semctl(semid, 0, IPC_RMID);
                printf("sem removed\n");
                break;
    }
} 
