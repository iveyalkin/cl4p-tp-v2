//
// Created by Basil Terkin on 10/30/16.
//

#ifndef CL4PTP_SQL_WRAPPER_H
#define CL4PTP_SQL_WRAPPER_H

#include "stdio.h"
#include "sqlite3.h"
#include "tgbot/types/User.h"

namespace ClapTp {

    class SqlWrapper {

    public:
        SqlWrapper(const char *pDatabaseName);

        ~SqlWrapper();

        sqlite3 *getSqliteInstance();

        char *getLastSqliteError();

        void saveUser(TgBot::User::Ptr user);

        void saveUrl(int32_t &userId, std::string &url, std::string &description);

        void execSql(std::basic_string<char, std::char_traits<char>, std::allocator<char>> query,
                     int (*callback)(void *, int, char **, char **));

        void execSql(std::basic_string<char, std::char_traits<char>, std::allocator<char>> query);

        // TODO temporary
        void fetchUrls(char *buffer);

    private:
        sqlite3 *pSqliteDB = NULL;

        char *pSqliteErrMsg = NULL;

        const char *_pDbName = NULL;

    protected:
        void initSchema(sqlite3 *pDb);

        void closeSqlite();
    };
}

#endif //CL4PTP_SQL_WRAPPER_H
