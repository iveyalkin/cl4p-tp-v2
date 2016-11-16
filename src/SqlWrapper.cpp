//
// Created by Basil Terkin on 10/30/16.
//

#include "SqlWrapper.h"
#include "string.h"
#include "sstream"

using namespace std;

namespace ClapTp {

    SqlWrapper::SqlWrapper(const char *pDatabaseName) : _pDbName(pDatabaseName) {
        if (int rc = sqlite3_open_v2(_pDbName, &pSqliteDB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK) {
            printf("Open database error: %d\n", rc);
            closeSqlite();
            throw "Cannot open SQLite.";
        }
        initSchema(pSqliteDB);
    }

    SqlWrapper::~SqlWrapper() {
        closeSqlite();
    }

    sqlite3* SqlWrapper::getSqliteInstance() {
        return pSqliteDB;
    }

    char* SqlWrapper::getLastSqliteError() {
        return pSqliteErrMsg;
    }

    void SqlWrapper::initSchema(sqlite3 *pDb) {
        char *pErrMsg = NULL;
        std::vector<const char*> queries;
        queries.push_back("CREATE TABLE IF NOT EXISTS 'UrlCache' (\n"
                                  "'id' INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                                  "'user_id' INTEGER NOT NULL,\n"
                                  "'link' TEXT NOT NULL UNIQUE,\n"
                                  "'description' TEXT\n"
                                  ");");
        queries.push_back("CREATE UNIQUE INDEX IF NOT EXISTS URLCACHE_ID ON 'UrlCache'(id); ");
        queries.push_back("CREATE INDEX IF NOT EXISTS URLCACHE_USER_ID ON 'UrlCache'(user_id); ");

        queries.push_back("CREATE TABLE IF NOT EXISTS 'Stashes' ("
                          "'id' INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "'user_id' INTEGER NOT NULL, "
                          "'text' TEXT NOT NULL); ");
        queries.push_back("CREATE UNIQUE INDEX IF NOT EXISTS STASHES_ID ON 'Stashes'(id); ");
        queries.push_back("CREATE INDEX IF NOT EXISTS STASHES_USER_ID ON 'Stashes'(user_id); ");

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

    void SqlWrapper::closeSqlite() {
        sqlite3_close(pSqliteDB);
        pSqliteDB = NULL;
    }

    void SqlWrapper::saveUser(TgBot::User::Ptr user) {

    }

    void SqlWrapper::saveStash(TgBot::User::Ptr user, const std::string &stashText)
    {
        std::stringstream ss;
        ss << "INSERT INTO [Stashes] ('user_id', 'text') VALUES ("
           << user->id << ", '"
           << stashText << "'); ";

        execSql(ss.str().c_str());
    }

    std::vector<std::string> SqlWrapper::loadStash(TgBot::User::Ptr user)
    {
        std::stringstream sql;

        sql << "SELECT st.text ";
        sql << "FROM [Stashes] st ";
        sql << "WHERE st.user_id = :user_id ";
        sql << "ORDER BY st.id DESC; ";

        sqlite3_stmt *statement = NULL;
        sqlite3_prepare_v2(pSqliteDB, sql.str().c_str(), -1, &statement, 0);

        int index = sqlite3_bind_parameter_index(statement, ":user_id");
        sqlite3_bind_int(statement, index, user->id);

        std::vector<string> result;

        int sqlStepResult;
        while ((sqlStepResult = sqlite3_step(statement)) == SQLITE_ROW)
        {
            auto count = ::sqlite3_column_count(statement);
            for(int column = 0; column < count; ++column) {
                auto colname = ::sqlite3_column_name(statement, column);

                if(strcmp(colname, "text") == 0)
                {
                    const char *text = (const char*)sqlite3_column_text(statement, column);
                    result.push_back(std::string(text));
                }
            }
        }
        if (sqlStepResult != SQLITE_DONE) {
            printf("Failed to read table. Error code: %d", sqlStepResult);
        }

        sqlite3_finalize(statement);

        return result;
    }

    void SqlWrapper::removeNumStashMessages(TgBot::User::Ptr user, int count)
    {
        std::stringstream sql;
        sql << "DELETE FROM [Stashes] ";
        sql << "WHERE id IN ( ";
        sql <<      "SELECT id ";
        sql <<      "FROM [Stashes] ";
        sql <<      "WHERE user_id = :user_id ";
        sql <<      "ORDER BY id DESC ";
        sql <<      "LIMIT :count ";
        sql << "); ";

        sqlite3_stmt *statement = NULL;
        sqlite3_prepare_v2(pSqliteDB, sql.str().c_str(), -1, &statement, 0);

        int index = sqlite3_bind_parameter_index(statement, ":user_id");
        sqlite3_bind_int(statement, index, user->id);

        index = sqlite3_bind_parameter_index(statement, ":count");
        sqlite3_bind_int(statement, index, count);

        int sqlStepResult = sqlite3_step(statement);
        if (sqlStepResult != SQLITE_DONE)
            printf("Failed to delete 'Stashes' records. Error code: %d", sqlStepResult);
    }

    void SqlWrapper::saveUrl(int32_t& userId, std::string& url, std::string& description) {
        char buffer[512];
        sprintf(buffer,
                "INSERT INTO 'UrlCache'('user_id', 'link', 'description') VALUES(%d, '%s', '%s');",
                userId,
                url.c_str(),
                description.c_str()
        );
        execSql(buffer);
    }

    void SqlWrapper::fetchUrls(char *buffer) {
        sqlite3_stmt *statement = NULL;

        sqlite3_prepare_v2(pSqliteDB, "SELECT * FROM UrlCache;", -1, &statement, 0);

        // if there were parameters to bind, we'd do that here

        string result;
        char tmpBuffer[512];
        int sqlStepResult;
        while ((sqlStepResult = sqlite3_step(statement)) == SQLITE_ROW)
        {
            sprintf(
                    tmpBuffer, "User %d [%s]: %s",
                    sqlite3_column_int(statement, 1),
                    sqlite3_column_text(statement, 2),
                    sqlite3_column_text(statement, 3)
            );
            result.append(tmpBuffer).append(";\n");
        }
        strcpy(buffer, result.c_str());
        if (sqlStepResult != SQLITE_DONE) {
            printf("Failed to read table. Error code: %d", sqlStepResult);
        }

        sqlite3_finalize(statement);
    }

    void SqlWrapper::execSql(basic_string<char, char_traits<char>, allocator<char>> query, int (*callback)(void *, int, char **, char **)) {
        if (int rc = sqlite3_exec(pSqliteDB, query.c_str(), callback, 0, &pSqliteErrMsg) != SQLITE_OK) {
            printf("SQL error[%d]: %s\n", rc, pSqliteErrMsg);
            sqlite3_free(pSqliteErrMsg);
            pSqliteErrMsg = NULL;
        }
    }

    void SqlWrapper::execSql(basic_string<char, char_traits<char>, allocator<char>> query) {
        execSql(query, [](void *NotUsed, int argc, char **argv, char **azColName) -> int {
            return SQLITE_OK;
        });
    }
}
