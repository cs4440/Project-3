#ifndef ENTRY_H
#define ENTRY_H

#include <string>

namespace fs {

class Entry {
public:
    Entry(std::string n) : _name(n) {}

    // ACCESSORS
    std::string name() const { return _name; }
    long created() const { return _created; }
    long last_updated() const { return _last_updated; }
    long last_access() const { return _last_accessed; }

    // MUTATORS
    void set_name(std::string n) { _name = n; }
    void set_created(long d) { _created = d; }
    void set_last_access(long d) { _last_accessed = d; }
    void set_last_updated(long d) { _last_updated = d; }

    // FRIENDS
    friend bool operator==(const fs::Entry &lhs, const fs::Entry &rhs) {
        return lhs._name == rhs._name;
    }

    friend bool operator<(const fs::Entry &lhs, const fs::Entry &rhs) {
        return lhs._name < rhs._name;
    }

    friend bool operator<=(const fs::Entry &lhs, const fs::Entry &rhs) {
        return lhs._name <= rhs._name;
    }

    friend bool operator>(const fs::Entry &lhs, const fs::Entry &rhs) {
        return lhs._name < rhs._name;
    }

    friend bool operator>=(const fs::Entry &lhs, const fs::Entry &rhs) {
        return lhs._name <= rhs._name;
    }

protected:
    long _created;
    long _last_accessed;
    long _last_updated;
    std::string _name;
};

}  // namespace fs

#endif  // ENTRY_H
