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
#include <iostream>     // stream
#include <queue>        // queue
#include <stdexcept>    // exception
#include <string>       // string
#include "disk.h"       // Disk class

namespace fs {

enum ENTRY { DIR = 0, FILE = 1, ENDBLOCK = -1, MAX_NAME = 86 };

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
    FatCell(int* address = nullptr) : _cell(address), _block(address + 1) {}

    bool valid() const { return _cell != nullptr; }
    bool has_next() const { return _cell != nullptr && *_cell > END; }
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
 * Structure of Entry
 * |   name   | valid | type | dot | dotdot | dir ptr | file ptr | times...
 *   MAX_NAME   bool    bool   int    int       int       int      time_t
 *
 * Default values when constructed with valid address:
 * name: null bytes
 * valid: true
 * type: ENTRY:DIR
 * dot:  ENTRY::ENDBLOCK
 * dotdot: ENTRY:ENDBLOCK
 * dircell_index: ENTRY:ENDBLOCK
 * filecell_index: ENTRY:ENDBLOCK
 * created: current time
 * last_accessed: current time
 * last_modified: current time
 ******************************************************************************/
class DirEntry {
public:
    DirEntry(char* address = nullptr) { _reset_address(address); }

    bool has_dirs() const { return dircell_index() != ENTRY::ENDBLOCK; }
    bool has_files() const { return filecell_index() != ENTRY::ENDBLOCK; }
    bool is_valid() const { return _name != nullptr; }
    operator bool() const { return _name != nullptr; }  // explicit bool conv
    void clear() { _reset_address(nullptr); };

    std::string name() const { return std::string(_name); }
    bool valid() const { return *_valid; }
    bool type() const { return *_type; }
    int dot() const { return *_dot; }
    int dotdot() const { return *_dotdot; }
    int dircell_index() const { return *_dircell_index; }
    int filecell_index() const { return *_filecell_index; }
    time_t created() const { return *_created; }
    time_t last_accessed() const { return *_last_accessed; }
    time_t last_modified() const { return *_last_modified; }

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
        set_dircell_index(ENTRY::ENDBLOCK);
        set_filecell_index(ENTRY::ENDBLOCK);
        update_created();
        update_last_accessed();
        update_last_modified();
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
    void set_dircell_index(int cell) { *_dircell_index = cell; }
    void set_filecell_index(int cell) { *_filecell_index = cell; }
    void set_created(time_t t) { *_created = t; }
    void update_created() { *_created = std::time(_created); }
    void set_last_accessed(time_t t) { *_last_accessed = t; }
    void update_last_accessed() { *_last_accessed = std::time(_last_accessed); }
    void set_last_modified(time_t t) { *_last_modified = t; }
    void update_last_modified() { *_last_modified = std::time(_last_modified); }

    void _reset_address(char* address) {
        _name = address;
        _valid = (bool*)(_name + MAX_NAME);
        _type = _valid + 1;
        _dot = (int*)(_type + 1);
        _dotdot = _dot + 1;
        _dircell_index = _dotdot + 1;
        _filecell_index = _dircell_index + 1;
        _created = (time_t*)(_filecell_index + 1);
        _last_accessed = _created + 1;
        _last_modified = _last_accessed + 1;
    }

private:
    char* _name;
    bool* _valid;
    bool* _type;
    int* _dot;
    int* _dotdot;
    int* _dircell_index;
    int* _filecell_index;
    time_t* _created;
    time_t* _last_accessed;
    time_t* _last_modified;
};

/*******************************************************************************
 * Class to representation of a Directory Entry in a disk block.
 *
 * Structure of Entry
 * |   name   | valid | type | data ptr | times...
 *   MAX_NAME   bool    bool      int     time_t
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
 * last_modified: current time
 ******************************************************************************/
class FileEntry {
public:
    FileEntry(char* address = nullptr) { _reset_address(address); }

    bool has_data() const { return datacell_index() != ENTRY::ENDBLOCK; }
    bool is_valid() const { return _name != nullptr; }
    operator bool() const { return _name != nullptr; }  // explicit bool conv

    std::string name() const { return std::string(_name); }
    bool valid() const { return *_valid; }
    bool type() const { return *_type; }
    int datacell_index() const { return *_datacell_index; }
    time_t created() const { return *_created; }
    time_t last_accessed() const { return *_last_accessed; }
    time_t last_modified() const { return *_last_modified; }

    // set address if FileEntry was not created with valid address
    void set_address(char* address) { _reset_address(address); }

    // clear and initialize all fields to default values
    // WARNING: will delete existing data!
    void init() {
        memset(_name, 0, MAX_NAME);
        set_valid(true);
        set_type(ENTRY::FILE);
        set_datacell_index(ENTRY::ENDBLOCK);
        update_created();
        update_last_accessed();
        update_last_modified();
    }

    void set_name(std::string name) {
        if(name.size() > MAX_NAME)
            throw std::range_error("File name size exceeded");

        memset(_name, 0, MAX_NAME);
        strncpy(_name, name.c_str(), name.size());
    }

    void set_valid(bool valid) { *_valid = valid; }
    void set_type(bool type) { *_type = type; }
    void set_datacell_index(int cell) { *_datacell_index = cell; }
    void set_created(time_t t) { *_created = t; }
    void update_created() { *_created = std::time(_created); }
    void set_last_accessed(time_t t) { *_last_accessed = t; }
    void update_last_accessed() { *_last_accessed = std::time(_last_accessed); }
    void set_last_modified(time_t t) { *_last_modified = t; }
    void update_last_modified() { *_last_modified = std::time(_last_modified); }

    void _reset_address(char* address) {
        _name = address;
        _valid = (bool*)(_name + MAX_NAME);
        _type = _valid + 1;
        _datacell_index = (int*)(_type + 1);
        _created = (time_t*)(_datacell_index + 1);
        _last_accessed = _created + 1;
        _last_modified = _last_accessed + 1;
    }

private:
    char* _name;
    bool* _valid;
    bool* _type;
    int* _datacell_index;
    time_t* _created;
    time_t* _last_accessed;
    time_t* _last_modified;
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

    bool create();                    // create FAT table on disk
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

    std::size_t size() const;  // size of FS in bytes
    DirEntry current() const;  // return current directory entry
    void print_dirs();         // print directories at current dir
    void print_files();        // print files at current dir

    // WARNING Will delete both disk and fat file!
    void remove_filesystem();

    DirEntry add_dir(std::string name);     // add directory at current dir
    FileEntry add_file(std::string name);   // add file at current dir
    bool change_dir(std::string name);      // change current directory
    FileEntry find_file(std::string name);  // get file entry at current dir

private:
    std::string _name;      // name of Fat file system
    std::string _diskname;  // name of physical disk
    std::string _fatname;   // name of physical fat file
    Disk _disk;             // physical disk
    Fat _fat;               // FAT table
    DirEntry _root;         // root directory entry
    DirEntry _current;      // current directory entry
    std::queue<int> _free;  // queue of free cells -> free blocks

    void _init_root();
    void _init_free();

    // return by ref a list of subdirectory blocks at given disk block
    void _dirs_at(int start_block, std::queue<int>& entry_blocks);
    void _files_at(int start_block, std::queue<int>& entry_blocks);
    DirEntry _find_dir_at(int start_block, std::string name);
    FileEntry _find_file_at(int start_block, std::string name);
    FatCell _last_cell_from_block(int start_block);
    FatCell _last_cell_from_cell(int start_cell);
};

}  // namespace fs

#endif  // DAT_H
