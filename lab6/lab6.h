#ifndef LAB6_LAB6_H
#define LAB6_LAB6_H

#endif //LAB6_LAB6_H

#define INIT_PTHREAD_T_ARR_SIZE 10
#define INC_FACTOR 2
#define COPY_BUF_SIZE 8192
#define SLEEP_TIME 3 // in seconds

struct process_func_args {
    const char *src_path;
    const char *dest_path;
};

const struct process_func_args *
init_process_func_args(const char *src_path, const char *dest_path);

void free_process_func_args(const struct process_func_args *args);

int process_dir(const char *src_dir_path, const char *dest_dir_path);

int process_file(const char *src_file_path, const char *dest_file_path);

int open_file(const char *filepath, int oflag);

int copy_file(int src_fd, int dest_fd);

void *wrapped_process_dir(void *arg);

void *wrapped_process_file(void *arg);
