#ifndef HOSPITAL_H
#define HOSPITAL_H

#include <vector>
#include <string>
#include "person.h"

class Hospital {
    std::vector<Doctor> doctors;
    std::vector<Nurse> nurses;
    std::vector<Staff> staff;
    std::vector<Patient> patients;
    std::vector<Appt> appointments;

    // next id to hand out for each kind of record
    int nextDoctorId, nextNurseId, nextStaffId, nextPatientId, nextApptId;

    std::string dir;

public:
    Hospital(std::string d = "data");

    void load();
    void save();
    void loadSamples();               // fills in some records to try things with
    bool isEmpty() const;

    // doctors
    int addDoctor(std::string name, int age, char sex, std::string phone,
               std::string specialty, double fee, double base, int days, double rate);
    Doctor* findDoctor(int id);
    bool removeDoctor(int id);
    const std::vector<Doctor>& allDoctors() const { return doctors; }

    // nurses
    int addNurse(std::string name, int age, char sex, std::string phone,
                 std::string ward, int nightShifts, double base, int days, double rate);
    Nurse* findNurse(int id);
    bool removeNurse(int id);
    const std::vector<Nurse>& allNurses() const { return nurses; }

    // support staff
    int addStaff(std::string name, int age, char sex, std::string phone,
                 std::string department, double overtime, double base, int days, double rate);
    Staff* findStaff(int id);
    bool removeStaff(int id);
    const std::vector<Staff>& allStaff() const { return staff; }

    // patients
    int addPatient(std::string name, int age, char sex, std::string phone,
               std::string illness, std::string bloodGroup, int doctorId);
    Patient* findPatient(int id);
    bool removePatient(int id);
    const std::vector<Patient>& allPatients() const { return patients; }

    // appointments
    int book(int patientId, int doctorId, std::string date, std::string time, int minutes);
    bool setStatus(int id, std::string status);
    std::vector<std::string> freeSlots(int doctorId, std::string date, int minutes) const;
    const std::vector<Appt>& allAppointments() const { return appointments; }

    // reports
    double payroll() const;
    void search(std::string term) const;

    std::string error() const { return lastError; }

private:
    std::string lastError;   // why the last call failed

    std::string filePath(std::string f) const { return dir + "/" + f; }
    void loadDocs();   void saveDocs();
    void loadNurses(); void saveNurses();
    void loadStaff();  void saveStaff();
    void loadPats();   void savePats();
    void loadAppts();  void saveAppts();
};

// small helpers, used by hospital.cpp and main.cpp
namespace tools {
    std::string trim(std::string s);
    std::vector<std::string> split(std::string s, char sep);
    int toInt(std::string s, int fallback = 0);
    double toDouble(std::string s, double fallback = 0.0);
    int toMinutes(std::string hhmm);       // "09:30" -> 570, -1 if bad
    std::string toClock(int minutes);      // 570 -> "09:30"
    bool goodDate(std::string d);       // yyyy-mm-dd
    std::string sanitize(std::string s);   // strips | so it can't break the filePath
}

#endif
