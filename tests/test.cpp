#include <fcntl.h>      // file constants
#include <stdio.h>      // remove()
#include <sys/mman.h>   // mmap()
#include <sys/stat.h>   // path stat and constants
#include <sys/types.h>  // unix types
#include <unistd.h>
#include <unistd.h>  // open(), read(), write(), usleep()
#include <cstring>   // strncpy()
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>  // std::exception
#include <string>     // std::string
#include <vector>
#include "../include/fat.h"

int main() {
    std::ostringstream oss;
    char *buff = nullptr;
    int bytes = 0;
    fs::FileEntry fentry;
    std::string dirname, filename, path, output, data;
    std::string name = "testfile";
    fs::Disk disk(name, 10, 10);

    std::cout << "\nCreating disk and filesystem" << std::endl;

    if(!disk.create()) {
        disk.open(name);
    }

    fs::FatFS fatfs;
    fatfs.set_disk(&disk);

    if(!fatfs.open_disk()) fatfs.format();

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    dirname = "dir1";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_dir(dirname);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    dirname = "dir2";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_dir(dirname);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    filename = "file1";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_file(filename);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    dirname = "dir1/dir2";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_dir(dirname);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    filename = "dir1/dir2/file3";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fentry = fatfs.add_file(filename);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    data =
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz1";
    fatfs.write_file_data(fentry, data.c_str(), data.size());

    buff = new char[fentry.data_size() + 1];
    fatfs.read_file_data(fentry, buff, data.size());
    std::cout << buff << std::endl;

    delete[] buff;

    data =
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz1";
    fatfs.write_file_data(fentry, data.c_str(), data.size());

    buff = new char[fentry.data_size() + 1];
    fatfs.read_file_data(fentry, buff, data.size());
    std::cout << buff << std::endl;

    delete[] buff;

    data =
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz1";
    fatfs.write_file_data(fentry, data.c_str(), data.size());

    buff = new char[fentry.data_size() + 1];
    fatfs.read_file_data(fentry, buff, data.size());
    std::cout << buff << std::endl;

    delete[] buff;

    path = "/";
    std::cout << "\nChanging directory with path: " << path << std::endl;
    fatfs.change_dir(path);

    fatfs.print_all(oss, ".", true);
    std::cout << oss.str() << std::endl;

    fatfs.remove();

    return 0;
}
