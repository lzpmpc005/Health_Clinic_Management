# README

# Health Clinic Management Web (MacOS)

## Overview

This project, named **Health Clinic Management Web**, is a C++ web application for managing health clinic operations. It includes features for registering patients and doctors, scheduling appointments, recording medical history, handling insurance claims, managing inventory, and placing orders.

## Requirements

* CMake version 3.26 or higher
* C++ Compiler supporting C++20
* [Crow](https://crowcpp.org/master/) C++ microframework
* [Homebrew](https://brew.sh/) package manager
* [SQLite library](https://formulae.brew.sh/formula/sqlite)
* [Boost library (version 1.83.0)](https://formulae.brew.sh/formula/boost#default)
* [ASIO library (version 1.28.1)](https://formulae.brew.sh/formula/asio#default)

## Building and Running

1. Ensure that the required dependencies are installed on your system.
2. Adjust the `CMakeLists.txt`​​ file if necessary.
3. (Recommonded) Use CLion to compile.
4. (Recommonded) Use Postman to compile.
5. Ensure the database is prepared before testing every specific function.

## Usage

The web application will be accessible at `http://127.0.0.1:18080`​.

To implement the web application the address should be like: `http://127.0.0.1:18080/register_patient`​

1. **Register Patient**: Send a POST request to `/register_patient`​ with patient details.
2. **View Patients**: Send a GET request to `/patients`​ to view the list of registered patients.
3. **Register Doctor**: Send a POST request to `/register_doctor`​ with doctor details.
4. **View Doctors**: Send a GET request to `/doctors`​ to view the list of registered doctors.
5. **Make Appointment**: Send a POST request to `/make_appointment`​ with appointment details.
6. **View Appointments**: Send a GET request to `/appointments`​ to view the list of appointments.
7. **Record Medical History**: Send a POST request to `/record_medical_history`​ with medical history details.
8. **View Medical History**: Send a GET request to `/medical_history`​ with the patient_id parameter.
9. **Update Insurance Claim**: Send a POST request to `/update_insurance_claim`​ with insurance claim details.
10. **View Insurance Claims**: Send a GET request to `/insurance_claims`​ to view the list of insurance claims.
11. **Make Order**: Send a POST request to `/make_order`​ with order details.
12. **Update Order Status**: Send a POST request to `/update_order_status`​ with order status details.
13. **Add to Inventory**: Send a POST request to `/add_to_inventory`​ with inventory details.
14. **Use from Inventory**: Send a POST request to `/use_from_inventory`​ with inventory usage details.

## Notes

* The application runs on `127.0.0.1`​ and port `18080`​ by default.
* Ensure proper input validation when interacting with the API.
* The SQLite database file (`clinic.db`​) is used for data storage.

Feel free to explore and extend the functionality of the Health Clinic Management Web as needed for your specific use case. Also feel free to leave you comments or contact us.
