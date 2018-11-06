#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <memory.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>


// returns 0 if specified path is valid absolute pathname
// returns 1 if specified path isn't valid absolute pathname
// returns -1 upon error and sets corresponding errno
int validate_pathname(const char *source) {
    char *rp = realpath(source, NULL);
    if (rp == NULL) {
        return -1;
    }

    // specified path is not canonicalized absolute pathname
    if (strcmp(source, rp) != 0) {
        return 1;
    }

    return 0;
}


int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s source destination", argv[0]);
        exit(EXIT_FAILURE);
    }

    int ret;

    ret = validate_pathname(argv[1]);
    if (ret == 1) {
        fprintf(stderr, "You must specify canonicalized absolute pathname for source");
        exit(EXIT_FAILURE);
    }
    if (ret == -1) {
        perror("validate_path");
        exit(EXIT_FAILURE);
    }

    ret = validate_pathname(argv[2]);
    if (ret == 1) {
        fprintf(stderr, "You must specify canonicalized absolute pathname for destination");
        exit(EXIT_FAILURE);
    }
    if (ret == -1) {
        perror("validate_path");
        exit(EXIT_FAILURE);
    }



}