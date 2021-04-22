#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

// Dana jest następująca struktura:
struct CharRingBuffer {
    char data[30];
    char char_count, string_count;
    char *head;
    char *tail;
    // TODO (1):
};
// (1) Uzupełnij strukturę CharRingBuffer tak, aby mogła opisywać bufor kołowy rozpięty na tablicy "data"

// (2) Zdefiniuj poniżej typ o nazwie CharRingBufferPtr będący wskaźnikiem do struktury CharRingBuffer
// TODO (2):
typedef struct CharRingBuffer* CharRingBufferPtr;

// (3) Napisz funkcję inicjalizującą bufor kołowy
void init(CharRingBufferPtr buffer) 
{
        *buffer = (struct CharRingBuffer) {
                .head = &buffer->data[0],
                .tail = &buffer->data[0]
        };
}

// (4) Napisz funkcję, która wpisze do bufora kołowego łańcuch tekstowy "source".
//     Jeśli łańcuch ten nie zmieści się w całości do bufora, to nie należy go wogóle wpisywać.
//     Pamiętaj o tym, że każdy zapisany łańcuch znaków musi się kończyć zerem.
//     Funkcja ma zwracać 'true' jeśli udało się wpisać dany łańcuch lub 'false' w przeciwnym razie.
bool putText(CharRingBufferPtr buffer, const char* source) 
{
        size_t len = 0;
        size_t i = 0;
        char *p = buffer->head;

        while (source[len++] != 0);
        if (len > 30 - buffer->char_count)
                return false;
        for (i = 0; i < len; i++) {
                if (&p[i] == &buffer->data[30])
                        p = &p[i] - (30 + i);
                p[i] = source[i];
        }
        buffer->char_count += len;
        buffer->string_count++;
        buffer->head = p + i;
        return true;
}

// (5) Napisz funkcję, która będzie wyciągać (odczytywać i usuwać) z bufora kolejne wpisane do niego łańcuchy tekstowe.
//     Odczytany łańcuch znaków kopiowany jest do 'destination'
//     Funkcja ma zwracać 'true' jeśli udało się wyciągnąć jakiś łańcuch lub 'false' w przeciwnym razie.
bool getText(CharRingBufferPtr buffer, char* destination) 
{
        if (buffer->string_count == 0)
                return false;

        memset(destination, 0, 30);
        size_t len = 0;
        char *p = buffer->tail;

        while (p[len] != 0) {
                if (&p[len] == &buffer->data[30])
                        p = &p[len] - (30 + len);
                destination[len] = p[len];
                p[len] = 0;
                len++;
        }
        buffer->char_count -= len;
        buffer->string_count--;
        buffer->tail = p + len + 1;
        return true;
}

// (6) Napisz funkcję, która zwróci liczbę łańcuchów tekstowych zapisanych w buforze kołowym, bez wprowadzania zmian w samym buforze.
int getCount(CharRingBufferPtr buffer) {
// TODO (6):
    return buffer->string_count;
}


int main()
{
    printf("Testujemy...\n");

    struct CharRingBuffer buffer;
    char text[100];
       
    // najpierw inicjalizacja
    init(&buffer);
    // na początku bufor powinien być pusty
    assert(0 == getCount(&buffer));
    assert(false == getText(&buffer, text));

    // dodajemy 2 łańcuchy
    assert(true == putText(&buffer, "hello world"));
    assert(true == putText(&buffer, "good morning"));
    assert(2 == getCount(&buffer));
    // wyciągamy pierwszy
    assert(true == getText(&buffer, text));
    assert(0 == strcmp(text, "hello world"));
    assert(1 == getCount(&buffer));
    // wyciągamy drugi
    assert(true == getText(&buffer, text));
    assert(0 == strcmp(text, "good morning"));
    // teraz znów bufor powinien być pusty
    assert(0 == getCount(&buffer));
    assert(false == getText(&buffer, text));

    // wkładamy 3 łańcuchy po 8 znaków
    assert(true == putText(&buffer, "12345678"));
    assert(true == putText(&buffer, "abcdefgh"));
    assert(true == putText(&buffer, "ijklmnop"));
    // czwarty nie wejdzie...
    assert(false == putText(&buffer, "qrstuvwx"));
    // ..dopóki nie zrobimy miejsca:
    assert(true == getText(&buffer, text));
    assert(0 == strcmp(text, "12345678"));
    assert(true == putText(&buffer, "qrstuvwx"));

    // wyciągamy po kolei 3 łańcuchy
    assert(true == getText(&buffer, text));
    assert(0 == strcmp(text, "abcdefgh"));
    assert(true == getText(&buffer, text));
    assert(0 == strcmp(text, "ijklmnop"));
    assert(true == getText(&buffer, text));
    assert(0 == strcmp(text, "qrstuvwx"));

    // teraz znów bufor powinien być pusty
    assert(0 == getCount(&buffer));
    assert(false == getText(&buffer, text));

    printf("Koniec testu\n");
    return 0;
}