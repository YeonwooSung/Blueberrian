/********************************************
 * Created by Yeonwoo Sung on 2018. 4. 27.. *
 ********************************************/

#include "blueberrySemaphore.h"

// Initialize the semaphore.
int msg_sem_init(int semNum, int s) {
    semNum += SEMSTART; //change the value of the semNum (to check if the current index is out of bound of the semaphore counter.

    if (semNum > SEMEND || semNum < SEMSTART) {
        retrun -1; //if the number of the semaphore is out of bound, return -1.
    }

    msgmng.free_msg_pool[semNum].flag = s;

    return 0;
}

//Locking function of this semaphore.
int msg_sem_p(int semNum) {
    semNum += SEMSTART; //change the value of the semNum (to check if the current index is out of bound of the semaphore counter.

    if (semNum > SEMEND || semNum < SEMSTART) {
        retrun -1; //if the number of the semaphore is out of bound, return -1.
    }

    if (msgmng.free_msg_pool[semNum].flag <= 0) {
        //if the flag value of the semaphore variable in the given index is less than 0, return -2.
        return -2; //when the -2 is returned, the task manager will call the system call that calls the scheduler to block the task.
    }

    msgmng.free_msg_pool[semnum].flag--; //decrease the flag value of the current semaphore variable.

    return 0;
}

//Adds the semaphore counter
int msg_sem_v(int semNum) {
    semnum += SEMSTART; //change the value of the semNum (to check if the current index is out of bound of the semaphore counter.

    if (semNum > SEMEND || semNum < SEMSTART) {
        retrun -1; //if the number of the semaphore is out of bound, return -1.
    }

    msgmng.free_msg_pool[semnum].flag++; //increase the flag value of the current semaphore variable.

    return 0;
}

//Initializes of the message manager.
void msg_init(void) {
    int i;

    for (i = 0; i < MAXMSG; i++) {
        msgmng.free_msg_pool[i].data = 0;
        msgmng.free_msg_pool[i].flag = 0;
    }

    msgmng.init = msg_init;
    msgmng.itc_send = msg_itc_send;
    msgmng.itc_get = msg_itc_get;
    msgmng.sem_init = msg_sem_init;
    msgmng.sem_p = msg_sem_p;
    msgmng.sem_v = msg_sem_v;
}
