#ifndef DAT_H
#define DAT_H

#include <fcntl.h>       // io macro
#include <sys/mman.h>    // mmap()
#include <sys/stat.h>    // fstat
#include <sys/types.h>   // struct stat
#include <unistd.h>      // open()
#include <cstdio>        // remove()
#include <cstring>       // strncpy(), memset()
#include <ctime>         // ctime(), time_t
#include <iomanip>       // setw()
#include <iostream>      // stream
#include <list>          // list
#include <set>           // set
#include <stdexcept>     // exception
#include <string>        // string
#include "ansi_style.h"  // terminaal ANSI styling in unix
#include "disk.h"        // Disk class

namespace fs {

enum ENTRY { DIR = 0, FILE = 1, ENDBLOCK = -1, MAX_NAME = 79 };

/*******************************************************************************
 * Class to representation of a cell in File Allocation Table (FAT).
 * The FatCell takes in an address (of a FAT) to represent its data.
 *
 * Structure of a cell: [ int next cell/block #]
 * Size: sizeof(int) for each fat cell
 *
 * Values of status: FREE or USED
 *
 * Cell value of USED state is FatCell::END or greater
 * Cell value of FREE is FatCell::FREE or less
 ******************************************************************************/
struct FatCell {
    enum {
        FREE = ENDBLOCK - 1,    // indicates free cell
        END = ENTRY::ENDBLOCK,  // end of cell/block indicator
        SIZE = sizeof(int)      // bytes of a FatCell
    };

    int* _next_cell;

    // CONSTRUCTOR
    FatCell(char* address = nullptr) : _next_cell((int*)(address)) {}

    bool valid() const { return _next_cell != nullptr; }
    bool has_next() const { return _next_cell != nullptr && *_next_cell > END; }
    bool free() const { return *_next_cell <= FREE; }
    bool used() const { return *_next_cell > FREE; }

    // get cell/block data from address
    int cell() const { return *_next_cell; }

    // set cell/block from address
    void set_free() { *_next_cell = FREE; }
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
 * Subdirectories (DirEntry's) are contained in dircell_index linked list.
 * Files (FileEntry's) are contained in filecell_index linked list.
 *
 * Structure of Entry
 * |      name     | valid | type | dot | dotdot | dir ptr | file ptr | times...
 *  char* MAX_NAME   bool    bool   int    int       int       int      time_t
 *
 * Default values when constructed with valid address:
 * name: null bytes
 * type: ENTRY:DIR
 * dot: ENTRY::ENDBLOCK, self pointer
 * dotdot: ENTRY:ENDBLOCK, parent pointer
 * dircell_index: ENTRY:ENDBLOCK, head pointer of linked list to DirEntry
 * filecell_index: ENTRY:ENDBLOCK, head pointer of linked list to FileEntry
 * created: current time
 * last_accessed: current time
 * last_modified: current time
 ******************************************************************************/
class DirEntry {
public:
    DirEntry(char* address = nullptr) { _reset_address(address); }

    bool has_dirs() const { return dircell_index() > ENTRY::ENDBLOCK; }
    bool has_files() const { return filecell_index() > ENTRY::ENDBLOCK; }
    bool has_parent() const { return dotdot() != ENTRY::ENDBLOCK; }
    bool valid() const { return _name != nullptr; }
    operator bool() const { return _name != nullptr; }  // explicit bool conv
    void clear() { _reset_address(nullptr); };

    std::string name() const { return std::string(_name); }
    bool type() const { return *_type; }
    int dot() const { return *_dot; }
    int dotdot() const { return *_dotdot; }
    int dircell_index() const { return *_dircell_index; }
    int filecell_index() const { return *_filecell_index; }
    int size() const { return *_size; }
    time_t created() const { return *_created; }
    time_t* created_ptr() const { return _created; }
    char* created_str() const { return std::ctime(_created); }
    time_t last_accessed() const { return *_last_accessed; }
    time_t* last_accessed_ptr() const { return _created; }
    char* last_accessed_str() const { return std::ctime(_last_accessed); }
    time_t last_modified() const { return *_last_modified; }
    time_t* last_modified_ptr() const { return _created; }
    char* last_modified_str() const { return std::ctime(_last_modified); }

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
        set_size(0);
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
    void set_size(int size) { *_size = size; }
    void inc_size(int inc) { *_size += inc; }
    void dec_size(int dec) { *_size -= dec; }
    void set_created(time_t t) { *_created = t; }
    void update_created() { *_created = std::time(_created); }
    void set_last_accessed(time_t t) { *_last_accessed = t; }
    void update_last_accessed() { *_last_accessed = std::time(_last_accessed); }
    void set_last_modified(time_t t) { *_last_modified = t; }
    void update_last_modified() { *_last_modified = std::time(_last_modified); }

    friend bool operator<(const DirEntry& lhs, const DirEntry& rhs) {
        return lhs.name() < rhs.name();
    }

    void _reset_address(char* address) {
        _name = address;
        _type = (bool*)(_name + MAX_NAME);
        _dot = (int*)(_type + 1);
        _dotdot = _dot + 1;
        _dircell_index = _dotdot + 1;
        _filecell_index = _dircell_index + 1;
        _size = _filecell_index + 1;
        _created = (time_t*)(_size + 1);
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
    int* _size;
    time_t* _created;
    time_t* _last_accessed;
    time_t* _last_modified;
};

/*******************************************************************************
 * Class to representation of a Directory Entry in a disk block.
 * File data blocks (DataEntry's) are contained in datacell_index linked list.
 *
 * Structure of Entry
 * |      name      | valid | type | data ptr | times...
 *  char* MAX_NAME    bool    bool      int     time_t
 *
 * Default values when constructed with valid address:
 * name: null bytes
 * type: ENTRY:DIR
 * dot: ENTRY::ENDBLOCK, self pointer
 * dotdot: ENTRY:ENDBLOCK, parent pointer
 * datacell_index: ENTRY:ENDBLOCK, head pointer linked list to DataEntry
 * size: bytes of all data links (does not include nul byte)
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
    int dotdot() const { return *_dotdot; }
    int datacell_index() const { return *_datacell_index; }
    int size() const { return *_size; }
    time_t created() const { return *_created; }
    time_t* created_ptr() const { return _created; }
    char* created_str() const { return std::ctime(_created); }
    time_t last_accessed() const { return *_last_accessed; }
    time_t* last_accessed_ptr() const { return _created; }
    char* last_accessed_str() const { return std::ctime(_last_accessed); }
    time_t last_modified() const { return *_last_modified; }
    time_t* last_modified_ptr() const { return _created; }
    char* last_modified_str() const { return std::ctime(_last_modified); }

    // set address if FileEntry was not created with valid address
    void set_address(char* address) { _reset_address(address); }

    // clear and initialize all fields to default values
    // WARNING: will delete existing data!
    void init() {
        memset(_name, 0, MAX_NAME);
        set_type(ENTRY::FILE);
        set_dot(ENTRY::ENDBLOCK);
        set_dotdot(ENTRY::ENDBLOCK);
        set_datacell_index(ENTRY::ENDBLOCK);
        set_size(0);
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
    void set_datacell_index(int cell) { *_datacell_index = cell; }
    void set_size(int size) { *_size = size; }
    void inc_size(int inc) { *_size += inc; }
    void dec_size(int dec) { *_size -= dec; }
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
        _dotdot = _dot + 1;
        _datacell_index = _dotdot + 1;
        _size = _datacell_index + 1;
        _created = (time_t*)(_size + 1);
        _last_accessed = _created + 1;
        _last_modified = _last_accessed + 1;
    }

private:
    char* _name;
    bool* _type;
    int* _dot;
    int* _dotdot;
    int* _datacell_index;
    int* _size;
    time_t* _created;
    time_t* _last_accessed;
    time_t* _last_modified;
};

/*******************************************************************************
 * DataEntry represent a data block in disk of size Disk::MAX_BLOCK
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

    bool valid() const { return _data != nullptr; }
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
    Fat(char* address = nullptr, int cells = -1, int cell_offset = -1);
    ~Fat();

    bool create();  // create FAT table on disk
    bool open();    // load existing FAT table in disk
    void remove();  // clear

    bool valid() const;     // check if this FAT table is valid
    operator bool() const;  // explicit bool conv
    std::size_t size() const;
    std::size_t full() const;

    // return FatCell to read/write data to
    FatCell get_cell(int index);

    std::set<int>& free_blocks() { return _free; }

private:
    char* _file;       // mmap of file
    int _cells;        // number of cells
    int _cell_offset;  // starting cell index
    std::set<int> _free;
};

/*******************************************************************************
 * Class to representation a FAT File System. It uses a FAT table to manage
 * file and directory structures. The FAT table near the begining of a disk.
 *
 * Structure of formatted disk:
 * | META DATA | FAT TABLE | LOGICAL DISK BLOCKS START HERE
 *
 * Meta data
 * ---------
 * int fat_offset: offset from disk where FAT table starts
 * int _block_offset: disk block index offset to start data blocks
 * int logical_blocks: the number of actual data blocks for in disk
 *
 * FAT TABLE
 * ---------
 * Fat table start at disk address + fat_offset
 * Size is logical_blocks * FatCell::SIZE
 *
 * DATA BLOCKS
 * -----------
 * Data blocks start at _block_offset
 *
 * NOTE
 * ----
 * One-to-one relationship of FatCell index to Data blocks
 * (index + block_offset) of FatCell to disk data block #
 *
 * Ex:
 *  - index of FAT table = 3
 *  - block offset = 5
 *  - FAT[3] = FatCell index = 3 + 5 = 8
 *  - Thus FatCell index of 8 corresponds to disk block of 8.
 ******************************************************************************/
class FatFS {
public:
    typedef std::set<DirEntry, bool (*)(const DirEntry&, const DirEntry&)>
        DirSet;
    typedef std::set<FileEntry, bool (*)(const FileEntry&, const FileEntry&)>
        FileSet;

    enum { META_SZ = 3 * sizeof(int) };

    FatFS(Disk* disk = nullptr);

    // FILE SYSTEM INITIALIZATIONS!!!
    bool set_disk(Disk* disk);  // set disk for file system to use
    bool open_disk();           // open formatted disk, false if not formatted
    bool format();              // format disk
    bool valid() const;         // check if FatFS instance is valid
    void remove();              // WARNING Will delete both disk and fat file!

    std::size_t size() const;        // size of FS in bytes
    std::size_t free_space() const;  // return unused bytes left in disk
    std::size_t used_space() const;  // return used bytes in disk
    bool full() const;               // if disk is full
    std::string name() const;        // name of the file system
    std::string info() const;        // return string filesystem info
    std::string size_info() const;   // return string only size info
    std::string pwd() const;         // print working directory
    DirEntry current() const;        // return current directory entry

    // print directory at path, default to cwd of "."
    void print_dirs(std::ostream& outs = std::cout, std::string path = ".",
                    bool is_details = false);
    void print_files(std::ostream& outs = std::cout, std::string path = ".",
                     bool is_details = false);
    void print_all(std::ostream& outs = std::cout, std::string path = ".",
                   bool is_details = false);

    void set_name(std::string name);
    DirEntry add_dir(std::string path);     // add last entry in path
    FileEntry add_file(std::string path);   // add last entry in path
    bool delete_dir(std::string path);      // remove last entry in path
    bool delete_file(std::string path);     // remove last entry in path
    bool change_dir(std::string path);      // change to path if valid
    FileEntry find_file(std::string path);  // find last entry in path

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
    std::string _name;  // name of Fat file system
    Disk* _disk;        // physical disk
    Fat _fat;           // FAT table
    DirEntry _root;     // root directory entry
    DirEntry _current;  // current directory entry

    int _logical_blocks;  // number of available blocks in disk after format
    int _block_offset;    // block offset after format

    // create a root DirEntry at begining of logical blocks
    void _init_root();

    // add dir or file at given directory
    DirEntry _add_dir_at(DirEntry& target, std::string name);
    FileEntry _add_file_at(DirEntry& target, std::string name);

    // get a set of all entry at given directory entry by comparison function
    void _dirs_at(DirEntry& dir_entry, DirSet& entries_set);
    void _files_at(DirEntry& dir_entry, FileSet& entries_set);

    // find an entry by name at given directory or return invalid entry
    DirEntry _find_dir_at(DirEntry& dir_entry, std::string name);
    FileEntry _find_file_at(DirEntry& dir_entry, std::string name);

    // find an entry by name at given directory or return last entry
    DirEntry _find_dir_orlast_at(DirEntry& dir_entry, std::string name);
    FileEntry _find_file_orlast_at(DirEntry& dir_entry, std::string name);

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
    FatCell _last_dircell_from_dir(DirEntry& dir_entry);
    FatCell _last_filecell_from_dir(DirEntry& dir_entry);
    FatCell _last_datacell_from_file(FileEntry& file_entry);
    FatCell _last_cell_from_cell(int cell_offset);

    // tokenize a path string and return a list of name entries
    void _tokenize_path(std::string path, std::list<std::string>& entries);

    // parse a path of string named entries; return a valid DirEntry if found
    DirEntry _parse_dir_entries(std::list<std::string>& entries);

    void _find_entries_len_details(DirSet& entries_set, std::size_t& name_len,
                                   std::size_t& byte_len);
    void _find_entries_len_details(FileSet& entries_set, std::size_t& name_len,
                                   std::size_t& byte_len);
};

}  // namespace fs

#endif  // DAT_H
