#include <iostream>
#include <string>
#include <iomanip>

using namespace std;

void prac3q1() {
    int userInput, weeks, days;
    userInput = weeks = days = 0;

    cout << "Enter the number of days : \n";
    cin >> userInput;
    weeks = userInput / 7;
    days =  userInput % 7;
    cout << userInput << "days = " << weeks << "weeks, " << days << "days.\n";
}

void prac3q2() {
    int userInput, d1, d2, d3, d4;
    bool flag = true;

do {
    cout << "Enter a 4-digit integer : ";
    cin >> userInput;
    if (userInput < 1000 || userInput > 9999) {
        cout << "!!!MUST be a 4-digit integer !!!\n";
    } else {
        flag = false;
        d1 = userInput / 1000;
        d2 = userInput % 1000 / 100;
        d3 = userInput % 1000 % 100 / 10;
        d4 = userInput % 10;
        cout << "\n1st digit : " << d1 << endl;
        cout << "2nd digit : " << d2 << endl;
        cout << "3rd digit : " << d3 << endl;
        cout << "4th digit : " << d4 << endl;
    }
} while (flag);
}

void prac3q3() {
    int userInput, num1, num2, num3, num4;
    bool flag = true;

    cout << "Enter a 4-digit integer : \n";
    cin >> userInput;
    if (userInput < 1000 || userInput > 9999) {
        cout << "!!! Must be a 4-digit integer !!!\n";
    }
    else {
        flag = false;
        num1 = userInput / 1000 * 1000;
        num2 = userInput % 1000 / 100 * 100;
        num3 = userInput % 1000 % 100 / 10 * 10;
        num4 = userInput % 10;

        cout << "\n" << num1 << "+" << num2 << "+" << num3 << "+" << num4 << endl;
        
    }
}

void prac3q4() {
    double centigrade, fahrenheit;

    cout << "Enter Centigrade (C) value :";
    cin >> centigrade;

    fahrenheit = 32 + (centigrade * (180.0 / 100.0));

    cout << "Fahrenheit(F) value :" << fahrenheit << endl;
}
void prac3q5() {
    double length, height;
     
    cout << "Enter the height of the triangle :";
    cin >> height;
    cout << "Enter the length of the triangle :";
    cin >> length;

    cout << "Area of triangle\n";
    cout << "------------------------\n";
    cout << "Height : " << setw(10) << height << "cm\n";
    cout << "Length : " << setw(10) << length << "cm\n";
    cout << "Area   : " << setw(10) << height * length / 2 << " cm\n";
    
}
void prac3q6() {
    double radius, height, cost;
    int amount;
    cout << "Please enter the radius of the cylinder (in cm) :";
    cin >> radius;
    cout << "Please enter the height of the cylinder (in cm) :";
    cin >> height;
    cout << "Please enter the cost of the material (in RM/cm^2) :";
    cin >> cost;
    cout << "Please enter the amount of cylinder :";
    cin >> amount;

    cout << "The total cost for producing" << amount << "containers with" << radius << "cm radius and" << height << "is RM" << cost << endl;
}
int main() {
     prac3q1();
    prac3q2();
    //prac3q3();
    //prac3q4();
    //prac3q5();
    //prac3q6();
    return 0;
}