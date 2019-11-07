#ifndef FILE_H
#define FILE_H

#include <string>
#include "entry.h"

namespace fs {

class File : public Entry {
public:
    File(std::string n) : Entry(n) {}

private:
};

}  // namespace fs

#endif  // FILE_H
