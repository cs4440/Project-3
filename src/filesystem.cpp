#include "../include/filesystem.h"

namespace fs {

FileSystem::FileSystem() : _root(nullptr) {}

FileSystem::~FileSystem() {
    if(_root) delete _root;
}

std::ostream& FileSystem::print_entries(std::ostream& outs) const {
    print_dirs(outs);
    print_files(outs);

    return outs;
}

std::ostream& FileSystem::print_dirs(std::ostream& outs) const {
    auto dirs = _cwd.back()->dirs();
    for(auto it = dirs.begin(); it != dirs.end(); ++it)
        outs << it->first << std::endl;

    return outs;
}

std::ostream& FileSystem::print_files(std::ostream& outs) const {
    auto files = _cwd.back()->files();
    for(auto it = files.begin(); it != files.end(); ++it)
        outs << it->first << std::endl;

    return outs;
}

std::ostream& FileSystem::print_cwd(std::ostream& outs) const {
    if(_cwd.size() == 1)
        outs << _cwd.front()->name();
    else
        for(auto it = ++_cwd.begin(); it != _cwd.end(); ++it)
            outs << '/' << (*it)->name();

    return outs;
}

bool FileSystem::empty() const { return current()->empty(); }

std::size_t FileSystem::dirs_size() const { return current()->dirs_size(); }

std::size_t FileSystem::files_size() const { return current()->files_size(); }

std::size_t FileSystem::size() const { return current()->size(); }

Directory* FileSystem::current() const { return _cwd.back(); }

Directory* FileSystem::root() const { return _root; }

const std::map<std::string, Directory*>& FileSystem::dirs() const {
    return current()->dirs();
}

const std::map<std::string, File*>& FileSystem::files() const {
    return current()->files();
}

Directory* FileSystem::find_dir(std::string n) const {
    return current()->find_dir(n);
}

File* FileSystem::find_file(std::string n) const {
    return current()->find_file(n);
}

void FileSystem::init() {
    _root = new Directory("/");
    _cwd.push_back(_root);
}

// if path is invalid, _cwd is not changed
bool FileSystem::change_dir(std::string path) {
    bool success = true;
    Directory* find = nullptr;
    std::list<Directory*> cwd = _cwd;
    std::queue<std::string> entries;
    std::string entry;

    entries = _parse_path(path);  // tokenize path

    // traverse entries
    while(!entries.empty()) {
        entry = entries.front();
        entries.pop();

        if(entry == "/") {
            cwd.clear();
            cwd.push_back(_root);
        } else if(entry == "..") {
            if(!cwd.empty() && cwd.back()->name() != "/") cwd.pop_back();
        } else if(entry == ".") {
            // do nothing
        } else {
            find = cwd.back()->find_dir(entry);

            if(find)
                cwd.push_back(find);
            else {
                success = false;
                break;
            }
        }
    }

    if(success) _cwd = cwd;

    return success;
}

Directory* FileSystem::add_dir(std::string n) {
    if(n.size() > FNAME_MAX)
        throw std::runtime_error("Name exceeded size limit " + FNAME_MAX);
    else {
        Directory* new_dir = current()->add_dir(n);

        if(new_dir)
            return new_dir;
        else
            throw std::runtime_error("Name already exists");
    }
}

File* FileSystem::add_file(std::string n) {
    if(n.size() > FNAME_MAX)
        throw std::runtime_error("Name exceeded size limit " + FNAME_MAX);
    else {
        File* new_file = current()->add_file(n);

        if(new_file)
            return new_file;
        else
            throw std::runtime_error("Name already exists");
    }
}

bool FileSystem::remove_dir(std::string n) { return current()->remove_dir(n); }

void FileSystem::remove_dirs() { current()->remove_dirs(); }

bool FileSystem::remove_file(std::string n) {
    return current()->remove_file(n);
}

void FileSystem::remove_files() { current()->remove_files(); }

std::queue<std::string> FileSystem::_parse_path(std::string path) {
    char* token = nullptr;
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
