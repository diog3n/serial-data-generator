// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

#include "utils.h"

int main(int argc, char** argv) {
    char *filename = argv[1];

    char *wr_str = argv[2];

    if (filename == NULL) {
        printf("No filename given, trying /dev/ttyUSB0...\n");
        filename = "/dev/ttyUSB0";
    }

    if (wr_str == NULL) {
        printf("No test string given, trying \"Hello World\\n\"\n");
        wr_str = "Hello world\n";
    }

    printf("Working with file \"%s\"\n", filename);

    int tty_desc = open("/dev/ttyUSB0", O_WRONLY);
    
    if (tty_desc < 0) {
        printf("File descriptor incorrect. Error code: %d\n", tty_desc);
        return tty_desc;
    }
    
    if (isatty(tty_desc) == 0) {
        printf("Device is not a tty. Aborting...\n");
        close(tty_desc);
        return 0;
    }

    long counter = 100000L;

    while (--counter != 0) {
        size_t wr_size = strlen(wr_str);
        
        if (write(tty_desc, wr_str, wr_size) < 0) {
            printf("Error while writing to tty %s", filename);
        }

        char wr_str_clean[strlen(wr_str) + 1];
        
        printf("WRITTEN: %s (%ld bytes)\n", 
               remove_unprintable_chars(wr_str_clean, wr_str), 
               wr_size);
        delay(1000);
    }

    close(tty_desc);
    return 0;
}