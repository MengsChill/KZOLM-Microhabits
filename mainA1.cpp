#include <iostream>
#include <string>
#include <iomanip>
#include <cctype>    // toupper
using namespace std;

// =====================================================
// COLOR CODES (ANSI). Safe to ignore if terminal no-ANSI
// =====================================================
#define RED    "\033[1;31m"
#define GREEN  "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE   "\033[1;34m"
#define CYAN   "\033[1;36m"
#define RESET  "\033[0m"

// =====================================================
// CONSTANTS
// =====================================================
const int MAX_MOVIES      = 3;
const int ROWS            = 5;     // A..E
const int COLS            = 5;     // 1..5
const int MAX_TICKETS     = 10;    // per booking
const int MAX_ACCOUNTS    = 20;
const int MAX_BOOKINGS    = 200;   // total stored bookings per run
const double TAX_RATE     = 0.06;
const double SERVICE_FEE  = 1.50;  // per ticket

// Showtimes fixed (4)
const int SHOWTIMES = 4;

// Ticket types
enum TicketType { ADULT = 0, STUDENT = 1, CHILD = 2, SENIOR = 3, TTYPE_COUNT = 4 };

// Prices by ticket type (Adult base = RM 20)
const double TICKET_PRICES[TTYPE_COUNT] = {
    20.00, // Adult
    16.00, // Student
    12.00, // Child
    14.00  // Senior
};

// =====================================================
// GLOBALS (kept minimal and simple for this scope)
// =====================================================
string movies[MAX_MOVIES] = {
    "Mission Impossible: Laundry Day",
    "Kung Fu Goldfish",
    "The Mushroom Head"
};

// All movies use the same base concept; we display Adult base RM 20.00 to match your brief
double movieBasePriceDisplay = 20.00;

// Seats per movie per showtime: 'O' open, 'X' booked
char seats[MAX_MOVIES][SHOWTIMES][ROWS][COLS];

// Simple account store
string usernames[MAX_ACCOUNTS];
string passwords[MAX_ACCOUNTS];
int accountCount = 1; // admin exists

// Daily totals per movie (tickets sold)
int dailyTicketsPerMovie[MAX_MOVIES] = {0};

// Daily totals by ticket type (array to satisfy category summary requirement)
int dailyTicketsByType[TTYPE_COUNT] = {0};

// Showtimes (fixed labels)
const char* showtimeLabels[SHOWTIMES] = { "12:00 PM", "3:30 PM", "7:00 PM", "10:30 PM" };

// Booking record
struct Booking {
    bool    active;                 // if cancelled -> false
    string  username;               // who booked
    int     movieIndex;             // 0..MAX_MOVIES-1
    int     showtimeIndex;          // 0..SHOWTIMES-1
    TicketType ttype;               // ticket type chosen
    int     qty;                    // 1..MAX_TICKETS
    char    seatRows[MAX_TICKETS];  // 'A'..'E'
    int     seatCols[MAX_TICKETS];  // 1..5
    double  subtotal;
    double  serviceFee;
    double  tax;
    double  total;
};
Booking bookings[MAX_BOOKINGS];
int bookingCount = 0;

// =====================================================
// UTILS
// =====================================================
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
void displayRegisterTitle() {
    cout << BLUE;
    cout << "\t\t\t\t\t\t################################################################################\n";
    cout << "\t\t\t\t\t\t#                                                                              #\n";
    cout << "\t\t\t\t\t\t#   RRRRRR   EEEEEEE   GGGGGGG   IIIII   SSSSSSS   TTTTTTT   EEEEEEE  RRRRRR   #\n";
    cout << "\t\t\t\t\t\t#   R    R   E         G           I     S            T      E        R    R   #\n";
    cout << "\t\t\t\t\t\t#   RRRRRR   EEEEE     G   GGG     I     SSSSSSS      T      EEEEE    RRRRRR   #\n";
    cout << "\t\t\t\t\t\t#   R  R     E         G     G     I           S      T      E        R   R    #\n";
    cout << "\t\t\t\t\t\t#   R   R    EEEEEEE    GGGGGG   IIIII   SSSSSSS      T      EEEEEEE  R     R  #\n";
    cout << "\t\t\t\t\t\t#                                                                              #\n";
    cout << "\t\t\t\t\t\t#                     PPPPPP     AAAAA     GGGGGGG    EEEEEEE                  #\n";
    cout << "\t\t\t\t\t\t#                     P    P    A     A    G          E                        #\n";
    cout << "\t\t\t\t\t\t#                     PPPPPP    AAAAAAA    G   GGG    EEEEE                    #\n";
    cout << "\t\t\t\t\t\t#                     P         A     A    G     G    E                        #\n";
    cout << "\t\t\t\t\t\t#                     P         A     A     GGGGGG    EEEEEEE                  #\n";
    cout << "\t\t\t\t\t\t#                                                                              #\n";
    cout << "\t\t\t\t\t\t################################################################################\n";
    cout << RESET;
}
// Input safe integer in [minV, maxV] with validation (Data Validation #1)
int readIntInRange(const string& prompt, int minV, int maxV) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= minV && v <= maxV) {
            return v;
        }
        cout << RED << "Invalid input. Enter a number between " << minV << " and " << maxV << ".\n" << RESET;
        cin.clear();
        cin.ignore(1000, '\n');
    }
}

// Read seat like 'A1' -> returns rowChar, col (Data Validation #2: seat format & bounds)
bool parseSeat(const string& token, char& rowCharOut, int& colOut) {
    if (token.size() < 2 || token.size() > 3) return false;
    char r = (char)toupper(token[0]);
    if (r < 'A' || r >= 'A' + ROWS) return false;

    // parse numeric part
    int col = 0;
    for (size_t i = 1; i < token.size(); ++i) {
        if (!isdigit((unsigned char)token[i])) return false;
        col = col * 10 + (token[i] - '0');
    }
    if (col < 1 || col > COLS) return false;

    rowCharOut = r;
    colOut = col;
    return true;
}

// Show intro
void showIntro() {
    cout << YELLOW
         << "\t\t\t\t\t\t\t\t=============================================\n"
         << "\t\t\t\t\t\t\t\t      WELCOME TO GALAXY CINEMA BOOKING\n"
         << "\t\t\t\t\t\t\t\t=============================================\n" << RESET;
}

// Initialize seat maps
void initSeats() {
    for (int m = 0; m < MAX_MOVIES; ++m)
        for (int s = 0; s < SHOWTIMES; ++s)
            for (int r = 0; r < ROWS; ++r)
                for (int c = 0; c < COLS; ++c)
                    seats[m][s][r][c] = 'O';
}

// Display movies & base price
void showMovies() {
    cout << CYAN << "\nAvailable Movies (Adult base RM " << fixed << setprecision(2) << movieBasePriceDisplay << "):\n";
    cout << "------------------------------------------------\n" << RESET;
    for (int i = 0; i < MAX_MOVIES; i++) {
        cout << " [" << (i + 1) << "] " << movies[i] << '\n';
    }
    cout << "------------------------------------------------\n";
}

// Choose movie
int chooseMovie() {
    return readIntInRange("Select Movie ID (1-3): ", 1, MAX_MOVIES) - 1;
}

// Display showtimes
void showShowtimes() {
    cout << CYAN << "Choose Showtime:\n" << RESET;
    for (int i = 0; i < SHOWTIMES; ++i) {
        cout << "  [" << (i + 1) << "] " << showtimeLabels[i] << '\n';
    }
}

// Choose showtime
int chooseShowtime() {
    return readIntInRange("Select Showtime (1-4): ", 1, SHOWTIMES) - 1;
}

// Show ticker types & prices
void showTicketTypes() {
    cout << CYAN << "Choose Ticket Type:\n" << RESET;
    cout << "  [1] Adult   (RM "   << fixed << setprecision(2) << TICKET_PRICES[ADULT]   << ")\n";
    cout << "  [2] Student (RM "   << TICKET_PRICES[STUDENT] << ")\n";
    cout << "  [3] Child   (RM "   << TICKET_PRICES[CHILD]   << ")\n";
    cout << "  [4] Senior  (RM "   << TICKET_PRICES[SENIOR]  << ")\n";
}

// Choose ticket type
TicketType chooseTicketType() {
    int t = readIntInRange("Ticket Type (1-4): ", 1, 4);
    return static_cast<TicketType>(t - 1);
}

// Display seats map for a movie+showtime
void showSeats(int movieIndex, int showtimeIndex) {
    cout << "\nSeat Layout for " << movies[movieIndex] << " (" << showtimeLabels[showtimeIndex] << ")\n";
    cout << "    ";
    for (int c = 0; c < COLS; ++c) cout << setw(3) << (c + 1);
    cout << '\n';
    for (int r = 0; r < ROWS; ++r) {
        cout << " " << char('A' + r) << "  ";
        for (int c = 0; c < COLS; ++c) {
            cout << setw(3) << seats[movieIndex][showtimeIndex][r][c];
        }
        cout << '\n';
    }
    cout << "('O' = available, 'X' = booked)\n\n";
}

// Try book a single seat cell
bool bookSeatCell(int movieIndex, int showtimeIndex, char rowChar, int col) {
    int r = rowChar - 'A';
    int c = col - 1;
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return false;
    if (seats[movieIndex][showtimeIndex][r][c] == 'X') return false;
    seats[movieIndex][showtimeIndex][r][c] = 'X';
    return true;
}

// Calculate price breakdown
void calcPrice(TicketType ttype, int qty, double& subtotal, double& serviceFee, double& tax, double& total) {
    subtotal   = TICKET_PRICES[ttype] * qty;
    serviceFee = SERVICE_FEE * qty;
    tax        = (subtotal + serviceFee) * TAX_RATE;
    total      = subtotal + serviceFee + tax;
}

// Print receipt
void printReceipt(const Booking& b) {
    cout << YELLOW
         << "===========================================\n"
         << "               BOOKING RECEIPT\n"
         << "===========================================\n" << RESET;

    cout << "Movie: " << movies[b.movieIndex] << '\n';
    cout << "Showtime: " << showtimeLabels[b.showtimeIndex] << '\n';
    cout << "Seats: ";
    for (int i = 0; i < b.qty; ++i) {
        cout << b.seatRows[i] << b.seatCols[i];
        if (i != b.qty - 1) cout << ", ";
    }
    cout << '\n';

    const char* tname = (b.ttype == ADULT ? "Adult" : b.ttype == STUDENT ? "Student" : b.ttype == CHILD ? "Child" : "Senior");
    cout << "Tickets: " << b.qty << " x " << tname << '\n';
    cout << fixed << setprecision(2);
    cout << "Subtotal: RM " << b.subtotal << '\n';
    cout << "Service Fee: RM " << b.serviceFee << '\n';
    cout << "Tax (6%): RM " << b.tax << '\n';
    cout << "-------------------------------------------\n";
    cout << GREEN << "TOTAL: RM " << b.total << RESET << '\n';
    cout << YELLOW
         << "===========================================\n"
         << "  Thank you for your purchase! Enjoy the show\n"
         << "===========================================\n" << RESET;
}

// =====================================================
// LOGIN / REGISTER
// =====================================================
bool loginSystem(string& loggedInUser, bool& isAdmin) {
    // default admin
    usernames[0] = "admin";
    passwords[0] = "1234";

    while (true) {
        cout << BLUE
             << "\t\t\t\t\t\t\t\t\t============================\n"
             << "\t\t\t\t\t\t\t\t\t        LOGIN SYSTEM        \n"
             << "\t\t\t\t\t\t\t\t\t============================\n" << RESET;
        cout << "\t\t\t\t\t\t\t\t\t1. Login\n";
        cout << "\t\t\t\t\t\t\t\t\t2. Register\n";
        cout << "\t\t\t\t\t\t\t\t\t3. Exit\n";
        int choice = readIntInRange("\t\t\t\t\t\t\t\t\tPlease Select An Option: ", 1, 3);
clearScreen();


        if (choice == 1) {
            string u, p;
            cout << "Username: ";
            cin >> u;
            cout << "Password: ";
            cin >> p;
            for (int i = 0; i < accountCount; ++i) {
                if (usernames[i] == u && passwords[i] == p) {
                    cout << GREEN << "Login successful! Welcome, " << u << ".\n\n" << RESET;
                    loggedInUser = u;
                    isAdmin = (u == "admin");
                    return true;
                }
            }
            cout << RED << "Invalid username or password.\n\n" << RESET;
        } else if (choice == 2) {
            if (accountCount >= MAX_ACCOUNTS) {
                cout << RED << "Account limit reached.\n" << RESET;
                continue;
            }

            string u, p;
            cout << "\t\t\t\t\t\t\tEnter new username: ";
            cin >> u;
            // Data Validation #3: ensure unique username
            bool exists = false;
            for (int i = 0; i < accountCount; ++i) {
                if (usernames[i] == u) { exists = true; break; }
            }
            if (exists) {
                cout << RED << "Username already exists. Try again.\n" << RESET;
                continue;
            }
            cout << "\t\t\t\t\t\t\tEnter new password: ";
            cin >> p;
            usernames[accountCount] = u;
            passwords[accountCount] = p;
            accountCount++;
            cout << GREEN << "Account registered successfully!\n\n" << RESET;
        } else {
            cout << "Exiting system. Goodbye!\n";
            return false;
        }
    }
}

// =====================================================
// SNACKS, POINTS, LUCKY DRAW (Extras)
// =====================================================
void orderSnacks() {
    cout << "\n--- Snacks & Combos ---\n";
    cout << "1. Popcorn Combo - 50 pts\n";
    cout << "2. Nachos & Coke - 70 pts\n";
    cout << "3. Family Snack Box - 100 pts\n";
    int snack = readIntInRange("Choose snack combo (1-3): ", 1, 3);
    (void)snack;
    cout << GREEN << "Order placed! Enjoy your snacks.\n\n" << RESET;
}

void redeemPoints(int& points) {
    if (points >= 100) {
        points -= 100;
        cout << GREEN << "You redeemed 100 points for a free popcorn! Remaining points: " << points << "\n\n" << RESET;
    } else {
        cout << RED << "Not enough points to redeem.\n\n" << RESET;
    }
}

void luckyDraw(int& points) {
    cout << "\n--- Lucky Draw ---\n";
    cout << "Pick a number between 1 and 5: ";
    int number;
    if (!(cin >> number)) { // tidy input
        cin.clear();
        cin.ignore(1000, '\n');
        cout << RED << "Invalid input.\n" << RESET;
        return;
    }
    if (number == 3) {
        cout << GREEN << "Congratulations! You won 50 bonus points!\n" << RESET;
        points += 50;
    } else {
        cout << YELLOW << "Better luck next time!\n" << RESET;
    }
    cout << "Current points: " << points << "\n\n";
}

// =====================================================
// BOOKING HELPERS
// =====================================================
void addDailyTotals(int movieIndex, TicketType ttype, int qty) {
    dailyTicketsPerMovie[movieIndex] += qty;
    dailyTicketsByType[ttype] += qty;
}

// Create a booking (returns index or -1)
int createBooking(const string& username, int movieIndex, int showtimeIndex, TicketType ttype,
                  int qty, char seatRows[], int seatCols[],
                  double subtotal, double serviceFee, double tax, double total) {
    if (bookingCount >= MAX_BOOKINGS) return -1;
    Booking& b = bookings[bookingCount];
    b.active = true;
    b.username = username;
    b.movieIndex = movieIndex;
    b.showtimeIndex = showtimeIndex;
    b.ttype = ttype;
    b.qty = qty;
    for (int i = 0; i < qty; ++i) {
        b.seatRows[i] = seatRows[i];
        b.seatCols[i] = seatCols[i];
    }
    b.subtotal = subtotal;
    b.serviceFee = serviceFee;
    b.tax = tax;
    b.total = total;
    return bookingCount++;
}

// Cancel a booking: frees seats & updates daily totals inversely
bool cancelBookingById(const string& username, int bookingId) {
    if (bookingId < 0 || bookingId >= bookingCount) return false;
    Booking& b = bookings[bookingId];
    if (!b.active || b.username != username) return false;

    // Free seats
    for (int i = 0; i < b.qty; ++i) {
        int r = b.seatRows[i] - 'A';
        int c = b.seatCols[i] - 1;
        seats[b.movieIndex][b.showtimeIndex][r][c] = 'O';
    }
    // Deduct daily totals
    dailyTicketsPerMovie[b.movieIndex] -= b.qty;
    dailyTicketsByType[b.ttype] -= b.qty;

    b.active = false;
    return true;
}

// Print user’s booking history
void viewBookingHistory(const string& username) {
    cout << CYAN << "========== BOOKING HISTORY ==========\n" << RESET;
    bool any = false;
    cout << fixed << setprecision(2);
    for (int i = 0; i < bookingCount; ++i) {
        const Booking& b = bookings[i];
        if (b.username == username) {
            any = true;
            cout << (b.active ? GREEN : RED);
            cout << "#ID " << i << "  " << (b.active ? "[ACTIVE]" : "[CANCELLED]") << RESET << '\n';
            cout << " Movie: " << movies[b.movieIndex] << " | Showtime: " << showtimeLabels[b.showtimeIndex] << '\n';
            cout << " Seats: ";
            for (int j = 0; j < b.qty; ++j) {
                cout << b.seatRows[j] << b.seatCols[j];
                if (j != b.qty - 1) cout << ", ";
            }
            const char* tname = (b.ttype == ADULT ? "Adult" : b.ttype == STUDENT ? "Student" : b.ttype == CHILD ? "Child" : "Senior");
            cout << "\n Tickets: " << b.qty << " x " << tname;
            cout << "\n Total Paid: RM " << b.total << "\n";
            cout << "-------------------------------------\n";
        }
    }
    if (!any) cout << YELLOW << "No bookings found.\n" << RESET;
}

// =====================================================
// USER FLOW: BOOK TICKETS (with nested loops)
// =====================================================
void bookTicketsFlow(const string& username) {
    bool moreOrders = true;
    while (moreOrders) { // Nested loop to handle multiple orders in one session
        showMovies();
        int movieIndex = chooseMovie();
        clearScreen();

        showShowtimes();
        int showtimeIndex = chooseShowtime();
        clearScreen();

        showSeats(movieIndex, showtimeIndex);

        showTicketTypes();
        TicketType ttype = chooseTicketType();

        int qty = readIntInRange("Enter number of tickets (1-10): ", 1, MAX_TICKETS);

        // Seat picking loop with validation & collision check
        char chosenRows[MAX_TICKETS];
        int  chosenCols[MAX_TICKETS];

        for (int i = 0; i < qty; ++i) {
            while (true) {
                cout << "Select seat " << (i + 1) << " (e.g., A1): ";
                string token;
                cin >> token;

                char r; int c;
                if (!parseSeat(token, r, c)) {
                    cout << RED << "Invalid seat format or out of range. Use A1..E5.\n" << RESET;
                    continue;
                }
                // Prevent duplicate selection in same booking
                bool dup = false;
                for (int k = 0; k < i; ++k) {
                    if (chosenRows[k] == r && chosenCols[k] == c) { dup = true; break; }
                }
                if (dup) {
                    cout << RED << "You already selected that seat in this order.\n" << RESET;
                    continue;
                }
                // Check availability
                int rr = r - 'A';
                int cc = c - 1;
                if (seats[movieIndex][showtimeIndex][rr][cc] == 'X') {
                    cout << RED << "Seat already booked. Choose another.\n" << RESET;
                    continue;
                }
                chosenRows[i] = r;
                chosenCols[i] = c;
                break;
            }
        }

        // Commit seats
        for (int i = 0; i < qty; ++i) {
            bookSeatCell(movieIndex, showtimeIndex, chosenRows[i], chosenCols[i]); // safe now
        }

        // Price math
        double subtotal, svc, tax, total;
        calcPrice(ttype, qty, subtotal, svc, tax, total);

        // Create and print receipt
        int bid = createBooking(username, movieIndex, showtimeIndex, ttype, qty,
                                chosenRows, chosenCols, subtotal, svc, tax, total);

        addDailyTotals(movieIndex, ttype, qty);

        if (bid >= 0) {
            printReceipt(bookings[bid]);
        } else {
            cout << RED << "System capacity reached; booking not stored (seats were reserved). Contact admin.\n" << RESET;
        }

        // Earn points = floor(total) for demo
        cout << CYAN << "You earned approx. " << static_cast<int>(total) << " pts for this purchase.\n" << RESET;

        // Another order?
        int again = readIntInRange("Make another booking? (1=Yes, 2=No): ", 1, 2);
        moreOrders = (again == 1);
        clearScreen();
    }
}

// =====================================================
// ADMIN: SIMPLE REPORT
// =====================================================
void adminReport() {
    cout << YELLOW << "=========== SALES REPORT (DAILY) ===========\n" << RESET;
    cout << "Tickets sold by Movie:\n";
    for (int i = 0; i < MAX_MOVIES; ++i) {
        cout << "  - " << setw(30) << left << movies[i]
             << "Tickets: " << dailyTicketsPerMovie[i] << '\n';
    }
    cout << "\nTickets sold by Category:\n";
    const char* names[TTYPE_COUNT] = {"Adult", "Student", "Child", "Senior"};
    for (int t = 0; t < TTYPE_COUNT; ++t) {
        cout << "  - " << setw(8) << left << names[t]
             << ": " << dailyTicketsByType[t] << '\n';
    }
    cout << "============================================\n";
}

// =====================================================
// DASHBOARDS
// =====================================================
void userDashboard(const string& username) {
    int points = 120;
    string membership = "Silver";

    int choice;
    do {
        cout << YELLOW
             << "===========================================\n"
             << "              USER DASHBOARD\n"
             << "===========================================\n" << RESET;
        cout << "Hello, " << username << "!\n";
        cout << "Membership Level: " << membership << " (" << points << " pts)\n";
        cout << "Points can be redeemed for discounts or free popcorn!\n\n";

        cout << "1. View Movies & Showtimes\n";
        cout << "2. Book Tickets\n";
        cout << "3. View Booking History\n";
        cout << "4. Cancel Booking\n";
        cout << "5. Order Snacks & Combos \n";
        cout << "6. Redeem Membership Points\n";
        cout << "7. Lucky Draw \n";
        cout << "8. Logout\n";
        cout << "-------------------------------------------\n";

        choice = readIntInRange("Enter your choice: ", 1, 8);
        clearScreen();

        switch (choice) {
            case 1: {
                showMovies();
                cout << "Showtimes:\n";
                for (int i = 0; i < SHOWTIMES; ++i) {
                    cout << "  [" << (i + 1) << "] " << showtimeLabels[i] << '\n';
                }
                cout << '\n';
                break;
            }
            case 2:
                bookTicketsFlow(username);
                break;
            case 3:
                viewBookingHistory(username);
                break;
            case 4: {
                viewBookingHistory(username);
                if (bookingCount == 0) break;
                cout << "Enter booking ID to cancel: ";
                int bid;
                if (!(cin >> bid)) {
                    cin.clear(); cin.ignore(1000, '\n');
                    cout << RED << "Invalid input.\n" << RESET;
                    break;
                }
                if (cancelBookingById(username, bid)) {
                    cout << GREEN << "Booking cancelled and seats released.\n" << RESET;
                } else {
                    cout << RED << "Unable to cancel. Check ID or status.\n" << RESET;
                }
                break;
            }
            case 5:
                orderSnacks();
                break;
            case 6:
                redeemPoints(points);
                break;
            case 7:
                luckyDraw(points);
                break;
            case 8:
                cout << GREEN << "Logging out... Goodbye, " << username << "!\n" << RESET;
                break;
        }
    } while (choice != 8);

    // End-of-day summary (for the session)
    cout << "\n=== DAILY TICKET SUMMARY (Session) ===\n";
    for (int i = 0; i < MAX_MOVIES; ++i) {
        cout << setw(30) << left << movies[i]
             << " Tickets Sold: " << dailyTicketsPerMovie[i] << '\n';
    }
    cout << "======================================\n";
}

// =====================================================
// EXTRA ADMIN FUNCTIONS (ADDED ONLY, NOT CHANGING EXISTING CODE)
// =====================================================

// Admin can add a new movie
void addMovie() {
    if (MAX_MOVIES <= 0) {
        cout << RED << "Movie capacity is full. Cannot add more movies.\n" << RESET;
        return;
    }

    cout << CYAN << "Enter new movie name: " << RESET;
    cin.ignore();
    string newMovie;
    getline(cin, newMovie);

    // Simple check if movie already exists
    for (int i = 0; i < MAX_MOVIES; ++i) {
        if (movies[i] == newMovie) {
            cout << RED << "Movie already exists!\n" << RESET;
            return;
        }
    }

    // Replace the first empty slot
    for (int i = 0; i < MAX_MOVIES; ++i) {
        if (movies[i] == "") {
            movies[i] = newMovie;
            cout << GREEN << "Movie \"" << newMovie << "\" added successfully!\n" << RESET;
            return;
        }
    }
    cout << RED << "No empty slot to add the movie.\n" << RESET;
}

// Admin can remove a movie
void removeMovie() {
    showMovies();
    int movieIndex = readIntInRange("Enter the movie number to remove: ", 1, MAX_MOVIES) - 1;
    cout << RED << "Are you sure you want to remove \"" << movies[movieIndex] << "\"? (1=Yes, 2=No): " << RESET;
    int confirm;
    cin >> confirm;
    if (confirm == 1) {
        cout << GREEN << "Movie \"" << movies[movieIndex] << "\" removed successfully!\n" << RESET;
        movies[movieIndex] = ""; // clear slot
        for (int s = 0; s < SHOWTIMES; ++s) {
            for (int r = 0; r < ROWS; ++r) {
                for (int c = 0; c < COLS; ++c) {
                    seats[movieIndex][s][r][c] = 'O'; // reset seats
                }
            }
        }
    } else {
        cout << YELLOW << "Removal cancelled.\n" << RESET;
    }
}

// Admin view all user accounts
void viewAllUsers() {
    cout << CYAN << "===== ALL REGISTERED USERS =====\n" << RESET;
    for (int i = 0; i < accountCount; ++i) {
        cout << i + 1 << ". Username: " << usernames[i];
        if (i == 0) cout << " (ADMIN)";
        cout << '\n';
    }
    cout << "================================\n";
}

// Admin reset a user's password
void resetUserPassword() {
    viewAllUsers();
    int index = readIntInRange("Enter user number to reset password: ", 1, accountCount) - 1;
    cout << CYAN << "Enter new password for " << usernames[index] << ": " << RESET;
    string newPass;
    cin >> newPass;
    passwords[index] = newPass;
    cout << GREEN << "Password reset successfully!\n" << RESET;
}

// =====================================================
// ENHANCED ADMIN DASHBOARD
// =====================================================
void adminDashboard() {
    int choice;
    do {
        cout << YELLOW
             << "===========================================\n"
             << "             ADMIN DASHBOARD\n"
             << "===========================================\n" << RESET;
        cout << "1. View All Movies\n";
        cout << "2. Add Movie\n";
        cout << "3. Remove Movie\n";
        cout << "4. View Sales Report\n";
        cout << "5. View All Users\n";
        cout << "6. Reset User Password\n";
        cout << "7. Access User Dashboard (simulate customer)\n";
        cout << "8. Logout\n";
        cout << "-------------------------------------------\n";
        choice = readIntInRange("Enter your choice: ", 1, 8);
        clearScreen();

        switch (choice) {
            case 1:
                showMovies();
                cout << "Showtimes:\n";
                for (int i = 0; i < SHOWTIMES; ++i) {
                    cout << "  [" << (i + 1) << "] " << showtimeLabels[i] << '\n';
                }
                cout << '\n';
                break;
            case 2:
                addMovie();
                break;
            case 3:
                removeMovie();
                break;
            case 4:
                adminReport();
                break;
            case 5:
                viewAllUsers();
                break;
            case 6:
                resetUserPassword();
                break;
            case 7: {
                cout << CYAN << "Simulating user dashboard as admin...\n" << RESET;
                userDashboard("admin");
                break;
            }
            case 8:
                cout << GREEN << "Admin logging out.\n" << RESET;
                break;
        }
    } while (choice != 8);
}




// =====================================================
// MAIN
// =====================================================
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Initialize system
    usernames[0] = "admin";
    passwords[0] = "1234";
    accountCount = 1;

    showIntro();
    initSeats();
    displayRegisterTitle();

    string loggedInUser;
    bool isAdmin = false;

    if (!loginSystem(loggedInUser, isAdmin)) {
        return 0; // Exit if user chooses to quit
    }

    cout << "DEBUG: isAdmin = " << isAdmin << ", loggedInUser = " << loggedInUser << "\n";
    cout << "Press Enter to continue...";
    cin.ignore();
    cin.get();
    clearScreen();

    // Go to correct dashboard
    if (isAdmin) {
        adminDashboard();
    } else {
        userDashboard(loggedInUser);
    }

    cout << "\n" << CYAN << "Program ended. See you again!\n" << RESET;
    return 0;
}


