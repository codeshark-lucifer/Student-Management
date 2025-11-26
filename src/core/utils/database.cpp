#include "database.h"

// -------------------------------
// Constructor (auto-create folder/file)
// -------------------------------
Database::Database(const std::string &filename) {
    this->filename = filename;

    std::filesystem::path path(filename);
    std::filesystem::path folder = path.parent_path();

    if (!folder.empty() && !std::filesystem::exists(folder))
        std::filesystem::create_directories(folder);

    if (!std::filesystem::exists(filename)) {
        std::ofstream file(filename);
        if (file.is_open()) file << "{}";
    }

    database = fetch();
    initializeIfEmpty();
}

// -------------------------------
// Read JSON from file
// -------------------------------
json Database::fetch() {
    std::ifstream file(filename);
    json db;
    try { file >> db; }
    catch (...) { db = json::object(); }
    file.close();
    return db;
}

// -------------------------------
// Save JSON to file
// -------------------------------
void Database::save() {
    std::ofstream file(filename);
    file << database.dump(4);
    file.close();
}

// -------------------------------
// Split "key=value"
// -------------------------------
std::pair<std::string, std::string> Database::splitCondition(const std::string &cond) {
    size_t pos = cond.find('=');
    if (pos == std::string::npos) return {"", ""};
    return { cond.substr(0, pos), cond.substr(pos + 1) };
}

// -------------------------------
// Find table by name
// -------------------------------
json* Database::findTable(const std::string &tableName) {
    if (!database.contains("tables")) return nullptr;
    for (auto &tbl : database["tables"])
        if (tbl["name"] == tableName) return &tbl;
    return nullptr;
}

// -------------------------------
// Find one row in table by key=value
// -------------------------------
json Database::findOne(const std::string &tableName, const std::string &condition) {
    json *table = findTable(tableName);
    if (!table) return json();

    auto [key, value] = splitCondition(condition);
    if (key.empty()) return json();

    for (auto &row : (*table)["columns"]) {
        if (row.contains(key) && ((row[key].is_string() && row[key] == value) ||
                                  (row[key].is_number() && std::to_string((int)row[key]) == value)))
            return row;
    }
    return json();
}

// -------------------------------
// Find many rows (optional condition)
// -------------------------------
json Database::findMany(const std::string &tableName, const std::string &condition) {
    json result = json::array();
    json *table = findTable(tableName);
    if (!table) return result;

    if (condition.empty()) return (*table)["columns"];

    auto [key, value] = splitCondition(condition);
    if (key.empty()) return result;

    for (auto &row : (*table)["columns"]) {
        if (row.contains(key) && ((row[key].is_string() && row[key] == value) ||
                                  (row[key].is_number() && std::to_string((int)row[key]) == value)))
            result.push_back(row);
    }
    return result;
}

// -------------------------------
// Users operations
// -------------------------------
bool Database::addUser(const json &user) {
    json *table = findTable("users");
    if (!table) return false;
    (*table)["columns"].push_back(user);
    save();
    return true;
}

bool Database::editUser(const std::string &username, const json &newData) {
    json *table = findTable("users");
    if (!table) return false;

    for (auto &row : (*table)["columns"]) {
        if (row["username"] == username) {
            for (auto it = newData.begin(); it != newData.end(); ++it)
                row[it.key()] = it.value();
            save();
            return true;
        }
    }
    return false;
}

bool Database::removeUser(const std::string &username) {
    json *table = findTable("users");
    if (!table) return false;

    auto &cols = (*table)["columns"];
    for (auto it = cols.begin(); it != cols.end(); ++it) {
        if ((*it)["username"] == username) {
            cols.erase(it);
            save();
            return true;
        }
    }
    return false;
}

// -------------------------------
// Students operations
// -------------------------------
bool Database::addStudent(const json &personal, const json &education) {
    json *personalTable = findTable("students_personal");
    json *eduTable = findTable("students_education");
    if (!personalTable || !eduTable) return false;

    // Auto-generate id
    int newId = 0;
    for (auto &row : (*personalTable)["columns"]) {
        if (row.contains("id") && row["id"].is_number())
            newId = std::max(newId, (int)row["id"]);
    }
    newId++;

    json newPersonal = personal;
    newPersonal["id"] = newId;
    json newEdu = education;
    newEdu["student_id"] = newId;

    (*personalTable)["columns"].push_back(newPersonal);
    (*eduTable)["columns"].push_back(newEdu);
    save();
    return true;
}

bool Database::editStudent(int student_id, const json &newPersonal, const json &newEducation) {
    json *personalTable = findTable("students_personal");
    json *eduTable = findTable("students_education");
    if (!personalTable || !eduTable) return false;

    bool edited = false;

    for (auto &row : (*personalTable)["columns"]) {
        if (row["id"] == student_id) {
            for (auto it = newPersonal.begin(); it != newPersonal.end(); ++it)
                row[it.key()] = it.value();
            edited = true;
            break;
        }
    }

    for (auto &row : (*eduTable)["columns"]) {
        if (row["student_id"] == student_id) {
            for (auto it = newEducation.begin(); it != newEducation.end(); ++it)
                row[it.key()] = it.value();
            edited = true;
            break;
        }
    }

    if (edited) save();
    return edited;
}

bool Database::removeStudent(int student_id) {
    json *personalTable = findTable("students_personal");
    json *eduTable = findTable("students_education");
    if (!personalTable || !eduTable) return false;

    bool removed = false;

    auto &pCols = (*personalTable)["columns"];
    for (auto it = pCols.begin(); it != pCols.end(); ++it) {
        if ((*it)["id"] == student_id) {
            pCols.erase(it);
            removed = true;
            break;
        }
    }

    auto &eCols = (*eduTable)["columns"];
    for (auto it = eCols.begin(); it != eCols.end(); ++it) {
        if ((*it)["student_id"] == student_id) {
            eCols.erase(it);
            removed = true;
            break;
        }
    }

    if (removed) save();
    return removed;
}

// -------------------------------
// Initialize demo data if empty
// -------------------------------
void Database::initializeIfEmpty() {
    if (!database.contains("tables") || database["tables"].empty()) {
        database = {
            { "tables", json::array({
                {
                    {"name", "students_personal"},
                    {"columns", json::array({
                        { {"id", 0}, {"first_name","morm"}, {"middle_name","leap"}, {"last_name","sovann"},
                          {"date_of_birth","2007-01-17"}, {"gender","male"}, {"address","battambang"}, {"phone_number","+85516677462"} }
                    })}
                },
                {
                    {"name", "students_education"},
                    {"columns", json::array({
                        { {"student_id",0}, {"major","Information Technology"}, {"year","2"} }
                    })}
                },
                {
                    {"name", "users"},
                    {"columns", json::array({
                        { {"username","admin"}, {"password","admin123"}, {"role","administrator"} },
                        { {"username","teacher1"}, {"password","teachpass"}, {"role","instructor"} }
                    })}
                }
            })}
        };
        save();
        std::cout << "Database initialized with demo data.\n";
    }
}
bool Database::authenticateUser(const std::string &username, const std::string &password)
{
    json user = findOne("users", "username=" + username);
    if (user.is_null())
        return false;

    if (!user.contains("password"))
        return false;

    return user["password"] == password;
}
