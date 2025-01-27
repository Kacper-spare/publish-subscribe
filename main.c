#include "queue.h"

#include <unistd.h>

void* function(void* args)
{
    unsigned long t1 = pthread_self();
    TQueue* queue = (TQueue*)args;
    int var = 1;
    int *m1 = &var;
    subscribe(queue, t1);
    addMsg(queue, m1);
    printf("%d", getAvailable(queue, t1));
    return NULL;
}

int main()
{  
    int sizeOfQueue = 10;
    // int array[20] = {0};
    TQueue* queue = createQueue(sizeOfQueue);
    
    // pthread_t t1;
    // pthread_create(&t1, NULL, function, queue);
    // pthread_join(t1, NULL);

    int var = 1;
    int var1 = 1;
    int *m2 = &var1;
    int *m1 = &var;
    pthread_t t1 = pthread_self();
    subscribe(queue, t1);
    addMsg(queue, m1);
    addMsg(queue, m2);
    printf("%d \n", getAvailable(queue, t1));
    
    destroyQueue(queue);
}