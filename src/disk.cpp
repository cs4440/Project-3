#include "../include/disk.h"

namespace fs {

Disk::Disk(std::string name, std::size_t cyl, std::size_t sec)
    : _cylinders(cyl),
      _sectors(sec),
      _max_block(MAX_BLOCK),
      _offset(sizeof(_cylinders) + sizeof(_sectors) + sizeof(_max_block)),
      _logical_bytes(_cylinders * _sectors * _max_block),
      _physical_bytes(_logical_bytes + _offset),
      _track_time(TRACK_TIME),
      _name(name),
      _disk_name(name + ".disk"),
      _fd(-1),
      _file(nullptr),
      _pfile(nullptr) {}

Disk::~Disk() {
    _unmap_file();  // unmap virtual memory from file
    _close_fd();    // close file descriptor
}

bool Disk::create() {
    bool is_created = false;
    struct stat sb;  // file stats

    // if file doesn't exist
    if(stat(_disk_name.c_str(), &sb) != 0) {
        // open and create file
        _fd = ::open(_disk_name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

        if(_fd == -1)
            throw std::runtime_error("Error creating Disk file: create");

        // update private vairables
        _logical_bytes = _cylinders * _sectors * _max_block;
        _physical_bytes = _logical_bytes + _offset;

        // zero fill file of size bytes
        for(std::size_t i = 0; i < _physical_bytes; ++i) write(_fd, "\0", 1);

        // write geometry info at start of physical disk
        lseek(_fd, 0, SEEK_SET);
        write(_fd, (char *)&_cylinders, sizeof(_cylinders));
        write(_fd, (char *)&_sectors, sizeof(_sectors));
        write(_fd, (char *)&_max_block, sizeof(_max_block));

        // check if file stats is consisent
        if(fstat(_fd, &sb) == 0 && sb.st_size != (int)_physical_bytes)
            throw std::runtime_error(
                "Error checking Disk file descriptor: create");

        // map physical file to memory
        _pfile = (char *)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE,
                              MAP_SHARED, _fd, 0);

        // offset starting file address by eometry info size
        _file = _pfile + _offset;

        is_created = true;
    }

    return is_created;
}

bool Disk::open(std::string n) {
    bool is_opened = false;
    std::string diskname = n + ".disk";
    struct stat sb;  // file stats

    // remove this disk if exists
    if(_fd > -1) Disk::remove();

    // if file exists
    if(stat(diskname.c_str(), &sb) == 0) {
        // open existing file
        _fd = ::open(diskname.c_str(), O_RDWR, S_IRUSR | S_IWUSR);

        if(_fd == -1) throw std::runtime_error("Error opening Disk file: open");

        // check if file stats is consisent
        if(fstat(_fd, &sb) == -1)
            throw std::runtime_error(
                "Error checking Disk file descriptor: open");

        // map phyhsical file to memory
        _pfile = (char *)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE,
                              MAP_SHARED, _fd, 0);

        // offset starting file address by eometry info size
        _file = _pfile + _offset;

        // update geometry info
        read(_fd, (char *)&_cylinders, sizeof(_cylinders));
        read(_fd, (char *)&_sectors, sizeof(_sectors));
        read(_fd, (char *)&_max_block, sizeof(_max_block));

        _physical_bytes = sb.st_size;
        _logical_bytes = _physical_bytes - _offset;
        _name = n;
        _disk_name = diskname;

        is_opened = true;
    }

    return is_opened;
}

bool Disk::remove() {
    bool is_removed = false;

    _unmap_file();  // unmap virtual memory from file
    _close_fd();    // close file descriptor
    if(::remove(_disk_name.c_str()) == 0) is_removed = true;
    _physical_bytes = _logical_bytes = _cylinders = _sectors = 0;

    return is_removed;
}

bool Disk::valid() const { return _pfile != nullptr; }

Disk::operator bool() const { return _pfile != nullptr; }

std::size_t Disk::cylinder() const { return _cylinders; }

std::size_t Disk::sector() const { return _sectors; }

std::size_t Disk::max_block() const { return _max_block; }

std::size_t Disk::track_time() const { return _track_time; }

std::size_t Disk::logical_bytes() const { return _logical_bytes; }

std::size_t Disk::physical_bytes() const { return _physical_bytes; }

std::string Disk::name() const { return _name; }

std::string Disk::disk_name() const { return _disk_name; }

std::size_t Disk::block(std::size_t cyl, std::size_t sec) const {
    return cyl * _sectors + sec;
}

std::size_t Disk::total_blocks() const { return _cylinders * _sectors; }

std::size_t Disk::location(std::size_t cyl, std::size_t sec) const {
    return block(cyl, sec) * _max_block;
}

std::size_t Disk::location(std::size_t block) const {
    return block * _max_block;
}

std::string Disk::geometry() const {
    return std::string(std::to_string(_cylinders) + " " +
                       std::to_string(_sectors));
}

int Disk::fd() const { return _fd; }

char *Disk::file() const { return _file; }

char *Disk::data_at(int block) const {
    if(block > -1 && block < int(_cylinders * _sectors))
        return _file + (block * _max_block);
    else
        return nullptr;
}

void Disk::set_cylinders(std::size_t c) {
    if(!valid()) _cylinders = c;
}

void Disk::set_sectors(std::size_t s) {
    if(!valid()) _sectors = s;
}

void Disk::set_block_size(std::size_t s) {
    if(!valid()) _max_block = s;
}

bool Disk::set_name(std::string n) {
    if(!valid()) {
        _name = n;
        _disk_name = n + ".disk";

        return true;
    } else
        return false;
}

void Disk::set_track_time(std::size_t t) { _track_time = t; }

std::string Disk::read_at(std::size_t cyl, std::size_t sec) const {
    if(cyl > _cylinders - 1 || sec > _sectors - 1)
        return "0";
    else {
        usleep(_track_time);

        return "1" + std::string(_file + location(cyl, sec), _max_block);
    }
}

bool Disk::write_at(const char *buf, std::size_t cyl, std::size_t sec,
                    std::size_t bufsz) {
    if(cyl > _cylinders - 1 || sec > _sectors - 1 || bufsz > _max_block)
        return false;
    else {
        usleep(_track_time);

        strncpy(_file + location(cyl, sec), buf, bufsz);
        return true;
    }
}

bool Disk::write_at(char *buf, std::size_t cyl, std::size_t sec,
                    std::size_t bufsz) {
    if(cyl > _cylinders - 1 || sec > _sectors - 1 || bufsz > _max_block)
        return false;
    else {
        usleep(_track_time);

        strncpy(_file + location(cyl, sec), buf, bufsz);
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
    if(_pfile) {
        munmap(_pfile, _physical_bytes);
        _pfile = _file = nullptr;
    }
}

}  // namespace fs
