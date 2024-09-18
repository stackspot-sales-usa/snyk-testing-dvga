#include <iostream>
#include <string>
#include <sqlite3.h>

using namespace std;

class Database {
public:
    Database(const string& dbName) {
        if (sqlite3_open(dbName.c_str(), &db)) {
            cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
            exit(1);
        }
    }

    ~Database() {
        sqlite3_close(db);
    }

    bool executeQuery(const string& query) {
        char* errMsg = 0;
        int rc = sqlite3_exec(db, query.c_str(), callback, 0, &errMsg);
        if (rc != SQLITE_OK) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }

    static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
        for (int i = 0; i < argc; i++) {
            cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << endl;
        }
        return 0;
    }

private:
    sqlite3* db;
};

class User {
public:
    User(const string& username, const string& password)
        : username(username), password(password) {}

    string getUsername() const { return username; }
    string getPassword() const { return password; }

private:
    string username;
    string password;
};

class LoginSystem {
public:
    LoginSystem(Database& db) : db(db) {}

    void registerUser(const User& user) {
        string query = "INSERT INTO users (username, password) VALUES ('" + user.getUsername() + "', '" + user.getPassword() + "');";
        db.executeQuery(query);
    }

    bool loginUser(const string& username, const string& password) {
        string query = "SELECT * FROM users WHERE username = '" + username + "' AND password = '" + password + "';";
        return db.executeQuery(query);
    }

private:
    Database& db;
};

void setupDatabase(Database& db) {
    string createTableQuery = "CREATE TABLE IF NOT EXISTS users ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                              "username TEXT NOT NULL,"
                              "password TEXT NOT NULL);";
    db.executeQuery(createTableQuery);
}

int main() {
    Database db("test.db");
    setupDatabase(db);

    LoginSystem loginSystem(db);

    // Registering users
    User user1("admin", "admin123");
    User user2("user", "user123");

    loginSystem.registerUser(user1);
    loginSystem.registerUser(user2);

    // Simulating user input
    string inputUsername;
    string inputPassword;

    cout << "Enter username: ";
    getline(cin, inputUsername);
    cout << "Enter password: ";
    getline(cin, inputPassword);

    // Vulnerable login attempt
    if (loginSystem.loginUser(inputUsername, inputPassword)) {
        cout << "Login successful!" << endl;
    } else {
        cout << "Login failed!" << endl;
    }

    return 0;
}