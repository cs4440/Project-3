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
        MAX_BLOCK = 128,  // bytes in a sector
        TRACK_TIME = 100  // microseconds
    };

    Disk(std::string name, std::size_t cyl = 1, std::size_t sec = 32);
    ~Disk();

    // DISK INITIALIZATIONS !!!
    bool create();             // create Disk
    bool open(std::string n);  // initialize Disk from existing file
    bool remove();             // remove disk file from system
    bool valid() const;        // check if disk is valid

    std::size_t cylinder() const;
    std::size_t sector() const;
    std::size_t max_block() const;
    std::size_t track_time() const;
    std::size_t logical_bytes() const;
    std::size_t physical_bytes() const;
    std::string name() const;
    std::string disk_name() const;
    std::size_t block(std::size_t cyl, std::size_t sec) const;
    std::size_t total_blocks() const;
    std::size_t location(std::size_t cyl, std::size_t sec) const;
    std::size_t location(std::size_t block) const;
    std::string geometry() const;  // return a string with disk geometry
    int fd();
    char* file();
    char* file_at(int block);

    void set_cylinders(std::size_t c);   // set cylinders if valid
    void set_sectors(std::size_t s);     // set sectors per cylinder if valid
    void set_block_size(std::size_t s);  // set sector size if valid
    void set_track_time(std::size_t t);  // set sector size if valid
    bool set_name(std::string n);        // set disk name when not valid

    // read at cylinder and sector index
    std::string read_at(std::size_t cyl, std::size_t sec);
    // write str of _sec_sz
    bool write_at(const char* buf, std::size_t cyl, std::size_t sec,
                  std::size_t bufsz = MAX_BLOCK);
    bool write_at(char* buf, std::size_t cyl, std::size_t sec,
                  std::size_t bufsz = MAX_BLOCK);

private:
    std::size_t _cylinders;       // number of cyclinders
    std::size_t _sectors;         // number of sectors per cylinder
    std::size_t _max_block;       // number of bytes in a sector
    std::size_t _offset;          // offset to read real data
    std::size_t _logical_bytes;   // bytes of disk without geometry info
    std::size_t _physical_bytes;  // total bytes with geometry info
    std::size_t _track_time;      // microseconds to sleep during read/write

    std::string _name;       // disk basename
    std::string _disk_name;  // full disk filename with extension
    int _fd;                 // file decriptor to physical file
    char* _file;             // logical file where actual data starts
    char* _pfile;            // physical file, original address from mmap

    void _close_fd();    // close file descriptor
    void _unmap_file();  // unmap virtual memory from file
};

}  // namespace fs

#endif  // DISK_H
