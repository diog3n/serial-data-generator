// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

int main(void) {
    int tty_desc = open("/dev/ttyUSB0", O_RDWR);
    
    if (tty_desc < 0) {
        printf("File descriptor incorrect. Error code: %d\n", tty_desc);
        return tty_desc;
    }
    
    close(tty_desc);
}