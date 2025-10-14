
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cctype>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <algorithm>
#include <cmath>

using namespace std;

// -------------------- Config / Files -----------------
const string ACCOUNTS_FILE = "accounts.txt";   // username|password|points|spins|voucher
const string BOOKINGS_FILE = "bookings.txt";   // id|active|username|movie|showtime|ttype|qty|seatlist|subtotal|svc|tax|total|isSnack|note
const string SALES_FILE = "sales.txt";      // movie0,movie1,movie2|type0,type1,type2,type3

// -------------------- Console colors (optional) ------
#define RED    "\033[31m"
#define GREEN  "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE   "\033[1;34m"
#define CYAN   "\033[1;36m" 
#define RESET  "\033[0m"

// -------------------- Constants ----------------------
const int MAX_MOVIES = 3;
const int ROWS = 5;     // A..E
const int COLS = 5;     // 1..5
const int MAX_TICKETS = 10;
const int SHOWTIMES = 4;
const double TAX_RATE = 0.06;
const double SERVICE_FEE = 1.50;

enum TicketType { ADULT = 0, STUDENT = 1, CHILD = 2, SENIOR = 3, TTYPE_COUNT = 4 };

const double TICKET_PRICES[TTYPE_COUNT] = { 20.00, 16.00, 12.00, 14.00 };
const char* SHOWTIME_LABELS[SHOWTIMES] = { "12:00 PM", "3:30 PM", "7:00 PM", "10:30 PM" };

// -------------------- Snack catalog ------------------
struct SnackItem { int id; string name; double price; int pointsCost; };
vector<SnackItem> SNACK_CATALOG = {
    {1, "Small Popcorn", 5.00, 50},
    {2, "Popcorn Combo (Popcorn + Drink)", 12.00, 100},
    {3, "Nachos & Coke", 10.00, 80},
    {4, "Family Snack Box", 25.00, 200}
};

// -------------------- Data structs -------------------
struct Account {
    string username;
    string password;
    int points = 0;
    int pendingSpins = 0;
    double voucherBalance = 0.0;
};

struct Booking {
    int id = -1;
    bool active = true;
    string username;
    int movieIndex = -1;
    int showtimeIndex = -1;
    TicketType ttype = ADULT;
    int qty = 0;
    vector<char> seatRows;
    vector<int>  seatCols;
    double subtotal = 0.0;
    double serviceFee = 0.0;
    double tax = 0.0;
    double total = 0.0;
    bool isSnackOnly = false;
    string note;
};

// -------------------- Global state -------------------
string movies[MAX_MOVIES] = {
    "Mission Impossible: Laundry Day",
    "Kung Fu Goldfish",
    "The Mushroom Head"
};
double movieBasePriceDisplay = 20.00;
char seats[MAX_MOVIES][SHOWTIMES][ROWS][COLS];
vector<Account> accounts;
vector<Booking> bookings;
int dailyTicketsPerMovie[MAX_MOVIES] = { 0 };
int dailyTicketsByType[TTYPE_COUNT] = { 0 };
int nextBookingId = 1;

// -------------------- Utilities ----------------------
void pause_screen() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int readIntInRange(const string& prompt, int minV, int maxV) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= minV && v <= maxV) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return v;
        }
        cout << RED << "Invalid input. Enter a number between " << minV << " and " << maxV << ".\n" << RESET;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

bool parseSeat(const string& token, char& rowCharOut, int& colOut) {
    if (token.size() < 2 || token.size() > 3) return false;
    char r = (char)toupper(static_cast<unsigned char>(token[0]));
    if (r < 'A' || r >= 'A' + ROWS) return false;
    int col = 0;
    for (size_t i = 1; i < token.size(); ++i) {
        if (!isdigit(static_cast<unsigned char>(token[i]))) return false;
        col = col * 10 + (token[i] - '0');
    }
    if (col < 1 || col > COLS) return false;
    rowCharOut = r; colOut = col; return true;
}

void initSeats() {
    for (int m = 0; m < MAX_MOVIES; ++m)
        for (int s = 0; s < SHOWTIMES; ++s)
            for (int r = 0; r < ROWS; ++r)
                for (int c = 0; c < COLS; ++c)
                    seats[m][s][r][c] = 'O';
}

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
    cout << RESET;
}
void showIntro() {
    cout << YELLOW << "=============================================\n"
        << "       WELCOME TO GALAXY CINEMA BOOKING\n"
        << "=============================================\n" << RESET;
}

void showMovies() {
    cout << CYAN << "\nAvailable Movies (Adult base RM " << fixed << setprecision(2) << movieBasePriceDisplay << "):\n";
    cout << "------------------------------------------------\n" << RESET;
    for (int i = 0; i < MAX_MOVIES; ++i) cout << " [" << (i + 1) << "] " << movies[i] << "\n";
    cout << "------------------------------------------------\n";
}

int chooseMovie() { return readIntInRange("Select Movie ID (1-3): ", 1, MAX_MOVIES) - 1; }
void showShowtimes() { for (int i = 0;i < SHOWTIMES;++i) cout << "  [" << (i + 1) << "] " << SHOWTIME_LABELS[i] << "\n"; }
int chooseShowtime() { return readIntInRange("Select Showtime (1-4): ", 1, SHOWTIMES) - 1; }

void showTicketTypes() {
    cout << CYAN << "Choose Ticket Type:\n" << RESET;
    cout << "  [1] Adult   (RM " << fixed << setprecision(2) << TICKET_PRICES[ADULT] << ")\n";
    cout << "  [2] Student (RM " << TICKET_PRICES[STUDENT] << ")\n";
    cout << "  [3] Child   (RM " << TICKET_PRICES[CHILD] << ")\n";
    cout << "  [4] Senior  (RM " << TICKET_PRICES[SENIOR] << ")\n";
}
TicketType chooseTicketType() { int t = readIntInRange("Ticket Type (1-4): ", 1, 4); return static_cast<TicketType>(t - 1); }

void showSeats(int movieIndex, int showtimeIndex) {
    cout << "\nSeat Layout for " << movies[movieIndex] << " (" << SHOWTIME_LABELS[showtimeIndex] << ")\n";
    cout << "    "; for (int c = 0;c < COLS;++c) cout << setw(3) << (c + 1); cout << '\n';
    for (int r = 0;r < ROWS;++r) {
        cout << " " << char('A' + r) << "  ";
        for (int c = 0;c < COLS;++c) cout << setw(3) << seats[movieIndex][showtimeIndex][r][c];
        cout << '\n';
    }
    cout << "('O' = available, 'X' = booked)\n\n";
}

bool bookSeatCell(int movieIndex, int showtimeIndex, char rowChar, int col) {
    int r = rowChar - 'A'; int c = col - 1;
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return false;
    if (seats[movieIndex][showtimeIndex][r][c] == 'X') return false;
    seats[movieIndex][showtimeIndex][r][c] = 'X'; return true;
}

void calcTicketPrice(TicketType ttype, int qty, double& subtotal, double& svc, double& tax, double& total) {
    subtotal = TICKET_PRICES[ttype] * qty;
    svc = SERVICE_FEE * qty;
    tax = (subtotal + svc) * TAX_RATE;
    total = subtotal + svc + tax;
}

void calcSnackPrice(double pricePerItem, int qty, double& subtotal, double& svc, double& tax, double& total) {
    subtotal = pricePerItem * qty;
    svc = 0.0;
    tax = subtotal * TAX_RATE;
    total = subtotal + svc + tax;
}

void printReceipt(const Booking& b) {
    cout << YELLOW << "===========================================\n"
        << "               BOOKING RECEIPT\n"
        << "===========================================\n" << RESET;
    if (b.isSnackOnly) {
        cout << "Purchase: " << b.note << "\n";
    }
    else {
        cout << "Movie: " << ((b.movieIndex >= 0 && b.movieIndex < MAX_MOVIES) ? movies[b.movieIndex] : "Unknown") << "\n";
        cout << "Showtime: " << ((b.showtimeIndex >= 0 && b.showtimeIndex < SHOWTIMES) ? SHOWTIME_LABELS[b.showtimeIndex] : "-") << "\n";
        cout << "Seats: ";
        for (size_t i = 0;i < b.seatRows.size();++i) { cout << b.seatRows[i] << b.seatCols[i]; if (i + 1 < b.seatRows.size()) cout << ", "; }
        cout << "\n";
        const char* tname = (b.ttype == ADULT ? "Adult" : b.ttype == STUDENT ? "Student" : b.ttype == CHILD ? "Child" : "Senior");
        cout << "Tickets: " << b.qty << " x " << tname << "\n";
    }

    cout << fixed << setprecision(2);
    cout << "Subtotal: RM " << b.subtotal << "\n";
    if (b.serviceFee > 0.0) cout << "Service Fee: RM " << b.serviceFee << "\n";
    cout << "Tax (6%): RM " << b.tax << "\n";
    cout << "-------------------------------------------\n";
    cout << GREEN << "TOTAL: RM " << b.total << RESET << "\n";
    if (!b.note.empty()) cout << "Note: " << b.note << "\n";
    cout << YELLOW << "===========================================\n"
        << "  Thank you for your purchase! Enjoy the show\n"
        << "===========================================\n" << RESET;
}

// -------------------- File I/O -----------------------
void loadAccounts() {
    accounts.clear();
    ifstream fin(ACCOUNTS_FILE);
    if (!fin.is_open()) return;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string u, p, pts, spins, voucher;
        if (!getline(ss, u, '|')) continue;
        if (!getline(ss, p, '|')) continue;
        if (!getline(ss, pts, '|')) pts = "0";
        if (!getline(ss, spins, '|')) spins = "0";
        if (!getline(ss, voucher)) voucher = "0";
        Account a; a.username = u; a.password = p; a.points = stoi(pts); a.pendingSpins = stoi(spins); a.voucherBalance = stod(voucher);
        accounts.push_back(a);
    }
    fin.close();
}

void saveAccounts() {
    ofstream fout(ACCOUNTS_FILE);
    for (const auto& a : accounts) {
        fout << a.username << "|" << a.password << "|" << a.points << "|" << a.pendingSpins << "|" << fixed << setprecision(2) << a.voucherBalance << "\n";
    }
}

void loadBookings() {
    bookings.clear();
    ifstream fin(BOOKINGS_FILE);
    if (!fin.is_open()) return;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Booking b; string idStr, activeStr, user, movieStr, showStr, ttypeStr, qtyStr, seatList, subStr, svcStr, taxStr, totStr, isSnackStr, noteStr;
        if (!getline(ss, idStr, '|')) continue;
        if (!getline(ss, activeStr, '|')) continue;
        if (!getline(ss, user, '|')) continue;
        if (!getline(ss, movieStr, '|')) continue;
        if (!getline(ss, showStr, '|')) continue;
        if (!getline(ss, ttypeStr, '|')) continue;
        if (!getline(ss, qtyStr, '|')) continue;
        if (!getline(ss, seatList, '|')) continue;
        if (!getline(ss, subStr, '|')) continue;
        if (!getline(ss, svcStr, '|')) continue;
        if (!getline(ss, taxStr, '|')) continue;
        if (!getline(ss, totStr, '|')) continue;
        if (!getline(ss, isSnackStr, '|')) continue;
        if (!getline(ss, noteStr)) noteStr = "";
        b.id = stoi(idStr);
        b.active = (activeStr == "1");
        b.username = user;
        b.movieIndex = stoi(movieStr);
        b.showtimeIndex = stoi(showStr);
        b.ttype = static_cast<TicketType>(stoi(ttypeStr));
        b.qty = stoi(qtyStr);
        b.seatRows.clear(); b.seatCols.clear();
        if (!seatList.empty()) {
            stringstream s2(seatList);
            string token;
            while (getline(s2, token, ',')) {
                char r; int c; if (parseSeat(token, r, c)) { b.seatRows.push_back(r); b.seatCols.push_back(c); }
            }
        }
        b.subtotal = stod(subStr);
        b.serviceFee = stod(svcStr);
        b.tax = stod(taxStr);
        b.total = stod(totStr);
        b.isSnackOnly = (isSnackStr == "1");
        b.note = noteStr;
        bookings.push_back(b);
        if (b.id >= nextBookingId) nextBookingId = b.id + 1;
        if (b.active && b.movieIndex >= 0 && b.movieIndex < MAX_MOVIES) {
            dailyTicketsPerMovie[b.movieIndex] += b.qty;
            if (b.ttype >= 0 && b.ttype < TTYPE_COUNT) dailyTicketsByType[b.ttype] += b.qty;
            for (size_t i = 0;i < b.seatRows.size();++i) {
                int rr = b.seatRows[i] - 'A'; int cc = b.seatCols[i] - 1; if (rr >= 0 && rr < ROWS && cc >= 0 && cc < COLS) seats[b.movieIndex][b.showtimeIndex][rr][cc] = 'X';
            }
        }
    }
    fin.close();
}

void saveBookings() {
    ofstream fout(BOOKINGS_FILE);
    for (const auto& b : bookings) {
        string seatlist;
        for (size_t i = 0;i < b.seatRows.size();++i) { if (i) seatlist += ","; seatlist += b.seatRows[i]; seatlist += to_string(b.seatCols[i]); }
        fout << b.id << "|" << (b.active ? "1" : "0") << "|" << b.username << "|" << b.movieIndex << "|" << b.showtimeIndex << "|" << static_cast<int>(b.ttype) << "|" << b.qty << "|" << seatlist << "|"
            << fixed << setprecision(2) << b.subtotal << "|" << b.serviceFee << "|" << b.tax << "|" << b.total << "|" << (b.isSnackOnly ? "1" : "0") << "|" << b.note << "\n";
    }
}

void loadSales() {
    ifstream fin(SALES_FILE);
    if (!fin.is_open()) return;
    string line;
    if (!getline(fin, line)) return;
    stringstream ss(line);
    string moviePart, typePart;
    if (!getline(ss, moviePart, '|')) return;
    if (!getline(ss, typePart)) return;
    stringstream mss(moviePart); string token; int idx = 0;
    while (getline(mss, token, ',') && idx < MAX_MOVIES) { dailyTicketsPerMovie[idx++] = stoi(token); }
    stringstream tss(typePart); idx = 0;
    while (getline(tss, token, ',') && idx < TTYPE_COUNT) { dailyTicketsByType[idx++] = stoi(token); }
}

void saveSales() {
    ofstream fout(SALES_FILE);
    for (int i = 0;i < MAX_MOVIES;++i) { if (i) fout << ","; fout << dailyTicketsPerMovie[i]; }
    fout << "|";
    for (int t = 0;t < TTYPE_COUNT;++t) { if (t) fout << ","; fout << dailyTicketsByType[t]; }
    fout << "\n";
}

// -------------------- Auth/Admin ---------------------
Account* findAccount(const string& username) {
    for (auto& a : accounts) if (a.username == username) return &a;
    return nullptr;
}

bool loadAllFromFiles() {
    initSeats();
    loadAccounts();
    if (accounts.empty()) {
        Account a; a.username = "admin"; a.password = "1234"; a.points = 0; a.pendingSpins = 0; a.voucherBalance = 0.0; accounts.push_back(a);
    }
    loadBookings();
    loadSales();
    return true;
}

bool saveAllToFiles() {
    saveAccounts(); saveBookings(); saveSales(); return true;
}

bool loginSystem(string& loggedInUser, bool& isAdmin) {
    while (true) {
        cout << BLUE << "============================\n" << "        LOGIN SYSTEM        \n" << "============================\n" << RESET;
        cout << "1. Login\n2. Register\n3. Exit\n";
        int choice = readIntInRange("Please Select An Option: ", 1, 3);
        clearScreen();
        if (choice == 1) {
            string u, p; cout << "Username: "; getline(cin, u);
            cout << "Password: "; getline(cin, p);
            Account* ac = findAccount(u);
            if (ac && ac->password == p) { cout << GREEN << "Login successful! Welcome, " << u << ".\n" << RESET; loggedInUser = u; isAdmin = (u == "admin"); return true; }
            cout << RED << "Invalid username or password.\n" << RESET;
        }
        else if (choice == 2) {
            string u, p;
            cout << "Enter new username: "; getline(cin, u);
            if (u.empty()) { cout << RED << "Username cannot be empty.\n" << RESET; continue; }
            if (findAccount(u)) { cout << RED << "Username already exists.\n" << RESET; continue; }
            cout << "Enter new password: "; getline(cin, p);
            Account a; a.username = u; a.password = p; a.points = 0; a.pendingSpins = 0; a.voucherBalance = 0.0; accounts.push_back(a);
            saveAccounts();
            cout << GREEN << "Account registered successfully!\n" << RESET;
        }
        else {
            return false;
        }
    }
}

// Admin utilities
void adminViewUsers() {
    cout << CYAN << "--- Registered Users ---\n" << RESET;
    for (size_t i = 0;i < accounts.size();++i) {
        cout << i << ": " << accounts[i].username << " (pts: " << accounts[i].points << ", spins: " << accounts[i].pendingSpins << ", voucher: RM " << fixed << setprecision(2) << accounts[i].voucherBalance << ")\n";
    }
}

void adminEditUser() {
    adminViewUsers();
    int idx = readIntInRange("Enter user index to edit (or -1 to cancel): ", -1, (int)accounts.size() - 1);
    if (idx == -1) return;
    cout << "Editing user: " << accounts[idx].username << "\n";
    cout << "Enter new password (leave empty to keep): "; string np; getline(cin, np);
    if (!np.empty()) accounts[idx].password = np;
    cout << "Enter new points (or -1 to keep): ";
    int pts; if (cin >> pts) { if (pts >= 0) accounts[idx].points = pts; } cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter new spins (or -1 to keep): ";
    int sp; if (cin >> sp) { if (sp >= 0) accounts[idx].pendingSpins = sp; } cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter voucher amount (or -1 to keep): ";
    double v; if (cin >> v) { if (v >= 0) accounts[idx].voucherBalance = v; } cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
    saveAccounts();
    cout << GREEN << "User updated.\n" << RESET;
}

void adminDeleteUser() {
    adminViewUsers();
    int idx = readIntInRange("Enter user index to delete (or -1 to cancel): ", -1, (int)accounts.size() - 1);
    if (idx == -1) return;
    string uname = accounts[idx].username;
    if (uname == "admin") { cout << RED << "Cannot delete admin account.\n" << RESET; return; }
    accounts.erase(accounts.begin() + idx);
    for (auto& b : bookings) if (b.username == uname && b.active) {
        if (b.movieIndex >= 0 && b.movieIndex < MAX_MOVIES) {
            for (size_t i = 0;i < b.seatRows.size();++i) {
                int rr = b.seatRows[i] - 'A'; int cc = b.seatCols[i] - 1; if (rr >= 0 && rr < ROWS && cc >= 0 && cc < COLS) seats[b.movieIndex][b.showtimeIndex][rr][cc] = 'O';
            }
            dailyTicketsPerMovie[b.movieIndex] -= b.qty;
            dailyTicketsByType[b.ttype] -= b.qty;
        }
        b.active = false;
    }
    saveAllToFiles();
    cout << GREEN << "User deleted and their active bookings cancelled.\n" << RESET;
}

void adminResetSales() {
    for (int i = 0;i < MAX_MOVIES;++i) dailyTicketsPerMovie[i] = 0;
    for (int t = 0;t < TTYPE_COUNT;++t) dailyTicketsByType[t] = 0;
    saveSales();
    cout << GREEN << "Sales/reset summary cleared.\n" << RESET;
}

// -------------------- Booking helpers ----------------
int createBookingRecord(const string& username, int movieIndex, int showtimeIndex, TicketType ttype, int qty, const vector<char>& rows, const vector<int>& cols, double sub, double svc, double tax, double total, bool isSnackOnly = false, const string& note = "") {
    Booking b; b.id = nextBookingId++; b.active = true; b.username = username; b.movieIndex = movieIndex; b.showtimeIndex = showtimeIndex; b.ttype = ttype; b.qty = qty; b.seatRows = rows; b.seatCols = cols; b.subtotal = sub; b.serviceFee = svc; b.tax = tax; b.total = total; b.isSnackOnly = isSnackOnly; b.note = note;
    bookings.push_back(b);
    saveBookings();
    if (movieIndex >= 0 && movieIndex < MAX_MOVIES) {
        dailyTicketsPerMovie[movieIndex] += qty;
        if (ttype >= 0 && ttype < TTYPE_COUNT) dailyTicketsByType[ttype] += qty;
        saveSales();
    }
    Account* acc = findAccount(username);
    if (acc) {
        int earned = static_cast<int>(floor(total));
        if (earned > 0) acc->points += earned;
        if (total > 0.0) acc->pendingSpins += 1;
        saveAccounts();
    }
    return b.id;
}

bool cancelBookingById(const string& username, int bookingId) {
    for (auto& b : bookings) {
        if (b.id == bookingId && b.username == username && b.active) {
            if (b.movieIndex >= 0 && b.movieIndex < MAX_MOVIES) {
                for (size_t i = 0;i < b.seatRows.size();++i) {
                    int rr = b.seatRows[i] - 'A'; int cc = b.seatCols[i] - 1; if (rr >= 0 && rr < ROWS && cc >= 0 && cc < COLS) seats[b.movieIndex][b.showtimeIndex][rr][cc] = 'O';
                }
                dailyTicketsPerMovie[b.movieIndex] -= b.qty;
                dailyTicketsByType[b.ttype] -= b.qty;
            }
            b.active = false;
            saveBookings(); saveSales();
            return true;
        }
    }
    return false;
}

void viewBookingHistory(const string& username) {
    cout << CYAN << "========== BOOKING HISTORY ==========" << RESET << "\n";
    bool any = false;
    for (const auto& b : bookings) {
        if (b.username == username) {
            any = true;
            cout << (b.active ? GREEN : RED) << "#ID " << b.id << " " << (b.active ? "[ACTIVE]" : "[CANCELLED]") << RESET << "\n";
            if (b.isSnackOnly) {
                cout << " Purchase: " << b.note << "\n";
            }
            else {
                cout << " Movie: " << ((b.movieIndex >= 0 && b.movieIndex < MAX_MOVIES) ? movies[b.movieIndex] : "Unknown") << " | Showtime: " << ((b.showtimeIndex >= 0 && b.showtimeIndex < SHOWTIMES) ? SHOWTIME_LABELS[b.showtimeIndex] : "-") << "\n";
                cout << " Seats: "; for (size_t i = 0;i < b.seatRows.size();++i) { cout << b.seatRows[i] << b.seatCols[i]; if (i + 1 < b.seatRows.size()) cout << ", "; } cout << "\n";
                const char* tname = (b.ttype == ADULT ? "Adult" : b.ttype == STUDENT ? "Student" : b.ttype == CHILD ? "Child" : "Senior");
                cout << " Tickets: " << b.qty << " x " << tname << "\n";
            }
            cout << " Total Paid: RM " << fixed << setprecision(2) << b.total << "\n";
            if (!b.note.empty()) cout << " Note: " << b.note << "\n";
            cout << "-------------------------------------\n";
        }
    }
    if (!any) cout << YELLOW << "No bookings found." << RESET << "\n";
}

// -------------------- Snacks & Redeem ----------------
void orderSnacksPaid(const string& username) {
    cout << "\n--- Snacks & Combos (Paid) ---\n";
    for (auto& s : SNACK_CATALOG) cout << "[" << s.id << "] " << s.name << " - RM " << fixed << setprecision(2) << s.price << "\n";
    int sid = readIntInRange("Select snack ID (1-" + to_string((int)SNACK_CATALOG.size()) + "): ", 1, (int)SNACK_CATALOG.size());
    SnackItem chosen = SNACK_CATALOG[0];
    for (auto& s : SNACK_CATALOG) if (s.id == sid) chosen = s;
    int qty = readIntInRange("Enter quantity: ", 1, 20);
    double sub, svc, tax, total;
    calcSnackPrice(chosen.price, qty, sub, svc, tax, total);
    vector<char> rows; vector<int> cols;
    string note = chosen.name + " x" + to_string(qty);
    int bid = createBookingRecord(username, -1, -1, ADULT, 0, rows, cols, sub, svc, tax, total, true, note);
    cout << GREEN << "Snack purchase successful. Booking ID: " << bid << RESET << "\n";
    for (auto& b : bookings) if (b.id == bid) { printReceipt(b); break; }
}

void redeemSnackWithPoints(Account& acc) {
    cout << "\n--- Redeem Snacks with Points ---\n";
    cout << "You have " << acc.points << " points.\n";
    vector<SnackItem> redeemable;
    for (auto& s : SNACK_CATALOG) if (acc.points >= s.pointsCost) redeemable.push_back(s);
    if (redeemable.empty()) { 
        cout << RED << "No redeemable snacks available. Earn more points.\n" << RESET; return; 
    }
    for (auto& s : redeemable) cout << "[" << s.id << "] " << s.name << " - " << s.pointsCost << " pts\n";
    int sid = readIntInRange("Select snack ID to redeem: ", 1, (int)SNACK_CATALOG.size());
    SnackItem chosen; bool found = false;
    for (auto& s : SNACK_CATALOG) if (s.id == sid) { chosen = s; found = true; break; }
    if (!found || acc.points < chosen.pointsCost) { cout << RED << "Cannot redeem that item.\n" << RESET; return; }
    int qty = readIntInRange("Enter quantity to redeem: ", 1, 10);
    int totalCost = chosen.pointsCost * qty;
    if (acc.points < totalCost) { cout << RED << "Not enough points for that quantity.\n" << RESET; return; }
    acc.points -= totalCost;
    saveAccounts();
    vector<char> rows; vector<int> cols; double sub = 0, svc = 0, tax = 0, tot = 0;
    string note = "Redeemed: " + chosen.name + " x" + to_string(qty);
    int bid = createBookingRecord(acc.username, -1, -1, ADULT, 0, rows, cols, sub, svc, tax, tot, true, note);
    cout << GREEN << "Redeemed successfully. Booking ID: " << bid << RESET << "\n";
    for (auto& b : bookings) if (b.id == bid) { printReceipt(b); break; }
}

// -------------------- Lucky Draw --------------------
void luckyDraw(Account& acc) {
    if (acc.pendingSpins <= 0) { cout << RED << "No free spins available. Make a booking to earn a spin.\n" << RESET; return; }
    cout << "\n--- Lucky Draw ---\n";
    int pick = readIntInRange("Pick a number between 1 and 5: ", 1, 5);
    int drawn = (rand() % 5) + 1;
    cout << "Wheel spins... drawn number is: " << drawn << "\n";
    if (pick == drawn) {
        int prizeRoll = rand() % 100;
        if (prizeRoll < 10) {
            acc.voucherBalance += 20.0;
            cout << GREEN << "Congratulations! You won a RM20 voucher (applies to next booking).\n" << RESET;
        }
        else if (prizeRoll < 30) {
            acc.voucherBalance += 5.0;
            cout << GREEN << "You won a RM5 voucher!\n" << RESET;
        }
        else if (prizeRoll < 60) {
            acc.points += 50;
            cout << GREEN << "You won a free small popcorn (credited as 50 pts)!\n" << RESET;
        }
        else {
            acc.points += 20;
            cout << GREEN << "You won 20 bonus points!\n" << RESET;
        }
    }
    else {
        cout << YELLOW << "Better luck next time!\n" << RESET;
    }
    acc.pendingSpins -= 1;
    saveAccounts();
}

// -------------------- User flows ---------------------
void bookTicketsFlow(const string& username) {
    Account* acc = findAccount(username);
    bool moreOrders = true;
    while (moreOrders) {
        showMovies(); int movieIndex = chooseMovie(); clearScreen();
        showShowtimes(); int showtimeIndex = chooseShowtime(); clearScreen();
        showSeats(movieIndex, showtimeIndex);
        showTicketTypes(); TicketType ttype = chooseTicketType();
        int qty = readIntInRange("Enter number of tickets (1-10): ", 1, MAX_TICKETS);
        vector<char> chosenRows(qty); vector<int> chosenCols(qty);
        for (int i = 0;i < qty;++i) {
            while (true) {
                cout << "Select seat " << (i + 1) << " (e.g., A1): ";
                string token; getline(cin, token);
                if (token.empty()) { continue; }
                char r; int c; if (!parseSeat(token, r, c)) { cout << RED << "Invalid seat. Use A1..E5." << RESET << "\n"; continue; }
                bool dup = false; for (int k = 0;k < i;++k) if (chosenRows[k] == r && chosenCols[k] == c) { dup = true; break; }
                if (dup) { cout << RED << "Already selected.\n" << RESET; continue; }
                int rr = r - 'A', cc = c - 1; if (seats[movieIndex][showtimeIndex][rr][cc] == 'X') { cout << RED << "Seat already booked.\n" << RESET; continue; }
                chosenRows[i] = r; chosenCols[i] = c; break;
            }
        }
        for (int i = 0;i < qty;++i) bookSeatCell(movieIndex, showtimeIndex, chosenRows[i], chosenCols[i]);
        double sub, svc, tax, total; calcTicketPrice(ttype, qty, sub, svc, tax, total);
        double appliedVoucher = 0.0;
        if (acc && acc->voucherBalance > 0.0) {
            cout << CYAN << "You have RM " << fixed << setprecision(2) << acc->voucherBalance << " in vouchers. Apply to this booking? (1=Yes,2=No): " << RESET;
            int a = readIntInRange("Choice: ", 1, 2);
            if (a == 1) {
                appliedVoucher = min(acc->voucherBalance, total);
                acc->voucherBalance -= appliedVoucher;
                cout << GREEN << "Applied voucher RM " << fixed << setprecision(2) << appliedVoucher << RESET << "\n";
            }
        }
        double finalTotal = total - appliedVoucher;
        int bid = createBookingRecord(username, movieIndex, showtimeIndex, ttype, qty, chosenRows, chosenCols, sub, svc, tax, finalTotal, false, "");
        cout << GREEN << "Booking successful! ID: " << bid << RESET << "\n";
        for (auto& b : bookings) 
        if (b.id == bid) { 
            printReceipt(b); 
            break; 
        }
        if (acc) cout << CYAN << "Points balance: " << acc->points << ", pending spins: " << acc->pendingSpins << ", vouchers: RM " << fixed << setprecision(2) << acc->voucherBalance << RESET << "\n";
        int again = readIntInRange("Make another booking? (1=Yes,2=No): ", 1, 2); moreOrders = (again == 1); clearScreen();
    }
}

// -------------------- Dashboards ---------------------
void userDashboard(const string& username) {
    Account* acc = findAccount(username);
    if (!acc) { cout << RED << "Account not found.\n" << RESET; return; }
    int choice = 0;
    do {
        cout << YELLOW << "===========================================\n" << "              USER DASHBOARD\n" << "===========================================\n" << RESET;
        cout << "Hello, " << username << "! Points: " << acc->points << ", Spins: " << acc->pendingSpins << ", Vouchers: RM " << fixed << setprecision(2) << acc->voucherBalance << "\n";
        cout << "1. View Movies & Showtimes\n2. Book Tickets\n3. View Booking History\n4. Cancel Booking\n5. Order Snacks & Combos\n6. Redeem Membership Points\n7. Lucky Draw\n8. Logout\n";
        choice = readIntInRange("Enter your choice: ", 1, 8); clearScreen();
        switch (choice) {
        case 1: showMovies(); showShowtimes(); pause_screen(); break;
        case 2: bookTicketsFlow(username); break;
        case 3: viewBookingHistory(username); pause_screen(); break;
        case 4: { viewBookingHistory(username); int id = readIntInRange("Enter booking ID to cancel (or -1 to cancel): ", -1, 1000000); if (id == -1) break; if (cancelBookingById(username, id)) cout << GREEN << "Cancelled." << RESET << "\n"; else cout << RED << "Unable to cancel." << RESET << "\n"; pause_screen(); break; }
        case 5: orderSnacksPaid(username); pause_screen(); break;
        case 6: redeemSnackWithPoints(*acc); pause_screen(); break;
        case 7: luckyDraw(*acc); pause_screen(); break;
        case 8: cout << GREEN << "Logging out..." << RESET << "\n"; break;
        }
    } while (choice != 8);
}

void adminDashboard() {
    int choice = 0;
    do {
        cout << YELLOW << "===========================================\n" << "             ADMIN DASHBOARD\n" << "===========================================\n" << RESET;
        cout << "1. View All Movies\n2. View Sales Report\n3. View Users\n4. Edit User\n5. Delete User\n6. Reset Sales\n7. Logout\n";
        choice = readIntInRange("Enter your choice: ", 1, 7); clearScreen();
        switch (choice) {
        case 1: showMovies(); showShowtimes(); pause_screen(); break;
        case 2: {
            cout << YELLOW << "=========== SALES REPORT (DAILY) ===========\n" << RESET;
            cout << "Tickets sold by Movie:\n";
            for (int i = 0;i < MAX_MOVIES;++i) cout << " - " << movies[i] << ": " << dailyTicketsPerMovie[i] << "\n";
            cout << "\nTickets sold by Category:\n";
            const char* names[TTYPE_COUNT] = { "Adult","Student","Child","Senior" };
            for (int t = 0;t < TTYPE_COUNT;++t) cout << " - " << names[t] << ": " << dailyTicketsByType[t] << "\n";
            pause_screen(); break;
        }
        case 3: adminViewUsers(); pause_screen(); break;
        case 4: adminEditUser(); pause_screen(); break;
        case 5: adminDeleteUser(); pause_screen(); break;
        case 6: adminResetSales(); pause_screen(); break;
        case 7: cout << GREEN << "Admin logout." << RESET << "\n"; break;
        }
    } while (choice != 7);
}

// -------------------- main ---------------------------
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    srand(static_cast<unsigned int>(time(nullptr)));

    loadAllFromFiles();
    showIntro();
    displayRegisterTitle();

    string loggedInUser; bool isAdmin = false;
    if (!loginSystem(loggedInUser, isAdmin)) { cout << "Goodbye.\n"; return 0; }
    clearScreen();
    if (isAdmin) adminDashboard(); else userDashboard(loggedInUser);

    saveAllToFiles();
    cout << CYAN << "Program ended. Files saved: " << ACCOUNTS_FILE << ", " << BOOKINGS_FILE << ", " << SALES_FILE << "\n" << RESET;
    return 0;
}



