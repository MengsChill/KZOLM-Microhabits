#include <iostream>
#include <string>

using namespace std;

void prac1q1() {
	string name;
	cout << "Welcome to TARUMT" << endl; 
	cout << "I'm a C++ program" << endl;
	cout << "What is your name?" << endl;
	
	getline(cin, name); 
	cout << "Hi, " << name << ". How are you?\n";
}

void prac1q2() {
    string name;
    int age;
    char gender;
    float income;

    cout << "Input\n"
         << "-------\n"
         << "Enter <name> <age> <gender> <income>: ";
         cin >> name >> age >> gender >> income;

    cout << "Output\n"
         << "-------\n"
         << "Name   : " << name << "\n"
         << "Age    : " << age << "\n"
         << "Gender : " << gender <<"\n"
         << "Income : " << income <<"\n";
}

void prac1q3() {
    string str1 = " * * * *\n";
    string str2 = "* * * *\n";
    cout << str1 << str2;
    cout << str1 << str2;
    cout << str1 << str2 << "\n";
}

void prac1q4() {
    cout << "Welcome to \"Malaysia\"! Malaysia is a beautiful country.\n\a";
}

int main() {
    //prac1q1();
    //prac1q2();
    //prac1q3();
    //prac1q4();
}