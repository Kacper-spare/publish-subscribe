#include "queue.h"

#include <unistd.h>

void* function(void* args)
{
    pthread_t t1 = pthread_self();
    subscribe((TQueue*)args, t1);
    for (int i = 0; i < 95; i++)
    {
        getMsg((TQueue*)args, t1);
    }
    unsubscribe((TQueue*)args, t1);
    return NULL;
}

int main()
{  
    int sizeOfQueue = 20;
    TQueue* queue = createQueue(sizeOfQueue);

    int var = 1;
    int *m1 = &var;
    int array[100] = {0};
    pthread_t t1 = pthread_self();
    pthread_t test[10];
    addMsg(queue, m1);
    for (int i = 0; i < 100; i++)
    {
        addMsg(queue, array+i);
        if (i < 10)
        {
            pthread_create(&test[i], NULL, function, (void*)queue);
        }
    }
    for (int i = 0; i < 10; i++)
    {
        pthread_join(test[i], NULL);
    }
    destroyQueue(queue);
}