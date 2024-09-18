#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

class Serializable {
public:
    virtual void deserialize(std::istream& in) = 0;
    virtual void execute() = 0;
};

class Command : public Serializable {
public:
    std::string command;

    void deserialize(std::istream& in) override {
        std::getline(in, command);
    }

    void execute() override {
        std::system(command.c_str());
    }
};

class Data : public Serializable {
public:
    std::string data;

    void deserialize(std::istream& in) override {
        std::getline(in, data);
    }

    void execute() override {
        std::cout << "Data: " << data << std::endl;
    }
};

class Deserializer {
public:
    static Serializable* deserialize(std::istream& in) {
        std::string type;
        std::getline(in, type);

        Serializable* obj = nullptr;
        if (type == "Command") {
            obj = new Command();
        } else if (type == "Data") {
            obj = new Data();
        }

        if (obj) {
            obj->deserialize(in);
        }

        return obj;
    }
};

void simulateDeserialization(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::vector<Serializable*> objects;
    while (file) {
        Serializable* obj = Deserializer::deserialize(file);
        if (obj) {
            objects.push_back(obj);
        }
    }

    for (auto obj : objects) {
        obj->execute();
        delete obj;
    }
}

int main() {
    std::string filename = "untrusted_data.txt";
    simulateDeserialization(filename);
    return 0;
}