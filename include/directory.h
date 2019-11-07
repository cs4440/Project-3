#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <map>
#include <string>
#include "entry.h"
#include "file.h"

namespace fs {

class Directory : public Entry {
public:
    Directory(std::string n, Directory *p = nullptr) : Entry(n), _parent(p) {}
    ~Directory();

    // ACCESSOR
    bool empty() const;
    std::size_t get_dirs_size() const;
    std::size_t get_files_size() const;
    std::size_t get_size() const;
    std::map<std::string, Directory *> get_dirs() const;
    std::map<std::string, File *> get_files() const;

    // MUTATOR
    void add_dir(Directory *d);
    void add_file(File *f);
    void remove_dir(std::string n);
    void remove_file(std::string n);
    void remove_files();

private:
    Directory *_parent;
    std::map<std::string, Directory *> _dirs;
    std::map<std::string, File *> _files;

    void _delete_dir(Directory *node);
};

}  // namespace fs

#endif  // DIRECTORY_H
