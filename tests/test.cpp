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

    int cylinders = 10;
    int sectors = 10;
    fs::FatFS fatfs("client", cylinders, sectors);

    std::cout << "\nUsed space (root entry takes up 128 bytes): "
              << fatfs.size() << std::endl;

    std::cout << "\nAdding directories and files at root" << std::endl;
    fatfs.add_dir("Hello Folder 1");

    fatfs.add_dir("Hello Folder 2");

    fatfs.add_dir("Hello Folder 3");

    fatfs.add_file("File1 at root");

    fatfs.add_file("File2 at root");

    std::cout << "\nUsed space: " << fatfs.size() << std::endl;

    std::cout << "\nPrinting dirs and files" << std::endl;

    fatfs.print_dirs();

    fatfs.print_files();

    std::cout << "\nAdding directories and files at root again" << std::endl;

    fatfs.add_dir("Hello Folder 4");

    fatfs.add_dir("Hello Folder 5");

    fatfs.add_file("File3 at root");

    fatfs.add_file("File4 at root");

    std::cout << "\nPrinting dirs and files again" << std::endl;

    fatfs.print_dirs();

    fatfs.print_files();

    std::cout << "\nUsed space: " << fatfs.size() << std::endl;

    // std::cout << "\nRemoving file system" << std::endl;

    // // fatfs.remove_filesystem();

    return 0;
}
