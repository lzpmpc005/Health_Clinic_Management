@startuml

actor Patient
actor Doctor
actor Staff
participant "User Interface" as UserInterface
participant Database

activate Doctor
activate Staff
activate UserInterface
activate Database

== Register Doctors ==
Staff -> UserInterface: Register Doctor
activate UserInterface
UserInterface -> Database: Save Doctor Information
Database --> UserInterface: Doctor Registered
deactivate UserInterface

== Register Patients ==
Staff -> UserInterface: Register Patient
UserInterface -> Database: Save Patient Information
Database --> UserInterface: Patient Registered

== Make Appointment ==
Patient -> Doctor: Request Appointment
activate Patient
Doctor -> UserInterface: Check Doctor's Schedule
UserInterface -> Database: Check Availability
Database --> UserInterface: Availability Checked
UserInterface --> Doctor: Appointment Scheduled
deactivate Patient

== Record Medical History and Prescriptions ==
Patient -> Doctor: Provide Medical History
Doctor -> UserInterface: Record Medical History
UserInterface -> Database: Record Medical History
Database --> UserInterface: Medical History Recorded
Doctor -> UserInterface: Provide Prescription
UserInterface -> Database: Provide Prescription
Database --> UserInterface: Prescription Provided

== Check and Update Medicine Inventory ==
Staff -> UserInterface: Check Medicine Inventory
UserInterface -> Database: Get Medicine Inventory
Database --> UserInterface: Medicine Inventory Checked
Staff -> UserInterface: Update Medicine Inventory
UserInterface -> Database: Update Medicine Inventory
Database -->UserInterface: Medicine Inventory Updated

== Generate Bills ==
Staff -> UserInterface: Generate Bill
UserInterface -> Database: Generate Bill
Database --> UserInterface: Bill Generated

== Generate Insurance Reports ==
Staff -> UserInterface: Generate Insurance Report
UserInterface -> Database: Generate Insurance Report
Database --> UserInterface: Insurance Report Generated

deactivate Doctor
deactivate Staff
deactivate UserInterface
deactivate Database

@enduml
