#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <cstdlib>
using namespace std;

struct Movie {
    string title;
    string showtime;
    vector<string> availableSeats;
};

struct User {
    string username;
    string password;
    int points = 0;
    string membership = "Silver";
};

vector<Movie> movies;
vector<pair<string, string>> comingSoon;
User* loggedInUser = nullptr;

void updateMembership(User& user) {
    if (user.points >= 500) user.membership = "Gold";
    else if (user.points >= 200) user.membership = "Silver";
    else user.membership = "Bronze";
}

void viewMovies() {
    cout << "\n===========================================\n";
    cout << "             MOVIES LIST\n";
    cout << "===========================================\n";
    for (size_t i = 0; i < movies.size(); i++) {
        cout << i + 1 << ". " << movies[i].title << " — " << movies[i].showtime << "\n";
    }
}

void addMovie() {
    Movie m;
    cout << "Enter movie title: ";
    cin.ignore();
    getline(cin, m.title);
    cout << "Enter showtime: ";
    getline(cin, m.showtime);
    m.availableSeats = {"A1", "A2", "B1", "B2", "C1"};
    movies.push_back(m);
    cout << "Movie added successfully!\n";
}

void removeMovie() {
    viewMovies();
    cout << "Select movie to remove: ";
    int c; cin >> c;
    if (c >= 1 && c <= movies.size()) {
        movies.erase(movies.begin() + c - 1);
        cout << "Movie removed successfully!\n";
    } else {
        cout << "Invalid selection!\n";
    }
}

// ================= ADMIN DASHBOARD =================
void adminDashboard() {
    int c;
    do {
        cout << "\n===========================================\n";
        cout << "             ADMIN DASHBOARD\n";
        cout << "===========================================\n";
        cout << "1. View All Movies\n";
        cout << "2. Add Movie / Showtime\n";
        cout << "3. Remove Movie\n";
        cout << "4. View Sales Report (demo)\n";
        cout << "5. Logout\n";
        cout << "-------------------------------------------\n";
        cout << "Enter choice: "; cin >> c;

        switch (c) {
        case 1: viewMovies(); break;
        case 2: addMovie(); break;
        case 3: removeMovie(); break;
        case 4: cout << "(Sales report TBD)\n"; break;
        case 5: break;
        }
    } while (c != 5);
}

// ================= USER DASHBOARD =================
double bookTicket() {
    if (movies.empty()) { cout << "No movies.\n"; return 0; }
    viewMovies();
    cout << "Select movie: ";
    int c; cin >> c;
    if (c<1 || c>movies.size()) return 0;
    Movie& m = movies[c - 1];

    cout << "Seats available: ";
    for (auto& s : m.availableSeats) cout << s << " ";
    cout << "\nChoose seat(s) (comma-separated, e.g., A1,B2): ";
    string seat; cin >> seat;

    bool found = false;
    for (auto it = m.availableSeats.begin(); it != m.availableSeats.end(); it++) {
        if (*it == seat) { found = true; m.availableSeats.erase(it); break; }
    }
    if (!found) { cout << "Seat not available.\n"; return 0; }

    cout << "Ticket type? (1=Adult RM19, 2=Student RM10, 3=Senior RM12): ";
    int t; cin >> t;
    double price = 19;
    if (t == 2) price = 10; else if (t == 3) price = 12;

    cout << "\n===========================================\n";
    cout << "              SNACKS & COMBOS MENU\n";
    cout << "===========================================\n";
    cout << "1. Popcorn Combo — RM 8.00\n";
    cout << "2. Family Feast — RM 20.00\n";
    cout << "3. Sweet Treat — RM 10.00\n";
    cout << "4. Skip\n";
    cout << "Choice: ";
    int sn; cin >> sn;
    double snack = 0;
    if (sn == 1) snack = 8; else if (sn == 2) snack = 20; else if (sn == 3) snack = 10;

    cout << "\nEnter promo code (or NONE): ";
    string promo; cin >> promo;
    double discount = 0;
    if (promo == "STUDENT10") discount = 5;
    else if (promo == "HAPPYHOUR") discount = price;

    srand(time(0));
    string prize = "None";
    if (rand() % 5 == 0) prize = "Free Popcorn";

    double subtotal = price + snack;
    double service = 2;
    double tax = (subtotal + service - discount) * 0.06;
    double total = subtotal + service + tax - discount;

    int earned = (int)total;
    loggedInUser->points += earned;
    updateMembership(*loggedInUser);

    cout << "\n===========================================\n";
    cout << "               BOOKING RECEIPT\n";
    cout << "===========================================\n";
    cout << "Movie: " << m.title << endl;
    cout << "Showtime: " << m.showtime << endl;
    cout << "Seat: " << seat << endl;
    cout << "Subtotal: RM " << subtotal << endl;
    cout << "Service Fee: RM " << service << endl;
    cout << "Tax (6%): RM " << tax << endl;
    cout << "Discount: -RM " << discount << endl;
    cout << "Lucky Draw Prize: " << prize << endl;
    cout << "-------------------------------------------\n";
    cout << fixed << setprecision(2);
    cout << "TOTAL: RM " << total << endl;
    cout << "Points Earned: +" << earned << " pts\n";
    cout << "===========================================\n";
    cout << "   Thank you for choosing Starlight Cinema!\n";
    cout << "===========================================\n";
    return total;
}

void viewComingSoon() {
    cout << "\n===========================================\n";
    cout << "          COMING SOON — TRAILERS\n";
    cout << "===========================================\n";
    for (size_t i = 0; i < comingSoon.size(); i++) {
        cout << i + 1 << ". " << comingSoon[i].first << " — (Releasing " << comingSoon[i].second << ")\n";
    }
}

void luckyDraw() {
    srand(time(0));
    vector<string> rewards = { "Free Small Popcorn","RM5 Voucher","Free Ticket","Better luck next time" };
    string reward = rewards[rand() % rewards.size()];
    cout << "You won: " << reward << "!\n";
}

int main() {
    cout << "Welcome to the Cinema Booking System!\n";
    // Entry point for testing added features
    return 0;
}
