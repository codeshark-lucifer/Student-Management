#include <application.hpp>
#include <iostream>
#include <filesystem>

Application::Application()
{
    db = std::make_shared<Database>("codeshark");

    // Load database only if file exists
    if (std::filesystem::exists("database.json"))
    {
        LoadFromFile(*db, "database.json");
        std::cout << "[DB] Loaded database.json\n";
    }
    else
    {
        std::cout << "[DB] No database.json found, starting fresh (no default tables)\n";
        std::cout << "[Tip] Use: CREATE TABLE <name> (col TYPE, ...) to create tables\n";
    }

    // Authentication flow (optional)
    if (db->HasCredentials())
    {
        std::cout << "[Auth] Please login\n";
        bool ok = false;
        for (int attempt = 0; attempt < 3; ++attempt)
        {
            std::string user, pass;
            std::cout << "Username: ";
            std::getline(std::cin, user);
            std::cout << "Password: ";
            std::getline(std::cin, pass);
            // prevent empty input
            if(user.empty() || pass.empty()) {
                running = false;
                break;
            }

            if (db->Authenticate(user, pass))
            {
                std::cout << "[Auth] Authentication successful\n";
                ok = true;
                authenticated = true;
                break;
            }
            std::cout << "[Auth] Invalid credentials (" << (2 - attempt) << " attempts left)\n";
        }

        if (!ok)
        {
            std::cout << "[Auth] Failed to authenticate. Application will not accept commands.\n";
            running = false;
            return;
        }
    }
    else
    {
        std::cout << "No credentials set. Create credentials now? (y/n): ";
        std::string ans;
        std::getline(std::cin, ans);
        if (!ans.empty() && (ans[0] == 'y' || ans[0] == 'Y'))
        {
            std::string user, pass, pass2;
            std::cout << "New username: ";
            std::getline(std::cin, user);
            while (true)
            {
                std::cout << "New password: ";
                std::getline(std::cin, pass);
                std::cout << "Confirm password: ";
                std::getline(std::cin, pass2);
                if (pass == pass2)
                    break;
                std::cout << "Passwords do not match, try again.\n";
            }
            db->SetCredentials(user, pass);
            SaveToFile(*db, "database.json");
            std::cout << "[Auth] Credentials created and saved\n";
            authenticated = true; // newly created credentials -> mark as logged in
        }
        else
        {
            std::cout << "[Auth] Running without database credentials\n";
        }
    }

    running = true;
}

void Application::Run()
{
    while (running)
    {
        try
        {
            std::cout << ">> ";
            std::string input;
            std::getline(std::cin, input);

            if (input == "exit")
            {
                running = false;
                continue;
            }

            if (input.empty())
                continue;

            // help command is available only when authenticated
            if (input == "help" || input == "HELP")
            {
                if (authenticated)
                {
                    std::cout << "  CREATE TABLE <name> (...)\n";
                    std::cout << "  INSERT <TableName> {json}\n";
                    std::cout << "  SELECT <TableName> [WHERE col = value]\n";
                    std::cout << "  REMOVE <TableName> [WHERE col = value]\n";
                    std::cout << "  REMOVE TABLE <TableName>\n";
                    std::cout << "  REMOVE ENTITY <TableName> WHERE col = value\n";
                    std::cout << "  clear | cls | clean\n";
                    std::cout << "  exit\n";
                }
                else
                {
                    std::cout << "[Auth] 'help' is available only after login.\n";
                }
                continue;
            }
            // clear / clean terminal command
            if (input == "clear" || input == "cls" || input == "clean")
            {
                ClearConsole();
                continue;
            }

            if (input == "example")
            {
                std::string msg = R"([EXAMPLES]:
** Create users table
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name TEXT NOT NULL,
    age INT DEFAULT 18
)
    
** Create students table
CREATE TABLE students (
    id INT AUTO_INCREMENT PRIMARY KEY,
    fullname TEXT NOT NULL,
    grade INT,
    email TEXT
)

** Insert with auto ID
INSERT users {"name":"Alice","age":21}
INSERT users {"name":"Bob"}
INSERT users {"name":"Carol","age":30}

** Insert students
INSERT students {"fullname":"John Doe","grade":12,"email":"john@gmail.com"}
INSERT students {"fullname":"Jane Smith","grade":11}

** Select all rows
SELECT users
SELECT students

** Select with condition
SELECT users WHERE name = "Alice"
SELECT students WHERE grade = 12

** Remove all rows from a table
REMOVE users

** Remove specific rows
REMOVE users WHERE name = "Bob"
REMOVE students WHERE grade = 11

** Alias version (same behavior)
REMOVE ENTITY users WHERE id = 1

** Delete entire table
REMOVE TABLE users
REMOVE TABLE students
                )";
            }

            QueryResult result = ExecuteQuery(*db, input);

            if (result.hasResult)
            {
                PrintResult(result);
                SaveToFile(*db, "database.json");
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "[Error] " << e.what() << "\n";
        }
    }
}

Application::~Application()
{
    SaveToFile(*db, "database.json");
    std::cout << "[DB] Saved database.json\n";
}

void Application::PrintResult(const QueryResult &result)
{
    if (result.rows.empty())
    {
        std::cout << "(no rows)\n";
        return;
    }

    for (const auto &row : result.rows)
    {
        std::cout << "{ ";
        for (const auto &[key, value] : row.fields)
        {
            std::cout << key << ": " << value.data << " ";
        }
        std::cout << "}\n";
    }
}
