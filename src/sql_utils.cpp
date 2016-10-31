//
// Created by Basil Terkin on 10/30/16.
//

#include "sql_utils.h"

using namespace std;

namespace claptp {

    sqlite3 *sqliteDB = NULL;

    char *sqliteErrMsg = NULL;

    sqlite3* getSqliteInstance() {
        return sqliteDB;
    }

    char* getLastSqliteError() {
        return sqliteErrMsg;
    }

    void initSqlite(const char *dbName) {
        if (int rc = sqlite3_open(dbName, &sqliteDB)) {
            printf("SQL error: %d\n", rc);
            closeSqlite();
            throw "Cannot open SQLite.";
        }
    }

    void closeSqlite() {
        sqlite3_close(sqliteDB);
        sqliteDB = NULL;
    }

    void execSql(basic_string<char, char_traits<char>, allocator<char>> query, int (*callback)(void *, int, char **, char **)) {
        if (int rc = sqlite3_exec(sqliteDB, query.c_str(), callback, 0, &sqliteErrMsg) != SQLITE_OK) {
            printf("SQL error: %s\n", sqliteErrMsg);
            sqlite3_free(sqliteErrMsg);
            sqliteErrMsg = NULL;
            throw "Cannot execute query";
        }
    }

    void execSql(basic_string<char, char_traits<char>, allocator<char>> query) {
        execSql(query, [](void *NotUsed, int argc, char **argv, char **azColName) -> int {
            return SQLITE_OK;
        });
    }

    /*void execSql(const char *query) {
        execSql(string(query));
    }*/

    /*void execSql(const char *query, int (*callback)(void *, int, char **, char **)) {
        execSql(string(query), callback);
    }*/
}