//============================================================================
// Name        : ProjectTwo.cpp
// Author      : Avery Andrews
// Version     : 1.0
// Description : CS 300 Project Two - ABCU Advising Assistance Program (BST)
//============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

/**
 * Structure to hold course information
 */
struct Course {
    std::string courseNumber;
    std::string courseTitle;
    std::vector<std::string> prerequisites;
};

/**
 * Internal structure for Binary Search Tree node
 */
struct Node {
    Course course;
    Node* left;
    Node* right;

    // Default constructor
    Node() : left(nullptr), right(nullptr) {}

    // Initialize node with a Course object
    Node(Course aCourse) : course(aCourse), left(nullptr), right(nullptr) {}
};

/**
 * Binary Search Tree class to store, search, and sort courses alphanumerically
 */
class BinarySearchTree {
private:
    Node* root;

    /**
     * Recursively inserts a course into the tree based on courseNumber
     */
    void addNode(Node* node, Course course) {
        // If new courseNumber is alphanumerically less than current node's courseNumber
        if (course.courseNumber < node->course.courseNumber) {
            if (node->left == nullptr) {
                node->left = new Node(course);
            }
            else {
                addNode(node->left, course);
            }
        }
        // Otherwise, traverse or insert on the right subtree
        else {
            if (node->right == nullptr) {
                node->right = new Node(course);
            }
            else {
                addNode(node->right, course);
            }
        }
    }

    /**
     * Performs an in-order traversal (Left, Root, Right) to print courses
     * sorted alphanumerically from lowest to highest
     */
    void PrintInOrder(Node* node) const {
        if (node != nullptr) {
            PrintInOrder(node->left);
            std::cout << node->course.courseNumber << ", "
                << node->course.courseTitle << std::endl;
            PrintInOrder(node->right);
        }
    }

    /**
     * Recursively frees dynamically allocated tree nodes to prevent memory leaks
     */
    void destroyTree(Node* node) {
        if (node != nullptr) {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }

public:
    // Constructor initializes an empty tree
    BinarySearchTree() : root(nullptr) {}

    // Destructor cleans up all allocated nodes
    ~BinarySearchTree() {
        destroyTree(root);
    }

    /**
     * Resets the tree so multiple input files can be loaded cleanly
     */
    void Clear() {
        destroyTree(root);
        root = nullptr;
    }

    /**
     * Inserts a Course object into the Binary Search Tree
     */
    void Insert(Course course) {
        if (root == nullptr) {
            root = new Node(course);
        }
        else {
            addNode(root, course);
        }
    }

    /**
     * Public method to print the entire course schedule in alphanumeric order
     */
    void PrintInOrder() const {
        PrintInOrder(root);
    }

    /**
     * Searches the BST for a matching courseNumber and returns the Course object
     */
    Course Search(const std::string& courseNumber) const {
        Node* current_node = root;

        while (current_node != nullptr) {
            if (current_node->course.courseNumber == courseNumber) {
                return current_node->course;
            }
            if (courseNumber < current_node->course.courseNumber) {
                current_node = current_node->left;
            }
            else {
                current_node = current_node->right;
            }
        }

        // Return an empty Course if no match is found
        return Course();
    }
};

/**
 * Helper function to strip leading/trailing whitespace and newline/carriage returns
 */
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

/**
 * Helper function to convert a string to uppercase for case-insensitive lookup
 */
std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return str;
}

/**
 * Reads course data from a CSV file, validates line format and prerequisites,
 * and loads the courses into the Binary Search Tree
 */
bool LoadData(const std::string& filename, BinarySearchTree& tree) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: Could not open file \"" << filename << "\"." << std::endl;
        return false;
    }

    // Clear existing data in case Option 1 is run multiple times
    tree.Clear();

    std::vector<std::string> validCourseNumbers;
    std::vector<std::vector<std::string>> parsedRows;
    std::string line;

    // Pass 1: Read lines, strip any UTF-8 BOM, split by commas, and validate format
    while (std::getline(file, line)) {
        if (line.size() >= 3 &&
            static_cast<unsigned char>(line[0]) == 0xEF &&
            static_cast<unsigned char>(line[1]) == 0xBB &&
            static_cast<unsigned char>(line[2]) == 0xBF) {
            line = line.substr(3);
        }

        line = trim(line);
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            token = trim(token);
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }

        // Ensure line has at least Course Number and Course Title
        if (tokens.size() < 2) {
            std::cout << "Formatting error: skipping line with missing course number or title." << std::endl;
            continue;
        }

        validCourseNumbers.push_back(toUpper(tokens[0]));
        parsedRows.push_back(tokens);
    }
    file.close();

    // Pass 2: Validate prerequisite existence against catalog and insert into BST
    int loadedCount = 0;
    for (const auto& tokens : parsedRows) {
        Course course;
        course.courseNumber = toUpper(tokens[0]);
        course.courseTitle = tokens[1];

        for (size_t i = 2; i < tokens.size(); ++i) {
            std::string prereq = toUpper(tokens[i]);
            if (std::find(validCourseNumbers.begin(), validCourseNumbers.end(), prereq) != validCourseNumbers.end()) {
                course.prerequisites.push_back(prereq);
            }
            else {
                std::cout << "Formatting warning: Prerequisite " << prereq
                    << " for " << course.courseNumber
                    << " is not in the catalog and was omitted." << std::endl;
            }
        }

        tree.Insert(course);
        loadedCount++;
    }

    std::cout << loadedCount << " courses loaded successfully." << std::endl;
    return true;
}

/**
 * Looks up a specific course and prints its number, title, and prerequisites
 */
void PrintCourse(const BinarySearchTree& courseTree, const std::string& searchNumber) {
    Course result = courseTree.Search(toUpper(searchNumber));

    if (result.courseNumber.empty()) {
        std::cout << "Course \"" << searchNumber << "\" not found." << std::endl;
        return;
    }

    // Print the main course number and title
    std::cout << result.courseNumber << ", " << result.courseTitle << std::endl;

    // 1. Print the exact format shown in the Sample Output
    std::cout << "Prerequisites: ";
    if (result.prerequisites.empty()) {
        std::cout << "None" << std::endl;
    }
    else {
        for (size_t i = 0; i < result.prerequisites.size(); ++i) {
            std::cout << result.prerequisites[i];
            if (i < result.prerequisites.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;

        // 2. Also print Prerequisite Numbers and Titles as specified in Directions #3
        for (const std::string& prereqCode : result.prerequisites) {
            Course prereqCourse = courseTree.Search(prereqCode);
            if (!prereqCourse.courseNumber.empty()) {
                std::cout << "  - " << prereqCourse.courseNumber << ", "
                    << prereqCourse.courseTitle << std::endl;
            }
        }
    }
}

/**
 * Main entry point providing the interactive advising menu
 */
int main() {
    BinarySearchTree courseTree;
    bool dataLoaded = false;
    int choice = 0;

    std::cout << "Welcome to the course planner." << std::endl;

    while (choice != 9) {
        std::cout << "\n  1. Load Data Structure." << std::endl;
        std::cout << "  2. Print Course List." << std::endl;
        std::cout << "  3. Print Course." << std::endl;
        std::cout << "  9. Exit\n" << std::endl;
        std::cout << "What would you like to do? ";

        // Handle non-numeric input safely without entering an infinite loop
        if (!(std::cin >> choice)) {
            std::cout << "Invalid input. Please enter a valid menu number." << std::endl;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }
        std::cin.ignore(10000, '\n'); // Clear trailing newline from input buffer

        switch (choice) {
        case 1: {
            std::string filename;
            std::cout << "Enter the file name containing the course data: ";
            std::getline(std::cin, filename);
            filename = trim(filename);

            // Default to the official input file if the user just presses Enter
            if (filename.empty()) {
                filename = "ABCU_Advising_Program_Input.csv";
            }

            if (LoadData(filename, courseTree)) {
                dataLoaded = true;
            }
            break;
        }

        case 2:
            if (!dataLoaded) {
                std::cout << "Error: Please load the data structure first (Option 1)." << std::endl;
            }
            else {
                std::cout << "Here is a sample schedule:\n" << std::endl;
                courseTree.PrintInOrder();
            }
            break;

        case 3: {
            if (!dataLoaded) {
                std::cout << "Error: Please load the data structure first (Option 1)." << std::endl;
            }
            else {
                std::string searchNumber;
                std::cout << "What course do you want to know about? ";
                std::getline(std::cin, searchNumber);
                PrintCourse(courseTree, trim(searchNumber));
            }
            break;
        }

        case 9:
            std::cout << "Thank you for using the course planner!" << std::endl;
            break;

        default:
            std::cout << choice << " is not a valid option." << std::endl;
            break;
        }
    }

    return 0;
}