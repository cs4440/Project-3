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
    // fs::Fat fat("client.fat", 10);

    // if(!fat.create()) fat.open_fat("client.fat");

    // auto cell = fat.get_cell(0);
    // cell.set_cell(1);
    // cell.set_block(2);
    // std::cout << cell.cell() << ", " << cell.block() << std::endl;

    // cell = fat.get_cell(1);
    // cell.cell(10);
    // cell.block(20);
    // std::cout << cell.cell() << ", " << cell.block() << std::endl;

    // cell = fat.get_cell(0);
    // std::cout << cell.cell() << ", " << cell.block() << std::endl;

    // std::cout << "Number of FatCells " << fat.size() << std::endl;
    // std::cout << "Fat file size " << fat.file_size() << std::endl;

    // fs::Disk disk("client.dat");
    // if(!disk.create()) disk.open_disk("client.dat");

    // char* file = disk.file(0);
    // fs::DirEntry direntry(file);

    // direntry.set_name("root");
    // direntry.set_valid(true);
    // direntry.set_type(0);
    // direntry.set_dot(0);
    // direntry.set_dotdot(-1);
    // direntry.set_dircell(10);
    // direntry.set_filecell(15);

    // std::cout << "name " << direntry.name() << std::endl;
    // std::cout << "valid " << direntry.valid() << std::endl;
    // std::cout << "type " << direntry.type() << std::endl;
    // std::cout << "dot " << direntry.dot() << std::endl;
    // std::cout << "dotdot " << direntry.dotdot() << std::endl;
    // std::cout << "dircell " << direntry.dircell() << std::endl;
    // std::cout << "file " << direntry.filecell() << std::endl;

    int cylinders = 10;
    int sectors = 10;
    fs::FatFS fatfs("client", cylinders, sectors);

    return 0;
}
