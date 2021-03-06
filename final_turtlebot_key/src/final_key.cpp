
#include <cstdio>
#include <iostream>
#include <limits>
#include <string>

#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <signal.h>


#include "final_turtlebot_key/final_key.h"

// This code is based on 
//   https://cboard.cprogramming.com/linux-programming/158476-termios-examples.html
// and 
//   http://www.cplusplus.com/forum/general/5304/#msg23940

// Usage:
//
//   ....
//   terminal_init();
//   ....
//   set_terminal_raw_mode();
//   ....
//   bool isContinue = true;
//   while(isContinue) {
//     ....    
//     while(true) {
//       if (wait_for_key_pressed(100)) break;
//       ....
//     }
//     ....
//     char c = getKeyStroke();
//     switch(c) {
//       ....
//       case 'q':
//         isContinue = false;
//         break;
//       ....
//     }
//   }
//   ....
//   set_terminal_original_mode();



/* Restore terminal to original settings */



void terminal_done() {
  if (terminal_descriptor != -1)
    tcsetattr(terminal_descriptor, TCSANOW, &terminal_original);
}
 

/* "Default" signal handler: restore terminal, then exit.  */
void terminal_signal(int signum) {
  cout << "Terminal_signal(" << signum << ")" << endl;
  if (terminal_descriptor != -1)
    tcsetattr(terminal_descriptor, TCSANOW, &terminal_original);
  /* exit() is not async-signal safe, but _exit() is.
   * Use the common idiom of 128 + signal number for signal exits.
   * Alternative approach is to reset the signal to default handler,
   * and immediately raise() it. */
  _exit(128 + signum);
}
 

/* 
 * Initialize terminal for non-canonical, non-echo mode,
 * that should be compatible with standard C I/O.
 * Returns 0 if success, nonzero errno otherwise.
*/
int terminal_init() {

  /* Already initialized? */
  if (terminal_descriptor != -1)
    return errno = 0;

  /* Which standard stream is connected to our TTY? */
  if (isatty(STDERR_FILENO))
    terminal_descriptor = STDERR_FILENO;
  else
  if (isatty(STDIN_FILENO))
    terminal_descriptor = STDIN_FILENO;
  else
  if (isatty(STDOUT_FILENO))
    terminal_descriptor = STDOUT_FILENO;
  else
    return errno = ENOTTY;

  /* Obtain terminal settings. */
  if (tcgetattr(terminal_descriptor, &terminal_original) ||
      tcgetattr(terminal_descriptor, &terminal_settings))
  {
    return errno = ENOTSUP;
  }

  // Disable buffering for terminal streams.
  if (isatty(STDIN_FILENO))
    setvbuf(stdin, NULL, _IONBF, 0);
  if (isatty(STDOUT_FILENO))
    setvbuf(stdout, NULL, _IONBF, 0);
  if (isatty(STDERR_FILENO))
    setvbuf(stderr, NULL, _IONBF, 0);

  /* At exit() or return from main(),
   * restore the original settings. */
  if (atexit(terminal_done))
    return errno = ENOTSUP;

  /* Set new "default" handlers for typical signals,
   * so that if this process is killed by a signal,
   * the terminal settings will still be restored first. */
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_handler = terminal_signal;
  act.sa_flags = 0;
  if (sigaction(SIGHUP,  &act, NULL) ||
      sigaction(SIGINT,  &act, NULL) ||
      sigaction(SIGQUIT, &act, NULL) ||
      sigaction(SIGTERM, &act, NULL) ||
#ifdef SIGXCPU
      sigaction(SIGXCPU, &act, NULL) ||
#endif
#ifdef SIGXFSZ    
      sigaction(SIGXFSZ, &act, NULL) ||
#endif
#ifdef SIGIO
      sigaction(SIGIO,   &act, NULL) ||
#endif
      sigaction(SIGPIPE, &act, NULL) ||
      sigaction(SIGALRM, &act, NULL))
  {
    return errno = ENOTSUP;
  }

  /* Let BREAK cause a SIGINT in input. */
  terminal_settings.c_iflag &= ~IGNBRK;
  terminal_settings.c_iflag |=  BRKINT;
 
  /* Ignore framing and parity errors in input. */
  terminal_settings.c_iflag |=  IGNPAR;
  terminal_settings.c_iflag &= ~PARMRK;
 
  /* Do not strip eighth bit on input. */
  terminal_settings.c_iflag &= ~ISTRIP;
 
  /* Do not do newline translation on input. */
  // terminal_settings.c_iflag &= ~(INLCR | IGNCR | ICRNL);
 
#ifdef IUCLC
  /* Do not do uppercase-to-lowercase mapping on input. */
  terminal_settings.c_iflag &= ~IUCLC;
#endif
 
  /* Use 8-bit characters. This too may affect standard streams,
   * but any sane C library can deal with 8-bit characters. */
  terminal_settings.c_cflag &= ~CSIZE;
  terminal_settings.c_cflag |=  CS8;

  /* Enable receiver. */
  terminal_settings.c_cflag |=  CREAD;

  /* Let INTR/QUIT/SUSP/DSUSP generate the corresponding signals. */
  terminal_settings.c_lflag |=  ISIG;

  /* Enable noncanonical mode.
   * This is the most important bit, as it disables line buffering etc. */
  terminal_settings.c_lflag &= ~ICANON;

  /* Disable echoing input characters. */
  terminal_settings.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

  /* Disable implementation-defined input processing. */
  terminal_settings.c_lflag &= ~IEXTEN;

  /* To maintain best compatibility with normal behaviour of terminals,
   * we set TIME=0 and MAX=1 in noncanonical mode. This means that
   * read() will block until at least one byte is available. */
  terminal_settings.c_cc[VTIME] = 0;
  terminal_settings.c_cc[VMIN] = 1;

  /* Done. */
  return errno = 0;
}


void set_terminal_raw_mode() {

  /* Set the new terminal settings.
   * Note that we don't actually check which ones were successfully
   * set and which not, because there isn't much we can do about it. */
  if (terminal_descriptor != -1)
    tcsetattr(terminal_descriptor, TCSANOW, &terminal_settings);
}


void set_terminal_original_mode() {
  /* Restore original terminal settings. */
  if (terminal_descriptor != -1)
    tcsetattr(terminal_descriptor, TCSANOW, &terminal_original);
}
 

bool wait_for_key_pressed(unsigned timeout_ms /* = 0 */) {
  if (terminal_descriptor == -1) return false;

  struct pollfd pls[ 1 ];
  pls[ 0 ].fd     = terminal_descriptor;
  pls[ 0 ].events = POLLIN | POLLPRI;

  return poll( pls, 1, timeout_ms ) > 0;
}



