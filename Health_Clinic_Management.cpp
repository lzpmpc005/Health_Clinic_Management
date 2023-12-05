#include <iostream>
#include <cstdlib>
#include <ctime>
#include <crow.h>
#include <sqlite3.h>
#include <vector>

using namespace std;
using namespace crow;

class Patient {
public:
    int P_id;
    string P_name;
    string P_address;
    string P_history;

    Patient(int id, string name, string address, string history) : P_id(id), P_name(name), P_address(address), P_history(history) {}
};


class Doctor {
public:
    int D_id;
    string D_name;
    string D_specialization;
    vector<int> availableSlots;

    Doctor(int id, string name, string specialization) : D_id(id), D_name(name), D_specialization(specialization) {
        // Initialize available slots
        for (int i = 9; i <= 18; ++i) {
            availableSlots.push_back(i);
        }
    }
};


int Generate_id() {
    srand(time(NULL));
    int id = rand() % 9999999 + 1000000;
    return id;
}

void register_patient(Patient& p) {
    sqlite3* db;
    sqlite3_open("patients.db", &db);

    string sql = "CREATE TABLE IF NOT EXISTS patients (id INTEGER PRIMARY KEY, name TEXT, address TEXT, history TEXT);";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);

    sql = "INSERT INTO patients (id, name, address, history) VALUES (" + to_string(p.P_id) + ", '" + p.P_name + "', '" + p.P_address + "', '" + p.P_history + "');";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);

    sqlite3_close(db);
}

json::wvalue print_patients() {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("patients.db", &db);

    string sql = "SELECT * FROM patients ORDER BY name;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    json::wvalue patients;
    int index = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        string address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string history = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        patients[index]["id"] = id;
        patients[index]["name"] = name;
        patients[index]["address"] = address;
        patients[index]["history"] = history;
        index++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return patients;
}

void register_doctor(Doctor& d) {
    sqlite3* db;
    sqlite3_open("doctors.db", &db);

    string sql = "CREATE TABLE IF NOT EXISTS doctors (id INTEGER PRIMARY KEY, name TEXT, specialization TEXT);";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);

    sql = "INSERT INTO doctors (id, name, specialization) VALUES (" + to_string(d.D_id) + ", '" + d.D_name + "', '" + d.D_specialization + "');";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);

    sqlite3_close(db);
}

json::wvalue print_doctors() {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("doctors.db", &db);

    string sql = "SELECT * FROM doctors ORDER BY name;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    json::wvalue doctors;
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


Doctor get_doctor(int doctorId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("doctors.db", &db);

    string sql = "SELECT * FROM doctors WHERE id = " + to_string(doctorId) + ";";
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

bool patientExists(sqlite3* db, int patientId) {
    string sql = "SELECT * FROM patients WHERE id = " + to_string(patientId) + ";";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);

    return exists;
}


int make_appointment_with_doctor(int patientId, int doctorId, int slot) {
    sqlite3* db;
    sqlite3_open("appointments.db", &db);

    if (!patientExists(db, patientId)) {
        sqlite3_close(db);
        return -1; 
    }

    Doctor doctor = get_doctor(doctorId);
    auto it = find(doctor.availableSlots.begin(), doctor.availableSlots.end(), slot);

    if (it == doctor.availableSlots.end()) {
        sqlite3_close(db);
        return -1; 
    }

    string sql = "SELECT * FROM appointments WHERE doctor_id = " + to_string(doctorId) + " AND slot = " + to_string(slot) + ";";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }

    sqlite3_finalize(stmt);

    doctor.availableSlots.erase(it); // remove booked slot to avoid double booked

    sql = "CREATE TABLE IF NOT EXISTS appointments (patient_id INTEGER, doctor_id INTEGER, slot INTEGER);";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);

    sql = "INSERT INTO appointments (patient_id, doctor_id, slot) VALUES (" + to_string(patientId) + ", " + to_string(doctorId) + ", " + to_string(slot) + ");";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);

    sqlite3_close(db);

    return 0; 
}


int main() {
    SimpleApp hcm;

    CROW_ROUTE(hcm, "/register_patient").methods("POST"_method)
        ([](const request& req) {
        auto data = json::load(req.body);
        if (!data || !data.has("name") || !data.has("address") || !data.has("history")) {
            return response(400, "Invalid data");
        }

        string name = data["name"].s();
        string address = data["address"].s();
        string history = data["history"].s();
        int id = Generate_id();

        Patient patient(id, name, address, history);

        register_patient(patient);

        json::wvalue response_data;
        response_data["id"] = id;
        response_data["message"] = "Patient Successfully Registered";

        return response(200, response_data);
            });

    CROW_ROUTE(hcm, "/patients").methods("GET"_method)
        ([](const request& req) {
        return response(200, print_patients().dump());
            });

    CROW_ROUTE(hcm, "/register_doctor").methods("POST"_method)
        ([](const request& req) {
        auto data = json::load(req.body);
        if (!data || !data.has("name") || !data.has("specialization")) {
            return response(400, "Invalid data");
        }

        string name = data["name"].s();
        string specialization = data["specialization"].s();
        int id = Generate_id();

        Doctor doctor(id, name, specialization);

        register_doctor(doctor);

        json::wvalue response_data;
        response_data["id"] = id;
        response_data["message"] = "Doctor Successfully Registered";

        return response(200, response_data);
            });

    CROW_ROUTE(hcm, "/doctors").methods("GET"_method)
        ([](const request& req) {
        return response(200, print_doctors().dump());
            });

    CROW_ROUTE(hcm, "/make_appointment").methods("POST"_method)
        ([&](const request& req) {
        auto data = json::load(req.body);
        if (!data || !data.has("patient_id") || !data.has("doctor_id") || !data.has("slot")) {
            return response(400, "Invalid data");
        }

        int patientId = data["patient_id"].i();
        int doctorId = data["doctor_id"].i();
        int slot = data["slot"].i();

        int result = make_appointment_with_doctor(patientId, doctorId, slot);

        if (result == -1) {
            return response(400, "Slot not available or already booked");
        }

        json::wvalue response_data;
        response_data["patient_id"] = patientId;
        response_data["doctor_id"] = doctorId;
        response_data["slot"] = slot;
        response_data["message"] = "Appointment Successfully Made";

        return response(200, response_data);
            });

    hcm.bindaddr("127.0.0.1").port(18080).run();
}
