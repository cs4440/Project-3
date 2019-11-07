#include "../include/filesystem.h"

namespace fs {

FileSystem::FileSystem() : _root(nullptr) {
    _root = new Directory("/");
    _current = _root;
}

FileSystem::~FileSystem() {
    //
}

}  // namespace fs
