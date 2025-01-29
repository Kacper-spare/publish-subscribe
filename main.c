#include "queue.h"

#include <unistd.h>

int main()
{  
    int sizeOfQueue = 3;
    TQueue* queue = createQueue(sizeOfQueue);
    destroyQueue(queue);
    
}