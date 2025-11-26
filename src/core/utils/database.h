#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Database
{
public:
    Database() = default;
    Database(const std::string &filename);

    // Read/write database
    json fetch();
    void save();

    // -------------------------------
    // Generic table operations
    // -------------------------------
    json *findTable(const std::string &tableName);
    json findOne(const std::string &tableName, const std::string &condition);
    json findMany(const std::string &tableName, const std::string &condition = "");

    // -------------------------------
    // Users operations
    // -------------------------------
    bool addUser(const json &user);
    bool editUser(const std::string &username, const json &newData);
    bool removeUser(const std::string &username);

    // -------------------------------
    // Students operations
    // -------------------------------
    bool addStudent(const json &personal, const json &education);
    bool editStudent(int student_id, const json &newPersonal, const json &newEducation);
    bool removeStudent(int student_id);
    bool authenticateUser(const std::string &username, const std::string &password);

private:
    std::string filename;
    json database;

    std::pair<std::string, std::string> splitCondition(const std::string &cond);
    void initializeIfEmpty();
};

#endif // DATABASE_H
