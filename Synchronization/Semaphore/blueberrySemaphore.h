/********************************************
 * Created by Yeonwoo Sung on 2018. 4. 27.. *
 ********************************************/

#ifndef BLUEBERRIAN_BLUEBERRYSEMAPHORE_H
#define BLUEBERRIAN_BLUEBERRYSEMAPHORE_H

#define MAXMSG 255 /* the size of the message area that semaphore and process could use. */

#define ITCSTART 0 /* The starting point of the message area that the process could use */
#define ITCEND 0 /* The end point of the message area that the process could use */

typedef struct _blueberrian_free_msg {
    int data;
    int flag;
} Blueberrian_free_msg;

typdedef struct _blueberrian_msg_mng {
    Blueberrian_free_msg free_msg_pool[MAXMSG]; //to manage the messages that the semaphore and process uses.
};

#endif //BLUEBERRIAN_BLUEBERRYSEMAPHORE_H
