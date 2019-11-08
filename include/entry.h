#ifndef ENTRY_H
#define ENTRY_H

#include <ctime>
#include <string>

namespace fs {

class Entry {
public:
    Entry(std::string n);

    // ACCESSORS
    std::string name() const;
    time_t created() const;
    char *created_cstr() const;
    time_t last_accessed() const;
    char *last_accessed_cstr() const;
    time_t last_updated() const;
    char *last_updated_cstr() const;

    // MUTATORS
    void set_name(std::string n);
    void set_created(time_t t);
    void set_last_access(time_t t);
    void set_last_updated(time_t t);
    void update_last_accessed();
    void update_last_updated();

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
    std::string _name;
    time_t _created;
    time_t _last_accessed;
    time_t _last_updated;
};

}  // namespace fs

#endif  // ENTRY_H
