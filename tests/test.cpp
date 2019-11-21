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

    dirname = "Dir1 @ root";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_dir(dirname);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    dirname = "Dir2 @ root";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_dir(dirname);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    dirname = "Dir3 @ root";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_dir(dirname);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    filename = "File1 @ root";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_file(filename);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    filename = "File2 @ root";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_file(filename);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    filename = "File2 @ root";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_file(filename);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    fentry = fatfs.find_file(filename);
    std::cout << "\nWriting data to: " << filename << std::endl;
    data = "Hello";
    std::cout << "Data size: " << data.size() << std::endl;
    bytes = fatfs.write_file_data(fentry, data.c_str(), data.size());

    std::cout << "\nReading data from: " << filename << std::endl;
    buff = new char[fentry.size() + 1];
    bytes = fatfs.read_file_data(fentry, buff, fentry.size());
    buff[bytes] = '\0';
    std::cout << buff << std::endl;

    std::cout << "Bytes read: " << bytes << std::endl;
    std::cout << "File size: " << fentry.size() << std::endl;

    delete[] buff;

    fentry = fatfs.find_file(filename);
    std::cout << "\nWriting data to: " << filename << std::endl;
    data =
        "SOmasdfliajsdofjlaekjlajoidjfglidsjglj3242354o9ert9udfgidgkldjrflgijl0"
        "fsfjlsertjwle4jte90dgudgiehtke4jto9923u434534546476567lkjljdrtzzzzzz"
        "SOmasdfliajsdofjlaekjlajoidjfglidsjglj3242354o9ert9udfgidgkldjrflgijl0"
        "fsfjlsertjwle4jte90dgudgiehtke4jto9923u434534546476567lkjljdrtzzzzzz"
        "SOmasdfliajsdofjlaekjlajoidjfglidsjglj3242354o9ert9udfgidgkldjrflgijl0"
        "fsfjlsertjwle4jte90dgudgiehtke4jto9923u434534546476567lkjljdrtzzzzzz"
        "SOmasdfliajsdofjlaekjlajoidjfglidsjglj3242354o9ert9udfgidgkldjrflgijl0"
        "fsfjlsertjwle4jte90dgudgiehtke4jto9923u434534546476567lkjljdrtzzzzzz"
        "SOmasdfliajsdofjlaekjlajoidjfglidsjglj3242354o9ert9udfgidgkldjrflgijl0"
        "fsfjlsertjwle4jte90dgudgiehtke4jto9923u434534546476567lkjljdrtzzzzzz"
        "SOmasdfliajsdofjlaekjlajoidjfglidsjglj3242354o9ert9udfgidgkldjrflgijl0"
        "fsfjlsertjwle4jte90dgudgiehtke4jto9923u434534546476567lkjljdrtzzzzzz"
        "SOmasdfliajsdofjlaekjlajoidjfglidsjglj3242354o9ert9udfgidgkldjrflgijl0"
        "fsfjlsertjwle4jte90dgudgiehtke4jto9923u434534546476567lkjljdrtzzzzzz"
        "SOmasdfliajsdofjlaekjlajoidjfglidsjglj3242354o9ert9udfgidgkldjrflgijl0"
        "fsfjlsertjwle4jte90dgudgiehtke4jto9923u434534546476567lkjljdrtzzzzz "
        "end of file";
    std::cout << "Data size: " << data.size() << std::endl;
    bytes = fatfs.write_file_data(fentry, data.c_str(), data.size());

    std::cout << "\nReading data from: " << filename << std::endl;
    buff = new char[fentry.size() + 1];
    bytes = fatfs.read_file_data(fentry, buff, fentry.size());
    buff[bytes] = '\0';
    std::cout << buff << std::endl;

    std::cout << "Bytes read: " << bytes << std::endl;
    std::cout << "File size: " << fentry.size() << std::endl;

    delete[] buff;

    fentry = fatfs.find_file(filename);
    std::cout << "\nWriting data to: " << filename << std::endl;
    data = "Hello";
    std::cout << "Data size: " << data.size() << std::endl;
    bytes = fatfs.write_file_data(fentry, data.c_str(), data.size());

    std::cout << "Bytes written: " << bytes << std::endl;
    std::cout << "File size: " << fentry.size() << std::endl;

    std::cout << "\nReading data from: " << filename << std::endl;
    buff = new char[fentry.size() + 1];
    bytes = fatfs.read_file_data(fentry, buff, fentry.size());
    buff[bytes] = '\0';
    std::cout << buff << std::endl;

    std::cout << "Bytes read: " << bytes << std::endl;
    std::cout << "File size: " << fentry.size() << std::endl;

    delete[] buff;

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    path = "/Dir1 @ root";
    std::cout << "\nChanging directory with path: " << path << std::endl;
    fatfs.change_dir(path);

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    dirname = "Nested 1 Dir1";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_dir(dirname);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    path = "Nested 1 Dir1";
    std::cout << "\nChanging directory with path: " << path << std::endl;
    fatfs.change_dir(path);

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    dirname = "Nested 2 Dir1";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_dir(dirname);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    path = "/Dir1 @ root/Nested 1 Dir1/Nested 2 Dir1";
    std::cout << "\nChanging directory with path: " << path << std::endl;
    fatfs.change_dir(path);

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    path = "/";
    std::cout << "\nChanging directory with path: " << path << std::endl;
    fatfs.change_dir(path);

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    dirname = "Dir1 @ root/Nested 1 Dir1/Nested 2 Dir1/Nested 3 Dir1";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    try {
        fatfs.add_dir(dirname);
    } catch(const std::exception &e) {
        std::cout << "Cannot add: " << e.what() << std::endl;
    }

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    path = "Dir1 @ root/Nested 1 Dir1/Nested 2 Dir1/";
    std::cout << "\nChanging directory with path: " << path << std::endl;
    fatfs.change_dir(path);

    std::cout << "\nPrinting fatfs info:" << std::endl;
    std::cout << "----------------------" << std::endl;
    std::cout << fatfs.info() << std::endl;

    std::cout << "\nListing at: " << fatfs.current().name() << std::endl;
    std::cout << "------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();

    // fatfs.change_dir("/path1/path2/");
    // fatfs.change_dir("/path1");
    // fatfs.change_dir("/path1/");
    // fatfs.change_dir("path1/path2");

    // fatfs.change_dir("path1/path2/");

    fatfs.remove();

    return 0;
}
