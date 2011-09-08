#ifndef SAS_THREADS_H
#define SAS_THREADS_H

// Creates 2 * n threads in the thread pool, where n is the number of CPU cores.
void sas_spawn_threads(void);
// Executes the given function via a thread and copies the given memory area
// into a new one, which is eventually given as a parameter to that function.
void sas_thread_execute(void (*function)(void *data), void *data, size_t size);
// Waits until all threads are waiting for data (i.e., the pool is full).
void sas_wait_for_thread_pool(void);

#endif
