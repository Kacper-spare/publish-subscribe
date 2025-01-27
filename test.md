---
lang:     pl-PL
fig_caption: false
---

```c
int main()
{  
    int sizeOfQueue = 10;
    TQueue* queue = createQueue(sizeOfQueue);
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
```

wynik programu `0`

```c
int main()
{  
    int sizeOfQueue = 10;
    TQueue* queue = createQueue(sizeOfQueue);
    int var = 1;
    int var1 = 1;
    int *m2 = &var1;
    int *m1 = &var;
    pthread_t t1 = pthread_self();
    subscribe(queue, t1);
    addMsg(queue, m1);
    addMsg(queue, m2);
    printf("%d \n%lu", getAvailable(queue, t1), pthread_self());
    destroyQueue(queue);
}
```

wynik programu `2`


