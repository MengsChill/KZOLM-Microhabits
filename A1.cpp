#include <iostream>
#include <string>
#include <iomanip>
using namespace std;

// =============================
// COLOR CODES (MAC & LINUX)
// =============================
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define CYAN "\033[1;36m"
#define RESET "\033[0m"

// =============================
// CONSTANTS
// =============================
const int MAX_MOVIES = 3;
const int ROWS = 5;
const int COLS = 5;
const int MAX_TICKETS = 10;
const double TAX_RATE = 0.06;
const double SERVICE_FEE = 1.50;

// =============================
// GLOBAL VARIABLES
// =============================
string movies[MAX_MOVIES] = {
    "Mission Impossible: Laundry Day",
    "Kung Fu Goldfish",
    "The Mushroom Head"
};
double prices[MAX_MOVIES] = {18.00, 20.00, 15.00};
int dailyTickets[MAX_MOVIES] = {0};
char seats[MAX_MOVIES][ROWS][COLS]; // 'O' = Open, 'X' = Booked

// Login data
string usernames[10], passwords[10];
int accountCount = 1; // start with default account

// =============================
// CLEAR SCREEN
// =============================
void clearScreen() {
    system("clear"); // works for Mac & Linux
}
// =============================
// INITIALIZE SEATS
// =============================
void initSeats() {
    for (int m = 0; m < MAX_MOVIES; m++) {
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                seats[m][r][c] = 'O';
            }
        }
    }
}

// =============================
// INTRO
// =============================
void showIntro() {
    cout << YELLOW << "=============================================\n";
    cout << "          MOVIE BOOKING SYSTEM\n";
    cout << "=============================================\n" << RESET;
}

// =============================
// DISPLAY MOVIES
// =============================
void showMovies() {
    cout << CYAN << "\nAvailable Movies:\n";
    cout << "------------------------------------------------\n" << RESET;
    for (int i = 0; i < MAX_MOVIES; i++) {
        cout << " [" << (i + 1) << "] "
             << setw(30) << left << movies[i]
             << " RM " << fixed << setprecision(2) << prices[i] << endl;
    }
    cout << "------------------------------------------------\n";
}

// =============================
// CHOOSE MOVIE
// =============================
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

// =============================
// SHOW SEATS
// =============================
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

// =============================
// BOOK SEAT
// =============================
bool bookSeat(int movieIndex, char rowChar, int col) {
    int row = rowChar - 'A';
    col -= 1;

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

// =============================
// CHOOSE TICKETS
// =============================
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

// =============================
// CALCULATE TOTAL PRICE
// =============================
double calculateTotal(int movieIndex, int qty) {
    double subtotal = prices[movieIndex] * qty;
    double service = SERVICE_FEE * qty;
    double tax = (subtotal + service) * TAX_RATE;
    return subtotal + service + tax;
}

// =============================
// ORDER SUMMARY
// =============================
void orderSummary(int movieIndex, int qty, double total) {
    cout << "\n========== ORDER SUMMARY ==========\n";
    cout << "Movie: " << movies[movieIndex] << endl;
    cout << "Tickets: " << qty << endl;
    cout << "Price per Ticket: RM " << fixed << setprecision(2) << prices[movieIndex] << endl;
    cout << "-----------------------------------\n";
    cout << GREEN << "TOTAL (incl. fees & tax): RM " << total << RESET << endl;
    cout << "===================================\n\n";
}

// =============================
// LOGIN SYSTEM
// =============================
bool loginSystem(string &loggedInUser) {
    usernames[0] = "admin";
    passwords[0] = "1234";

    int choice;
    string username, password;

do {
    cout << BLUE << "============================\n";
    cout << "        LOGIN SYSTEM        \n";
    cout << "============================\n" << RESET;
    cout << "1. Login\n";
    cout << "2. Register\n";
    cout << "3. Exit\n";
    cout << "Choice: ";
    cin >> choice;
    if (choice == 1) {
        cout << "Username: ";
        cin >> username;
            cout << "Password: ";
            cin >> password;

            for (int i = 0; i < accountCount; i++) {
                if (usernames[i] == username && passwords[i] == password) {
                    cout << GREEN << "Login successful! Welcome, " << username << ".\n\n" << RESET;
                    loggedInUser = username;
                    return true;
                }
            }
            cout << RED << "Invalid username or password.\n\n" << RESET;
        } else if (choice == 2) {
            if (accountCount >= 10) {
                cout << RED << "Account limit reached.\n" << RESET;
                continue;
            }
            cout << "Enter new username: ";
            cin >> usernames[accountCount];
            cout << "Enter new password: ";
            cin >> passwords[accountCount];
            accountCount++;
            cout << GREEN << "Account registered successfully!\n\n" << RESET;
        } else if (choice == 3) {
            cout << "Exiting system. Goodbye!\n";
            return false;
        } else {
            cout << RED << "Invalid choice.\n" << RESET;
        }
    } while (true);
}

// =============================
// SNACKS MENU
// =============================
void orderSnacks() {
    int snack;
    cout << "\n--- Snacks & Combos ---\n";
    cout << "1. Popcorn Combo - 50 pts\n";
    cout << "2. Nachos & Coke - 70 pts\n";
    cout << "3. Family Snack Box - 100 pts\n";
    cout << "Choose snack combo: ";
    cin >> snack;
    cout << GREEN << "Order placed! Enjoy your snacks.\n\n" << RESET;
}

// =============================
// REDEEM POINTS
// =============================
void redeemPoints(int &points) {
    if (points >= 100) {
        points -= 100;
        cout << GREEN << "You redeemed 100 points for a free popcorn! Remaining points: " << points << "\n\n" << RESET;
    } else {
        cout << RED << "Not enough points to redeem.\n\n" << RESET;
    }
}

// =============================
// LUCKY DRAW
// =============================
void luckyDraw(int &points) {
    int number;
    cout << "\n--- Lucky Draw  ---\n";
    cout << "Pick a number between 1 and 5: ";
    cin >> number;
    if (number == 3) {
        cout << GREEN << "Congratulations! You won 50 bonus points!\n" << RESET;
        points += 50;
    } else {
        cout << RED << "Better luck next time!\n" << RESET;
    }
    cout << "Current points: " << points << "\n\n";
}

// =============================
// USER DASHBOARD
// =============================
void userDashboard(string username) {
    int points = 120;
    string membership = "Silver";
    int choice;

    do {
        cout << YELLOW << "===========================================\n";
        cout << "           USER DASHBOARD\n";
        cout << "===========================================\n" << RESET;
        cout << "Hello, " << username << "!\n";
        cout << "Membership Level: " << membership << " (" << points << " pts)\n";
        cout << "Points can be redeemed for discounts or free popcorn!\n\n";

        cout << "1. View Movies & Showtimes\n";
        cout << "2. Book Tickets\n";
        cout << "3. Order Snacks & Combos \n";
        cout << "4. Redeem Membership Points\n";
        cout << "5. Lucky Draw \n";
        cout << "6. Logout\n";
        cout << "-------------------------------------------\n";
        cout << "Enter your choice: ";
        cin >> choice;

        clearScreen();

        switch(choice) {
            case 1:
                showMovies();
                break;
            case 2: {
                showMovies();
                int movieIndex = chooseMovie();
                clearScreen();
                showSeats(movieIndex);
                int qty = chooseTickets();

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
                dailyTickets[movieIndex] += qty;
                break;
            }
            case 3:
                orderSnacks();
                break;
            case 4:
                redeemPoints(points);
                break;
            case 5:
                luckyDraw(points);
                break;
            case 6:
                cout << GREEN << "Logging out... Goodbye, " << username << "!\n" << RESET;
                break;
            default:
                cout << RED << "Invalid choice. Please try again.\n\n" << RESET;
        }

    } while(choice != 6);

    // End-of-day summary
    cout << "\n=== DAILY TICKET SUMMARY ===\n";
    for (int i = 0; i < MAX_MOVIES; i++) {
        cout << setw(30) << left << movies[i]
             << " Tickets Sold: " << dailyTickets[i] << endl;
    }
    cout << "=============================\n";
}

// =============================
// MAIN FUNCTION
// =======================
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
    cout << "#                     PPPPPP     AAAAA     GGGGGGG    EEEEEEE                  #\n";
    cout << "#                     P    P    A     A    G          E                        #\n";
    cout << "#                     PPPPPP    AAAAAAA    G   GGG    EEEEE                    #\n";
    cout << "#                     P         A     A    G     G    E                        #\n";
    cout << "#                     P         A     A     GGGGGG    EEEEEEE                  #\n";
    cout << "#                                                                              #\n";
    cout << "################################################################################\n";
    cout << RESET << endl;
}

int main() {
    showIntro();
    initSeats();
    displayRegisterTitle();

    string loggedInUser;
    if (loginSystem(loggedInUser)) {
        clearScreen();
        userDashboard(loggedInUser);
    }

    return 0;
}
