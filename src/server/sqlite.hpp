#ifndef SQLITE_HPP_INCLUDED
#define SQLITE_HPP_INCLUDED

#include "../global.hpp"

#include <sqlite3.h>

#include <vector>
#include <string>

class sqlite_database {
    public:
        sqlite_database(const std::string& filename);
        ~sqlite_database();
        int exec(const std::string& query, std::vector<std::string>* data =NULL);
    private:
        sqlite3* database_;
};

#endif
