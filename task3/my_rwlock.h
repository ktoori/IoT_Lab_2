

#ifndef MY_RWLOCK_H
#define MY_RWLOCK_H

#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;          
    pthread_cond_t readers_ok;      
    pthread_cond_t writers_ok;       
    int read_count;                 
    int write_count;                 
    int read_waiters;                
    int write_waiters;               
} my_rwlock_t;

int my_rwlock_init(my_rwlock_t* lock);

int my_rwlock_destroy(my_rwlock_t* lock);

int my_rwlock_rdlock(my_rwlock_t* lock);


int my_rwlock_wrlock(my_rwlock_t* lock);

int my_rwlock_unlock(my_rwlock_t* lock);

#endif /* MY_RWLOCK_H */