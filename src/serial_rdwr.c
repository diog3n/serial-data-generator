// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include "utils.h"

int main(void) {
    int tty_desc = open("/dev/ttyUSB0", O_RDWR);
    
    if (tty_desc < 0) {
        printf("File descriptor incorrect. Error code: %d\n", tty_desc);
        return tty_desc;
    }
    
    if (isatty(tty_desc) == 0) {
        printf("Device is not a tty. Aborting...\n");
        close(tty_desc);
        return 0;
    }

    long counter = 1000L;

    while (--counter != 0) {
        size_t wr_size = 12;
        char* wr_buffer = "Hello world\n";
        write(tty_desc, wr_buffer, wr_size);
        delay(1000);
    }

    close(tty_desc);
    return 0;
}