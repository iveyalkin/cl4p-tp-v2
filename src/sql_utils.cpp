//
// Created by Basil Terkin on 10/30/16.
//

#include <vector>
#include "sql_utils.h"

using namespace std;

namespace claptp {

    sqlite3 *pSqliteDB = NULL;

    char *pSqliteErrMsg = NULL;

    sqlite3* getSqliteInstance() {
        return pSqliteDB;
    }

    char* getLastSqliteError() {
        return pSqliteErrMsg;
    }

    void initSchema(sqlite3 *pDb) {
        char *pErrMsg = NULL;
        std::vector<const char*> queries;
        queries.push_back("CREATE TABLE IF NOT EXISTS  `SampleTable` (\n"
                                  "\t`user_id`\tTEXT,\n"
                                  "\t`link`\tTEXT NOT NULL UNIQUE,\n"
                                  "\t`description`\tTEXT,\n"
                                  "\tPRIMARY KEY(`user_id`)\n"
                                  ");");
        vector<const char*>::const_iterator
                iter = queries.begin(),
                end = queries.end();
        while (iter != end) {
            if (int rc = sqlite3_exec(pDb, *iter, NULL, 0, &pErrMsg) != SQLITE_OK) {
                printf("Creating DB schema error[%d]: %s\n", rc, pErrMsg);
                sqlite3_free(pErrMsg);
                pErrMsg = NULL;
                throw "Cannot execute query";
            }
            iter++;
        }
    }

    void initSqlite(const char *pDbName) {
        if (int rc = sqlite3_open_v2(pDbName, &pSqliteDB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK) {
            printf("Open database error: %d\n", rc);
            closeSqlite();
            throw "Cannot open SQLite.";
        }
        initSchema(pSqliteDB);
    }


    void closeSqlite() {
        sqlite3_close(pSqliteDB);
        pSqliteDB = NULL;
    }

    void execSql(basic_string<char, char_traits<char>, allocator<char>> query, int (*callback)(void *, int, char **, char **)) {
        if (int rc = sqlite3_exec(pSqliteDB, query.c_str(), callback, 0, &pSqliteErrMsg) != SQLITE_OK) {
            printf("SQL error[%d]: %s\n", rc, pSqliteErrMsg);
            sqlite3_free(pSqliteErrMsg);
            pSqliteErrMsg = NULL;
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