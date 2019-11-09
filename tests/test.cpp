#include <unistd.h>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include "../include/filesystem.h"

int main() {
    std::string dir_name, file_name, path;

    fs::FileSystem fs;
    fs.init();

    std::cout << "root node: " << fs.root()->name() << std::endl;
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    dir_name = "dir1";
    std::cout << "\nadding dir: " << dir_name << std::endl;
    fs.add_dir(dir_name);
    dir_name = "dir2";
    std::cout << "adding dir: " << dir_name << std::endl;
    fs.add_dir(dir_name);
    dir_name = "dir3";
    std::cout << "adding dir: " << dir_name << std::endl;
    fs.add_dir(dir_name);
    try {
        dir_name = "dir3";
        std::cout << "adding dir: " << dir_name << std::endl;
        fs.add_dir(dir_name);
    } catch(const std::exception &e) {
        std::cout << "adding dir: " << dir_name << " failed b/c: " << e.what()
                  << std::endl;
    }
    file_name = "file1.txt";
    std::cout << "\nadding file: " << file_name << std::endl;
    fs.add_file(file_name);
    file_name = "file2.txt";
    std::cout << "adding file: " << file_name << std::endl;
    fs.add_file(file_name);
    file_name = "file3.txt";
    std::cout << "adding file: " << file_name << std::endl;
    fs.add_file(file_name);

    std::cout << "\nprinting entries at cwd: " << fs << std::endl;
    fs.print_entries();

    path = "dir1";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    dir_name = "dir10";
    std::cout << "\nadding dir: " << dir_name << ", at node "
              << fs.current()->name() << std::endl;
    fs.add_dir(dir_name);

    std::cout << "\nprinting entries at cwd: " << fs << std::endl;
    fs.print_entries();

    path = "dir10";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    dir_name = "dir100";
    std::cout << "\nadding dir: " << dir_name << ", at node "
              << fs.current()->name() << std::endl;
    fs.add_dir(dir_name);

    path = "dir100";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    path = "..";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    path = "..";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    path = "dir10/dir100";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    path = "/";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    path = "dir1/dir10/";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    path = "dir4";
    std::cout << "\nchanging to invalid path: " << path << std::endl;
    if(!fs.change_dir(path))
        std::cout << "changing path failed at: " << path << std::endl;
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    path = "/dir1/dir10/dir100";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    path = ".";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    path = "../..";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    path = "../dir1/dir10/../../dir2";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;
    std::cout << "cwd: " << fs << std::endl;

    fs.remove_dirs();

    std::cout << "\nprinting timestamps " << std::endl;
    std::cout << "created: " << fs.root()->created_cstr();
    std::cout << "last_accessed: " << fs.root()->last_accessed_cstr();
    std::cout << "last_updated: " << fs.root()->last_updated_cstr();

    return 0;
}
