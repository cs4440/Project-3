#include <fcntl.h>      // file constants
#include <stdio.h>      // remove()
#include <sys/mman.h>   // mmap()
#include <sys/stat.h>   // path stat and constants
#include <sys/types.h>  // unix types
#include <unistd.h>
#include <unistd.h>  // open(), read(), write(), usleep()
#include <cstring>
#include <cstring>  // strncpy()
#include <iostream>
#include <map>
#include <stdexcept>  // std::exception
#include <string>
#include <string>  // std::string
#include <vector>
#include "../include/fat.h"

int main() {
    std::cout << "Creating file system\n" << std::endl;

    std::string dirname;
    std::string filename;
    int cylinders = 10;
    int sectors = 10;
    fs::FatFS fatfs("client", cylinders, sectors);

    std::cout << "Used space (root entry takes up 128 bytes): " << fatfs.size()
              << std::endl;
    std::cout << "Current directory: " << fatfs.current().name() << std::endl;
    std::cout << "Printing dirs/files" << std::endl;
    std::cout << "-------------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();
    std::cout << "-------------------" << std::endl;

    dirname = "Dir1 at root";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    fatfs.add_dir(dirname);

    dirname = "Dir2 at root";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    fatfs.add_dir(dirname);

    filename = "File1 at root";
    std::cout << "\nAdding directory: " << filename << std::endl;
    fatfs.add_file(filename);

    filename = "File2 at root";
    std::cout << "\nAdding directory: " << filename << std::endl;
    fatfs.add_file(filename);

    std::cout << "\nUsed space (root entry takes up 128 bytes): "
              << fatfs.size() << std::endl;
    std::cout << "Current directory: " << fatfs.current().name() << std::endl;
    std::cout << "Printing dirs/files" << std::endl;
    std::cout << "-------------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();
    std::cout << "-------------------" << std::endl;

    dirname = "Dir2 at root";
    std::cout << "\nCD into: " << dirname << std::endl;
    fatfs.change_dir(dirname);

    std::cout << "\nUsed space (root entry takes up 128 bytes): "
              << fatfs.size() << std::endl;
    std::cout << "Current directory: " << fatfs.current().name() << std::endl;
    std::cout << "Printing dirs/files" << std::endl;
    std::cout << "-------------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();
    std::cout << "-------------------" << std::endl;

    dirname = "Nested1 Dir1 at Dir1";
    filename = "Nested1 File1 at Dir1";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    fatfs.add_dir(dirname);
    std::cout << "Adding directory: " << filename << std::endl;
    fatfs.add_file(filename);

    dirname = "Nested1 Dir2 at Dir1";
    filename = "Nested1 File2 at Dir1";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    fatfs.add_dir(dirname);
    std::cout << "Adding directory: " << filename << std::endl;
    fatfs.add_file(filename);

    std::cout << "\nUsed space (root entry takes up 128 bytes): "
              << fatfs.size() << std::endl;
    std::cout << "Current directory: " << fatfs.current().name() << std::endl;
    std::cout << "Printing dirs/files" << std::endl;
    std::cout << "-------------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();
    std::cout << "-------------------" << std::endl;

    dirname = "Nested1 Dir1 at Dir1";
    std::cout << "\nCD into: " << dirname << std::endl;
    fatfs.change_dir(dirname);

    std::cout << "\nUsed space (root entry takes up 128 bytes): "
              << fatfs.size() << std::endl;
    std::cout << "Current directory: " << fatfs.current().name() << std::endl;
    std::cout << "Printing dirs/files" << std::endl;
    std::cout << "-------------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();
    std::cout << "-------------------" << std::endl;

    dirname = "Nested2 Dir1 at Dir1";
    filename = "Nested2 File1 at Dir1";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    fatfs.add_dir(dirname);
    std::cout << "Adding directory: " << filename << std::endl;
    fatfs.add_file(filename);

    dirname = "Nested2 Dir2 at Dir1";
    filename = "Nested2 File2 at Dir1";
    std::cout << "\nAdding directory: " << dirname << std::endl;
    fatfs.add_dir(dirname);
    std::cout << "Adding directory: " << filename << std::endl;
    fatfs.add_file(filename);

    std::cout << "\nUsed space (root entry takes up 128 bytes): "
              << fatfs.size() << std::endl;
    std::cout << "Current directory: " << fatfs.current().name() << std::endl;
    std::cout << "Printing dirs/files" << std::endl;
    std::cout << "-------------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();
    std::cout << "-------------------" << std::endl;

    dirname = "..";
    std::cout << "\nCD into: " << dirname << std::endl;
    fatfs.change_dir(dirname);

    dirname = "..";
    std::cout << "\nCD into: " << dirname << std::endl;
    fatfs.change_dir(dirname);

    std::cout << "\nUsed space (root entry takes up 128 bytes): "
              << fatfs.size() << std::endl;
    std::cout << "Current directory: " << fatfs.current().name() << std::endl;
    std::cout << "Printing dirs/files" << std::endl;
    std::cout << "-------------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();
    std::cout << "-------------------" << std::endl;

    dirname = "Dir2 at root";
    std::cout << "\nDeleting: " << dirname << std::endl;
    fatfs.delete_dir(dirname);

    std::cout << "\nUsed space (root entry takes up 128 bytes): "
              << fatfs.size() << std::endl;
    std::cout << "Current directory: " << fatfs.current().name() << std::endl;
    std::cout << "Printing dirs/files" << std::endl;
    std::cout << "-------------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();
    std::cout << "-------------------" << std::endl;

    dirname = "Dir1 at root";
    std::cout << "\nDeleting: " << dirname << std::endl;
    fatfs.delete_dir(dirname);

    filename = "File1 at root";
    std::cout << "\nDeleting: " << filename << std::endl;
    fatfs.delete_file(filename);

    filename = "File2 at root";
    std::cout << "\nDeleting: " << filename << std::endl;
    fatfs.delete_file(filename);

    std::cout << "\nUsed space (root entry takes up 128 bytes): "
              << fatfs.size() << std::endl;
    std::cout << "Current directory: " << fatfs.current().name() << std::endl;
    std::cout << "Printing dirs/files" << std::endl;
    std::cout << "-------------------" << std::endl;
    fatfs.print_dirs();
    fatfs.print_files();
    std::cout << "-------------------" << std::endl;

    std::cout << "\nRemoving file system" << std::endl;

    fatfs.remove_filesystem();

    return 0;
}
