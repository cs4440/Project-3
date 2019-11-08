#include "../include/filesystem.h"

namespace fs {

FileSystem::FileSystem() : _root(nullptr), _current(nullptr) {}

FileSystem::~FileSystem() {
    _root->remove_dirs();
    _root->remove_files();
}

bool FileSystem::empty() const { return _current->empty(); }

std::size_t FileSystem::dirs_size() const { return _current->dirs_size(); }

std::size_t FileSystem::files_size() const { return _current->files_size(); }

std::size_t FileSystem::size() const { return _current->size(); }

std::shared_ptr<Directory> FileSystem::current() const { return _current; }

std::shared_ptr<Directory> FileSystem::root() const { return _root; }

const std::map<std::string, std::shared_ptr<Directory> >& FileSystem::dirs()
    const {
    return _current->dirs();
}

const std::map<std::string, std::shared_ptr<File> >& FileSystem::files() const {
    return _current->files();
}

std::shared_ptr<Directory> FileSystem::find_dir(std::string n) const {
    return _current->find_dir(n);
}

std::shared_ptr<File> FileSystem::find_file(std::string n) const {
    return _current->find_file(n);
}

void FileSystem::init() {
    _root = _current = std::shared_ptr<Directory>(new Directory("root"));
}

// if change directory fails
// then reset current and change path to entry that fails
bool FileSystem::change_dir(std::string path) {
    bool success = true;
    std::queue<std::string> entries;
    std::string entry;
    std::shared_ptr<Directory> find, current = _current;

    entries = _parse_path(path);  // tokenize path

    // traverse entries
    while(!entries.empty()) {
        entry = entries.front();
        entries.pop();

        if(entry == "/")
            current = _root;
        else if(entry == "..") {
            if(current->parent()) current = current->parent();
        } else if(entry == ".") {
            // do nothing
        } else {
            auto find = current->find_dir(entry);

            if(find)
                current = find;
            else {
                success = false;
                break;
            }
        }
    }

    if(success) _current = current;

    return success;
}

std::shared_ptr<Directory> FileSystem::add_dir(std::string n) {
    return _current->add_dir(n, _current);
}

std::shared_ptr<File> FileSystem::add_file(std::string n) {
    return _current->add_file(n);
}

bool FileSystem::remove_dir(std::string n) { return _current->remove_dir(n); }

void FileSystem::remove_dirs() { _current->remove_dirs(); }

bool FileSystem::remove_file(std::string n) { return _current->remove_file(n); }

void FileSystem::remove_files() { _current->remove_files(); }

std::queue<std::string> FileSystem::_parse_path(std::string path) {
    char* token;
    std::queue<std::string> entries;

    if(!path.empty()) {
        char* cstr = new char[path.length() + 1]();
        strncpy(cstr, path.c_str(), path.size());

        if(cstr[0] == '/') {
            entries.push("/");
            token = strtok(cstr + 1, "/");
        } else
            token = strtok(cstr, "/");

        while(token != NULL) {
            entries.push(token);
            token = strtok(NULL, "/");
        }

        delete[] cstr;
    }

    return entries;
}

}  // namespace fs
