#ifndef DISK_H
#define DISK_H

#include <fcntl.h>      // file constants
#include <stdio.h>      // remove()
#include <sys/mman.h>   // mmap()
#include <sys/stat.h>   // path stat and constants
#include <sys/types.h>  // unix types
#include <unistd.h>     // open/write
#include <cstring>      // strncpy()
#include <stdexcept>    // std::exception
#include <string>       // std::string
#include "utils.h"      // int_to_char(), char_to_int()

namespace fs {

class Disk {
public:
    enum { BYTES = 4096, SECTOR_SZ = 128 };

    Disk(std::string name, std::size_t cyl = 1, std::size_t sec = 32);
    ~Disk();

    std::size_t cylinder();
    std::size_t sector();
    std::size_t bytes();
    std::string name();
    int fd();
    char* file();
    std::string geometry();  // return a string with disk geometry
    bool valid();

    bool create();                      // create Disk
    bool open_disk(std::string n);      // initialize Disk from existing file
    bool remove_disk();                 // remove disk file from system
    void set_cylinders(std::size_t c);  // set cylinders before init
    void set_sectors(std::size_t s);    // set sectors before init

    // read at cylinder and sector index
    std::string read_at(std::size_t cyl, std::size_t sec);
    // write str of _sec_sz
    bool write_at(const char* buf, std::size_t cyl, std::size_t sec,
                  std::size_t bufsz = SECTOR_SZ);
    bool write_at(char* buf, std::size_t cyl, std::size_t sec,
                  std::size_t bufsz = SECTOR_SZ);

private:
    unsigned _sec_sz;         // number of bytes in a sector
    std::size_t _cylinders;   // number of cyclinders
    std::size_t _sectors;     // number of sectors per cylinder
    std::size_t _bytes;       // bytes of disk without geometry info
    std::size_t _totalbytes;  // total bytes with geometry info

    std::string _name;  // disk name
    int _fd;            // file decriptor to physical file
    char* _file;        // memory pointer to file
    char* _ofile;       // original file without offset
};

}  // namespace fs

#endif  // DISK_H
