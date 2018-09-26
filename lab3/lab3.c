#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define THREAD_NUM 4
#define STRING_NUM 5
#define MAX_STR_SIZE 64

struct str_node {
    struct str_node *next;
    char str[MAX_STR_SIZE];
};

void *print_routine(void *arg) {
    struct str_node *args = (struct str_node *) arg;
    args = args->next;
    while (args != NULL) {
        printf("%s", args->str);
        args = args->next;
    }
    pthread_exit(NULL);
}

// returns 0 upon siccessfil completion and -1 upon failure
// and sets corresponding errno
int init_list(struct str_node **args) {
    struct str_node head;
    for(int i = 0; i < THREAD_NUM; ++i) {
        struct str_node *prev = &head;

        for(int j = 0; j < STRING_NUM; ++j) {
            struct str_node *new = malloc(sizeof(struct str_node));
            if (new == NULL) {
                return -1;
            }
            snprintf(new->str, MAX_STR_SIZE, "String #%d from %dth thread\n", j, i);
            new->next = NULL;
            prev->next = new;
            prev = new;
        }
        args[i] = head.next;
    }
    return 0;
}

void cleanup(struct str_node **args) {
     48     for(int i = 0; i < THREAD_NUM; ++i) {
        struct str_node *it = args[i];
        while(it != NULL) {
            struct str_node *temp2 = it;
            it = it->next;
            free(temp2);
        }
    }
}

int main(int argc, char **argv) {
    pthread_t ths[THREAD_NUM];
    struct str_node *args[THREAD_NUM];
    int ret;

    ret = init_list(args);
    if (ret == -1) {
        perror("init_list");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < THREAD_NUM; ++i) {
        ret = pthread_create(&ths[i], NULL, print_routine, (void *)(&args[i]));
        if (ret != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i < THREAD_NUM; ++i) {
        ret = pthread_join(ths[i], NULL);
        if (ret != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    cleanup(args);
    pthread_exit(NULL);
}


