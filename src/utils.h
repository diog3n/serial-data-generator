#pragma once
#include <time.h>    // for delay function
#include <string.h>

void delay(const int seconds);

char * remove_unprintable_chars(char destination[], const char source[]);