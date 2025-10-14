#include <iostream>
#include <string>
#include <iomanip>
#include <cctype>  // For toupper
using namespace std;

// =====================================================
// COLOR CODES (ANSI) - Optional Styling
// =====================================================
#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define CYAN    "\033[1;36m"

// =====================================================
// GLOBAL VARIABLES
// =====================================================
string usernames[10];
string passwords[10];
string roles[10]; // Store "User" or "Admin"
int accountCount = 0;

string sessionRole = ""; // Remember whether current person is User or Admin

// =====================================================
// FUNCTION PROTOTYPES
// =====================================================
void adminDashboard();
void userDashboard(const string &loggedInUser);
void registerUser();
bool loginSystem(string &loggedInUser, bool &isAdmin);
void selectRoleAtStart();

// =====================================================
// ADMIN DASHBOARD
// =====================================================
void adminDashboard() {
    int c;
    do {
        cout << CYAN << "\n===========================================\n";
        cout << "             ADMIN DASHBOARD\n";
        cout << "===========================================\n" << RESET;
        cout << "1. View All Movies\n";
        cout << "2. Add Movie / Showtime\n";
        cout << "3. Remove Movie\n";
        cout << "4. View Sales Report\n";
        cout << "5. Logout\n";
        cout << "-------------------------------------------\n";
        cout << "Enter choice: ";
        cin >> c;

        switch (c) {
            case 1: cout << "Showing all movies (demo)...\n"; break;
            case 2: cout << "Add movie feature (demo)...\n"; break;
            case 3: cout << "Remove movie feature (demo)...\n"; break;
            case 4: cout << "Sales report (demo)...\n"; break;
            case 5: cout << GREEN << "Logging out...\n" << RESET; break;
            default: cout << RED << "Invalid choice. Try again.\n" << RESET;
        }
    } while (c != 5);
}

// =====================================================
// USER DASHBOARD
// =====================================================
void userDashboard(const string &loggedInUser) {
    int c;
    do {
        cout << CYAN << "\n===========================================\n";
        cout << "              USER DASHBOARD\n";
        cout << "===========================================\n" << RESET;
        cout << "1. View Movies & Showtimes\n";
        cout << "2. Book Tickets\n";
        cout << "3. View Booking History\n";
        cout << "4. Cancel Booking\n";
        cout << "5. Logout\n";
        cout << "-------------------------------------------\n";
        cout << "Enter choice: ";
        cin >> c;

        switch (c) {
            case 1: cout << "Movies & showtimes (demo)...\n"; break;
            case 2: cout << "Booking tickets (demo)...\n"; break;
            case 3: cout << "Viewing booking history (demo)...\n"; break;
            case 4: cout << "Cancel booking (demo)...\n"; break;
            case 5: cout << GREEN << "Logging out...\n" << RESET; break;
            default: cout << RED << "Invalid choice. Try again.\n" << RESET;
        }
    } while (c != 5);
}

// =====================================================
// REGISTER USER
// =====================================================
void registerUser() {
    if (accountCount >= 10) {
        cout << RED << "Account limit reached! Cannot register more users.\n" << RESET;
        return;
    }

    string newUsername, newPassword;

    cout << CYAN << "\n========== REGISTER PAGE ==========\n" << RESET;

    // Enter username
    cout << "Enter username: ";
    cin >> newUsername;

    // Check for duplicate username
    for (int i = 0; i < accountCount; i++) {
        if (usernames[i] == newUsername) {
            cout << RED << "Username already exists! Please try another.\n" << RESET;
            return;
        }
    }

    // Enter password
    cout << "Enter password: ";
    cin >> newPassword;

    // Save account data
    usernames[accountCount] = newUsername;
    passwords[accountCount] = newPassword;
    roles[accountCount] = sessionRole; // Role determined at start
    accountCount++;

    cout << GREEN << "\nRegistration successful! Account created as "
         << sessionRole << ".\n" << RESET;
}

// =====================================================
// SELECT ROLE AT PROGRAM START
// =====================================================
void selectRoleAtStart() {
    int roleChoice;

    cout << CYAN << "\n========== WHO ARE YOU? ==========\n" << RESET;
    cout << "1. I am a User\n";
    cout << "2. I am an Admin\n";
    cout << "Choice: ";
    cin >> roleChoice;

    if (roleChoice == 2) {
        sessionRole = "Admin";
    } else {
        sessionRole = "User";
    }

    cout << GREEN << "\nYou are now using the system as: " << sessionRole << "\n" << RESET;
}

// =====================================================
// LOGIN FUNCTION
// =====================================================
bool loginSystem(string &loggedInUser, bool &isAdmin) {
    string u, p;

    cout << CYAN << "\n========== LOGIN ==========\n" << RESET;
    cout << "Enter username: ";
    cin >> u;
    cout << "Enter password: ";
    cin >> p;

    for (int i = 0; i < accountCount; i++) {
        if (usernames[i] == u && passwords[i] == p) {
            loggedInUser = u;
            isAdmin = (roles[i] == "Admin"); // Check stored role

            cout << GREEN << "\nLogin successful! Welcome, " << u << ".\n" << RESET;
            cout << "Role: " << roles[i] << "\n";
            return true;
        }
    }

    cout << RED << "Invalid username or password.\n" << RESET;
    return false;
}

// =====================================================
// MAIN FUNCTION
// =====================================================
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Create a default Admin account
    usernames[0] = "admin";
    passwords[0] = "1234";
    roles[0] = "Admin";
    accountCount = 1;

    string loggedInUser;
    bool isAdmin = false;
    int choice;

    // Step 1: Ask role BEFORE main menu
    selectRoleAtStart();

    // Step 2: Main Menu
    do {
        cout << YELLOW << "\n========== CINEMA SYSTEM ==========\n" << RESET;
        cout << "1. Register\n";
        cout << "2. Login\n";
        cout << "3. Quit\n";
        cout << "Choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                registerUser(); // Role already set at program start
                break;
            case 2:
                if (loginSystem(loggedInUser, isAdmin)) {
                    if (isAdmin)
                        adminDashboard();
                    else
                        userDashboard(loggedInUser);
                }
                break;
            case 3:
                cout << CYAN << "Goodbye!\n" << RESET;
                break;
            default:
                cout << RED << "Invalid choice. Please try again.\n" << RESET;
        }
    } while (choice != 3);

    return 0;
}
