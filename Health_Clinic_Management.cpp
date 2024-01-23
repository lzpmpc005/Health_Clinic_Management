#include <iostream>
#include <random>
#include <crow.h>
#include <sqlite3.h>
#include <vector>
#include <map>
#include <chrono>
#include <regex>
#include <mailio/message.hpp>
#include <mailio/smtp.hpp>

using namespace std;
using namespace mailio;

class Doctor {
public:
    int D_id;
    string D_name;
    string D_specialization;
    map<string, vector<int>> D_appointments;

    Doctor(int id, string name, string specialization)
        : D_id(id), D_name(name), D_specialization(specialization) {}

    void addAppointment(const string& date, const string& period, int patientId) {
        D_appointments[date + " " + period].push_back(patientId);
    }
    //The date and period are combined to create a key in the format "date period" in the appointments map.
};
class Patient {
public:
    struct Detail {
        int doctorId;
        string doctorName;
        string prescription;

        Detail(int id, string name, string description)
            : doctorId(id), doctorName(name), prescription(description) {}
    };
    struct InsuranceClaims {
        int claimId;
        string status;
        InsuranceClaims(int id, string status)
            : claimId(id), status(status) {}
    };

    int P_id;
    string P_name;
    string P_address;
    string P_phone;
    string P_insuranceProvider;
    string P_insurancePolicyNumber;
    map<string, vector<Detail>> P_MedicalHistory;
    map<string, vector<InsuranceClaims>> InsuranceClaims;


    Patient(int id, string name, string address, string phone, string insuranceProvider, string insurancePolicyNumber)
        : P_id(id), P_name(name), P_address(address), P_phone(phone), P_insuranceProvider(insuranceProvider), P_insurancePolicyNumber(insurancePolicyNumber) {}

    void addMedicalHistory(const string& date, const string& period, int doctorId, const string& doctorName, const string& prescription) {
        string key = date + " " + period;
        P_MedicalHistory[key].emplace_back(doctorId, doctorName, prescription);
    }

    //void fileInsuranceClaims(const string)
};
class MedicalHistory {
public:
    int P_id;
    int D_id;
    string M_date;
    string M_prescription;
    int M_timeCost;
    double M_examinationFee;
    string M_medication;

    MedicalHistory(int patientID, int doctorID, const string& date, const string& prescription, int timeCost, double examinationFee, const string& medication)
        : P_id(patientID), D_id(doctorID), M_date(date), M_prescription(prescription), M_timeCost(timeCost), M_examinationFee(examinationFee), M_medication(medication) {}
};
class Appointment {
public:
    int P_id;
    int D_id;
    string A_date;
    string A_period;

    Appointment(int patientID, int doctorID, const string& date, const string& period)
        : P_id(patientID), D_id(doctorID), A_date(date), A_period(period) {}
};
class InsuranceClaim {
public:
    int P_id;
    int C_id;
    string C_insuranceProvider;
    string C_insurancePolicyNumber;
    string C_status;

    InsuranceClaim(int patientID, int claimID, const string& insuranceProvider, const string& insurancePolicyNumber, const string& status)
        : P_id(patientID), C_id(claimID), C_insuranceProvider(insuranceProvider), C_insurancePolicyNumber(insurancePolicyNumber), C_status(status) {}
};
class Supplies {
public:
    string S_name;
    int S_quantity;
    double S_price;

    Supplies(const string& name, int quantity, double price)
        : S_name(name), S_quantity(quantity), S_price(price) {}
};
class Orders {
public:
    int O_id;
    string O_supplies;
    int O_quantity;
    string O_status;

    Orders(int orderID, const string& supplies, int quantity, const string& status)
        : O_id(orderID), O_supplies(supplies), O_quantity(quantity), O_status(status) {}
};

void createTables(sqlite3* db) {
    char* errorMessage;

    const char* createDoctorTableSQL = "CREATE TABLE IF NOT EXISTS doctor ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "specialization TEXT NOT NULL);";

    if (sqlite3_exec(db, createDoctorTableSQL, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        cerr << "Error creating Doctor table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    const char* createPatientTableSQL = "CREATE TABLE IF NOT EXISTS patient ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "address TEXT,"
        "phone TEXT NOT NULL,"
        "insuranceProvider TEXT,"
        "insurancePolicyNumber TEXT);";

    if (sqlite3_exec(db, createPatientTableSQL, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        cerr << "Error creating Patient table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    const char* createMedicalHistoryTableSQL = "CREATE TABLE IF NOT EXISTS medical_history ("
        "patientID INTEGER,"
        "doctorID INTEGER,"
        "date TEXT NOT NULL,"
        "prescription TEXT,"
        "timeCost INTEGER,"
        "examinationFee DOUBLE,"
        "medication TEXT,"
        "FOREIGN KEY(patientID) REFERENCES patient(id),"
        "FOREIGN KEY(doctorID) REFERENCES doctor(id));";

    if (sqlite3_exec(db, createMedicalHistoryTableSQL, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        cerr << "Error creating MedicalHistory table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    const char* createAppointmentTableSQL = "CREATE TABLE IF NOT EXISTS appointment ("
        "patientID INTEGER,"
        "doctorID INTEGER,"
        "date TEXT NOT NULL,"
        "period TEXT NOT NULL,"
        "FOREIGN KEY(patientID) REFERENCES patient(id),"
        "FOREIGN KEY(doctorID) REFERENCES doctor(id));";

    if (sqlite3_exec(db, createAppointmentTableSQL, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        cerr << "Error creating Appointment table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    const char* createInsuranceClaimsTableSQL = "CREATE TABLE IF NOT EXISTS insuranceClaims ("
        "patientID INTEGER,"
        "claimID INTEGER,"
        "insuranceProvider TEXT NOT NULL,"
        "insurancePolicyNumber TEXT NOT NULL,"
        "status TEXT NOT NULL,"
        "FOREIGN KEY(patientID) REFERENCES patient(id));";

    if (sqlite3_exec(db, createInsuranceClaimsTableSQL, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        cerr << "Error creating InsuranceClaims table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    const char* createSuppliesTableSQL = "CREATE TABLE IF NOT EXISTS supplies ("
        "supplyName TEXT NOT NULL,"
        "quantity INTEGER,"
        "price DOUBLE);";

    if (sqlite3_exec(db, createSuppliesTableSQL, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        cerr << "Error creating Supplies table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    const char* createOrdersTableSQL = "CREATE TABLE IF NOT EXISTS orders ("
        "orderID INTEGER,"
        "supplies TEXT NOT NULL,"
        "quantity INTEGER,"
        "status TEXT NOT NULL);";

    if (sqlite3_exec(db, createOrdersTableSQL, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        cerr << "Error creating orders table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }
}

int Generate_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(1000000, 9999999);

    return distribution(gen);
}

bool isValidName(const string& name) {
    if (name.empty() || name.size() > 20) {
        return false;
    }
    for (char c : name) {
        if (!isalpha(c) && c != ' ') {
            return false;
        }
    }
    return true;
}

bool isValidDate(const string& appointmentDate) {

    std::regex pattern("^\\d{4}-\\d{2}-\\d{2}$");
    if (!std::regex_match(appointmentDate, pattern)) {
        std::cerr << "Invalid date format: " << appointmentDate << std::endl;
        return false;
    }

    auto now = std::chrono::system_clock::now();
    time_t currentTime = std::chrono::system_clock::to_time_t(now);

    std::tm tmDate = {};
    std::istringstream ss(appointmentDate);
    ss >> std::get_time(&tmDate, "%Y-%m-%d");

    time_t appointmentTime = std::mktime(&tmDate);

    std::chrono::seconds diff = std::chrono::seconds(appointmentTime - currentTime);

    return diff.count() > 0 && diff.count() <= 365 * 24 * 60 * 60;
}

bool isValidPeriod(const string& period) {
    return (period == "9-11" || period == "11-13" || period == "13-15" || period == "15-17");
}

bool areSlotsAvailable(int doctorID, const string& date, const string& period) {
    sqlite3* db = nullptr;
    char* errMsg = nullptr;

    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    const char* query = "SELECT COUNT(*) FROM appointment WHERE doctorID = ? AND date = ? AND period = ?";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "SQL error (prepare statement)" << endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, doctorID);
    sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, period.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int appointmentCount = sqlite3_column_int(stmt, 0);

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return appointmentCount < 1;
    }

    cerr << "SQL error (execution)" << endl;
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return false;
}

void register_doctor(Doctor& d) {
    sqlite3* db;
    sqlite3_open("clinic.db", &db);

    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr);
    string sql = "INSERT INTO doctor (id, name, specialization) VALUES (" + to_string(d.D_id) + ", '" + d.D_name + "', '" + d.D_specialization + "');";

    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        cerr << "Error registering doctor: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
    else {
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    }

    sqlite3_close(db);
}

Doctor get_doctor(int doctorId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM doctor WHERE id = " + to_string(doctorId) + ";";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    Doctor doctor(0, "", "");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        doctor.D_id = sqlite3_column_int(stmt, 0);
        doctor.D_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        doctor.D_specialization = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return doctor;
}

// for retrieving id by phone and check if phone number already registered
int getPatientId(sqlite3* db, const string& phone) {
    sqlite3_stmt* stmt;

    string sql = "SELECT id FROM patient WHERE phone = '" + phone + "';";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    int existingPatientId = 0;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        existingPatientId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return existingPatientId;
}

int register_patient(Patient& p, int& existingPatientId) {
    sqlite3* db;
    sqlite3_open("clinic.db", &db);

    if ((p.P_phone.size() != 11) || (!std::all_of(p.P_phone.begin(), p.P_phone.end(), ::isdigit))) {
        sqlite3_close(db);
        return -1;
    }

    existingPatientId = getPatientId(db, p.P_phone);
    if (existingPatientId) {
        sqlite3_close(db);
        return -2;
    }

    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr);
    string sql = "INSERT INTO patient (id, name, address, phone, insuranceProvider, insurancePolicyNumber) VALUES (" + to_string(p.P_id) + ", '" + p.P_name + "', '" + p.P_address + "', '" + p.P_phone + "', '" + p.P_insuranceProvider + "', '" + p.P_insurancePolicyNumber + "');";
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        cerr << "Error registering patient: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
    else {
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    }
    sqlite3_close(db);

    return 0;
}

Patient get_patient(int patientId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM patient WHERE id = " + to_string(patientId) + ";";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    Patient patient(0, "", "", "", "", "");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        patient.P_id = sqlite3_column_int(stmt, 0);
        patient.P_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        patient.P_address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        patient.P_phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        patient.P_insuranceProvider = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        patient.P_insurancePolicyNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return patient;
}

vector<MedicalHistory> get_medical_history(int patientId);

// Can print_patients call get_patient?
crow::json::wvalue print_patients() {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM patient ORDER BY name;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    crow::json::wvalue patients;
    int index = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        string address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        patients[index]["id"] = id;
        patients[index]["name"] = name;
        patients[index]["address"] = address;
        patients[index]["phone"] = phone;

        vector<MedicalHistory> history = get_medical_history(id);
        if (!history.empty()) {
            int historyIndex = 0;
            for (const auto& entry : history) {
                patients[index]["medical_history"][historyIndex]["doctor_id"] = entry.D_id;
                patients[index]["medical_history"][historyIndex]["date"] = entry.M_date;
                patients[index]["medical_history"][historyIndex]["prescription"] = entry.M_prescription;
                historyIndex++;
            }
        }

        index++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return patients;
}

crow::json::wvalue print_doctors() {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM doctor ORDER BY name;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    crow::json::wvalue doctors;
    int index = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        string specialization = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        doctors[index]["id"] = id;
        doctors[index]["name"] = name;
        doctors[index]["specialization"] = specialization;

        index++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return doctors;
}

void make_appointment(Appointment& a) {
    sqlite3* db;
    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        return;
    }

    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr);

    string sql = "INSERT INTO appointment (patientID, doctorID, date, period) VALUES (" + to_string(a.P_id) + ", " + to_string(a.D_id) + ", '" + a.A_date + "', '" + a.A_period + "');";
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        cerr << "Error making appointment: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
    else {
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    }

    sqlite3_close(db);
}

Appointment get_appointment(int patientId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM appointment WHERE patientID = " + to_string(patientId) + " ORDER BY date DESC LIMIT 1;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    Appointment appointment(0, 0, "", "");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        appointment.P_id = sqlite3_column_int(stmt, 0);
        appointment.D_id = sqlite3_column_int(stmt, 1);
        appointment.A_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        appointment.A_period = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return appointment;
}

crow::json::wvalue print_appointments() {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM appointment ORDER BY date;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    crow::json::wvalue appointments;
    int index = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int patient_id = sqlite3_column_int(stmt, 0);
        int doctor_id = sqlite3_column_int(stmt, 1);
        string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string period = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        appointments[index]["patient_id"] = patient_id;
        appointments[index]["doctor_id"] = doctor_id;
        appointments[index]["date"] = date;
        appointments[index]["period"] = period;

        index++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return appointments;
}

void record_medical_history(int patientId, int doctorId, const string& date, const string& prescription, int timeCost, double examinationFee, const string& medication) {
    sqlite3* db;
    sqlite3_open("clinic.db", &db);

    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr);

    string sql = "INSERT INTO medical_history (patientID, doctorID, date, prescription, timeCost, examinationFee, medication) VALUES ("
        + to_string(patientId) + ", " + to_string(doctorId) + ", '" + date + "', '" + prescription + "', " + to_string(timeCost) + ", " + to_string(examinationFee) + ", '" + medication + "');";

    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    sqlite3_close(db);
}

vector<MedicalHistory> get_medical_history(int patientId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM medical_history WHERE patientID = " + to_string(patientId) + ";";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    vector<MedicalHistory> medicalHistory;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int doctorId = sqlite3_column_int(stmt, 1);
        string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string prescription = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int timeCost = sqlite3_column_double(stmt, 4);
        double examinationFee = sqlite3_column_double(stmt, 5);
        string medication = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        medicalHistory.emplace_back(patientId, doctorId, date, prescription, timeCost, examinationFee, medication);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return medicalHistory;
}

double getMedicinePrice(const string& medicineName) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("medicine.db", &db);

    string sql = "SELECT price FROM price WHERE medicine = ?;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, medicineName.c_str(), -1, SQLITE_STATIC);

    double medicinePrice = 0.0;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        medicinePrice = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return medicinePrice;
}

double calculateMedicationsCost(const vector<string>& medications) {
    double medationsCost = 0.0;

    for (const string& medicine : medications) {
        double medicinePrice = getMedicinePrice(medicine);

        medationsCost += medicinePrice;
    }

    return medationsCost;
}

struct Bill {
    double totalCost;
    double baseFee;
    double examinationFee;
    double medicationsFee;
};

Bill generateBill(int timeCost, double examinationFee, const string& medication) {
    // Split medications string into individual medications
    istringstream medicationStream(medication);
    vector<std::string> medications;
    string singleMedication;
    while (getline(medicationStream, singleMedication, ',')) {
        medications.push_back(singleMedication);
    }

    double totalMedicationsFee = calculateMedicationsCost(medications);

    Bill bill;
    bill.baseFee = timeCost * 2.3;
    bill.examinationFee = examinationFee;
    bill.medicationsFee = totalMedicationsFee;
    bill.totalCost = timeCost * 2.3 + examinationFee + totalMedicationsFee;

    return bill;
}

void fileInsuranceClaim(int patientId, const string& insuranceProvider, const string& insurancePolicyNumber, const string& prescription, Bill& bill)
{
    sqlite3* db;
    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        return;
    }

    int C_id = Generate_id();
    string status = "Submitted";

    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr);

    string sql = "INSERT INTO insuranceClaims (patientID, claimID, insuranceProvider, insurancePolicyNumber, status) VALUES (" + to_string(patientId) + ", " + to_string(C_id) + ", '" + insuranceProvider + "', '" + insurancePolicyNumber + "', '" + status + "');";
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
    else {
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    }

    sqlite3_close(db);
}

void updateInsuranceClaim(int claimId, const string& status)
{
    sqlite3* db;
    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        return;
    }

    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr);

    string sql = "UPDATE insuranceClaims SET status = '" + status + "' WHERE claimID = " + to_string(claimId) + ";";

    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
    else {
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    }

    sqlite3_close(db);
}

InsuranceClaim get_claim(int claimId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM insuranceClaims WHERE claimID = " + to_string(claimId) + ";";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    InsuranceClaim claim(0, 0, "", "", "");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        claim.P_id = sqlite3_column_int(stmt, 0);
        claim.C_id = sqlite3_column_int(stmt, 1);
        claim.C_insuranceProvider = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        claim.C_insurancePolicyNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        claim.C_status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return claim;
}

crow::json::wvalue print_insuranceClaims() {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM insuranceClaims ORDER BY patientID;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    crow::json::wvalue claims;
    int index = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int P_id = sqlite3_column_int(stmt, 0);
        int C_id = sqlite3_column_int(stmt, 1);
        string C_insuranceProvider = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string C_insurancePolicyNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        string C_status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        claims[index]["patientID"] = P_id;
        claims[index]["claimID"] = C_id;
        claims[index]["insuranceProvider"] = C_insuranceProvider;
        claims[index]["insurancePolicyNumber"] = C_insurancePolicyNumber;
        claims[index]["status"] = C_status;

        index++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return claims;
}

void make_order(const string& supply, int quantity) {
    sqlite3* db;
    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        return;
    }
    int id = Generate_id();
    string status = "Ordered";

    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr);

    string sql = "INSERT INTO orders (orderID, supplies, quantity, status) VALUES (" + to_string(id) + ", '" + supply + "', " + to_string(quantity) + ", '" + status + "');";
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
    else {
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    }

    sqlite3_close(db);
}

void updateOrderStatus(int orderId, const string& status)
{
    sqlite3* db;
    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        return;
    }

    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr);

    string sql = "UPDATE orders SET status = '" + status + "' WHERE orderId = " + to_string(orderId) + ";";

    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
    else {
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    }

    sqlite3_close(db);
}

Orders get_order(int orderId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM orders WHERE orderID = " + to_string(orderId) + ";";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    Orders order(0, "", 0, "");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        order.O_id = sqlite3_column_int(stmt, 0);
        order.O_supplies = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        order.O_quantity = sqlite3_column_int(stmt, 2);
        order.O_status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return order;
}

void addSupply(int orderID) {

    Orders order = get_order(orderID);

    sqlite3* db;
    sqlite3_open("clinic.db", &db);

    int quantity = order.O_quantity;
    string supplyName = order.O_supplies;
    double price = 9.9;

    string checkSql = "SELECT COUNT(*) FROM supplies WHERE supplyName = ?";
    sqlite3_stmt* checkStmt;
    sqlite3_prepare_v2(db, checkSql.c_str(), -1, &checkStmt, NULL);
    sqlite3_bind_text(checkStmt, 1, supplyName.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(checkStmt) == SQLITE_ROW && sqlite3_column_int(checkStmt, 0) == 0) {
        // Supply name doesn't exist, insert a new row
        string insertSql = "INSERT INTO supplies (supplyName, quantity, price) VALUES (?, ?, ?)";
        sqlite3_stmt* insertStmt;
        sqlite3_prepare_v2(db, insertSql.c_str(), -1, &insertStmt, NULL);

        sqlite3_bind_text(insertStmt, 1, supplyName.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(insertStmt, 2, quantity);
        sqlite3_bind_double(insertStmt, 3, price);

        sqlite3_step(insertStmt);
        sqlite3_finalize(insertStmt);
    }
    else {
        // Supply name exists, update the quantity
        string updateSql = "UPDATE supplies SET quantity = quantity + ? WHERE supplyName = ?";
        sqlite3_stmt* updateStmt;
        sqlite3_prepare_v2(db, updateSql.c_str(), -1, &updateStmt, NULL);

        sqlite3_bind_int(updateStmt, 1, quantity);
        sqlite3_bind_text(updateStmt, 2, supplyName.c_str(), -1, SQLITE_STATIC);

        sqlite3_step(updateStmt);
        sqlite3_finalize(updateStmt);
    }

    sqlite3_finalize(checkStmt);
    sqlite3_close(db);
}
/*
void alert() {
    try {
        // Create an email message
        message msg;
        msg.from(mail_address("mailio library", "youremail@gmail.com"));
        msg.add_recipient(mail_address("mailio library", "recipienteamil@gmail.com"));
        msg.subject("smtps simple message");
        msg.content("Alert content");

        smtps conn("smtp.gmail.com", 465);
        conn.authenticate("youreamil@gmail.com", "Sc121314£¡", smtps::auth_method_t::LOGIN);
        conn.submit(msg);
    }
    catch (const std::exception& e) {
        // Handle specific exceptions of type std::exception
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    catch (...) {
        // Handle any other unknown exceptions
        std::cerr << "Unknown exception occurred." << std::endl;
    }
}*/

int main() {
    //alert();
    sqlite3* db;
    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    createTables(db);
    sqlite3_close(db);

    crow::SimpleApp hcm;

    CROW_ROUTE(hcm, "/register_patient").methods("POST"_method)([&](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data || !data.has("name") || !data.has("address") || !data.has("phone") || !data.has("insuranceProvider") || !data.has("insurancePolicyNumber")) {
            return crow::response(400, "Invalid data");
        }

        int id = Generate_id();
        string name = data["name"].s();
        string address = data["address"].s();
        string phone = data["phone"].s();
        string insuranceProvider = data["insuranceProvider"].s();
        string insurancePolicyNumber = data["insurancePolicyNumber"].s();

        int existingPatientId;

        if (!isValidName(name)) {
            return crow::response(400, "Invalid name! Only letters and space and 20 characters most.");
        }

        Patient patient(id, name, address, phone, insuranceProvider, insurancePolicyNumber);

        int result = register_patient(patient, existingPatientId);

        crow::json::wvalue response_data;

        if (result == -1) {
            response_data["error_message"] = "Invalid Phone Number!";
        }
        else if (result == -2) {
            response_data["error_message"] = "Patient with the same phone number already exists!";
            response_data["existing_patient_id"] = existingPatientId;
        }
        else if (result == 0) {
            response_data["id"] = id;
            response_data["message"] = "Patient Successfully Registered";
        }
        else {
            response_data["error_message"] = "Failed to register patient!";
        }

        return crow::response(200, response_data);
        });

    CROW_ROUTE(hcm, "/patients").methods("GET"_method)([](const crow::request& req) {
        return crow::response(200, print_patients().dump());
        });

    CROW_ROUTE(hcm, "/register_doctor").methods("POST"_method)([](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data || !data.has("name") || !data.has("specialization")) {
            return crow::response(400, "Invalid data");
        }

        string name = data["name"].s();
        string specialization = data["specialization"].s();
        int id = Generate_id();

        if (!isValidName(name)) {
            return crow::response(400, "Invalid name");
        }

        Doctor doctor(id, name, specialization);

        register_doctor(doctor);

        crow::json::wvalue response_data;
        response_data["id"] = id;
        response_data["message"] = "Doctor Successfully Registered";

        return crow::response(200, response_data);
        });

    CROW_ROUTE(hcm, "/doctors").methods("GET"_method)([](const crow::request& req) {
        return crow::response(200, print_doctors().dump());
        });

    CROW_ROUTE(hcm, "/make_appointment").methods("POST"_method)([&](const crow::request& req) {

        auto data = crow::json::load(req.body);
        if (!data || !data.has("patient_id") || !data.has("doctor_id") || !data.has("date") || !data.has("period")) {
            return crow::response(400, "Invalid data");
        }

        int patientId = data["patient_id"].i();
        int doctorId = data["doctor_id"].i();
        string date = data["date"].s();
        string period = data["period"].s();

        if (!isValidDate(date)) {
            crow::json::wvalue response_data;
            response_data["error"] = "Invalid Date! Please Check Again and Input Date As: YYYY-MM-DD";
            return crow::response(400, response_data);
        }

        if (!isValidPeriod(period)) {
            crow::json::wvalue response_data;
            response_data["error"] = "Invalid Period! Please choose from 9-11, 11-13, 13-15, 15-17.";
            return crow::response(400, response_data);
        }

        Patient patient = get_patient(patientId);
        if (patient.P_id == 0) {
            crow::json::wvalue response_data;
            response_data["error"] = "Patient not found";
            return crow::response(400, response_data);
        }

        Appointment existingAppointment = get_appointment(patientId);
        if (existingAppointment.P_id != 0) {
            crow::json::wvalue response_data;
            response_data["error"] = "Patient already has an appointment";
            response_data["doctor"] = get_doctor(existingAppointment.D_id).D_name;
            response_data["date"] = existingAppointment.A_date;
            response_data["period"] = existingAppointment.A_period;
            return crow::response(400, response_data);
        }

        Doctor doctor = get_doctor(doctorId);
        if (doctor.D_id == 0) {
            crow::json::wvalue response_data;
            response_data["error"] = "Doctor not found";
            return crow::response(400, response_data);
        }

        if (!areSlotsAvailable(doctorId, date, period)) {
            crow::json::wvalue response_data;
            response_data["error"] = "Doctor " + doctor.D_name + " is not available at this time";
            return crow::response(400, response_data);
        }

        Appointment appointment(patientId, doctorId, date, period);

        try {
            make_appointment(appointment);
            crow::json::wvalue response_data;
            response_data["message"] = "Appointment Successfully Made";
            response_data["patient_id"] = patientId;
            response_data["doctor_id"] = doctorId;
            response_data["date"] = date;
            response_data["period"] = period;
            return crow::response(200, response_data);
        }
        catch (const std::runtime_error& e) {
            return crow::response(500, e.what());
        }
        });

    CROW_ROUTE(hcm, "/appointments").methods("GET"_method)([](const crow::request& req) {
        return crow::response(200, print_appointments().dump());
        });

    CROW_ROUTE(hcm, "/record_medical_history").methods("POST"_method)([&](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data || !data.has("patient_id") || !data.has("doctor_id") || !data.has("date") || !data.has("prescription") || !data.has("startTime") || !data.has("finishTime") || !data.has("examinationFee") || !data.has("medications")) {
            return crow::response(400, "Invalid data");
        }

        int patientId = data["patient_id"].i();
        int doctorId = data["doctor_id"].i();
        string date = data["date"].s();
        string prescription = data["prescription"].s();
        int timeCost = data["finishTime"].d() * 100 - data["startTime"].d() * 100;
        double examinationFee = data["examinationFee"].d();
        string medication = data["medications"].s();

        if (!isValidDate(date)) {
            crow::json::wvalue response_data;
            response_data["error"] = "Invalid Date! Please Check Again and Input Date As: YYYY-MM-DD";
            return crow::response(400, response_data);
        }

        Patient patient = get_patient(patientId);
        if (patient.P_id == 0) {
            crow::json::wvalue response_data;
            response_data["error"] = "Patient not found";
            return crow::response(400, response_data);
        }


        Doctor doctor = get_doctor(doctorId);
        if (doctor.D_id == 0) {
            crow::json::wvalue response_data;
            response_data["error"] = "Doctor not found";
            return crow::response(400, response_data);
        }

        try {
            record_medical_history(patientId, doctorId, date, prescription, timeCost, examinationFee, medication);

            crow::json::wvalue response_data_medical_history;
            response_data_medical_history["message"] = "Medical History Successfully Recorded";
            response_data_medical_history["patient_id"] = patientId;
            response_data_medical_history["doctor_id"] = doctorId;
            response_data_medical_history["date"] = date;
            response_data_medical_history["prescription"] = prescription;

            Bill bill = generateBill(timeCost, examinationFee, medication);

            crow::json::wvalue response_data_bill;
            response_data_bill["Bill"] = "Generated";
            response_data_bill["Total Cost"] = bill.totalCost;
            response_data_bill["Base Fee"] = bill.baseFee;
            response_data_bill["Examination Fee"] = bill.examinationFee;
            response_data_bill["Medication Fee"] = bill.medicationsFee;


            crow::json::wvalue response_data_InsuranceClaim;
            if (patient.P_insuranceProvider.empty())
            {
                response_data_InsuranceClaim["Insurance Claim"] = "No Insurance Provider found!";
            }
            else
            {
                fileInsuranceClaim(patientId, patient.P_insuranceProvider, patient.P_insurancePolicyNumber, prescription, bill);
                response_data_InsuranceClaim["Insurance Claim"] = "Submitted!";
            }


            crow::response response(200);
            response.write(response_data_medical_history.dump());
            response.write(response_data_bill.dump());
            response.write(response_data_InsuranceClaim.dump());
            return response;
        }
        catch (const std::runtime_error& e) {
            return crow::response(500, e.what());
        }
        });

    CROW_ROUTE(hcm, "/medical_history").methods("GET"_method)([&](const crow::request& req) {
        int patientId = 0;
        const char* patientIdParam = req.url_params.get("patient_id");
        if (patientIdParam && *patientIdParam) {
            try {
                patientId = boost::lexical_cast<int>(patientIdParam);
            }
            catch (const boost::bad_lexical_cast& e) {
            }
        }

        Patient patient = get_patient(patientId);
        if (patient.P_id == 0) {
            crow::json::wvalue response_data;
            response_data["error"] = "Patient not found";
            return crow::response(400, response_data);
        }

        vector<MedicalHistory> history = get_medical_history(patientId);

        crow::json::wvalue response_data;
        response_data["patient_id"] = patientId;

        if (!history.empty()) {
            int index = 0;
            for (const auto& entry : history) {
                response_data["history"][index]["doctor_id"] = entry.D_id;
                response_data["history"][index]["date"] = entry.M_date;
                response_data["history"][index]["prescription"] = entry.M_prescription;
                index++;
            }
        }
        else {
            response_data["message"] = "No medical history found for the given patient ID.";
        }

        return crow::response(200, response_data);
        });

    CROW_ROUTE(hcm, "/update_insurance_claim").methods("POST"_method)([](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data || !data.has("claimID") || !data.has("status")) {
            return crow::response(400, "Invalid data");
        }

        int claimID = data["claimID"].i();
        string status = data["status"].s();

        InsuranceClaim claim = get_claim(claimID);
        if (claim.P_id == 0) {
            return crow::response(400, "Insurance claim not found! Please check claimID!");
        }

        updateInsuranceClaim(claimID, status);

        crow::json::wvalue response_data;
        response_data["Insurance Claim"] = claimID;
        response_data["Message"] = "Insurance claim status successfully updated!";

        return crow::response(200, response_data);
        });

    CROW_ROUTE(hcm, "/insurance_claims").methods("GET"_method)([](const crow::request& req) {
        return crow::response(200, print_insuranceClaims().dump());
        });

    CROW_ROUTE(hcm, "/make_order").methods("POST"_method)([&](const crow::request& req) {

        auto data = crow::json::load(req.body);
        if (!data || !data.has("supplyName") || !data.has("quantity")) {
            return crow::response(400, "Invalid data");
        }

        string supplyName = data["supplyName"].s();
        int quantity = data["quantity"].i();

        try {
            make_order(supplyName, quantity);
            crow::json::wvalue response_data;
            response_data["message"] = "Order Submitted";
            response_data["supplyName"] = supplyName;
            response_data["quantity"] = quantity;
            return crow::response(200, response_data);
        }
        catch (const std::runtime_error& e) {
            return crow::response(500, e.what());
        }
        });

    CROW_ROUTE(hcm, "/update_order_status").methods("POST"_method)([](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data || !data.has("orderID") || !data.has("status")) {
            return crow::response(400, "Invalid data");
        }

        int orderID = data["orderID"].i();
        string status = data["status"].s();

        updateOrderStatus(orderID, status);

        crow::json::wvalue response_data;
        response_data["Order"] = orderID;
        response_data["Message"] = "Order status updated!";

        if (status == "received") {
            addSupply(orderID);
        }

        return crow::response(200, response_data);
        });

    hcm.bindaddr("127.0.0.1").port(18080).run();
}