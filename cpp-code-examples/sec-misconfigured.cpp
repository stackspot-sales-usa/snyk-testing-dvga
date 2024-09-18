#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>

class WebServer {
public:
    WebServer(int port) : port(port) {
        // Default settings
        rootDirectory = "/var/www/html";
        enableDirectoryListing = true;
        enableDefaultCredentials = true;
        enableUnnecessaryServices = true;
        loadDefaultConfig();
    }

    void start() {
        std::cout << "Starting web server on port " << port << "...\n";
        while (true) {
            // Simulate handling requests
            std::this_thread::sleep_for(std::chrono::seconds(1));
            handleRequest();
        }
    }

private:
    int port;
    std::string rootDirectory;
    bool enableDirectoryListing;
    bool enableDefaultCredentials;
    bool enableUnnecessaryServices;
    std::map<std::string, std::string> defaultCredentials;
    std::vector<std::string> unnecessaryServices;

    void loadDefaultConfig() {
        // Load default credentials
        defaultCredentials["admin"] = "admin";
        defaultCredentials["user"] = "password";

        // Load unnecessary services
        unnecessaryServices.push_back("telnet");
        unnecessaryServices.push_back("ftp");
        unnecessaryServices.push_back("finger");

        std::cout << "Loaded default configuration.\n";
    }

    void handleRequest() {
        // Simulate a request
        std::string request = "GET /index.html HTTP/1.1";
        std::cout << "Handling request: " << request << "\n";

        // Check for directory listing
        if (enableDirectoryListing) {
            std::cout << "Directory listing is enabled. Exposing file structure...\n";
            listDirectory(rootDirectory);
        }

        // Check for default credentials
        if (enableDefaultCredentials) {
            std::cout << "Default credentials are enabled. Potential security risk...\n";
            for (const auto& cred : defaultCredentials) {
                std::cout << "Username: " << cred.first << ", Password: " << cred.second << "\n";
            }
        }

        // Check for unnecessary services
        if (enableUnnecessaryServices) {
            std::cout << "Unnecessary services are enabled. Potential security risk...\n";
            for (const auto& service : unnecessaryServices) {
                std::cout << "Service: " << service << " is running.\n";
            }
        }
    }

    void listDirectory(const std::string& path) {
        // Simulate directory listing
        std::cout << "Listing directory: " << path << "\n";
        std::vector<std::string> files = {"index.html", "about.html", "contact.html"};
        for (const auto& file : files) {
            std::cout << file << "\n";
        }
    }
};

int main() {
    // Simulate reading configuration from a file
    std::ifstream configFile("server_config.txt");
    std::string line;
    while (std::getline(configFile, line)) {
        std::cout << "Config: " << line << "\n";
    }

    // Start the web server
    WebServer server(8080);
    server.start();

    return 0;
}