#include <iostream>
#include <cstdlib>
#include <ctime>
#include <crow.h>
#include <sqlite3.h>
#include <vector>
#include <map>
#include <chrono>

using namespace std;

class MedicalHistory {
public:
    int patientId;
    int doctorId;
    string date;
    string prescription;

    MedicalHistory(int pId, int dId, const string& d, const string& pres)
        : patientId(pId), doctorId(dId), date(d), prescription(pres) {}
};


class Patient {
public:
    int P_id;
    string P_name;
    string P_address;
    string P_phone;

    Patient(int id, string name, string address, string phone) : P_id(id), P_name(name), P_address(address), P_phone(phone) {}
};


class Doctor {
public:
    int D_id;
    string D_name;
    string D_specialization;
    map<string, vector<int>> appointments;

    Doctor(int id, string name, string specialization) : D_id(id), D_name(name), D_specialization(specialization) {}

    void addAppointment(const string& date, int patientId, const string& period) {
        appointments[date + " " + period].push_back(patientId);
    }
};


class Appointment {
public:
    int patientId;
    int doctorId;
    string date;
    string period;

    Appointment(int pId, int dId, const string& d, const string& per)
        : patientId(pId), doctorId(dId), date(d), period(per) {}
};


int Generate_id() {
    srand(static_cast<unsigned int>(time(NULL)));
    int id = rand() % 9999999 + 1000000;
    return id;
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
    auto now = std::chrono::system_clock::now();
    time_t currentTime = std::chrono::system_clock::to_time_t(now);

    std::tm tmDate = {};
    std::istringstream ss(appointmentDate);
    ss >> std::get_time(&tmDate, "%Y-%m-%d");

    time_t appointmentTime = std::mktime(&tmDate);

    std::chrono::seconds diff = std::chrono::seconds(appointmentTime - currentTime);

    return diff.count() > 0 && diff.count() <= 365 * 24 * 60 * 60;
}


bool patientExists(sqlite3* db, const string& phone) {
    sqlite3_stmt* stmt;

    string sql = "SELECT COUNT(*) FROM patients WHERE phone = '" + phone + "';";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return count > 0;
}

// for retrieving id by phong
int getPatientId(sqlite3* db, const string& phone) {
    sqlite3_stmt* stmt;

    string sql = "SELECT id FROM patients WHERE phone = '" + phone + "';";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    int existingPatientId = -1;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        existingPatientId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return existingPatientId;
}


int register_patient(Patient& p, int& existingPatientId) {
    sqlite3* db;
    sqlite3_open("patients.db", &db);

    if (p.P_phone.size() != 11) {
        sqlite3_close(db);
        return -2;
    }

    existingPatientId = getPatientId(db, p.P_phone);
    if (existingPatientId != -1) {
        sqlite3_close(db);
        return -1;
    }

    string sql = "CREATE TABLE IF NOT EXISTS patients (id INTEGER PRIMARY KEY, name TEXT, address TEXT, phone TEXT UNIQUE);";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);

    sql = "INSERT INTO patients (id, name, address, phone) VALUES (" + to_string(p.P_id) + ", '" + p.P_name + "', '" + p.P_address + "', '" + p.P_phone + "');";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);

    sqlite3_close(db);

    return 0;
}


crow::json::wvalue print_patients() {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("patients.db", &db);

    string sql = "SELECT * FROM patients ORDER BY name;";
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


crow::json::wvalue print_doctors() {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("doctors.db", &db);

    string sql = "SELECT * FROM doctors ORDER BY name;";
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


Patient get_patient(int patientId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("patients.db", &db);

    string sql = "SELECT * FROM patients WHERE id = " + to_string(patientId) + ";";
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


bool appointmentExists(sqlite3* db, int patientId, int doctorId, const string& date, const string& period) {
    sqlite3_stmt* stmt;

    string sql = "SELECT * FROM appointments WHERE patient_id = " + to_string(patientId) +
        " AND doctor_id = " + to_string(doctorId) +
        " AND day = '" + date + "' AND period = '" + period + "';";

    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);

    return exists;
}


bool appointmentExistsForDoctor(sqlite3* db, int doctorId, const string& date, const string& period) {
    sqlite3_stmt* stmt;

    string sql = "SELECT * FROM appointments WHERE doctor_id = " + to_string(doctorId) +
        " AND date = '" + date + "' AND period = '" + period + "';";

    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);

    return exists;
}

vector<Appointment> get_appointment(int patientId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("appointments.db", &db);

    string sql = "SELECT * FROM appointments WHERE patient_id = " + to_string(patientId) + ";";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    vector<Appointment> appointments;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int doctorId = sqlite3_column_int(stmt, 1);
        string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string period = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        appointments.emplace_back(patientId, doctorId, date, period);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return appointments;
}


int make_appointment_with_doctor(int patientId, int doctorId, const string& date, const string& period) {
    sqlite3* db;
    sqlite3_open("appointments.db", &db);

    Patient patient = get_patient(patientId);
    if (patient.P_id == 0) {
        sqlite3_close(db);
        return -1;
    }

    Doctor doctor = get_doctor(doctorId);
    if (doctor.D_id == 0) {
        sqlite3_close(db);
        return -2;
    }

    vector<Appointment> patientAppointments = get_appointment(patientId);
    if (!patientAppointments.empty()) {
        sqlite3_close(db);
        return -3;
    }

    if (appointmentExists(db, patientId, doctorId, date, period) || appointmentExistsForDoctor(db, doctorId, date, period)) {
        sqlite3_close(db);
        return -4;
    }

    if (period == "AM" || period == "PM") {
        doctor.addAppointment(date, patientId, period);
    }
    else {
        sqlite3_close(db);
        return -5;
    }

    string sql = "CREATE TABLE IF NOT EXISTS appointments (patient_id INTEGER, doctor_id INTEGER, date TEXT, period TEXT);";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);

    sql = "INSERT INTO appointments (patient_id, doctor_id, date, period) VALUES (" + to_string(patientId) +
        ", " + to_string(doctorId) + ", '" + date + "', '" + period + "');";

    if (sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL) != SQLITE_OK) {
        cout << "Debug: Failed to insert appointment into the appointments table!" << endl;
        sqlite3_close(db);
        return -6;
    }

    sqlite3_close(db);

    return 0;
}


crow::json::wvalue print_appointments() {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("appointments.db", &db);

    string sql = "SELECT * FROM appointments ORDER BY date;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    crow::json::wvalue appointments;
    int index = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int patient_id = sqlite3_column_int(stmt, 0);
        int doctor_id = sqlite3_column_int(stmt, 1);
        string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string period = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        appointments[index]["Patient Id"] = patient_id;
        appointments[index]["Doctor ID"] = doctor_id;
        appointments[index]["Date"] = date;
        appointments[index]["Period"] = period;
        index++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return appointments;
}


int record_medical_history(int patientId, int doctorId, const string& date, const string& prescription) {
    sqlite3* db;
    sqlite3_open("medical_history.db", &db);

    Patient patient = get_patient(patientId);
    if (patient.P_id == 0) {
        cout << "Debug: Patient does not exist!" << endl;
        sqlite3_close(db);
        return -1;
    }

    Doctor doctor = get_doctor(doctorId);
    if (doctor.D_id == 0) {
        sqlite3_close(db);
        return -2;
    }

    string sql = "CREATE TABLE IF NOT EXISTS medical_history (patient_id INTEGER, doctor_id INTEGER, date TEXT, prescription TEXT);";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);

    sql = "INSERT INTO medical_history (patient_id, doctor_id, date, prescription) VALUES (" +
        to_string(patientId) + ", " + to_string(doctorId) + ", '" + date + "', '" + prescription + "');";

    if (sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL) != SQLITE_OK) {
        cout << "Debug: Failed to insert medical history into the medical history table!" << endl;
        sqlite3_close(db);
        return -3;
    }

    sqlite3_close(db);

    return 0;
}


vector<MedicalHistory> get_medical_history(int patientId) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    sqlite3_open("medical_history.db", &db);

    string sql = "SELECT * FROM medical_history WHERE patient_id = " + to_string(patientId) + ";";
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


int main() {
    crow::SimpleApp hcm;

    CROW_ROUTE(hcm, "/register_patient").methods("POST"_method)([&](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data || !data.has("name") || !data.has("address") || !data.has("phone")) {
            return crow::response(400, "Invalid data");
        }

        string name = data["name"].s();
        string address = data["address"].s();
        string phone = data["phone"].s();
        int id = Generate_id();
        int existingPatientId;

        if (!isValidName(name)) {
            return crow::response(400, "Invalid name! Only letters and space and 20 characters most.");
        }

        Patient patient(id, name, address, phone);

        int result = register_patient(patient, existingPatientId);

        crow::json::wvalue response_data;

        if (result == -1) {
            response_data["error_message"] = "Patient with the same phone number already exists!";
            response_data["existing_patient_id"] = existingPatientId;
        }
        else if (result == -2) {
            response_data["error_message"] = "Invalid Phone Number!";
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
            response_data["Error"] = "Invalid Date! Please Check Again and Input Date As : YYYY-MM-DD ";

            return crow::response(200, response_data);
        }

        int result = make_appointment_with_doctor(patientId, doctorId, date, period);

        crow::json::wvalue response_data;

        if (result == -1) {
            response_data["error_message"] = "Patient doesn't exist! Please Check the patient ID or Register first";
        }
        else if (result == -2) {
            response_data["error_message"] = "Doctor doesn't exist! Check the Doctor ID please";
        }
        else if (result == -3) {
            Appointment existingAppointment = get_appointment(patientId).front();
            response_data["error_message"] = "Already has an appointment on " +
                existingAppointment.date + " " +
                existingAppointment.period + " with Doctor " +
                get_doctor(doctorId).D_name +
                ". Cannot make another appointment.";
        }
        else if (result == -4) {
            response_data["error_message"] = "Doctor " + get_doctor(doctorId).D_name + " is not available on " + date + " " + period;
        }
        else if (result == -5) {
            response_data["error_message"] = "Invalid Period! Please choose AM or PM !";
        }
        else {
            response_data["message"] = "Appointment Successfully Made";
            response_data["patient_id"] = patientId;
            response_data["doctor_id"] = doctorId;
            response_data["date"] = date;
            response_data["period"] = period;
        }

        return crow::response(200, response_data);
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

        int result = record_medical_history(patientId, doctorId, date, prescription);

        crow::json::wvalue response_data;

        if (result == 0) {
            response_data["message"] = "Medical History Successfully Recorded";
            response_data["patient_id"] = patientId;
            response_data["doctor_id"] = doctorId;
            response_data["date"] = date;
            response_data["prescription"] = prescription;
        }
        else if (result == -1) {
            response_data["error_message"] = "Patient doesn't exist! Check the patient ID or Register first please";
        }
        else if (result == -2) {
            response_data["error_message"] = "Doctor doesn't exist! Check the Doctor ID please";
        }
        else {
            response_data["error_message"] = "Failed to record medical history!";
        }

        return crow::response(200, response_data);
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

        vector<MedicalHistory> history = get_medical_history(patientId);

        crow::json::wvalue response_data;
        response_data["patient_id"] = patientId;

        if (!history.empty()) {
            int index = 0;
            for (const auto& entry : history) {
                response_data["history"][index]["doctor_id"] = entry.doctorId;
                response_data["history"][index]["date"] = entry.date;
                response_data["history"][index]["prescription"] = entry.prescription;
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