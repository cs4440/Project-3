/*******************************************************************************
 * CLASS       : CS4440
 * HEADER      : shell
 * DESCRIPTION : Defines a unix-like shell.
 ******************************************************************************/
#ifndef SHELL_H
#define SHELL_H

#include <sys/types.h>                 // pid_t
#include <sys/wait.h>                  // wait()
#include <unistd.h>                    // exec(), fork()
#include <algorithm>                   // copy
#include <cstdlib>                     // exit()
#include <iostream>                    // cin
#include <queue>                       // queue
#include <stdexcept>                   // exceptions
#include <string>                      // getline()
#include <vector>                      // vector
#include "../include/parser.h"         // Parser, get cli tokens with grammar
#include "../include/state_machine.h"  // Token's states
#include "../include/token.h"          // Token class

class Shell {
public:
    Shell() {}

    void run();

private:
    Parser _parser;

    void _parse_cmds_and_ops(const std::vector<Token> &tokens,
                             std::queue<std::vector<std::string>> &vvstr,
                             std::queue<std::string> &qops);
    void _run_cmds(std::queue<std::vector<std::string>> &args,
                   std::queue<std::string> &ops);

    void _deallocate_all(char **arr, std::size_t size);

    char **_vec_str_to_char_args(std::vector<std::string> &v_str);
};

#endif  // SHELL_H
