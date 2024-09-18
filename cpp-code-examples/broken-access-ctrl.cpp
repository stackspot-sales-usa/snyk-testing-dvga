#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class User {
public:
    std::string username;
    std::string role;

    User(const std::string& uname, const std::string& r) : username(uname), role(r) {}
};

class WebApplication {
private:
    std::unordered_map<std::string, User> users;
    std::vector<std::string> restrictedAreas;

public:
    WebApplication() {
        // Initialize some users
        users["admin"] = User("admin", "admin");
        users["user1"] = User("user1", "user");
        users["user2"] = User("user2", "user");

        // Initialize restricted areas
        restrictedAreas.push_back("/admin");
        restrictedAreas.push_back("/settings");
    }

    void login(const std::string& username) {
        if (users.find(username) != users.end()) {
            std::cout << "User " << username << " logged in successfully.\n";
        } else {
            std::cout << "Login failed for user " << username << ".\n";
        }
    }

    void accessArea(const std::string& username, const std::string& area) {
        if (users.find(username) != users.end()) {
            std::cout << "User " << username << " is accessing " << area << ".\n";
            // Broken access control: No proper authorization check
            if (std::find(restrictedAreas.begin(), restrictedAreas.end(), area) != restrictedAreas.end()) {
                std::cout << "Warning: User " << username << " accessed a restricted area: " << area << ".\n";
            }
        } else {
            std::cout << "Access denied for user " << username << ".\n";
        }
    }

    void changeSettings(const std::string& username) {
        if (users.find(username) != users.end()) {
            std::cout << "User " << username << " is changing settings.\n";
            // Broken access control: No proper authorization check
            std::cout << "Warning: User " << username << " changed settings without proper authorization.\n";
        } else {
            std::cout << "Access denied for user " << username << ".\n";
        }
    }

    void viewAdminPage(const std::string& username) {
        if (users.find(username) != users.end()) {
            std::cout << "User " << username << " is viewing the admin page.\n";
            // Broken access control: No proper authorization check
            std::cout << "Warning: User " << username << " viewed the admin page without proper authorization.\n";
        } else {
            std::cout << "Access denied for user " << username << ".\n";
        }
    }
};

int main() {
    WebApplication app;

    // Simulate user actions
    app.login("admin");
    app.accessArea("admin", "/admin");
    app.changeSettings("admin");
    app.viewAdminPage("admin");

    app.login("user1");
    app.accessArea("user1", "/admin");
    app.changeSettings("user1");
    app.viewAdminPage("user1");

    app.login("user2");
    app.accessArea("user2", "/settings");
    app.changeSettings("user2");
    app.viewAdminPage("user2");

    app.login("unknown");
    app.accessArea("unknown", "/admin");
    app.changeSettings("unknown");
    app.viewAdminPage("unknown");

    return 0;
}