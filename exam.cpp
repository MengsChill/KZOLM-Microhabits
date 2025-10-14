#include <iostream>
#include <fstream>

using namespace std;

int main() {
    ofstream wFile;
    wFile.open("progress.txt");

    if (!wFile) {
    cout << "Error opening file!";
    return 1;
}
    string teamName;
    int p1, p2;
    for (int i=0; i<10; i++) {
        cout << "Enter team: #" << i << ":" ;
        cin >> teamName;
        cout << "Enter player 1 point:\n";
        cin >> p1;
        cout << "Enter player 2 point:\n";
        cin >> p2;

        wFile << teamName << "\t" << p1 << "\t" << p2 << "\n";
    
    }
    wFile.close();
    return 0;
}




