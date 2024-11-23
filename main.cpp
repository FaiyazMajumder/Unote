#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>

using namespace std;

// We use this class to do our file handling operations.
class FileHandler {
public:
    // Opens a file and reads it's content here.
    bool openFile(const string& filename, vector<string>& lines) {
        ifstream file(filename);
        if (!file.is_open()) return false; // Returns false when file cannot be opened, in most cases the file just don't exists
        string line;
        while (getline(file, line)) {
            lines.push_back(line); //Adds line to the vector
        }
        file.close();
        return true; // Returns true when it works :3
    }

    // We are using this to save the file contents
    bool saveFile(const string& filename, const vector<string>& lines) {
        ofstream file(filename);
        if (!file.is_open()) return false;
        for (const auto& line : lines) {
            file << line << "\n"; // Writes each line onto the file, which is inefficient but hey, it works.
        }
        file.close(); // Closes the file, obviously.
        return true;
    }

    // Creates a new empty file
    bool createFile(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) return false;
        file.close();
        return true;
    }
};

// Class to handle the cursor position in the text editor
class Cursor {
public:
    int x = 0, y = 0; // Cursor coordinates (x: column, y: row)

    void moveUp() { if (y > 0) y--; } // Move cursor up
    void moveDown(int max_y) { if (y < max_y - 1) y++; } // Move cursor down
    void moveLeft() { if (x > 0) x--; } // Move cursor left
    void moveRight(int max_x) { if (x < max_x) x++; } // Move cursor right
};

// The class to handle our copy, paste, and cut operations
class Clipboard {
private:
    string buffer; // We need to use buffer to store copied or cut text
public:
    void copy(const string& text) { buffer = text; } // Copies the text to buffer
    string paste() const { return buffer; } // Pastes copied text from buffer
    void cut(string& text) {
        buffer = text; // Copies text to buffer
        text.clear(); // Clears the original text
    }
};

// Class to manage undo and redo operations for our note
class UndoRedoManager {
private:
    stack<vector<string>> undoStack;
    stack<vector<string>> redoStack;
public:
    void saveState(const vector<string>& lines) {
        undoStack.push(lines);
        while (!redoStack.empty()) redoStack.pop();
    }

    bool undo(vector<string>& lines) {
        if (undoStack.empty()) return false; // Returns false if undo stack is empty
        redoStack.push(lines); // Save current state to redo stack
        lines = undoStack.top();
        undoStack.pop();
        return true;
    }

    bool redo(vector<string>& lines) {
        if (redoStack.empty()) return false;
        undoStack.push(lines);
        lines = redoStack.top();
        redoStack.pop();
        return true;
    }
};

// Class to handle the text editor operations
class Editor {
private:
    vector<string> lines; // Using vector to store lines of text
    FileHandler fileHandler; // Instance of FileHandler for file operations
    Cursor cursor; // Instance of Cursor to manage cursor position
    Clipboard clipboard; // Instance of Clipboard for copy/cut/paste
    UndoRedoManager undoRedo; // Instance of UndoRedoManager for undo/redo
    string filename; // Filename for the current file
public:
    Editor() : filename("") {}
    void createNewFile(const string& filename) {
        if (fileHandler.createFile(filename)) {
            this->filename = filename;
            lines.clear(); // Clears current lines
            cout << "New file created: " << filename << "\n";
        } else {
            cout << "Error: Could not create file.\n";
        }
    }

    // Opens an existing file or create a new one if it doesn't exist
    void open(const string& filename) {
        if (fileHandler.openFile(filename, lines)) {
            this->filename = filename;
            cout << "Opened file: " << filename << "\n";
        } else {
            cout << "File not found. Creating a new file: " << filename << "\n";
            createNewFile(filename);
        }
    }

    // Saves the current file
    void save() {
        if (fileHandler.saveFile(filename, lines)) {
            cout << "File saved.\n";
        } else {
            cout << "Error: Could not save file.\n";
        }
    }

    // Inserts text at the current cursor position
    void insertText(const string& text) {
        undoRedo.saveState(lines); // Saves the current state for undo
        if (lines.empty()) lines.push_back(""); // Ensures at least one line exists
        lines[cursor.y].insert(cursor.x, text); // Inserts text at cursor position
        cursor.x += text.length(); // Moves cursor to the end of the inserted text
    }

    // Deletes the last character of said line, highly inefficient will need to work on this properly
    void deleteText() {
        if (lines[cursor.y].empty() || cursor.x == 0) return;
        undoRedo.saveState(lines);
        lines[cursor.y].erase(--cursor.x, 1); // Erase character at cursor position
    }


    void display() const {
        for (int i = 0; i < lines.size(); ++i) {
            cout << (i + 1) << ": " << lines[i] << "\n"; // Print each line with its number
        }
    }

    // Interactive edit menu for user input
    void interactiveEdit() {
        char choice;
        string input;
        while (true) {
            // Display menu
            cout << "\n--- Editor Menu ---\n";
            cout << "1. Insert Text\n";
            cout << "2. Delete Text\n";
            cout << "3. Save\n";
            cout << "4. Display\n";
            cout << "5. Exit\n";
            cout << "Enter choice: ";
            cin >> choice;
            cin.ignore(); // Ignore newline character

            switch (choice) { //Using switch case instead of if else, as it's far more efficient in this context.
                case '1':
                    cout << "Enter text to insert: ";
                    getline(cin, input); // Read input text
                    insertText(input); // Insert the input text
                    break;
                case '2':
                    deleteText(); // Delete text at the current cursor position
                    cout << "Deleted last character at cursor position.\n";
                    break;
                case '3':
                    save(); // Save the current file
                    break;
                case '4':
                    display(); // Display the current text
                    break;
                case '5':
                    cout << "Exiting editor...\n";
                    return; // Exit the editor
                default:
                    cout << "Invalid choice. Try again.\n";
            }
        }
    }
};

int main() {
    Editor editor;
    string filename;

    cout << "Enter filename to open or create: ";
    getline(cin, filename); // Read filename from user
    editor.open(filename); // Open the file

    editor.interactiveEdit(); // Launches the interactive editing session

    return 0;
}
