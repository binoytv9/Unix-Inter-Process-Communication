#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/types.h>

// message
struct msgbuf{
    long mtype;
    int num;
};

void errExit(char *errMsg);

main()
{
    int i;
    int msgid;
    key_t key;
    struct msgbuf mb;

    // generating key
    if((key = ftok("message-queue.c", 'b')) == -1)
        errExit("ftok");

    // creating message queue
    if((msgid = msgget(key, IPC_CREAT | 0600)) == -1)
        errExit("msgget");

    // forking
    switch(fork()){
        case -1: // error
            errExit("fork");

        case 0: // producer
            for(i = 1;; ++i){
                mb.mtype = i;
                mb.num = i;
                // writing message to queue
                if(msgsnd(msgid, &mb, sizeof(struct msgbuf) - sizeof(long), 0) == -1)
                    errExit("msgsnd");
            }
            break;

        default: // consumer
            while(1){
                // reading message from queue and printing to screen
                if(msgrcv(msgid, &mb, sizeof(struct msgbuf) - sizeof(long), 0, 0) == -1)
                    errExit("msgrcv");

                printf("%d\n", mb.num);
            }
            if(msgctl(msgid, IPC_RMID, NULL) == -1)
                errExit("msgctl");

            break;
    }
}

void errExit(char *errMsg)
{
    perror(errMsg);
    exit(1);
}
