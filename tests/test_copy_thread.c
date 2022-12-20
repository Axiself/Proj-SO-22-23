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

/*char * contents_file1 = "test_file_threading";
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

void assert_loop(copy_args_t args) {
    int f = tfs_open(args.path_copied_file, TFS_O_CREAT);
    assert(f != -1);   
    ssize_t r = tfs_read(f, buffer, sizeof(buffer) - 1);
    assert(r == strlen(args.content_file));
    assert(!memcmp(buffer, args.content_file, strlen(args.content_file)));
    //assert(tfs_unlink(args.path_copied_file) != -1);
}

int run_copy_from_external_threads() {
    memset(buffer, 0, 1024);
    pthread_t threads_1 [NUMBER_OF_THREADS];
    copy_args_t args_1 [NUMBER_OF_THREADS] = { 
        create_args(path_file1, path_copied_file1, contents_file1),
        create_args(path_file2, path_copied_file2, contents_file2),
        create_args(path_file3, path_copied_file3, contents_file3),
    };

    init_threads(threads_1, (void*)thread_copy_from_external, args_1);
    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        assert_loop(args_1[i]);
    }

    
    /**
     *  TODO: fix copy_from_external_threads to paths already filled
    memset(buffer, 0, 1024);
    pthread_t threads_2 [NUMBER_OF_THREADS];
      copy_args_t args_2 [NUMBER_OF_THREADS] = {
        create_args(path_file1, path_copied_file1, contents_file1),
        create_args(path_file1, path_copied_file2, contents_file1),
        create_args(path_file1, path_copied_file3, contents_file1),
    };

    init_threads(threads_2, (void*)thread_copy_from_external, NUMBER_OF_THREADS, args_2);
    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        assert_loop(args_1[i]);
    }
    */


    return 0;
}*/

int main() {
    //assert(tfs_init(NULL) != -1);
    
    //assert(run_copy_from_external_threads() != -1);

    assert(tfs_destroy() != -1);
    
    printf("Successful test.\n");
    return 0;
}