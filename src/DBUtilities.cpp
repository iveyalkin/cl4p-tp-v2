
#include "DBUtilities.h"
#include <boost/algorithm/string/replace.hpp>


std::string sqlite3_escape(const std::string &input)
{
    if(input.empty()) return "''";

    char *buffer = ::sqlite3_mprintf("%Q", input.c_str());
    if(!buffer || !*buffer) return "''";

    std::string output = buffer;
    ::sqlite3_free(buffer);

    return output;
};

std::string sqlite3_escape_like(const std::string &input, const char escape)
{
    auto string = input;
    auto esc = std::string{escape};

    boost::replace_all(string, esc, esc + escape);
    boost::replace_all(string, std::string{'%'}, esc + '%');
    boost::replace_all(string, std::string{'_'}, esc + '_');

    return string;
};

#undef _
#define _ ::sqlite3_escape

database_ptr sqlite3_open(int uid, const std::string &path)
{
    sqlite3* db = nullptr;
    auto rc = ::sqlite3_open(path.c_str(), &db);
    if(SQLITE_OK != rc) {
        DATABASE_LOG(uid, "sqlite3_open(" << path << "): #" << rc);
        assert(false);
    }

    database_ptr database(db, [=](sqlite3 *db){
        if(db) ::sqlite3_close(db);
    });

    DATABASE_LOG(uid, "OPEN DATABASE '" << path << "';");

    {
        std::stringstream sql;
        sql << "PRAGMA auto_vacuum = OFF;";
        ::sqlite3_execute(database, uid, sql);
    }

    {
        std::stringstream sql;
        sql << "PRAGMA journal_mode=WAL;";
        ::sqlite3_execute(database, uid, sql);
    }

    {
        std::stringstream sql;
        sql << "PRAGMA foreign_keys = ON;";
        ::sqlite3_execute(database, uid, sql);
    }

    struct userfunc {
        // IS_EMPTY(<value>) -> BOOL
        static void sqlite3_user_is_empty(sqlite3_context *context, int argc, sqlite3_value **argv) {
            if(argc != 1) {
                sqlite3_result_error(context, "too few argments", -1);
                return;
            }

            int type = sqlite3_value_type(argv[0]);

            bool empty = false;
            if(type == SQLITE_INTEGER)
                empty = 0 == sqlite3_value_int(argv[0]);
            else if(type == SQLITE_NULL)
                empty = true;
            else if(type == SQLITE_TEXT)
                empty = std::string(reinterpret_cast<const char*>(sqlite3_value_text(argv[0])) ?: "").empty();
            else if(type == SQLITE_BLOB)
                empty = NULL == sqlite3_value_blob(argv[0]);

            sqlite3_result_int(context, empty ? 1 : 0);
        }

        // IS_NOT_EMPTY(<value>) -> BOOL
        static void sqlite3_user_is_not_empty(sqlite3_context *context, int argc, sqlite3_value **argv) {
            if(argc != 1) {
                sqlite3_result_error(context, "too few argments", -1);
                return;
            }

            int type = sqlite3_value_type(argv[0]);

            bool empty = false;
            if(type == SQLITE_INTEGER)
                empty = 0 == sqlite3_value_int(argv[0]);
            else if(type == SQLITE_NULL)
                empty = true;
            else if(type == SQLITE_TEXT)
                empty = std::string(reinterpret_cast<const char*>(sqlite3_value_text(argv[0]))).empty();
            else if(type == SQLITE_BLOB)
                empty = NULL == sqlite3_value_blob(argv[0]);

            sqlite3_result_int(context, empty ? 0 : 1);
        }
    };

    sqlite3_create_function(database.get(), "IS_EMPTY",     1, SQLITE_UTF8, nullptr, userfunc::sqlite3_user_is_empty,     nullptr, nullptr);
    sqlite3_create_function(database.get(), "IS_NOT_EMPTY", 1, SQLITE_UTF8, nullptr, userfunc::sqlite3_user_is_not_empty, nullptr, nullptr);

    return std::move(database);
};

database_ptr sqlite3_open(int uid, sqlite3_context *context)
{
    database_ptr database(::sqlite3_context_db_handle(context), [](sqlite3 *db){
        // AV: do not close DB - it is opened by FMDatabase
    });

    return database;
};

statement_ptr sqlite3_prepare(database_ptr &database, int uid, const std::string &sql)
{
    DATABASE_LOG(uid, sql);

    sqlite3_stmt *stmt = nullptr;
    auto rc = ::sqlite3_prepare_v2(database.get(), sql.c_str(), -1, &stmt, nullptr);
    if(SQLITE_OK != rc) {
        DATABASE_LOG(uid, "sqlite3_prepare_v2(): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));
        assert(false);
    }

#ifdef DEBUG
    std::map<std::string, bool> binds;
    auto count = ::sqlite3_bind_parameter_count(stmt);
    for(int index = 1; index <= count; ++ index) {
        auto name = ::sqlite3_bind_parameter_name(stmt, index);
        binds[name] = false;
    }

    statement_ptr statement(new statement_type{stmt, binds}, [](statement_type *stmt){
        if(stmt) ::sqlite3_finalize(stmt->first);
    });
#else /*DEBUG*/
    statement_ptr statement(stmt, [](sqlite3_stmt *stmt){
        if(stmt) ::sqlite3_finalize(stmt);
    });
#endif /*DEBUG*/

    return statement;
};


statement_ptr sqlite3_prepare(database_ptr &database, int uid, const std::stringstream &sql)
{
    return ::sqlite3_prepare(database, uid, sql.str());
};

static inline sqlite3_stmt *sqlite3_statement(statement_ptr &statement)
{
#ifdef DEBUG
    return statement.get()->first;
#else /*DEBUG*/
    return statement.get();
#endif /*DEBUG*/
};

static inline int sqlite3_bind_index(statement_ptr &statement, const std::string &name)
{
#ifdef DEBUG
    auto iter = statement.get()->second.find(name);
    if(std::end(statement.get()->second) == iter) {
        DATABASE_LOG(0, "sqlite3_bind_index(" << name << "): can't find SQL parameter");
        assert(false);
    } else if(iter->second) {
        DATABASE_LOG(0, "sqlite3_bind_index(" << name << "): SQL parameter is already bound");
        assert(false);
    } else {
        iter->second = true;
    }
    return ::sqlite3_bind_parameter_index(::sqlite3_statement(statement), name.c_str());
#else /*DEBUG*/
    return ::sqlite3_bind_parameter_index(::sqlite3_statement(statement), name.c_str());
#endif /*DEBUG*/
};

void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, value_ptr &value)
{
    auto rc = ::sqlite3_bind_value(::sqlite3_statement(statement), ::sqlite3_bind_index(statement, name), value.get());
    if(SQLITE_OK != rc) {
        DATABASE_LOG(uid, "sqlite3_bind_value(" << name << "): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));
        assert(false);
    }
};

void sqlite3_bind_null(database_ptr &database, statement_ptr &statement, int uid, const std::string &name)
{
    DATABASE_LOG(uid, "        " << name << " ==> NULL");
    auto rc = ::sqlite3_bind_null(::sqlite3_statement(statement), ::sqlite3_bind_index(statement, name));
    if(SQLITE_OK != rc) {
        DATABASE_LOG(uid, "sqlite3_bind_null(" << name << "): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));
        assert(false);
    }
};
void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, const std::string &value)
{
    DATABASE_LOG(uid, "        " << name << " ==> " << ::sqlite3_escape(value));
    auto rc = ::sqlite3_bind_text(::sqlite3_statement(statement), ::sqlite3_bind_index(statement, name), value.c_str(), -1, SQLITE_TRANSIENT);
    if(SQLITE_OK != rc) {
        DATABASE_LOG(uid, "sqlite3_bind_text(" << name << "): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));
        assert(false);
    }
};
void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, int value)
{
    DATABASE_LOG(uid, "        " << name << " ==> " << ::sqlite3_escape(value));
    auto rc = ::sqlite3_bind_int(::sqlite3_statement(statement), ::sqlite3_bind_index(statement, name), value);
    if(SQLITE_OK != rc) {
        DATABASE_LOG(uid, "sqlite3_bind_int(" << name << "): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));
        assert(false);
    }
};
void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, int64_t value)
{
    DATABASE_LOG(uid, "        " << name << " ==> " << ::sqlite3_escape(value));
    auto rc = ::sqlite3_bind_int64(::sqlite3_statement(statement), ::sqlite3_bind_index(statement, name), value);
    if(SQLITE_OK != rc) {
        DATABASE_LOG(uid, "sqlite3_bind_int(" << name << "): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));
        assert(false);
    }
};
void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, std::size_t value)
{
    DATABASE_LOG(uid, "        " << name << " ==> " << ::sqlite3_escape(value));
    auto rc = ::sqlite3_bind_int64(::sqlite3_statement(statement), ::sqlite3_bind_index(statement, name), static_cast<int64_t>(value));
    if(SQLITE_OK != rc) {
        DATABASE_LOG(uid, "sqlite3_bind_int(" << name << "): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));
        assert(false);
    }
};
void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, double value)
{
    DATABASE_LOG(uid, "        " << name << " ==> " << ::sqlite3_escape(value));
    auto rc = ::sqlite3_bind_double(::sqlite3_statement(statement), ::sqlite3_bind_index(statement, name), value);
    if(SQLITE_OK != rc) {
        DATABASE_LOG(uid, "sqlite3_bind_double(" << name << "): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));
        assert(false);
    }
};
void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, const std::vector<char> &value)
{
    DATABASE_LOG(uid, "        " << name << " ==> BLOB#" << value.size());
    auto rc = ::sqlite3_bind_blob(::sqlite3_statement(statement), ::sqlite3_bind_index(statement, name), &value.front(), static_cast<int>(value.size()), nullptr);
    if(SQLITE_OK != rc) {
        DATABASE_LOG(uid, "sqlite3_bind_blob(" << name << "): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));
        assert(false);
    }
};

value_ptr sqlite3_column(statement_ptr &statement, const std::string &name)
{
    auto count = ::sqlite3_column_count(::sqlite3_statement(statement));
    for(int column=0; column<count; ++column) {
        auto colname = ::sqlite3_column_name(::sqlite3_statement(statement), column);
        if(colname == name) return value_ptr{::sqlite3_column_value(::sqlite3_statement(statement), column), [](sqlite3_value*){}};
    }
    assert(false);
    return value_ptr{};
};

int sqlite3_int(statement_ptr &statement, const std::string &name, int def)
{
    auto value = ::sqlite3_column(statement, name);
    return value ? ::sqlite3_value_int(value.get()) : def;
};

int64_t sqlite3_uid(statement_ptr &statement, const std::string &name, int64_t def)
{
    auto value = ::sqlite3_column(statement, name);
    return value ? ::sqlite3_value_int64(value.get()) : def;
};

size_t sqlite3_size(statement_ptr &statement, const std::string &name, size_t def)
{
    auto value = ::sqlite3_column(statement, name);
    return value ? ::sqlite3_value_int64(value.get()) : def;
};

long long sqlite3_longlong(statement_ptr &statement, const std::string &name, long long def)
{
    auto value = ::sqlite3_column(statement, name);
    return value ? ::sqlite3_value_int64(value.get()) : def;
};

double sqlite3_float(statement_ptr &statement, const std::string &name, double def)
{
    auto value = ::sqlite3_column(statement, name);
    return value ? ::sqlite3_value_double(value.get()) : def;
};

std::string sqlite3_string(statement_ptr &statement, const std::string &name, const std::string &def)
{
    auto value = ::sqlite3_column(statement, name);
    auto str = ::sqlite3_value_text(value.get());
    return value && str ? std::string((const char*)str) : def;
};

std::vector<char> sqlite3_binary(statement_ptr &statement, const std::string &name)
{
    std::vector<char> data;

    auto value = ::sqlite3_column(statement, name);
    auto size = ::sqlite3_value_bytes(value.get());

    if(size) {
        data.resize(size);
        ::memcpy(&data.front(), ::sqlite3_value_blob(value.get()), size);
    }

    return data;
};

bool sqlite3_execute(database_ptr &database, int uid, statement_ptr &statement, bool ignore)
{
#ifdef DEBUG
    std::for_each(std::begin(statement.get()->second), std::end(statement.get()->second), [=](const decltype(statement.get()->second)::value_type &pair){
        if(!pair.second) {
            DATABASE_LOG(0, "sqlite3_bind_index(" << pair.first << "): SQL parameter isn't bound");
            assert(false);
        }
    });
#endif /*DEBUG*/

    auto rc = ::sqlite3_step(::sqlite3_statement(statement));

#ifdef DEBUG
    static sqlite3_stmt *prev_statement = nullptr;
    static int           row_index = 0;
    static const int     max_index = 1000;
    sqlite3_stmt        *next_statement = ::sqlite3_statement(statement);

    if(row_index < max_index || prev_statement != next_statement) {
        if(prev_statement != next_statement) {
            prev_statement = next_statement;
            row_index      = 1;
        } else {
            ++ row_index;
        }

        if(SQLITE_OK == rc || SQLITE_DONE == rc) {
            DATABASE_LOG(uid, "#" << row_index << ": " << rc << " (" << ::sqlite3_changes(database.get()) << ", #" << ::sqlite3_last_insert_rowid(database.get()) << ")");
        } else if(SQLITE_ROW == rc) {
            std::stringstream log;
            log << "#" << row_index << ": " << rc << " (" << ::sqlite3_changes(database.get()) << ", #" << ::sqlite3_last_insert_rowid(database.get()) << ") {" << std::endl;

            auto count = ::sqlite3_column_count(next_statement);
            for(int column=0; column<count; ++column) {
                auto name = ::sqlite3_column_name(next_statement, column);
                switch(::sqlite3_column_type(next_statement, column)) {
                    case SQLITE_INTEGER: log << "        " << name << " => " << _(::sqlite3_column_int(next_statement, column)) << std::endl; break;
                    case SQLITE_FLOAT: log << "        " << name << " => " << _(::sqlite3_column_double(next_statement, column)) << std::endl; break;
                    case SQLITE_BLOB: log << "        " << name << " => BLOB#" << ::sqlite3_column_bytes(next_statement, column) << std::endl; break;
                    case SQLITE_NULL: log << "        " << name << " => NULL" << std::endl; break;
                    case SQLITE_TEXT: log << "        " << name << " => " << _((const char*)::sqlite3_column_text(next_statement, column)) << std::endl; break;
                }
            }

            log << "}";
            DATABASE_LOG(uid, log.str());
        } else {
            DATABASE_LOG(uid, "sqlite3_step(): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));
            assert(false);
        }
    } else if(row_index == max_index) {
        ++ row_index;
        DATABASE_LOG(uid, "#" << row_index << ": Too many rows");
    }
#endif /*DEBUG*/

    if (ignore == false) {
        if (SQLITE_OK == rc || SQLITE_DONE == rc || SQLITE_ROW == rc) {
            //  it's ok
        } else {
            DATABASE_LOG(uid, "sqlite3_step(): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));

            assert(false);
        }
    }

    return SQLITE_ROW == rc;
};

void sqlite3_reset(statement_ptr &statement)
{
#ifdef DEBUG
    std::for_each(std::begin(statement.get()->second), std::end(statement.get()->second), [=](decltype(statement.get()->second)::value_type &pair){
        pair.second = false;
    });
#endif /*DEBUG*/

    ::sqlite3_reset(::sqlite3_statement(statement));
};

void sqlite3_validate_database(const std::string &path)
{
    auto uid = -1;
    sqlite3* db = nullptr;
    auto rc = ::sqlite3_open(path.c_str(), &db);
    if(SQLITE_OK != rc) {
        DATABASE_LOG(uid, "sqlite3_open(" << path << "): #" << rc);
    }

    database_ptr database(db, [=](sqlite3 *db){
        DATABASE_LOG(uid, "CLOSE DATABASE '" << path << "';");
        if(db) ::sqlite3_close(db);
    });

    DATABASE_LOG(uid, "OPEN DATABASE '" << path << "';");

    std::string sql = "PRAGMA integrity_check;";

    sqlite3_stmt *stmt = nullptr;
    rc = ::sqlite3_prepare_v2(database.get(), sql.c_str(), -1, &stmt, nullptr);
    if(SQLITE_OK != rc) {
        DATABASE_LOG(uid, "sqlite3_prepare_v2(): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));
    }

    using statement_ptr = std::unique_ptr<sqlite3_stmt, std::function<void(sqlite3_stmt*)>>;
    statement_ptr statement(stmt, [](sqlite3_stmt *stmt){
        if(stmt) ::sqlite3_finalize(stmt);
    });

    rc = ::sqlite3_step(statement.get());

    if (SQLITE_OK != rc && SQLITE_DONE != rc && SQLITE_ROW != rc) {
        DATABASE_LOG(uid, "sqlite3_step(): #" << rc << ' ' << ::sqlite3_errmsg(database.get()));
    }
};

void sqlite3_drop_table(database_ptr &database, int uid, const std::string &table)
{
    std::stringstream sql;
    sql << "DROP TABLE IF EXISTS [" << table << "];";
    ::sqlite3_execute(database, uid, sql);
};

bool sqlite3_is_table_exists(database_ptr &database, int uid, const std::string &prefix, const std::string &table)
{
    std::string sqlite_master;
    if(prefix.empty()) {
        sqlite_master = "[sqlite_master]";
    } else {
        sqlite_master = "[" + prefix + "].[sqlite_master]";
    }

    std::stringstream sql;
    sql << "SELECT COUNT(i.[name]) AS [count] ";
    sql << "FROM " << sqlite_master << " i ";
    sql << "WHERE (i.[type] = " << _("table") << ") AND ";
    sql << "      (i.[name] = " << _(table) << ")";

    auto count = ::sqlite3_count(database, uid, sql);
    return 0 < count;
};

bool sqlite3_is_column_exists(database_ptr &database, int uid, const std::string &prefix, const std::string &table, const std::string &column)
{
    bool exists = false;

    std::string table_info;
    if(prefix.empty()) {
        table_info = "[table_info]";
    } else {
        table_info = "[" + prefix + "].[table_info]";
    }

    std::stringstream sql;
    sql << "PRAGMA " << table_info << "(" << _(table) << ");";

    ::sqlite3_foreach(database, uid, sql, [=, &exists](statement_ptr &statement){
        std::string name = ::sqlite3_string(statement, "name");
        exists |= name == column;
    });

    return exists;
};
