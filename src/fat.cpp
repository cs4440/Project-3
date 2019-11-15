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

    return FatCell(_file + index * 2);
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

    // initialize root
    _init_root();

    // check if disk and fat is still valid
    if(!_disk.valid()) throw std::logic_error("FatFS create fail on disk file");
    if(!_fat.valid()) throw std::logic_error("FatFS create fail on fat file");
}

void FatFS::remove_filesystem() {
    _disk.remove_disk();
    _fat.remove_fat();
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
    }
}

}  // namespace fs
