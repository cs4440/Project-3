#include "../include/fat.h"

namespace fs {

// ENTRY COMPARATORS
bool Entry::cmp_entry(const Entry &a, const Entry &b) {
    return std::forward_as_tuple(a.type(), a.name()) <
           std::forward_as_tuple(b.type(), b.name());
}

bool Entry::cmp_entry_name(const Entry &a, const Entry &b) {
    return a.name() < b.name();
}

// read data up to Disk::MAX_BLOCK and return successful bytes read
std::size_t DataEntry::read(char *buf, std::size_t size, std::size_t limit) {
    std::size_t bytes = 0;

    while(bytes < limit && _data[bytes] != '\0' && bytes < size) {
        buf[bytes] = _data[bytes];
        ++bytes;
    }

    return bytes;
}

// write data up to Disk::MAX_BLOCK and return successful bytes read
std::size_t DataEntry::write(const char *src, std::size_t size,
                             std::size_t limit, bool is_nullfill) {
    if(size > limit) {
        memcpy(_data, src, limit);
        return limit;
    } else {
        memcpy(_data, src, size);

        if(is_nullfill)
            while(size < limit) _data[size++] = '\0';

        return size;
    }
}

// write data up to Disk::MAX_BLOCK and return successful bytes read
std::size_t DataEntry::write(char *src, std::size_t size, std::size_t limit,
                             bool is_nullfill) {
    if(size > limit) {
        memcpy(_data, src, limit);
        return limit;
    } else {
        memcpy(_data, src, size);

        if(is_nullfill)
            while(size < limit) _data[size++] = '\0';

        return size;
    }
}

// append data up to Disk::MAX_BLOCK and return successful bytes read
std::size_t DataEntry::append(const char *src, std::size_t size,
                              std::size_t offset, std::size_t limit,
                              bool is_nullfill) {
    std::size_t avail = limit - offset;
    char *data = _data + offset;

    if(size > avail) {
        memcpy(data, src, avail);
        return avail;
    } else {
        memcpy(data, src, size);

        if(is_nullfill)
            while(size < limit) data[size++] = '\0';

        return size;
    }
}

// append data up to Disk::MAX_BLOCK and return successful bytes read
std::size_t DataEntry::append(char *src, std::size_t size, std::size_t offset,
                              std::size_t limit, bool is_nullfill) {
    std::size_t avail = limit - offset;

    if(size > avail) {
        memcpy(_data + offset, src, avail);
        return avail;
    } else {
        memcpy(_data + offset, src, avail);

        if(is_nullfill)
            while(size < limit) _data[size++] = '\0';

        return size;
    }
}

Fat::Fat(char *address, int cells, int cell_offset)
    : _file(address), _cells(cells), _cell_offset(cell_offset) {}

Fat::~Fat() {}

bool Fat::create() {
    if(_file) {
        char *file = _file;
        FatCell cell;

        for(int i = 0; i < _cells; ++i) {
            cell = FatCell(file);
            cell.set_free();
            file += FatCell::SIZE;

            _free.emplace(_cell_offset + i);  // populate free cell list
        }

        return true;
    } else
        return false;
}

bool Fat::open() {
    if(_file) {
        FatCell cell;
        char *file = _file;

        for(int i = 0; i < _cells; ++i) {
            cell = FatCell(file);
            file += FatCell::SIZE;

            // populate free cell list
            if(cell.free()) _free.emplace(i + _cell_offset);
        }
        return true;
    } else
        return false;
}

void Fat::remove() {
    _file = nullptr;
    _cells = _cell_offset = 0;
    _free.clear();
}

bool Fat::valid() const { return _file != nullptr; }

Fat::operator bool() const { return _file != nullptr; }

std::size_t Fat::size() const { return _free.size(); }

std::size_t Fat::full() const { return _free.size() == 0; }

FatCell Fat::get_cell(int index) const {
    if((index - _cell_offset) > -1 && (index - _cell_offset) < _cells)
        return FatCell(_file + (index - _cell_offset) * FatCell::SIZE);
    else
        return FatCell(nullptr);
}

FatFS::FatFS(Disk *disk) : _disk(disk) {}

bool FatFS::set_disk(Disk *disk) {
    if(disk && disk->valid()) {
        _name = disk->name();
        _disk = disk;

        return true;
    } else
        return false;
}

bool FatFS::open_disk() {
    if(_disk) {
        char *diskfile = _disk->file();

        // read FS metadata
        int fat_offset = *(int *)diskfile;
        diskfile += sizeof(fat_offset);

        int block_offset = *(int *)diskfile;
        diskfile += sizeof(_block_offset);

        int logical_blocks = *(int *)diskfile;

        if(fat_offset > 0 && block_offset > 0 && logical_blocks > 0) {
            // error checks
            if(fat_offset > FatFS::META_SZ) return false;
            if(logical_blocks > (int)_disk->total_blocks()) return false;
            if(((int)_disk->total_blocks() - logical_blocks) != block_offset)
                return false;

            _block_offset = block_offset;
            _logical_blocks = logical_blocks;

            char *fat_address = _disk->file() + fat_offset;
            _fat = Fat(fat_address, _logical_blocks, _block_offset);

            if(_fat.open()) {
                // root entry always at begining of block offset
                // get root entry at block offset
                _root = _current = DirEntry(_disk->file_at(block_offset));

                return true;
            } else
                return false;
        } else
            return false;
    } else
        return false;
}

bool FatFS::format() {
    if(_disk->valid()) {
        // calculate bytes of of FAT table
        int initial_fat_sz = _disk->total_blocks() * FatCell::SIZE;

        // total bytes usage at start of disk with FatFS meta data
        int total_meta_sz = initial_fat_sz + FatFS::META_SZ;

        // get blocks offset to start data blocks in disk
        _block_offset =
            (total_meta_sz + _disk->max_block()) / _disk->max_block();

        // available disk blocks for data
        _logical_blocks = _disk->total_blocks() - _block_offset;

        // write FS metadata: fat offset, logical block offset
        // and number of logical blocks
        char *diskfile = _disk->file();
        int fat_offset = FatFS::META_SZ;
        memcpy(diskfile, &fat_offset, sizeof(fat_offset));
        diskfile += sizeof(fat_offset);
        memcpy(diskfile, &_block_offset, sizeof(_block_offset));
        diskfile += sizeof(_block_offset);
        memcpy(diskfile, &_logical_blocks, sizeof(_logical_blocks));

        // create FAT table in disk
        char *fat_address = _disk->file() + FatFS::META_SZ;
        _fat = Fat(fat_address, _logical_blocks, _block_offset);
        _fat.create();

        // initialize root entry
        _init_root();

        return true;
    } else
        return false;
}

bool FatFS::valid() const {
    return _disk != nullptr && _disk->valid() && _fat.valid();
}

void FatFS::remove_file_data(FileEntry &file_entry) {
    file_entry.update_last_accessed();
    file_entry.update_last_modified();
    _free_file_data_blocks(file_entry);
}

std::size_t FatFS::total_size() const {
    if(_disk)
        return _logical_blocks * _disk->max_block();
    else
        return 0;
}

std::size_t FatFS::size() const {
    if(_root)
        return _root.size();
    else
        return 0;
}

std::size_t FatFS::free_size() const {
    if(_disk)
        return _fat.size() * _disk->max_block();
    else
        return 0;
}

bool FatFS::full() const { return _fat.size() == 0; }

std::string FatFS::name() const { return _name; }

std::string FatFS::info() const {
    return "Disk name: " + _name + '\n' + "Valid: " + std::to_string(valid()) +
           '\n' + size_info();
}

std::string FatFS::size_info() const {
    return "Logical disk size: " + std::to_string(total_size()) + '\n' +
           "Free space (bytes): " + std::to_string(free_size()) + '\n' +
           "Used space (bytes): " + std::to_string(size());
}

std::string FatFS::pwd() const {
    std::string path;
    DirEntry dir = _current;

    if(dir) {
        path = dir.name();

        while(dir.dotdot() != FatCell::END) {
            dir = DirEntry(_disk->file_at(dir.dotdot()));

            if(dir.name() != "/")
                path = dir.name() + "/" + path;
            else
                path = dir.name() + path;
        }
    }
    return path;
}

DirEntry FatFS::current() const { return _current; }

void FatFS::print_dirs(std::ostream &outs, std::string path,
                       bool is_details) const {
    using namespace style;

    struct tm *tm_info;
    std::size_t max_name_len = 0, max_byte_len = 0;
    DirEntry dir;
    std::list<std::string> entries_path;
    DirSet entries(Entry::cmp_entry_name);

    // tokenize a path string to list of named entries
    _tokenize_path(path, entries_path);

    // find a valid end point of the path of named entries
    dir = _parse_dir_entries(entries_path);

    // get an ordered set of DirEntry by comparator
    _dirs_at(dir, entries);

    // find max name column size
    if(is_details)
        _find_entries_len_details(entries, max_name_len, max_byte_len);

    for(auto it = entries.begin(); it != entries.end(); ++it) {
        outs << Ansi(BOLD) << Ansi(BLUE) << Ansi(REVERSE) << it->name()
             << Ansi(RESET);

        if(is_details)
            outs << std::setw(max_name_len - it->name().size() + 1) << ' ';

        if(is_details) {
            outs << std::right << std::setw(max_byte_len) << it->size();

            tm_info = std::localtime(it->last_modified_ptr());
            outs << ' ' << std::put_time(tm_info, "%b %d %H:%M");
        }
        auto next = it;
        if(++next != entries.end()) outs << '\n';
    }
}

void FatFS::print_files(std::ostream &outs, std::string path,
                        bool is_details) const {
    using namespace style;

    struct tm *tm_info;
    std::size_t max_name_len = 0, max_byte_len = 0;
    DirEntry dir;
    std::list<std::string> entries_path;
    FileSet entries(Entry::cmp_entry_name);

    // tokenize a path string to list of named entries
    _tokenize_path(path, entries_path);

    // find a valid end point of the path of named entries
    dir = _parse_dir_entries(entries_path);

    // get an ordered set of FileEntry by comparator
    _files_at(dir, entries);

    // find max name column size
    if(is_details)
        _find_entries_len_details(entries, max_name_len, max_byte_len);

    for(auto it = entries.begin(); it != entries.end(); ++it) {
        outs << Ansi(BOLD) << Ansi(BLUE) << it->name() << Ansi(RESET);

        if(is_details)
            outs << std::setw(max_name_len - it->name().size() + 1) << ' ';

        if(is_details) {
            outs << std::right << std::setw(max_byte_len) << it->size();

            tm_info = std::localtime(it->last_modified_ptr());
            outs << ' ' << std::put_time(tm_info, "%b %d %H:%M");
        }
        auto next = it;
        if(++next != entries.end()) outs << '\n';
    }
}

void FatFS::print_all(std::ostream &outs, std::string path,
                      bool is_details) const {
    using namespace style;

    struct tm *tm_info;
    std::size_t max_name_len = 0, max_byte_len = 0;
    DirEntry dir;
    std::list<std::string> path_entries;
    EntrySet entries(Entry::cmp_entry);

    // tokenize a path string to list of named entries
    _tokenize_path(path, path_entries);

    // find a valid end point of the path of named entries
    dir = _parse_dir_entries(path_entries);

    // get an ordered set of Entry by comparator
    _entries_at(dir, entries);

    // find max name column size
    if(is_details)
        _find_entries_len_details(entries, max_name_len, max_byte_len);

    for(auto it = entries.begin(); it != entries.end(); ++it) {
        outs << Ansi(BOLD) << Ansi(BLUE);

        if(is_details && it->type() == Entry::DIR) outs << Ansi(REVERSE);

        outs << it->name() << Ansi(RESET);

        if(is_details)
            outs << std::setw(max_name_len - it->name().size() + 1) << ' ';

        if(is_details) {
            outs << std::right << std::setw(max_byte_len) << it->size();

            tm_info = std::localtime(it->last_modified_ptr());
            outs << ' ' << std::put_time(tm_info, "%b %d %H:%M");
        }
        auto next = it;
        if(++next != entries.end()) outs << '\n';
    }
}

void FatFS::remove() {
    if(_disk) _disk->remove();
    _fat.remove();
    _name.clear();
    _root = _current = DirEntry();
    _logical_blocks = 0;
    _block_offset = 0;
}

void FatFS::set_name(std::string name) { _name = name; }

DirEntry FatFS::add_dir(std::string path) {
    DirEntry dir, added_dir;
    std::list<std::string> entries;
    std::string add_name;

    // tokenize a path string to list of named entries
    _tokenize_path(path, entries);

    // remove last named entry as our dir name to add
    add_name = entries.back();
    entries.pop_back();

    // find a valid end point of the path of named entries
    dir = _parse_dir_entries(entries);

    try {
        if(dir) {
            added_dir = _add_dir_at(dir, add_name);

            if(added_dir)
                return added_dir;
            else
                throw std::invalid_argument("File or directory exists");
        } else
            throw std::invalid_argument("No such directory");
    } catch(const std::exception &e) {
        throw;
    }
}

FileEntry FatFS::add_file(std::string path) {
    DirEntry dir;
    FileEntry added_file;
    std::list<std::string> entries;
    std::string add_name;

    // tokenize a path string to list of named entries
    _tokenize_path(path, entries);

    // remove last named entry as our dir name to add
    add_name = entries.back();
    entries.pop_back();

    // find a valid end point of the path of named entries
    dir = _parse_dir_entries(entries);

    try {
        if(dir) {
            added_file = _add_file_at(dir, add_name);

            if(added_file)
                return added_file;
            else
                throw std::invalid_argument("File or directory exists");
        } else
            throw std::invalid_argument("No such directory");
    } catch(const std::exception &e) {
        throw;
    }
}

bool FatFS::delete_dir(std::string path) {
    DirEntry dir;
    std::list<std::string> entries;
    std::string remove_name;

    // tokenize a path string to list of named entries
    _tokenize_path(path, entries);

    // remove last named entry as our dir name to add
    remove_name = entries.back();
    entries.pop_back();

    // find a valid end point of the path of named entries
    dir = _parse_dir_entries(entries);

    return _delete_dir_at_dir(dir, remove_name);
}

bool FatFS::delete_file(std::string path) {
    DirEntry dir;
    std::list<std::string> entries;
    std::string remove_name;

    // tokenize a path string to list of named entries
    _tokenize_path(path, entries);

    // remove last named entry as our dir name to add
    remove_name = entries.back();
    entries.pop_back();

    // find a valid end point of the path of named entries
    dir = _parse_dir_entries(entries);

    return _delete_file_at_dir(dir, remove_name);
}

bool FatFS::change_dir(std::string path) {
    DirEntry dir;
    std::list<std::string> entries;

    // tokenize a path string to list of named entries
    _tokenize_path(path, entries);

    // find a valid end point of the path of named entries
    dir = _parse_dir_entries(entries);

    if(dir) {
        _current = dir;
        _current.update_last_accessed();

        return true;
    } else
        return false;
}

FileEntry FatFS::find_file(std::string path) const {
    DirEntry dir;
    std::list<std::string> entries;
    std::string find_name;

    // tokenize a path string to list of named entries
    _tokenize_path(path, entries);

    // remove last named entry as our dir name to add
    find_name = entries.back();
    entries.pop_back();

    // find a valid end point of the path of named entries
    dir = _parse_dir_entries(entries);

    return _find_file_at(dir, find_name);
}

std::size_t FatFS::read_file_data(FileEntry &file_entry, char *data,
                                  std::size_t size) const {
    std::size_t bytes = 0;
    FatCell datacell;
    DataEntry data_entry;

    if(!_disk) throw std::runtime_error("No disk or filesystem");

    if(file_entry) {
        std::size_t max_block = _disk->max_block();
        file_entry.update_last_accessed();

        if(file_entry.has_data()) {
            // get first data entry from file entry data pointer
            data_entry = _disk->file_at(file_entry.data_head());
            bytes = data_entry.read(data, size, max_block);

            // get next data block
            datacell = _fat.get_cell(file_entry.data_head());

            // read the rest of the data entry links
            while(datacell.has_next()) {
                data_entry = _disk->file_at(datacell.cell());
                bytes += data_entry.read(data + bytes, size, max_block);

                // get next data block
                datacell = _fat.get_cell(datacell.cell());
            }
            return bytes;
        } else
            return 0;
    } else
        return 0;
}

std::size_t FatFS::write_file_data(FileEntry &file_entry, const char *data,
                                   std::size_t size) {
    int freeindex, bytes_to_write = size, blocks = 0;
    std::size_t bytes = 0;
    FatCell freecell, prevcell;
    DataEntry data_entry;
    std::set<int>::iterator it;

    if(!_disk) throw std::runtime_error("No disk or filesystem");

    if(_fat.full()) throw std::runtime_error("Disk size full");

    if(file_entry) {
        if(size > (file_entry.size() + _fat.size() * _disk->max_block()))
            throw std::runtime_error("Not enough space to write data");

        std::size_t prev_file_size = file_entry.size();
        std::size_t max_block = _disk->max_block();

        // update file entry timestamps
        file_entry.update_last_accessed();
        file_entry.update_last_modified();

        // free all associated data blocks before writing
        _free_file_data_blocks(file_entry);

        // write first block of data and connect to file_entry's data pointer

        // get a free cell to start writing
        it = _fat.free_blocks().begin();
        freeindex = *it;
        _fat.free_blocks().erase(it);
        freecell = _fat.get_cell(freeindex);
        freecell.set_next_cell(FatCell::END);

        // connect file_entry's data pointer to freeindex
        file_entry.set_data_head(freeindex);

        // get DataEntry with freecell's block pointer
        data_entry = _disk->file_at(freeindex);

        // write first data block
        bytes = data_entry.write(data, bytes_to_write, max_block);
        bytes_to_write -= bytes;
        data += bytes;
        blocks += 1;

        // write rest of data to data links
        while(bytes_to_write > 0) {
            // store previous cell
            prevcell = freecell;

            // get a free cell to start writing
            it = _fat.free_blocks().begin();
            freeindex = *it;
            _fat.free_blocks().erase(it);
            freecell = _fat.get_cell(freeindex);
            freecell.set_next_cell(FatCell::END);

            // connect previous cell to freeindex
            prevcell.set_next_cell(freeindex);

            // get DataEntr with freecell's block pointer
            data_entry = _disk->file_at(freeindex);

            // write data blocks
            bytes = data_entry.write(data, bytes_to_write, max_block);
            bytes_to_write -= bytes;
            data += bytes;
            blocks += 1;
        }

        // update file entry size for data
        file_entry.set_data_size(size);
        file_entry.set_size(128 + blocks * _disk->max_block());

        // update parents size
        _update_parents_size(DirEntry(_disk->file_at(file_entry.dotdot())),
                             file_entry.size() - prev_file_size);

        return size;
    } else
        return 0;
}

std::size_t FatFS::append_file_data(FileEntry &file_entry, const char *data,
                                    std::size_t size) {
    int freeindex, bytes_to_write = size, blocks = 0;
    std::size_t bytes = 0;
    FatCell nextcell, prevcell, sec_last;
    DataEntry data_entry;
    std::set<int>::iterator it;

    if(!_disk) throw std::runtime_error("No disk or filesystem");

    if(_fat.full()) throw std::runtime_error("Disk size full");

    if(file_entry) {
        if(size > _fat.size() * _disk->max_block())
            throw std::runtime_error("Not enough space to write data");

        std::size_t prev_file_size = file_entry.size();
        std::size_t max_block = _disk->max_block();

        // update file entry timestamps
        file_entry.update_last_accessed();
        file_entry.update_last_modified();

        // find offset
        std::size_t append =
            file_entry.size() - _disk->max_block() - file_entry.data_size();

        // if append is 0 size, then the last block is full
        // so write a new data block
        if(append == 0) {
            // get a free cell to start writing
            it = _fat.free_blocks().begin();
            freeindex = *it;
            _fat.free_blocks().erase(it);
            nextcell = _fat.get_cell(freeindex);
            nextcell.set_next_cell(FatCell::END);

            // connect file_entry's da{ta pointer to freeindex
            if(file_entry.has_data())
                _last_datacell_from_file(file_entry).set_next_cell(freeindex);
            else
                file_entry.set_data_head(freeindex);

            // get DataEntry with nextcell's block pointer
            data_entry = _disk->file_at(freeindex);

            // write first data block
            bytes = data_entry.write(data, bytes_to_write, max_block);
            bytes_to_write -= bytes;
            data += bytes;
            blocks += 1;
        } else {
            // get second last FatCell from file to get last index
            sec_last = _sec_last_cell_from_cell(file_entry.data_head());

            if(sec_last.has_next()) {
                nextcell = _fat.get_cell(sec_last.cell());
                data_entry = _disk->file_at(sec_last.cell());
            } else {  // data_head is the only entry
                nextcell = _fat.get_cell(file_entry.data_head());
                data_entry = _disk->file_at(file_entry.data_head());
            }

            // get offset to continue writing from last non-nul char
            std::size_t offset = _disk->max_block() - append;

            // append data to this data entry
            bytes = data_entry.append(data, bytes_to_write, offset, max_block);
            bytes_to_write -= bytes;
            data += bytes;
        }

        // write rest of data to data links
        while(bytes_to_write > 0) {
            // store previous cell
            prevcell = nextcell;

            // get a free cell to start writing
            it = _fat.free_blocks().begin();
            freeindex = *it;
            _fat.free_blocks().erase(it);
            nextcell = _fat.get_cell(freeindex);
            nextcell.set_next_cell(FatCell::END);

            // connect previous cell to freeindex
            prevcell.set_next_cell(freeindex);

            // get DataEntr with nextcell's block pointer
            data_entry = _disk->file_at(freeindex);

            // write data blocks
            bytes = data_entry.write(data, bytes_to_write, max_block);
            bytes_to_write -= bytes;
            data += bytes;
            blocks += 1;
        }

        // update file entry size for data
        file_entry.inc_data_size(size);
        file_entry.inc_size(blocks * _disk->max_block());

        // update parents size
        _update_parents_size(DirEntry(_disk->file_at(file_entry.dotdot())),
                             file_entry.size() - prev_file_size);

        return size;
    } else
        return 0;
}

void FatFS::_init_root() {
    // index of root starts @ block offset

    if(_disk) {
        // get fatcell
        FatCell root_cell = _fat.get_cell(_block_offset);

        // set root entry at disk block of index
        _root.set_address(_disk->file_at(_block_offset));

        // if root cell is free, then add Directory entry to block 0 on disk
        if(root_cell.free()) {
            // set root cell
            root_cell.set_next_cell(FatCell::END);

            // initialize root block
            _root.init();                        // init default values
            _root.set_name("/");                 // root name
            _root.set_dot(_block_offset);        // set start of logical block
            _root.set_size(_disk->max_block());  // size to one disk block

            // sanity check that top block is same as offset
            auto it = _fat.free_blocks().begin();
            if(_block_offset != *it)
                throw std::logic_error("Fat data does not match block offset");

            // pop free list
            _fat.free_blocks().erase(it);
        }
        _current = _root;
    }
}

// DirEntry target: add Entry to this directory
// return invalid Entry if can not add
DirEntry FatFS::_add_dir_at(DirEntry &target, std::string name) {
    DirEntry dir, find_dir;
    FileEntry find_file;
    std::set<int>::iterator it;

    if(!_disk) throw std::runtime_error("No disk or filesystem");

    if(_fat.full()) throw std::runtime_error("Disk size full");

    if(name.size() > Entry::MAX_NAME - 1)
        throw std::length_error("File name size exceeded: " +
                                std::to_string(Entry::MAX_NAME - 1));

    if(target) {
        // find if file name exists
        find_file = _find_file_at(target, name);

        if(!find_file) {
            // check if dir name exists
            find_dir = _find_dir_orlast_at(target, name);

            if(find_dir && find_dir.name() == name)
                return dir;
            else {
                // get a new index from free index list
                it = _fat.free_blocks().begin();
                int new_index = *it;
                _fat.free_blocks().erase(it);

                // get a new cell from free index
                FatCell newcell = _fat.get_cell(new_index);

                // mark new cell's cell pointer to END
                // mark disk block at free index
                newcell.set_next_cell(FatCell::END);

                // free index also indicates free block in disk
                // get directory entry at block and update values
                dir = DirEntry(_disk->file_at(new_index));
                dir.init();                        // init default values
                dir.set_name(name);                // set dir name
                dir.set_dot(new_index);            // set self index
                dir.set_dotdot(target.dot());      // set parent index
                dir.set_size(_disk->max_block());  // size to disk block

                // update last cell pointer
                if(target.has_dirs()) {
                    _fat.get_cell(find_dir.dot()).set_next_cell(new_index);
                } else
                    target.set_dir_head(new_index);  // update dir ptr

                // update target timestamp
                target.update_last_modified();

                // update parents size
                _update_parents_size(target, dir.size());
            }
        }
    }
    return dir;
}

// DirEntry target: add Entry to this directory
// return invalid Entry if can not add
FileEntry FatFS::_add_file_at(DirEntry &target, std::string name) {
    DirEntry find_dir;
    FileEntry file, find_file;
    std::set<int>::iterator it;

    if(!_disk) throw std::runtime_error("No disk or filesystem");

    if(_fat.full()) throw std::runtime_error("Disk size full");

    if(name.size() > Entry::MAX_NAME - 1)
        throw std::range_error("File name size exceeded: " +
                               std::to_string(Entry::MAX_NAME - 1));

    if(target) {
        // check if dir name exists
        find_dir = _find_dir_at(target, name);

        if(!find_dir) {
            // check if file name exists
            find_file = _find_file_orlast_at(target, name);

            if(find_file && find_file.name() == name)
                return file;
            else {  // get a new index from free index list
                it = _fat.free_blocks().begin();
                int new_index = *it;
                _fat.free_blocks().erase(it);

                // get a new cell from free index
                FatCell newcell = _fat.get_cell(new_index);

                // mark new cell's cell pointer to END
                // mark disk block at free index
                newcell.set_next_cell(FatCell::END);

                // free index also indicates free block in disk
                // get file entry at block and update values
                file = FileEntry(_disk->file_at(new_index));
                file.init();                        // init default values
                file.set_name(name);                // set file name
                file.set_dot(new_index);            // set self index
                file.set_dotdot(target.dot());      // set parent index
                file.set_size(_disk->max_block());  // size to one disk block

                // update last cell pointer
                if(target.has_files())
                    _last_filecell_from_dir(target).set_next_cell(new_index);
                else
                    // update file ptr
                    target.set_file_head(new_index);

                // update target timestamps
                target.update_last_modified();

                // update parents size
                _update_parents_size(target, file.size());
            }
        }
    }
    return file;
}

void FatFS::_entries_at(DirEntry &dir_entry, EntrySet &entries_set) const {
    FatCell dircell;
    FatCell filecell;

    // clear directory if not empty
    entries_set.clear();

    // iterate DirEntry linked list
    if(_disk && dir_entry && dir_entry.has_dirs()) {
        entries_set.emplace(_disk->file_at(dir_entry.dir_head()));
        dircell = _fat.get_cell(dir_entry.dir_head());

        while(dircell.has_next()) {
            entries_set.emplace(_disk->file_at(dircell.cell()));
            dircell = _fat.get_cell(dircell.cell());
        }
    }

    // iterate FileEntry linked list
    if(_disk && dir_entry && dir_entry.has_files()) {
        entries_set.emplace(_disk->file_at(dir_entry.file_head()));
        filecell = _fat.get_cell(dir_entry.file_head());

        while(filecell.has_next()) {
            entries_set.emplace(_disk->file_at(filecell.cell()));
            filecell = _fat.get_cell(filecell.cell());
        }
    }
}

// dir_entry: directory entry to start looking at
// entries_set: set of ordered entires by comparator
void FatFS::_dirs_at(DirEntry &dir_entry, DirSet &entries_set) const {
    FatCell dircell;

    // clear directory if not empty
    entries_set.clear();

    if(_disk && dir_entry && dir_entry.has_dirs()) {
        entries_set.emplace(_disk->file_at(dir_entry.dir_head()));
        dircell = _fat.get_cell(dir_entry.dir_head());

        while(dircell.has_next()) {
            entries_set.emplace(_disk->file_at(dircell.cell()));
            dircell = _fat.get_cell(dircell.cell());
        }
    }
}

// dir_entry: directory entry to start looking at
// entries_set: set of ordered entires by comparator
void FatFS::_files_at(DirEntry &dir_entry, FileSet &entries_set) const {
    FatCell filecell;

    // clear directory if not empty
    entries_set.clear();

    if(_disk && dir_entry && dir_entry.has_files()) {
        entries_set.emplace(_disk->file_at(dir_entry.file_head()));
        filecell = _fat.get_cell(dir_entry.file_head());

        while(filecell.has_next()) {
            entries_set.emplace(_disk->file_at(filecell.cell()));
            filecell = _fat.get_cell(filecell.cell());
        }
    }
}

// dir_entry: directory entry to start looking at
DirEntry FatFS::_find_dir_at(DirEntry &dir_entry, std::string name) const {
    DirEntry dir, found;
    FatCell dircell;

    if(_disk && dir_entry) {
        if(name == ".") {
            found = dir_entry;
        } else if(name == "..") {
            int parent_block = dir_entry.dotdot();

            if(parent_block != FatCell::END)
                found = DirEntry(_disk->file_at(parent_block));
            else
                found = dir_entry;
        } else if(dir_entry.has_dirs()) {
            dir = DirEntry(_disk->file_at(dir_entry.dir_head()));
            dircell = _fat.get_cell(dir_entry.dir_head());

            if(dir.name() != name) {
                while(dircell.has_next()) {
                    dir = DirEntry(_disk->file_at(dircell.cell()));
                    dircell = _fat.get_cell(dircell.cell());

                    if(dir.name() == name) {
                        found = dir;
                        break;
                    }
                }
            } else
                found = dir;
        }
    }
    return found;
}

// dir_entry: directory entry to start looking at
FileEntry FatFS::_find_file_at(DirEntry &dir_entry, std::string name) const {
    FileEntry file, found;
    FatCell filecell;

    if(_disk && dir_entry && dir_entry.has_files()) {
        file = FileEntry(_disk->file_at(dir_entry.file_head()));
        filecell = _fat.get_cell(dir_entry.file_head());

        if(file.name() != name) {
            while(filecell.has_next()) {
                file = FileEntry(_disk->file_at(filecell.cell()));
                filecell = _fat.get_cell(filecell.cell());

                if(file.name() == name) {
                    found = file;
                    break;
                }
            }
        } else
            found = file;
    }
    return found;
}

// dir_entry: directory entry to start looking at
DirEntry FatFS::_find_dir_orlast_at(DirEntry &dir_entry,
                                    std::string name) const {
    DirEntry dir;
    FatCell dircell;

    if(_disk && dir_entry) {
        if(name == ".") {
            dir = dir_entry;
        } else if(name == "..") {
            int parent_block = dir_entry.dotdot();

            if(parent_block != FatCell::END)
                dir = DirEntry(_disk->file_at(parent_block));
            else
                dir = dir_entry;
        } else if(dir_entry.has_dirs()) {
            dir = DirEntry(_disk->file_at(dir_entry.dir_head()));
            dircell = _fat.get_cell(dir_entry.dir_head());

            if(dir.name() != name)
                while(dircell.has_next()) {
                    dir = DirEntry(_disk->file_at(dircell.cell()));
                    dircell = _fat.get_cell(dircell.cell());

                    if(dir.name() == name) break;
                }
        }
    }
    return dir;
}

// dir_entry: directory entry to start looking at
FileEntry FatFS::_find_file_orlast_at(DirEntry &dir_entry,
                                      std::string name) const {
    FileEntry file;
    FatCell filecell;

    if(_disk && dir_entry && dir_entry.has_files()) {
        file = FileEntry(_disk->file_at(dir_entry.file_head()));
        filecell = _fat.get_cell(dir_entry.file_head());

        if(file.name() != name)
            while(filecell.has_next()) {
                file = FileEntry(_disk->file_at(filecell.cell()));
                filecell = _fat.get_cell(filecell.cell());

                if(file.name() == name) break;
            }
    }
    return file;
}

bool FatFS::_delete_dir_at_dir(DirEntry &dir_entry, std::string name) {
    bool is_deleted = false;
    DirEntry dir;
    FatCell dircell, prevcell;
    std::size_t prev_size = 0;

    if(_disk && dir_entry && dir_entry.has_dirs()) {
        dir = DirEntry(_disk->file_at(dir_entry.dir_head()));
        dircell = _fat.get_cell(dir_entry.dir_head());

        // found @ first link, popfront
        if(dir.name() == name) {
            prev_size = dir.size();

            dir_entry.update_last_modified();
            dir_entry.set_dir_head(dircell.cell());

            _free_cell(dircell, dir.dot());
            _free_dir_contents(dir);  // recursively free dir

            // update parents' size
            _update_parents_size(dir_entry, -prev_size);

            is_deleted = true;
        }
        // found @ rest of link
        else {
            while(dircell.has_next()) {
                prevcell = dircell;
                dir = DirEntry(_disk->file_at(dircell.cell()));
                dircell = _fat.get_cell(dircell.cell());

                if(dir.name() == name) {
                    prev_size = dir.size();

                    dir_entry.update_last_modified();

                    // link previous cell to next cell
                    prevcell.set_next_cell(dircell.cell());

                    _free_cell(dircell, dir.dot());
                    _free_dir_contents(dir);  // recursively free dir

                    // update parents' size
                    _update_parents_size(dir_entry, -prev_size);

                    is_deleted = true;

                    break;
                }
            }
        }
    }
    return is_deleted;
}

bool FatFS::_delete_file_at_dir(DirEntry &dir_entry, std::string name) {
    bool is_deleted = false;
    FileEntry file;
    FatCell filecell, prevcell;
    std::size_t prev_size = 0;

    if(_disk && dir_entry && dir_entry.has_files()) {
        file = FileEntry(_disk->file_at(dir_entry.file_head()));
        filecell = _fat.get_cell(dir_entry.file_head());

        // found @ first link, popfront
        if(file.name() == name) {
            prev_size = file.size();

            dir_entry.update_last_modified();

            // set directory entry's filecell pointer to next cell
            dir_entry.set_file_head(filecell.cell());

            // free filecell and data blocks for this file
            _free_cell(filecell, file.dot());
            _free_file_data_blocks(file);

            // update parents' size
            _update_parents_size(dir_entry, -prev_size);

            is_deleted = true;
        }
        // found @ rest of link
        else {
            while(filecell.has_next()) {
                prevcell = filecell;
                file = FileEntry(_disk->file_at(filecell.cell()));
                filecell = _fat.get_cell(filecell.cell());

                if(file.name() == name) {
                    prev_size = file.size();

                    dir_entry.update_last_modified();

                    // link previous cell to next cell
                    prevcell.set_next_cell(filecell.cell());

                    // free filecell data blocks for this file
                    _free_cell(filecell, file.dot());
                    _free_file_data_blocks(file);

                    // update parents' size
                    _update_parents_size(dir_entry, -prev_size);

                    is_deleted = true;

                    break;
                }
            }
        }
    }
    return is_deleted;
}

void FatFS::_free_dir_contents(DirEntry &dir_entry) {
    if(_disk && dir_entry) {
        while(dir_entry.has_dirs()) {
            // get sub directories
            DirEntry dir = DirEntry(_disk->file_at(dir_entry.dir_head()));
            FatCell dircell = _fat.get_cell(dir_entry.dir_head());

            // set directory entry's dir cell pointer to next cell
            dir_entry.set_dir_head(dircell.cell());

            _free_cell(dircell, dir.dot());  // free dircell
            _free_dir_contents(dir);         // recursively free dir's contents
        }

        while(dir_entry.has_files()) {
            // get FileEntry
            FileEntry file = FileEntry(_disk->file_at(dir_entry.file_head()));
            FatCell filecell = _fat.get_cell(dir_entry.file_head());

            // set diretory entry's file cell pointer to next cell
            dir_entry.set_file_head(filecell.cell());

            // free filecell and data blocks associated with FileEntry
            _free_cell(filecell, file.dot());
            _free_file_data_blocks(file);
        }
    }
}

void FatFS::_free_file_data_blocks(FileEntry &file_entry) {
    int data_head = FatCell::END;
    FatCell datacell;

    while(file_entry && file_entry.has_data()) {
        // get datacell from data pointer in FileEntry
        data_head = file_entry.data_head();
        datacell = _fat.get_cell(file_entry.data_head());

        // relink FileEntry data pointer to next cell
        file_entry.set_data_head(datacell.cell());

        // free datacell
        _free_cell(datacell, data_head);
    }
    file_entry.set_size(_disk->max_block());
}

void FatFS::_free_cell(FatCell &cell, int cell_index) {
    // add this cell to free list
    _fat.free_blocks().emplace(cell_index);

    // mark this cell as free
    cell.set_free();
}

void FatFS::_update_parents_size(DirEntry dir_entry, std::size_t size) {
    while(dir_entry) {
        dir_entry.inc_size(size);
        dir_entry = DirEntry(_disk->file_at(dir_entry.dotdot()));
    }
}

FatCell FatFS::_last_dircell_from_dir(DirEntry &dir_entry) const {
    return _last_cell_from_cell(dir_entry.dir_head());
}

FatCell FatFS::_last_filecell_from_dir(DirEntry &dir_entry) const {
    return _last_cell_from_cell(dir_entry.file_head());
}

FatCell FatFS::_last_datacell_from_file(FileEntry &file_entry) const {
    return _last_cell_from_cell(file_entry.data_head());
}

FatCell FatFS::_last_cell_from_cell(int start_cell) const {
    FatCell current = _fat.get_cell(start_cell);

    while(current.has_next()) current = _fat.get_cell(current.cell());

    return current;
}

FatCell FatFS::_sec_last_cell_from_cell(int start_cell) const {
    FatCell current, prev;
    current = prev = _fat.get_cell(start_cell);

    while(current.has_next()) {
        prev = current;
        current = _fat.get_cell(current.cell());
    }
    return prev;
}

void FatFS::_tokenize_path(std::string path,
                           std::list<std::string> &entries) const {
    char *token = nullptr;

    while(!entries.empty()) entries.pop_back();

    if(!path.empty()) {
        char *cstr = new char[path.length() + 1]();
        strncpy(cstr, path.c_str(), path.size());

        if(cstr[0] == '/') {
            entries.emplace_back("/");
            token = strtok(cstr + 1, "/");
        } else
            token = strtok(cstr, "/");

        while(token != NULL) {
            entries.emplace_back(token);
            token = strtok(NULL, "/");
        }
        delete[] cstr;
    }
}

DirEntry FatFS::_parse_dir_entries(std::list<std::string> &entries) const {
    DirEntry dir = _current;
    std::string entry_name;

    while(!entries.empty()) {
        entry_name = entries.front();
        entries.pop_front();

        if(entry_name == "/")
            dir = _root;
        else if(entry_name == ".") {
            // do nothing
        } else if(entry_name == "..") {
            if(dir && dir.has_parent())
                dir = DirEntry(_disk->file_at(dir.dotdot()));
        } else {
            dir = _find_dir_at(dir, entry_name);

            if(!dir) break;
        }
    }
    return dir;
}

void FatFS::_find_entries_len_details(EntrySet &entries_set,
                                      std::size_t &name_len,
                                      std::size_t &byte_len) const {
    std::size_t name_size = 0, byte_size = 0;

    for(const Entry &d : entries_set) {
        name_size = d.name().size();
        byte_size = std::to_string(d.size()).size();

        if(name_len < name_size) name_len = name_size;
        if(byte_len < byte_size) byte_len = byte_size;
    }
}

void FatFS::_find_entries_len_details(DirSet &entries_set,
                                      std::size_t &name_len,
                                      std::size_t &byte_len) const {
    std::size_t name_size = 0, byte_size = 0;

    for(const DirEntry &d : entries_set) {
        name_size = d.name().size();
        byte_size = std::to_string(d.size()).size();

        if(name_len < name_size) name_len = name_size;
        if(byte_len < byte_size) byte_len = byte_size;
    }
}

void FatFS::_find_entries_len_details(FileSet &entries_set,
                                      std::size_t &name_len,
                                      std::size_t &byte_len) const {
    std::size_t name_size = 0, byte_size = 0;

    for(const FileEntry &d : entries_set) {
        name_size = d.name().size();
        byte_size = std::to_string(d.size()).size();

        if(name_len < name_size) name_len = name_size;
        if(byte_len < byte_size) byte_len = byte_size;
    }
}

}  // namespace fs
