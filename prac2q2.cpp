#include <iostream>
#include <string>
#include <iomanip>

#define PI 3.14159

using namespace std;

void prac2q1() {
    int num;
    double floatNum;
    string str;

    cout << "Enter integer number >: ";
    cin >> num;
    cout << "Enter floating point number >: ";
    cin >> floatNum;
    cout << "Enter string value >: ";
    cin >> str;

    cout << "Decimal value : " << left << setw(6) << dec << num << endl;
    cout << "Hexadecimal value : " << left << setw(6) << hex << num << endl;
    cout << "Octal value : " << left << setw(6) << oct << num << endl;

    cout << "Signed floating value : " << showpos << fixed << setprecision(4) << floatNum << endl;
    cout << "Scientific value : " << scientific << floatNum << endl;
    cout << "String value : " << right << setw(25) << setfill('*') << str << endl;

}

void prac2q2() {
    cout << setw(3) << "*" << endl;
    cout << setw(2) << "*" << setw(2) << "*" << endl;
    cout << "*" << setw(4) << "*" << endl;
    cout << setw(2) << "*" << setw(2) << "*" << endl;
    cout << setw(3) << "*" << endl;
}

void prac2q3() {
    double r, c;

    cout << "Enter radius >: ";
    cin >> r;
    c = 2 * PI * r;
    cout << "Circumference >: " << fixed << setprecision(2) << c << endl;

}
   
void prac2q4() {
    double  floatNum, fractionalNum;
    int integralNum;

    cout << "Enter a floating-point number > ";
    cin >> floatNum;
    integralNum = (int)floatNum;
    fractionalNum = floatNum - integralNum;
    cout << "\nIts integral part   : " << integralNum;
    cout << "\nIts fractional part : " << fractionalNum;
}

void prac2q5() {
    char ch;
    cout << "Enter a character: ";
    cin >> ch;

    cout << "The decimal value for " << ch << " is " << static_cast<int>(ch) << endl;
}

void prac2q6() {
    const int MAX_LOAN_DAYS = 14;
    const double FINE_RATE = 0.20;

    int numBooks, loanDays;
    cout << "------------------\n";
    cout << "BOOK LOAN SYSTEM\n";
    cout << "------------------\n";
    cout << "Enter the number of books  : \n";
    cin >> numBooks;
    cout << "Enter the days of the loan : \n";
    cin >> loanDays;
    int overdueDays = loanDays - MAX_LOAN_DAYS;
    double fine = 0.0;

    if (overdueDays > 0) {
        fine = overdueDays * numBooks * FINE_RATE;
    }
    cout << "-------------------------------------\n";
    if (overdueDays > 0) {
        cout << "Days overdue    : " << overdueDays << endl;
        cout << "Fine    : RM " << fine << endl;
    } else {
        cout << "No fine. Books returned on time." << endl;
    }

}
int main() {
    //prac2q1();
    //prac2q2();
    //prac2q3();
    //prac2q4();
    //prac2q5();
    //prac2q6();
    return 0;
}