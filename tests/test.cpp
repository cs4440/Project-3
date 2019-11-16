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

    std::cout << "\nAdding duplicate dir: 'Hello Folder 1'" << std::endl;
    fatfs.add_dir("Hello Folder 1");

    std::cout << "\nAdding duplicate dir: 'Hello Folder 4'" << std::endl;
    fatfs.add_dir("Hello Folder 4");

    std::cout << "\nAdding duplicate dir: 'Hello Folder 5'" << std::endl;
    fatfs.add_dir("Hello Folder 5");

    std::cout << "\nAdding duplicate dir: 'File1 at root'" << std::endl;
    fatfs.add_file("File1 at root");

    std::cout << "\nAdding duplicate dir: 'File3 at root'" << std::endl;
    fatfs.add_file("File3 at root");

    std::cout << "\nAdding duplicate dir: 'File4 at root'" << std::endl;
    fatfs.add_file("File4 at root");

    std::cout << "\nPrinting dirs and files again" << std::endl;

    fatfs.print_dirs();

    fatfs.print_files();

    std::cout << "\nUsed space: " << fatfs.size() << std::endl;

    std::string dirname = "Hello Folder 6";
    std::cout << "\nChanging directory: " << dirname << std::endl;
    fatfs.change_dir(dirname);
    std::cout << "Current dir is: " << fatfs.current().name() << std::endl;

    dirname = "Hello Folder 3";
    std::cout << "\nChanging directory: " << dirname << std::endl;
    fatfs.change_dir(dirname);
    std::cout << "Current dir is: " << fatfs.current().name() << std::endl;

    std::cout << "\nAddding folder/file at: " << dirname << std::endl;

    fatfs.add_dir("Nested Folder1");

    fatfs.add_file("Nested File1");

    fatfs.add_dir("Nested Folder2");

    fatfs.add_file("Nested File2");

    std::cout << "\nPrinting dirs/files at: " << dirname << std::endl;

    fatfs.print_dirs();

    fatfs.print_files();

    dirname = "..";
    std::cout << "\nChanging directory: " << dirname << std::endl;
    fatfs.change_dir(dirname);
    std::cout << "Current dir is: " << fatfs.current().name() << std::endl;

    std::cout << "\nPrinting dirs and files" << std::endl;

    fatfs.print_dirs();

    fatfs.print_files();

    dirname = "Hello Folder 1";
    std::cout << "\nChanging directory: " << dirname << std::endl;
    fatfs.change_dir(dirname);
    std::cout << "Current dir is: " << fatfs.current().name() << std::endl;

    std::cout << "\nPrinting dirs and files" << std::endl;

    fatfs.print_dirs();

    fatfs.print_files();

    dirname = "..";
    std::cout << "\nChanging directory: " << dirname << std::endl;
    fatfs.change_dir(dirname);
    std::cout << "Current dir is: " << fatfs.current().name() << std::endl;

    std::cout << "\nPrinting dirs and files" << std::endl;

    fatfs.print_dirs();

    fatfs.print_files();

    dirname = "Hello Folder 3";
    std::cout << "\nChanging directory: " << dirname << std::endl;
    fatfs.change_dir(dirname);
    std::cout << "Current dir is: " << fatfs.current().name() << std::endl;

    std::cout << "\nPrinting dirs and files" << std::endl;

    fatfs.print_dirs();

    fatfs.print_files();

    dirname = "..";
    std::cout << "\nChanging directory: " << dirname << std::endl;
    fatfs.change_dir(dirname);
    std::cout << "Current dir is: " << fatfs.current().name() << std::endl;

    std::cout << "\nPrinting dirs and files" << std::endl;

    fatfs.print_dirs();

    fatfs.print_files();

    dirname = "..";
    std::cout << "\nChanging directory: " << dirname << std::endl;
    fatfs.change_dir(dirname);
    std::cout << "Current dir is: " << fatfs.current().name() << std::endl;

    std::cout << "\nPrinting dirs and files" << std::endl;

    fatfs.print_dirs();

    fatfs.print_files();

    std::string filename = "File1 at root";
    std::cout << "\nFinding file: " << filename << std::endl;

    fs::FileEntry fileentry = fatfs.find_file(filename);
    if(fileentry) {
        std::cout << "File found: " << fileentry.name() << std::endl;
    } else {
        std::cout << "File NOT FOUND" << std::endl;
    }

    filename = "File2 at root";
    std::cout << "\nFinding file: " << filename << std::endl;

    fileentry = fatfs.find_file(filename);
    if(fileentry) {
        std::cout << "File found: " << fileentry.name() << std::endl;
    } else {
        std::cout << "File NOT FOUND" << std::endl;
    }

    filename = "File4 at root";
    std::cout << "\nFinding file: " << filename << std::endl;

    fileentry = fatfs.find_file(filename);
    if(fileentry) {
        std::cout << "File found: " << fileentry.name() << std::endl;
    } else {
        std::cout << "File NOT FOUND" << std::endl;
    }

    filename = "File5 at root";
    std::cout << "\nFinding file: " << filename << std::endl;

    fileentry = fatfs.find_file(filename);
    if(fileentry) {
        std::cout << "File found: " << fileentry.name() << std::endl;
    } else {
        std::cout << "File NOT FOUND" << std::endl;
    }

    // std::cout << "\nRemoving file system" << std::endl;

    // fatfs.remove_filesystem();

    return 0;
}
