#pragma once

#include "sqlite3.h"
#include <set>
#include <unordered_set>
#include <sstream>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

//#import "SQLiteDatabase.h"


#ifdef DEBUG
#   define DATABASE_LOG(uid, data) (std::cerr << "DATABASE[" << uid << "]: " << data << std::endl)
#else
#   define DATABASE_LOG(uid, data)
#endif /*DEBUG*/

//@class SQLiteClient;
//@class FMDatabase;


using database_ptr = std::unique_ptr<sqlite3, std::function<void(sqlite3*)>>;
#ifdef DEBUG
using statement_type = std::pair<sqlite3_stmt*, std::map<std::string, bool>>;
using statement_ptr = std::unique_ptr<statement_type, std::function<void(statement_type*)>>;
#else /*DEBUG*/
using statement_ptr = std::unique_ptr<sqlite3_stmt, std::function<void(sqlite3_stmt*)>>;
#endif /*DEBUG*/
using value_ptr = std::unique_ptr<sqlite3_value, std::function<void(sqlite3_value*)>>;


namespace database {

    void processChanges(database_ptr &database, int uid, const std::set<long long> &old_ids, const std::string &table);
};


extern std::string sqlite3_escape(const std::string &input);
extern std::string sqlite3_escape_like(const std::string &input, char escape);
inline int sqlite3_escape(int input) { return input; };
inline double sqlite3_escape(double input) { return input; };
inline int64_t sqlite3_escape(int64_t input) { return input; };
inline std::size_t sqlite3_escape(std::size_t input) { return input; };

inline std::string sqlite3_escape(const char *input) {
    return ::sqlite3_escape(std::string{input && *input ? input : ""});
};
inline int sqlite3_escape(time_t timestamp) {
    return static_cast<int>(timestamp);
};

template <typename value>
inline std::string sqlite3_escape(const std::deque<value> &values, bool brackets) {
    if (values.empty()) {
        if (brackets) {
            return "(0)";
        } else {
            return "0";
        }
    } else {
        std::stringstream sql;

        if (brackets) {
            sql << "(";
        }

        bool first = true;
        std::for_each(std::begin(values), std::end(values), [&sql, &first](value val){
            if(!first) sql << ",";
            sql << ::sqlite3_escape(val);
            first = false;
        });

        if (brackets) {
            sql << ")";
        }

        return sql.str();
    }
};

template <typename value>
inline std::string sqlite3_escape(const std::deque<value> &values) {
    return ::sqlite3_escape(values, true);
};

template <typename value>
inline std::string sqlite3_escape(const std::set<value> &values) {
    if(values.empty()) return "(0)";

    std::stringstream sql;
    sql << '(';

    bool first = true;
    std::for_each(std::begin(values), std::end(values), [&sql, &first](value val){
        if(!first) sql << ',';
        sql << ::sqlite3_escape(val);
        first = false;
    });

    sql << ')';
    return sql.str();
};

template <typename value>
inline std::string sqlite3_escape(const std::unordered_set<value> &values) {
    if(values.empty()) return "(0)";

    std::stringstream sql;
    sql << '(';

    bool first = true;
    std::for_each(std::begin(values), std::end(values), [&sql, &first](value val){
        if(!first) sql << ',';
        sql << ::sqlite3_escape(val);
        first = false;
    });

    sql << ')';
    return sql.str();
};

template <typename value>
inline std::string sqlite3_escape(const std::initializer_list<value> &values) {
    if(0 == values.size()) return "(0)";

    std::stringstream sql;
    sql << '(';

    bool first = true;
    std::for_each(std::begin(values), std::end(values), [&sql, &first](value val){
        if(!first) sql << ',';
        sql << ::sqlite3_escape(val);
        first = false;
    });

    sql << ')';
    return sql.str();
};


extern database_ptr sqlite3_open(int uid, const std::string &path);
extern database_ptr sqlite3_open(int uid, sqlite3_context *context);
extern statement_ptr sqlite3_prepare(database_ptr &database, int uid, const std::stringstream &sql);
extern statement_ptr sqlite3_prepare(database_ptr &database, int uid, const std::string &sql);

extern value_ptr sqlite3_column(statement_ptr &statement, const std::string &name);
extern int sqlite3_int(statement_ptr &statement, const std::string &name, int def = 0);
extern int64_t sqlite3_uid(statement_ptr &statement, const std::string &name, int64_t def = 0);
extern size_t sqlite3_size(statement_ptr &statement, const std::string &name, size_t def = 0);
extern long long sqlite3_longlong(statement_ptr &statement, const std::string &name, long long def = 0);
extern double sqlite3_float(statement_ptr &statement, const std::string &name, double def = 0);
extern std::string sqlite3_string(statement_ptr &statement, const std::string &name, const std::string &def = "");
extern std::vector<char> sqlite3_binary(statement_ptr &statement, const std::string &name);

inline std::string sqlite3_url(statement_ptr &statement, const std::string &name) {
    return ::sqlite3_string(statement, name);
};

inline bool sqlite3_bool(statement_ptr &statement, const std::string &name) {
    return 0 != ::sqlite3_int(statement, name);
};
inline time_t sqlite3_timestamp(statement_ptr &statement, const std::string &name) {
    return ::sqlite3_int(statement, name);
};

inline boost::filesystem::path sqlite3_path(statement_ptr &statement, const std::string &name, bool required = true) {
    auto path = boost::filesystem::path(::sqlite3_string(statement, name));
//    auto path = boost::filesystem::to_path(::sqlite3_string(statement, name));
    if(required && !boost::filesystem::exists(path)) {
        return {};
    } else {
        return path;
    }
};

extern bool sqlite3_execute(database_ptr &database, int uid, statement_ptr &statement, bool ignore = false);
extern void sqlite3_reset(statement_ptr &statement);

extern void sqlite3_validate_database(const std::string &path);

extern void sqlite3_attach_database(database_ptr &database, int uid, const std::string &path);
extern void sqlite3_detach_database(database_ptr &database, int uid);

extern void sqlite3_bind_null(database_ptr &database, statement_ptr &statement, int uid, const std::string &name);
extern void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, value_ptr &value);
extern void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, const std::string &value);
extern void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, int value);
extern void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, int64_t value);
extern void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, std::size_t value);
extern void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, double value);
extern void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, const std::vector<char> &value);

inline void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, const char *value) {
    ::sqlite3_bind(database, statement, uid, name, value && *value ? std::string{value} : std::string{});
};
inline void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, bool value) {
    ::sqlite3_bind(database, statement, uid, name, static_cast<int>(value ? 1 : 0));
};
inline void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, time_t value) {
    ::sqlite3_bind(database, statement, uid, name, static_cast<int>(value));
};
inline void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, float value) {
    ::sqlite3_bind(database, statement, uid, name, static_cast<double>(value));
};

inline void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, const boost::filesystem::path &path) {
    ::sqlite3_bind(database, statement, uid, name, path.string());
};

template <typename T>
struct Nullable {
private:
    bool _valid;
    T _value;

public:
    Nullable(T v) : _valid{!!v}, _value(v) {};

    bool operator!(void) const {
        return !this->_valid;
    };
    const T operator*(void) const {
        assert(this->_valid);
        return this->_value;
    };
};
template <typename T>
inline Nullable<T> nullable(T value) {
    return Nullable<T>(value);
};

template <typename T>
inline void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, const Nullable<T> &nullable) {
    if(!nullable) ::sqlite3_bind_null(database, statement, uid, name);
    else          ::sqlite3_bind(database, statement, uid, name, *nullable);
};

template <typename T>
inline std::string sqlite3_escape(const Nullable<T> &nullable) {
    if(!nullable) return "null";
    else          return std::to_string(::sqlite3_escape(*nullable));
};

inline void sqlite3_bind(database_ptr &database, statement_ptr &statement, int uid, const std::string &name, std::nullptr_t) {
    ::sqlite3_bind_null(database, statement, uid, name);
};

extern void sqlite3_drop_table(database_ptr &database, int uid, const std::string &table);
extern bool sqlite3_is_table_exists(database_ptr &database, int uid, const std::string &prefix, const std::string &table);
extern bool sqlite3_is_column_exists(database_ptr &database, int uid, const std::string &prefix, const std::string &table, const std::string &column);

inline void sqlite3_bind_helper(database_ptr &database, statement_ptr &statement, int uid) {
    /* do nothing */
};
template <typename key, typename value>
inline void sqlite3_bind_helper(database_ptr &database, statement_ptr &statement, int uid, const std::pair<key, boost::optional<value>> &item) {
    if(item.second) {
        ::sqlite3_bind(database, statement, uid, std::string(":") + item.first, *item.second);
    }
};
template <typename key, typename value>
inline void sqlite3_bind_helper(database_ptr &database, statement_ptr &statement, int uid, const std::pair<key, value> &item) {
    ::sqlite3_bind(database, statement, uid, std::string(":") + item.first, item.second);
};
template <typename key, typename value>
inline void sqlite3_bind_helper(database_ptr &database, statement_ptr &statement, int uid, const boost::optional<std::pair<key, value>> &item) {
    if(item) {
        ::sqlite3_bind_helper(database, statement, uid, *item);
    } else {
        /* do nothing - item hasn't value */
    }
};
template <typename key, typename value, typename ...fields>
inline void sqlite3_bind_helper(database_ptr &database, statement_ptr &statement, int uid, const std::pair<key, value> &item, const fields&... items) {
    ::sqlite3_bind_helper(database, statement, uid, item);
    ::sqlite3_bind_helper(database, statement, uid, items...);
};
template <typename key, typename value, typename ...fields>
inline void sqlite3_bind_helper(database_ptr &database, statement_ptr &statement, int uid, const boost::optional<std::pair<key, value>> &item, const fields&... items) {
    ::sqlite3_bind_helper(database, statement, uid, item);
    ::sqlite3_bind_helper(database, statement, uid, items...);
};


inline void sqlite3_update_helper(std::stringstream &sql, bool first) {
};
template <typename field>
inline void sqlite3_update_helper(std::stringstream &sql, bool first, const field &item) {
    if(!first) sql << ", ";
    sql << "[" << item.first << "] = :" << item.first;
};
template <typename field, typename ...fields>
inline void sqlite3_update_helper(std::stringstream &sql, bool first, const field &item, const fields&... items) {
    ::sqlite3_update_helper(sql, first, item);
    ::sqlite3_update_helper(sql, false, items...);
};
template <typename ...fields>
inline void sqlite3_update_id(database_ptr &database, int uid, const std::string &table, int_fast64_t item_id, const fields&... items) {
    std::stringstream sql;
    sql << "UPDATE [" << table << "] SET ";
    ::sqlite3_update_helper(sql, true, items...);
    sql << " WHERE ([id] = :item_id) ";

    statement_ptr statement = ::sqlite3_prepare(database, uid, sql);
    ::sqlite3_bind_helper(database, statement, uid, items...);
    ::sqlite3_bind(database, statement, uid, ":item_id", item_id);

    ::sqlite3_execute(database, uid, statement);
};
template <typename object_uid, typename ...fields>
inline void sqlite3_update_uid(database_ptr &database, int uid, const std::string &table, const object_uid &item_uid, const fields&... items) {
    std::stringstream sql;
    sql << "UPDATE [" << table << "] SET ";
    ::sqlite3_update_helper(sql, true, items...);
    sql << " WHERE ([id] = :item_uid) ";

    statement_ptr statement = ::sqlite3_prepare(database, uid, sql);
    ::sqlite3_bind_helper(database, statement, uid, items...);
    ::sqlite3_bind(database, statement, uid, ":item_uid", item_uid);

    ::sqlite3_execute(database, uid, statement);
};


template <typename field>
inline void sqlite3_insert_column_helper(std::stringstream &sql, bool first, const field &item) {
    if(!first) sql << ", ";
    sql << "[" << item.first << "]";
};
template <typename field, typename ...fields>
inline void sqlite3_insert_column_helper(std::stringstream &sql, bool first, const field &item, const fields&... items) {
    ::sqlite3_insert_column_helper(sql, first, item);
    ::sqlite3_insert_column_helper(sql, false, items...);
};
template <typename field>
inline void sqlite3_insert_value_helper(std::stringstream &sql, bool first, const field &item) {
    if(!first) sql << ", ";
    sql << ':' << item.first;
};
template <typename field, typename ...fields>
inline void sqlite3_insert_value_helper(std::stringstream &sql, bool first, const field &item, const fields&... items) {
    ::sqlite3_insert_value_helper(sql, first, item);
    ::sqlite3_insert_value_helper(sql, false, items...);
};
template <typename ...fields>
inline int_fast64_t sqlite3_insert_id(database_ptr &database, int uid, const std::string &table, int_fast64_t item_id, const fields&... items) {
    std::stringstream sql;
    sql << "INSERT OR IGNORE INTO [" << table << "] (";
    ::sqlite3_insert_column_helper(sql, true, std::make_pair("id", item_id), items...);
    sql << ") VALUES (";
    ::sqlite3_insert_value_helper(sql, true, std::make_pair("id", item_id), items...);
    sql << ")";

    statement_ptr statement = ::sqlite3_prepare(database, uid, sql);
    ::sqlite3_bind_helper(database, statement, uid, std::make_pair("id", item_id), items...);

    ::sqlite3_execute(database, uid, statement);

    ::sqlite3_update_id(database, uid, table, item_id, items...);
    return item_id;
};
inline int_fast64_t sqlite3_insert_id(database_ptr &database, int uid, const std::string &table, int_fast64_t item_id) {
    std::stringstream sql;
    sql << "INSERT OR IGNORE INTO [" << table << "] (";
    ::sqlite3_insert_column_helper(sql, true, std::make_pair("id", item_id));
    sql << ") VALUES (";
    ::sqlite3_insert_value_helper(sql, true, std::make_pair("id", item_id));
    sql << ")";

    statement_ptr statement = ::sqlite3_prepare(database, uid, sql);
    ::sqlite3_bind_helper(database, statement, uid, std::make_pair("id", item_id));

    ::sqlite3_execute(database, uid, statement);
    return item_id;
};
template <typename object_uid, typename ...fields>
inline object_uid sqlite3_insert_uid(database_ptr &database, int uid, const std::string &table, const object_uid &item_id, const fields&... items) {
    std::stringstream sql;
    sql << "INSERT OR IGNORE INTO [" << table << "] (";
    ::sqlite3_insert_column_helper(sql, true, std::make_pair("id", item_id), items...);
    sql << ") VALUES (";
    ::sqlite3_insert_value_helper(sql, true, std::make_pair("id", item_id), items...);
    sql << ")";

    statement_ptr statement = ::sqlite3_prepare(database, uid, sql);
    ::sqlite3_bind_helper(database, statement, uid, std::make_pair("id", item_id), items...);

    ::sqlite3_execute(database, uid, statement);

    ::sqlite3_update_uid(database, uid, table, item_id, items...);
    return item_id;
};
template <typename object_uid>
inline object_uid sqlite3_insert_uid(database_ptr &database, int uid, const std::string &table, const object_uid &item_id) {
    std::stringstream sql;
    sql << "INSERT OR IGNORE INTO [" << table << "] (";
    ::sqlite3_insert_column_helper(sql, true, std::make_pair("id", item_id));
    sql << ") VALUES (";
    ::sqlite3_insert_value_helper(sql, true, std::make_pair("id", item_id));
    sql << ")";

    statement_ptr statement = ::sqlite3_prepare(database, uid, sql);
    ::sqlite3_bind_helper(database, statement, uid, std::make_pair("id", item_id));

    ::sqlite3_execute(database, uid, statement);
    return item_id;
};
template <typename ...fields>
inline void sqlite3_insert(database_ptr &database, int uid, const std::string &table, const fields&... items) {
    std::stringstream sql;
    sql << "INSERT OR REPLACE INTO [" << table << "] (";
    ::sqlite3_insert_column_helper(sql, true, items...);
    sql << ") VALUES (";
    ::sqlite3_insert_value_helper(sql, true, items...);
    sql << ")";

    statement_ptr statement = ::sqlite3_prepare(database, uid, sql);
    ::sqlite3_bind_helper(database, statement, uid, items...);
    ::sqlite3_execute(database, uid, statement);
};


template <typename result, typename ...fields>
inline result sqlite3_fetch(database_ptr &database, int uid, const std::stringstream &sql, std::function<result(statement_ptr&)> callback, const fields&... items) {
    auto statement = ::sqlite3_prepare(database, uid, sql);
    ::sqlite3_bind_helper(database, statement, uid, items...);
    if(!::sqlite3_execute(database, uid, statement)) {
        statement_ptr stmt;
        return callback(stmt);
    } else return callback(statement);
};

template <typename result, typename ...fields>
inline boost::optional<result> sqlite3_fetch_optional(database_ptr &database, int uid, const std::stringstream &sql, std::function<boost::optional<result>(statement_ptr&)> callback, const fields&... items) {
    auto statement = ::sqlite3_prepare(database, uid, sql);
    ::sqlite3_bind_helper(database, statement, uid, items...);
    if(!::sqlite3_execute(database, uid, statement)) {
        statement_ptr stmt;
        return callback(stmt);
    } else return callback(statement);
};

template <typename ...fields>
inline void sqlite3_foreach(database_ptr &database, int uid, const std::stringstream &sql, std::function<void(statement_ptr&)> callback, const fields&... items)
{
    auto statement = ::sqlite3_prepare(database, uid, sql);
    ::sqlite3_bind_helper(database, statement, uid, items...);
    while(::sqlite3_execute(database, uid, statement))
        callback(statement);
};

template <typename string, typename ...fields>
inline void sqlite3_execute(database_ptr &database, int uid, const string &sql, const fields&... items)
{
    auto statement = ::sqlite3_prepare(database, uid, sql);
    ::sqlite3_bind_helper(database, statement, uid, items...);
    ::sqlite3_execute(database, uid, statement);
};

template <typename field>
inline void sqlite3_uid_helper(std::stringstream &stream, bool first, const field &item) {
    if(!first) stream << '-';
    stream << item.first;
};
template <typename field, typename ...fields>
inline void sqlite3_uid_helper(std::stringstream &stream, bool first, const field &item, const fields&... items) {
    ::sqlite3_uid_helper(stream, first, item);
    ::sqlite3_uid_helper(stream, false, items...);
};

template <typename ...fields>
int sqlite3_count(database_ptr &database, int uid, const std::stringstream &sql, const fields&... items)
{
    return ::sqlite3_fetch<int>(database, uid, sql, [](statement_ptr &statement)->int{
        return statement ? ::sqlite3_int(statement, "count") : 0;
    }, items...);
};

template <typename result, typename ...fields>
result sqlite3_fetch_id(database_ptr &database, int uid, const std::stringstream &sql, const fields&... items) {
    return ::sqlite3_fetch<result>(database, uid, sql, [](statement_ptr &statement)->result{
        return static_cast<result>(statement ? ::sqlite3_longlong(statement, "id") : 0);
    }, items...);
};

template <typename object>
object sqlite3_fetch(database_ptr &database, int uid, const std::string &table, typename object::UID objectId, std::function<object(statement_ptr&)> callback, std::false_type) {
    std::stringstream sql;

    sql << "SELECT x.* ";
    sql << "FROM [" << table << "] x ";
    sql << "WHERE (x.[id] = :object_id) ";
    sql << "LIMIT 1; ";

    return ::sqlite3_fetch<object>(database, uid, sql, callback,
                                   std::make_pair("object_id", objectId));
};

template <typename object>
boost::optional<object> sqlite3_fetch_optional(database_ptr &database, int uid, const std::string &table, typename object::UID objectId, std::function<boost::optional<object>(statement_ptr&)> callback, std::false_type) {
    std::stringstream sql;

    sql << "SELECT x.* ";
    sql << "FROM [" << table << "] x ";
    sql << "WHERE (x.[id] = :object_id) ";
    sql << "LIMIT 1; ";

    return ::sqlite3_fetch<boost::optional<object>>(database, uid, sql, callback,
                                                    std::make_pair("object_id", objectId));
};

template <typename object>
void sqlite3_delete(database_ptr &database, int uid, const std::string &table, typename object::UID objectId, std::false_type) {
    std::stringstream sql;

    sql << "DELETE FROM [" << table << "] ";
    sql << "WHERE ([id] = :object_id); ";

    ::sqlite3_execute(database, uid, sql,
                      std::make_pair("object_id", objectId));
};

template <typename ...fields>
bool sqlite3_exists(database_ptr &database, int uid, const std::stringstream &sql, const fields&... items) {
    std::stringstream full;
    full << "SELECT EXISTS(" << sql.str() << " LIMIT 1) AS [exists]; ";

    return ::sqlite3_fetch<bool>(database, uid, full, [](statement_ptr &statement)->bool{
        return (statement ? ::sqlite3_bool(statement, "exists") : false);
    }, items...);
};

template <typename object>
bool sqlite3_exists(database_ptr &database, int uid, const std::string &table, typename object::UID objectId, std::false_type) {
    std::stringstream sql;
    sql << "SELECT x.[id] ";
    sql << "FROM [" << table << "] x ";
    sql << "WHERE (x.[id] = :object_id) ";

    return ::sqlite3_exists(database, uid, sql,
                            std::make_pair("object_id", objectId));
};

template <typename container, typename ...fields>
inline container sqlite3_fetch(database_ptr &database, int uid, const std::stringstream &sql, std::function<typename container::value_type(statement_ptr&)> callback, const fields&... items) {
    container objects;

    ::sqlite3_foreach(database, uid, sql, [=, &objects](statement_ptr &statement){
        auto object = callback(statement);
        objects.insert(std::end(objects), object);
    }, items...);

    return objects;
};

template <typename container, typename ...fields>
inline container sqlite3_fetch_optional(database_ptr &database, int uid, const std::stringstream &sql, std::function<boost::optional<typename container::value_type>(statement_ptr&)> callback, const fields&... items) {
    container objects;

    ::sqlite3_foreach(database, uid, sql, [=, &objects](statement_ptr &statement){
        if(auto object = callback(statement)) {
            objects.insert(std::end(objects), *object);
        }
    }, items...);

    return objects;
};

template <typename container, typename ...fields>
container sqlite3_fetch_ids(database_ptr &database, int uid, const std::stringstream &sql, const fields&... items) {
    using object_uid = typename container::value_type;
    return ::sqlite3_fetch<container>(database, uid, sql, [=](statement_ptr &statement)->object_uid{
        return statement ? object_uid{::sqlite3_uid(statement, "id")} : object_uid{};
    }, items...);
};

template <typename container, typename ...fields>
container sqlite3_fetch_optional_ids(database_ptr &database, int uid, const std::stringstream &sql, const fields&... items) {
    using object_uid = typename container::value_type;
    return ::sqlite3_fetch_optional<container>(database, uid, sql, [=](statement_ptr &statement)->boost::optional<object_uid>{
        if(statement) {
            return object_uid{::sqlite3_uid(statement, "id")};
        } else {
            return boost::none;
        }
    }, items...);
};

//template <typename ...fields>
//inline NSArray* sqlite3_fetch_array(database_ptr &database, int uid, const std::stringstream &sql, std::function<NSObject*(statement_ptr&)> callback, const fields&... items) {
//    NSMutableArray *objects = [NSMutableArray array];

//    ::sqlite3_foreach(database, uid, sql, [=](statement_ptr &statement){
//        auto object = callback(statement);
//        if(!!object) [objects addObject:object];
//    }, items...);

//    return objects;
//};

template <typename ...fields>
inline void sqlite3_upsert_id(database_ptr &database, int uid, const std::string &table, int_fast64_t item_id, const fields&... items) {
    std::stringstream sql;
    sql << "SELECT x.[id] AS [id] ";
    sql << "FROM [" << table << "] x ";
    sql << "WHERE (x.[id] = :item_id) ";

    auto exists = ::sqlite3_exists(database, uid, sql, std::make_pair("item_id", item_id));
    if(exists) {
        ::sqlite3_update_id(database, uid, table, item_id, items...);
    } else {
        ::sqlite3_insert_id(database, uid, table, item_id, items...);
    }
};


//template <typename Result>
//Result databaseFetch(database_ptr *client, const std::string &table, typename Result::UID objectUid, std::function<Result(statement_ptr&)> callback) {
//    return ::databaseWithoutTransaction<Result>(client, [=](database_ptr &database, int uid)->Result{
//        return ::sqlite3_fetch<Result>(database, uid, table, objectUid, callback);
//    });
//};

//template <typename Result>
//boost::optional<Result> databaseFetchOptional(SQLiteClient *client, const std::string &table, typename Result::UID objectUid, std::function<boost::optional<Result>(statement_ptr&)> callback) {
//    return ::databaseWithoutTransaction<boost::optional<Result>>(client, [=](database_ptr &database, int uid)->boost::optional<Result>{
//        return ::sqlite3_fetch_optional<Result>(database, uid, table, objectUid, callback);
//    });
//};
//
//template <typename Result, typename ...Fields>
//Result databaseFetch(SQLiteClient *client, const std::stringstream &sql, std::function<Result(statement_ptr&)> callback, const Fields&... fields) {
//    return ::databaseWithoutTransaction<Result>(client, [=, &sql](database_ptr &database, int uid)->Result{
//        return ::sqlite3_fetch<Result>(database, uid, sql, callback, fields...);
//    });
//};
//template <typename Result, typename ...Fields>
//boost::optional<Result> databaseFetchOptional(SQLiteClient *client, const std::stringstream &sql, std::function<boost::optional<Result>(statement_ptr&)> callback, const Fields&... fields) {
//    return ::databaseWithoutTransaction<boost::optional<Result>>(client, [=, &sql](database_ptr &database, int uid)->boost::optional<Result>{
//        return ::sqlite3_fetch_optional<Result>(database, uid, sql, callback, fields...);
//    });
//};
//
//template <typename Container, typename ...Fields>
//Container databaseFetch(SQLiteClient *client, const std::stringstream &sql, std::function<typename Container::value_type(statement_ptr&)> callback, const Fields&... fields) {
//    return ::databaseWithoutTransaction<Container>(client, [=, &sql](database_ptr &database, int uid)->Container{
//        return ::sqlite3_fetch<Container>(database, uid, sql, callback, fields...);
//    });
//};
//
//template <typename Container, typename ...Fields>
//Container databaseFetchOptional(SQLiteClient *client, const std::stringstream &sql, std::function<boost::optional<typename Container::value_type>(statement_ptr&)> callback, const Fields&... fields) {
//    return ::databaseWithoutTransaction<Container>(client, [=, &sql](database_ptr &database, int uid)->Container{
//        return ::sqlite3_fetch_optional<Container>(database, uid, sql, callback, fields...);
//    });
//};
