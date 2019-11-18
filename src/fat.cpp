#include "../include/fat.h"

namespace fs {

Fat::Fat(std::string name, int cells)
    : _name(name),
      _fatname(_name + ".fat"),
      _cells(cells),
      _file(nullptr),
      _fd(-1),
      _bytes(cells * FatCell::SIZE) {}

Fat::~Fat() {
    _unmap_file();  // unmap virtual memory from file
    _close_fd();    // close file descriptor
}

bool Fat::create() {
    bool is_created = false;
    struct stat sb;  // file stats
    _bytes = _cells * FatCell::SIZE;

    // if file doesn't exist
    if(stat(_fatname.c_str(), &sb) != 0) {
        // open and create file
        _fd = open(_fatname.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

        if(_fd == -1)
            throw std::runtime_error("Error creating Fat file: create");

        // zero fill file of size bytes
        bool free = FatCell::FREE;
        int cell = ENTRY::ENDBLOCK;
        for(int i = 0; i < _cells; ++i) {
            write(_fd, (bool *)&free, sizeof(free));
            write(_fd, (int *)&cell, sizeof(cell));
        }

        // check if file stats is consisent
        if(fstat(_fd, &sb) == 0 && sb.st_size != (int)_bytes)
            throw std::runtime_error(
                "Error checking Fat file descriptor: create");

        // map physical file to memory
        _file = (char *)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE,
                             MAP_SHARED, _fd, 0);

        is_created = true;
    }

    return is_created;
}

bool Fat::open_fat(std::string name) {
    bool is_loaded = false;
    std::string fatname = name + ".fat";
    struct stat sb;  // file stats

    // remove this disk if exists
    if(_fd > -1) remove_fat();

    // if file exists
    if(stat(fatname.c_str(), &sb) == 0) {
        // open existing file
        _fd = open(fatname.c_str(), O_RDWR, S_IRUSR | S_IWUSR);

        if(_fd == -1) throw std::runtime_error("Error opening Fat file: open");

        // check if file stats is consisent
        if(fstat(_fd, &sb) == 0 && sb.st_size != (int)_bytes)
            throw std::runtime_error(
                "Error checking Fat file descriptor: open");

        // map physical file to memory
        _file = (char *)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE,
                             MAP_SHARED, _fd, 0);

        _bytes = sb.st_size;
        _cells = _bytes / FatCell::SIZE;
        _name = name;
        _fatname = fatname;

        is_loaded = true;
    }

    return is_loaded;
}

bool Fat::remove_fat() {
    bool is_removed = false;

    _unmap_file();  // unmap virtual memory from file
    _close_fd();    // close file descriptor
    if(remove(_fatname.c_str()) == 0) is_removed = true;  // remove system file

    return is_removed;
}

bool Fat::valid() const { return _file != nullptr && _fd > -1; }

std::string Fat::name() const { return _name; }

std::size_t Fat::size() const { return _cells; }

std::size_t Fat::file_size() const { return _bytes; }

bool Fat::set_name(std::string name) {
    if(!valid()) {
        _name = name;
        _fatname = _name + ".fat";

        return true;
    } else
        return false;
}

void Fat::set_cells(int cells) { _cells = cells; }

FatCell Fat::get_cell(int index) {
    if(index < _cells)
        return FatCell(_file + index * FatCell::SIZE);
    else
        throw std::range_error("Cell index out of bound");
}

void Fat::read_cell(int index, bool &free, int &cell) {
    if(index < _cells) {
        free = *(bool *)_file + index * FatCell::SIZE;
        cell = *(int *)(_file + index * FatCell::SIZE + sizeof(bool));
    } else
        throw std::range_error("Cell index out of bound");
}

void Fat::write_cell(int index, bool free, int cell) {
    if(index < _cells) {
        ((bool *)_file)[index * FatCell::SIZE] = free;
        ((int *)_file)[index * FatCell::SIZE + 1] = cell;
    } else
        throw std::range_error("Cell index out of bound");
}

void Fat::_close_fd() {
    if(_fd > -1) {
        close(_fd);
        _fd = -1;
    }
}

void Fat::_unmap_file() {
    if(_file) {
        munmap(_file, _bytes);
        _file = nullptr;
    }
}

FatFS::FatFS(Disk *disk) : _disk(disk) {}

bool FatFS::set_disk(Disk *disk) {
    if(disk && disk->valid()) {
        _name = disk->name();
        _disk = disk;
        _fat.set_name(disk->name());
        _fat.set_cells(disk->total_blocks());

        return true;
    } else
        return false;
}

bool FatFS::format() {
    if(_disk->valid()) {
        if(!_fat.create()) {
            _fat.open_fat(_name);

            // check if fat file's size equal disk's total blocks
            if(!_fat.valid() || _fat.size() != _disk->total_blocks())
                throw std::runtime_error("ERROR Corrupted fat file");
        }
        _init_root();  // initialize root
        _init_free();  // find list of free cells

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

std::size_t FatFS::size() const { return _disk->logical_bytes(); }

std::size_t FatFS::used_space() const {
    return (_disk->total_blocks() - _free.size()) * _disk->max_block();
}

bool FatFS::full() const { return _free.size() == 0; }

std::size_t FatFS::free_space() const { return _free.size() * Disk::MAX_BLOCK; }

std::string FatFS::info() const {
    return "Disk name: " + _name + '\n' + "Valid: " + std::to_string(valid()) +
           '\n' + size_info();
}

std::string FatFS::size_info() const {
    return "Logical disk size: " + std::to_string(size()) + '\n' +
           "Free space (bytes): " + std::to_string(free_space()) + '\n' +
           "Used space (bytes): " + std::to_string(used_space());
}

DirEntry FatFS::current() const { return _current; }

void FatFS::print_dirs() {
    int block = -1;
    std::stack<int> dir_entry_blocks;

    // get a queue of disk blocks for subdirectories
    _dirs_at(_current, dir_entry_blocks);

    while(!dir_entry_blocks.empty()) {
        block = dir_entry_blocks.top();
        dir_entry_blocks.pop();

        DirEntry dir = DirEntry(_disk->file_at(block));
        std::cout << dir.name() << std::endl;
    }
}

void FatFS::print_dirs_str(std::string &output) {
    int block = -1;
    std::stack<int> dir_entry_blocks;

    // get a queue of disk blocks for subdirectories
    _dirs_at(_current, dir_entry_blocks);

    while(!dir_entry_blocks.empty()) {
        block = dir_entry_blocks.top();
        dir_entry_blocks.pop();

        DirEntry dir = DirEntry(_disk->file_at(block));
        output += dir.name();

        if(!dir_entry_blocks.empty()) output += "\n";
    }
}

void FatFS::print_files() {
    int block = -1;
    std::stack<int> file_entry_blocks;

    // get a queue of disk blocks for subdirectories
    _files_at(_current, file_entry_blocks);

    while(!file_entry_blocks.empty()) {
        block = file_entry_blocks.top();
        file_entry_blocks.pop();

        FileEntry file = FileEntry(_disk->file_at(block));
        std::cout << file.name() << std::endl;
    }
}

void FatFS::print_files_str(std::string &output) {
    int block = -1;
    std::stack<int> file_entry_blocks;

    // get a queue of disk blocks for subdirectories
    _files_at(_current, file_entry_blocks);

    while(!file_entry_blocks.empty()) {
        block = file_entry_blocks.top();
        file_entry_blocks.pop();

        FileEntry file = FileEntry(_disk->file_at(block));
        output += file.name();

        if(!file_entry_blocks.empty()) output += "\n";
    }
}

void FatFS::remove_filesystem() {
    _disk->remove_disk();
    _fat.remove_fat();
    _name.clear();
    _free = std::stack<int>();
}

DirEntry FatFS::add_dir(std::string name) {
    DirEntry dir, found;

    if(name.size() > ENTRY::MAX_NAME - 1)
        throw std::range_error("File name size exceeded: " +
                               std::to_string(ENTRY::MAX_NAME - 1));

    // check if name exists
    found = _find_dir_at(_current, name);

    // check if there's free fat cell
    if(!found && _free.size()) {
        // get a new index from free index list
        int new_index = _free.top();  // get a free cell index
        _free.pop();                  // remove cell index

        // get a new cell from free index
        FatCell newcell = _fat.get_cell(new_index);

        // mark new cell's cell pointer to END and mark disk block at free index
        newcell.set_next_cell(ENTRY::ENDBLOCK);
        newcell.set_used();

        // free index also indicates free block in disk
        // get directory entry at block and update values
        dir = DirEntry(_disk->file_at(new_index));
        dir.init();                      // initialize default values
        dir.set_name(name);              // set diirectory name
        dir.set_dot(new_index);          // set dir's location
        dir.set_dotdot(_current.dot());  // set parent's location

        // update last cell pointer
        if(_current.has_dirs())
            _last_dircell_from_dir_entry(_current).set_next_cell(new_index);
        else
            _current.set_dircell_index(new_index);  // update dir pointer

        _current.update_last_modified();  // update current directory times
    }

    return dir;
}

FileEntry FatFS::add_file(std::string name) {
    FileEntry file, found;

    if(name.size() > ENTRY::MAX_NAME - 1)
        throw std::range_error("File name size exceeded: " +
                               std::to_string(ENTRY::MAX_NAME - 1));

    // check if name exists
    found = _find_file_at(_current, name);

    // check if there's free fat cell
    if(!found && _free.size()) {
        // get a new index from free index list
        int new_index = _free.top();  // get a free cell index
        _free.pop();                  // remove cell index

        // get a new cell from free index
        FatCell newcell = _fat.get_cell(new_index);

        // mark new cell's cell pointer to END and mark disk block at free index
        newcell.set_next_cell(ENTRY::ENDBLOCK);
        newcell.set_used();

        // free index also indicates free block in disk
        // get file entry at block and update values
        file = FileEntry(_disk->file_at(new_index));
        file.init();          // initialize default values
        file.set_name(name);  // set diirectory name
        file.set_dot(new_index);

        // update last cell pointer
        if(_current.has_files())
            _last_filecell_from_dir_entry(_current).set_next_cell(new_index);
        else
            _current.set_filecell_index(new_index);  // update dir pointer

        _current.update_last_modified();  // update current directory times
    }

    return file;
}

bool FatFS::delete_dir(std::string name) {
    return _delete_dir_at_dir(_current, name);
}

bool FatFS::delete_file(std::string name) {
    return _delete_file_at_dir(_current, name);
}

bool FatFS::change_dir(std::string name) {
    DirEntry find = _find_dir_at(_current, name);

    if(find) {
        _current = find;
        _current.update_last_accessed();

        return true;
    } else
        return false;
}

FileEntry FatFS::find_file(std::string name) {
    return _find_file_at(_current, name);
}

std::size_t FatFS::read_file_data(FileEntry &file_entry, char *data,
                                  std::size_t size) {
    std::size_t bytes = 0;
    FatCell datacell;
    DataEntry data_entry;

    file_entry.update_last_accessed();

    if(file_entry.has_data()) {
        // get first data entry from file entry data pointer
        data_entry = _disk->file_at(file_entry.datacell_index());
        bytes = data_entry.read(data, size);

        // get next data block
        datacell = _fat.get_cell(file_entry.datacell_index());

        // read the rest of the data entry links
        while(datacell.has_next()) {
            data_entry = _disk->file_at(datacell.cell());
            bytes += data_entry.read(data + bytes, size);

            // get next data block
            datacell = _fat.get_cell(datacell.cell());
        }

        return bytes;
    } else
        return 0;
}

std::size_t FatFS::write_file_data(FileEntry &file_entry, const char *data,
                                   std::size_t size) {
    int freeindex, bytes_to_write = size;
    std::size_t bytes = 0;
    FatCell freecell, prevcell;
    DataEntry data_entry;

    // check if file system have enough space for new data
    if(size > (this->free_space() + file_entry.data_blocks() * Disk::MAX_BLOCK))
        return bytes;
    else {
        // update file entry timestamps
        file_entry.update_last_accessed();
        file_entry.update_last_modified();

        // free all associated data blocks before writing
        _free_file_data_blocks(file_entry);

        // write first block of data and connect to file_entry's data pointer

        // get a free cell to start writing
        freeindex = _free.top();
        _free.pop();
        freecell = _fat.get_cell(freeindex);
        freecell.set_next_cell(ENTRY::ENDBLOCK);
        freecell.set_used();

        // connect file_entry'd data pointer to freeindex
        file_entry.set_datacell_index(freeindex);

        // get DataEntr with freecell's block pointer
        data_entry = _disk->file_at(freeindex);

        if(bytes_to_write < Disk::MAX_BLOCK) {
            bytes = data_entry.write(data, bytes_to_write);
            bytes_to_write -= bytes_to_write;
        } else {
            bytes = data_entry.write(data, bytes_to_write);
            bytes_to_write -= Disk::MAX_BLOCK;
        }

        // write rest of data to data links
        while(bytes_to_write > 0) {
            // store previous cell
            prevcell = freecell;

            // get a free cell to start writing
            freeindex = _free.top();
            _free.pop();
            freecell = _fat.get_cell(freeindex);
            freecell.set_next_cell(ENTRY::ENDBLOCK);
            freecell.set_used();

            // connect previous cell to freeindex
            prevcell.set_next_cell(freeindex);

            // get DataEntr with freecell's block pointer
            data_entry = _disk->file_at(freeindex);

            if(bytes_to_write < Disk::MAX_BLOCK) {
                bytes += data_entry.write(data + bytes, bytes_to_write);
                bytes_to_write -= bytes_to_write;
            } else {
                bytes += data_entry.write(data + bytes, bytes_to_write);
                bytes_to_write -= Disk::MAX_BLOCK;
            }
        }

        file_entry.set_size(size);  // update file entry size for data

        return size;
    }
}

void FatFS::_init_root() {
    // get fat cell #0
    FatCell root_cell = _fat.get_cell(0);

    // set root entry at disk block #0
    _root.set_address(_disk->file_at(0));

    // if root cell is free, then add Directory entry to block 0 on disk
    if(root_cell.free()) {
        // set root cell
        root_cell.set_next_cell(ENTRY::ENDBLOCK);
        root_cell.set_used();

        // initialize root block
        _root.init();
        _root.set_name("/");
        _root.set_dot(0);  // set current block blocation
    }
    _current = _root;
}

void FatFS::_init_free() {
    FatCell fatcell;

    for(int i = (int)_fat.size() - 1; i >= 0; --i) {
        fatcell = _fat.get_cell(i);

        if(fatcell.free()) _free.push(i);
    }
}

// dir_entry: directory entry to start looking at
// entry_blocks: list of subdirectory blocks to populate
void FatFS::_dirs_at(DirEntry &dir_entry, std::stack<int> &entry_blocks) {
    FatCell dircell;

    // clear directory if not empty
    while(!entry_blocks.empty()) entry_blocks.pop();

    if(dir_entry.has_dirs()) {
        entry_blocks.push(dir_entry.dircell_index());
        dircell = _fat.get_cell(dir_entry.dircell_index());

        while(dircell.has_next()) {
            entry_blocks.push(dircell.cell());
            dircell = _fat.get_cell(dircell.cell());
        }
    }
}

// dir_entry: directory entry to start looking at
// entry_blocks: list of file entry blocks to populate
void FatFS::_files_at(DirEntry &dir_entry, std::stack<int> &entry_blocks) {
    FatCell filecell;

    // clear directory if not empty
    while(!entry_blocks.empty()) entry_blocks.pop();

    if(dir_entry.has_files()) {
        entry_blocks.push(dir_entry.filecell_index());
        filecell = _fat.get_cell(dir_entry.filecell_index());

        while(filecell.has_next()) {
            entry_blocks.push(filecell.cell());
            filecell = _fat.get_cell(filecell.cell());
        }
    }
}

// dir_entry: directory entry to start looking at
DirEntry FatFS::_find_dir_at(DirEntry &dir_entry, std::string name) {
    DirEntry dir, found;
    FatCell dircell;

    if(name == ".") {
        found = dir_entry;
    } else if(name == "..") {
        int parent_block = dir_entry.dotdot();

        if(parent_block != ENDBLOCK)
            found = DirEntry(_disk->file_at(parent_block));
        else
            found = dir_entry;
    } else if(dir_entry.has_dirs()) {
        dir = DirEntry(_disk->file_at(dir_entry.dircell_index()));
        dircell = _fat.get_cell(dir_entry.dircell_index());

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

    return found;
}

// dir_entry: directory entry to start looking at
FileEntry FatFS::_find_file_at(DirEntry &dir_entry, std::string name) {
    FileEntry file, found;
    FatCell filecell;

    if(dir_entry.has_files()) {
        file = FileEntry(_disk->file_at(dir_entry.filecell_index()));
        filecell = _fat.get_cell(dir_entry.filecell_index());

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

bool FatFS::_delete_dir_at_dir(DirEntry &dir_entry, std::string name) {
    bool is_deleted = false;
    DirEntry dir;
    FatCell dircell, prevcell;

    if(dir_entry.has_dirs()) {
        dir = DirEntry(_disk->file_at(dir_entry.dircell_index()));
        dircell = _fat.get_cell(dir_entry.dircell_index());

        // found @ first link, popfront
        if(dir.name() == name) {
            dir_entry.update_last_modified();
            dir_entry.set_dircell_index(dircell.cell());

            _free_cell(dircell, dir.dot());
            _free_dir_contents(dir);  // recursively free dir

            is_deleted = true;
        }
        // found @ rest of link
        else {
            while(dircell.has_next()) {
                prevcell = dircell;
                dir = DirEntry(_disk->file_at(dircell.cell()));
                dircell = _fat.get_cell(dircell.cell());

                if(dir.name() == name) {
                    dir_entry.update_last_modified();

                    // link previous cell to next cell
                    prevcell.set_next_cell(dircell.cell());

                    _free_cell(dircell, dir.dot());
                    _free_dir_contents(dir);  // recursively free dir

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

    if(dir_entry.has_files()) {
        file = FileEntry(_disk->file_at(dir_entry.filecell_index()));
        filecell = _fat.get_cell(dir_entry.filecell_index());

        // found @ first link, popfront
        if(file.name() == name) {
            dir_entry.update_last_modified();

            // set directory entry's filecell pointer to next cell
            dir_entry.set_filecell_index(filecell.cell());

            // free filecell and data blocks for this file
            _free_cell(filecell, file.dot());
            _free_file_data_blocks(file);

            is_deleted = true;
        }
        // found @ rest of link
        else {
            while(filecell.has_next()) {
                prevcell = filecell;
                file = FileEntry(_disk->file_at(filecell.cell()));
                filecell = _fat.get_cell(filecell.cell());

                if(file.name() == name) {
                    dir_entry.update_last_modified();

                    // link previous cell to next cell
                    prevcell.set_next_cell(filecell.cell());

                    // free filecell data blocks for this file
                    _free_cell(filecell, file.dot());
                    _free_file_data_blocks(file);

                    is_deleted = true;

                    break;
                }
            }
        }
    }
    return is_deleted;
}

void FatFS::_free_dir_contents(DirEntry &dir_entry) {
    while(dir_entry.has_dirs()) {
        // get sub directories
        DirEntry dir = DirEntry(_disk->file_at(dir_entry.dircell_index()));
        FatCell dircell = _fat.get_cell(dir_entry.dircell_index());

        // set directory entry's dir cell pointer to next cell
        dir_entry.set_dircell_index(dircell.cell());

        _free_cell(dircell, dir.dot());  // free dircell
        _free_dir_contents(dir);         // recursively free dir's contents
    }

    while(dir_entry.has_files()) {
        // get FileEntry
        FileEntry file = FileEntry(_disk->file_at(dir_entry.filecell_index()));
        FatCell filecell = _fat.get_cell(dir_entry.filecell_index());

        // set diretory entry's file cell pointer to next cell
        dir_entry.set_filecell_index(filecell.cell());

        // free filecell and data blocks associated with FileEntry
        _free_cell(filecell, file.dot());
        _free_file_data_blocks(file);
    }
}

void FatFS::_free_file_data_blocks(FileEntry &file_entry) {
    int datacell_index = ENTRY::ENDBLOCK;
    FatCell datacell;

    while(file_entry.has_data()) {
        // get datacell from data pointer in FileEntry
        datacell_index = file_entry.datacell_index();
        datacell = _fat.get_cell(file_entry.datacell_index());

        // relink FileEntry data pointer to next cell
        file_entry.set_datacell_index(datacell.cell());

        // free datacell
        _free_cell(datacell, datacell_index);
    }
    file_entry.set_size(0);
}

void FatFS::_free_cell(FatCell &cell, int cell_index) {
    // add this cell to free list
    _free.push(cell_index);

    // mark this cell as free
    cell.set_next_cell(ENTRY::ENDBLOCK);
    cell.set_free();
}

FatCell FatFS::_last_dircell_from_dir_entry(DirEntry &dir_entry) {
    return _last_cell_from_cell(dir_entry.dircell_index());
}

FatCell FatFS::_last_filecell_from_dir_entry(DirEntry &dir_entry) {
    return _last_cell_from_cell(dir_entry.filecell_index());
}

FatCell FatFS::_last_datacell_from_file_entry(FileEntry &file_entry) {
    return _last_cell_from_cell(file_entry.datacell_index());
}

FatCell FatFS::_last_cell_from_cell(int start_cell) {
    FatCell current = _fat.get_cell(start_cell);

    while(current.has_next()) current = _fat.get_cell(current.cell());

    return current;
}

}  // namespace fs
