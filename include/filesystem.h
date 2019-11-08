#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <cstring>
#include <map>
#include <queue>
#include <string>
#include "directory.h"
#include "file.h"

namespace fs {

class FileSystem {
public:
    FileSystem();

    // BIG THREE
    ~FileSystem();
    FileSystem(const FileSystem& src);
    FileSystem& operator=(const FileSystem& rhs);

    // ACCESSORS
    void print_root() const;     // recursively print ALL dirs and files
    void print_current() const;  // print both curent dirs and files
    void print_dirs() const;     // print current directories
    void print_files() const;    // print current files

    bool empty() const;
    std::size_t dirs_size() const;
    std::size_t files_size() const;
    std::size_t size() const;

    Directory* current() const;
    Directory* root() const;

    const std::map<std::string, Directory*>& dirs() const;
    const std::map<std::string, File*>& files() const;
    Directory* find_dir(std::string n) const;
    File* find_file(std::string n) const;

    // MUTATORS
    void init();
    bool change_dir(std::string path);

    Directory* add_dir(std::string n);
    File* add_file(std::string n);
    bool remove_dir(std::string n);
    void remove_dirs();
    bool remove_file(std::string n);
    void remove_files();

private:
    Directory* _root;     // never be changed after init
    Directory* _current;  // points to current dir node

    // convert a path string to queue of entry
    std::queue<std::string> _parse_path(std::string path);
};

}  // namespace fs

#endif  // FILESYSTEM_H
