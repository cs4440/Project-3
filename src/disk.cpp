#include "../include/disk.h"
#include <iostream>

namespace fs {

Disk::Disk(std::string name, std::size_t cyl, std::size_t sec)
    : _sec_sz(SECTOR_SZ),
      _cylinders(cyl),
      _sectors(sec),
      _bytes(_cylinders * _sectors * _sec_sz),
      _totalbytes(_cylinders * _sectors * _sec_sz + 8),
      _name(name),
      _fd(-1),
      _file(nullptr) {}

Disk::~Disk() {
    _close_fd();    // close file descriptor
    _unmap_file();  // unmap virtual memory from file
}

std::size_t Disk::cylinder() { return _cylinders; }

std::size_t Disk::sector() { return _sectors; }

std::size_t Disk::bytes() { return _bytes; }

std::string Disk::name() { return _name; }

int Disk::fd() { return _fd; }

char *Disk::file() { return _file; }

std::string Disk::geometry() {
    return std::string(std::to_string(_cylinders) + " " +
                       std::to_string(_sectors));
}

bool Disk::valid() { return _file != nullptr; }

bool Disk::create() {
    bool is_created = false;
    char cyl[4] = {0}, sec[4] = {0};
    struct stat sb;  // file stats

    // if file doesn't exist
    if(stat(_name.c_str(), &sb) != 0) {
        // open and create file
        _fd = open(_name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

        if(_fd == -1) throw std::runtime_error("Error creating disk file");

        // zero fill file of size bytes
        for(std::size_t i = 0; i < _totalbytes; ++i) write(_fd, "\0", 1);

        // convert int cylinders and sectors to 4 byte chars
        utils::int_to_char(_cylinders, cyl);
        utils::int_to_char(_sectors, sec);

        // write geometry info to first 8 bytes of file
        lseek(_fd, 0, SEEK_SET);
        write(_fd, cyl, 4);
        write(_fd, sec, 4);
        lseek(_fd, 0, SEEK_SET);

        // check if file stats is consisent
        if(fstat(_fd, &sb) == 0 && sb.st_size != (int)_totalbytes)
            throw std::runtime_error("Error disk size inconsistent");

        // map file to memory
        _ofile = _file = (char *)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE,
                                      MAP_SHARED, _fd, 0);

        // offset starting file address by eometry info size
        _file = _file + sizeof(cyl) + sizeof(sec);

        is_created = true;
    }

    return is_created;
}

bool Disk::open_disk(std::string n) {
    bool is_opened = false;
    int cyl = 0, sec = 0;
    struct stat sb;  // file stats

    // remove this disk if exists
    if(_fd > -1) remove_disk();

    // if file exists
    if(stat(n.c_str(), &sb) == 0) {
        // open existing file
        _fd = open(n.c_str(), O_RDWR, S_IRUSR | S_IWUSR);

        if(_fd == -1) throw std::runtime_error("Error opening disk file");

        // check if file stats is consisent
        if(fstat(_fd, &sb) == -1)
            throw std::runtime_error("Error disk size inconsistent");

        // map file to memory
        _ofile = _file = (char *)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE,
                                      MAP_SHARED, _fd, 0);

        // read geometry info from file
        utils::char_to_int(_file, cyl);
        utils::char_to_int(_file + 4, sec);

        // update private vairables
        _cylinders = cyl;
        _sectors = sec;
        _totalbytes = sb.st_size;
        _bytes = _totalbytes - 8;
        _name = n;

        // offset starting file address by eometry info size
        _file = _file + 8;

        is_opened = true;
    }

    return is_opened;
}

bool Disk::remove_disk() {
    bool is_removed = false;

    _close_fd();    // close file descriptor
    _unmap_file();  // unmap virtual memory from file
    if(remove(_name.c_str()) == 0) is_removed = true;  // remove sysmte file

    return is_removed;
}

void Disk::set_cylinders(std::size_t c) { _cylinders = c; }

void Disk::set_sectors(std::size_t s) { _sectors = s; }

std::string Disk::read_at(std::size_t cyl, std::size_t sec) {
    if(cyl > _cylinders - 1 || sec > _sectors - 1)
        return "0";
    else
        return "1" + std::string(_file + (cyl * sec), SECTOR_SZ);
}

bool Disk::write_at(const char *buf, std::size_t cyl, std::size_t sec,
                    std::size_t bufsz) {
    if(cyl > _cylinders - 1 || sec > _sectors - 1 || bufsz > SECTOR_SZ)
        return false;
    else {
        strncpy(_file + (cyl * sec), buf, bufsz);
        return true;
    }
}

bool Disk::write_at(char *buf, std::size_t cyl, std::size_t sec,
                    std::size_t bufsz) {
    if(cyl > _cylinders - 1 || sec > _sectors - 1 || bufsz > SECTOR_SZ)
        return false;
    else {
        strncpy(_file + (cyl * sec), buf, bufsz);
        return true;
    }
}

void Disk::_close_fd() {
    if(_fd > -1) {
        close(_fd);
        _fd = -1;
    }
}

void Disk::_unmap_file() {
    if(_ofile) {
        munmap(_ofile, _totalbytes);
        _ofile = _file = nullptr;
    }
}

}  // namespace fs
