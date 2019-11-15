#ifndef DAT_H
#define DAT_H

#include <fcntl.h>      // io macro
#include <stdio.h>      // remove()
#include <sys/mman.h>   // mmap()
#include <sys/stat.h>   // fstat
#include <sys/types.h>  // struct stat
#include <unistd.h>     // open
#include <cstring>      // strncpy(), memset()
#include <ctime>        // ctime(), time_t
#include <iostream>
#include <stdexcept>  // exception
#include <string>     // string
#include "disk.h"     // Disk class

namespace fs {

enum ENTRY { DIR = 0, FILE = 1, ENDBLOCK = -1 };

/*******************************************************************************
 * Class to representation of a cell in File Allocation Table (FAT).
 * The FatCell takes in an address (of a FAT) to represent its data.
 *
 * Structure of a cell: [ int cell #, int block #]
 * Size: 2 * sizeof(int) for each fat cell
 *
 * Values of cell:
 * -2 = unused cell
 * -1 = end of cell
 * 0 >= next cell #
 ******************************************************************************/
struct FatCell {
    enum {
        FREE = -2,  // Indicates available cell
        END = -1,   // Indicates end of cell
        SIZE = 8    // 8 bytes for a FatCell
    };

    int* _cell;
    int* _block;

    // CONSTRUCTOR
    FatCell(int* address) : _cell(address), _block(address + 1) {}

    bool free() const { return *_cell == FREE; }
    bool end() const { return *_cell == END; }

    // get cell/block data from address
    int cell() const { return *_cell; }
    int block() const { return *_block; }

    // set cell/block from address
    void set_cell(int c) { *_cell = c; }
    void set_block(int b) { *_block = b; }

    friend bool operator==(const FatCell& lhs, const FatCell& rhs) {
        return lhs.cell() == rhs.block();
    }

    friend bool operator!=(const FatCell& lhs, const FatCell& rhs) {
        return lhs.cell() != rhs.block();
    }

    friend bool operator<(const FatCell& lhs, const FatCell& rhs) {
        return lhs.cell() < rhs.block();
    }

    friend bool operator<=(const FatCell& lhs, const FatCell& rhs) {
        return lhs.cell() <= rhs.block();
    }

    friend bool operator>(const FatCell& lhs, const FatCell& rhs) {
        return lhs.cell() > rhs.block();
    }

    friend bool operator>=(const FatCell& lhs, const FatCell& rhs) {
        return lhs.cell() >= rhs.block();
    }
};

/*******************************************************************************
 * Class to representation of a Directory Entry in a disk block.
 *
 * Structure of a cell
 * |   name   | valid | type | dot | dotdot | dir cell | file cell | times...
 *   MAX_NAME   bool    bool   int    int       int         int      time_t
 *
 * Default values when constructed with valid address:
 * name: null bytes
 * valid: true
 * type: ENTRY:DIR
 * dot:  ENTRY::ENDBLOCK
 * dotdot: ENTRY:ENDBLOCK
 * dir_cell: ENTRY:ENDBLOCK
 * file_cell: ENTRY:ENDBLOCK
 * created: current time
 * last_accessed: current time
 * last_updated: current time
 * 0 >= next cell #
 ******************************************************************************/
struct DirEntry {
    enum { MAX_NAME = 86 };

    char* _name;
    bool* _valid;
    bool* _type;
    int* _dot;
    int* _dotdot;
    int* _dir_cell;
    int* _file_cell;
    time_t* _created;
    time_t* _last_accessed;
    time_t* _last_updated;

    DirEntry(char* address = nullptr) { _reset_address(address); }

    std::string name() const { return std::string(_name, MAX_NAME); }
    bool valid() const { return *_valid; }
    bool type() const { return *_type; }
    int dot() const { return *_dot; }
    int dotdot() const { return *_dotdot; }
    int dircell() const { return *_dir_cell; }
    int filecell() const { return *_file_cell; }
    time_t created() const { return *_created; }
    time_t last_accessed() const { return *_last_accessed; }
    time_t last_updated() const { return *_last_updated; }

    // set address if DirEntry was not created with valid address
    void set_address(char* address) { _reset_address(address); }

    // clear and initialize all fields to default values
    // WARNING: will delete existing data!
    void init() {
        memset(_name, 0, MAX_NAME);
        set_valid(true);
        set_type(ENTRY::DIR);
        set_dot(ENTRY::ENDBLOCK);
        set_dotdot(ENTRY::ENDBLOCK);
        set_dircell(ENTRY::ENDBLOCK);
        set_filecell(ENTRY::ENDBLOCK);
        update_created();
        update_last_accessed();
        update_last_updated();
    }

    void set_name(std::string name) {
        if(name.size() > MAX_NAME)
            throw std::range_error("File name size exceeded");

        memset(_name, 0, MAX_NAME);
        strncpy(_name, name.c_str(), name.size());
    }

    void set_valid(bool valid) { *_valid = valid; }
    void set_type(bool type) { *_type = type; }
    void set_dot(int block) { *_dot = block; }
    void set_dotdot(int block) { *_dotdot = block; }
    void set_dircell(int cell) { *_dir_cell = cell; }
    void set_filecell(int cell) { *_file_cell = cell; }
    void set_created(time_t t) { *_created = t; }
    void update_created() { *_created = std::time(_created); }
    void set_last_accessed(time_t t) { *_last_accessed = t; }
    void update_last_accessed() { *_last_accessed = std::time(_last_accessed); }
    void set_last_updated(time_t t) { *_last_updated = t; }
    void update_last_updated() { *_last_updated = std::time(_last_updated); }

    void _reset_address(char* address) {
        if(address) {
            _name = address;
            _valid = (bool*)(_name + MAX_NAME);
            _type = _valid + 1;
            _dot = (int*)(_type + 1);
            _dotdot = _dot + 1;
            _dir_cell = _dotdot + 1;
            _file_cell = _dir_cell + 1;
            _created = (time_t*)(_file_cell + 1);
            _last_accessed = _created + 1;
            _last_updated = _last_accessed + 1;
        }
    }
};

/*******************************************************************************
 * Class to representation of File Allocation Table (FAT), which is comprised
 * of FatCell. FAT table size is an array of FatCells, which is mapped from
 * a physical file to memory.
 ******************************************************************************/
class Fat {
public:
    Fat(std::string name, int cells);
    ~Fat();

    bool create();                    // create FAT table on fisk
    bool open_fat(std::string name);  // load existing FAT table
    bool remove_fat();  // remove FAT table and invalidate instance

    bool valid() const;             // check if this FAT table is valid
    std::string name() const;       // name of fat file
    std::size_t size() const;       // total name of fat cells
    std::size_t file_size() const;  // total bytes of fat file

    void set_name(std::string name);
    void set_cells(int cells);

    // return FatCell to read/write data to
    FatCell get_cell(int index);

    // read cell by reference arguments
    void read_cell(int index, int& cell, int& block);

    // write to fat cell
    void write_cell(int index, int cell, int block);

private:
    std::string _name;   // file name
    int _cells;          // number of cells
    int* _file;          // mmap of file
    int _fd;             // file descriptor
    std::size_t _bytes;  // file size
    void _close_fd();    // close file descriptor
    void _unmap_file();  // unmap virtual memory from file
};

class FatFS {
public:
    FatFS(std::string name, std::size_t cylinders, std::size_t sectors);

    // WARNING Will delete both disk and fat file!
    void remove_filesystem();

private:
    std::string _name;      // name of Fat file system
    std::string _diskname;  // name of physical disk
    std::string _fatname;   // name of physical fat file
    Disk _disk;             // physical disk
    Fat _fat;               // FAT table
    DirEntry _root;

    void _init_root();
};

}  // namespace fs

#endif  // DAT_H
