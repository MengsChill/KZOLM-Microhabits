#include <iostream>
#include <string> 
#include <iomanip>

using namespace std;

void prac4q1() {
  string cust_name;
  double purch_amt, sales_tax = 0.0, total_amt;
  int tax_code;

  cout << "Enter customer name: ";
  getline(cin, cust_name);
  cout << "Enter purchase amount: ";
  cin >> purch_amt;
  cout << "Enter tax code (0-4): ";
  cin >> tax_code;

  switch (tax_code) {
    case 0:
        sales_tax = 0.0;
        break;
    case 1:
    case 4:
        sales_tax = 0.06 * purch_amt;
        break;
    case 2:
        sales_tax = 0.10 * purch_amt;
        break;
    default:
        cout << "Invalid tax code entered." << endl;
  }

  total_amt = purch_amt + sales_tax;

  cout << fixed << setprecision(2);
  cout << "\nCustomer Name: " << cust_name << endl;
  cout << "Purchase Amount: $" << purch_amt << endl;
  cout << "Sales Tax: $" << sales_tax << endl;
  cout << "Total Amount Due: $" << total_amt << endl; 
}

void prac4q2() {
    int temp;

    cout << "Enter temperature: ";
    cin >> temp;

    if (temp > 85)
        cout << "Activity: Swimming" << endl;
    else if (temp > 70)
        cout << "Activity: Tennis" << endl;
    else if (temp > 32)
        cout << "Activity: Golf" << endl;
    else if (temp > 0)
        cout << "Activity: Skiing" << endl;
    else
        cout << "Activity: Dancing" << endl;    
}

void prac4q3() {
    int num1, num2, num3;
    int sum, product;
    float average;
    int smallest, largest;

    cout << "Enter three integers: ";
    cin >> num1 >> num2 >> num3;

    sum = num1 + num2 + num3;
    product = num1 * num2 * num3;
    average = sum / 3.0;

    smallest = num1;
    if (num2 < smallest) smallest = num2;
    if (num3 < smallest) smallest = num3;

    largest = num1;
    if (num2 > largest) largest = num2;
    if (num3 > largest) largest = num3;

    cout << "Sum: " << sum << endl;
    cout << "Product: " << product << endl;
    cout << "Average: " << average << endl;
    cout << "Smallest: " << smallest << endl;
    cout << "Largest: " << largest << endl;
}

void prac4q4() {
    float wage, total;
    int hours;

    cout << "Enter hourly wage: ";
    cin >> wage;
    cout << "Enter hours: ";
    cin >> hours;

    if (hours > 40) {
        total = (40 * wage) + ((hours - 40) * wage * 1.5);
    } else {
        total = hours * wage;
    }

    cout << "You have earned RM" << total << endl;

    if (hours < 40) {
        cout << "You have underworked." << endl;
    }
}

void prac4q5() {
    int capacity, people;

    cout << "Enter room capacity: ";
    cin >> capacity;

    cout << "Enter number of people: ";
    cin >> people;

    if (people <= capacity) {
        cout << "Meeting is allowed. ";
        cout << (capacity - people) << " more people can attend." << endl;
    } else {
        cout << "Meeting not allowed. ";
        cout << (capacity - people) << " people must leave. " << endl;
    }
}

void prac4q6() {
    double sales, income = 0;

    cout << "Enter monthly sales: ";
    cin >> sales;

    if (sales > 50000) {
        income = 375 + sales * 0.16;
    }
    else if (sales < 50000) {
        income = 350 + sales * 0.14;
    }
    else if (sales >= 40000 && sales < 50000) {
        income = 325 + sales * 0.12;
    }

}

void prac4q8() {
    char ch;
    int level;

    cout << "Enter colour code (R, G, B): ";
    cin >> ch;
    switch (ch) {
    case 'r': case 'R':
        cout << "RED\n";
        cout << "Enter level(1-3): ";
        cin >> level;
        switch (level) {
        case 1:
            cout << "Caution\n"; break;
        case 2: 
            cout << "Dangerous\n"; break;
        case 3:
            cout << "Critical\n"; break;
        default :
            cout << "Invalid level\n";
        }
        break;
    case 'g' :case 'G':
        cout << "GREEN\n"; break;
    case 'b' :case 'B':
        cout << "BLUE\n"; break;
    default:
        cout << "BLACK\n";
    }
}
int main() {
    //prac4q1();
    //prac4q2();
    //prac4q3();
   //prac4q4();
   //prac4q5();
   prac4q8();
    return 0;
}