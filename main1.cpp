#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <conio.h> // For _getch on Windows
#include <windows.h> // For file listing on Windows

using namespace std;

// Function to hide input characters and read password
string getHiddenPassword() {
    string password;
    char ch;
    while ((ch = _getch()) != '\r') { // '\r' is Enter key
        if (ch == '\b') { // Handle backspace
            if (!password.empty()) {
                cout << "\b \b";
                password.pop_back();
            }
        } else {
            password += ch;
            cout << '*'; // Mask input
        }
    }
    cout << endl;
    return password;
}

// Utility function for simple XOR-based encryption
string encryptDecrypt(const string& data, const string& key) {
    string result = data;
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] ^= key[i % key.size()];
    }
    return result;
}

// File to store metadata (password info) for each file
string getMetaFileName(const string& filename) {
    return "." + filename + ".meta";
}

// Function to list all files in the current directory
void listFiles() {
    WIN32_FIND_DATA fileData;
    HANDLE hFind = FindFirstFile("*", &fileData); // Wildcard to find all files

    if (hFind == INVALID_HANDLE_VALUE) {
        cout << "No files found in the current directory.\n";
        return;
    }

    cout << "\n--- Available Files ---\n";
    do {
        if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            cout << "- " << fileData.cFileName << "\n";
        }
    } while (FindNextFile(hFind, &fileData));
    FindClose(hFind);
    cout << "-----------------------\n";
}

// Handles file operations including password protection
class FileHandler {
public:
    bool openFile(const string& filename, vector<string>& lines) {
        ifstream file(filename);
        if (!file.is_open()) return false;
        string line;
        while (getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
        return true;
    }

    bool saveFile(const string& filename, const vector<string>& lines) {
        ofstream file(filename);
        if (!file.is_open()) return false;
        for (const auto& line : lines) {
            file << line << "\n";
        }
        file.close();
        return true;
    }

    bool createFile(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) return false;
        file.close();
        return true;
    }

    bool isProtected(const string& filename) {
        ifstream metaFile(getMetaFileName(filename));
        return metaFile.is_open();
    }

    void saveProtection(const string& filename, const string& password) {
        ofstream metaFile(getMetaFileName(filename));
        if (metaFile.is_open()) {
            metaFile << encryptDecrypt("protected", password);
            metaFile.close();
        }
    }

    bool loadProtection(const string& filename, string& password) {
        ifstream metaFile(getMetaFileName(filename));
        if (metaFile.is_open()) {
            string encryptedStatus;
            getline(metaFile, encryptedStatus);
            password = encryptedStatus;
            metaFile.close();
            return true;
        }
        return false;
    }

    void removeProtection(const string& filename) {
        remove(getMetaFileName(filename).c_str());
    }
};

// Class to handle text editor operations
class Editor {
private:
    vector<string> lines;
    FileHandler fileHandler;
    string filename;
    string filePassword;

    void display() const {
        cout << "\n--- File Content ---\n";
        for (size_t i = 0; i < lines.size(); ++i) {
            cout << (i + 1) << ": " << lines[i] << "\n";
        }
    }

    void inlineEditLine(size_t lineNumber) {
        if (lineNumber < 1 || lineNumber > lines.size()) {
            cout << "Invalid line number.\n";
            return;
        }

        string& line = lines[lineNumber - 1];
        cout << "Editing line " << lineNumber << ": " << line << "\n";
        cout << "Use keyboard to edit text. Press Enter when done.\n";

        string editedLine;
        char ch;
        for (size_t i = 0; i < line.size(); i++) {
            cout << line[i];
        }
        while ((ch = _getch()) != '\r') { // '\r' is Enter key
            if (ch == '\b') { // Backspace
                if (!editedLine.empty()) {
                    cout << "\b \b";
                    editedLine.pop_back();
                }
            } else {
                cout << ch;
                editedLine += ch;
            }
        }

        line = editedLine;
        cout << "\nLine " << lineNumber << " updated.\n";
    }

    void inlineInsertInLine(size_t lineNumber) {
        if (lineNumber < 1 || lineNumber > lines.size() + 1) {
            cout << "Invalid line number.\n";
            return;
        }

        string newLine;
        cout << "Enter the text to insert: ";
        getline(cin, newLine);

        lines.insert(lines.begin() + lineNumber - 1, newLine);
        cout << "Text inserted successfully at line " << lineNumber << ".\n";
    }

    void deleteFromLine(size_t lineNumber) {
        if (lineNumber < 1 || lineNumber > lines.size()) {
            cout << "Invalid line number.\n";
            return;
        }

        string& line = lines[lineNumber - 1];
        cout << "Deleting from line " << lineNumber << ": " << line << "\n";
        cout << "Use keyboard to delete text. Press Enter when done.\n";

        string editedLine;
        char ch;
        while ((ch = _getch()) != '\r') { // '\r' is Enter key
            if (ch == '\b') { // Backspace
                if (!editedLine.empty()) {
                    cout << "\b \b";
                    editedLine.pop_back();
                }
            } else {
                cout << ch;
                editedLine += ch;
            }
        }

        line = editedLine;
        cout << "\nLine " << lineNumber << " updated after deletion.\n";
    }

    void protectFile() {
        cout << "Enter a code word to protect this file: ";
        filePassword = getHiddenPassword();
        fileHandler.saveProtection(filename, filePassword);
        cout << "File is now protected.\n";
    }

    void removeProtection() {
        cout << "To remove protection, enter the current password.\n";
        if (verifyPassword()) {
            fileHandler.removeProtection(filename);
            filePassword.clear();
            cout << "Protection removed successfully.\n";
        }
    }

    bool verifyPassword() {
        if (filePassword.empty()) return true;
        for (int attempt = 1; attempt <= 3; ++attempt) {
            cout << "Enter the code word (attempt " << attempt << " of 3): ";
            string enteredPassword = getHiddenPassword();
            if (encryptDecrypt("protected", enteredPassword) == filePassword) return true;
            cout << "Incorrect password. Please try again.\n";
        }
        cout << "Access denied. Exiting program...\n";
        exit(0);
    }

public:
    void createNewFile(const string& filename) {
        if (fileHandler.createFile(filename)) {
            this->filename = filename;
            lines.clear();
            cout << "New file created: " << filename << "\n";
        } else {
            cout << "Error: Could not create file.\n";
        }
    }

    void open(const string& filename) {
        if (fileHandler.isProtected(filename)) {
            if (!fileHandler.loadProtection(filename, filePassword) || !verifyPassword()) {
                cout << "Access denied. Exiting program...\n";
                exit(0);
            }
        }
        if (fileHandler.openFile(filename, lines)) {
            this->filename = filename;
            cout << "Opened file: " << filename << "\n";
        } else {
            cout << "Error: File not found.\n";
        }
    }

    void save() {
        if (fileHandler.saveFile(filename, lines)) {
            cout << "File saved successfully.\n";
        } else {
            cout << "Error: Could not save file.\n";
        }
    }

    void interactiveEdit() {
        char choice;
        while (true) {
            cout << "\n--- Editor Menu ---\n";
            cout << "1. Edit Text\n";
            cout << "2. Insert Text\n";
            cout << "3. Delete Text\n";
            cout << "4. Protect File\n";
            cout << "5. Remove Protection\n";
            cout << "6. Display Content\n";
            cout << "7. Save and Exit\n";
            cout << "Enter choice: ";
            cin >> choice;
            cin.ignore();

            switch (choice) {
                case '1': {
                    display();
                    cout << "Enter the line number to edit: ";
                    size_t lineNumber;
                    cin >> lineNumber;
                    cin.ignore();
                    inlineEditLine(lineNumber);
                    break;
                }
                case '2': {
                    display();
                    cout << "Enter the line number to insert before: ";
                    size_t lineNumber;
                    cin >> lineNumber;
                    cin.ignore();
                    inlineInsertInLine(lineNumber);
                    break;
                }
                case '3': {
                    display();
                    cout << "Enter the line number to delete from: ";
                    size_t lineNumber;
                    cin >> lineNumber;
                    cin.ignore();
                    deleteFromLine(lineNumber);
                    break;
                }
                case '4':
                    protectFile();
                    break;
                case '5':
                    removeProtection();
                    break;
                case '6':
                    display();
                    break;
                case '7':
                    save();
                    return;
                default:
                    cout << "Invalid choice. Try again.\n";
            }
        }
    }
};

int main() {
    Editor editor;
    string filename;
    char mainChoice;

    while (true) {
        cout << "\n--- Main Menu ---\n";
        cout << "1. Open an existing file\n";
        cout << "2. Create a new file\n";
        cout << "3. Exit\n";
        cout << "Enter choice: ";
        cin >> mainChoice;
        cin.ignore();

        switch (mainChoice) {
            case '1':
                listFiles();
                cout << "Enter filename to open: ";
                getline(cin, filename);
                editor.open(filename);
                editor.interactiveEdit();
                break;
            case '2':
                cout << "Enter filename to create: ";
                getline(cin, filename);
                editor.createNewFile(filename);
                editor.interactiveEdit();
                break;
            case '3':
                cout << "Exiting program...\n";
                return 0;
            default:
                cout << "Invalid choice. Try again.\n";
        }
    }
}
