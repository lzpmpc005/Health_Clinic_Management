# Health_Clinic_Management

## 1 Introduction

An automated health clinic management application. This version uses Crow package to handle http requests, and sqlite3 to manage data. The application has been implemented register function for both patients and doctors, we use phone number as a key to prevent duplicated registering. And you can also make appointments with doctor and doctors can record patient's medical history. And of course, all these data can be retrieved by senting GET request. More function is comming soon.
		
This application is built with C++ and can be run in both Windows/MacOs/Linux. But you need to make sure you set up your environment properly based on your OS environment.


## 2 How to Install and Run Health_Clinic_Management

* 1. Set up your environment

  * Install Crow     
Crow is a C++ framework for creating HTTP or Websocket web services, similar to Python's Flask.   
Install Crow follow the instruction ['here'](https://crowcpp.org/master/).

  * Install splite3        
We use sqlite3 to manage our data. If you don't have sqlite3 in your environment, search online based on your OS and IDE. May God Help You!


* 2. To be continue


## 3 Features

*(1) Register patient        
http://localhost:18080/register_patient        
request method: POST        
request body:        
{        
    "name": "Contain only letters and space, no longer than 20 characters",        
    "address": "No restriction now",        
    "phone": "11-digit phone number"        
}        

*(2) Retrieve patients        
http://localhost:18080/patients        
request method: GET        
   
*(3) Register doctor        
http://localhost:18080/register_doctor        
request method: POST        
request body:        
{        
    "name": "Contain only letters and space, no longer than 20 characters",        
    "specialization": "No restriction now"        
}        

*(4) Retrieve doctors        
http://localhost:18080/doctors        
request method: GET        

*(5) Make appointment        
http://localhost:18080/make_appointment        
request method: POST        
request body:        
{        
    "patient_id": XXXXXXX,        
    "doctor_id": XXXXXXX,        
    "date":"YYYY-MM-DD",        
    "period":" choose from [9-11, 11-13, 13-15, 15-17] "        
}        

*(6) Retrieve appointments        
http://localhost:18080/appointments        
request method: GET        

*(7) Record medical history        
http://localhost:18080/record_medical_history        
request method: POST             
request body:        
{        
    "patient_id": XXXXXXX,        
    "doctor_id": XXXXXXX,        
    "date": "YYYY-MM-DD",        
    "prescription": "no restriction"        
}        

*(8) Retrieve medical history        
http://localhost:18080/medical_history?patient_id=1029172        
request method: GET           

OR     

http://localhost:18080/medical_history        
request method: GET           
request params :            
	key: patient_id        
	value: XXXXXXX        

## 4 Project file structure

* Not now

## 5 Contribution

If you want to contribute or comment on this project, email lihongtaoix7@gmail.com.
