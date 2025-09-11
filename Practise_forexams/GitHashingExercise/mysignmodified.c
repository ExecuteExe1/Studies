#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// counting the signals
int count = 0;

//  we need to handle the sign here
void handle_sigint(int sig) {
    count++;
    printf("Caught SIGINT (Ctrl+C). Count: %d\n", count);
    if (count == 2) {
        printf("Terminating after 2 signals.\n");
        exit(0);
    }
}

int main() {
    //initilization of the sign handler
    signal(SIGINT, handle_sigint);

    // Infinite loop 
    printf("Running... Press Ctrl+C to trigger the signal.\n");
    while (1) {
    }

    return 0;
}
