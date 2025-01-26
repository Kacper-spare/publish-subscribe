#include "queue.h"

//linked list functions

TNode* newNode(pthread_t* thread, TQueue* queue)
{
    TNode* node = (TNode*) malloc(sizeof(TNode));
    if (node == NULL)
    {
        perror("malloc node memory allocation error\n");
        exit(0);
    }
    node->head = queue->tail + 1;
    node->data = thread;
    node->next = NULL;
    return node;
}

TNode* removeNode(TNode* head)
{
    if (head == NULL)
    {
        return NULL;
    }
    TNode* toDel = head;
    head = head->next;
    free(toDel);
    return head;
}

//queue functions

TQueue* createQueue(int size)
{
    //returs NULL when wronf size is given
    if (size <= 0)
    {
        // printf("Incorrect size of queue\n");
        return NULL;
    }

    TQueue* queue = malloc(sizeof(struct TQueue));
    if (queue == NULL)
    {
        perror("malloc queue memory allocation error");
        exit(0);
    }
    
    //initializing mutex to lock queue 
    pthread_mutex_init(&queue->mutexEditing, NULL);
    pthread_mutex_lock(&queue->mutexEditing);

    //default values of variables for queue
    queue->activeReaders = 0;    
    queue->tail = -1;
    queue->capacity = size;
    queue->messageArray = malloc(queue->capacity*sizeof(void*));
    queue->subscribers = NULL;
    
    //initialization of all locks
    pthread_cond_init(&queue->lockGetMsg, NULL);
    pthread_cond_init(&queue->lockAddMsg, NULL);
    pthread_cond_init(&queue->lockEditing, NULL);

    pthread_mutex_init(&queue->mutexGetMsg, NULL);
    pthread_mutex_init(&queue->mutexAddMsg, NULL);
    pthread_mutex_init(&queue->mutexEditing, NULL);

    pthread_mutex_unlock(&queue->mutexEditing);
    return queue;
}

void destroyQueue(TQueue* queue)
{
    //returns when queue is NULL
    if (queue == NULL)
    {
        return;
    }

    //critical section for value editing purposses
    pthread_mutex_lock(&queue->mutexEditing);
    while (queue->activeReaders != 0)
    {
        pthread_cond_wait(&queue->lockEditing, &queue->mutexEditing);
    }

    //deleting allocated memory 
    if (queue->messageArray != NULL)
    {
        free(queue->messageArray);
    }
    queue->messageArray = NULL;
    if (queue->subscribers != NULL)
    {
        while (removeNode((queue->subscribers)) == NULL);
    }

    pthread_cond_destroy(&queue->lockAddMsg);
    pthread_cond_destroy(&queue->lockGetMsg);

    pthread_mutex_destroy(&queue->mutexAddMsg);
    pthread_mutex_destroy(&queue->mutexGetMsg);

    pthread_cond_destroy(&queue->lockEditing);

    //exiting critical section due to mutex deleting 
    pthread_mutex_unlock(&queue->mutexEditing);
    pthread_mutex_destroy(&queue->mutexEditing);

    free(queue);
    queue = NULL;
    return;
}

void setSize(TQueue* queue, int size)
{
    //checking if values are proper
    if (queue == NULL)
    {
        printf("Incorrect queue pointer\n");
        exit(0);
    }
    if (size <= 0)
    {
        // printf("Incorect value of size\n");
        return;
    }

    //critical section due to editing values
    pthread_mutex_lock(&queue->mutexEditing);
    while (queue->activeReaders != 0)
    {
        pthread_cond_wait(&queue->lockEditing, &queue->mutexEditing);
    }

    //algorythmic part
    int delta = queue->tail - size + 1;
    TNode* subscriber = queue->subscribers;
    if (delta > 0)    
    {
        for (int i = 0; i+delta <= queue->tail; i++)
        {
            queue->messageArray[i] = queue->messageArray[i+delta];

        }
        while (subscriber != NULL)
        {
            subscriber->head -= delta;
            if (subscriber->head < 0)
            {
                subscriber->head = 0;
            }
            subscriber = subscriber->next;
        }
    }
    queue->messageArray = realloc(queue->messageArray, size*sizeof(void*));

    //if realloc fails 
    if (queue->messageArray == NULL)
    {
        perror("Realloc failed");
        exit(0);
    }

    //set values in queue approperly
    queue->capacity = size;
    if (queue->capacity <= queue->tail)
    {
        queue->tail = queue->capacity - 1;
    }

    pthread_mutex_unlock(&queue->mutexEditing);
    return;
}

void* getMsg(TQueue *queue, pthread_t thread)
{
    //checking if values are proper
    if (queue == NULL)
    {
        printf("Queue isn't initiated\n");
        exit(0);
    }

    //critical section for reading purposes
    pthread_mutex_lock(&queue->mutexEditing);
    queue->activeReaders++;
    pthread_mutex_unlock(&queue->mutexEditing);

    //algorythmic part
    TNode* subscriber = queue->subscribers;
    while (subscriber != NULL && *(subscriber->data) != thread)
    {
        subscriber = subscriber->next;
    }
    if (subscriber == NULL)
    {
        pthread_mutex_lock(&queue->mutexEditing);
        queue->activeReaders--;
        if (queue->activeReaders == 0)
        {
            pthread_cond_broadcast(&queue->lockEditing);
        }
        pthread_mutex_unlock(&queue->mutexEditing);
        return NULL;
    }

    //this critical section may be a bit exessive 
    pthread_mutex_lock(&queue->mutexEditing);
    queue->activeReaders--;
    if (queue->activeReaders == 0)
    {
        pthread_cond_broadcast(&queue->lockEditing);
    }

    //thread suspension
    while (subscriber->head > queue->tail)
    {
        pthread_mutex_unlock(&queue->mutexEditing);
        pthread_mutex_lock(&queue->mutexGetMsg);
        pthread_cond_wait(&queue->lockGetMsg, &queue->mutexGetMsg);
        pthread_mutex_unlock(&queue->mutexGetMsg);
        pthread_mutex_lock(&queue->mutexEditing);
    }
    queue->activeReaders++;
    pthread_mutex_unlock(&queue->mutexEditing);
    //rest of algorythm
    void* toReturn = (void*) queue->messageArray[subscriber->head];
    pthread_mutex_lock(&queue->mutexEditing);
    queue->activeReaders--;
    if (queue->activeReaders == 0)
    {
        pthread_cond_broadcast(&queue->lockEditing);
    }
    subscriber->head++;
    queue->activeReaders++;
    pthread_mutex_unlock(&queue->mutexEditing);
    TNode* allSubscribers = queue->subscribers;

    //checking if every subscriber has already read this message (only deletes first messages due to how this structure works)
    while (allSubscribers != NULL && allSubscribers->head >= subscriber->head)
    {
        allSubscribers = allSubscribers->next;
    }
    if (allSubscribers == NULL)
    {
        pthread_mutex_lock(&queue->mutexEditing);
        queue->activeReaders--;
        if (queue->activeReaders == 0)
        {
            pthread_cond_broadcast(&queue->lockEditing);
        }
        //removing item
        pthread_mutex_unlock(&queue->mutexEditing);
        removeMsg(queue, toReturn);
    }
    else
    {
        pthread_mutex_lock(&queue->mutexEditing);
        queue->activeReaders--;
        if (queue->activeReaders == 0)
        {
            pthread_cond_broadcast(&queue->lockEditing);
        }
        pthread_mutex_unlock(&queue->mutexEditing);
    }
    return toReturn;
}

void removeMsg(TQueue *queue, void *msg)
{
    //checking if values are correct
    if (queue == NULL)
    {
        printf("Queue isn't initiated\n");
        exit(0);
    }

    //algorythmic part
    pthread_mutex_lock(&queue->mutexEditing);
    queue->activeReaders++;
    pthread_mutex_unlock(&queue->mutexEditing);
    
    //check if there are any messages
    if (queue->tail <= -1)
    {
        // printf("No msgs in queue to remove\n");
        pthread_mutex_lock(&queue->mutexEditing);
        queue->activeReaders--;
        if (queue->activeReaders == 0)
        {
            pthread_cond_broadcast(&queue->lockEditing);
        }
        pthread_mutex_unlock(&queue->mutexEditing);
        return;
    }

    //algorythmic part
    int i = 0;
    //search for given message in queue
    while (i < queue->tail+1 && queue->messageArray[i] != msg)
    {
        i++;
    }
    //check if message was found
    if (i >= queue->tail+1)
    {
        // printf("No msg found in queue\n");
        pthread_mutex_lock(&queue->mutexEditing);
        queue->activeReaders--;
        if (queue->activeReaders == 0)
        {
            pthread_cond_broadcast(&queue->lockEditing);
        }
        pthread_mutex_unlock(&queue->mutexEditing);
        return;
    }

    //critical section for editing
    pthread_mutex_lock(&queue->mutexEditing);
    queue->activeReaders--;
    if (queue->activeReaders == 0)
    {
        pthread_cond_broadcast(&queue->lockEditing);
    }
    //wait for all readers to finish
    while (queue->activeReaders != 0)
    {
        pthread_cond_wait(&queue->lockEditing, &queue->mutexEditing);
    }

    //moving subscriber heads
    TNode* subscriber = queue->subscribers;
    while (subscriber != NULL)
    {
        if (subscriber->head > i)
        {
            subscriber->head--;
        }
        subscriber = subscriber->next;
    }
    //shifting all items to the left after deleting the chosen item
    while (i < queue->tail)
    {
        queue->messageArray[i] = queue->messageArray[i+1];
        i++;
    }
    queue->messageArray[i] = NULL;
    //setting properly the tail
    queue->tail--;
    pthread_cond_signal(&queue->lockAddMsg);
    pthread_mutex_unlock(&queue->mutexEditing);
    return;
}

void addMsg(TQueue *queue, void *msg)
{
    //critical section reading
    pthread_mutex_lock(&queue->mutexEditing);
    queue->activeReaders++;
    pthread_mutex_unlock(&queue->mutexEditing);

    //checking variables
    if (queue == NULL)
    {
        printf("Queue isn't initiated\n");
        exit(0);
    }
    if (queue->subscribers == NULL)
    {
        pthread_mutex_lock(&queue->mutexEditing);
        queue->activeReaders--;
        if (queue->activeReaders == 0)
        {
            pthread_cond_broadcast(&queue->lockEditing);
        }
        pthread_mutex_unlock(&queue->mutexEditing);
        return;
    }

    //algorythmic part
    pthread_mutex_lock(&queue->mutexEditing);
    queue->activeReaders--;
    if (queue->activeReaders == 0)
    {
        pthread_cond_broadcast(&queue->lockEditing);
    }
    //suspending thread when queue is full
    while(queue->tail+1 >= queue->capacity)
    {
        pthread_mutex_unlock(&queue->mutexEditing);
        pthread_mutex_lock(&queue->mutexAddMsg);
        pthread_cond_wait(&queue->lockAddMsg, &queue->mutexAddMsg);
        pthread_mutex_unlock(&queue->mutexAddMsg);
        pthread_mutex_lock(&queue->mutexEditing);
    }
    while (queue->activeReaders != 0)
    {
        pthread_cond_wait(&queue->lockEditing, &queue->mutexEditing);
    }
    queue->tail++;
    queue->messageArray[queue->tail] = msg;
    pthread_mutex_unlock(&queue->mutexEditing);
    //sending signal that there is more things to read
    pthread_cond_broadcast(&queue->lockGetMsg);
    return;
}

int getAvailable(TQueue *queue, pthread_t thread)
{
    //critical section reading
    pthread_mutex_lock(&queue->mutexEditing);
    queue->activeReaders++;
    pthread_mutex_unlock(&queue->mutexEditing);

    //checking if queue is NULL
    if (queue == NULL)
    {
        // printf("Queue isn't initiated\n");
        return -1;
    }

    //algorythmic part
    TNode* subscriber = queue->subscribers;
    //searches for a subscriber thread
    while (subscriber != NULL && *(subscriber->data) != thread)
    {
        subscriber = subscriber->next;
    }
    //if not subscribed than returns 0
    if (subscriber == NULL)
    {
        return 0;
    }
    //number of messages that can be received without getting suspended
    int toReturn = queue->tail - subscriber->head + 1;
    pthread_mutex_lock(&queue->mutexEditing);
    queue->activeReaders--;
    if (queue->activeReaders == 0)
    {
        pthread_cond_broadcast(&queue->lockEditing);
    }
    pthread_mutex_unlock(&queue->mutexEditing);
    return toReturn;
}

void subscribe(TQueue *queue, pthread_t thread)
{
    //checking if queue is null
    if (queue == NULL)
    {
        printf("Queue isn't initiated\n");
        exit(0);
    }

    //critical section editing
    pthread_mutex_lock(&queue->mutexEditing);
    while (queue->activeReaders != 0)
    {
        pthread_cond_wait(&queue->lockEditing, &queue->mutexEditing);
    }

    //algorythmic part
    TNode** subscriber = &queue->subscribers;
    //check if this is 1st subscriber
    if (*subscriber == NULL)
    {
        *subscriber = newNode(&thread, queue);
        pthread_mutex_unlock(&queue->mutexEditing);
        return;
    }
    //iterate untill found thread or last item
    while ((*subscriber)->next != NULL && *((*subscriber)->data) != thread)
    {
        subscriber = &((*subscriber)->next);
    }
    //adds a new node at the end
    if ((*subscriber)->next == NULL)
    {
        (*subscriber)->next = newNode(&thread, queue);
        pthread_mutex_unlock(&queue->mutexEditing);
        return;
    }
    //found this thread to be already subscribed
    pthread_mutex_unlock(&queue->mutexEditing);
    // printf("Thread already subscribed to this queue\n");
    return;
}

void unsubscribe(TQueue *queue, pthread_t thread)
{
    //checking if queue is NULL
    if (queue == NULL)
    {
        printf("Queue isn't initiated\n");
        exit(0);
    }

    //critical section editing
    pthread_mutex_lock(&queue->mutexEditing);
    while (queue->activeReaders != 0)
    {
        pthread_cond_wait(&queue->lockEditing, &queue->mutexEditing);
    }

    //algorythmic part
    TNode** subscriber = &queue->subscribers;
    //check if there are any subscribers
    if (*subscriber == NULL)
    {
        // printf("Nobody is subscribed\n");
        pthread_mutex_unlock(&queue->mutexEditing);
        return;
    }
    //searches for the subscriber to unsubscribe
    while (*subscriber != NULL && *((*subscriber)->data) != thread)
    {
        subscriber = &(*subscriber)->next;
    }
    //check if subscriber was found
    if (*subscriber == NULL)
    {
        pthread_mutex_unlock(&queue->mutexEditing);
        // printf("Not yet subscribed\n");
        return;
    }
    //subscriber was found, now deletes him (unsubscribes him)
    *subscriber = removeNode(*subscriber);
    //deletes all messages because there are no subscribers
    if (queue->subscribers == NULL)
    {
        for (int i = 0; i <= queue->tail; i++)
        {
            queue->messageArray[i] = NULL;
        }
        queue->tail = -1;
        pthread_cond_broadcast(&queue->lockAddMsg);
        pthread_mutex_unlock(&queue->mutexEditing);
        return;
    }
    //algorythm to delete all read messages after removing a subscriber
    TNode* subscribed = queue->subscribers;
    int delta = subscribed->head;
    while (subscribed != NULL)
    {
        if (subscribed->head < delta)
        {
            delta = subscribed->head;
        }
        subscribed = subscribed->next;
    }
    subscribed = queue->subscribers;
    //check if there needs to be any deleted messages
    if (delta > 0)
    {
        for (int i = 0; i+delta <= queue->tail; i++)
        {
            queue->messageArray[i] = queue->messageArray[i+delta];
        }
        for (int i = queue->tail-delta+1; i <= queue->tail; i++)
        {
            queue->messageArray[i] = NULL;
        }
        while (subscribed != NULL)
        {
            subscribed->head -= delta;
            subscribed = subscribed->next;
        }
        queue->tail-=delta;
        pthread_cond_broadcast(&queue->lockAddMsg);
    }
    pthread_mutex_unlock(&queue->mutexEditing);
    return;
}