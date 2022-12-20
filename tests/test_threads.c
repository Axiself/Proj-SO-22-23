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

char const file_contents[] = "test_file_threading";
char const target_path1[] = "/f1";
char const link_path1[] = "/l1";
char const target_path2[] = "/f2";
char const link_path2[] = "/l2";
char const hard_path1[] = "/h1";
char const hard_path2[] = "/h2";

char *path_src = "tests/file_thread_test.txt";

void assert_file_contents(char const *path, char const * file) {
    int f = tfs_open(path, 0);
    assert(f != -1);

    uint8_t buffer[sizeof(file)];
    assert(tfs_read(f, buffer, sizeof(buffer)) == sizeof(buffer));
    assert(memcmp(buffer, file, sizeof(buffer)) == 0);

    assert(tfs_close(f) != -1);
}

void assert_empty_file(char const *path, char const *file) {
    int f = tfs_open(path, 0);
    assert(f != -1);

    uint8_t buffer[sizeof(file)];
    assert(tfs_read(f, buffer, sizeof(buffer)) == 0);

    assert(tfs_close(f) != -1);
}

void write_contents(char const *path, char const *file) {
    int f = tfs_open(path, 0);
    assert(f != -1);
    assert(tfs_write(f, file, sizeof(file)) == sizeof(file));
    assert(tfs_close(f) != -1);
}


/**
 * TODO: Change all tests
*/

void symlink_test() {
    // Write to symlink and read original file

    assert_empty_file(target_path1, file_contents); // sanity check
    
    assert(tfs_sym_link(target_path1, link_path1) != -1);
    
    assert_empty_file(link_path1, file_contents);

    write_contents(link_path1, file_contents);
    assert_file_contents(target_path1, file_contents);

    // Write to original file and read through symlink
    
    write_contents(target_path2, file_contents);
    assert_file_contents(target_path2, file_contents); // sanity check

    assert(tfs_sym_link(target_path2, link_path2) != -1);
    assert_file_contents(link_path2, file_contents);

    printf(" ---> Symlink_test = %ld\n", pthread_self()); //get current thread id

}

void hardlink_test() {
    // try hard link with soft link from symlink_test
    assert(tfs_link(link_path1, hard_path1) != 0);
    assert(tfs_link(target_path1, hard_path1) != -1);

    assert(tfs_unlink(hard_path1) != -1);
    assert(tfs_unlink(hard_path2) != 0);

    assert(tfs_link(target_path2, hard_path2) != -1);

    //fails here when it shouldn't


    assert_file_contents(hard_path2, file_contents);
    printf(" ---> Hardlink_test = %ld\n", pthread_self()); //get current thread id
    
}

void copy_test() {
     printf(" ---> Copy_test = %ld\n", pthread_self()); //get current thread id
}

void general_test() {
    printf(" ---> General_test = %ld\n", pthread_self()); //get current thread id
}

/**
 * Testing 4 total threads
*/
void (*funcs[4])(int) = { symlink_test, hardlink_test, copy_test, general_test };

void init_files() {
    int f = tfs_open(target_path1, TFS_O_CREAT);
    assert(f != -1);
    assert(tfs_close(f) != -1);

    f = tfs_open(target_path2, TFS_O_CREAT);
    assert(f != -1);
    assert(tfs_close(f) != -1);
}

int run_thread_tests(fs_thread_t * fs_thread) { 

    for (int i = 0; i < fs_thread->n_threads; i++) { 
        create_fs_thread(fs_thread, funcs[i%4]);
    }

    for (int i = 0; i < fs_thread->n_threads; i++) { 
        if (pthread_join(fs_thread->threads[i], NULL) != 0) 
            return -1;
    }
    return 0;
}

int main() {
    assert(tfs_init(NULL) != -1);

    init_files();
    fs_thread_t * fs_thread = init_fs_thread(NUMBER_OF_THREADS);

    assert(run_thread_tests(fs_thread) != -1);

    destroy_fs_thread(fs_thread);
    
    assert(tfs_destroy() != -1);

    printf("Successful test.");
    return 0;
}