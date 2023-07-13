// C library headers
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

void print_entry(long entry_number, char c) {
    printf("%ld\t\t%c\t\t%d\t\t%02x\n", 
           entry_number, 
           c < 32 || c > 126 ? '%' : c, 
           (int) c, 
           (int) c);
}

void print_error_entry() {
    printf("\t\t***Read error***\t\t\n");
}

void print_no_data_entry() {
    printf("***Timeout reached. Connection lost...***\n");
}

int main(int argc, char** argv) {
    const long MAX_ITERATIONS = 10000L;

    char *filename = argv[1];

    char *option = argv[2];

    int endless_flag = 0;

    if (option == NULL) {
        option = "--limit";
        endless_flag = 0;
    } else if (strcmp(option, "--endless") == 0) {
        printf("Endless reading mode\n");
        endless_flag = 1;
    } else if (strcmp(option, "--limit") == 0) {
        printf("Reading loop is limited\n");
        endless_flag = 0;
    } else {
        printf("Unknown option: %s\n", option);
    }

    if (filename == NULL) {
        printf("No filename given, trying /dev/ttyUSB0...");
        filename = "/dev/ttyUSB0";
    }

    printf("Working with file \"%s\"\n", filename);

    int tty_desc = open(filename, O_RDONLY);
    
    if (tty_desc < 0) {
        printf("File descriptor incorrect. Error code: %d\n", tty_desc);
        return tty_desc;
    }
    
    if (isatty(tty_desc) == 0) {
        printf("Device is not a tty. Aborting...\n");
        close(tty_desc);
        return 0;
    }

    long iteration = 0L;
    long dead_ticks = 5000;
    long ticks = 0L;
    printf("DEAD TICKS: %ld\n", dead_ticks);
    printf("Iteration\tCharacter\tCode\t\tCode (HEX)\n");

    while (++iteration != MAX_ITERATIONS || endless_flag == 1) {
        ssize_t size = 1;
        char buffered_char;
        int read_result = read(tty_desc, &buffered_char, size);

        if (read_result < 0) {
            ticks++;
            print_error_entry();
            continue;
        } else if (buffered_char == '\0') {
            ticks++;
            if (ticks >= dead_ticks) {
                print_no_data_entry();
                ticks = 0L;
            }
            continue;
        } else {
            print_entry(iteration, buffered_char);
            ticks = 0L;
        }
    }

    close(tty_desc);
    return 0;
}