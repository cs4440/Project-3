#include <unistd.h>
#include <iostream>
#include <map>
#include <string>
#include "../include/filesystem.h"

int main() {
    bool added = false;
    bool removed = false;
    std::string dir_name, file_name, path;

    fs::FileSystem fs;
    fs.init();

    std::cout << "root node: " << fs.root()->name() << std::endl;
    std::cout << "current node: " << fs.current()->name() << std::endl;

    dir_name = "dir1";
    std::cout << "\nadding dir: " << dir_name << std::endl;
    fs.add_dir(dir_name);
    dir_name = "dir2";
    std::cout << "adding dir: " << dir_name << std::endl;
    fs.add_dir(dir_name);
    dir_name = "dir3";
    std::cout << "adding dir: " << dir_name << std::endl;
    fs.add_dir(dir_name);

    file_name = "file1.txt";
    std::cout << "\nadding file: " << file_name << std::endl;
    fs.add_file(file_name);
    file_name = "file2.txt";
    std::cout << "adding file: " << file_name << std::endl;
    fs.add_file(file_name);
    file_name = "file3.txt";
    std::cout << "adding file: " << file_name << std::endl;
    fs.add_file(file_name);

    std::cout << "\nprinting current directories at node "
              << fs.current()->name() << std::endl;
    auto dirs = fs.dirs();
    for(auto it = dirs.begin(); it != dirs.end(); ++it) {
        std::cout << it->first << std::endl;
    }

    std::cout << "\nprinting current files at node " << fs.current()->name()
              << std::endl;
    auto files = fs.files();
    for(auto it = files.begin(); it != files.end(); ++it) {
        std::cout << it->first << std::endl;
    }

    path = "dir1";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;

    path = "..";
    std::cout << "\nchanging to directory: " << path << std::endl;
    fs.change_dir(path);
    std::cout << "current node: " << fs.current()->name() << std::endl;

    path = "dir4";
    std::cout << "\nchanging to invalid path: " << path << std::endl;
    if(!fs.change_dir(path))
        std::cout << "changing path failed at: " << path << std::endl;
    std::cout << "current node: " << fs.current()->name() << std::endl;

    std::cout << "\nrinting created, last_accessed, and last_created at root"
              << std::endl;
    std::cout << "created: " << fs.root()->created_cstr();
    std::cout << "last_accessed: " << fs.root()->last_accessed_cstr();
    std::cout << "last_updated: " << fs.root()->last_updated_cstr();

    return 0;
}
