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
        queries.push_back("CREATE TABLE IF NOT EXISTS 'SampleTable' (\n"
                                  "'user_id' TEXT,\n"
                                  "'link' TEXT NOT NULL UNIQUE,\n"
                                  "'description' TEXT,\n"
                                  "PRIMARY KEY('user_id')\n"
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

    void readSampleTable(char* buffer) {
        execSql("DELETE FROM 'SampleTable';");

        execSql("INSERT INTO 'SampleTable' VALUES('1234567',\n"
                        "'https://github.com/iveyalkin/cl4p-tp-v2/',\n"
                        "'Check this out! I`m dancing! I`m dancing!');");

        sqlite3_stmt *statement = NULL;

        // prepare query
        sqlite3_prepare_v2(pSqliteDB, "SELECT * FROM SampleTable;", -1, &statement, 0);

        // if there were parameters to bind, we'd do that here

        // retrieve the row by row of the results
        // for() {}
        if (int result = sqlite3_step(statement) == SQLITE_ROW)
        {
            // retrieve the value of the first column (0-based)
            sprintf(
                    buffer, "User %s [%s]: %s",
                    sqlite3_column_text(statement, 0),
                    sqlite3_column_text(statement, 1),
                    sqlite3_column_text(statement, 2)
            );
        } else {
            printf("Failed to read sample table error %d", result);
        }

// free our statement
        sqlite3_finalize(statement);
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