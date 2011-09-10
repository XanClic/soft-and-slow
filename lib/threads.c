#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include <soft-and-slow/threads.h>


typedef struct sas_thread_info sas_thread_info_t;

struct sas_thread_info
{
    pthread_t id;

    pthread_cond_t work_available;
    pthread_mutex_t wa_mutex;

    bool unemployed;

    void (*execute)(void *param);
    void *param;
};


int sas_thread_count;
static sas_thread_info_t *threads;
static sem_t free_threads;


static void *thread_entry(void *i)
{
    sas_thread_info_t *info = i;

    pthread_mutex_lock(&info->wa_mutex);

    info->unemployed = true;


    for (;;)
    {
        pthread_cond_wait(&info->work_available, &info->wa_mutex);


        info->execute(info->param);
        free(info->param);


        info->unemployed = true;

        sem_post(&free_threads);
    }


    return NULL;
}


void sas_spawn_threads(void)
{
    sas_thread_count = sysconf(_SC_NPROCESSORS_ONLN);

    if (sas_thread_count < 1)
        sas_thread_count = 1;

    sas_thread_count *= 2;


    if (sem_init(&free_threads, 0, sas_thread_count))
    {
        fprintf(stderr, "[S&S] Could not init free threads semaphore.\n");
        abort();
    }


    threads = calloc(sas_thread_count, sizeof(sas_thread_info_t));


    for (int i = 0; i < sas_thread_count; i++)
    {
        if (pthread_mutex_init(&threads[i].wa_mutex, NULL))
        {
            fprintf(stderr, "[S&S] Could not create all necessary threads (could not init mutex).\n");
            abort();
        }

        if (pthread_create(&threads[i].id, NULL, &thread_entry, &threads[i]))
        {
            fprintf(stderr, "[S&S] Could not create all necessary threads.\n");
            abort();
        }
    }
}

void sas_thread_execute(void (*function)(void *data), void *data, size_t size)
{
    void *tmp = malloc(size);
    memcpy(tmp, data, size);


    sem_wait(&free_threads);

    // No really atomic operations needed here, because there is just one thread
    // which may execute this function
    for (int i = 0; i < sas_thread_count; i++)
    {
        if (threads[i].unemployed)
        {
            threads[i].unemployed = false;

            threads[i].execute = function;
            threads[i].param = tmp;

            // pthread_cond_wait will release the mutex and we have to wait
            // until the thread is waiting for the condition signal (else
            // our signal will get lost).
            pthread_mutex_lock(&threads[i].wa_mutex);
            pthread_mutex_unlock(&threads[i].wa_mutex);

            pthread_cond_signal(&threads[i].work_available);

            break;
        }
    }
}

void sas_wait_for_thread_pool(void)
{
    int sem_val = 0;

    for (;;)
    {
        sem_getvalue(&free_threads, &sem_val);
        if (sem_val >= sas_thread_count)
            break;

        pthread_yield();
    }
}
