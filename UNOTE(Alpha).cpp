#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <conio.h> // For _getch on Windows
#include <windows.h> // For directory listing on Windows

using namespace std;

// Function to hide input characters and read password
string getHiddenPassword()
{
    string password;
    char ch;
    while ((ch = _getch()) != '\r')   // '\r' is Enter key
    {
        if (ch == '\b')   // Handle backspace
        {
            if (!password.empty())
            {
                cout << "\b \b";
                password.pop_back();
            }
        }
        else
        {
            password += ch;
            cout << '*'; // Mask input
        }
    }
    cout << endl;
    return password;
}

// Utility function for simple XOR-based encryption
string encryptDecrypt(const string& data, const string& key)
{
    string result = data;
    for (size_t i = 0; i < data.size(); ++i)
    {
        result[i] ^= key[i % key.size()];
    }
    return result;
}

// File to store metadata (password info) for each file
string getMetaFileName(const string& filename)
{
    return "." + filename + ".meta";
}

// Function to list all files in the current directory
void listFiles()
{
    cout << "\n--- Available Files ---\n";

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("*", &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        cerr << "Error: Unable to list files.\n";
        return;
    }

    do
    {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            cout << "- " << findFileData.cFileName << "\n";
        }
    }
    while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

    cout << "-----------------------\n";
}

// Handles file operations including password protection
class FileHandler
{
public:
    bool openFile(const string& filename, vector<string>& lines)
    {
        lines.clear(); // Clear any existing content before loading a new file

        ifstream file(filename);
        if (!file.is_open())
        {
            cerr << "Error: Failed to open file '" << filename << "'.\n";
            return false;
        }
        string line;
        while (getline(file, line))
        {
            lines.push_back(line);
        }
        file.close();
        return true;
    }


    bool saveFile(const string& filename, const vector<string>& lines)
    {
        ofstream file(filename);
        if (!file.is_open())
        {
            cerr << "Error: Failed to save file '" << filename << "'.\n";
            return false;
        }
        for (const auto& line : lines)
        {
            file << line << "\n";
        }
        file.close();
        return true;
    }

    bool createFile(const string& filename)
    {
        if (filename.empty())
        {
            cerr << "Error: Filename cannot be empty.\n";
            return false;
        }
        ofstream file(filename);
        if (!file.is_open())
        {
            cerr << "Error: Could not create file '" << filename << "'.\n";
            return false;
        }
        file.close();
        return true;
    }

    bool isProtected(const string& filename)
    {
        ifstream metaFile(getMetaFileName(filename));
        return metaFile.is_open();
    }

    void saveProtection(const string& filename, const string& password)
    {
        ofstream metaFile(getMetaFileName(filename));
        if (metaFile.is_open())
        {
            metaFile << encryptDecrypt("protected", password);
            metaFile.close();
        }
    }

    bool loadProtection(const string& filename, string& password)
    {
        ifstream metaFile(getMetaFileName(filename));
        if (metaFile.is_open())
        {
            string encryptedStatus;
            getline(metaFile, encryptedStatus);
            password = encryptedStatus;
            metaFile.close();
            return true;
        }
        return false;
    }

    void removeProtection(const string& filename)
    {
        DeleteFile(getMetaFileName(filename).c_str());
    }
};


// Class to handle text editor operations
class Editor
{
private:
    vector<string> lines;
    FileHandler fileHandler;
    string filename;
    string filePassword;

    void display() const
    {
        cout << "\n--- File Content ---\n";
        for (size_t i = 0; i < lines.size(); ++i)
        {
            cout << (i + 1) << ": " << lines[i] << "\n";
        }
    }

    void inlineEditLines(size_t startLine, size_t endLine)
    {
        if (startLine < 1 || endLine > lines.size() || startLine > endLine)
        {
            cout << "Invalid line numbers.\n";
            return;
        }

        for (size_t i = startLine - 1; i < endLine; ++i)
        {
            cout << "Editing line " << (i + 1) << ": " << lines[i] << "\n";
            cout << "New text: ";
            string newText;
            getline(cin, newText);
            lines[i] = newText;
            cout << "Line updated.\n";
        }
    }

    void inlineInsertInLine(size_t lineNumber)
    {
        if (lineNumber < 1 || lineNumber > lines.size() + 1)
        {
            cout << "Invalid line number.\n";
            return;
        }
        cout << "Enter the text to insert: ";
        string newLine;
        getline(cin, newLine);
        lines.insert(lines.begin() + lineNumber - 1, newLine);
        cout << "Text inserted successfully at line " << lineNumber << ".\n";
    }

    void deleteLines(size_t startLine, size_t endLine)
    {
        if (startLine < 1 || endLine > lines.size() || startLine > endLine)
        {
            cout << "Invalid line numbers.\n";
            return;
        }
        lines.erase(lines.begin() + startLine - 1, lines.begin() + endLine);
        cout << "Lines deleted successfully.\n";
    }

    void protectFile()
    {
        cout << "Enter a code word to protect this file: ";
        filePassword = getHiddenPassword();
        if (!filePassword.empty())
        {
            fileHandler.saveProtection(filename, filePassword); // Save the new password
            cout << "File is now protected with a new password.\n";
        }
        else
        {
            cout << "Password cannot be empty. Protection not added.\n";
        }
    }

    void removeProtection()
    {
        cout << "To remove protection, enter the current password.\n";
        if (verifyPassword())
        {
            fileHandler.removeProtection(filename); // Delete the metadata file
            filePassword.clear(); // Clear the in-memory password
            cout << "Protection removed successfully. You can set a new password if needed.\n";
        }
    }


    bool verifyPassword()
    {
        if (filePassword.empty()) return true;
        for (int attempt = 1; attempt <= 3; ++attempt)
        {
            cout << "Enter the code word (attempt " << attempt << " of 3): ";
            string enteredPassword = getHiddenPassword();
            if (encryptDecrypt("protected", enteredPassword) == filePassword) return true;
            cout << "Incorrect password. Please try again.\n";
        }
        cout << "Access denied. Exiting program...\n";
        exit(0);
    }

public:
    void createNewFile(const string& filename)
    {
        if (fileHandler.createFile(filename))
        {
            this->filename = filename;
            lines.clear();
            cout << "New file created: " << filename << "\n";
        }
        else
        {
            cout << "Error: Could not create file.\n";
        }
    }

    void open(const string& filename)
    {
        if (fileHandler.isProtected(filename))
        {
            if (!fileHandler.loadProtection(filename, filePassword) || !verifyPassword())
            {
                cout << "Access denied. Exiting program...\n";
                exit(0);
            }
        }
        if (fileHandler.openFile(filename, lines))
        {
            this->filename = filename;
            cout << "Opened file: " << filename << "\n";
        }
        else
        {
            cout << "Error: File not found.\n";
        }
    }

    void save()
    {
        if (fileHandler.saveFile(filename, lines))
        {
            cout << "File saved successfully.\n";
        }
        else
        {
            cout << "Error: Could not save file.\n";
        }
    }

    void interactiveEdit()
    {
        char choice;
        while (true)
        {
            cout << "\n---*********Editor Menu*********---\n";
            cout << "| 1. Edit Text                   |\n";
            cout << "| 2. Insert Text                 |\n";
            cout << "| 3. Delete Text                 |\n";
            cout << "| 4. Protect File                |\n";
            cout << "| 5. Remove Protection           |\n";
            cout << "| 6. Display Content             |\n";
            cout << "| 7. Save and Exit               |\n";
            cout << "Enter choice: ";
            cin >> choice;
            cin.ignore();
            cout << "\n---******************************---\n";
            switch (choice)
            {
            case '1':
            {
                display();
                size_t startLine, endLine;
                cout << "Enter the start line number to edit: ";
                cin >> startLine;
                cout << "Enter the end line number to edit: ";
                cin >> endLine;
                cin.ignore();
                inlineEditLines(startLine, endLine);
                break;
            }
            case '2':
            {
                display();
                size_t lineNumber;
                cout << "Enter the line number to insert before: ";
                cin >> lineNumber;
                cin.ignore();
                inlineInsertInLine(lineNumber);
                break;
            }
            case '3':
            {
                display();
                size_t startLine, endLine;
                cout << "Enter the start line number to delete from: ";
                cin >> startLine;
                cout << "Enter the end line number to delete to: ";
                cin >> endLine;
                cin.ignore();
                deleteLines(startLine, endLine);
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

int main()
{
    Editor editor;
    string filename;
    char mainChoice;

    while (true)
    {
        cout << "\n---------- Main Menu ----------\n";
        cout << "|  1. Open an existing file   |\n";
        cout << "|  2. Create a new file       |\n";
        cout << "|  3. Exit                    |\n";
        cout << "Enter choice: ";
        cin >> mainChoice;
        cin.ignore();
        cout << "\n==========ooooooooooo==========\n";
        switch (mainChoice)
        {
        case '1':
            listFiles();
            cout << "Enter filename to open: ";
            getline(cin, filename);
            editor.open(filename);
            editor.interactiveEdit();
            break;
        case '2':
            listFiles();
            cout << "Enter filename to create: ";
            getline(cin, filename);
            editor.createNewFile(filename);
            editor.interactiveEdit();
            break;
        case '3':
            cout << "Exiting program...BYeeee t_t\n";
            return 0;
        default:
            cout << "Invalid choice T_T. Try again ^_^.\n";
        }
    }
}



