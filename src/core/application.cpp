#include "application.h"

Application::Application(const std::string &title)
{
    this->title = title;
}

void Application::Initialize()
{
    std::cout << "Welcome! Student Management.\n";
    db = new Database("database/data.json");

    // LOGIN LOOP
    while (true)
    {
        std::string username = Input("Username: ");
        if (username == "exit") { isRunning = false; return; }

        std::string password = Input("Password: ");
        if (password == "exit") { isRunning = false; return; }

        if (db->authenticateUser(username, password))
        {
            std::cout << "Login successful! Welcome " << username << "\n";
            loggedInUser = username; // optional: store who logged in
            break;
        }

        std::cout << "Invalid username or password. Try again.\n";
    }

    std::cout << "Type ? or help for help.\n";
    isRunning = true;
}


bool Application::ShouldClose()
{
    return !isRunning;
}

void Application::Update()
{
    std::string user = Input(">> ");
    ProcessCommand(user.c_str());
}

void Application::ProcessCommand(const char *command)
{
    std::vector<std::string> cmd = splitCommand(command);
    if (cmd.empty())
        return;

    auto showHelp = []()
    {
        std::cout << "Available commands:\n";
        std::cout << "  add student <first_name> <middle_name> <last_name> <dob> <gender> <address> <phone> <major> <year>\n";
        std::cout << "  add user <username> <password> <role>\n";
        std::cout << "  edit student <id> <field>=<value> [...]\n";
        std::cout << "  edit user <username> <field>=<value> [...]\n";
        std::cout << "  remove student <id>\n";
        std::cout << "  remove user <username>\n";
        std::cout << "  list students\n";
        std::cout << "  list users\n";
        std::cout << "  help | ?\n";
        std::cout << "  exit\n";
    };

    if (cmd[0] == "help" || cmd[0] == "?")
    {
        showHelp();
        return;
    }

    if (cmd[0] == "add")
    {
        if (cmd.size() < 2)
        {
            std::cerr << "Specify type: user/student\n";
            return;
        }

        if (cmd[1] == "student")
        {
            if (cmd.size() < 11)
            {
                std::cerr << "Insufficient arguments for add student\n";
                return;
            }

            // Build personal JSON
            json personal = {
                {"first_name", cmd[2]},
                {"middle_name", cmd[3]},
                {"last_name", cmd[4]},
                {"date_of_birth", cmd[5]},
                {"gender", cmd[6]},
                {"address", cmd[7]},
                {"phone_number", cmd[8]}};

            // Build education JSON
            json education = {
                {"major", cmd[9]},
                {"year", cmd[10]}};

            if (db->addStudent(personal, education))
                std::cout << "Student added successfully.\n";
            else
                std::cerr << "Failed to add student.\n";
        }
        else if (cmd[1] == "user")
        {
            if (cmd.size() < 5)
            {
                std::cerr << "Insufficient arguments for add user\n";
                return;
            }

            json user = {
                {"username", cmd[2]},
                {"password", cmd[3]},
                {"role", cmd[4]}};

            if (db->addUser(user))
                std::cout << "User added successfully.\n";
            else
                std::cerr << "Failed to add user.\n";
        }
        else
        {
            std::cerr << "Unknown add type: " << cmd[1] << std::endl;
        }
    }
    else if (cmd[0] == "edit")
    {
        if (cmd.size() < 3)
        {
            std::cerr << "Specify type and identifier\n";
            return;
        }

        if (cmd[1] == "student")
        {
            int id = std::stoi(cmd[2]);
            json newPersonal, newEducation;

            for (size_t i = 3; i < cmd.size(); i++)
            {
                auto pos = cmd[i].find('=');
                if (pos == std::string::npos)
                    continue;
                std::string key = cmd[i].substr(0, pos);
                std::string val = cmd[i].substr(pos + 1);

                if (key == "major" || key == "year")
                    newEducation[key] = val;
                else
                    newPersonal[key] = val;
            }

            if (db->editStudent(id, newPersonal, newEducation))
                std::cout << "Student updated successfully.\n";
            else
                std::cerr << "Failed to update student.\n";
        }
        else if (cmd[1] == "user")
        {
            std::string username = cmd[2];
            json newData;

            for (size_t i = 3; i < cmd.size(); i++)
            {
                auto pos = cmd[i].find('=');
                if (pos == std::string::npos)
                    continue;
                std::string key = cmd[i].substr(0, pos);
                std::string val = cmd[i].substr(pos + 1);
                newData[key] = val;
            }

            if (db->editUser(username, newData))
                std::cout << "User updated successfully.\n";
            else
                std::cerr << "Failed to update user.\n";
        }
        else
        {
            std::cerr << "Unknown edit type: " << cmd[1] << std::endl;
        }
    }
    else if (cmd[0] == "remove")
    {
        if (cmd.size() < 3)
        {
            std::cerr << "Specify type and identifier\n";
            return;
        }

        if (cmd[1] == "student")
        {
            int id = std::stoi(cmd[2]);
            if (db->removeStudent(id))
                std::cout << "Student removed.\n";
            else
                std::cerr << "Failed to remove student.\n";
        }
        else if (cmd[1] == "user")
        {
            std::string username = cmd[2];
            if (db->removeUser(username))
                std::cout << "User removed.\n";
            else
                std::cerr << "Failed to remove user.\n";
        }
        else
        {
            std::cerr << "Unknown remove type: " << cmd[1] << std::endl;
        }
    }
    else if (cmd[0] == "list")
    {
        if (cmd.size() < 2)
        {
            std::cerr << "Specify type to list: users/students\n";
            return;
        }

        if (cmd[1] == "students")
        {
            json personal = db->findMany("students_personal");
            json edu = db->findMany("students_education");
            std::cout << "Students Personal:\n"
                      << personal.dump(4) << "\n";
            std::cout << "Students Education:\n"
                      << edu.dump(4) << "\n";
        }
        else if (cmd[1] == "users")
        {
            json users = db->findMany("users");
            std::cout << "Users:\n"
                      << users.dump(4) << "\n";
        }
        else
        {
            std::cerr << "Unknown list type: " << cmd[1] << std::endl;
        }
    }
    else if (cmd[0] == "exit")
    {
        isRunning = false;
        std::cout << "[*] Application has been ShutDown!\n";
    }
    else
    {
        std::cerr << "COMMAND NOT FOUND: " << command << std::endl;
    }
}