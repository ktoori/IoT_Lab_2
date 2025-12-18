

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "my_rand.h"
#include "timer.h"

#ifdef USE_CUSTOM
#include "my_rwlock.h"
#define rwlock_t my_rwlock_t
#define rwlock_init(l) my_rwlock_init(l)
#define rwlock_destroy(l) my_rwlock_destroy(l)
#define rwlock_rdlock(l) my_rwlock_rdlock(l)
#define rwlock_wrlock(l) my_rwlock_wrlock(l)
#define rwlock_unlock(l) my_rwlock_unlock(l)
#else
#define rwlock_t pthread_rwlock_t
#define rwlock_init(l) pthread_rwlock_init((l), NULL)
#define rwlock_destroy(l) pthread_rwlock_destroy(l)
#define rwlock_rdlock(l) pthread_rwlock_rdlock(l)
#define rwlock_wrlock(l) pthread_rwlock_wrlock(l)
#define rwlock_unlock(l) pthread_rwlock_unlock(l)
#endif

const int MAX_KEY = 100000000;
const int DEFAULT_INITIAL_INSERTS = 100;
const int DEFAULT_TOTAL_OPS = 10000;
const double DEFAULT_SEARCH_PERCENT = 0.8;
const double DEFAULT_INSERT_PERCENT = 0.1;

struct list_node_s {
    int data;
    struct list_node_s* next;
};

struct list_node_s* head = NULL;
int thread_count;
int total_ops;
double search_percent;
double insert_percent;
double delete_percent;
rwlock_t rwlock;
pthread_mutex_t count_mutex;
int member_count = 0, insert_count = 0, delete_count = 0;


void Usage(char* prog_name);
void Get_input(int* inserts_in_main_p);
void* Thread_work(void* rank);


int Insert(int value);
void Print(void);
int Member(int value);
int Delete(int value);
void Free_list(void);
int Is_empty(void);


int main(int argc, char* argv[]) {
    long i;
    int key, success, attempts;
    pthread_t* thread_handles;
    int inserts_in_main;
    unsigned seed = 1;
    double start, finish;
    
    if (argc != 2) Usage(argv[0]);
    
    thread_count = strtol(argv[1], NULL, 10);
    Get_input(&inserts_in_main);
    

    i = attempts = 0;
    while (i < inserts_in_main && attempts < 2*inserts_in_main) {
        key = my_rand(&seed) % MAX_KEY;
        success = Insert(key);
        attempts++;
        if (success) i++;
    }
    
    printf("=================================================\n");
#ifdef USE_CUSTOM
    printf("Using CUSTOM RWLock implementation\n");
#else
    printf("Using PTHREAD_RWLOCK_T (standard library)\n");
#endif
    printf("=================================================\n");
    printf("Inserted %ld keys in empty list\n", i);
    printf("Initial list size: %d\n", (int)i);
    printf("Threads: %d\n", thread_count);
    printf("Total operations: %d\n", total_ops);
    printf("Search/Insert/Delete ratio: %.1f%% / %.1f%% / %.1f%%\n",
           search_percent*100, insert_percent*100, delete_percent*100);
    printf("=================================================\n");

    thread_handles = malloc(thread_count * sizeof(pthread_t));
    

    pthread_mutex_init(&count_mutex, NULL);
    rwlock_init(&rwlock);

    GET_TIME(start);
    
    for (i = 0; i < thread_count; i++)
        pthread_create(&thread_handles[i], NULL, Thread_work, (void*) i);
    
    for (i = 0; i < thread_count; i++)
        pthread_join(thread_handles[i], NULL);
    
    GET_TIME(finish);
    

    printf("\nResults:\n");
    printf("Elapsed time = %e seconds\n", finish - start);
    printf("Total operations completed = %d\n", total_ops);
    printf("Operations per thread = %d\n", total_ops / thread_count);
    printf("\nOperation breakdown:\n");
    printf("  Member (search) ops = %d (%.1f%%)\n", member_count, 
           member_count*100.0/total_ops);
    printf("  Insert ops = %d (%.1f%%)\n", insert_count,
           insert_count*100.0/total_ops);
    printf("  Delete ops = %d (%.1f%%)\n", delete_count,
           delete_count*100.0/total_ops);
    printf("\nOperations per second: %e\n", total_ops / (finish - start));
    printf("=================================================\n");
    
    Free_list();
    rwlock_destroy(&rwlock);
    pthread_mutex_destroy(&count_mutex);
    free(thread_handles);
    
    return 0;
}


void Usage(char* prog_name) {
    fprintf(stderr, "Usage: %s <number_of_threads>\n", prog_name);
    exit(0);
}

void Get_input(int* inserts_in_main_p) {
    printf("How many keys should be inserted in main thread? [%d] ",
           DEFAULT_INITIAL_INSERTS);
    if (scanf("%d", inserts_in_main_p) != 1) {
        *inserts_in_main_p = DEFAULT_INITIAL_INSERTS;
        while (getchar() != '\n'); 
    }
    
    printf("How many total operations? [%d] ", DEFAULT_TOTAL_OPS);
    if (scanf("%d", &total_ops) != 1) {
        total_ops = DEFAULT_TOTAL_OPS;
        while (getchar() != '\n');
    }
    
    printf("Percent of ops that should be searches? [%.1f] ",
           DEFAULT_SEARCH_PERCENT);
    if (scanf("%lf", &search_percent) != 1) {
        search_percent = DEFAULT_SEARCH_PERCENT;
        while (getchar() != '\n');
    }
    
    printf("Percent of ops that should be inserts? [%.1f] ",
           DEFAULT_INSERT_PERCENT);
    if (scanf("%lf", &insert_percent) != 1) {
        insert_percent = DEFAULT_INSERT_PERCENT;
        while (getchar() != '\n');
    }
    
    delete_percent = 1.0 - (search_percent + insert_percent);
    if (delete_percent < 0.0) {
        printf("Invalid percentages! Adjusting...\n");
        delete_percent = 0.1;
        insert_percent = 0.1;
        search_percent = 0.8;
    }
}


int Insert(int value) {
    struct list_node_s* curr = head;
    struct list_node_s* pred = NULL;
    struct list_node_s* temp;
    int rv = 1;
    
    while (curr != NULL && curr->data < value) {
        pred = curr;
        curr = curr->next;
    }
    
    if (curr == NULL || curr->data > value) {
        temp = malloc(sizeof(struct list_node_s));
        temp->data = value;
        temp->next = curr;
        if (pred == NULL)
            head = temp;
        else
            pred->next = temp;
    } else {
        rv = 0;  
    }
    
    return rv;
}


int Member(int value) {
    struct list_node_s* temp;
    
    temp = head;
    while (temp != NULL && temp->data < value)
        temp = temp->next;
    
    if (temp == NULL || temp->data > value)
        return 0;
    else
        return 1;
}


int Delete(int value) {
    struct list_node_s* curr = head;
    struct list_node_s* pred = NULL;
    int rv = 1;
    
    while (curr != NULL && curr->data < value) {
        pred = curr;
        curr = curr->next;
    }
    
    if (curr != NULL && curr->data == value) {
        if (pred == NULL)
            head = curr->next;
        else
            pred->next = curr->next;
        free(curr);
    } else {
        rv = 0;
    }
    
    return rv;
}


void Print(void) {
    struct list_node_s* temp;
    
    printf("list = ");
    temp = head;
    while (temp != NULL) {
        printf("%d ", temp->data);
        temp = temp->next;
    }
    printf("\n");
}


void Free_list(void) {
    struct list_node_s* current;
    struct list_node_s* following;
    
    if (Is_empty()) return;
    
    current = head;
    following = current->next;
    
    while (following != NULL) {
        free(current);
        current = following;
        following = current->next;
    }
    free(current);
}


int Is_empty(void) {
    return (head == NULL);
}


void* Thread_work(void* rank) {
    long my_rank = (long) rank;
    int i, val;
    double which_op;
    unsigned seed = my_rank + 1;
    int my_member_count = 0, my_insert_count = 0, my_delete_count = 0;
    int ops_per_thread = total_ops / thread_count;
    
    for (i = 0; i < ops_per_thread; i++) {
        which_op = my_drand(&seed);
        val = my_rand(&seed) % MAX_KEY;
        
        if (which_op < search_percent) {
            
            rwlock_rdlock(&rwlock);
            Member(val);
            rwlock_unlock(&rwlock);
            my_member_count++;
        } else if (which_op < search_percent + insert_percent) {
           
            rwlock_wrlock(&rwlock);
            Insert(val);
            rwlock_unlock(&rwlock);
            my_insert_count++;
        } else {
        
            rwlock_wrlock(&rwlock);
            Delete(val);
            rwlock_unlock(&rwlock);
            my_delete_count++;
        }
    }
    

    pthread_mutex_lock(&count_mutex);
    member_count += my_member_count;
    insert_count += my_insert_count;
    delete_count += my_delete_count;
    pthread_mutex_unlock(&count_mutex);
    
    return NULL;
}