#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define TRUE (1)
#define FALSE (0)

#define RETVAL_SUCCESS  (0)
#define RETVAL_FAIL     (-1)

struct termios orig_termios;

void die(const char * const s)
{
    perror(s);
    exit(1);
}

void disableRawMode()
{
    int error_code = 0;
    error_code = tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    if (-1 == error_code)
    {
        die("tcsetattr failed");
    }
}

int enableRawMode()
{
    int retval = RETVAL_SUCCESS;
    retval = tcgetattr(STDIN_FILENO, &orig_termios);
    if (RETVAL_FAIL == retval)
    {
        die("tcgetattr failed");
    }

    atexit(disableRawMode);
    
    struct termios raw_mode = orig_termios;
    raw_mode.c_iflag &=  ~( BRKINT    // disable break condition (should not affect modern systems)
                    | ICRNL     // treat \r as \r (disables the terminal from automatically adding \n to \r)
                    | INPCK     // enable parity checking (doesn't seem to apply on modern machines)
                    | ISTRIP    // strip 8th bit of input (active by default on normal modern machines)
                    | IXON      // DISABLE CTRL+S and CTRL+Q
                    );

    raw_mode.c_iflag &= ~(IXON      // disable CTRL+S, CTRL+Q
                        | ICRNL     // disable terminal from automatically adding \n to \r character
                        );

    // this should be bitwise OR (and not bitwise AND)
    raw_mode.c_iflag |= (CS8);      // Set character size to 8 bits

    raw_mode.c_iflag &= ~(OPOST);   // disable replacing \n with \r\n automatically by the terminal
    raw_mode.c_lflag &= ~(ECHO      // disable echoing
                        | ICANON    // disable canonical mode
                        | ISIG      // disable CTRL+Z, CTRL+C
                        | IEXTEN    // disable CTRL+V+char and CTRL+0 on macOS
                        );

    raw_mode.c_cc[VMIN] = 0;    // the minimum amount of bytes to read before returning from read func.
    raw_mode.c_cc[VTIME] = 0;   // wait the lowest amount of time possible before returning from read func.
    
    
    retval = tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_mode);
    if (RETVAL_FAIL == retval)
    {
        die("tcsetattr failed");
    }
    return retval;
}

int main(void)
{
    int retval = RETVAL_SUCCESS;
    enableRawMode();
    char c = '\0';
    while (TRUE)
    {
        retval = read(STDIN_FILENO, &c, 1);
        if (RETVAL_FAIL == retval || EAGAIN == errno)
        {
            die("read failed");
        }

        if (TRUE == iscntrl(c))
        {
            printf("%d\r\n", c);
        }
        else
        {
            printf("%d ('%c')\r\n", c, c);
        }
        if ('q' == c)
        {
            break;
        }
    }
    return 0;
}
