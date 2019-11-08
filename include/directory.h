#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <map>
#include <memory>
#include <string>
#include "entry.h"
#include "file.h"

namespace fs {

class Directory : public Entry {
public:
    Directory(std::string n, std::shared_ptr<Directory> p = nullptr)
        : Entry(n), _parent(p) {}

    // BIG THREE
    ~Directory();
    Directory(const Directory& src);
    Directory& operator=(const Directory& rhs);

    // ACCESSOR
    bool empty() const;
    std::size_t dirs_size() const;
    std::size_t files_size() const;
    std::size_t size() const;

    std::shared_ptr<Directory> parent() const;
    const std::map<std::string, std::shared_ptr<Directory> >& dirs() const;
    const std::map<std::string, std::shared_ptr<File> >& files() const;
    std::shared_ptr<Directory> find_dir(std::string n) const;
    std::shared_ptr<File> find_file(std::string n) const;

    // MUTATOR
    std::shared_ptr<Directory> add_dir(std::string n,
                                       std::shared_ptr<Directory> p);
    std::shared_ptr<File> add_file(std::string n);
    bool remove_dir(std::string n);
    void remove_dirs();
    bool remove_file(std::string n);
    void remove_files();

private:
    std::shared_ptr<Directory> _parent;
    std::map<std::string, std::shared_ptr<Directory> > _dirs;
    std::map<std::string, std::shared_ptr<File> > _files;

    void _recursive_remove_dir(std::shared_ptr<Directory> node);
};

}  // namespace fs

#endif  // DIRECTORY_H
