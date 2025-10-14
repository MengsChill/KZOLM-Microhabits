#include <iostream>
#include <string>
#include <iomanip>
using namespace std;

// ===== Color Codes =====
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define CYAN "\033[1;36m"
#define RESET "\033[0m"

// ===== Constants =====
const int MAX_MOVIES = 3;
const int ROWS = 5;
const int COLS = 5;
const int MAX_TICKETS = 10;
const double TAX_RATE = 0.06;
const double SERVICE_FEE = 1.50;

// ===== Movie Data =====
string movies[MAX_MOVIES] = { "Mission Impossible: Laundry Day", "Kung Fu Goldfish", "The Mushroom Head" };
double prices[MAX_MOVIES] = { 18.00, 20.00, 15.00 };
int dailyTickets[MAX_MOVIES] = { 0 };

// Seats: 'O' = open, 'X' = booked
char seats[MAX_MOVIES][ROWS][COLS];

// ===== Utility: Clear Screen =====
void clearScreen() {
    system("clear"); // Mac/Linux
}

// ===== Register Page ASCII Art =====
void displayRegisterTitle() {
    cout << BLUE;
    cout << "################################################################################\n";
    cout << "#                                                                              #\n";
    cout << "#   RRRRRR   EEEEEEE   GGGGGGG   IIIII   SSSSSSS   TTTTTTT   EEEEEEE  RRRRRR   #\n";
    cout << "#   R    R   E         G           I     S            T      E        R    R   #\n";
    cout << "#   RRRRRR   EEEEE     G   GGG     I     SSSSSSS      T      EEEEE    RRRRRR   #\n";
    cout << "#   R  R     E         G     G     I           S      T      E        R   R    #\n";
    cout << "#   R   R    EEEEEEE    GGGGGG   IIIII   SSSSSSS      T      EEEEEEE  R     R  #\n";
    cout << "#                                                                              #\n";
    cout << "#                     PPPPPP     AAAAA     GGGGGGG    EEEEEEE                   #\n";
    cout << "#                     P    P    A     A    G          E                        #\n";
    cout << "#                     PPPPPP    AAAAAAA    G   GGG    EEEEE                    #\n";
    cout << "#                     P         A     A    G     G    E                        #\n";
    cout << "#                     P         A     A     GGGGGG    EEEEEEE                  #\n";
    cout << "#                                                                              #\n";
    cout << "################################################################################\n";
    cout << RESET;
}

// ===== Seats Initialization =====
void initSeats() {
    for (int m = 0; m < MAX_MOVIES; m++)
        for (int r = 0; r < ROWS; r++)
            for (int c = 0; c < COLS; c++)
                seats[m][r][c] = 'O';
}

// ===== Show Movies =====
void showMovies() {
    cout << "\nAvailable Movies:\n";
    cout << "------------------------------------------------\n";
    for (int i = 0; i < MAX_MOVIES; i++) {
        cout << " [" << (i + 1) << "] "
             << setw(30) << left << movies[i]
             << " RM " << fixed << setprecision(2) << prices[i] << endl;
    }
    cout << "------------------------------------------------\n";
}

// ===== Choose Movie =====
int chooseMovie() {
    int choice;
    do {
        cout << "Choose a movie (1-" << MAX_MOVIES << "): ";
        cin >> choice;
        if (cin.fail() || choice < 1 || choice > MAX_MOVIES) {
            cout << RED << "Invalid movie choice. Try again.\n" << RESET;
            cin.clear();
            cin.ignore(1000, '\n');
            choice = -1;
        }
    } while (choice < 1 || choice > MAX_MOVIES);
    return choice - 1;
}

// ===== Show Seats =====
void showSeats(int movieIndex) {
    cout << "\nSeat Layout for " << movies[movieIndex] << ":\n";
    cout << "   ";
    for (int c = 0; c < COLS; c++) cout << setw(3) << (c + 1);
    cout << endl;
    for (int r = 0; r < ROWS; r++) {
        cout << " " << char('A' + r) << " ";
        for (int c = 0; c < COLS; c++) {
            cout << setw(3) << seats[movieIndex][r][c];
        }
        cout << endl;
    }
    cout << "('O' = available, 'X' = booked)\n\n";
}

// ===== Book a Seat =====
bool bookSeat(int movieIndex, char rowChar, int col) {
    int row = rowChar - 'A';
    col = col - 1;
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) {
        cout << RED << "Invalid seat selection.\n" << RESET;
        return false;
    }
    if (seats[movieIndex][row][col] == 'X') {
        cout << RED << "Seat already booked.\n" << RESET;
        return false;
    }
    seats[movieIndex][row][col] = 'X';
    return true;
}

// ===== Ticket Quantity =====
int chooseTickets() {
    int qty;
    do {
        cout << "Enter number of tickets (1-" << MAX_TICKETS << "): ";
        cin >> qty;
        if (cin.fail() || qty < 1 || qty > MAX_TICKETS) {
            cout << RED << "Invalid ticket quantity.\n" << RESET;
            cin.clear();
            cin.ignore(1000, '\n');
            qty = -1;
        }
    } while (qty < 1 || qty > MAX_TICKETS);
    return qty;
}

// ===== Calculate Total =====
double calculateTotal(int movieIndex, int qty) {
    double subtotal = prices[movieIndex] * qty;
    double service = SERVICE_FEE * qty;
    double tax = (subtotal + service) * TAX_RATE;
    return subtotal + service + tax;
}

// ===== Order Summary =====
void orderSummary(int movieIndex, int qty, double total) {
    cout << "\n========== ORDER SUMMARY ==========\n";
    cout << "Movie: " << movies[movieIndex] << endl;
    cout << "Tickets: " << qty << endl;
    cout << "Price per Ticket: RM " << fixed << setprecision(2) << prices[movieIndex] << endl;
    cout << "-----------------------------------\n";
    cout << "TOTAL (incl. fees & tax): RM " << total << endl;
    cout << "===================================\n\n";
}

// ===== Booking Flow =====
void bookingSystem() {
    initSeats();
    char again = 'Y';

    while (toupper(again) == 'Y') {
        showMovies();
        int movieIndex = chooseMovie();
        showSeats(movieIndex);

        int qty = chooseTickets();

        // Seat Selection
        for (int i = 0; i < qty; i++) {
            bool booked = false;
            while (!booked) {
                char row; int col;
                cout << "Select seat " << (i + 1) << " (e.g., A1): ";
                cin >> row >> col;
                row = toupper(row);
                booked = bookSeat(movieIndex, row, col);
            }
        }

        double total = calculateTotal(movieIndex, qty);
        orderSummary(movieIndex, qty, total);

        // Update daily report summary
        dailyTickets[movieIndex] += qty;

        cout << "Do you want to make another booking? (Y/N): ";
        cin >> again;
        cout << endl;
    }

    // End-of-day summary
    cout << "\n=== DAILY TICKET SUMMARY ===\n";
    for (int i = 0; i < MAX_MOVIES; i++) {
        cout << setw(30) << left << movies[i]
             << " Tickets Sold: " << dailyTickets[i] << endl;
    }
    cout << "=============================\n";
}

// ===== Login / Register =====
void loggedIn() {
    string usernames[10], passwords[10];
    int accountCount = 1; 
    usernames[0] = "admin";
    passwords[0] = "1234";

    string username;
    bool isLoggedIn = false;
    int choice;

    do {
        cout << BLUE << "===============================================\n";
        cout << "        MOVIE TICKET BOOKING SYSTEM            \n";
        cout << "===============================================\n" << RESET;
        cout << CYAN << "        Welcome to Galaxy Cinema!              \n";
        cout << "===============================================\n" << RESET;
        cout << "1. Login\n";
        cout << "2. Register New Account\n";
        cout << "3. Exit\n";
        cout << "-------------------------------------------\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << RED << "Invalid input! Please enter a number.\n\n" << RESET;
            continue;
        }

        clearScreen();

        if (choice == 1) {
            // Login
            string u, p;
            cout << "\n--- Login ---\n";
            cout << "Username: ";
            cin >> u;
            cout << "Password: ";
            cin >> p;

            bool found = false;
            for (int i = 0; i < accountCount; i++) {
                if (usernames[i] == u && passwords[i] == p) {
                    cout << GREEN << "Login successful! Welcome, " << u << "!\n\n" << RESET;
                    username = u;
                    isLoggedIn = true;
                    found = true;
                    break;
                }
            }
            if (!found) {
                cout << RED << "Login failed. Wrong username or password.\n\n" << RESET;
            }

        } else if (choice == 2) {
            // Register
            if (accountCount >= 10) {
                cout << RED << "Account limit reached.\n\n" << RESET;
                continue;
            }
            displayRegisterTitle();
            cout << "\nEnter new username: ";
            cin >> usernames[accountCount];
            cout << "Enter new password: ";
            cin >> passwords[accountCount];
            accountCount++;
            cout << GREEN << "Account registered successfully!\n\n" << RESET;

        } else if (choice == 3) {
            cout << "Thank you for using Cinema Booking. Goodbye!\n";
            return;
        } else {
            cout << RED << "Invalid choice. Please try again.\n\n" << RESET;
        }

    } while (!isLoggedIn);

    // ===== Start Booking After Login =====
    bookingSystem();
}

// ===== Main =====
int main() {
    loggedIn();
    return 0;
}
