#include "fs_thread.h"


int init_fs_thread(fs_thread_t * fs_thread, int n_threads) { 
    if(pthread_mutex_init(&(fs_thread->mutex), NULL) == -1) return -1;
    fs_thread->n_threads = n_threads;
    return 0;
}

void destroy_fs_thread (fs_thread_t * fs_thread) {
    /*
    for (int i = 0; i < fs_thread->n_threads; i++) { 
        free(&(fs_thread->threads[i]));
    }
    */ 
    pthread_mutex_destroy(&(fs_thread->mutex));
    free(fs_thread);
}


int create_fs_thread (fs_thread_t * fs_thread, void * fun) {
    fun = (char*)fun;
    for (int i = 0; i < fs_thread->n_threads; i++) { 
        if(fs_thread->threads[i] == 0) {
            if(pthread_create(&(fs_thread->threads[i]), NULL, fun, NULL) != 0) {
                return -1;
            }
            printf("threads%d -> %ld\n", i, fs_thread->threads[i]);
            return 0;
        } 
    }
    return -1;
}