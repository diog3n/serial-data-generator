#include "utils.h"

void delay(const int seconds) {
    long clock_rate = CLOCKS_PER_SEC;

    for (int i = 0; i < clock_rate * seconds; i++);
}

char * remove_unprintable_chars(char destination[], const char source[]) {
    char *tmp = destination;
    int index = 0;
    while (1) {
        if (source[index] == '\000') {
            destination[index] = source[index];
            return tmp;
        }

        destination[index] = source[index] < 32 || source[index] > 127 
                             ? '%' 
                             : source[index];
        
        index++;
    }
    return tmp;
}