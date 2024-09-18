#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <libxml/parser.h>
#include <libxml/tree.h>

// Function to parse XML content
void parseXML(const std::string& xmlContent) {
    // Parse the XML content from memory
    xmlDocPtr doc = xmlReadMemory(xmlContent.c_str(), xmlContent.size(), "noname.xml", NULL, 0);
    if (doc == NULL) {
        throw std::runtime_error("Failed to parse XML");
    }

    // Get the root element of the XML document
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == NULL) {
        xmlFreeDoc(doc);
        throw std::runtime_error("Empty XML document");
    }

    // Print the name of the root element
    std::cout << "Root element: " << root->name << std::endl;

    // Traverse the XML tree and print element names
    for (xmlNodePtr node = root->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE) {
            std::cout << "Element: " << node->name << std::endl;
        }
    }

    // Free the XML document
    xmlFreeDoc(doc);
}

// Function to read the content of a file
std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    try {
        // Read the XML content from a file
        std::string xmlContent = readFile("input.xml");

        // Parse the XML content
        parseXML(xmlContent);
    } catch (const std::exception& e) {
        // Print any errors that occur
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}