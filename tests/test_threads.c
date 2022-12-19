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


#define NUMBER_OF_THREADS (4)

uint8_t const file_contents[] = "AAA!";
char const target_path1[] = "/f1";
char const link_path1[] = "/l1";
char const target_path2[] = "/f2";
char const link_path2[] = "/l2";
char *str_ext_file =
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! BBB! "
        "BBB! BBB! BBB! BBB! BBB! ";
char *path_copied_file = "/f1";
char *path_src = "tests/file_to_copy_over512.txt";
char buffer[600];

int m = 0;

/**
 * TODO: Change all tests
*/

int test1() {
    for (int i = 0; i < 100000000; i++) { 
        m = m +1;
        //printf("test 1-> %d\n", m);
    }
    return 0;
}

int test2() {
    for (int i = 0; i < 1000; i++) { 
        m++;
        printf("test 2-> %d\n", m);
    }
    return 0;
}

int test3() {
    for (int i = 0; i < 1000; i++) { 
        m++;
        printf("test 3-> %d\n", m);
    }
    return 0;
}

int test4() {
    for (int i = 0; i < 1000; i++) { 
        m++;
        printf("test 4-> %d\n", m);
    }
    return 0;
}

/**
 * Testing 4 total threads
*/
int (*funcs[4])(int) = { test1, test2, test3, test4 };

int run_thread_tests(fs_thread_t * fs_thread) { 

    for (int i = 0; i < fs_thread->n_threads; i++) { 
        create_fs_thread(fs_thread, funcs[0]);
    }

    for (int i = 0; i < fs_thread->n_threads; i++) { 
        if (pthread_join(fs_thread->threads[i], NULL) != 0) 
            return -1;
    }
    return 0;
}

int main() {
    fs_thread_t * fs_thread = malloc(sizeof(fs_thread_t) + NUMBER_OF_THREADS*sizeof(pthread_t));
    if(init_fs_thread(fs_thread, NUMBER_OF_THREADS) == -1) return -1;

    if(run_thread_tests(fs_thread) != 0) return -1;

    printf("%d", m);

    destroy_fs_thread(fs_thread);
    return 0;
}