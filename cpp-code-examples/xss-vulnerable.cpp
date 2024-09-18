#include <iostream>
#include <string>
#include <map>
#include <regex>

// Simulate a simple HTTP request
struct HttpRequest {
    std::string method;
    std::string path;
    std::map<std::string, std::string> queryParams;
};

// Simulate a simple HTTP response
struct HttpResponse {
    int statusCode;
    std::string body;
};

// Function to parse query parameters from a URL
std::map<std::string, std::string> parseQueryParams(const std::string& url) {
    std::map<std::string, std::string> queryParams;
    std::regex queryParamRegex("([\\w+%]+)=([^&]*)");
    auto queryBegin = std::sregex_iterator(url.begin(), url.end(), queryParamRegex);
    auto queryEnd = std::sregex_iterator();

    for (std::sregex_iterator i = queryBegin; i != queryEnd; ++i) {
        std::smatch match = *i;
        std::string key = match[1].str();
        std::string value = match[2].str();
        queryParams[key] = value;
    }

    return queryParams;
}

// Function to handle HTTP requests
HttpResponse handleRequest(const HttpRequest& request) {
    HttpResponse response;
    response.statusCode = 200;

    if (request.path == "/greet") {
        std::string name = request.queryParams.at("name");
        response.body = "<html><body><h1>Hello, " + name + "!</h1></body></html>";
    } else {
        response.statusCode = 404;
        response.body = "<html><body><h1>404 Not Found</h1></body></html>";
    }

    return response;
}

// Function to simulate a web server
void runServer() {
    std::string requestLine;
    while (true) {
        std::cout << "Enter HTTP request (or 'exit' to quit): ";
        std::getline(std::cin, requestLine);

        if (requestLine == "exit") {
            break;
        }

        // Parse the HTTP request
        std::regex requestLineRegex("(GET|POST)\\s+([^\\s]+)\\s+HTTP/1.1");
        std::smatch match;
        if (std::regex_match(requestLine, match, requestLineRegex)) {
            HttpRequest request;
            request.method = match[1].str();
            std::string url = match[2].str();

            // Split the URL into path and query string
            size_t queryPos = url.find('?');
            if (queryPos != std::string::npos) {
                request.path = url.substr(0, queryPos);
                std::string queryString = url.substr(queryPos + 1);
                request.queryParams = parseQueryParams(queryString);
            } else {
                request.path = url;
            }

            // Handle the request and generate a response
            HttpResponse response = handleRequest(request);

            // Output the response
            std::cout << "HTTP/1.1 " << response.statusCode << " OK\n";
            std::cout << "Content-Type: text/html\n\n";
            std::cout << response.body << "\n";
        } else {
            std::cout << "Invalid HTTP request format.\n";
        }
    }
}

int main() {
    std::cout << "Starting web server...\n";
    runServer();
    std::cout << "Server stopped.\n";
    return 0;
}