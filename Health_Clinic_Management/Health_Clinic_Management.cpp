#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <sstream>

using namespace std;

// Structure the data of Patient 
class Patient {
public:
    int P_id{};
    string P_name;
    string P_address;
    string P_history;

    friend ostream& operator<<(ostream & out, Patient & p) {
        out << p.P_id << "," << p.P_name << "," << p.P_address << "," << p.P_history << "\n";

        return out;
    }

    friend istream& operator>>(istream& in, Patient& p) {
        char comma; // to read the comma
        in >> p.P_id >> comma >> p.P_name >> comma >> p.P_address >> comma >> p.P_history;

        return in;
    }

};

// Structure the data of Appointment
class Appointment {
public:
    int A_id{};
    int P_id{};
    string P_name;
    string Comments;

    friend ostream& operator<<(ostream & out, Appointment & a) {
        out << a.A_id << "," << a.P_id << "," << a.P_name << "," << a.Comments << "\n";

        return out;
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

// Read data from patient.txt
vector<Patient> ReadPatientsFromFile() {
    vector<Patient> patients;
    ifstream infile("patients.txt");
    Patient p;

    if (infile.is_open()) {
        string line;
        while (getline(infile, line)) {
            stringstream ss(line);
            char comma; // to read the comma
            ss >> p.P_id >> comma >> p.P_name >> comma >> p.P_address >> comma >> p.P_history;
            patients.push_back(p);
        }
        infile.close();
    } else {
        cout << "Unable to open file.\n";
    }

    return patients;
}

// Make an appointment by Patient ID, current seats <= 10
int current_seats = 10 ;

void MakeAppointment(vector<Patient>& patients)
{
    Appointment a;
    Patient p;

    // Check if current seats enough
    if (current_seats <= 0) {
        cout << "No available seats for appointment. Please try again later.\n";
        return;
    }

    // Check if Patient ID exists
    int PatientID;
    cout << "Remaining seats: " << current_seats << "\n";
    cout << "Enter Patient ID for appointment: ";
    cin >> PatientID;

    bool PatientExists = false;
    for (const Patient& p : patients) {
        if (p.P_id == PatientID) {
            PatientExists = true;
            a.P_id = p.P_id;
            a.P_name = p.P_name;
            break;
        }
    }

    // If Patient Exist Make Appointment Else Return Error
    if (PatientExists) {
        //Write appointment logs into file
        ofstream outfile("appointments.txt", ios::app);
        if (outfile.is_open()) {
            srand(time(NULL)); // generate a unique appointment ID
            a.A_id = rand() % 9999999 + 1000000;

            cout << "Additional Comments: ";
            cin >> a.Comments;

            outfile << a;
            outfile.close();

            cout << "\nAppointment made successfully\n" << "Appointment ID is : "<<a.A_id<<"\n\n";

            current_seats = current_seats -1 ;

        }
        else {
            cout << "Unable to open file.\n";
        }
    }
    else{
        cout << "Patient ID not found. Please register first.\n";
    }
}

int main()
{
    vector<Patient> patients = ReadPatientsFromFile();

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
        MakeAppointment(patients);
		break;
	case 3:
		exit(0);
	default:
		cout << "\n Please Select Right Number!";
	}
	goto m;
    
    return 0;
}