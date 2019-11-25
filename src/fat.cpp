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

Entry::Entry(char *address) { _reset_address(address); }

bool Entry::has_parent() const { return dotdot() != Entry::ENDBLOCK; }

bool Entry::valid() const { return _name != nullptr; }

Entry::operator bool() const { return _name != nullptr; }

std::string Entry::name() const { return std::string(_name); }

bool Entry::type() const { return *_type; }

int Entry::dot() const { return *_dot; }

int Entry::dotdot() const { return *_dotdot; }

int Entry::size() const { return *_size; }

time_t Entry::created() const { return *_created; }

time_t *Entry::created_ptr() const { return _created; }

char *Entry::created_str() const { return std::ctime(_created); }

time_t Entry::last_accessed() const { return *_last_accessed; }

time_t *Entry::last_accessed_ptr() const { return _created; }

char *Entry::last_accessed_str() const { return std::ctime(_last_accessed); }

time_t Entry::last_modified() const { return *_last_modified; }

time_t *Entry::last_modified_ptr() const { return _created; }

char *Entry::last_modified_str() const { return std::ctime(_last_modified); }

void Entry::init() {
    memset(_name, 0, Entry::MAX_NAME);
    set_type(Entry::DIR);
    set_dot(Entry::ENDBLOCK);
    set_dotdot(Entry::ENDBLOCK);
    set_size(0);
    update_created();
    update_last_accessed();
    update_last_modified();
}

void Entry::clear() { _reset_address(nullptr); }

void Entry::set_address(char *address) { _reset_address(address); }

void Entry::set_name(std::string name) {
    memset(_name, 0, Entry::MAX_NAME);

    if(name.size() > Entry::MAX_NAME - 1)
        strncpy(_name, name.c_str(), Entry::MAX_NAME - 1);
    else
        strncpy(_name, name.c_str(), name.size());
}

void Entry::set_type(bool type) { *_type = type; }

void Entry::set_dot(int block) { *_dot = block; }

void Entry::set_dotdot(int block) { *_dotdot = block; }

void Entry::set_size(int size) { *_size = size; }

void Entry::inc_size(int inc) { *_size += inc; }

void Entry::dec_size(int dec) { *_size -= dec; }

void Entry::set_created(time_t t) { *_created = t; }

void Entry::update_created() { *_created = std::time(_created); }

void Entry::set_last_accessed(time_t t) { *_last_accessed = t; }

void Entry::update_last_accessed() {
    *_last_accessed = std::time(_last_accessed);
}

void Entry::set_last_modified(time_t t) { *_last_modified = t; }

void Entry::update_last_modified() {
    *_last_modified = std::time(_last_modified);
}

void Entry::_reset_address(char *address) {
    _name = address;
    _type = (bool *)(_name + Entry::MAX_NAME);
    _dot = (int *)(_type + 1);
    _dotdot = _dot + 1;
    _size = _dotdot + 1;
    _created = (time_t *)(_size + 1);
    _last_accessed = _created + 1;
    _last_modified = _last_accessed + 1;
}

DirEntry::DirEntry(char *address) : Entry(address) { _init_dir(); }

bool DirEntry::has_dirs() const { return dir_head() > Entry::ENDBLOCK; }

bool DirEntry::has_files() const { return file_head() > Entry::ENDBLOCK; }

int DirEntry::dir_head() const { return *_dir_head; }

int DirEntry::file_head() const { return *_file_head; }

void DirEntry::init() {
    Entry::init();
    set_type(Entry::DIR);
    set_dir_head(Entry::ENDBLOCK);
    set_file_head(Entry::ENDBLOCK);
}

void DirEntry::clear() { DirEntry::_reset_address(nullptr); }

void DirEntry::set_address(char *address) { DirEntry::_reset_address(address); }

void DirEntry::set_dir_head(int cell) { *_dir_head = cell; }

void DirEntry::set_file_head(int cell) { *_file_head = cell; }

// set address offsets from Entry's last adddress
void DirEntry::_init_dir() {
    _dir_head = (int *)(_last_modified + 1);
    _file_head = _dir_head + 1;
}

// reset all attribute addresses
void DirEntry::_reset_address(char *address) {
    Entry::_reset_address(address);
    _init_dir();
}

FileEntry::FileEntry(char *address) : Entry(address) { _init_file(); }

bool FileEntry::has_data() const { return data_head() > Entry::ENDBLOCK; }

int FileEntry::data_head() const { return *_data_head; }

int FileEntry::data_size() const { return *_data_size; }

void FileEntry::init() {
    Entry::init();
    set_type(Entry::FILE);
    set_data_head(Entry::ENDBLOCK);
    set_data_size(0);
}

void FileEntry::clear() { FileEntry::_reset_address(nullptr); }

void FileEntry::set_address(char *address) {
    FileEntry::_reset_address(address);
}

void FileEntry::set_data_head(int cell) { *_data_head = cell; }

void FileEntry::set_data_size(int size) { *_data_size = size; }

void FileEntry::inc_data_size(int size) { *_data_size += size; }

void FileEntry::dec_data_size(int size) { *_data_size -= size; }

void FileEntry::_init_file() {
    _data_head = (int *)(_last_modified + 1);
    _data_size = _data_head + 1;
}

void FileEntry::_reset_address(char *address) {
    Entry::_reset_address(address);
    _init_file();
}

DataEntry::DataEntry(char *address) : _data(address) {}

bool DataEntry::valid() const { return _data != nullptr; }

DataEntry::operator bool() const { return _data != nullptr; }

char *DataEntry::data() { return _data; }

void DataEntry::clear(std::size_t size) { memset(_data, 0, size); }

std::size_t DataEntry::read(char *buf, std::size_t size, std::size_t limit) {
    std::size_t bytes = 0;

    while(bytes < limit && _data[bytes] != '\0' && bytes < size) {
        buf[bytes] = _data[bytes];
        ++bytes;
    }

    return bytes;
}

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

FatCell::FatCell(char *address) : _next_cell((int *)(address)) {}

bool FatCell::has_next() const {
    return _next_cell != nullptr && *_next_cell > END;
}

bool FatCell::free() const { return *_next_cell <= FREE; }

bool FatCell::used() const { return *_next_cell > FREE; }

bool FatCell::valid() const { return _next_cell != nullptr; }

FatCell::operator bool() const { return _next_cell != nullptr; }

int FatCell::next_cell() const { return *_next_cell; }

void FatCell::set_free() { *_next_cell = FREE; }

void FatCell::set_next_cell(int c) { *_next_cell = c; }

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
    if(_disk && _disk->valid()) {
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
                _root = _current = DirEntry(_disk->data_at(block_offset));

                return true;
            } else
                return false;
        } else
            return false;
    } else
        return false;
}

bool FatFS::format() {
    if(_disk && _disk->valid()) {
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

void FatFS::remove_file_data(FileEntry &file) {
    file.update_last_modified();
    _free_data_at(file);
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
            dir = DirEntry(_disk->data_at(dir.dotdot()));

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

    return _delete_dir_at(dir, remove_name);
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

    return _delete_file_at(dir, remove_name);
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

std::size_t FatFS::read_file_data(FileEntry &file, char *data,
                                  std::size_t size) const {
    std::size_t bytes = 0;
    FatCell datacell;
    DataEntry data_entry;

    if(!_disk) throw std::runtime_error("No disk or filesystem");

    if(file) {
        std::size_t max_block = _disk->max_block();
        file.update_last_accessed();

        if(file.has_data()) {
            // get first data entry from file's data pointer
            data_entry = _disk->data_at(file.data_head());
            bytes = data_entry.read(data, size, max_block);

            // get next data block
            datacell = _fat.get_cell(file.data_head());

            // read the rest of the data entry links
            while(datacell.has_next()) {
                data_entry = _disk->data_at(datacell.next_cell());
                bytes += data_entry.read(data + bytes, size, max_block);

                // get next data block
                datacell = _fat.get_cell(datacell.next_cell());
            }
            return bytes;
        } else
            return 0;
    } else
        return 0;
}

std::size_t FatFS::write_file_data(FileEntry &file, const char *data,
                                   std::size_t size) {
    int freeindex, bytes_to_write = size, blocks = 0;
    std::size_t bytes = 0;
    FatCell freecell, prevcell;
    DataEntry data_entry;
    std::set<int>::iterator it;

    if(!_disk) throw std::runtime_error("No disk or filesystem");

    if(_fat.full()) throw std::runtime_error("Disk size full");

    if(file) {
        if(size > (file.size() + _fat.size() * _disk->max_block()))
            throw std::runtime_error("Not enough space to write data");

        std::size_t prev_file_size = file.size();
        std::size_t max_block = _disk->max_block();

        // update file entry timestamps
        file.update_last_modified();

        // free all associated data blocks before writing
        _free_data_at(file);

        // write first block of data and connect to file's data pointer

        // get a free cell to start writing
        it = _fat.free_blocks().begin();
        freeindex = *it;
        _fat.free_blocks().erase(it);
        freecell = _fat.get_cell(freeindex);
        freecell.set_next_cell(FatCell::END);

        // connect file's data pointer to freeindex
        file.set_data_head(freeindex);

        // get DataEntry with freeindex
        data_entry = _disk->data_at(freeindex);

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

            // get DataEntry with freeindex
            data_entry = _disk->data_at(freeindex);

            // write data blocks
            bytes = data_entry.write(data, bytes_to_write, max_block);
            bytes_to_write -= bytes;
            data += bytes;
            blocks += 1;
        }

        // update file entry size for data
        file.set_data_size(size);
        file.set_size(128 + blocks * _disk->max_block());

        // update parents size
        _update_parents_size(DirEntry(_disk->data_at(file.dotdot())),
                             file.size() - prev_file_size);

        return size;
    } else
        return 0;
}

std::size_t FatFS::append_file_data(FileEntry &file, const char *data,
                                    std::size_t size) {
    int freeindex, bytes_to_write = size, blocks = 0;
    std::size_t bytes = 0;
    FatCell nextcell, prevcell, sec_last;
    DataEntry data_entry;
    std::set<int>::iterator it;

    if(!_disk) throw std::runtime_error("No disk or filesystem");

    if(_fat.full()) throw std::runtime_error("Disk size full");

    if(file) {
        if(size > _fat.size() * _disk->max_block())
            throw std::runtime_error("Not enough space to write data");

        std::size_t prev_file_size = file.size();
        std::size_t max_block = _disk->max_block();

        // update file entry timestamps
        file.update_last_modified();

        // find offset
        std::size_t append =
            file.size() - _disk->max_block() - file.data_size();

        // if append is 0 size, then the last block is full
        // so write a new data block
        if(append == 0) {
            // get a free cell to start writing
            it = _fat.free_blocks().begin();
            freeindex = *it;
            _fat.free_blocks().erase(it);
            nextcell = _fat.get_cell(freeindex);
            nextcell.set_next_cell(FatCell::END);

            // connect file's data pointer to freeindex
            if(file.has_data())
                _last_datacell_from(file).set_next_cell(freeindex);
            else
                file.set_data_head(freeindex);

            // get DataEntry with freeindex
            data_entry = _disk->data_at(freeindex);

            // write first data block
            bytes = data_entry.write(data, bytes_to_write, max_block);
            bytes_to_write -= bytes;
            data += bytes;
            blocks += 1;
        } else {
            // get second last FatCell from file to get last index
            sec_last = _sec_last_cell_from(file.data_head());

            if(sec_last.has_next()) {
                nextcell = _fat.get_cell(sec_last.next_cell());
                data_entry = _disk->data_at(sec_last.next_cell());
            } else {  // data_head is the only entry
                nextcell = _fat.get_cell(file.data_head());
                data_entry = _disk->data_at(file.data_head());
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

            // get DataEntry with freeindex
            data_entry = _disk->data_at(freeindex);

            // write data blocks
            bytes = data_entry.write(data, bytes_to_write, max_block);
            bytes_to_write -= bytes;
            data += bytes;
            blocks += 1;
        }

        // update file entry size for data
        file.inc_data_size(size);
        file.inc_size(blocks * _disk->max_block());

        // update parents size
        _update_parents_size(DirEntry(_disk->data_at(file.dotdot())),
                             file.size() - prev_file_size);

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
        _root.set_address(_disk->data_at(_block_offset));

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

// DirEntry dir: add Entry to this directory
// return invalid Entry if can not add
DirEntry FatFS::_add_dir_at(DirEntry &dir, std::string name) {
    DirEntry newdir, find_dir;
    FileEntry find_file;
    std::set<int>::iterator it;

    if(!_disk) throw std::runtime_error("No disk or filesystem");

    if(_fat.full()) throw std::runtime_error("Disk size full");

    if(name.size() > Entry::MAX_NAME - 1)
        throw std::length_error("File name size exceeded: " +
                                std::to_string(Entry::MAX_NAME - 1));

    if(dir) {
        // find if file name exists
        find_file = _find_file_at(dir, name);

        if(!find_file) {
            // check if dir name exists
            find_dir = _find_dir_orlast_at(dir, name);

            if(find_dir && find_dir.name() == name)
                return newdir;
            else {
                // get a new index from free index list
                it = _fat.free_blocks().begin();
                int newindex = *it;
                _fat.free_blocks().erase(it);

                // get a new cell from free index
                FatCell newcell = _fat.get_cell(newindex);

                // mark new cell's next cell pointer to END
                // mark disk block at free index
                newcell.set_next_cell(FatCell::END);

                // free index also indicates free block in disk
                // get directory entry at block and update values
                newdir = DirEntry(_disk->data_at(newindex));
                newdir.init();                        // init default values
                newdir.set_name(name);                // set dir name
                newdir.set_dot(newindex);             // set self index
                newdir.set_dotdot(dir.dot());         // set parent index
                newdir.set_size(_disk->max_block());  // size 1 disk block

                // update last cell pointer
                if(dir.has_dirs()) {
                    _fat.get_cell(find_dir.dot()).set_next_cell(newindex);
                } else
                    dir.set_dir_head(newindex);  // update dir ptr

                // update dir timestamp
                dir.update_last_modified();

                // update parents size
                _update_parents_size(dir, newdir.size());
            }
        }
    }
    return newdir;
}

// DirEntry dir: add Entry to this directory
// return invalid Entry if can not add
FileEntry FatFS::_add_file_at(DirEntry &dir, std::string name) {
    DirEntry find_dir;
    FileEntry newfile, find_file;
    std::set<int>::iterator it;

    if(!_disk) throw std::runtime_error("No disk or filesystem");

    if(_fat.full()) throw std::runtime_error("Disk size full");

    if(name.size() > Entry::MAX_NAME - 1)
        throw std::range_error("File name size exceeded: " +
                               std::to_string(Entry::MAX_NAME - 1));

    if(dir) {
        // check if dir name exists
        find_dir = _find_dir_at(dir, name);

        if(!find_dir) {
            // check if file name exists
            find_file = _find_file_orlast_at(dir, name);

            if(find_file && find_file.name() == name)
                return newfile;
            else {  // get a new index from free index list
                it = _fat.free_blocks().begin();
                int newindex = *it;
                _fat.free_blocks().erase(it);

                // get a new cell from free index
                FatCell newcell = _fat.get_cell(newindex);

                // mark new cell's cell pointer to END
                // mark disk block at free index
                newcell.set_next_cell(FatCell::END);

                // free index also indicates free block in disk
                // get file entry at block and update values
                newfile = FileEntry(_disk->data_at(newindex));
                newfile.init();                        // init default values
                newfile.set_name(name);                // set file name
                newfile.set_dot(newindex);             // set self index
                newfile.set_dotdot(dir.dot());         // set parent index
                newfile.set_size(_disk->max_block());  // size 1 disk block

                // update last cell pointer
                if(dir.has_files())
                    _last_filecell_from(dir).set_next_cell(newindex);
                else
                    // update file ptr
                    dir.set_file_head(newindex);

                // update dir timestamps
                dir.update_last_modified();

                // update parents size
                _update_parents_size(dir, newfile.size());
            }
        }
    }
    return newfile;
}

void FatFS::_entries_at(DirEntry &dir, EntrySet &entries_set) const {
    FatCell cell;

    // clear directory if not empty
    entries_set.clear();

    // iterate DirEntry linked list
    if(_disk && dir && dir.has_dirs()) {
        entries_set.emplace(_disk->data_at(dir.dir_head()));
        cell = _fat.get_cell(dir.dir_head());

        while(cell.has_next()) {
            entries_set.emplace(_disk->data_at(cell.next_cell()));
            cell = _fat.get_cell(cell.next_cell());
        }
    }

    // iterate FileEntry linked list
    if(_disk && dir && dir.has_files()) {
        entries_set.emplace(_disk->data_at(dir.file_head()));
        cell = _fat.get_cell(dir.file_head());

        while(cell.has_next()) {
            entries_set.emplace(_disk->data_at(cell.next_cell()));
            cell = _fat.get_cell(cell.next_cell());
        }
    }
}

// dir: directory entry to start looking at
// entries_set: set of ordered entires by comparator
void FatFS::_dirs_at(DirEntry &dir, DirSet &entries_set) const {
    FatCell cell;

    // clear directory if not empty
    entries_set.clear();

    if(_disk && dir && dir.has_dirs()) {
        entries_set.emplace(_disk->data_at(dir.dir_head()));
        cell = _fat.get_cell(dir.dir_head());

        while(cell.has_next()) {
            entries_set.emplace(_disk->data_at(cell.next_cell()));
            cell = _fat.get_cell(cell.next_cell());
        }
    }
}

// dir: directory entry to start looking at
// entries_set: set of ordered entires by comparator
void FatFS::_files_at(DirEntry &dir, FileSet &entries_set) const {
    FatCell cell;

    // clear directory if not empty
    entries_set.clear();

    if(_disk && dir && dir.has_files()) {
        entries_set.emplace(_disk->data_at(dir.file_head()));
        cell = _fat.get_cell(dir.file_head());

        while(cell.has_next()) {
            entries_set.emplace(_disk->data_at(cell.next_cell()));
            cell = _fat.get_cell(cell.next_cell());
        }
    }
}

// dir: directory entry to start looking at
DirEntry FatFS::_find_dir_at(DirEntry &dir, std::string name) const {
    DirEntry subdir, found;
    FatCell cell;

    if(_disk && dir) {
        if(name == ".") {
            found = dir;
        } else if(name == "..") {
            int parent_block = dir.dotdot();

            if(parent_block != FatCell::END)
                found = DirEntry(_disk->data_at(parent_block));
            else
                found = dir;
        } else if(dir.has_dirs()) {
            subdir = DirEntry(_disk->data_at(dir.dir_head()));
            cell = _fat.get_cell(dir.dir_head());

            if(subdir.name() != name) {
                while(cell.has_next()) {
                    subdir = DirEntry(_disk->data_at(cell.next_cell()));
                    cell = _fat.get_cell(cell.next_cell());

                    if(subdir.name() == name) {
                        found = subdir;
                        break;
                    }
                }
            } else
                found = subdir;
        }
    }
    return found;
}

// dir: directory entry to start looking at
FileEntry FatFS::_find_file_at(DirEntry &dir, std::string name) const {
    FileEntry file, found;
    FatCell cell;

    if(_disk && dir && dir.has_files()) {
        file = FileEntry(_disk->data_at(dir.file_head()));
        cell = _fat.get_cell(dir.file_head());

        if(file.name() != name) {
            while(cell.has_next()) {
                file = FileEntry(_disk->data_at(cell.next_cell()));
                cell = _fat.get_cell(cell.next_cell());

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

// dir: directory entry to start looking at
DirEntry FatFS::_find_dir_orlast_at(DirEntry &dir, std::string name) const {
    DirEntry subdir;
    FatCell cell;

    if(_disk && dir) {
        if(name == ".") {
            subdir = dir;
        } else if(name == "..") {
            int parent_block = dir.dotdot();

            if(parent_block != FatCell::END)
                subdir = DirEntry(_disk->data_at(parent_block));
            else
                subdir = dir;
        } else if(dir.has_dirs()) {
            subdir = DirEntry(_disk->data_at(dir.dir_head()));
            cell = _fat.get_cell(dir.dir_head());

            if(subdir.name() != name)
                while(cell.has_next()) {
                    subdir = DirEntry(_disk->data_at(cell.next_cell()));
                    cell = _fat.get_cell(cell.next_cell());

                    if(subdir.name() == name) break;
                }
        }
    }
    return subdir;
}

// dir: directory entry to start looking at
FileEntry FatFS::_find_file_orlast_at(DirEntry &dir, std::string name) const {
    FileEntry file;
    FatCell cell;

    if(_disk && dir && dir.has_files()) {
        file = FileEntry(_disk->data_at(dir.file_head()));
        cell = _fat.get_cell(dir.file_head());

        if(file.name() != name)
            while(cell.has_next()) {
                file = FileEntry(_disk->data_at(cell.next_cell()));
                cell = _fat.get_cell(cell.next_cell());

                if(file.name() == name) break;
            }
    }
    return file;
}

bool FatFS::_delete_dir_at(DirEntry &dir, std::string name) {
    bool is_deleted = false;
    DirEntry subdir;
    FatCell cell, prevcell;
    std::size_t prev_size = 0;

    if(_disk && dir && dir.has_dirs()) {
        subdir = DirEntry(_disk->data_at(dir.dir_head()));
        cell = _fat.get_cell(dir.dir_head());

        // found @ first link, popfront
        if(subdir.name() == name) {
            prev_size = subdir.size();

            dir.update_last_modified();
            dir.set_dir_head(cell.next_cell());

            _free_cell(cell, subdir.dot());
            _free_dir_at(subdir);  // recursively free dir

            // update parents' size
            _update_parents_size(dir, -prev_size);

            is_deleted = true;
        }
        // found @ rest of link
        else {
            while(cell.has_next()) {
                prevcell = cell;
                subdir = DirEntry(_disk->data_at(cell.next_cell()));
                cell = _fat.get_cell(cell.next_cell());

                if(subdir.name() == name) {
                    prev_size = subdir.size();

                    dir.update_last_modified();

                    // link previous cell to next cell
                    prevcell.set_next_cell(cell.next_cell());

                    _free_cell(cell, subdir.dot());
                    _free_dir_at(subdir);  // recursively free dir

                    // update parents' size
                    _update_parents_size(dir, -prev_size);

                    is_deleted = true;

                    break;
                }
            }
        }
    }
    return is_deleted;
}

bool FatFS::_delete_file_at(DirEntry &dir, std::string name) {
    bool is_deleted = false;
    FileEntry file;
    FatCell cell, prevcell;
    std::size_t prev_size = 0;

    if(_disk && dir && dir.has_files()) {
        file = FileEntry(_disk->data_at(dir.file_head()));
        cell = _fat.get_cell(dir.file_head());

        // found @ first link, popfront
        if(file.name() == name) {
            prev_size = file.size();

            dir.update_last_modified();

            // set directory file pointer to next cell
            dir.set_file_head(cell.next_cell());

            // free cell and data blocks for this file
            _free_cell(cell, file.dot());
            _free_data_at(file);

            // update parents' size
            _update_parents_size(dir, -prev_size);

            is_deleted = true;
        }
        // found @ rest of link
        else {
            while(cell.has_next()) {
                prevcell = cell;
                file = FileEntry(_disk->data_at(cell.next_cell()));
                cell = _fat.get_cell(cell.next_cell());

                if(file.name() == name) {
                    prev_size = file.size();

                    dir.update_last_modified();

                    // link previous cell to next cell
                    prevcell.set_next_cell(cell.next_cell());

                    // free cell data blocks for this file
                    _free_cell(cell, file.dot());
                    _free_data_at(file);

                    // update parents' size
                    _update_parents_size(dir, -prev_size);

                    is_deleted = true;

                    break;
                }
            }
        }
    }
    return is_deleted;
}

void FatFS::_free_dir_at(DirEntry &dir) {
    if(_disk && dir) {
        DirEntry subdir;
        FileEntry file;
        FatCell cell;

        while(dir.has_dirs()) {
            // get sub directories
            subdir = DirEntry(_disk->data_at(dir.dir_head()));
            cell = _fat.get_cell(dir.dir_head());

            // set directory dir pointer to next cell
            dir.set_dir_head(cell.next_cell());

            _free_cell(cell, subdir.dot());  // free cell
            _free_dir_at(subdir);            // recursively free contents
        }

        while(dir.has_files()) {
            // get FileEntry
            file = FileEntry(_disk->data_at(dir.file_head()));
            cell = _fat.get_cell(dir.file_head());

            // set diretory file pointer to next cell
            dir.set_file_head(cell.next_cell());

            // free cell and data blocks associated with FileEntry
            _free_cell(cell, file.dot());
            _free_data_at(file);
        }
    }
}

void FatFS::_free_data_at(FileEntry &file) {
    int data_head = FatCell::END;
    FatCell cell;

    while(file && file.has_data()) {
        // get cell from data pointer in FileEntry
        data_head = file.data_head();
        cell = _fat.get_cell(data_head);

        // relink file data pointer to next cell
        file.set_data_head(cell.next_cell());

        // free cell
        _free_cell(cell, data_head);
    }
    file.set_size(_disk->max_block());
}

void FatFS::_free_cell(FatCell &cell, int cell_index) {
    // add this cell to free list
    _fat.free_blocks().emplace(cell_index);

    // mark this cell as free
    cell.set_free();
}

void FatFS::_update_parents_size(DirEntry dir, std::size_t size) {
    while(dir) {
        dir.inc_size(size);
        dir = DirEntry(_disk->data_at(dir.dotdot()));
    }
}

FatCell FatFS::_last_dircell_from(DirEntry &dir) const {
    return _last_cell_from(dir.dir_head());
}

FatCell FatFS::_last_filecell_from(DirEntry &dir) const {
    return _last_cell_from(dir.file_head());
}

FatCell FatFS::_last_datacell_from(FileEntry &file) const {
    return _last_cell_from(file.data_head());
}

FatCell FatFS::_last_cell_from(int start_cell) const {
    FatCell current = _fat.get_cell(start_cell);

    while(current.has_next()) current = _fat.get_cell(current.next_cell());

    return current;
}

FatCell FatFS::_sec_last_cell_from(int start_cell) const {
    FatCell current, prev;
    current = prev = _fat.get_cell(start_cell);

    while(current.has_next()) {
        prev = current;
        current = _fat.get_cell(current.next_cell());
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
                dir = DirEntry(_disk->data_at(dir.dotdot()));
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
