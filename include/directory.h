#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <map>
#include <string>
#include "entry.h"
#include "file.h"

namespace fs {

class Directory : public Entry {
public:
    Directory(std::string n, Directory* p = nullptr) : Entry(n), _parent(p) {}

    // BIG THREE
    ~Directory();
    Directory(const Directory& src);
    Directory& operator=(const Directory& rhs);

    // ACCESSOR
    bool empty() const;
    std::size_t dirs_size() const;
    std::size_t files_size() const;
    std::size_t size() const;

    Directory* parent() const;
    const std::map<std::string, Directory*>& dirs() const;
    const std::map<std::string, File*>& files() const;
    Directory* find_dir(std::string n) const;
    File* find_file(std::string n) const;

    // MUTATOR
    Directory* add_dir(std::string n);
    File* add_file(std::string n);
    bool remove_dir(std::string n);
    void remove_dirs();
    bool remove_file(std::string n);
    void remove_files();

private:
    Directory* _parent;
    std::map<std::string, Directory*> _dirs;
    std::map<std::string, File*> _files;

    void _delete_nodes(Directory* node);
};

}  // namespace fs

#endif  // DIRECTORY_H
