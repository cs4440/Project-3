#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "directory.h"

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

    std::map<std::string, Directory>* get_dirs() const;  // get current dirs
    std::map<std::string, File>* get_files() const;      // get current files
    Directory& get_dir(std::string d) const;  // get target from current dir
    File& get_file(std::string f) const;      // get file form current dir

    // MUTATORS
    void add_to_cur_dirs(Directory d);  // add to current dirs
    void add_to_cur_files(File f);      // add to current files
    void remove_dir(std::string n);     // recursively remove dirs and files
                                        // from target directory
    void remove_file(std::string n);    // remove file form current files

private:
    Directory* _root;
    Directory* _current;

    // void _copy_root(Directory* dest, Directory* src);
    // void _delete_node(Directory* node);
};

}  // namespace fs

#endif  // FILESYSTEM_H
