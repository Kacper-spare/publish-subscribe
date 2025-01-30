#include "queue.h"

#include <unistd.h>

int main()
{  
    TQueue* queue = createQueue(3);
    destroyQueue(queue);
    
}