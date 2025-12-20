#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <functional>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

/* =======================
   DATA TYPES
   ======================= */

enum class DType
{
    TEXT,
    CHAR,
    INT,
    FLOAT,
    REAL,
    RELATION
};

struct Value
{
    DType type{};
    json data;

    Value() = default;
    Value(DType t, const json& d) : type(t), data(d) {}
};

struct Attribute
{
    std::string name;
    DType type{};

    // SQL-like qualifiers
    bool isPrimaryKey = false;
    bool isAutoIncrement = false;
    bool isNotNull = false;
    bool hasDefault = false;
    json defaultValue;

    Attribute() = default;
    Attribute(const std::string& n, DType t) : name(n), type(t) {}
};

struct Entity
{
    std::unordered_map<std::string, Value> fields;
};

struct ForeignKey
{
    std::string column;
    std::string refTable;
    std::string refColumn;
};

/* =======================
   TABLE
   ======================= */

struct Table
{
    std::string name;
    std::vector<Attribute> schema;
    std::vector<Entity> rows;
    std::vector<ForeignKey> foreignKeys;

    // For columns declared AUTO_INCREMENT, track next available value
    std::unordered_map<std::string, int64_t> autoIncCounters;

    explicit Table(const std::string& n) : name(n) {}

    bool HasColumn(const std::string& col) const
    {
        return std::any_of(schema.begin(), schema.end(),
            [&](const Attribute& a) { return a.name == col; });
    }
};

/* =======================
   DATABASE
   ======================= */

class Database
{
public:
    explicit Database(const std::string& n) : name(n) {}

    Table& CreateTable(const std::string& tableName)
    {
        auto table = std::make_shared<Table>(tableName);
        tables[tableName] = table;
        return *table;
    }

    Table& GetTable(const std::string& tableName)
    {
        auto it = tables.find(tableName);
        if (it == tables.end())
            throw std::runtime_error("Table not found: " + tableName);
        return *it->second;
    }

    const Table& GetTable(const std::string& tableName) const
    {
        auto it = tables.find(tableName);
        if (it == tables.end())
            throw std::runtime_error("Table not found: " + tableName);
        return *it->second;
    }

    const std::unordered_map<std::string, std::shared_ptr<Table>>&
    GetTables() const
    {
        return tables;
    }

    // Simple authentication helpers (hashing uses std::hash — not cryptographically secure)
    bool HasCredentials() const { return !authUser.empty(); }

    void SetCredentials(const std::string& user, const std::string& pass)
    {
        authUser = user;
        authPassHash = HashPassword(pass);
    }

    // Used by Deserialize to set stored hash directly
    void SetCredentialsHash(const std::string& user, const std::string& hash)
    {
        authUser = user;
        authPassHash = hash;
    }

    bool Authenticate(const std::string& user, const std::string& pass) const
    {
        return (authUser == user && authPassHash == HashPassword(pass));
    }

    const std::string& GetAuthUser() const { return authUser; }
    const std::string& GetAuthHash() const { return authPassHash; }

private:
    // Non-cryptographic helper — sufficient for learning/demo purposes
    static std::string HashPassword(const std::string& pass)
    {
        std::hash<std::string> h;
        auto v = h(pass);
        std::ostringstream oss;
        oss << std::hex << v;
        return oss.str();
    }

    std::string name;
    std::unordered_map<std::string, std::shared_ptr<Table>> tables;

    // authentication state
    std::string authUser;
    std::string authPassHash;
};

/* =======================
   CORE OPERATIONS
   ======================= */

inline void Insert(Table& table, const json& values)
{
    Entity row;

    for (const auto& attr : table.schema)
    {
        // Value provided explicitly
        if (values.contains(attr.name))
        {
            row.fields[attr.name] = Value(attr.type, values.at(attr.name));
            continue;
        }

        // AUTO_INCREMENT: generate value
        if (attr.isAutoIncrement)
        {
            auto& counter = table.autoIncCounters[attr.name];
            if (counter == 0) counter = 1; // start from 1
            row.fields[attr.name] = Value(attr.type, counter);
            counter++;
            continue;
        }

        // DEFAULT provided
        if (attr.hasDefault)
        {
            row.fields[attr.name] = Value(attr.type, attr.defaultValue);
            continue;
        }

        // NOT NULL without default -> error
        if (attr.isNotNull)
            throw std::runtime_error("Missing column: " + attr.name);

        // otherwise insert null
        row.fields[attr.name] = Value(attr.type, json());
    }

    // Enforce primary key uniqueness (simple single-column keys)
    for (const auto& attr : table.schema)
    {
        if (!attr.isPrimaryKey) continue;
        const auto& key = attr.name;
        const auto& val = row.fields.at(key).data;
        for (const auto& existing : table.rows)
        {
            if (existing.fields.at(key).data == val)
                throw std::runtime_error("Duplicate primary key: " + key);
        }
    }

    table.rows.push_back(row);
}

inline std::vector<Entity> Select(
    const Table& table,
    const std::string& column,
    const json& value)
{
    std::vector<Entity> result;

    for (const auto& row : table.rows)
    {
        if (row.fields.at(column).data == value)
            result.push_back(row);
    }

    return result;
}

inline bool ValidateForeignKeys(
    const Table& table,
    const Database& db)
{
    for (const auto& fk : table.foreignKeys)
    {
        const auto& refTable = db.GetTable(fk.refTable);

        for (const auto& row : table.rows)
        {
            const auto& val = row.fields.at(fk.column).data;

            bool found = false;
            for (const auto& refRow : refTable.rows)
            {
                if (refRow.fields.at(fk.refColumn).data == val)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
                return false;
        }
    }
    return true;
}

/* =======================
   QUERY SYSTEM
   ======================= */

struct QueryResult
{
    bool hasResult = false;
    std::vector<Entity> rows;
};

inline std::vector<std::string> Tokenize(const std::string& q)
{
    std::stringstream ss(q);
    std::string tok;
    std::vector<std::string> tokens;

    while (ss >> tok)
        tokens.push_back(tok);

    return tokens;
}

inline QueryResult ExecuteQuery(Database& db, const std::string& query)
{
    auto tokens = Tokenize(query);

    if (tokens.empty())
        throw std::runtime_error("Empty query");

    /* -------- CREATE --------
       CREATE TABLE Name (col TYPE, ...)
    */
    if (tokens[0] == "CREATE")
    {
        if (tokens.size() < 3 || tokens[1] != "TABLE")
            throw std::runtime_error("Invalid CREATE syntax");

        auto parenStart = query.find('(');
        auto parenEnd = query.rfind(')');
        if (parenStart == std::string::npos || parenEnd == std::string::npos || parenEnd < parenStart)
            throw std::runtime_error("CREATE TABLE requires column definitions in parentheses");

        std::string tableName = tokens[2];

        if (db.GetTables().find(tableName) != db.GetTables().end())
            throw std::runtime_error("Table already exists: " + tableName);

        auto& table = db.CreateTable(tableName);

        auto colsText = query.substr(parenStart + 1, parenEnd - parenStart - 1);
        std::stringstream ss(colsText);
        std::string colDef;
        while (std::getline(ss, colDef, ','))
        {
            auto l = colDef.find_first_not_of(" \t\n\r");
            auto r = colDef.find_last_not_of(" \t\n\r");
            if (l == std::string::npos) continue;
            std::string def = colDef.substr(l, r - l + 1);

            std::stringstream ds(def);
            std::string colName, typeStr;
            ds >> colName >> typeStr;
            if (colName.empty() || typeStr.empty())
                throw std::runtime_error("Invalid column definition: " + def);

            // remainder contains modifiers like PRIMARY KEY, AUTO_INCREMENT, NOT NULL, DEFAULT ...
            auto posAfterType = def.find(typeStr);
            std::string modifiers = "";
            if (posAfterType != std::string::npos)
                modifiers = def.substr(posAfterType + typeStr.size());

            std::transform(typeStr.begin(), typeStr.end(), typeStr.begin(), ::toupper);

            DType dtype;
            if (typeStr == "TEXT") dtype = DType::TEXT;
            else if (typeStr == "CHAR") dtype = DType::CHAR;
            else if (typeStr == "INT") dtype = DType::INT;
            else if (typeStr == "FLOAT") dtype = DType::FLOAT;
            else if (typeStr == "REAL") dtype = DType::REAL;
            else if (typeStr == "RELATION") dtype = DType::RELATION;
            else throw std::runtime_error("Unknown type: " + typeStr);

            Attribute attr(colName, dtype);

            // parse modifiers (case-insensitive)
            std::string up = modifiers;
            std::transform(up.begin(), up.end(), up.begin(), ::toupper);

            if (up.find("AUTO") != std::string::npos)
                attr.isAutoIncrement = true;
            if (up.find("PRIMARY") != std::string::npos && up.find("KEY") != std::string::npos)
                attr.isPrimaryKey = true;
            if (up.find("NOT") != std::string::npos && up.find("NULL") != std::string::npos)
                attr.isNotNull = true;

            // DEFAULT parsing: find 'DEFAULT' and extract the following token (allow quoted strings)
            auto defPos = up.find("DEFAULT");
            if (defPos != std::string::npos)
            {
                // find original 'DEFAULT' position in modifiers to get original-case token
                auto origDefPos = modifiers.find_first_of("DEFAULT");
                if (origDefPos == std::string::npos) origDefPos = defPos; // fallback
                size_t vpos = origDefPos + 7; // length of DEFAULT
                // skip whitespace
                while (vpos < modifiers.size() && isspace((unsigned char)modifiers[vpos])) ++vpos;
                if (vpos < modifiers.size())
                {
                    if (modifiers[vpos] == '"')
                    {
                        size_t endq = modifiers.find('"', vpos + 1);
                        if (endq == std::string::npos) throw std::runtime_error("Unterminated DEFAULT string in: " + def);
                        std::string dv = modifiers.substr(vpos + 1, endq - vpos - 1);
                        attr.hasDefault = true;
                        attr.defaultValue = dv;
                    }
                    else
                    {
                        // read until space or end
                        size_t endv = vpos;
                        while (endv < modifiers.size() && !isspace((unsigned char)modifiers[endv])) ++endv;
                        std::string dv = modifiers.substr(vpos, endv - vpos);
                        // try to parse as json (numbers, booleans), fall back to string
                        try { attr.defaultValue = json::parse(dv); }
                        catch (...) { attr.defaultValue = dv; }
                        attr.hasDefault = true;
                    }
                }
            }

            table.schema.push_back(attr);

            if (attr.isAutoIncrement)
                table.autoIncCounters[attr.name] = 1; // initialize counter
        }

        return {};
    }

    /* -------- INSERT --------
       INSERT TableName {json}
    */
    if (tokens[0] == "INSERT")
    {
        if (tokens.size() < 2)
            throw std::runtime_error("Invalid INSERT syntax");

        auto jsonStart = query.find('{');
        auto jsonEnd   = query.rfind('}');

        if (jsonStart == std::string::npos || jsonEnd == std::string::npos)
            throw std::runtime_error("INSERT requires JSON object");

        auto jsonText = query.substr(jsonStart, jsonEnd - jsonStart + 1);
        json values = json::parse(jsonText);

        Insert(db.GetTable(tokens[1]), values);
        return {};
    }

    /* -------- SELECT --------
       SELECT Table
       SELECT Table WHERE col = value
    */
    if (tokens[0] == "SELECT")
    {
        if (tokens.size() < 2)
            throw std::runtime_error("Invalid SELECT syntax");

        QueryResult result;
        result.hasResult = true;

        const auto& table = db.GetTable(tokens[1]);

        if (tokens.size() == 2)
        {
            result.rows = table.rows;
            return result;
        }

        if (tokens.size() >= 6 && tokens[2] == "WHERE" && tokens[4] == "=")
        {
            json value;
            if (tokens[5].front() == '"')
                value = tokens[5].substr(1, tokens[5].size() - 2);
            else
                value = json::parse(tokens[5]);

            result.rows = Select(table, tokens[3], value);
            return result;
        }

        throw std::runtime_error("Invalid SELECT syntax");
    }

    throw std::runtime_error("Unknown command: " + tokens[0]);
}

/* =======================
   SERIALIZATION
   ======================= */

inline json Serialize(const Database& db)
{
    json j;

    for (const auto& [name, table] : db.GetTables())
    {
        json jt;

        for (const auto& attr : table->schema)
        {
            json aj = {
                {"name", attr.name},
                {"type", static_cast<int>(attr.type)}
            };
            if (attr.isPrimaryKey) aj["primary"] = true;
            if (attr.isAutoIncrement) aj["auto"] = true;
            if (attr.isNotNull) aj["not_null"] = true;
            if (attr.hasDefault) aj["default"] = attr.defaultValue;
            jt["schema"].push_back(aj);
        }

        for (const auto& row : table->rows)
        {
            json jr;
            for (const auto& [k, v] : row.fields)
                jr[k] = v.data;
            jt["rows"].push_back(jr);
        }

        j[name] = jt;
    }

    // Store optional auth metadata under a reserved key
    if (db.HasCredentials())
    {
        j["__meta"]["auth"] = {
            {"user", db.GetAuthUser()},
            {"pass", db.GetAuthHash()}
        };
    }

    return j;
}

inline void Deserialize(Database& db, const json& j)
{
    // Handle optional metadata
    if (j.contains("__meta") && j["__meta"].contains("auth"))
    {
        const auto& auth = j["__meta"]["auth"];
        if (auth.contains("user") && auth.contains("pass"))
        {
            db.SetCredentialsHash(auth["user"].get<std::string>(), auth["pass"].get<std::string>());
        }
    }

    for (const auto& [tableName, tableData] : j.items())
    {
        if (tableName == "__meta") continue; // skip metadata

        auto& table = db.CreateTable(tableName);

        if (tableData.contains("schema"))
        {
            for (const auto& attr : tableData["schema"])
            {
                Attribute a(
                    attr["name"].get<std::string>(),
                    static_cast<DType>(attr["type"].get<int>())
                );
                if (attr.contains("primary")) a.isPrimaryKey = attr["primary"].get<bool>();
                if (attr.contains("auto")) a.isAutoIncrement = attr["auto"].get<bool>();
                if (attr.contains("not_null")) a.isNotNull = attr["not_null"].get<bool>();
                if (attr.contains("default")) { a.hasDefault = true; a.defaultValue = attr["default"]; }
                table.schema.push_back(a);
                if (a.isAutoIncrement) table.autoIncCounters[a.name] = 1;
            }
        }

        if (tableData.contains("rows"))
        {
            for (const auto& row : tableData["rows"])
            {
                Insert(table, row);
            }

            // adjust auto-increment counters based on max existing values
            for (const auto& a : table.schema)
            {
                if (!a.isAutoIncrement) continue;
                int64_t maxv = 0;
                for (const auto& r : table.rows)
                {
                    auto it = r.fields.find(a.name);
                    if (it == r.fields.end()) continue;
                    if (it->second.data.is_number())
                    {
                        int64_t v = it->second.data.get<int64_t>();
                        if (v > maxv) maxv = v;
                    }
                }
                table.autoIncCounters[a.name] = maxv + 1;
            }
        }
    }
}

inline void SaveToFile(const Database& db, const std::string& path)
{
    std::ofstream file(path);
    file << Serialize(db).dump(4);
}

inline void LoadFromFile(Database& db, const std::string& path)
{
    std::ifstream file(path);
    json j;
    file >> j;
    Deserialize(db, j);
}
