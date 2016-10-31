//
// Created by Basil Terkin on 10/30/16.
//

#ifndef CL4PTP_SQL_UTILS_H
#define CL4PTP_SQL_UTILS_H

#include <iostream>

#include "sqlite3.h"

namespace claptp {

    sqlite3* getSqliteInstance();

    char* getLastSqliteError();

    void initSqlite(const char *dbName);

    void closeSqlite();

    void execSql(std::basic_string<char, std::char_traits<char>, std::allocator<char>> query, int (*callback)(void *, int, char **, char **));

    void execSql(std::basic_string<char, std::char_traits<char>, std::allocator<char>> query);

    /*void execSql(const char *query);

    void execSql(const char *query, int (*callback)(void *, int, char **, char **));*/
}

#endif //CL4PTP_SQL_UTILS_H
