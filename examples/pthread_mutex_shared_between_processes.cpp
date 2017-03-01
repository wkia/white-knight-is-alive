#include <cstdio>
#include <iostream>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

using std::cout;

const int shared_segment_size = 0x6400;
const key_t key = 55555;


int main(void)
{
        pthread_mutex_t *shm_mutex = 0;
        int err;
        cout << __LINE__ << " : forking...\n";
        pid_t pid = fork();

        if (0 != pid)
        {
                /* Allocate a shared memory segment. */
                cout << __LINE__ << " : allocating shm..." << pid << "\n";
                int segment_id = shmget (key, shared_segment_size,
                                IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

                /* Attach the shared memory segment. */
                cout << __LINE__ << " : attaching shm " << segment_id << "... " << pid << "\n";
                char *shared_memory = (char*) shmat (segment_id, 0, 0);

                /* Determine the segment.s size. */
                shmid_ds shmbuffer;
                shmctl (segment_id, IPC_STAT, &shmbuffer);
                int segment_size = shmbuffer.shm_segsz;
                printf ("segment size: %d\n", segment_size);

                /* Write a string to the shared memory segment. */
                //sprintf (shared_memory, .Hello, world..);

                shm_mutex = (pthread_mutex_t *)shared_memory;

                cout << __LINE__ << " : initializing mutex... "  << pid << "\n";
                pthread_mutexattr_t attr;
                err = pthread_mutexattr_init(&attr); if (err) return err;
                err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); if (err) return err;
                err = pthread_mutex_init(shm_mutex, &attr); if (err) return err;
                err = pthread_mutexattr_destroy(&attr); if (err) return err;

                cout << __LINE__ << " : sleeping... : " << pid << "\n";
                sleep(5);
                cout << __LINE__ << " : locking... : " << pid << "\n";
                err = pthread_mutex_lock(shm_mutex); if (err) return err;
                cout << __LINE__ << " : locked : " << pid << "\n";
                sleep(2);
                cout << __LINE__ << " : unlocking... : " << pid << "\n";
                err = pthread_mutex_unlock(shm_mutex); if (err) return err;
                cout << __LINE__ << " : destroying... : " << pid << "\n";
                err = pthread_mutex_destroy(shm_mutex); if (err) return err;

                /* Detach the shared memory segment. */
                shmdt (shared_memory);
                /* Deallocate the shared memory segment. */
                shmctl (segment_id, IPC_RMID, 0);
        }
        else
        {
                sleep(2);

                /* Requesting an existing shared memory segment. */
                cout << __LINE__ << " : allocating shm..." << pid << "\n";
                int segment_id = shmget (key, shared_segment_size,
                                IPC_EXCL | S_IRUSR | S_IWUSR);

                /* Attach the shared memory segment. */
                cout << __LINE__ << " : attaching shm " << segment_id << "... " << pid << "\n";
                char *shared_memory = (char*) shmat (segment_id, 0, 0);

                /* Determine the segment.s size. */
                shmid_ds shmbuffer;
                shmctl (segment_id, IPC_STAT, &shmbuffer);
                int segment_size = shmbuffer.shm_segsz;
                printf ("segment size: %d\n", segment_size);

                shm_mutex = (pthread_mutex_t *)shared_memory;

                /* Write a string to the shared memory segment. */
                //sprintf (shared_memory, .Hello, world..);

                cout << __LINE__ << " : locking... : " << pid << "\n";
                err = pthread_mutex_lock(shm_mutex); if (err) return err;
                cout << __LINE__ << " : locked : " << pid << "\n";
                cout << __LINE__ << " : sleeping... : " << pid << "\n";
                sleep(10);
                cout << __LINE__ << " : unlocking... : " << pid << "\n";
                err = pthread_mutex_unlock(shm_mutex); if (err) return err;

                /* Detach the shared memory segment. */
                shmdt (shared_memory);
                /* Deallocate the shared memory segment. */
                shmctl (segment_id, IPC_RMID, 0);
        }
        return 0;
}
                                                                                      
