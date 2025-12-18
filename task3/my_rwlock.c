

#include <stdlib.h>
#include <errno.h>
#include "my_rwlock.h"


int my_rwlock_init(my_rwlock_t* lock) {
    int ret;
    
    if (lock == NULL) return EINVAL;
    
    ret = pthread_mutex_init(&lock->mutex, NULL);
    if (ret != 0) return ret;
    
    ret = pthread_cond_init(&lock->readers_ok, NULL);
    if (ret != 0) {
        pthread_mutex_destroy(&lock->mutex);
        return ret;
    }

    ret = pthread_cond_init(&lock->writers_ok, NULL);
    if (ret != 0) {
        pthread_cond_destroy(&lock->readers_ok);
        pthread_mutex_destroy(&lock->mutex);
        return ret;
    }
    

    lock->read_count = 0;
    lock->write_count = 0;
    lock->read_waiters = 0;
    lock->write_waiters = 0;
    
    return 0;
}


int my_rwlock_destroy(my_rwlock_t* lock) {
    int ret = 0;
    
    if (lock == NULL) return EINVAL;
    
    pthread_cond_destroy(&lock->readers_ok);
    pthread_cond_destroy(&lock->writers_ok);
    pthread_mutex_destroy(&lock->mutex);
    
    return ret;
}


int my_rwlock_rdlock(my_rwlock_t* lock) {
    int ret;
    
    if (lock == NULL) return EINVAL;
    
    ret = pthread_mutex_lock(&lock->mutex);
    if (ret != 0) return ret;
  
    lock->read_waiters++;
    while (lock->write_count > 0 || lock->write_waiters > 0) {
        ret = pthread_cond_wait(&lock->readers_ok, &lock->mutex);
        if (ret != 0) {
            lock->read_waiters--;
            pthread_mutex_unlock(&lock->mutex);
            return ret;
        }
    }
    lock->read_waiters--;
    
    lock->read_count++;
    
    pthread_mutex_unlock(&lock->mutex);
    return 0;
}


int my_rwlock_wrlock(my_rwlock_t* lock) {
    int ret;
    if (lock == NULL) return EINVAL;
    
    ret = pthread_mutex_lock(&lock->mutex);
    if (ret != 0) return ret;
    
  
    lock->write_waiters++;
    while (lock->read_count > 0 || lock->write_count > 0) {
        ret = pthread_cond_wait(&lock->writers_ok, &lock->mutex);
        if (ret != 0) {
            lock->write_waiters--;
            pthread_mutex_unlock(&lock->mutex);
            return ret;
        }
    }
    lock->write_waiters--;
    lock->write_count = 1;
    pthread_mutex_unlock(&lock->mutex);
    return 0;
}


int my_rwlock_unlock(my_rwlock_t* lock) {
    int ret;
    if (lock == NULL) return EINVAL;
    ret = pthread_mutex_lock(&lock->mutex);
    if (ret != 0) return ret;
    
    if (lock->write_count > 0) {
        lock->write_count = 0;
        pthread_cond_broadcast(&lock->readers_ok);
        pthread_cond_broadcast(&lock->writers_ok);
    } else if (lock->read_count > 0) {     
        lock->read_count--;
   
        if (lock->read_count == 0) {
            pthread_cond_broadcast(&lock->writers_ok);
        }
    }
    
    pthread_mutex_unlock(&lock->mutex);
    return 0;
}