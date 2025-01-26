#include "queue.h"

#include <unistd.h>

void* function(void* args)
{
    subscribe((TQueue*) args, pthread_self());
    for (int i = 0; i < 10; i++)
    {
        printf("%d %lu\n", *(int*)getMsg((TQueue*) args, pthread_self()), pthread_self());
    }
    unsubscribe((TQueue*) args, pthread_self());
}

int main()
{
    /* Plik main.c nie musi posiadac zadnych dodatkowych informacji (prawdopodobnie biblioteka tutaj 
     * dodana jest rowniez niepotrzebana), wszystkie algorytmy zwiazane z kolejka sa w pliku queue.c, 
     * a ich definicje i opis w pliku queue.h 
     * Nie powinno sie probowac otrzymac dostepu do kolejki w trakcie jej usuwania poniewaz glowny mutex
     * blokujacy dostep do struktury TQueue musi zostac zwolniony przed jego usunieciem (w przeciwnym wypadku
     * mamy do czynienia z undefined behavior)
     * Struktura TQueue zawiera wlasne zmienne synchronizujace sa to zmienne typu mutex oraz conditional variable
     * Zamki mutex sa uzywane jako semafory wielowartosciowe poprzez zwiekszanie zmiennej mowiecej o ilosci
     * watkow aktualnie czytajacych z struktury TQueue
     */
    int sizeOfQueue = 10;
    int array[20] = {0};
    TQueue* queue = createQueue(sizeOfQueue);

    pthread_t threads[7];
    
    for (int i = 0; i < 7; i++)
    {
        pthread_create(&threads[i], NULL, function, (void*) queue);
    }
    for (int i = 0; i < 20; i++)
    {
        array[i] = i;
        addMsg(queue, (void*)(&array[i]));
    }
    
    
    for (int i = 0; i < 7; i++)
    {
        pthread_join(threads[i], NULL);
    }
    destroyQueue(queue);
}