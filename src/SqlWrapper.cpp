//
// Created by Basil Terkin on 10/30/16.
//

#include "SqlWrapper.h"
#include "string.h"
#include "sstream"

using namespace std;

namespace ClapTp {

    SqlWrapper::SqlWrapper(const char *pDatabaseName) :
        _pDbName(pDatabaseName), dbClient(::sqlite3_open(1, _pDbName))
    {
        initSchema(dbClient.get());
    }

    SqlWrapper::~SqlWrapper() {
        closeSqlite();
    }

    sqlite3* SqlWrapper::getSqliteInstance() {
        return dbClient.get();
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
        sqlite3_close(dbClient.get());
    }

    void SqlWrapper::saveUser(TgBot::User::Ptr user) {

    }

    void SqlWrapper::saveStash(TgBot::User::Ptr user, const std::string &stashText)
    {
        std::stringstream ss;
        ss << "INSERT INTO [Stashes] ('user_id', 'text') VALUES (:user_id, :text);";

        ::sqlite3_execute(dbClient, 101, ss,
                          std::make_pair("user_id", user->id),
                          std::make_pair("text"   , stashText));
    }

    std::vector<std::string> SqlWrapper::loadStash(TgBot::User::Ptr user)
    {
        std::stringstream sql;

        sql << "SELECT st.text as [text]";
        sql << "FROM [Stashes] st ";
        sql << "WHERE st.user_id = :user_id ";
        sql << "ORDER BY st.id DESC; ";

        return ::sqlite3_fetch<std::vector<std::string> >(dbClient, 101, sql, [=](statement_ptr &statement) -> std::string
                                                          {
                                                                if (statement) {
                                                                    return ::sqlite3_string(statement, "text");
                                                                }
                                                                else {
                                                                    return std::string("");
                                                                }
                                                          },
                                                          std::make_pair("user_id", user->id));
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

        ::sqlite3_execute(dbClient, 101, sql,
                          std::make_pair("user_id", user->id),
                          std::make_pair("count"  , count));
    }

    void SqlWrapper::saveUrl(int32_t& userId, std::string& url, std::string& description) {
        std::stringstream sql;

        sql << "INSERT INTO 'UrlCache'('user_id', 'link', 'description') ";
        sql << "VALUES(:user_id, :url, :description); ";

        ::sqlite3_execute(dbClient, 101, sql,
                          std::make_pair("user_id", userId),
                          std::make_pair("url"    , url),
                          std::make_pair("description", description));
    }

    void SqlWrapper::fetchUrls(char *buffer) {
        bool feelingLucky = true;

        if (feelingLucky)
        {
            std::stringstream sql;
            sql << "SELECT * FROM [UrlCache]; ";
            auto strings = ::sqlite3_fetch<std::vector<std::string> >(dbClient, 101, sql, [](statement_ptr &statement) -> std::string
                                                                      {
                                                                          if (!statement) return {};

                                                                          std::stringstream ss;
                                                                          ss << "User " << ::sqlite3_int(statement, "user_id");
                                                                          ss << " [" << ::sqlite3_string(statement, "url") << "]: ";
                                                                          ss << ::sqlite3_string(statement, "description");
                                                                          return ss.str();
                                                                      });
            auto result = std::accumulate(strings.begin(), strings.end(), std::string(), [](std::string a, std::string b)
                               {
                                   if (a.empty())
                                       return b;
                                   else
                                       return a + "\n" + b;
                               });

           strcpy(buffer, result.c_str());
        }

        sqlite3_stmt *statement = NULL;

        sqlite3_prepare_v2(dbClient.get(), "SELECT * FROM UrlCache;", -1, &statement, 0);

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
        if (int rc = sqlite3_exec(dbClient.get(), query.c_str(), callback, 0, &pSqliteErrMsg) != SQLITE_OK) {
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
