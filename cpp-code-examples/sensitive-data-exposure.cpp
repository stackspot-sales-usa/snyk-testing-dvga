#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <curl/curl.h>

class User {
public:
    std::string username;
    std::string password;
    std::string email;

    User(std::string u, std::string p, std::string e) : username(u), password(p), email(e) {}
};

void saveUserData(const std::vector<User>& users) {
    std::ofstream file("user_data.txt");
    if (file.is_open()) {
        for (const auto& user : users) {
            file << "Username: " << user.username << "\n";
            file << "Password: " << user.password << "\n";
            file << "Email: " << user.email << "\n";
            file << "--------------------------\n";
        }
        file.close();
    } else {
        std::cerr << "Unable to open file for writing.\n";
    }
}

void readUserData() {
    std::ifstream file("user_data.txt");
    std::string line;
    if (file.is_open()) {
        while (getline(file, line)) {
            std::cout << line << "\n";
        }
        file.close();
    } else {
        std::cerr << "Unable to open file for reading.\n";
    }
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void sendUserDataOverHttp(const std::vector<User>& users) {
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        for (const auto& user : users) {
            std::string postData = "username=" + user.username + "&password=" + user.password + "&email=" + user.email;
            curl_easy_setopt(curl, CURLOPT_URL, "http://example.com/submit_user_data");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

            std::string response;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
            } else {
                std::cout << "Server Response: " << response << "\n";
            }
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

int main() {
    std::vector<User> users;
    users.emplace_back("user1", "password123", "user1@example.com");
    users.emplace_back("user2", "password456", "user2@example.com");
    users.emplace_back("user3", "password789", "user3@example.com");

    // Save user data to a plain text file
    saveUserData(users);

    // Read user data from the plain text file
    readUserData();

    // Send user data over an insecure HTTP channel
    sendUserDataOverHttp(users);

    return 0;
}