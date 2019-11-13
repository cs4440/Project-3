#ifndef DISK_H
#define DISK_H

#include <fcntl.h>      // file constants
#include <stdio.h>      // remove()
#include <sys/mman.h>   // mmap()
#include <sys/stat.h>   // path stat and constants
#include <sys/types.h>  // unix types
#include <unistd.h>     // open(), read(), write(), usleep()
#include <cstring>      // strncpy()
#include <stdexcept>    // std::exception
#include <string>       // std::string

namespace fs {

class Disk {
public:
    enum {
        BLOCK_SZ = 128,   // bytes in a sector
        TRACK_TIME = 100  // microseconds
    };

    Disk(std::string name, std::size_t cyl = 1, std::size_t sec = 32);
    ~Disk();

    std::size_t cylinder();
    std::size_t sector();
    std::size_t block();
    std::size_t track_time();
    std::size_t bytes();
    std::size_t total_bytes();
    std::string name();
    int fd();
    char* file();
    std::string geometry();  // return a string with disk geometry
    bool valid();

    bool create();                       // create Disk
    bool open_disk(std::string n);       // initialize Disk from existing file
    bool remove_disk();                  // remove disk file from system
    void set_cylinders(std::size_t c);   // set cylinders if valid
    void set_sectors(std::size_t s);     // set sectors per cylinder if valid
    void set_block_size(std::size_t s);  // set sector size if valid
    void set_track_time(std::size_t t);  // set sector size if valid

    // read at cylinder and sector index
    std::string read_at(std::size_t cyl, std::size_t sec);
    // write str of _sec_sz
    bool write_at(const char* buf, std::size_t cyl, std::size_t sec,
                  std::size_t bufsz = BLOCK_SZ);
    bool write_at(char* buf, std::size_t cyl, std::size_t sec,
                  std::size_t bufsz = BLOCK_SZ);

private:
    std::size_t _cylinders;   // number of cyclinders
    std::size_t _sectors;     // number of sectors per cylinder
    std::size_t _block;       // number of bytes in a sector
    std::size_t _bytes;       // bytes of disk without geometry info
    std::size_t _totalbytes;  // total bytes with geometry info
    std::size_t _track_time;

    std::string _name;  // disk name
    int _fd;            // file decriptor to physical file
    char* _file;        // memory pointer to file
    char* _ofile;       // original file without offset

    void _close_fd();    // close file descriptor
    void _unmap_file();  // unmap virtual memory from file
};

}  // namespace fs

#endif  // DISK_H
