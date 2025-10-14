#include <iostream>
#include <string>
#include <iomanip>

#define MONTHLY_FEE 50.00

using namespace std;

int main() {
    int option = 0, monthSub = 0;
    double received = 0.00, change = 0.00, total = 0.00, discount = 0.00;
	cout << "ABC Streaming Payment Portal\n";
    cout << "============================\n";

    string custName;
    cout << "Enter customer name:";
    cin >> custName;
    getline (cin, custName);

    cout << "Customer membership Tier List\n";
    cout << "1. Normal\n";
    cout << "2. Silver\n";
    cout << "3. Gold\n";

    do {
        cout << "Enter customer membership (1-3):";
        cin >> option;
        if (option < 1 || option > 3) {
            cout << "Invalid option, please retry!\n";
        }
    } while (option < 1 || option > 3);

    do {
    cout << "Enter number of subscription month(s):";
    cin >> monthSub;
    if (monthSub < 1) {
        cout << "Invalid month subscription, please retry!\n";
    }
    } while (monthSub < 1);

    switch (option) {
        case 1:
            (monthSub >= 6) ? discount = (MONTHLY_FEE * monthSub) * 0.05 : discount = 0.00;
            break;
        case 2:
            (monthSub >= 4) ? discount = (MONTHLY_FEE * monthSub) * 0.10 : discount = 0.05;
            break;
        default:
            (monthSub >= 2) ? discount = (MONTHLY_FEE * monthSub) * 0.20 : discount = 0.10;
            break;
    }

    total = MONTHLY_FEE * monthSub - discount;
    cout << "Total Fee is RM" << fixed << setprecision (2) << total << endl;

    while (received < total) {
        cout << "Enter cash received: RM";
        cin >> received;

        if (received > total){
            cout << "Cash received is insufficient...\n";
        }
        else {
            change = received - total;
        }
    }
    cout << endl;
    cout << setw(29) << "Subscription Receipt\n";
    cout << "---------------------------------\n";
    cout << "Name               :" << custName << endl;
    cout << "Membership Type    :";
    switch (option) {
        case 1:
        cout << "NORMAL" << endl;
        break;
        case 2:      
        cout << "SILVER" << endl;
        break;
        default:     
        cout << "GOLD" << endl;
        break;
    }
    cout << "Months Subscribed :" << monthSub << endl;
    cout << "Monthly Fee : RM" << MONTHLY_FEE << endl;
    cout << "Discounted  : RM" << fixed << setprecision(2) << discount << endl;
    cout << "Final Payment   : RM" << fixed << setprecision(2) << total << endl;
    cout << "Payment Received : RM" << fixed << setprecision(2) << received << endl;
    cout << "Change         : RM" << fixed << setprecision(2) << change << endl;
    cout << "Thank You!\n";


return 0;
}
