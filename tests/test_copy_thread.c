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

char * contents_file1 = "test_file_threading";
char * contents_file2 = "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! ";
char * contents_file3 = "BBB!";

char * path_file1 = "tests/file_thread_test.txt";
char * path_file2 = "tests/file_to_copy_over512.txt";
char * path_file3 = "tests/file_to_copy.txt";

char * path_copied_file1 = "/f1";
char * path_copied_file2 = "/f2";
char * path_copied_file3 = "/f3";

char buffer[1024];

typedef struct { 
    char * path_file;
    char * path_copied_file;
    char * content_file;
} copy_args_t;

void thread_copy_from_external(void* args) {
    copy_args_t cpy_args = *((copy_args_t *)args);
    tfs_copy_from_external_fs(cpy_args.path_file, cpy_args.path_copied_file);    
}

copy_args_t create_args(char * path, char * path_copy, char * content) { 
    copy_args_t args = { 
        .path_file = path,
        .path_copied_file = path_copy,
        .content_file = content,
    };
    return args;
}


int init_threads(pthread_t thread, void * fun, size_t num_threads, copy_args_t args[]) {


    if (pthread_create(&thread, NULL, fun, &args) != 0)
        return -1;
    
   
    if (pthread_join(thread, NULL) != 0) 
        return -1;
    
    return 0;
    
}

void assert_loop(copy_args_t args) {
   
    int f = tfs_open(args.path_copied_file, TFS_O_CREAT);
    assert(f != -1);   
    ssize_t r = tfs_read(f, buffer, sizeof(buffer) - 1);
    assert(r == strlen(args.content_file));
    assert(!memcmp(buffer, args.content_file, strlen(args.content_file)));
    if(single != 1) assert(tfs_unlink(args.path_copied_file) != -1);

}

void fun_loop(size_t num, copy_args_t args [], pthread_t threads [],  void (*fun)(copy_args_t, pthread_t)) { 
    for (size_t i = 0; i < num; i++)
        (*fun)(args[i], threads[i]);
    
}

int run_copy_from_external_threads() {
    memset(buffer, 0, 1024);
    pthread_t threads [NUMBER_OF_THREADS];
    copy_args_t args [NUMBER_OF_THREADS] = { 
        create_args(path_file1, path_copied_file1, contents_file1),
        create_args(path_file2, path_copied_file2, contents_file2),
        create_args(path_file3, path_copied_file3, contents_file3),
    };

    init_threads(threads, (void*)thread_copy_from_external, NUMBER_OF_THREADS, args);
    
    assert_loop(args, NUMBER_OF_THREADS, 0);

    copy_args_t single = create_args(path_file1, path_copied_file1, contents_file1);
    pthread_t new_threads [NUMBER_OF_THREADS];

    init_threads(new_threads, (void*)thread_copy_from_external, NUMBER_OF_THREADS, single



    return 0;
}

int main() {
    assert(tfs_init(NULL) != -1);
    
    assert(run_copy_from_external_threads() != -1);

    printf("Successful test.\n");
    

}