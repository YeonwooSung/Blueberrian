/********************************************
 * Created by Yeonwoo Sung on 2018. 4. 27.. *
 ********************************************/

#ifndef BLUEBERRIAN_BLUEBERRYSEMAPHORE_H
#define BLUEBERRIAN_BLUEBERRYSEMAPHORE_H

#define MAXMSG 255 /* the size of the message area that semaphore and process could use. (0 ~ 255) */

#define ITCSTART 0 /* The starting point of the message area that the process could use */
#define ITCEND 99 /* The end point of the message area that the process could use */
#define SEMSTART 100 /* The starting point of the message area that the semaphore could use */
#define SEMEND 199 /* The end point of the message area that the semaphore could use */

// The Blueberrian  could have maximum 100 semaphore variables (100 ~ 199)

typedef struct _blueberrian_free_msg {
    int data;
    int flag;
} Blueberrian_free_msg;

typdedef struct _blueberrian_msg_mng {
    Blueberrian_free_msg free_msg_pool[MAXMSG]; //to manage the messages that the semaphore and process uses.

    //The function pointers.
    void (*init) (void);

    int (*itc_send) (int, int);
    int (*itc_get) (int, int*);

    int (*sem_init) (int, int);
    int (*sem_p) (int);
    int (*sem_v) (int);
} Blueberrian_msg_mng;

void msg_init(void);

int msg_itc_send(int, int*);
int msg_itc_get(int, int*);

int msg_sem_int(int, int);
int msg_sem_p(int); //Lock the semaphore.
int msg_sem_v(int); //Increase the counter.

#endif //BLUEBERRIAN_BLUEBERRYSEMAPHORE_H
