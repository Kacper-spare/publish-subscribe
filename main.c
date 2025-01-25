#include "queue.h"

#include <unistd.h>


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
    TQueue* queue = createQueue(sizeOfQueue);
}