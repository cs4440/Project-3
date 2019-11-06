#include "../include/shell.h"

// start the shell process
void Shell::run() {
    bool more_lines = false;
    std::string cmd, line;
    std::vector<Token> tokens;
    std::queue<std::vector<std::string>> args;
    std::queue<std::string> ops;

    while(cmd != "exit") {
        std::cout << "# ";
        std::getline(std::cin, line);

        // run parser on line
        _parser.clear();
        _parser.set_string(line.c_str());
        more_lines = _parser.parse();

        // parser will return more_lines = true if line ends in '\'
        while(more_lines) {
            std::cout << "> ";
            std::getline(std::cin, line);
            _parser.set_string(line.c_str());
            more_lines = _parser.parse();
        }

        tokens = _parser.get_tokens();

        if(!tokens.empty() && (cmd = tokens[0].string()) != "exit") {
            // split tokens into args and ops
            _parse_cmds_and_ops(tokens, args, ops);

            try {
                _run_cmds(args, ops);
            } catch(const std::exception &e) {
                std::cerr << e.what() << std::endl;
                break;
            }

            // clear args and ops if error occurs
            while(!args.empty()) args.pop();
            while(!ops.empty()) ops.pop();
        }
    }
}

/*
 * convert a vector of strings into commands and operators
 * pre-condition: non-empty vector of strings are passed in
 * post-condition: each command (w/ their arguments) is pushed in qvstr
 *                 queue of operators are populated if there are ops
 *                 tokens is unchanged
 * @param tokens - input, vector of Token strings
 * @param qvstr - ret ref, queue of vector of strings
 * @param qops - ret ref, queue of operators
 *
 * Example:
 * ls -al | cat | wc -l
 *
 * qvstr[0]: ls, -al
 * qvstr[1]: cat
 * qvstr[2]: wc, -l
 *
 * qops[0]: |
 * qops[1]: |
 */
void Shell::_parse_cmds_and_ops(const std::vector<Token> &tokens,
                                std::queue<std::vector<std::string>> &qvstr,
                                std::queue<std::string> &qops) {
    std::vector<std::string> args;

    for(std::size_t i = 0; i < tokens.size(); ++i) {
        if(tokens[i].type() == state_machine::STATE_OP) {
            qvstr.push(args);
            qops.push(tokens[i].string());
            args.clear();
        } else
            args.push_back(tokens[i].string());
    }

    if(!args.empty()) qvstr.push(args);
}

// run shell commands with fork(), pipe(), exec()
// @param args - queue of vector of strings
// @param ops - queue of operators
//
// DETAILS
// parent will fork n number children commands
// children will use dup2 to connect previous file descriptors to current
//      file descriptors
void Shell::_run_cmds(std::queue<std::vector<std::string>> &args,
                      std::queue<std::string> &ops) {
    std::string cur_op = "", prev_op = "";
    char **cargs = nullptr;
    int *fd = nullptr, *cur_fd = nullptr, *prev_fd = nullptr;
    int pipe_status, wait_status = 0;
    pid_t pid, wait_pid;

    // create file descriptors for n operators
    cur_fd = fd = new int[2 * ops.size()];
    for(std::size_t i = 0; i < ops.size(); ++i) {
        pipe_status = pipe(fd + 2 * i);
        if(pipe_status < 0) {
            delete[] fd;
            throw std::runtime_error("Pipe failed");
        }
    }

    while(!args.empty()) {
        // get char** from vector of strings for exec()
        cargs = _vec_str_to_char_args(args.front());

        if(ops.size()) cur_op = ops.front();  // get current op

        pid = fork();

        if(pid < 0) {
            delete[] fd;
            _deallocate_all(cargs, args.front().size());

            throw std::runtime_error("Fork failed");
        } else if(pid == 0) {
            if(!prev_op.empty()) {
                if(prev_op == "|") {
                    dup2(prev_fd[0], STDIN_FILENO);  // stdout to pipe write
                    close(prev_fd[0]);               // close pipe read
                    close(prev_fd[1]);               // close pipe write
                }
            }

            if(!cur_op.empty()) {
                if(cur_op == "|") {
                    dup2(cur_fd[1], STDOUT_FILENO);  // stdout to pipe write
                    close(cur_fd[0]);                // close pipe read
                    close(cur_fd[1]);                // close pipe write
                }
            }
            execvp(cargs[0], cargs);

            delete[] fd;
            _deallocate_all(cargs, args.front().size());

            throw std::runtime_error("Command not found");
        } else {
            if(prev_fd) {
                close(prev_fd[0]);
                close(prev_fd[1]);
            }

            prev_fd = cur_fd;  // store previous fd
            prev_op = cur_op;  // store previous op
            cur_fd += 2;       // get next file descriptors
            cur_op.clear();    // clear current op

            _deallocate_all(cargs, args.front().size());

            if(ops.size()) ops.pop();  // remove an op
            args.pop();                // remove an arg
        }
    }
    delete[] fd;

    // continue to wait for all children to terminate
    while((wait_pid = wait(&wait_status)) > 0) {
    }
}

// delete entire char** and its child elements
// @param arr - array of cstrings
// @param size - size of array
void Shell::_deallocate_all(char **arr, std::size_t size) {
    for(std::size_t i = 0; i < size; ++i) delete[] arr[i];
    delete[] arr;
}

// convert a vector of strings to char**
// return char** - arra of cstrings
// NOTE - caller must manually handle deallocations!
char **Shell::_vec_str_to_char_args(std::vector<std::string> &vstr) {
    char *cstr = nullptr;
    char **cargs = new char *[vstr.size() + 1];

    for(std::size_t i = 0; i < vstr.size(); ++i) {
        cstr = new char[vstr[i].size() + 1];
        std::copy(vstr[i].begin(), vstr[i].end(), cstr);
        cstr[vstr[i].size()] = '\0';
        cargs[i] = cstr;
    }
    cargs[vstr.size()] = nullptr;

    return cargs;
}
