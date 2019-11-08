#include "../include/entry.h"

namespace fs {

Entry::Entry(std::string n) : _name(n) {
    std::time(&_created);
    _last_accessed = _last_updated = _created;
}

std::string Entry::name() const { return _name; }

time_t Entry::created() const { return _created; }

char *Entry::created_cstr() const { return std::ctime(&_created); }

time_t Entry::last_accessed() const { return _last_accessed; }

char *Entry::last_accessed_cstr() const { return std::ctime(&_last_accessed); }

time_t Entry::last_updated() const { return _last_updated; }

char *Entry::last_updated_cstr() const { return std::ctime(&_last_updated); }

void Entry::set_name(std::string n) { _name = n; }

void Entry::set_created(time_t t) { _created = t; }

void Entry::set_last_access(time_t t) { _last_accessed = t; }

void Entry::set_last_updated(time_t t) { _last_updated = t; }

void Entry::update_last_accessed() {
    _last_accessed = std::time(&_last_accessed);
}

void Entry::update_last_updated() { _last_updated = std::time(&_last_updated); }

}  // namespace fs
