#include <iostream>
#include <string>
#include <unordered_map>
#include <ctime>
#include <cstdlib>

class User {
public:
    std::string username;
    std::string password;
    std::string sessionToken;
    time_t sessionExpiry;

    User(std::string uname, std::string pwd) : username(uname), password(pwd), sessionToken(""), sessionExpiry(0) {}
};

class AuthSystem {
private:
    std::unordered_map<std::string, User> users;
    std::unordered_map<std::string, std::string> activeSessions;

    std::string generateSessionToken() {
        std::string token = "";
        for (int i = 0; i < 16; ++i) {
            token += 'A' + rand() % 26;
        }
        return token;
    }

public:
    AuthSystem() {
        srand(time(0));
    }

    void registerUser(std::string username, std::string password) {
        if (password.length() < 6) {
            std::cout << "Password too weak. Must be at least 6 characters long.\n";
            return;
        }
        users[username] = User(username, password);
        std::cout << "User registered successfully.\n";
    }

    bool loginUser(std::string username, std::string password) {
        if (users.find(username) == users.end()) {
            std::cout << "User not found.\n";
            return false;
        }
        if (users[username].password != password) {
            std::cout << "Incorrect password.\n";
            return false;
        }

        std::string token = generateSessionToken();
        users[username].sessionToken = token;
        users[username].sessionExpiry = time(0) + 3600; // Session valid for 1 hour
        activeSessions[token] = username;

        std::cout << "Login successful. Session token: " << token << "\n";
        return true;
    }

    void logoutUser(std::string token) {
        if (activeSessions.find(token) == activeSessions.end()) {
            std::cout << "Invalid session token.\n";
            return;
        }

        std::string username = activeSessions[token];
        users[username].sessionToken = "";
        users[username].sessionExpiry = 0;
        activeSessions.erase(token);

        std::cout << "Logout successful.\n";
    }

    bool isAuthenticated(std::string token) {
        if (activeSessions.find(token) == activeSessions.end()) {
            return false;
        }

        std::string username = activeSessions[token];
        if (users[username].sessionExpiry < time(0)) {
            activeSessions.erase(token);
            users[username].sessionToken = "";
            users[username].sessionExpiry = 0;
            return false;
        }

        return true;
    }

    void simulate() {
        std::string username, password, token;
        int choice;

        while (true) {
            std::cout << "1. Register\n2. Login\n3. Logout\n4. Check Authentication\n5. Exit\n";
            std::cin >> choice;

            switch (choice) {
                case 1:
                    std::cout << "Enter username: ";
                    std::cin >> username;
                    std::cout << "Enter password: ";
                    std::cin >> password;
                    registerUser(username, password);
                    break;
                case 2:
                    std::cout << "Enter username: ";
                    std::cin >> username;
                    std::cout << "Enter password: ";
                    std::cin >> password;
                    loginUser(username, password);
                    break;
                case 3:
                    std::cout << "Enter session token: ";
                    std::cin >> token;
                    logoutUser(token);
                    break;
                case 4:
                    std::cout << "Enter session token: ";
                    std::cin >> token;
                    if (isAuthenticated(token)) {
                        std::cout << "User is authenticated.\n";
                    } else {
                        std::cout << "User is not authenticated.\n";
                    }
                    break;
                case 5:
                    return;
                default:
                    std::cout << "Invalid choice.\n";
            }
        }
    }
};

int main() {
    AuthSystem authSystem;
    authSystem.simulate();
    return 0;
}