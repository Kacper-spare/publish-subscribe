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
    int tak = getAvailable(queue, t1);
    printf("%d", tak);
    return NULL;
}

int main()
{  
    int sizeOfQueue = 10;
    TQueue* queue = createQueue(sizeOfQueue);

    int var = 1;
    // void* var2;
    int *m1 = &var;
    pthread_t t1 = pthread_self();

    // pthread_t t2;
    // pthread_create(&t2, NULL, function, queue);
    // pthread_join(t2, NULL);
    subscribe(queue, t1);
    addMsg(queue, m1);    
    printf("%d \n", getAvailable(queue, t1));

    destroyQueue(queue);
}