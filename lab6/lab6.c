#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <memory.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include "lab6.h"

const char *stralloc(const char *str) {
    if (str == NULL) {
        errno = EINVAL;
        return NULL;
    }

    char *copy = malloc(strlen(str) + 1);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, str, strlen(str) + 1);
    return copy;
}

const struct process_func_args *init_process_func_args(const char *src_path, const char *dest_path) {
    const char *copied_src_root_path = stralloc(src_path);
    if (copied_src_root_path == NULL) {
        return NULL;
    }

    const char *copied_dest_root_path = stralloc(dest_path);
    if (copied_dest_root_path == NULL) {
        return NULL;
    }

    struct process_func_args *args = malloc(sizeof(struct process_func_args));
    if (args == NULL) {
        return NULL;
    }

    args->src_path = copied_src_root_path;
    args->dest_path = copied_dest_root_path;

    return args;
}

void free_process_func_args(const struct process_func_args *args) {
    if (args == NULL) {
        return;
    }
    free((void *) args->src_path);
    free((void *) args->dest_path);
    free((void *) args);
}

DIR *open_directory(const char *dirpath) {
    while (1) {
        DIR *dir = opendir(dirpath);
        if (dir == NULL) {
            if (errno == EMFILE) { // no available file descriptors
                unsigned int sleep_remaining = SLEEP_TIME;
                do {
                    sleep_remaining = sleep(sleep_remaining);
                } while (sleep_remaining > 0);
            } else {
                perror("opendir");
                return NULL;
            }
        } else {
            return dir;
        }
    }
}

int process_dir(const char *src_dir_path, const char *dest_dir_path) {
    DIR *src_dir = open_directory(src_dir_path);
    if (src_dir == NULL) {
        perror("open_directory");
        return -1;
    }

    int ret;
    ret = mkdir(dest_dir_path, S_IRWXU | S_IRWXG | S_IROTH);
    if (ret == -1) {
        perror("mkdir");
        exit(EXIT_FAILURE);
    }

    // to store threads
    size_t count = 0;
    size_t size = INIT_PTHREAD_T_ARR_SIZE;
    pthread_t *threads = malloc(INIT_PTHREAD_T_ARR_SIZE * sizeof(pthread_t));
    if (threads == NULL) {
        perror("malloc");
        return -1;
    }

    int prev_errno = errno;
    // set errno to zero to distinguish errors from end of file stream
    errno = 0;

    struct dirent *entry = readdir(src_dir);
    while (entry != NULL) {
        printf("aaa\n");

        char full_file_path_src[strlen(src_dir_path) + strlen(entry->d_name) + 2];
        memcpy(full_file_path_src, src_dir_path, strlen(src_dir_path) + 1);
        strncat(full_file_path_src, "/", 1);
        strncat(full_file_path_src, entry->d_name, strlen(entry->d_name));

        struct stat stat_buffer;
        ret = stat(full_file_path_src, &stat_buffer);
        if (ret == -1) {
            perror("stat");
            return -1;
        }

        // TODO refactor!!!
        switch (stat_buffer.st_mode & S_IFMT) {
            case S_IFREG:
            case S_IFDIR: {
                if (stat_buffer.st_mode == S_IFDIR && (
                        strcmp(entry->d_name, ".") == 0  || strcmp(entry->d_name, "..") == 0)) {
                    continue;
                }

                char full_file_path_dest[strlen(dest_dir_path) + strlen(entry->d_name) + 2];
                memcpy(full_file_path_dest, dest_dir_path, strlen(dest_dir_path) + 1);
                strncat(full_file_path_dest, "/", 1);
                strncat(full_file_path_dest, entry->d_name, strlen(entry->d_name));

                const struct process_func_args *argzz = init_process_func_args(full_file_path_src, full_file_path_dest);

                if (stat_buffer.st_mode == S_IFDIR) {
                    ret = pthread_create(&threads[count], NULL, wrapped_process_dir, (void *) argzz);
                } else if (stat_buffer.st_mode == S_IFREG) {
                    ret = pthread_create(&threads[count], NULL, wrapped_process_file, (void *) argzz);
                }

                if (ret != 0) {
                    perror("pthread_create");
                    return -1;
                }
                count++;
                if (count == size) {
                    threads = realloc(threads, size * INC_FACTOR);
                    if (threads == NULL) {
                        perror("realloc");
                        return -1;
                    }
                }
                break;
            }
            default:
                break;
        }

        entry = readdir(src_dir);
    }
    if (errno != 0) {
        perror("readdir");
        return -1;
    }

    ret = closedir(src_dir);
    if (ret == -1) {
        perror("closedir");
        return -1;
    }

    errno = prev_errno;

    for (int i = 0; i < count; ++i) {
        ret = pthread_join(threads[i], NULL);
        if (ret != 0) {
            perror("pthread_join");
            return -1;
        }
    }

    free(threads);
    return 0;
}

void *wrapped_process_dir(void *arg) {
    struct process_func_args *args = (struct process_func_args *) arg;
    const char *src_root_path = args->src_path;
    const char *dest_root_path = args->dest_path;

    process_dir(src_root_path, dest_root_path);

    free_process_func_args(args);
    pthread_exit(NULL);
}

int open_file(const char *filepath, int oflag) {
    while (1) {
        int fd = open(filepath, oflag);
        if (fd == -1) {
            if (errno == EMFILE) { // no available file descriptors
                unsigned int sleep_remaining = SLEEP_TIME;
                do {
                    sleep_remaining = sleep(sleep_remaining);
                } while (sleep_remaining > 0);
            } else {
                perror("open");
                return -1;
            }
        } else {
            return fd;
        }
    }
}

int copy_file(int src_fd, int dest_fd) {
    char buffer[COPY_BUF_SIZE];
    ssize_t readnum;
    while (1) {
        readnum = read(src_fd, buffer, COPY_BUF_SIZE);
        if (readnum == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            return -1;
        }

        if (readnum == 0) {
            return 0;
        }

        while (1) {
            ssize_t writenum = write(dest_fd, buffer, (size_t) readnum);
            if (writenum == -1) {
                if (errno == EINTR) {
                    continue;
                }
                perror("write");
                return -1;
            }
            if (writenum < readnum) { // partial writing
                readnum = readnum - writenum; // remaining
                continue;
            }
            break;
        }
    }
}

int process_file(const char *src_file_path, const char *dest_file_path) {
    int src_fd = open_file(src_file_path, O_RDONLY);
    if (src_fd == -1) {
        perror("open_file");
        return -1;
    }

    int dest_fd = open_file(dest_file_path, O_WRONLY | O_CREAT | O_EXCL);
    if (dest_fd == -1) {
        perror("open_file");
        return -1;
    }

    int ret;
    ret = copy_file(src_fd, dest_fd);
    if (ret == -1) {
        perror("copy_file");
        return -1;
    }

    ret = close(dest_fd);
    if (ret == -1) {
        perror("close");
        return -1;
    }

    ret = close(src_fd);
    if (ret == -1) {
        perror("close");
        return -1;
    }

    return 0;
}

void *wrapped_process_file(void *arg) {
    struct process_func_args *args = (struct process_func_args *) arg;
    const char *src_file_path = args->src_path;
    const char *dest_file_path = args->dest_path;

    int ret;
    ret = process_file(src_file_path, dest_file_path);
    if (ret == -1) {
        perror("process_file");
    }

    free_process_func_args(args);
    pthread_exit(NULL);
}
