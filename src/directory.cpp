#include "../include/directory.h"

namespace fs {

Directory::~Directory() { _delete_nodes(this); }

bool Directory::empty() const { return size() == 0; }

std::size_t Directory::dirs_size() const { return _dirs.size(); }

std::size_t Directory::files_size() const { return _files.size(); }

std::size_t Directory::size() const { return _dirs.size() + _files.size(); }

Directory* Directory::parent() const { return _parent; }

const std::map<std::string, Directory*>& Directory::dirs() const {
    return _dirs;
}

const std::map<std::string, File*>& Directory::files() const { return _files; }

Directory* Directory::find_dir(std::string n) const {
    Directory* dir = nullptr;
    auto find = _dirs.find(n);

    if(find != _dirs.end()) dir = find->second;

    return dir;
}

File* Directory::find_file(std::string n) const {
    File* file = nullptr;
    auto find = _files.find(n);

    if(find != _files.end()) file = find->second;

    return file;
}

Directory* Directory::add_dir(std::string n) {
    Directory* new_dir = nullptr;
    auto find = _dirs.find(n);

    if(find == _dirs.end()) {
        _dirs[n] = new Directory(n, this);
        update_last_updated();
    }

    return new_dir;
}

File* Directory::add_file(std::string n) {
    File* new_file = nullptr;
    auto find = _files.find(n);

    if(find == _files.end()) {  // if doesn't exist, add
        new_file = new File(n);
        _files[n] = new_file;
        update_last_updated();
    }

    return new_file;
}

bool Directory::remove_dir(std::string n) {
    bool is_removed = true;
    auto find = _dirs.find(n);

    if(find != _dirs.end()) {
        _dirs.erase(find);
        update_last_updated();
    } else
        is_removed = false;

    return is_removed;
}

void Directory::remove_dirs() {
    _dirs.clear();
    update_last_updated();
}

bool Directory::remove_file(std::string n) {
    bool is_removed = true;
    auto find = _files.find(n);

    if(find != _files.end()) {
        _files.erase(find);
        update_last_updated();
    } else
        is_removed = false;

    return is_removed;
}

void Directory::remove_files() {
    _files.clear();
    update_last_updated();
}

void Directory::_delete_nodes(Directory* node) {
    if(node) {
        auto files = node->files();
        for(auto it = files.begin(); it != files.end(); ++it) {
            delete it->second;
            it->second = nullptr;
        }
        node->remove_files();

        auto dirs = node->dirs();
        for(auto it = dirs.begin(); it != dirs.end(); ++it) {
            auto dir = it->second;
            _delete_nodes(dir);

            delete it->second;
            it->second = nullptr;
        }
        node->remove_dirs();
    }
}

}  // namespace fs
