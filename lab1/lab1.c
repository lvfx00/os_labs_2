#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void *print_routine(void *arg) {
        for(int i = 0; i < 10; ++i)
                printf("Some text from child thread\n");

        pthread_exit(NULL);
}


int main(int argc, char **argv) {
        pthread_t th;

        if (pthread_create(&th, NULL, print_routine, NULL) != 0) {
                perror("pthread_create");
                exit(EXIT_FAILURE);
        }

        for(int i = 0; i < 10; ++i)
                printf("Some text from parent thread\n");

        pthread_exit(NULL);
}

   
