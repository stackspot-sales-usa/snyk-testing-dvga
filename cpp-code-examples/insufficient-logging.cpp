#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class User {
public:
    std::string username;
    std::string password;
    double balance;

    User(std::string uname, std::string pwd, double bal) : username(uname), password(pwd), balance(bal) {}
};

class BankSystem {
private:
    std::unordered_map<std::string, User> users;
    std::vector<std::string> logs;

public:
    BankSystem() {
        // Adding some users
        users["user1"] = User("user1", "password1", 1000.0);
        users["user2"] = User("user2", "password2", 2000.0);
    }

    bool login(std::string username, std::string password) {
        if (users.find(username) != users.end() && users[username].password == password) {
            std::cout << "Login successful for user: " << username << std::endl;
            return true;
        } else {
            std::cout << "Login failed for user: " << username << std::endl;
            // Insufficient logging: No detailed log of failed attempts
            return false;
        }
    }

    void viewBalance(std::string username) {
        if (users.find(username) != users.end()) {
            std::cout << "Balance for user " << username << ": " << users[username].balance << std::endl;
        } else {
            std::cout << "User not found: " << username << std::endl;
            // Insufficient logging: No log of unauthorized access attempts
        }
    }

    void transferMoney(std::string fromUser, std::string toUser, double amount) {
        if (users.find(fromUser) != users.end() && users.find(toUser) != users.end()) {
            if (users[fromUser].balance >= amount) {
                users[fromUser].balance -= amount;
                users[toUser].balance += amount;
                std::cout << "Transfer successful from " << fromUser << " to " << toUser << " of amount " << amount << std::endl;
                // Insufficient logging: No log of transaction details
            } else {
                std::cout << "Insufficient balance for user: " << fromUser << std::endl;
                // Insufficient logging: No log of failed transactions
            }
        } else {
            std::cout << "Invalid users for transfer" << std::endl;
            // Insufficient logging: No log of invalid transfer attempts
        }
    }

    void simulateAttack() {
        // Simulating a brute force attack
        for (int i = 0; i < 5; ++i) {
            login("user1", "wrongpassword" + std::to_string(i));
        }

        // Simulating unauthorized access
        viewBalance("unknownUser");

        // Simulating a transaction anomaly
        transferMoney("user1", "user2", 5000.0);
    }
};

int main() {
    BankSystem bank;

    // Normal operations
    bank.login("user1", "password1");
    bank.viewBalance("user1");
    bank.transferMoney("user1", "user2", 100.0);

    // Simulating attacks
    bank.simulateAttack();

    return 0;
}