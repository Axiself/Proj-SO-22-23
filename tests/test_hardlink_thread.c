#include "fs/operations.h"
#include "config.h"
#include "state.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "fs/fs_thread.h"
#include <assert.h>
#include <stdint.h>

#define NUMBER_OF_THREADS (3)

typedef struct { 
    char * path_target;
    char * path_hard;
} copy_args_t;

uint8_t const file_contents[] = "AAA!";

char *target_path1 = "/t1";
char *hard_path1 = "/h1";
char *target_path2 = "/t2";
char *hard_path2 = "/h2";
char *target_path3 = "/t3";
char *hard_path3 = "/h3";

copy_args_t create_args(char * target, char * link) { 
    copy_args_t args = { 
        .path_target = target,
        .path_hard = link,
    };
    return args;
}

void assert_contents_ok(char const *path) {
    int f = tfs_open(path, 0);
    assert(f != -1);

    uint8_t buffer[sizeof(file_contents)];
    assert(tfs_read(f, buffer, sizeof(buffer)) == sizeof(buffer));
    assert(memcmp(buffer, file_contents, sizeof(buffer)) == 0);

    assert(tfs_close(f) != -1);
}

void assert_empty_file(char const *path) {
    int f = tfs_open(path, 0);
    assert(f != -1);

    uint8_t buffer[sizeof(file_contents)];
    assert(tfs_read(f, buffer, sizeof(buffer)) == 0);

    assert(tfs_close(f) != -1);
}

void write_contents(char const *path) {
    int f = tfs_open(path, 0);
    assert(f != -1);

    assert(tfs_write(f, file_contents, sizeof(file_contents)) ==
           sizeof(file_contents));

    assert(tfs_close(f) != -1);
}

int init_threads(pthread_t thread[], void * fun, copy_args_t args[]) {
    for (size_t i = 0; i < NUMBER_OF_THREADS; i++) {
        if (pthread_create(&thread[i], NULL, fun, &args[i]) != 0)
            return -1;
    }    
    
    for (size_t i = 0; i < NUMBER_OF_THREADS; i++) {
        if (pthread_join(thread[i], NULL) != 0) 
            return -1;
    }
    return 0; 
}

void init_files() {
    int fd = tfs_open(target_path1, TFS_O_CREAT);
    assert(fd != -1);

    // Immediately close file
    assert(tfs_close(fd) != -1);

    fd = tfs_open(target_path2, TFS_O_CREAT);
    assert(fd != -1);

    // Immediately close file
    assert(tfs_close(fd) != -1);
    
    fd = tfs_open(target_path3, TFS_O_CREAT);
    assert(fd != -1);

    // Immediately close file
    assert(tfs_close(fd) != -1);
}

void thread_hardlink(void *args) {
    copy_args_t cpy_args = *((copy_args_t *)args);
    tfs_link(cpy_args.path_target, cpy_args.path_hard);  
}

void thread_hardunlink(void *args) { 
    copy_args_t cpy_args = *((copy_args_t *)args);
    tfs_unlink(cpy_args.path_hard);

}

int run_hardlink_thread() { 
    pthread_t threads_1 [NUMBER_OF_THREADS];
    copy_args_t args_1[NUMBER_OF_THREADS] = {
        create_args(target_path1, hard_path1),
        create_args(target_path2, hard_path2),
        create_args(target_path3, hard_path3),
    };

    assert(init_threads(threads_1, (void*)thread_hardlink, args_1) != -1);

    for (size_t i = 0; i < NUMBER_OF_THREADS; i++)
    {
        assert_empty_file(args_1[i].path_target);
        assert_empty_file(args_1[i].path_hard);
        write_contents(args_1[i].path_hard);
        assert_contents_ok(args_1[i].path_hard);
        assert_contents_ok(args_1[i].path_target);
    }    

    return 0;   
}

int run_hardunlink_thread() {
    pthread_t threads_1 [NUMBER_OF_THREADS];
    copy_args_t args_1[NUMBER_OF_THREADS] = {
        create_args(target_path1, hard_path1),
        create_args(target_path2, hard_path2),
        create_args(target_path3, hard_path3),
    };

    assert(init_threads(threads_1, (void*)thread_hardunlink, args_1) != -1);

    for (size_t i = 0; i < NUMBER_OF_THREADS; i++)
    {
        assert(tfs_open(args_1[i].path_hard, 0) == -1);
        assert(tfs_open(args_1[i].path_target, 0) != -1);
        assert(tfs_unlink(args_1[i].path_target) != -1);
        assert(tfs_open(args_1[i].path_target, 0) != 0);
    }

    return 0;
}

int main () { 
    assert(tfs_init(NULL) != -1);
    init_files();

    assert(run_hardlink_thread() != -1);
    assert(run_hardunlink_thread() != -1);

    assert(tfs_destroy() != -1);
    printf("Successful test.\n");
    return 0;
}