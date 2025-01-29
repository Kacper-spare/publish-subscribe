#ifndef LCL_QUEUE_H
#define LCL_QUEUE_H

// ==============================================
//
//  Version 1.1, 2025-01-16
//
// ==============================================

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

//assisting structure for dynamic list of subscribed threads without using dynamic array with malloc
//structure is a singly linked list
typedef struct Node
{
    //next index of message to read by thread
    int head;
    //pointer to id of a thread (taken from pthread_self() function)
    pthread_t data;
    //pointer to next item of a list
    struct Node* next;
} TNode;

struct TQueue
{
    //used only for condition value when there is no message to receive
    pthread_cond_t lockGetMsg;
    //used only for condition value when queue is full
    pthread_cond_t lockAddMsg;
    //used only for condition value when waiting for all readers to finish reading (writer may starve)
    pthread_cond_t lockEditing;
    //used for critical sections while editing values in queue
    pthread_mutex_t mutexEditing;

    //size of messageArray
    int capacity;
    //last index with valuable information past this all values in messageArray should be NULL
    int tail;
    //shows the start of the queue, used for delaying the deletion of messages
    int start;
    //singly linked list of all currenly subscribed threads
    TNode* subscribers;
    //array of any information
    void** messageArray;
};

typedef struct TQueue TQueue;

//used to create new nodes for list of subscribers
TNode* newNode(pthread_t thread, TQueue* queue);

//deletes first node returns NULL on failure to delete
TNode* removeNode(TNode* head);

//creates queue size should be greater than 0
TQueue* createQueue(int size);

//deletes queue (not safe to use queue when its being deleted because of mutex not being locked before deletion of said mutex)
void destroyQueue(TQueue *queue);

//adds a subscriber to the queue
void subscribe(TQueue *queue, pthread_t thread);

//removes subscriber from queue
//also removes all read messages when all subscribers point past them
//e.g. when every subscriber->head points past first two messages then those messages are deleted
void unsubscribe(TQueue *queue, pthread_t thread);

//adds message to queue, when queue is full the thread is susspended with condition variable
void addMsg(TQueue *queue, void *msg);

//receives message from queue for thread, when there are no messages to receive then thread is susspended with conditional variable
//when thread is the last one to read message it deletes the message
void* getMsg(TQueue *queue, pthread_t thread);

//return how many messages are left to read for thread returs -1 when queue is NULL
int getAvailable(TQueue *queue, pthread_t thread);

//removes given message from queue, may just shift the index by 1 to right
void removeMsg(TQueue *queue, void *msg);

//shifts queue to start at index 0 again
void shift(TQueue *queue);

//sets size of queue to size, when size is smaller than previous it deletes the oldest messages
void setSize(TQueue *queue, int size);

#endif //LCL_QUEUE_H