#include "sqlite.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>

sqlite_database::sqlite_database(const std::string& filename) {
    int result_code = sqlite3_open(filename.c_str(), &database_);
    if(result_code != SQLITE_OK){
        //! @todo Do any actual error handling
        std::cerr << "Error opening database " << filename << std::endl;
        sqlite3_close(database_);
    }
    std::cout << "Opened database " << filename << std::endl;
}

sqlite_database::~sqlite_database() {
    sqlite3_close(database_);
}

int sqlite_database::exec(const std::string& query, std::vector<std::string>* data) {
    int result_code;

    char** result;
    int num_rows, num_cols;
    char* errmsg;

    result_code = sqlite3_get_table(database_, query.c_str(), &result, &num_rows, &num_cols, &errmsg);

    if(result_code == SQLITE_OK) {
        if(data) {
            for(int i=0; i < num_cols * num_rows; ++i) {
                //The first row would be the titles of the columnes which we ignore
                data->push_back(result[num_cols+i]);
            }
        }
    } else {
        std::cerr << "Error executing SQL query: " << errmsg << std::endl;
    }

    sqlite3_free_table(result);

    return result_code;
}
