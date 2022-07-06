

#ifndef UNIST_TB3_KB_POSIX_TERMINAL_HPP
#define UNIST_TB3_KB_POSIX_TERMINAL_HPP

#include <termios.h>

// This code is based on 
//   https://cboard.cprogramming.com/linux-programming/158476-termios-examples.html
// and 
//   http://www.cplusplus.com/forum/general/5304/#msg23940

#define ESC_KEY         -1
#define UP_ARROW_KEY    -2
#define DOWN_ARROW_KEY  -3
#define LEFT_ARROW_KEY  -4
#define RIGHT_ARROW_KEY -5


using namespace std;

static int terminal_descriptor = -1;
static struct termios terminal_original;
static struct termios terminal_settings;

static void terminal_done();
static void terminal_signal(int signum);

int terminal_init();
void set_terminal_raw_mode();
void set_terminal_original_mode();
bool wait_for_key_pressed(unsigned timeout_ms = 0);
char getKeyStroke();

#endif /* UNIST_TB3_KB_POSIX_TERMINAL_HPP */
