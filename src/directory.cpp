#include "../include/directory.h"

namespace fs {

Directory::~Directory() {
    remove_files();

    for(auto it = _dirs.begin(); it != _dirs.end(); ++it)
        _recursive_remove_dir(it->second);
    remove_dirs();
}

bool Directory::empty() const { return size() == 0; }

std::size_t Directory::dirs_size() const { return _dirs.size(); }

std::size_t Directory::files_size() const { return _files.size(); }

std::size_t Directory::size() const { return _dirs.size() + _files.size(); }

std::shared_ptr<Directory> Directory::parent() const { return _parent; }

const std::map<std::string, std::shared_ptr<Directory> >& Directory::dirs()
    const {
    return _dirs;
}

const std::map<std::string, std::shared_ptr<File> >& Directory::files() const {
    return _files;
}

std::shared_ptr<Directory> Directory::find_dir(std::string n) const {
    std::shared_ptr<Directory> dir = nullptr;
    auto find = _dirs.find(n);

    if(find != _dirs.end()) dir = find->second;

    return dir;
}

std::shared_ptr<File> Directory::find_file(std::string n) const {
    std::shared_ptr<File> file = nullptr;
    auto find = _files.find(n);

    if(find != _files.end()) file = find->second;

    return file;
}

std::shared_ptr<Directory> Directory::add_dir(std::string n,
                                              std::shared_ptr<Directory> p) {
    std::shared_ptr<Directory> new_dir = nullptr;
    auto find = _dirs.find(n);

    if(find == _dirs.end()) {
        new_dir = std::shared_ptr<Directory>(new Directory(n, p));
        _dirs[n] = new_dir;
        update_last_updated();
    }

    return new_dir;
}

std::shared_ptr<File> Directory::add_file(std::string n) {
    std::shared_ptr<File> new_file = nullptr;
    auto find = _files.find(n);

    if(find == _files.end()) {  // if doesn't exist, add
        new_file = std::shared_ptr<File>(new File(n));
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

void Directory::_recursive_remove_dir(std::shared_ptr<Directory> node) {
    if(node) {
        node->remove_files();

        auto dirs = node->dirs();
        for(auto it = dirs.begin(); it != dirs.end(); ++it)
            _recursive_remove_dir(it->second);
        node->remove_dirs();
    }
}

}  // namespace fs
