#include "../include/directory.h"

namespace fs {

Directory::~Directory() {
    remove_files();
    remove_dirs();
}

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

// return a node from path; else nullptr;
Directory* Directory::find_path(std::queue<std::string>& entries) {
    return _traverse_path(entries, this);
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
    for(auto it = _dirs.begin(); it != _dirs.end(); ++it) delete it->second;
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
    for(auto it = _files.begin(); it != _files.end(); ++it) delete it->second;
    _files.clear();
    update_last_updated();
}

// recursively traverse a path starting from node
// return node on success; else nullptr
Directory* Directory::_traverse_path(std::queue<std::string>& entries,
                                     Directory* node) const {
    if(entries.size()) {
        std::string entry = entries.front();

        if(entry == "/") {
            entries.pop();

            // find root node
            while(node->parent()) node = node->parent();

            if(entries.empty())
                return node;
            else
                return _traverse_path(entries, node);
        } else if(entry == "..") {
            entries.pop();

            if(node->parent())
                return _traverse_path(entries, node->parent());
            else
                return node;
        } else {
            if(entry == ".") {
                entries.pop();
                if(entries.size()) entry = entries.front();
            }

            auto find = node->find_dir(entry);

            if(!find)
                return nullptr;
            else {
                entries.pop();
                return _traverse_path(entries, find);
            }
        }
    } else
        return node;
}

}  // namespace fs
