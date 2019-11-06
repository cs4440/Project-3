/*
 * Shell connects a chain of commands via pipe() and dup2() to connect pipes of
 * different commands.
 *
 * DETAILS
 * ------
 * Shell::run() will exect the shell program
 * Shell will parse a line of user inputs into argument commands and operators
 * It can chain operator | with multiple processes.
 */
#include "../include/shell.h"  // Shell class

int main() {
    Shell shell;

    shell.run();

    return 0;
}
