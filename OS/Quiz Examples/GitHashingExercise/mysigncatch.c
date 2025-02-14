#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sigint_handler(int sig){
printf("\n Gotcha!Caught signal %d (SIGINT).Exiting gracefully...\n",sig);
 exit(0);
}
int main(){
if(signal(SIGINT, sigint_handler) == SIG_ERR) {
 perror("signal");
 exit(EXIT_FAILURE);
}

printf("Press Ctrl+C to trigger SIGINT.\n");

while(1){

sleep(2);
}
return 0;
}
