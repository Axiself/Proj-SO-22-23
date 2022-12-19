#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


typedef struct {

    int n_threads;

    pthread_mutex_t mutex;
    pthread_t threads[];
} fs_thread_t;

int init_fs_thread (fs_thread_t * thread, int n_threads);

void destroy_fs_thread (fs_thread_t * thread);

int create_fs_thread (fs_thread_t * fs_thread, void * fun);


