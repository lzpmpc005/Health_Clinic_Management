#include<iostream>
#include<vector>
#include<fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

// Structure the data of Patient 
class Patient {
public:
    int P_id;
    string P_name;
    string P_address;
    string P_history;

    friend ostream& operator<<(ostream & out, Patient & p) {
        out << p.P_id << " , " << p.P_name << " , " << p.P_address << " , " << p.P_history << "\n";

        return out;}
    
    friend istream& operator>>(istream & in, Patient & p) {
        in >> p.P_name >> p.P_address >> p.P_history;

    return in; 
    }
};


void PatientRegister()
{
    ofstream outfile("patients.txt", ios::app);

    Patient p;

    if (outfile.is_open()) {

        srand(time(NULL)); //generate a 7-digit id
        p.P_id = rand() % 9999999 + 1000000;

        cout << "\nPatient Name: ";
        cin >> p.P_name;

        cout << "Patient Address: ";
        cin >> p.P_address;

        cout << "Patient Medical History: ";
        cin >> p.P_history;

        outfile << p;
        outfile.close();
    }

    else {
        cout << "Unable to open file.\n";
    }

    cout << "\nPatient Registered successfully\n" << "Patient ID is : "<<p.P_id<<"\n\n";
    }

void MakeAppointment() {
    
}

int main()
{
m:
	cout << "\n\t\t\t Select Function: ";
	cout << "\n\n1) REGISTER";
	cout << "\n2) MAKE APPOINTMENT";
	cout << "\n3) Exit";

	cout << "\n\n Please Enter Your Choice: ";
	int choice;
	cin >> choice;

	switch (choice) {
	case 1:
        PatientRegister();
		break;
	case 2:
        MakeAppointment();
		break;
	case 3:
		exit(0);
	default:
		cout << "\n Please Select Right Number!";
	}
	goto m;
    
    return 0;
}