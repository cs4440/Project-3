#include "../include/fat.h"

namespace fs {

Fat::Fat(std::string name, int cells)
    : _name(name),
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
    if(stat(_name.c_str(), &sb) != 0) {
        // open and create file
        _fd = open(_name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

        if(_fd == -1)
            throw std::runtime_error("Error creating Fat file: create");

        // zero fill file of size bytes
        int free = FatCell::FREE;
        for(int i = 0; i < _cells * 2; ++i)
            write(_fd, (char *)&free, sizeof(free));

        // check if file stats is consisent
        if(fstat(_fd, &sb) == 0 && sb.st_size != (int)_bytes)
            throw std::runtime_error(
                "Error checking Fat file descriptor: create");

        // map physical file to memory
        _file = (int *)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE,
                            MAP_SHARED, _fd, 0);

        is_created = true;
    }

    return is_created;
}

bool Fat::open_fat(std::string name) {
    bool is_loaded = false;
    struct stat sb;  // file stats

    // remove this disk if exists
    if(_fd > -1) remove_fat();

    // if file exists
    if(stat(name.c_str(), &sb) == 0) {
        // open existing file
        _fd = open(name.c_str(), O_RDWR, S_IRUSR | S_IWUSR);

        if(_fd == -1) throw std::runtime_error("Error opening Fat file: open");

        // check if file stats is consisent
        if(fstat(_fd, &sb) == 0 && sb.st_size != (int)_bytes)
            throw std::runtime_error(
                "Error checking Fat file descriptor: open");

        // map physical file to memory
        _file = (int *)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE,
                            MAP_SHARED, _fd, 0);

        _bytes = sb.st_size;
        _cells = _bytes / FatCell::SIZE;
        _name = name;

        is_loaded = true;
    }

    return is_loaded;
}

bool Fat::remove_fat() {
    bool is_removed = false;

    _unmap_file();  // unmap virtual memory from file
    _close_fd();    // close file descriptor
    if(remove(_name.c_str()) == 0) is_removed = true;  // remove system file

    return is_removed;
}

bool Fat::valid() const { return _file != nullptr && _fd > -1; }

std::string Fat::name() const { return _name; }

std::size_t Fat::size() const { return _cells; }

std::size_t Fat::file_size() const { return _bytes; }

void Fat::set_name(std::string name) { _name = name; }

void Fat::set_cells(int cells) { _cells = cells; }

FatCell Fat::get_cell(int index) {
    if(index >= _cells) throw std::range_error("Cell index out of bound");

    if(index > FatCell::END)
        return FatCell(_file + index * 2);
    else
        return FatCell(nullptr);
}

void Fat::read_cell(int index, int &cell, int &block) {
    if(index >= _cells) throw std::range_error("Cell index out of bound");

    cell = _file[index * 2];
    block = _file[index * 2 + 1];
}

void Fat::write_cell(int index, int cell, int block) {
    if(index >= _cells) throw std::range_error("Cell index out of bound");

    _file[index * 2] = cell;
    _file[index * 2 + 1] = block;
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

FatFS::FatFS(std::string name, std::size_t cylinders, std::size_t sectors)
    : _name(name),
      _diskname(_name + ".disk"),
      _fatname(_name + ".fat"),
      _disk(_diskname, cylinders, sectors),
      _fat(_fatname, cylinders * sectors) {
    // create physical disk
    if(_disk.create()) {
        // create fat
        // if fail bc fat file exists, then delete fat file and create
        if(!_fat.create()) {
            _fat.remove_fat();
            _fat.create();
        }
    }
    // if disk exists
    else {
        _disk.open_disk(_diskname);
        _fat.open_fat(_fatname);

        // if fat is not valid or total fat cells do not match total disk blocks
        // then remove and recreate
        if(!_fat.valid() || _fat.size() != _disk.total_blocks()) {
            _fat.remove_fat();
            _fat.create();
        }
    }

    _init_root();  // initialize root
    _init_free();  // find list of free cells

    // check if disk and fat is still valid
    if(!_disk.valid()) throw std::logic_error("FatFS create fail on disk file");
    if(!_fat.valid()) throw std::logic_error("FatFS create fail on fat file");
}

std::size_t FatFS::size() const {
    return (_disk.total_blocks() - _free.size()) * _disk.max_block();
}

DirEntry FatFS::current() const { return _current; }

void FatFS::print_dirs() {
    int block = -1;
    std::queue<int> dir_entry_blocks;

    // get a queue of disk blocks for subdirectories
    _dirs_at(_current.dot(), dir_entry_blocks);

    while(!dir_entry_blocks.empty()) {
        block = dir_entry_blocks.front();
        dir_entry_blocks.pop();

        DirEntry dir = DirEntry(_disk.file_at(block));
        std::cout << dir.name() << std::endl;
    }
}

void FatFS::print_files() {
    int block = -1;
    std::queue<int> file_entry_blocks;

    // get a queue of disk blocks for subdirectories
    _files_at(_current.dot(), file_entry_blocks);

    while(!file_entry_blocks.empty()) {
        block = file_entry_blocks.front();
        file_entry_blocks.pop();

        FileEntry file = FileEntry(_disk.file_at(block));
        std::cout << file.name() << std::endl;
    }
}

void FatFS::remove_filesystem() {
    _disk.remove_disk();
    _fat.remove_fat();
}

DirEntry FatFS::add_dir(std::string name) {
    DirEntry dir, found;

    if(name.size() > ENTRY::MAX_NAME - 1)
        throw std::range_error("File name size exceeded: " +
                               std::to_string(ENTRY::MAX_NAME - 1));

    // check if name exists
    found = _find_dir_at(_current.dot(), name);

    // check if there's free fat cell
    if(!found && _free.size()) {
        // get a new index from free index list
        int new_index = _free.front();  // get a free cell index
        _free.pop();                    // remove cell index

        // get a new cell from free index
        FatCell newcell = _fat.get_cell(new_index);

        // mark new cell's cell pointer to END and mark disk block at free index
        newcell.set_cell(FatCell::END);
        newcell.set_block(new_index);

        // free index also indicates free block in disk
        // get directory entry at block and update values
        dir = DirEntry(_disk.file_at(new_index));
        dir.init();                      // initialize default values
        dir.set_name(name);              // set diirectory name
        dir.set_dot(new_index);          // set dir's location
        dir.set_dotdot(_current.dot());  // set parent's location

        // update last cell pointer
        if(_current.has_dirs())
            _last_cell_from_cell(_current.dircell_index()).set_cell(new_index);
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
    found = _find_file_at(_current.dot(), name);

    // check if there's free fat cell
    if(!found && _free.size()) {
        // get a new index from free index list
        int new_index = _free.front();  // get a free cell index
        _free.pop();                    // remove cell index

        // get a new cell from free index
        FatCell newcell = _fat.get_cell(new_index);

        // mark new cell's cell pointer to END and mark disk block at free index
        newcell.set_cell(FatCell::END);
        newcell.set_block(new_index);

        // free index also indicates free block in disk
        // get file entry at block and update values
        file = FileEntry(_disk.file_at(new_index));
        file.init();          // initialize default values
        file.set_name(name);  // set diirectory name

        // update last cell pointer
        if(_current.has_files())
            _last_cell_from_cell(_current.filecell_index()).set_cell(new_index);
        else
            _current.set_filecell_index(new_index);  // update dir pointer

        _current.update_last_modified();  // update current directory times
    }

    return file;
}

bool FatFS::change_dir(std::string name) {
    DirEntry find = _find_dir_at(_current.dot(), name);

    if(find) {
        _current = find;
        return true;
    } else
        return false;
}

FileEntry FatFS::find_file(std::string name) {
    return _find_file_at(_current.dot(), name);
}

void FatFS::_init_root() {
    // get fat cell #0
    FatCell root_cell = _fat.get_cell(0);

    // set root entry at disk block #0
    _root.set_address(_disk.file_at(0));

    // if root cell is free, then add Directory entry to block 0 on disk
    if(root_cell.free()) {
        // set root cell
        root_cell.set_cell(FatCell::END);
        root_cell.set_block(0);

        // initialize root block
        _root.init();
        _root.set_name("/");
        _root.set_dot(0);  // set current block blocation
    }
    _current = _root;
}

void FatFS::_init_free() {
    FatCell fatcell;

    for(std::size_t i = 0; i < _fat.size(); ++i) {
        fatcell = _fat.get_cell(i);

        if(fatcell.free()) _free.push(i);
    }
}

// int start_block: starting directory block to get list of dir entry blocks
// entry_blocks: list of subdirectory blocks to populate
void FatFS::_dirs_at(int start_block, std::queue<int> &entry_blocks) {
    FatCell dircell;

    // clear directory if not empty
    while(!entry_blocks.empty()) entry_blocks.pop();

    // get directory entry at disk block
    DirEntry current = DirEntry(_disk.file_at(start_block));

    if(current.has_dirs()) {
        dircell = _fat.get_cell(current.dircell_index());
        entry_blocks.push(dircell.block());

        while(dircell.has_next()) {
            dircell = _fat.get_cell(dircell.cell());
            entry_blocks.push(dircell.block());
        }
    }
}

// int start_block: starting directory block to get list of file entry blocks
// entry_blocks: list of file entry blocks to populate
void FatFS::_files_at(int start_block, std::queue<int> &entry_blocks) {
    FatCell filecell;

    // clear directory if not empty
    while(!entry_blocks.empty()) entry_blocks.pop();

    // get directory entry at disk block
    DirEntry current = DirEntry(_disk.file_at(start_block));

    if(current.has_files()) {
        filecell = _fat.get_cell(current.filecell_index());
        entry_blocks.push(filecell.block());

        while(filecell.has_next()) {
            filecell = _fat.get_cell(filecell.cell());
            entry_blocks.push(filecell.block());
        }
    }
}

DirEntry FatFS::_find_dir_at(int start_block, std::string name) {
    DirEntry dir, found;
    FatCell dircell;

    // get directory entry at disk block
    DirEntry current = DirEntry(_disk.file_at(start_block));

    if(name == ".") {
        found = current;
    } else if(name == "..") {
        int parent_block = current.dotdot();

        if(parent_block != ENDBLOCK)
            found = DirEntry(_disk.file_at(parent_block));
        else
            found = current;
    } else if(current.has_dirs()) {
        dircell = _fat.get_cell(current.dircell_index());
        dir = DirEntry(_disk.file_at(dircell.block()));

        if(dir.name() != name) {
            while(dircell.has_next()) {
                dircell = _fat.get_cell(dircell.cell());
                dir = DirEntry(_disk.file_at(dircell.block()));

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

FileEntry FatFS::_find_file_at(int start_block, std::string name) {
    FileEntry file, found;
    FatCell filecell;

    // get directory entry at disk block
    DirEntry current = DirEntry(_disk.file_at(start_block));

    if(current.has_files()) {
        filecell = _fat.get_cell(current.filecell_index());
        file = FileEntry(_disk.file_at(filecell.block()));

        if(file.name() != name) {
            while(filecell.has_next()) {
                filecell = _fat.get_cell(filecell.cell());
                file = FileEntry(_disk.file_at(filecell.block()));

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

FatCell FatFS::_last_cell_from_block(int start_block) {
    // get directory entry at disk block
    DirEntry current = DirEntry(_disk.file_at(start_block));

    return _last_cell_from_cell(current.dircell_index());
}

FatCell FatFS::_last_cell_from_cell(int start_cell) {
    FatCell current = _fat.get_cell(start_cell);

    while(current.has_next()) current = _fat.get_cell(current.cell());

    return current;
}

}  // namespace fs
