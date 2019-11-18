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
#include <set>          // set
#include <stack>        // stack
#include <stdexcept>    // exception
#include <string>       // string
#include "disk.h"       // Disk class

namespace fs {

enum ENTRY { DIR = 0, FILE = 1, ENDBLOCK = -1, MAX_NAME = 87 };

/*******************************************************************************
 * Class to representation of a cell in File Allocation Table (FAT).
 * The FatCell takes in an address (of a FAT) to represent its data.
 *
 * Structure of a cell: [ bool status, int next cell/block #]
 * Size: sizeof(bool) + sizeof(int) for each fat cell
 *
 * Values of status: FREE or USED
 *
 * Values of _next_cell: ENTRY::ENDBLOCK or greater
 * ENTRY::ENDBLOCK: end of next cell/block chain
 * _next cell > ENDBLOCK: valid next cell/block chain
 ******************************************************************************/
struct FatCell {
    enum {
        FREE = 0,                          // indicates free cell
        USED = 1,                          // indicates used cell
        SIZE = sizeof(bool) + sizeof(int)  // 8 bytes for a FatCell
    };

    bool* _status;
    int* _next_cell;

    // CONSTRUCTOR
    FatCell(char* address = nullptr)
        : _status((bool*)address), _next_cell((int*)(_status + 1)) {}

    bool valid() const { return _status != nullptr; }
    bool has_next() const {
        return _next_cell != nullptr && *_next_cell > ENTRY::ENDBLOCK;
    }
    bool free() const { return *_status == FREE; }
    bool used() const { return *_status == USED; }

    // get cell/block data from address
    int cell() const { return *_next_cell; }

    // set cell/block from address
    void set_free() { *_status = FREE; }
    void set_used() { *_status = USED; }
    void set_next_cell(int c) { *_next_cell = c; }

    friend bool operator==(const FatCell& lhs, const FatCell& rhs) {
        return lhs.cell() == rhs.cell();
    }

    friend bool operator!=(const FatCell& lhs, const FatCell& rhs) {
        return lhs.cell() != rhs.cell();
    }

    friend bool operator<(const FatCell& lhs, const FatCell& rhs) {
        return lhs.cell() < rhs.cell();
    }

    friend bool operator<=(const FatCell& lhs, const FatCell& rhs) {
        return lhs.cell() <= rhs.cell();
    }

    friend bool operator>(const FatCell& lhs, const FatCell& rhs) {
        return lhs.cell() > rhs.cell();
    }

    friend bool operator>=(const FatCell& lhs, const FatCell& rhs) {
        return lhs.cell() >= rhs.cell();
    }
};

/*******************************************************************************
 * Class to representation of a Directory Entry in a disk block.
 *
 * Structure of Entry
 * |      name     | valid | type | dot | dotdot | dir ptr | file ptr | times...
 *  char* MAX_NAME   bool    bool   int    int       int       int      time_t
 *
 * Default values when constructed with valid address:
 * name: null bytes
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

    bool has_dirs() const { return dircell_index() > ENTRY::ENDBLOCK; }
    bool has_files() const { return filecell_index() > ENTRY::ENDBLOCK; }
    bool valid() const { return _name != nullptr; }
    operator bool() const { return _name != nullptr; }  // explicit bool conv
    void clear() { _reset_address(nullptr); };

    std::string name() const { return std::string(_name); }
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
        _type = (bool*)(_name + MAX_NAME);
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
 * |      name      | valid | type | data ptr | times...
 *  char* MAX_NAME    bool    bool      int     time_t
 *
 * Default values when constructed with valid address:
 * name: null bytes
 * type: ENTRY:DIR
 * datacell_index: ENTRY:ENDBLOCK
 * size: bytes of all data links (does not include nul byte)
 * data_blocks: the number of data blocks in a disk, excluding Entry itself
 * created: current time
 * last_accessed: current time
 * last_modified: current time
 ******************************************************************************/
class FileEntry {
public:
    FileEntry(char* address = nullptr) { _reset_address(address); }

    bool has_data() const { return datacell_index() > ENTRY::ENDBLOCK; }
    bool valid() const { return _name != nullptr; }
    operator bool() const { return _name != nullptr; }  // explicit bool conv

    std::string name() const { return std::string(_name); }
    bool type() const { return *_type; }
    int dot() const { return *_dot; }
    int datacell_index() const { return *_datacell_index; }
    int size() const { return *_size; }
    int data_blocks() const { return *_data_blocks; }
    time_t created() const { return *_created; }
    time_t last_accessed() const { return *_last_accessed; }
    time_t last_modified() const { return *_last_modified; }

    // set address if FileEntry was not created with valid address
    void set_address(char* address) { _reset_address(address); }

    // clear and initialize all fields to default values
    // WARNING: will delete existing data!
    void init() {
        memset(_name, 0, MAX_NAME);
        set_type(ENTRY::FILE);
        set_dot(ENTRY::ENDBLOCK);
        set_datacell_index(ENTRY::ENDBLOCK);
        set_size(0);
        set_data_blocks(0);
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

    void set_type(bool type) { *_type = type; }
    void set_dot(int block) { *_dot = block; }
    void set_datacell_index(int cell) { *_datacell_index = cell; }
    void set_size(int size) {
        *_size = size;

        // update block size
        set_data_blocks((size + Disk::MAX_BLOCK - 1) / Disk::MAX_BLOCK);
    };
    void set_data_blocks(int size) { *_data_blocks = size; };
    void set_created(time_t t) { *_created = t; }
    void update_created() { *_created = std::time(_created); }
    void set_last_accessed(time_t t) { *_last_accessed = t; }
    void update_last_accessed() { *_last_accessed = std::time(_last_accessed); }
    void set_last_modified(time_t t) { *_last_modified = t; }
    void update_last_modified() { *_last_modified = std::time(_last_modified); }

    friend bool operator<(const FileEntry& lhs, const FileEntry& rhs) {
        return lhs.name() < rhs.name();
    }

    void _reset_address(char* address) {
        _name = address;
        _type = (bool*)(_name + MAX_NAME);
        _dot = (int*)(_type + 1);
        _datacell_index = _dot + 1;
        _size = _datacell_index + 1;
        _data_blocks = _size + 1;
        _created = (time_t*)(_data_blocks + 1);
        _last_accessed = _created + 1;
        _last_modified = _last_accessed + 1;
    }

private:
    char* _name;
    bool* _type;
    int* _dot;
    int* _datacell_index;
    int* _size;
    int* _data_blocks;
    time_t* _created;
    time_t* _last_accessed;
    time_t* _last_modified;
};

/*******************************************************************************
 * DataEntry represent a data block in disk of siz Disk::MAX_BLOCK
 *
 * Structure of Entry
 * |          data          |
 *   char* Disk::MAX_BLOCK
 *
 * No default values. Data is whatever is in current block.
 ******************************************************************************/
class DataEntry {
public:
    DataEntry(char* address = nullptr) : _data(address) {}

    bool is_valid() const { return _data != nullptr; }
    char* data() { return _data; }
    void clear() { memset(_data, 0, Disk::MAX_BLOCK); }

    // read data up to Disk::MAX_BLOCK and return successful bytes read
    std::size_t read(char* buf, std::size_t size) {
        std::size_t bytes = 0;

        while(bytes < Disk::MAX_BLOCK && _data[bytes] != '\0' && bytes < size) {
            buf[bytes] = _data[bytes];
            ++bytes;
        }

        return bytes;
    }

    // write data up to Disk::MAX_BLOCK and return successful bytes read
    std::size_t write(const char* src, std::size_t size,
                      bool is_nullfill = true) {
        if(size > Disk::MAX_BLOCK) {
            memcpy(_data, src, Disk::MAX_BLOCK);
            return Disk::MAX_BLOCK;
        } else {
            memcpy(_data, src, size);

            if(is_nullfill)
                while(size < Disk::MAX_BLOCK) _data[size++] = '\0';

            return size;
        }
    }

    // write data up to Disk::MAX_BLOCK and return successful bytes read
    std::size_t write(char* src, std::size_t size, bool is_nullfill = true) {
        if(size > Disk::MAX_BLOCK) {
            memcpy(_data, src, Disk::MAX_BLOCK);
            return Disk::MAX_BLOCK;
        } else {
            memcpy(_data, src, size);

            if(is_nullfill)
                while(size < Disk::MAX_BLOCK) _data[size++] = '\0';

            return size;
        }
    }

private:
    char* _data;
};

// ENTRY COMPARATORS
bool cmp_dir_name(const DirEntry& a, const DirEntry& b);
bool cmp_file_name(const FileEntry& a, const FileEntry& b);

/*******************************************************************************
 * Class to representation of File Allocation Table (FAT), which is comprised
 * of FatCell. FAT table size is an array of FatCells, which is mapped from
 * a physical file to memory.
 ******************************************************************************/
class Fat {
public:
    Fat(std::string name = std::string(), int cells = -1);
    ~Fat();

    bool create();                    // create FAT table on disk
    bool open_fat(std::string name);  // load existing FAT table
    bool remove_fat();  // remove FAT table and invalidate instance

    bool valid() const;             // check if this FAT table is valid
    std::string name() const;       // name of fat file
    std::size_t size() const;       // total name of fat cells
    std::size_t file_size() const;  // total bytes of fat file

    bool set_name(std::string name);
    void set_cells(int cells);

    // return FatCell to read/write data to
    FatCell get_cell(int index);

    // read cell by reference arguments
    void read_cell(int index, bool& free, int& cell);

    // write to fat cell
    void write_cell(int index, bool free, int cell);

private:
    std::string _name;     // fat file base name
    std::string _fatname;  // // full fat filename with extension
    int _cells;            // number of cells
    char* _file;           // mmap of file
    int _fd;               // file descriptor
    std::size_t _bytes;    // file size
    void _close_fd();      // close file descriptor
    void _unmap_file();    // unmap virtual memory from file
};

class FatFS {
public:
    typedef std::set<DirEntry, bool (*)(const DirEntry&, const DirEntry&)>
        DirSet;
    typedef std::set<FileEntry, bool (*)(const FileEntry&, const FileEntry&)>
        FileSet;
    FatFS(Disk* disk = nullptr);

    // FILE SYSTEM INITIALIZATIONS!!!
    bool set_disk(Disk* disk);  // set disk for file system to use
    bool format();              // format disk
    bool valid() const;         // check if FatFS instance is valid
    void remove_filesystem();   // WARNING Will delete both disk and fat file!

    std::size_t size() const;        // size of FS in bytes
    std::size_t free_space() const;  // return unused bytes left in disk
    std::size_t used_space() const;  // return used bytes in disk
    bool full() const;               // if disk is full
    std::string info() const;        // return string file system information
    std::string size_info() const;   // return string only size info
    DirEntry current() const;        // return current directory entry
    void print_dirs();               // print directories at current dir
    void print_dirs_str(std::string& output);   // a string of dir listing
    void print_files();                         // print files at current dir
    void print_files_str(std::string& output);  // string of file listing

    DirEntry add_dir(std::string name);     // add directory entry @ current dir
    FileEntry add_file(std::string name);   // add file entry @ current dir
    bool delete_dir(std::string name);      // delete dir entry @ current dir
    bool delete_file(std::string name);     // delete file entry @ current dir
    bool change_dir(std::string name);      // change current directory
    FileEntry find_file(std::string name);  // get file entry @ current dir

    // read file data into data buffer of size
    // returns successful bytes read
    std::size_t read_file_data(FileEntry& file_entry, char* data,
                               std::size_t size);

    // overwrite data buffer to file entry
    std::size_t write_file_data(FileEntry& file_entry, const char* data,
                                std::size_t size);

    // remove all data blocks for this file entry
    void remove_file_data(FileEntry& file_entry);

private:
    std::string _name;      // name of Fat file system
    Disk* _disk;            // physical disk
    Fat _fat;               // FAT table
    DirEntry _root;         // root directory entry
    DirEntry _current;      // current directory entry
    std::stack<int> _free;  // queue of free cells -> free blocks

    void _init_root();
    void _init_free();

    // get a set of all entry at given directory entry by comparison function
    void _dirs_at(DirEntry& dir_entry, DirSet& entry_blocks);
    void _files_at(DirEntry& dir_entry, FileSet& entry_blocks);

    // find an entry by name at given directory entry
    DirEntry _find_dir_at(DirEntry& dir_entry, std::string name);
    FileEntry _find_file_at(DirEntry& dir_entry, std::string name);

    // delete directory of a specificed name
    bool _delete_dir_at_dir(DirEntry& dir_entry, std::string name);
    bool _delete_file_at_dir(DirEntry& dir_entry, std::string name);

    // recursively delete subdirectories and files within this dir entry
    // does not delete dir_entry itself
    void _free_dir_contents(DirEntry& dir_entry);

    // free all data blocks in file entry
    void _free_file_data_blocks(FileEntry& file_entry);

    // mark given FatCell as free and push to free list
    void _free_cell(FatCell& cell, int cell_index);

    // get last cell from entry
    FatCell _last_dircell_from_dir_entry(DirEntry& dir_entry);
    FatCell _last_filecell_from_dir_entry(DirEntry& dir_entry);
    FatCell _last_datacell_from_file_entry(FileEntry& file_entry);
    FatCell _last_cell_from_cell(int start_cell);
};

}  // namespace fs

#endif  // DAT_H
