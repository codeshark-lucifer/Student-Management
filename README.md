# Student-Management (mini in-memory DB)

A simple educational in-memory database with a tiny SQL-like query language, persistent JSON storage, and optional authentication.

## Quick start
- Build and run the `application` executable.
- On first run you'll be prompted to create credentials (optional). If credentials exist, you'll be asked to login.

## Commands
- CREATE TABLE
  - Syntax: `CREATE TABLE <name> (col TYPE [AUTO_INCREMENT] [PRIMARY KEY] [NOT NULL] [DEFAULT <value>], ...)`
  - Example: `CREATE TABLE users (id INT AUTO_INCREMENT PRIMARY KEY, name TEXT NOT NULL DEFAULT "anon")`

- INSERT
  - Syntax: `INSERT <TableName> {json}`
  - Example: `INSERT users {"name":"Alice"}` (auto-assigned id when `AUTO_INCREMENT`)

- SELECT
  - Syntax: `SELECT <TableName>` or `SELECT <TableName> WHERE <col> = <value>`
  - Example: `SELECT users` or `SELECT users WHERE name = "Alice"`

- help
  - Shows available commands (only available after login if authentication is enabled)

- exit
  - Quit the application and persist the database

## Authentication
- Credentials are stored in `database.json` under `__meta.auth` (username and a non-cryptographic hash).
- On startup, if credentials exist you'll be prompted to login (3 attempts). If not, you can create credentials.

## Notes & limitations
- PRIMARY KEY enforcement currently supports single-column primary keys only.
- Password hashing uses `std::hash` (not secure for production) — replace with a proper hash (bcrypt/argon2) for real use.
- AUTO_INCREMENT counters persist via scanning existing rows and start at `max(existing)+1`.

## Example session
```
CREATE TABLE users (id INT AUTO_INCREMENT PRIMARY KEY, name TEXT NOT NULL DEFAULT "anon")
INSERT users {"name":"Bob"}
INSERT users {"name":"Carol"}
SELECT users
```
