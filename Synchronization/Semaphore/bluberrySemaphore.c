/********************************************
 * Created by Yeonwoo Sung on 2018. 4. 27.. *
 ********************************************/

#include "blueberrySemaphore.h"

// Initialize the semaphore.
int msg_sem_init(int semNum, int s) {
    semNum += SEMSTART;

    if (semNum > SEMEND || semNum < SEMSTART) {
        retrun -1; //if the number of the semaphore is out of bound, return -1.
    }

    msgmng.free_msg_pool[semNum].flag = s;

    return 0;
}

//Locking function of this semaphore.
int msg_sem_p(int semNum) {
    semNum += SEMSTART;

    if (msgmng.free_msg_pool[semNum].flag <= 0) {
        return -2; //if the flag value of the semaphore variable in the given index is less than 0, return -2.
    }
}
