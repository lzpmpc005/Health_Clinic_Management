#include <iostream>
#include <random>
#include <crow.h>
#include <sqlite3.h>
#include <vector>
#include <map>
#include <chrono>
#include <regex>

using namespace std;

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
class MedicalHistory {
public:
    int P_id;
    int D_id;
    string M_date;
    string M_prescription;

    MedicalHistory(int patientID, int doctorID, const string& date, const string& prescription)
        : P_id(patientID), D_id(doctorID), M_date(date), M_prescription(prescription) {}
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
        "phone TEXT NOT NULL);";

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

    return 0;
}

Patient get_patient(int patientId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("clinic.db", &db);

    string sql = "SELECT * FROM patient WHERE id = " + to_string(patientId) + ";";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

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
        medicalHistory.emplace_back(patientId, doctorId, date, prescription);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return medicalHistory;
}


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
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
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


void record_medical_history(int patientId, int doctorId, const string& date, const string& prescription) {
    sqlite3* db;
    sqlite3_open("clinic.db", &db);

    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr);

    string sql = "INSERT INTO medical_history (patientID, doctorID, date, prescription) VALUES ("
        + to_string(patientId) + ", " + to_string(doctorId) + ", '" + date + "', '" + prescription + "');";
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    sqlite3_close(db);
}

int main() {
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
        if (!data || !data.has("name") || !data.has("address") || !data.has("phone")) {
            return crow::response(400, "Invalid data");
        }

        int id = Generate_id();
        string name = data["name"].s();
        string address = data["address"].s();
        string phone = data["phone"].s();

        int existingPatientId;

        if (!isValidName(name)) {
            return crow::response(400, "Invalid name! Only letters and space and 20 characters most.");
        }

        Patient patient(id, name, address, phone);

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

    CROW_ROUTE(hcm, "/patients").methods("GET"_method)
        ([](const crow::request& req) {
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
        if (!data || !data.has("patient_id") || !data.has("doctor_id") || !data.has("date") || !data.has("prescription")) {
            return crow::response(400, "Invalid data");
        }

        int patientId = data["patient_id"].i();
        int doctorId = data["doctor_id"].i();
        string date = data["date"].s();
        string prescription = data["prescription"].s();

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
            record_medical_history(patientId, doctorId, date, prescription);
            crow::json::wvalue response_data;
            response_data["message"] = "Medical History Successfully Recorded";
            response_data["patient_id"] = patientId;
            response_data["doctor_id"] = doctorId;
            response_data["date"] = date;
            response_data["prescription"] = prescription;
            return crow::response(200, response_data);
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

    hcm.bindaddr("127.0.0.1").port(18080).run();
}
