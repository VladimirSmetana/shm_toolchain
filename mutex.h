#ifndef MUT
#define MUT

#include "common.h"
#include <shared_mutex>


struct SemaphoreGuard
{
    sem_t* sem;
    const char* name;
    SemaphoreGuard(sem_t* s, const char* n)
        : sem(s)
        , name(n)
    {
        if (sem == SEM_FAILED)
        {
            perror("sem_open");
            exit(EXIT_FAILURE);
        }
    }
    ~SemaphoreGuard()
    {
        sem_close(sem);
        sem_unlink(name);
    }
};


#endif // MUT