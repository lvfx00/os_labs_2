#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WAIT_INTERVAL 2// in seconds

void *print_routine(void *arg) {
    for(int i = 0;; ++i) {
        printf("Some text #%d\n", i);
        pthread_testcancel();
    }
}

int main(int argc, char **argv) {
    int ret;
    pthread_t th;

    ret = pthread_create(&th, NULL, print_routine, NULL);
    if (ret != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    do {
	ret = sleep(WAIT_INTERVAL);
    } while (ret > 0);

    ret = pthread_cancel(th);
    if (ret != 0) {
        perror("pthread_cancel");
        exit(EXIT_FAILURE);
    }

    pthread_exit(NULL);
}
