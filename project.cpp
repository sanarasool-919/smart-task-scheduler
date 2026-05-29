// ===== SAME HEADERS =====
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
using namespace std;

/* ================= TASK ================= */
struct Task {
    int id;
    string title;
    string date;
    int hour;
    int minute;
    int duration;
    string priority;
};

/* ================= VALIDATION ================= */

bool isLeapYear(int year) {
    if (year % 400 == 0) return true;
    if (year % 100 == 0) return false;
    return (year % 4 == 0);
}


bool isValidDate(string d, string &error) {
    if (d.length() != 10) {
        error = "Date format must be YYYY-MM-DD";
        return false;
    }

    if (d[4] != '-' || d[7] != '-') {
        error = "Date separators must be '-'";
        return false;
    }

    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit(d[i])) {
            error = "Date must contain digits only";
            return false;
        }
    }
    int year  = stoi(d.substr(0, 4));
    int month = stoi(d.substr(5, 2));
    int day   = stoi(d.substr(8, 2));

    if (year < 2025) {
        error = "Year must be 2025 or later";
        return false;
    }

    if (month < 1 || month > 12) {
        error = "Month must be between 1 and 12";
        return false;
    }

    int maxDays;

    if (month == 2) {
        maxDays = isLeapYear(year) ? 29 : 28;
    }
    else if (month == 4 || month == 6 || month == 9 || month == 11) {
        maxDays = 30;
    }
    else {
        maxDays = 31;
    }

    if (day < 1 || day > maxDays) {
        error = "Invalid date for the given month and year";
        return false;
    }

    return true;
}

bool isValidTime(int h, int m, string &error) {
    if (h < 0 || h > 23) {
        error = "Hour must be between 0 and 23 (24-hour format)";
        return false;
    }
    if (m < 0 || m > 59) {
        error = "Minute must be between 0 and 59";
        return false;
    }
    return true;
}

bool isValidDuration(int d, string &error) {
    if (d < 1 || d > 60) {
        error = "Duration must be between 1 and 60 minutes";
        return false;
    }
    return true;
}

bool isValidPriority(string &p, string &error) {
    // convert to lowercase
    transform(p.begin(), p.end(), p.begin(), ::tolower);

    if (p == "high" || p == "medium" || p == "low") {
        return true;
    }

    error = "Priority must be High, Medium, or Low";
    return false;
}
/* ================= TIME COMPARE ================= */
bool earlier(Task a, Task b) {
    if (a.date != b.date) return a.date < b.date;
    if (a.hour != b.hour) return a.hour < b.hour;
    return a.minute < b.minute;
}

bool sameTask(Task a, Task b) {
    return (a.date == b.date &&
            a.hour == b.hour &&
            a.minute == b.minute);
}

bool isConflict(Task a, Task b) {
    if (a.date != b.date) return false;

    int startA = a.hour * 60 + a.minute;
    int endA   = startA + a.duration;
    int startB = b.hour * 60 + b.minute;
    int endB   = startB + b.duration;

    return !(endA <= startB || endB <= startA);
}

/* ================= AVL TREE ================= */
struct AVLNode {
    Task task;
    AVLNode* left;
    AVLNode* right;
    int height;
};

int height(AVLNode* n) { return n ? n->height : 0; }
int max(int a, int b) { return a > b ? a : b; }

AVLNode* newNode(Task t) {
    AVLNode* n = new AVLNode();
    n->task = t;
    n->left = n->right = NULL;
    n->height = 1;
    return n;
}

AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T = x->right;
    x->right = y;
    y->left = T;
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return x;
}

AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T = y->left;
    y->left = x;
    x->right = T;
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    return y;
}

AVLNode* minValueNode(AVLNode* node) {
    AVLNode* current = node;
    while (current && current->left)
        current = current->left;
    return current;
}

int balance(AVLNode* n) {
    return n ? height(n->left) - height(n->right) : 0;
}

bool hasConflict(AVLNode* root, Task t) {
    if (!root) return false;
    if (isConflict(root->task, t)) return true;
    return hasConflict(root->left, t) || hasConflict(root->right, t);
}

AVLNode* insertAVL(AVLNode* root, Task t) {
    if (!root) return newNode(t);

    if (earlier(t, root->task))
        root->left = insertAVL(root->left, t);
    else if (earlier(root->task, t))
        root->right = insertAVL(root->right, t);
    else {
        cout << "Error: Task already exists at same date and time\n";
        return root;
    }

    root->height = 1 + max(height(root->left), height(root->right));
    int b = balance(root);

    if (b > 1 && earlier(t, root->left->task))
        return rotateRight(root);
    if (b < -1 && earlier(root->right->task, t))
        return rotateLeft(root);

    return root;
}

AVLNode* deleteAVL(AVLNode* root, Task t, bool &deleted) {
    if (!root) return root;

    if (earlier(t, root->task))
        root->left = deleteAVL(root->left, t, deleted);
    else if (earlier(root->task, t))
        root->right = deleteAVL(root->right, t, deleted);
    else {
        // Task found
        deleted = true;

        if (!root->left || !root->right) {
            AVLNode* temp = root->left ? root->left : root->right;
            delete root;
            return temp;
        }

        AVLNode* temp = minValueNode(root->right);
        root->task = temp->task;
        root->right = deleteAVL(root->right, temp->task, deleted);
    }

    if (!root) return root;

    root->height = 1 + max(height(root->left), height(root->right));
    int b = balance(root);

    if (b > 1 && balance(root->left) >= 0)
        return rotateRight(root);

    if (b > 1 && balance(root->left) < 0) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }

    if (b < -1 && balance(root->right) <= 0)
        return rotateLeft(root);

    if (b < -1 && balance(root->right) > 0) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }

    return root;
}

bool findTask(AVLNode* root, Task &t) {
    if (!root) return false;

    if (sameTask(root->task, t)) {
        t = root->task; // copy full task
        return true;
    }

    if (earlier(t, root->task))
        return findTask(root->left, t);
    else
        return findTask(root->right, t);
}

/* ================= DISPLAY ================= */
void viewTasks(AVLNode* root) {
    if (!root) return;
    viewTasks(root->left);
    cout << root->task.title << " | "
         << root->task.date << " | "
         << (root->task.hour < 10 ? "0" : "") << root->task.hour << ":"
         << (root->task.minute < 10 ? "0" : "") << root->task.minute
         << " | " << root->task.priority << endl;  
    viewTasks(root->right);
}


/* ================= SEARCH ================= */
void searchByDate(AVLNode* root, string d) {
    if (!root) return;
    searchByDate(root->left, d);
    if (root->task.date == d) {
        cout << root->task.title << " at "
             << root->task.hour << ":"
             << (root->task.minute < 10 ? "0" : "")
             << root->task.minute << endl;
    }
    searchByDate(root->right, d);
}

    void searchByPriority(AVLNode* root, string p) {
    if (!root) return;
    searchByPriority(root->left, p);
    if (root->task.priority == p)
        cout << root->task.title << " on " << root->task.date << endl;
    searchByPriority(root->right, p);
}

    bool dateExists(AVLNode* root, string d) {
    if (!root) return false;
    if (root->task.date == d) return true;
    return dateExists(root->left, d) || dateExists(root->right, d);
}

    void dailyView(AVLNode* root, string d) {
    if (!root) return;
    dailyView(root->left, d);
    if (root->task.date == d)
        cout << root->task.title << " at "
             << root->task.hour << ":"
             << (root->task.minute < 10 ? "0" : "")
             << root->task.minute << endl;
    dailyView(root->right, d);
}


/* ================= STACK (UNDO / REDO) ================= */
struct StackNode {
    Task task;
    string action;
    StackNode* next;
};

StackNode* undoStack = NULL;
StackNode* redoStack = NULL;

void push(StackNode*& top, Task t, string act) {
    StackNode* n = new StackNode();
    n->task = t;
    n->action = act;
    n->next = top;
    top = n;
}

bool pop(StackNode*& top, Task& t, string& act) {
    if (!top) return false;
    t = top->task;
    act = top->action;
    StackNode* temp = top;
    top = top->next;
    delete temp;
    return true;
}

void performUndo(AVLNode*& root) {
    Task t;
    string act;

    if (!pop(undoStack, t, act)) {
        cout << "Nothing to undo\n";
        return;
    }

    bool dummy = false;

    if (act == "ADD") {
        root = deleteAVL(root, t, dummy);
        push(redoStack, t, "ADD");
        cout << "Undo successful (Add reverted)\n";
    }
    else if (act == "DELETE") {
        root = insertAVL(root, t);
        push(redoStack, t, "DELETE");
        cout << "Undo successful (Delete reverted)\n";
    }
}

void performRedo(AVLNode*& root) {
    Task t;
    string act;

    if (!pop(redoStack, t, act)) {
        cout << "Nothing to redo\n";
        return;
    }

    bool dummy = false;

    if (act == "ADD") {
        root = insertAVL(root, t);
        push(undoStack, t, "ADD");
        cout << "Redo successful (Add reapplied)\n";
    }
    else if (act == "DELETE") {
        root = deleteAVL(root, t, dummy);
        push(undoStack, t, "DELETE");
        cout << "Redo successful (Delete reapplied)\n";
    }
}
/* ================= QUEUE (REMINDERS) ================= */
struct QueueNode {
    Task task;
    QueueNode* next;
};

QueueNode* frontQ = NULL;
QueueNode* rearQ = NULL;

void enqueue(Task t) {
    QueueNode* n = new QueueNode();
    n->task = t;
    n->next = NULL;
    if (!rearQ) frontQ = rearQ = n;
    else {
        rearQ->next = n;
        rearQ = n;
    }
}

void showReminders() {
    QueueNode* temp = frontQ;
    if (!temp) {
        cout << "No reminders available\n";
        return;
    }
    while (temp) {
        cout << temp->task.title << " on " << temp->task.date << endl;
        temp = temp->next;
    }
}

/* ================= FILE SAVE ================= */
void saveTasks(AVLNode* root, ofstream& out) {
    if (!root) return;
    saveTasks(root->left, out);
    out << root->task.id << ","
        << root->task.title << ","
        << root->task.date << ","
        << root->task.hour << ","
        << root->task.minute << ","
        << root->task.duration << ","
        << root->task.priority
        << endl;
    saveTasks(root->right, out);
}




void loadTasks(AVLNode*& root) {
    ifstream in("tasks.txt");
    if (!in) return;  
    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        Task t;
        string token;

        getline(ss, token, ','); t.id = stoi(token);
        getline(ss, t.title, ',');
        getline(ss, t.date, ',');
        getline(ss, token, ','); t.hour = stoi(token);
        getline(ss, token, ','); t.minute = stoi(token);
        getline(ss, token, ','); t.duration = stoi(token);
        getline(ss, t.priority);

        root = insertAVL(root, t);
    }
    in.close();
}
// Clear all tasks in AVL tree
void clearTasks(AVLNode*& root) {
    if (!root) return;
    clearTasks(root->left);
    clearTasks(root->right);
    delete root;
    root = NULL;
}


void clearStack(StackNode*& top) {
    StackNode* temp;
    while (top) {
        temp = top;
        top = top->next;
        delete temp;
    }
}




/* ================= MAIN ================= */
int main() {
    
    AVLNode* root = NULL;
    int choice, id = 1;

    loadTasks(root);

   
    do {
    cout << "\n========== SMART TASK SCHEDULER ==========\n";
    cout << "1. Add Task\n";
    cout << "2. View All Tasks\n";
    cout << "3. View Daily Tasks\n";
    cout << "4. Search Task By Date\n";
    cout << "5. Search Task By Priority\n";
    cout << "6. Undo\n";
    cout << "7. Redo\n";
    cout << "8. View Reminders\n";
    cout << "9. Save Tasks\n";
    cout << "10. Load Tasks\n";  
    cout << "11. Clear All Tasks & History\n";  
    cout << "12. Delete Task\n";
    cout << "0. Exit\n";
    cout << "Enter choice: ";

    cin >> choice;

    if (choice == 1) {
    Task t;
    string error;

    cout << "Title: ";
    cin.ignore();
    getline(cin, t.title);

    cout << "Enter Date (YYYY-MM-DD): ";
    cin >> t.date;
    if (!isValidDate(t.date, error)) {
        cout << "Date Error: " << error << endl;
        continue;
    }

    cout << "Enter Time (HH MM): ";
    cin >> t.hour >> t.minute;
    if (!isValidTime(t.hour, t.minute, error)) {
        cout << "Time Error: " << error << endl;
        continue;
    }

    cout << "Enter Duration (minutes): ";
    cin >> t.duration;

    if (!isValidDuration(t.duration, error)) {
        cout << "Duration Error: " << error << endl;
        continue;
    }

    cout << "Enter Priority (High / Medium / Low): ";
    cin >> t.priority;

    if (!isValidPriority(t.priority, error)) {
        cout << "Priority Error: " << error << endl;
        continue;
    }


    t.id = id++;

    
    if (hasConflict(root, t)) {
        cout << "Error: Task conflicts with existing task!\n";
        continue;
    }

    root = insertAVL(root, t);
    enqueue(t);
    push(undoStack, t, "ADD");
    redoStack = NULL;

    cout << "Task added successfully\n";
}


      else if (choice == 2) {
    cout << "All tasks:\n";
    viewTasks(root); 
}


        else if (choice == 3) {
        string d, error;
        cout << "Enter date (YYYY-MM-DD): ";
        cin >> d;

        if (!isValidDate(d, error)) {
            cout << "Date Error: " << error << endl;
        } else {
            cout << "Tasks on " << d << ":\n";
            dailyView(root, d);
        }
    }
        else if (choice == 4) {
        string d, error;
        cout << "Enter date (YYYY-MM-DD): ";
        cin >> d;
        if (!isValidDate(d, error)) {
            cout << "Date Error: " << error << endl;
            continue;
        }
        AVLNode* tempRoot = NULL;
        loadTasks(tempRoot); 
        cout << "Tasks on " << d << ":\n";
        searchByDate(tempRoot, d);
    }



        else if (choice == 5) {
        string p;
        cout << "Enter priority (High/Medium/Low): ";
        cin >> p;
        searchByPriority(root, p);
    }

        else if (choice == 6) {
            performUndo(root);
        }
        else if (choice == 7) {
            performRedo(root);
        }

        else if (choice == 8) showReminders();
       
            else if (choice == 9) {
        ofstream out("tasks.txt");
        saveTasks(root, out);
        out.close();
        cout << "Tasks saved successfully\n";
    }



        else if (choice == 10) {
            loadTasks(root);
            cout << "Tasks loaded successfully\n";
        }
        else if (choice == 11) {
        clearTasks(root);         
        clearStack(undoStack);     
        clearStack(redoStack);     
        ofstream out("tasks.txt", ios::trunc); 
        out.close();
        cout << "All tasks and history cleared successfully!\n";
    }
    else if (choice == 12) {
    Task t;
    string error;

    cout << "Enter date of task to delete (YYYY-MM-DD): ";
    cin >> t.date;

    if (!isValidDate(t.date, error)) {
        cout << "Date Error: " << error << endl;
        continue;
    }

    cout << "Enter time of task to delete (HH MM): ";
    cin >> t.hour >> t.minute;

    if (!isValidTime(t.hour, t.minute, error)) {
        cout << "Time Error: " << error << endl;
        continue;
    }

    if (!findTask(root, t)) {
        cout << "Error: Task not found\n";
        continue;
    }

    bool deleted = false;
    root = deleteAVL(root, t, deleted);

    if (deleted) {
        push(undoStack, t, "DELETE");
        redoStack = NULL;
        cout << "Task deleted successfully\n";
    }

}

    } while (choice != 0);  


    return 0;
}