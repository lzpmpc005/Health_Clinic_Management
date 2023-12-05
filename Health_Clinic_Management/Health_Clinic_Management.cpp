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

bool valide(const std::string& str) {
    if (str.empty()) {
        std::cout << "Error: Input cannot be empty.\n";
        return false;
    }

    for (char ch : str) {
        if (!std::isalnum(ch) && ch != ' ') {
            std::cout << "Error: Input contains special characters.\n";
            return false;
        }
    }

    return true; // No special characters or empty string
}

void PatientRegister() {

    ofstream outfile("patients.txt", ios::app);

    Patient p;

    if (outfile.is_open()) {

        srand(time(NULL)); // generate a 7-digit id
        p.P_id = rand() % 9999999 + 1000000;

        do {
            cout << "\nPatient Name: ";
            cin >> p.P_name;
        } while (!valide(p.P_name));

        do {
            cout << "Patient Address: ";
            cin >> p.P_address;
        } while (!valide(p.P_address));

        do {
            cout << "Patient Medical History: ";
            cin >> p.P_history;
        } while (!valide(p.P_history));

        outfile << p;
        outfile.close();
    }

    else {
        cout << "Unable to open file.\n";
    }

    cout << "\nPatient Registered successfully\n" << "Patient ID is : " << p.P_id << "\n\n";
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

vector<Appointment> ReadAppointmentsFromFile() {
    vector<Appointment> appointments;
    ifstream infile("appointments.txt");
    Appointment a;

    if (infile.is_open()) {
        string line;
        while (getline(infile, line)) {
            stringstream ss(line);
            char comma;
            ss >> a.A_id >> comma >> a.P_id >> comma >> a.P_name >> comma >> a.Comments;
            appointments.push_back(a);
        }
        infile.close();
    }
    else {
        cout << "Unable to open file.\n";
    }

    return appointments;
}

// Make an appointment by Patient ID, current seats <= 10
int Booked_Seats() {
    ifstream infile("appointments.txt");
    if (!infile) {
        cout << "Can't open 'appointments.txt'" << endl;
        return 10000; 
    }

    int booked_seats = 0;
    string line;

    while (getline(infile, line)) {
        booked_seats++;
    }

    infile.close();

    return booked_seats;
}


bool PatientExists(int P_ID) {
    vector<Patient> patients = ReadPatientsFromFile();
    Appointment a;
    Patient p;
    bool PatientExists = false;
    for (const Patient& p : patients) {
        if (p.P_id == P_ID) {
            PatientExists = true;
            break;
        }
    }
    return PatientExists;
}


bool NotBookedAlready(int P_ID) {
    vector<Appointment> appointments = ReadAppointmentsFromFile();
    Appointment a;
    bool NotBookedAlready = true;
    for (const Appointment& a : appointments) {
        if (a.P_id == P_ID) {
            NotBookedAlready = false;
            break;
        }
    }
    return NotBookedAlready;
}

void MakeAppointment(){
    vector<Patient> patients = ReadPatientsFromFile();
    vector<Appointment> appointments = ReadAppointmentsFromFile();
    Appointment a;
    Patient p;

    // Check if current seats enough
    int total_seats = 10;
    int current_seats = total_seats - Booked_Seats();
    if (current_seats <= 0) {
        cout << "No available seats for appointment. Please try again later.\n";
        return;
    }

    // Check if Patient ID exists
    int PatientID;
    cout << "Remaining seats: " << current_seats << "\n";
    cout << "Enter Patient ID for appointment: ";
    cin >> PatientID;

    if (!PatientExists(PatientID)) {
        cout << "Patient ID not found. Please register first.\n";
        return;
    }

    for (const Patient& p : patients) {
        if (p.P_id == PatientID) {
            a.P_id = p.P_id;
            a.P_name = p.P_name;
            break;
        }
    }

    if (NotBookedAlready(PatientID)) {
        
        ofstream outfile("appointments.txt", ios::app);
        if (outfile.is_open()) {
            srand(time(NULL)); 
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
        cout << "Patient ID already made an appointment.\n";
    }
}

int main(){
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