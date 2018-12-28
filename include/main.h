// The header that contains the command struct, which is used in the main.c file.
//
// modified by Yeonwoo Sung on 2018.12.28

#ifndef BLUEBERRIAN_MAIN_HEADER
#define BLUEBERRIAN_MAIN_HEADER

typedef struct {
    char *CmdStr;                       // command string
    int (*func)(int argc, char **argv); // command function
} TCommand;

#endif //BLUEBERRIAN_MAIN_HEADER
