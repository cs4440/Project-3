#include "../include/fat.h"

namespace fs {

// ENTRY COMPARATORS
bool cmp_dir_name(const DirEntry &a, const DirEntry &b) {
    return a.name() < b.name();
}

bool cmp_file_name(const FileEntry &a, const FileEntry &b) {
    return a.name() < b.name();
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

bool Fat::load_fat() {
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

bool Fat::valid() const { return _file != nullptr; }

std::size_t Fat::free_size() const { return _free.size(); }

FatCell Fat::get_cell(int index) {
    if((index - _cell_offset) < _cells)
        return FatCell(_file + (index - _cell_offset) * FatCell::SIZE);
    else
        throw std::range_error("Cell index out of bound");
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

bool FatFS::load_disk() {
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

            if(_fat.load_fat()) {
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

std::size_t FatFS::size() const {
    if(_disk)
        return _logical_blocks * _disk->max_block();
    else
        return 0;
}

std::size_t FatFS::used_space() const {
    if(_disk)
        return (_logical_blocks - _fat.free_size()) * _disk->max_block();
    else
        return 0;
}

bool FatFS::full() const { return _fat.free_size() == 0; }

std::size_t FatFS::free_space() const {
    if(_disk)
        return _fat.free_size() * _disk->max_block();
    else
        return 0;
}

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
    DirSet entries(cmp_dir_name);

    // get an ordered set of Entry by comparator
    _dirs_at(_current, entries);

    for(auto it = entries.begin(); it != entries.end(); ++it)
        std::cout << it->name() << '\n';
}

void FatFS::print_dirs_str(std::string &output) {
    DirSet entries(cmp_dir_name);

    // get an ordered set of Entry by comparator
    _dirs_at(_current, entries);

    for(auto it = entries.begin(); it != entries.end(); ++it) {
        output += it->name();

        auto next = it;
        if(++next != entries.end()) output += '\n';
    }
}

void FatFS::print_files() {
    FileSet entries(cmp_file_name);

    // get an ordered set of Entry by comparator
    _files_at(_current, entries);

    for(auto it = entries.begin(); it != entries.end(); ++it)
        std::cout << it->name() << '\n';
}

void FatFS::print_files_str(std::string &output) {
    FileSet entries(cmp_file_name);

    // get an ordered set of Entry by comparator
    _files_at(_current, entries);

    for(auto it = entries.begin(); it != entries.end(); ++it) {
        output += it->name();

        auto next = it;
        if(++next != entries.end()) output += '\n';
    }
}

void FatFS::remove_filesystem() {
    if(_disk) _disk->remove_disk();
    _name.clear();
    _fat.free_blocks() = std::set<int>();
}

DirEntry FatFS::add_dir(std::string name) {
    return _add_dir_at(_current, name);
}

FileEntry FatFS::add_file(std::string name) {
    return _add_file_at(_current, name);
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

    if(_disk && file_entry.valid()) {
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
    } else
        return 0;
}

std::size_t FatFS::write_file_data(FileEntry &file_entry, const char *data,
                                   std::size_t size) {
    int freeindex, bytes_to_write = size;
    std::size_t bytes = 0;
    FatCell freecell, prevcell;
    DataEntry data_entry;
    std::set<int>::iterator it;

    if(_disk && file_entry.valid()) {
        // check if file system have enough space for new data
        if(size > this->free_space())
            return 0;
        else {
            // update file entry timestamps
            file_entry.update_last_accessed();
            file_entry.update_last_modified();

            // free all associated data blocks before writing
            _free_file_data_blocks(file_entry);

            // write first block of data and connect to file_entry's data
            // pointer

            // get a free cell to start writing
            it = _fat.free_blocks().begin();
            freeindex = *it;
            _fat.free_blocks().erase(it);
            freecell = _fat.get_cell(freeindex);
            freecell.set_next_cell(FatCell::END);

            // connect file_entry'd data pointer to freeindex
            file_entry.set_datacell_index(freeindex);

            // get DataEntr with freecell's block pointer
            data_entry = _disk->file_at(freeindex);

            if(bytes_to_write < (int)_disk->max_block()) {
                bytes = data_entry.write(data, bytes_to_write);
                bytes_to_write -= bytes_to_write;
            } else {
                bytes = data_entry.write(data, bytes_to_write);
                bytes_to_write -= _disk->max_block();
            }

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

                if(bytes_to_write < (int)_disk->max_block()) {
                    bytes += data_entry.write(data + bytes, bytes_to_write);
                    bytes_to_write -= bytes_to_write;
                } else {
                    bytes += data_entry.write(data + bytes, bytes_to_write);
                    bytes_to_write -= _disk->max_block();
                }
            }
            file_entry.set_size(size);  // update file entry size for data

            return size;
        }
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
            _root.init();
            _root.set_name("/");
            _root.set_dot(_block_offset);  // set current block blocation

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
DirEntry FatFS::_add_dir_at(DirEntry target, std::string name) {
    DirEntry dir, find_dir;
    FileEntry find_file;
    std::set<int>::iterator it;

    if(name.size() > ENTRY::MAX_NAME - 1)
        throw std::range_error("File name size exceeded: " +
                               std::to_string(ENTRY::MAX_NAME - 1));

    if(_disk && target.valid()) {
        // find if file name exists
        find_file = _find_file_at(target, name);

        if(!find_file) {
            // check if dir name exists
            find_dir = _find_dir_orlast_at(target, name);

            // check if there's free fat cell
            if(_fat.free_blocks().size()) {
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
                    dir.init();                    // init default values
                    dir.set_name(name);            // set dir name
                    dir.set_dot(new_index);        // set self index
                    dir.set_dotdot(target.dot());  // set parent's index

                    // update last cell pointer
                    if(target.has_dirs()) {
                        _fat.get_cell(find_dir.dot()).set_next_cell(new_index);
                    } else
                        target.set_dircell_index(new_index);  // update dir ptr

                    target.update_last_modified();  // update target timestamp
                }
            }
        }
    }
    return dir;
}

// DirEntry target: add Entry to this directory
// return invalid Entry if can not add
FileEntry FatFS::_add_file_at(DirEntry target, std::string name) {
    DirEntry find_dir;
    FileEntry file, find_file;
    std::set<int>::iterator it;

    if(name.size() > ENTRY::MAX_NAME - 1)
        throw std::range_error("File name size exceeded: " +
                               std::to_string(ENTRY::MAX_NAME - 1));

    if(_disk && target.valid()) {
        // check if dir name exists
        find_dir = _find_dir_at(target, name);

        if(!find_dir) {
            // check if file name exists
            find_file = _find_file_orlast_at(target, name);

            // check if there's free fat cell
            if(_fat.free_blocks().size()) {
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
                    file.init();              // init default values
                    file.set_name(name);      // set file name
                    file.set_dot(new_index);  // set self index

                    // update last cell pointer
                    if(target.has_files())
                        _last_filecell_from_dir(target).set_next_cell(
                            new_index);
                    else
                        // update file ptr
                        target.set_filecell_index(new_index);

                    target.update_last_modified();  // update target timestamps
                }
            }
        }
    }
    return file;
}

// dir_entry: directory entry to start looking at
// entries_set: set of ordered entires by comparator
void FatFS::_dirs_at(DirEntry &dir_entry, DirSet &entries_set) {
    FatCell dircell;

    // clear directory if not empty
    entries_set.clear();

    if(_disk && dir_entry.valid() && dir_entry.has_dirs()) {
        entries_set.emplace(_disk->file_at(dir_entry.dircell_index()));
        dircell = _fat.get_cell(dir_entry.dircell_index());

        while(dircell.has_next()) {
            entries_set.emplace(_disk->file_at(dircell.cell()));
            dircell = _fat.get_cell(dircell.cell());
        }
    }
}

// dir_entry: directory entry to start looking at
// entries_set: set of ordered entires by comparator
void FatFS::_files_at(DirEntry &dir_entry, FileSet &entries_set) {
    FatCell filecell;

    // clear directory if not empty
    entries_set.clear();

    if(_disk && dir_entry.valid() && dir_entry.has_files()) {
        entries_set.emplace(_disk->file_at(dir_entry.filecell_index()));
        filecell = _fat.get_cell(dir_entry.filecell_index());

        while(filecell.has_next()) {
            entries_set.emplace(_disk->file_at(filecell.cell()));
            filecell = _fat.get_cell(filecell.cell());
        }
    }
}

// dir_entry: directory entry to start looking at
DirEntry FatFS::_find_dir_at(DirEntry &dir_entry, std::string name) {
    DirEntry dir, found;
    FatCell dircell;

    if(_disk && dir_entry.valid()) {
        if(name == ".") {
            found = dir_entry;
        } else if(name == "..") {
            int parent_block = dir_entry.dotdot();

            if(parent_block != FatCell::END)
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
    }
    return found;
}

// dir_entry: directory entry to start looking at
FileEntry FatFS::_find_file_at(DirEntry &dir_entry, std::string name) {
    FileEntry file, found;
    FatCell filecell;

    if(_disk && dir_entry.valid() && dir_entry.has_files()) {
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

// dir_entry: directory entry to start looking at
DirEntry FatFS::_find_dir_orlast_at(DirEntry &dir_entry, std::string name) {
    DirEntry dir;
    FatCell dircell;

    if(_disk && dir_entry.valid()) {
        if(name == ".") {
            dir = dir_entry;
        } else if(name == "..") {
            int parent_block = dir_entry.dotdot();

            if(parent_block != FatCell::END)
                dir = DirEntry(_disk->file_at(parent_block));
            else
                dir = dir_entry;
        } else if(dir_entry.has_dirs()) {
            dir = DirEntry(_disk->file_at(dir_entry.dircell_index()));
            dircell = _fat.get_cell(dir_entry.dircell_index());

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
FileEntry FatFS::_find_file_orlast_at(DirEntry &dir_entry, std::string name) {
    FileEntry file;
    FatCell filecell;

    if(_disk && dir_entry.valid() && dir_entry.has_files()) {
        file = FileEntry(_disk->file_at(dir_entry.filecell_index()));
        filecell = _fat.get_cell(dir_entry.filecell_index());

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

    if(_disk && dir_entry.valid() && dir_entry.has_dirs()) {
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

    if(_disk && dir_entry.valid() && dir_entry.has_files()) {
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
    if(_disk && dir_entry.valid()) {
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
            FileEntry file =
                FileEntry(_disk->file_at(dir_entry.filecell_index()));
            FatCell filecell = _fat.get_cell(dir_entry.filecell_index());

            // set diretory entry's file cell pointer to next cell
            dir_entry.set_filecell_index(filecell.cell());

            // free filecell and data blocks associated with FileEntry
            _free_cell(filecell, file.dot());
            _free_file_data_blocks(file);
        }
    }
}

void FatFS::_free_file_data_blocks(FileEntry &file_entry) {
    int datacell_index = FatCell::END;
    FatCell datacell;

    while(file_entry.valid() && file_entry.has_data()) {
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
    _fat.free_blocks().emplace(cell_index);

    // mark this cell as free
    cell.set_free();
}

FatCell FatFS::_last_dircell_from_dir(DirEntry &dir_entry) {
    return _last_cell_from_cell(dir_entry.dircell_index());
}

FatCell FatFS::_last_filecell_from_dir(DirEntry &dir_entry) {
    return _last_cell_from_cell(dir_entry.filecell_index());
}

FatCell FatFS::_last_datacell_from_file(FileEntry &file_entry) {
    return _last_cell_from_cell(file_entry.datacell_index());
}

FatCell FatFS::_last_cell_from_cell(int start_cell) {
    FatCell current = _fat.get_cell(start_cell);

    while(current.has_next()) current = _fat.get_cell(current.cell());

    return current;
}

}  // namespace fs
