#include <iostream>
#include <string>
#include <iomanip>

using namespace std;

void prac04q1() {
	string cust_name;
	double purch_amt = 0.0, sales_tax = 0.0, total = 0.0;
	int tax_code;

	cout << "Enter your name >: ";
	getline(cin, cust_name);

	cout << "Enter purchase amount >: ";
	cin >> purch_amt;

	cout << "\nCode | Description                                | Percentage\n";
	cout << "  0  | Tax exempt                                   | 0%\n";
	cout << "  1  | Government service tax only                  | 6%\n";
	cout << "  2  | Service charge only                          | 10%\n";
	cout << "  3  | Government service tax and service charge    | 16%\n";
	cout << "  4  | Goods and services tax only (GST)            | 6%\n";
	do {
		cout << "\nEnter code >: ";
		cin >> tax_code;

		switch (tax_code) {
		case 0:
			sales_tax = 0; break;
		case 1: case 4:
			sales_tax = 0.06 * purch_amt; break;
		case 2:
			sales_tax = 0.1 * purch_amt; break;
		case 3:
			sales_tax = 0.16 * purch_amt; break;
		default:
			cout << "Wrong tax code, please retry!!\n";
		}
	} while (tax_code < 0 || tax_code>4);//loop till obtain valid selection

	total = purch_amt + sales_tax;

	cout << cust_name << " with tax code " << tax_code << ", total payment is RM"
		<< fixed << setprecision(2) << total << endl;
}

void prac04q2() {
	/*
	START
		Variable definition and initialization
		Read temperature
		IF (temperature > 85)
			Print "Swimming"
			ELSE-IF (temperature > 70 AND temperature <= 85)
				Print "Tennis"
			ELSE-IF (temperature > 32 AND temperature <= 70)
				Print "Golf"
			ELSE-IF (temperature > 0 AND temperature <= 32)
				Print "Skiing"
			ELSE
				Print "Dancing"
		ENDIF
	END
	*/
	double temperature = 0.0;

	cout << "Enter temperature >: ";
	cin >> temperature;

	if (temperature > 85) {
		cout << "Swimming\n";
	}
	else if (temperature > 70 && temperature <= 85) {
		cout << " Tennis\n";
	}
	else if (temperature > 32 && temperature <= 70) {
		cout << "Golf\n";
	}
	else if (temperature > 0 && temperature <= 32) {
		cout << "Skiing\n";
	}
	else if (temperature <= 0) {
    cout << "Dancing\n";
}
}

void prac04q3() {
	/*
	START
		define & initialize variable
		read 3 integers num1, num2, num3
		sum = num1 + num2 + num3
		product = num1 * num2 * num3
		average = sum / 3
		SET smallest = num1
		IF(num2 < smallest)
			smallest = num2
		ENDIF
		IF(num3 < smallest)
			smallest =  num3
		ENDIF
		SET largest = num1
		IF(num2 > largest)
			largest = num2
		ENDIF
		IF(num3 > largest)
			largest =  num3
		ENDIF
		print sum, product, average, smallest, largest
	END
	*/
	int num1, num2, num3, sum, product, smallest, largest;
	double average;

	do {
		cout << "Enter 3 different numbers >: ";
		cin >> num1 >> num2 >> num3;
	} while (num1 == num2 || num2 == num3 || num1 == num3);//if duplicate numbers

	sum = num1 + num2 + num3;
	product = num1 * num2 * num3;
	average = (double)sum / 3;
	
	smallest = num1;
	if (num2 < smallest) {
		smallest = num2;
	}
	if (num3 < smallest) {
		smallest = num3;
	}

	largest = num1;
	if (num2 > largest) {
		largest = num2;
	}
	if (num3 > largest) {
		largest = num3;
	}

	cout << "\nSUM      : " << sum;
	cout << "\nProduct  : " << product;
	cout << "\nAverage  : " << average;
	cout << "\nSmallest : " << smallest;
	cout << "\nLargest : " << largest << "\n";
}

void prac04q4() {
	int hoursWorked, overtime;
	double wagePerHr, earnings;
	const int minWorkHr = 40;

	cout << "Enter hourly wage: ";
	cin >> wagePerHr;
	cout << "Enter hours: ";
	cin >> hoursWorked;

	//check if OT, 50% extra given
	overtime = hoursWorked - minWorkHr;

	if (overtime <= 0) {
		earnings = hoursWorked * wagePerHr;
		cout << "You have earned RM" << fixed << setprecision(2) << earnings << ".\n\n";
		if (overtime < 0) {//means the time worked is lesser than the required min work hour 40
			cout << "You are underworked.\n";
		}
	}
	else {
		earnings = (minWorkHr * wagePerHr) + (wagePerHr *(overtime*0.5));
		cout << "You have earned RM" << fixed << setprecision(2) << earnings << ".\n";
	}
}


void prac04q5(){
	/*
	START
		define & initialize variable
		read roomCapacity, numPeople
		IF (numPeople <= roomCapacity)
			print "legal to hold meeting"
			addtionalPeople = roomCapacity - numPeople
			IF (additionalPeople < 0)
				print additionPeople "may legally attend"
			ELSE
				print "Full capacity"
			ENDIF
		ELSE
			additionalPeople = numPeople - roomCapacity
			print "meeting cannot be held due to fire regulations"
			print additionPeople "must be exlcuded to meet fire regulations"
		ENDIF
	END
	*/

	int roomCapacity, numPeople, additionalPeople;

	cout << "Provide room capacity: ";
	cin >> roomCapacity;
	cout << "Provide number of attendees: ";
	cin >> numPeople;

	if (numPeople <= roomCapacity) {
		cout << "Legal to hold this meeting.\n";
		additionalPeople = roomCapacity - numPeople;
		if (additionalPeople > 0) {
			cout << "Still have " << additionalPeople << " seats left.\n";
		}
		else {
			cout << "Room is at max capacity now.\n";
		}
	}
	else {
		additionalPeople = numPeople - roomCapacity;
		cout << "Meeting cannot be held due to fire regulations.\n";
		cout << "A total of " << additionalPeople << " attendees need to be excluded to meet the fire regulations.\n";
	}
}

void prac04q6i() { //nested-if
	double sales, income = 0;

	cout << "Enter monthly sales: ";
	cin >> sales;

	if (sales > 50000) {
		income = 375 + sales * 0.16;	
	}
	else if (sales < 50000) { 
		income = 350 + sales * 0.14;

		if (sales < 40000) {//with this method, it will run all the if statements
			income = 325 + sales * 0.12;
		}
		if (sales < 30000) {
			income = 300 + sales * 0.09;
		}
		if (sales < 20000) {
			income = 250 + sales * 0.05;
		}
		if (sales < 10000) {
			income = 200 + sales * 0.03;
		}
	}
	cout << "Your Income is RM" << fixed << setprecision(2) << income << "\n";
}

void prac04q6ii() { //multi-way if-else
	double sales, income = 0;

	cout << "Enter monthly sales: ";
	cin >> sales;

	if (sales > 50000) {//with this method, it only runs the statement if is TRUE
		income = 375 + sales * 0.16;
	}
	else if (sales >= 40000 && sales < 50000) {
		income = 350 + sales * 0.14;
	}
	else if (sales >= 30000 && sales < 40000) {
		income = 325 + sales * 0.12;
	}
	else if (sales >= 20000 && sales < 30000) {
		income = 300 + sales * 0.09;
	}
	else if (sales >= 10000 && sales < 20000) {
		income = 250 + sales * 0.05;
	}
	else{
		income = 200 + sales * 0.03;
	}
	cout << "Your Income is RM" << fixed << setprecision(2) << income << "\n";
}

void prac04q7() {
	int flag;

	cout << "Flag val(1-3): ";
	cin >> flag;

	switch (flag) {
	case 1:
		cout << "HOT\n"; break;
	case 2:
		cout << "WARM\n"; break;
	case 3:
		cout << "COLD\n"; break;
	default:
		cout << "OUT OF RANGE\n";
	}
}

void prac04q8() {
	char ch;
	int level;

	cout << "Enter colour code (R,G,B): ";
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
		default:
			cout << "Invalid level\n";
		}
		break;
	case 'g':case'G':
		cout << "GREEN\n"; break;
	case 'b':case'B':
		cout << "BLUE\n"; break;
	default:
		cout << "BLACK\n";
	}
}


void prac05q1() {
	int N;
	cout << "Enter number: ";
	cin >> N;
	for (int i = 1; i <= N * N; i++)
	{
		cout << setw(5) << i;
		if (i % N == 0)
			cout << "\n";
	}
}
/*
Enter number: 4
	1    2    3    4
	5    6    7    8
	9   10   11   12
   13   14   15   16
*/

void prac05q1removeSetW() {
	int N;
	cout << "Enter number: ";
	cin >> N;
	for (int i = 1; i <= N * N; i++)
	{
		cout << i;
		if (i % N == 0)
			cout << "\n";
	}
}
/*
Enter number: 4
1234
5678
9101112
13141516
*/

void prac05q1ii() {
	int N;
	cout << "Enter number: ";
	cin >> N;
	//for (int i = 1; i <= N * N; i++)
	for (int i = N * N; i >= 1; i--) //reversal
	{
		cout << setw(5) << i;
		if (i % N == 1)
			cout << "\n";
	}
}
/*
Enter number: 4
   16   15   14   13
   12   11   10    9
	8    7    6    5
	4    3    2    1
*/

void prac05q2() {
	int firstNum, secondNum;

	cout << "Provide <first num> <second num> : ";
	cin >> firstNum >> secondNum;

	cout << "Odd numbers between " << firstNum << " and " << secondNum << ":\n";
	for (int i = firstNum; i <= secondNum; i++) {//i starts at firstNum, i loops through until secondNum
		if (i % 2 == 1) {
			cout << i << endl;
		}
	}
}

void prac05q3() {
	int sum = 0;
	for (int i = 30; i <= 50; i += 3) {
		cout << i << " + ";
		sum += i;
	}
	cout << "\b\b= " << sum << endl;
	//once for-loop completed, we have extra + sign at the end
	//using \b for backspace, we backspace our display & remove the extra + sign
	//\b twice because \b for space, \b for +
}

void prac05q4() {
	for (int i = 2; i <= 10; i += 2) {
		cout << "The square of " << i << " is " << i * i << endl;
	}
}

void prac05q5() {
	int input, cnt50 = 0, cnt100 = 0, cnt150 = 0, cnt200 = 0, cntInvalid = 0;

	cout << "Enter 10 integers (1-200): \n";
	for (int i = 0; i < 10; i++) {
		cout << "> ";
		cin >> input;
		if (input < 1 || input > 200) {
			cntInvalid++;
		} 
		else if (input <= 50) {
			cnt50++;
		}
		else if (input <= 100) {
			cnt100++;
		}
		else if (input <= 150) {
			cnt150++;
		}
		else 
			cnt200++;
	}

	cout << "Range          Count\n";
	cout << "-----          -----\n";
	cout << "   1 - 50:     " << cnt50 << endl;
	cout << " 51 - 100:     " << cnt100 << endl;
	cout << "101 - 150:     " << cnt150 << endl;
	cout << "151 - 200:     " << cnt200 << endl;
	cout << "  Invalid:     " << cntInvalid << endl;
}

void prac5q6() {
	char cont = 'y';
	int num;

	do {
		cout << "Enter integer (10-50) >: ";
		cin >> num;
		if (num >= 10 && num <= 50) {
			cout << "Valid\n";
		}
		else
			cout << "Invalid\n";
		cout << "Done.\n";
		cout << "Do you want to continue(Y)? ";
		cin >> cont;

	} while (cont == 'y' || cont == 'Y');
}

void prac5q7() {
	int num, product = 1;

	do {
		cout << "Enter a number: ";
		cin >> num;
		if (num > 0) {
			product = product * num;
		}
	} while (num > 0);
	cout << "The product of all your positive numbers is " << product << endl;
}

void prac5q8() {
	int num;

	cout << "Enter a number: ";
	cin >> num;

	cout << "The divisors are: ";
	for (int i = 1; i <= num; i++) {
		if (num % i == 0) {
			cout << i << " ";
		}
	}
}

void prac5q9() {
	int num = -999, cnt = 0;
	int smallest = 0;

	cout << "Enter +ve number, -999 to quit: ";
	cin >> num;
	smallest = num; //1st entry will be the "smallest number"
	cnt++;

	while (num != -999) {
		cout << "Enter +ve number, -999 to quit: ";
		cin >> num;

		if (num < smallest) {
			smallest = num; //redefine/swap to "smallest number"
			cnt = 1; //restart count to 1
		}
		else if (num == smallest) {
			cnt++;
		}
	}

	cout << "The smallest value is " << smallest
		<< " and it was entered " << cnt << " time(s)." << endl;

}

void prac05q10() {
	int num;

	cout << "Multiplication table of what number? ";
	cin >> num;

	for (int i = 1; i <= 12; i++) {
		cout << num << " X " << i << " = " << num * i << endl;
	}
}

void prac05q11() {
	int num, sum = 0;

	cout << "enter a number to sum itself again & again >: ";
	cin >> num;

	for (int i = 1; i <= num; i++) {
		for (int j = 1; j <= i; j++) {
			sum = sum + j;
		}
	}

	cout << "The big SUM: " << sum << endl;
}

void prac05q12() {
	int hot = 0, pleasant = 0, cold = 0;
	int temperature = 1, total = 0;
	double avg;

	while (temperature > 0) {
		cout << "Enter a temperature (0 or -negative to exit) >: ";
		cin >> temperature;

		if (temperature >= 85) {
			hot++;
		}
		else if (temperature >= 60) {
			pleasant++;
		}
		else if (temperature >= 1) {
			cold++;
		}

		//only total positive temperatures
		if (temperature >= 1) {
			total = total + temperature;
		}
	}
	avg = (double)total / (hot + pleasant + cold);

	cout << "Total hot temperature      >: " << hot << endl;
	cout << "Total pleasant temperature >: " << pleasant << endl;
	cout << "Total cold temperature     >: " << cold << endl;
	cout << "Average temperature        >: " << avg << endl;

	/*
	Total hot temperature      >: 6
	Total pleasant temperature >: 13
	Total cold temperature     >: 6
	Average temperature        >: 71.28
	*/
}

int main() {
	//prac04q1();
	//prac04q2();
}