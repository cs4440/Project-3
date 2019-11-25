#ifndef FAT_H
#define FAT_H

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
#include <tuple>         // forward_as_tuple()
#include "ansi_style.h"  // terminaal ANSI styling in unix
#include "disk.h"        // Disk class

namespace fs {

/*******************************************************************************
 * Entry is the base class to hold metadata in a disk.
 *
 * Structure of Entry
 * |      name     | valid | type | dot | dotdot | timestamps...
 *  char* MAX_NAME   bool    bool   int    int       time_t
 *
 * Default values when constructed with valid address:
 * name: null bytes
 * type: Entry:DIR
 * dot: Entry::ENDBLOCK, self pointer
 * dotdot: Entry:ENDBLOCK, parent pointer
 * size: 0
 * created: current time
 * last_accessed: current time
 * last_modified: current time
 ******************************************************************************/
class Entry {
public:
    enum { ENDBLOCK = -1, DIR = 0, FILE = 1, MAX_NAME = 79 };

    // ENTRY COMPARATORS
    // compare entry by type and then by name
    static bool cmp_entry(const Entry& a, const Entry& b);
    // compare entry just by name
    static bool cmp_entry_name(const Entry& a, const Entry& b);

    Entry(char* address = nullptr);

    bool has_parent() const;
    bool valid() const;
    operator bool() const;

    std::string name() const;
    bool type() const;
    int dot() const;
    int dotdot() const;
    int size() const;
    time_t created() const;
    time_t* created_ptr() const;
    char* created_str() const;
    time_t last_accessed() const;
    time_t* last_accessed_ptr() const;
    char* last_accessed_str() const;
    time_t last_modified() const;
    time_t* last_modified_ptr() const;
    char* last_modified_str() const;

    // Clear and initialize all fields to default values
    // Must init when adding a new and fresh Entry!
    // WARNING: will delete existing data!
    void init();

    // clear the entry with invalid state
    void clear();

    // set address if DirEntry was not created with valid address
    void set_address(char* address);

    void set_name(std::string name);

    void set_type(bool type);
    void set_dot(int block);
    void set_dotdot(int block);
    void set_size(int size);
    void inc_size(int inc);
    void dec_size(int dec);
    void set_created(time_t t);
    void update_created();
    void set_last_accessed(time_t t);
    void update_last_accessed();
    void set_last_modified(time_t t);
    void update_last_modified();

    friend bool operator<(const Entry& lhs, const Entry& rhs) {
        return lhs.name() < rhs.name();
    }

protected:
    char* _name;
    bool* _type;
    int* _dot;
    int* _dotdot;
    int* _size;
    time_t* _created;
    time_t* _last_accessed;
    time_t* _last_modified;

    void _reset_address(char* address);
};

/*******************************************************************************
 * DirEntry is an extension of the Entry class for a directory.
 * It add these attributes: dir_head and file_head, which are linked list head
 * ptr to directory/file entry block in disk.
 *
 * Structure of Entry
 * |   Entry   | dir_head | file_head
 *                 int         int
 *
 * Default values when constructed with valid address:
 * dir_head: Entry:ENDBLOCK, head pointer of linked list to DirEntry
 * file_head: Entry:ENDBLOCK, head pointer of linked list to FileEntry
 ******************************************************************************/
class DirEntry : public Entry {
public:
    DirEntry(char* address = nullptr);

    bool has_dirs() const;
    bool has_files() const;
    int dir_head() const;
    int file_head() const;

    // Clear and initialize all fields to default values
    // Must init when adding a new and fresh Entry!
    // WARNING: will delete existing data!
    void init();

    // clear the entry with invalid state
    void clear();

    // set address if DirEntry was not created with valid address
    void set_address(char* address);

    void set_dir_head(int cell);
    void set_file_head(int cell);

protected:
    int* _dir_head;   // dir entry head ptr
    int* _file_head;  // file entry head ptr

    // set address offsets from Entry's last adddress
    void _init_dir();

    // reset all attribute addresses
    void _reset_address(char* address);
};

/*******************************************************************************
 * FileEntry is an extension of the Entry class for a file.
 * It add these attributes: data_head (linked list ptr to start of data blocks)
 * and data_size (the size of all data, not rounded up to data blocks).
 *
 * Structure of Entry
 * |   Entry   | data_head | data_size
 *                  int         int
 *
 * Default values when constructed with valid address:
 * data_head: Entry:ENDBLOCK, head pointer linked list to DataEntry
 * data_size: bytes of all data links (does not include nul byte)
 ******************************************************************************/
class FileEntry : public Entry {
public:
    FileEntry(char* address = nullptr);

    bool has_data() const;
    int data_head() const;
    int data_size() const;

    // Clear and initialize all fields to default values
    // Must init when adding a new and fresh Entry!
    // WARNING: will delete existing data!
    void init();

    // clear the entry with invalid state
    void clear();

    // set address if FileEntry was not created with valid address
    void set_address(char* address);

    void set_data_head(int cell);
    void set_data_size(int size);
    void inc_data_size(int size);
    void dec_data_size(int size);

protected:
    int* _data_head;  // data block head ptr
    int* _data_size;  // size of data (stops at nul byte)

    // set address offsets from Entry's last adddress
    void _init_file();

    // reset all attribute addresses
    void _reset_address(char* address);
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
    DataEntry(char* address = nullptr);

    bool valid() const;
    operator bool() const;

    char* data();
    void clear(std::size_t size);

    // read data up to Disk::MAX_BLOCK and return successful bytes read
    std::size_t read(char* buf, std::size_t size, std::size_t limit);

    // write data up to Disk::MAX_BLOCK and return successful bytes read
    std::size_t write(const char* src, std::size_t size, std::size_t limit,
                      bool is_nullfill = true);
    std::size_t write(char* src, std::size_t size, std::size_t limit,
                      bool is_nullfill = true);

    // append data up to Disk::MAX_BLOCK and return successful bytes read
    std::size_t append(const char* src, std::size_t size, std::size_t offset,
                       std::size_t limit, bool is_nullfill = true);
    std::size_t append(char* src, std::size_t size, std::size_t offset,
                       std::size_t limit, bool is_nullfill = true);

private:
    char* _data;
};

/*******************************************************************************
 * Data representation of a cell in File Allocation Table (FAT).
 * The FatCell is an array element of the FAT at a specified address.
 * The FatCell is also like a node in a linked list in the FAT. The status of
 * the node is marked by the _next_cell's value. If the cell is not free, then
 * it points to the next cell or signal end of block. If cell is free, it
 * points nowhere.
 *
 * Structure of a cell: [ int next cell/block #]
 * Size: sizeof(int) for each fat cell
 *
 * Value of cell: FREE or USED
 *  - USED state is FatCell::END or greater
 *  - FREE state is FatCell::FREE or less
 ******************************************************************************/
struct FatCell {
    enum {
        FREE = Entry::ENDBLOCK - 1,  // indicates free cell
        END = Entry::ENDBLOCK,       // end of cell/block indicator
        SIZE = sizeof(int)           // bytes of a FatCell
    };

    int* _next_cell;

    // CONSTRUCTOR
    FatCell(char* address = nullptr);

    bool has_next() const;
    bool free() const;
    bool used() const;
    bool valid() const;
    operator bool() const;

    // get next cell/block
    int next_cell() const;

    // set cell/block from address
    void set_free();
    void set_next_cell(int c);

    friend bool operator==(const FatCell& lhs, const FatCell& rhs) {
        return lhs.next_cell() == rhs.next_cell();
    }

    friend bool operator!=(const FatCell& lhs, const FatCell& rhs) {
        return lhs.next_cell() != rhs.next_cell();
    }

    friend bool operator<(const FatCell& lhs, const FatCell& rhs) {
        return lhs.next_cell() < rhs.next_cell();
    }
};

/*******************************************************************************
 * Class to representation of File Allocation Table (FAT), which is comprised
 * of FatCell. FAT table size is an array of FatCells, which is mapped from
 * a physical file to memory. FAT traversal is through a FatCell, which acts
 * like a node in a linked list.
 *
 * Each cell in a FAT also represent a block in disk at _cell_offset.
 * Ex: FAT[5] = Disk block at [5 + _cell_offset];
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
    FatCell get_cell(int index) const;

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
    // Set of DirEntry and FileEntry with custom Comparison object
    typedef std::set<Entry, bool (*)(const Entry&, const Entry&)> EntrySet;
    typedef std::set<DirEntry, bool (*)(const Entry&, const Entry&)> DirSet;
    typedef std::set<FileEntry, bool (*)(const Entry&, const Entry&)> FileSet;

    enum { META_SZ = 3 * sizeof(int) };  // filesystem metadata at start of disk

    FatFS(Disk* disk = nullptr);

    // FILE SYSTEM INITIALIZATIONS!!!
    bool set_disk(Disk* disk);  // set disk for file system to use
    bool open_disk();           // open formatted disk, false if not formatted
    bool format();              // format disk
    bool valid() const;         // check if FatFS instance is valid
    void remove();              // WARNING Will delete disk in system!

    std::size_t total_size() const;  // total logical size of disk
    std::size_t size() const;        // return used bytes in disk
    std::size_t free_size() const;   // return unused bytes left in disk
    bool full() const;               // if disk is full
    std::string name() const;        // name of the file system
    std::string info() const;        // return string filesystem info
    std::string size_info() const;   // return string only size info
    std::string pwd() const;         // print working directory
    DirEntry current() const;        // return current directory entry

    // print directory at path, default to cwd of "."
    void print_dirs(std::ostream& outs = std::cout, std::string path = ".",
                    bool is_details = false) const;
    void print_files(std::ostream& outs = std::cout, std::string path = ".",
                     bool is_details = false) const;
    void print_all(std::ostream& outs = std::cout, std::string path = ".",
                   bool is_details = false) const;

    void set_name(std::string name);
    DirEntry add_dir(std::string path);           // add last entry in path
    FileEntry add_file(std::string path);         // add last entry in path
    bool delete_dir(std::string path);            // remove last entry in path
    bool delete_file(std::string path);           // remove last entry in path
    bool change_dir(std::string path);            // change to path if valid
    FileEntry find_file(std::string path) const;  // find last entry in path

    // read file data into data buffer of size
    // returns successful bytes read
    std::size_t read_file_data(FileEntry& file, char* data,
                               std::size_t size) const;

    // overwrite data buffer to file entry
    std::size_t write_file_data(FileEntry& file, const char* data,
                                std::size_t size);

    // append data buffer to file entry
    std::size_t append_file_data(FileEntry& file, const char* data,
                                 std::size_t size);

    // remove all data blocks for this file entry
    void remove_file_data(FileEntry& file);

private:
    std::string _name;  // name of filesystem
    Disk* _disk;        // physical disk
    Fat _fat;           // FAT table
    DirEntry _root;     // root directory entry
    DirEntry _current;  // current directory entry

    int _logical_blocks;  // number of available blocks in disk after format
    int _block_offset;    // block offset after format

    // create a root DirEntry at begining of logical blocks
    void _init_root();

    // add dir or file at given directory
    DirEntry _add_dir_at(DirEntry& dir, std::string name);
    FileEntry _add_file_at(DirEntry& dir, std::string name);

    // get a set of all entry at given directory entry by comparison function
    void _entries_at(DirEntry& dir, EntrySet& entries_set) const;
    void _dirs_at(DirEntry& dir, DirSet& entries_set) const;
    void _files_at(DirEntry& dir, FileSet& entries_set) const;

    // find an entry by name at given directory or return invalid entry
    DirEntry _find_dir_at(DirEntry& dir, std::string name) const;
    FileEntry _find_file_at(DirEntry& dir, std::string name) const;

    // find an entry by name at given directory or return last entry
    DirEntry _find_dir_orlast_at(DirEntry& dir, std::string name) const;
    FileEntry _find_file_orlast_at(DirEntry& dir, std::string name) const;

    // delete directory of a specificed name
    bool _delete_dir_at(DirEntry& dir, std::string name);
    bool _delete_file_at(DirEntry& dir, std::string name);

    // recursively delete subdirectories and files within this dir entry
    // does not delete dir itself
    void _free_dir_at(DirEntry& dir);

    // free all data blocks in file entry
    void _free_data_at(FileEntry& file);

    // mark given FatCell as free and push to free list
    void _free_cell(FatCell& cell, int cell_index);

    // update all parents size, up to root directory
    void _update_parents_size(DirEntry dir, std::size_t size);

    // get last cell from entry
    FatCell _last_dircell_from(DirEntry& dir) const;
    FatCell _last_filecell_from(DirEntry& dir) const;
    FatCell _last_datacell_from(FileEntry& file) const;
    FatCell _last_cell_from(int cell_offset) const;
    FatCell _sec_last_cell_from(int cell_offset) const;

    // tokenize a path string and return a list of name entries
    void _tokenize_path(std::string path,
                        std::list<std::string>& entries) const;

    // parse a path of string named entries; return a valid DirEntry if found
    DirEntry _parse_dir_entries(std::list<std::string>& entries) const;

    // find max name length and size length
    void _find_entries_len_details(EntrySet& entries_set, std::size_t& name_len,
                                   std::size_t& byte_len) const;
    void _find_entries_len_details(DirSet& entries_set, std::size_t& name_len,
                                   std::size_t& byte_len) const;
    void _find_entries_len_details(FileSet& entries_set, std::size_t& name_len,
                                   std::size_t& byte_len) const;
};

}  // namespace fs

#endif  // FAT_H
