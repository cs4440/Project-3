#include "../include/directory.h"

namespace fs {

Directory::~Directory() {
    remove_files();

    for(auto it = _dirs.begin(); it != _dirs.end(); ++it) {
        if(it->second) _delete_dir(it->second);
        _dirs.erase(it);
    }
}

bool Directory::empty() const { return get_size() == 0; }

std::size_t Directory::get_dirs_size() const { return _dirs.size(); }

std::size_t Directory::get_files_size() const { return _files.size(); }

std::size_t Directory::get_size() const { return _dirs.size() + _files.size(); }

std::map<std::string, Directory *> Directory::get_dirs() const { return _dirs; }

std::map<std::string, File *> Directory::get_files() const { return _files; }

void Directory::add_dir(Directory *d) { _dirs[d->name()] = d; }

void Directory::add_file(File *f) { _files[f->name()] = f; }

void Directory::remove_dir(std::string n) {
    auto find = _dirs.find(n);  // find dir by string

    if(find != _dirs.end()) {     // if found
        auto dir = find->second;  // get pointer to Directory

        if(dir) _delete_dir(dir);  // if dir is not null, delete
        _dirs.erase(find);         // erase entry in map
    }
}

void Directory::remove_file(std::string n) {
    auto find = _files.find(n);  // find file by string

    if(find != _files.end()) {  // if found
        auto file = _files[n];  // get pointer to File

        if(file) delete file;  // if file is not null, delete
        _files.erase(find);    // erase entry in map
    }
}

void Directory::remove_files() {
    for(auto it = _files.begin(); it != _files.end(); ++it) {
        if(it->second) delete it->second;  // if file is not null, delete
        _files.erase(it);                  // erase entry
    }
}

void Directory::_delete_dir(Directory *node) {
    if(node) {                    // if node is not null
        node->remove_files();     // remove all files
        auto dirs = node->_dirs;  // get directory map

        // iterate through map of Directory
        for(auto it = dirs.begin(); it != dirs.end(); ++it) {
            // recursively call _delete_dir with Directory pointer
            if(it->second) _delete_dir(it->second);
            dirs.erase(it);
        }
    }
}

}  // namespace fs
