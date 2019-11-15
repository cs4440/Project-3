#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include "directory.h"
#include "file.h"

namespace fs {

class FileSystem {
public:
    enum { FNAME_MAX = 255 };

    FileSystem();

    // BIG THREE
    ~FileSystem();
    FileSystem(const FileSystem& src);
    FileSystem& operator=(const FileSystem& rhs);

    // ACCESSORS
    std::ostream& print_entries(std::ostream& outs = std::cout) const;
    std::ostream& print_dirs(std::ostream& outs = std::cout) const;
    std::ostream& print_files(std::ostream& outs = std::cout) const;
    std::ostream& print_pwd(std::ostream& outs = std::cout) const;

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

    // FRIENDS
    friend std::ostream& operator<<(std::ostream& outs, const FileSystem& fs) {
        return fs.print_pwd(outs);  // return output
    }

private:
    std::list<Directory*> _pwd;
    Directory* _root;  // never be changed after init

    // convert a path string to queue of entry
    std::queue<std::string> _parse_path(std::string path);
};

}  // namespace fs

#endif  // FILESYSTEM_H
