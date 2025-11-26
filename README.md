# Student Management System

A simple, command-line based Student Management System written in C++. It allows for managing student and user records through a text-based interface and persists data in a JSON file.

## Features

- **User Authentication:** Secure login system for application access.
- **Student Management:**
  - Add new student records.
  - Edit existing student information.
  - Remove students from the database.
  - List all students with their details.
- **User Management:**
  - Add new application users.
  - Edit existing user details.
  - Remove users.
  - List all registered users.
- **Data Persistence:** All data is saved to and loaded from a `data.json` file.

## Getting Started

Follow these instructions to get a copy of the project up and running on your local machine.

### Prerequisites

- **CMake:** Version 3.10 or higher.
- **C++ Compiler:** A C++17 compatible compiler (e.g., MinGW-w64, GCC, Clang).

### Building the Application

A batch script is provided for easy compilation on Windows.

```bash
# This will create the build directory and compile the source code.
./build.bat
```

The executable will be located at `build/bin/application.exe`.

### Running the Application

To run the application, use the provided batch script:

```bash
# This script simply executes the compiled application.
./run.bat
```

## Dependencies

The project uses the following third-party library:

- **nlohmann/json:** A header-only library for JSON manipulation in C++. It is included directly in the `include` directory.

## Contributors

- **Morm Leapsovann** mormleapsovann@gmail.com
- **Sok Molika:** sokmolika1111@gmail.com
- **Son Sreybon:** sreybon7753@gmail.com
- **Chorn Dara:** chorndara423@gmail.com
- **Lorm Kimlang:** lormkimlang@gmail.com
