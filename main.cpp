#include <iostream>
#include <cstdlib>
#include <ctime>
#include <crow.h>
#include <sqlite3.h>
#include <vector>
#include <map>
#include <chrono>
#include <boost/lexical_cast.hpp>

using namespace std;

class Doctor{
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
class Patient{
public:
    struct Detail{
        int doctorId;
        string doctorName;
        string prescription;

        Detail(int id, string name, string description)
                : doctorId(id), doctorName(name), prescription(description) {}
    };

    int P_id;
    string P_name;
    string P_address;
    string P_phone;
    map<string, vector<Detail>> P_MedicalHistory;

    Patient(int id, string name, string address, string phone)
            : P_id(id), P_name(name), P_address(address), P_phone(phone) {}

    void addMedicalHistory(const string& date, const string& period, int doctorId, const string& doctorName, const string& prescription) {
        string key = date + " " + period;
        P_MedicalHistory[key].emplace_back(doctorId, doctorName, prescription);
    }

};
class MedicalHistory{
public:
    int P_id;
    int D_id;
    string M_date;
    string M_period;
    string M_prescription;

    MedicalHistory(int patientID, int doctorID, const string& date, const string& period, const string& prescription)
            : P_id(patientID), D_id(doctorID), M_date(date), M_period(period), M_prescription(prescription) {}
};
class Appointment{
public:
    int P_id;
    int D_id;
    string A_date;
    string A_period;

    Appointment(int patientID, int doctorID, const string& date, const string& period)
            : P_id(patientID), D_id(doctorID), A_date(date), A_period(period) {}
};

// Function for creating tables in database
void createTables(sqlite3* db) {
    char* errorMessage;

    // Create Doctor table
    const char* createDoctorTableSQL = "CREATE TABLE IF NOT EXISTS doctor ("
                                       "id INTEGER PRIMARY KEY,"
                                       "name TEXT NOT NULL,"
                                       "specialization TEXT NOT NULL);";

    if (sqlite3_exec(db, createDoctorTableSQL, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        cerr << "Error creating Doctor table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    // Create Patient table
    const char* createPatientTableSQL = "CREATE TABLE IF NOT EXISTS patient ("
                                        "id INTEGER PRIMARY KEY,"
                                        "name TEXT NOT NULL,"
                                        "address TEXT,"
                                        "phone TEXT NOT NULL);";

    if (sqlite3_exec(db, createPatientTableSQL, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        cerr << "Error creating Patient table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    // Create MedicalHistory table
    const char* createMedicalHistoryTableSQL = "CREATE TABLE IF NOT EXISTS medical_history ("
                                               "patientID INTEGER,"
                                               "doctorID INTEGER,"
                                               "date TEXT NOT NULL,"
                                               "period TEXT NOT NULL,"
                                               "prescription TEXT,"
                                               "FOREIGN KEY(patientID) REFERENCES patient(id),"
                                               "FOREIGN KEY(doctorID) REFERENCES doctor(id));";

    if (sqlite3_exec(db, createMedicalHistoryTableSQL, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        cerr << "Error creating MedicalHistory table: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return;
    }

    // Create Appointment table
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

    cout << "Tables created successfully." << endl;
}

// Picking information from database
Patient get_patient_by_id(int patientId){
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM patient WHERE id = " + to_string(patientId) + ";";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    Patient patient(0, "", "", "");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        patient.P_id = sqlite3_column_int(stmt, 0);
        patient.P_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        patient.P_address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        patient.P_phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return patient;
}
Doctor get_doctor_by_id(int doctorId){
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM doctor WHERE id = " + to_string(doctorId) + ";";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

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
vector<MedicalHistory> get_medical_history_by_id(int patientId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM medical_history WHERE patientID = " + to_string(patientId) + ";";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    vector<MedicalHistory> medicalHistoryList;

    // Iterate through medical history records
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int doctorId = sqlite3_column_int(stmt, 1);
        string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string period = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        string prescription = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        MedicalHistory history(patientId, doctorId, date, period, prescription);
        medicalHistoryList.push_back(history);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return medicalHistoryList;
}
vector<Appointment> get_appointments_by_id(int doctorId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM appointment WHERE doctorID = " + to_string(doctorId) + ";";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    vector<Appointment> appointmentList;

    // Iterate through appointment records
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int patientId = sqlite3_column_int(stmt, 0);
        string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string period = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        Appointment appointment(patientId, doctorId, date, period);
        appointmentList.push_back(appointment);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return appointmentList;
}


// Random ID generating
int Generate_id(){
    srand(static_cast<unsigned int>(time(nullptr)));
    int id = rand() % 9999999 + 1000000;
    return id;
}

// Checking validation of input data
// Name
bool isValidName(const string& name){
    // No empty or > 20
    if (name.empty() || name.size() > 20) {
        return false;
    }
    // No other than alphabet and space
    for (char c : name) {
        if (!isalpha(c) && c != ' ') {
            return false;
        }
    }

    return true;
}
// Date
bool isValidDate(const string& date){
    auto now = chrono::system_clock::now();
    time_t currentTime = chrono::system_clock::to_time_t(now);

    tm tmDate = {};
    istringstream ss(date);
    ss >> get_time(&tmDate, "%Y-%m-%d");

    time_t appointmentTime = mktime(&tmDate);

    chrono::seconds diff = chrono::seconds(appointmentTime - currentTime);

    return diff.count() > 0 && diff.count() <= 365 * 24 * 60 * 60;
}
// Period
bool isValidPeriod(const string& period){
    return (period == "9-11" || period == "11-13" || period == "13-15" || period == "15-17");
}
// Slot to check if slots are available for a doctor on one period and day
bool areSlotsAvailable(int doctorID, const string& date, const string& period) {
    sqlite3* db = nullptr;
    char* errMsg = nullptr;

    // Open database
    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    // Use prepared statement to prevent SQL injection
    const char* query = "SELECT COUNT(*) FROM appointment WHERE doctorID = ? AND date = ? AND period = ?";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "SQL error (prepare statement)" << endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    // Bind parameters
    sqlite3_bind_int(stmt, 1, doctorID);
    sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, period.c_str(), -1, SQLITE_STATIC);

    // Execute the query
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int appointmentCount = sqlite3_column_int(stmt, 0);

        // If appointmentCount < availableCount which is 1, slot is available
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return appointmentCount < 1;
    }

    cerr << "SQL error (execution)" << endl;
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return false;
}

// Is patient existing check by phone
bool isExistingPatient(const string& phone){
    sqlite3 *db = nullptr;
    sqlite3_stmt* stmt;

    string sql = "SELECT COUNT(*) FROM patients WHERE phone = '" + phone + "';";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return count > 0;
}

// Doctor creating and retrieving
void register_doctor(Doctor& d) {

    // Open database and Insert data
    sqlite3* db;
    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return;
    }

    // Insert data into table and Commit changes
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

// Patient creating and retrieving
void register_patient(Patient& p) {

    // Open database and Insert data
    sqlite3* db;
    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return;
    }

    // Insert data into table and Commit changes
    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr);
    string sql = "INSERT INTO patient (id, name, address, phone) VALUES (" + to_string(p.P_id) + ", '" + p.P_name + "', '" + p.P_address + "', '" + p.P_phone + "');";
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        cerr << "Error registering patient: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
    else {
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    }

    sqlite3_close(db);
}

// Appointment making and retrieving
void make_appointment(Appointment& a){
    // Open database
    sqlite3* db;
    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return;
    }

    // Insert data into table and Commit changes
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

void record_medical_history(MedicalHistory& m){
    // Open database
    sqlite3* db;
    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return;
    }

    // Insert data into table and Commit changes
    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr);

    string sql = "INSERT INTO medical_history (patientID, doctorID, date, period, prescription) VALUES (" + to_string(m.P_id) + ", " + to_string(m.D_id) + ", '" + m.M_date + "', '" + m.M_period + "', '" + m.M_prescription + "');";
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        cerr << "Error recording medical history: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
    else {
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    }

    sqlite3_close(db);
}


int main(){
    sqlite3* db;
    // Open SQLite database
    if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    cout << "Opened database successfully" << endl;
    // Create Tables if not exists
    createTables(db);
    // Close database
    sqlite3_close(db);

    crow::SimpleApp hcm;

    // POST method input data
    CROW_ROUTE(hcm, "/register_doctor").methods("POST"_method)([](const crow::request& req) {
        // Parse JSON data from the request body and Check validation
        auto data = crow::json::load(req.body);
        if (!data || !data.has("name") || !data.has("specialization")) {
            return crow::response(400, "Invalid data");
        }

        // Extract values from JSON data
        int id = Generate_id();
        string name = data["name"].s();
        string specialization = data["specialization"].s();
        // Validate the doctor's name
        if (!isValidName(name)) {
            return crow::response(400, "Invalid name");
        }
        // Create a Doctor object
        Doctor doctor(id, name, specialization);
        // Create a JSON response and Return a response with the JSON data
        crow::json::wvalue response_data;
        response_data["id"] = id;
        response_data["message"] = "Doctor Successfully Registered";
        // Insert data into database or return error
        try {
            register_doctor(doctor);
            return crow::response(200, response_data);
        }
        catch (const runtime_error& e) {
            return crow::response(400, e.what());
        }
    });
    CROW_ROUTE(hcm, "/register_patient").methods("POST"_method)([](const crow::request& req) {
        // Parse JSON data from the request body and Check validation
        auto data = crow::json::load(req.body);
        if (!data || !data.has("name") || !data.has("phone")) {
            return crow::response(400, "Invalid data");
        }

        // Extract values from JSON data
        int id = Generate_id();
        string name = data["name"].s();
        string address = data["address"].s();
        string phone = data["phone"].s();
        // Validate the doctor's name
        if (!isValidName(name)) {
            return crow::response(400, "Invalid name");
        }
        // Create a Doctor object
        Patient patient(id, name, address, phone);
        // Create a JSON response and Return a response with the JSON data
        crow::json::wvalue response_data;
        response_data["id"] = id;
        response_data["message"] = "Patient Successfully Registered";
        // Insert data into database or return error
        try {
            register_patient(patient);
            return crow::response(200, response_data);
        }
        catch (const runtime_error& e) {
            return crow::response(400, e.what());
        }
    });
    CROW_ROUTE(hcm, "/make_appointment").methods("POST"_method)([&](const crow::request& req) {
        // Parse JSON data from the request body and Check validation
        auto data = crow::json::load(req.body);
        if (!data || !data.has("patient_id") || !data.has("doctor_id") || !data.has("date") || !data.has("period")) {
            return crow::response(400, "Invalid data");
        }

        // Extract values from JSON data
        int patientId = data["patient_id"].i();
        int doctorId = data["doctor_id"].i();
        string date = data["date"].s();
        string period = data["period"].s();

        // Validate the date
        if (!isValidDate(date)) {
            crow::json::wvalue response_data;
            response_data["error"] = "Invalid Date! Please Check Again and Input Date As: YYYY-MM-DD";
            return crow::response(400, response_data);
        }
        // Validate the period
        if (!isValidPeriod(period)) {
            crow::json::wvalue response_data;
            response_data["error"] = "Invalid Period! Please choose from 9-11, 11-13, 13-15, 15-17.";
            return crow::response(400, response_data);
        }

        // Check if the patient exists
        Patient patient = get_patient_by_id(patientId);
        if (patient.P_id == 0) {
            crow::json::wvalue response_data;
            response_data["error"] = "Patient not found";
            return crow::response(400, response_data);
        }

        // Check if the doctor exists
        Doctor doctor = get_doctor_by_id(doctorId);
        if (doctor.D_id == 0) {
            crow::json::wvalue response_data;
            response_data["error"] = "Doctor not found";
            return crow::response(400, response_data);
        }

        // Check if the appointment slot is available
        if (!areSlotsAvailable(doctorId, date, period)) {
            crow::json::wvalue response_data;
            response_data["error"] = "Appointment slot not available";
            return crow::response(400, response_data);
        }

        // Create an Appointment object
        Appointment appointment(patientId, doctorId, date, period);

        // Make the appointment and return the response
        try {
            make_appointment(appointment);
            crow::json::wvalue response_data;
            response_data["message"] = "Appointment Successfully Made";
            response_data["patient_id"] = patientId;
            response_data["doctor_id"] = doctorId;
            response_data["date"] = date;
            response_data["period"] = period;
            return crow::response(200, response_data);
        } catch (const std::runtime_error& e) {
            return crow::response(500, e.what());
        }
    });
    CROW_ROUTE(hcm, "/record_medical_history").methods("POST"_method)([&](const crow::request& req) {
        // Parse JSON data from the request body and Check validation
        auto data = crow::json::load(req.body);
        if (!data || !data.has("patient_id") || !data.has("doctor_id") || !data.has("date") || !data.has("period") || !data.has("prescription")) {
            return crow::response(400, "Invalid data");
        }

        // Extract values from JSON data
        int patientId = data["patient_id"].i();
        int doctorId = data["doctor_id"].i();
        string date = data["date"].s();
        string period = data["period"].s();
        string prescription = data["prescription"].s();

        // Validate the date
        if (!isValidDate(date)) {
            crow::json::wvalue response_data;
            response_data["error"] = "Invalid Date! Please Check Again and Input Date As: YYYY-MM-DD";
            return crow::response(400, response_data);
        }
        // Validate the period
        if (!isValidPeriod(period)) {
            crow::json::wvalue response_data;
            response_data["error"] = "Invalid Period! Please choose from 9-11, 11-13, 13-15, 15-17.";
            return crow::response(400, response_data);
        }

        // Check if the patient exists
        Patient patient = get_patient_by_id(patientId);
        if (patient.P_id == 0) {
            crow::json::wvalue response_data;
            response_data["error"] = "Patient not found";
            return crow::response(400, response_data);
        }

        // Check if the doctor exists
        Doctor doctor = get_doctor_by_id(doctorId);
        if (doctor.D_id == 0) {
            crow::json::wvalue response_data;
            response_data["error"] = "Doctor not found";
            return crow::response(400, response_data);
        }

        // Create an Appointment object
        MedicalHistory history(patientId, doctorId, date, period, prescription);

        // Make the appointment and return the response
        try {
            record_medical_history(history);
            crow::json::wvalue response_data;
            response_data["message"] = "Appointment Successfully Made";
            response_data["patient_id"] = patientId;
            response_data["doctor_id"] = doctorId;
            response_data["date"] = date;
            response_data["period"] = period;
            response_data["prescription"] = prescription;
            return crow::response(200, response_data);
        } catch (const std::runtime_error& e) {
            return crow::response(500, e.what());
        }
    });

    // GET method retrieve data
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

        // Check if the patient exists
        Patient patient = get_patient_by_id(patientId);
        if (patient.P_id == 0) {
            crow::json::wvalue response_data;
            response_data["error"] = "Patient not found";
            return crow::response(400, response_data);
        }

        vector<MedicalHistory> history = get_medical_history_by_id(patientId);

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
    CROW_ROUTE(hcm, "/appointments").methods("GET"_method)([&](const crow::request& req) {
        int doctorId = 0;
        const char* doctorIdParam = req.url_params.get("doctor_id");
        if (doctorIdParam && *doctorIdParam) {
            try {
                doctorId = boost::lexical_cast<int>(doctorIdParam);
            }
            catch (const boost::bad_lexical_cast& e) {
            }
        }

        // Check if the doctor exists
        Doctor doctor = get_doctor_by_id(doctorId);
        if (doctor.D_id == 0) {
            crow::json::wvalue response_data;
            response_data["error"] = "Doctor not found";
            return crow::response(400, response_data);
        }

        vector<Appointment> appointments = get_appointments_by_id(doctorId);

        crow::json::wvalue response_data;
        response_data["doctor_id"] = doctorId;

        if (!appointments.empty()) {
            int index = 0;
            for (const auto& entry : appointments) {
                response_data["appointments"][index]["patient_id"] = entry.P_id;
                response_data["appointments"][index]["date"] = entry.A_date;
                response_data["appointments"][index]["period"] = entry.A_period;
                index++;
            }
        }
        else {
            response_data["message"] = "No appointments found for the given doctor ID.";
        }

        return crow::response(200, response_data);
    });

    hcm.bindaddr("127.0.0.1").port(18080).run();
}
