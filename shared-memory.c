#include<stdio.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<sys/types.h>

#define SHMSIZE 1024

main()
{
    char *data;
    int shmid;
    key_t key;

    if((key = ftok("ipc.txt", 'M')) == -1){
        perror("ftok");
        exit(1);
    }
    printf("key created\n");
    printf("key = %d\n", key);
    printf("\n\n");

    if((shmid = shmget(key, SHMSIZE, IPC_CREAT | 0600)) == -1){
        perror("shmget");
        exit(1);
    }

    printf("shared memory created\n");
    printf("shmid = %d\n", shmid);
    printf("\n\n");

    data = shmat(shmid, (void *)0, 0);
    if(data == (char *)(-1)){ 
        perror("shmat");
        exit(1);
    }
    printf("pointer attached to shared memory\n");
    printf("\n\n");

    switch(fork()){
        case -1:
                perror("fork");
                exit(1);

        case 0: // child
                printf("child : \n");
                printf("enter a string : ");
                if(fgets(data, SHMSIZE, stdin) == NULL){
                    perror("fgets");
                    exit(1);
                }
                if(shmdt(data) == -1){
                    perror("shmdt");
                    exit(1);
                }
                printf("\n\n");
                break;

        default: // parent
                wait(NULL);
                printf("parent : \n");
                printf("data stored in shared memory\n");
                printf("shared content is : %s\n", data);

                if(shmdt(data) == -1){
                    perror("shmdt");
                    exit(1);
                }
                if(shmctl(shmid, IPC_RMID, NULL) == -1){
                    perror("shmctl");
                    exit(1);
                }
                break;
    }
}
