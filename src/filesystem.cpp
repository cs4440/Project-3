#include "../include/filesystem.h"

namespace fs {

FileSystem::FileSystem() : _root(nullptr), _current(nullptr) {}

FileSystem::~FileSystem() {
    //
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
bool FileSystem::change_dir(std::string& path) {
    bool changed = true;
    std::queue<std::string> entries;
    std::string entry;
    std::shared_ptr<Directory> find, current = _current;

    // parse path
    entries = _parse_path(path);

    while(!entries.empty()) {
        entry = entries.front();
        entries.pop();

        if(entry == "/")
            _current = _root;
        else if(entry == "..") {
            if(_current->parent()) _current = _current->parent();
        } else if(entry != ".") {
            find = _current->find_dir(entry);

            if(!find) {              // if not found
                _current = current;  // change to original current
                path = entry;        // set path to entry that failed
                changed = false;
                break;
            } else
                _current = find;
        }
    }

    return changed;
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
    std::queue<std::string> entries;
    std::size_t pos = 0, prev_pos = 0;

    while((pos = path.find('/', pos)) != std::string::npos) {
        if(pos == 0)
            entries.push("/");
        else
            entries.push(path.substr(prev_pos, pos - prev_pos));

        prev_pos = ++pos;
    }

    if(prev_pos < path.size() - 1)

        entries.push(path.substr(prev_pos, path.size() - prev_pos));

    return entries;
}

}  // namespace fs
