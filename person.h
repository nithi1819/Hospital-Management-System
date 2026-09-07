#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <iostream>

// Person
//   Employee   -> Doctor, Nurse, Staff
//   Patient

class Person {
protected:
    int id;
    std::string name;
    int age;
    char sex;
    std::string phone;

public:
    Person() { id = 0; age = 0; sex = 'U'; }
    Person(int i, std::string n, int a, char s, std::string p) {
        id = i; name = n; age = a; sex = s; phone = p;
    }
    virtual ~Person() {}          // needed, we delete through Person*

    int getId() const { return id; }
    std::string getName() const { return name; }
    int getAge() const { return age; }
    char getSex() const { return sex; }
    std::string getPhone() const { return phone; }

    void setName(std::string n) { name = n; }
    void setPhone(std::string p) { phone = p; }

    virtual std::string role() const = 0;
    virtual void show() const;
};


class Employee : public Person {
protected:
    double base;     // fixed part of the monthly salary
    int days;        // days present this month
    double rate;     // paid per day present

public:
    Employee() { base = 0; days = 0; rate = 0; }
    Employee(int i, std::string n, int a, char s, std::string p)
        : Person(i, n, a, s, p) { base = 0; days = 0; rate = 0; }

    double getBase() const { return base; }
    int getDays() const { return days; }
    double getRate() const { return rate; }
    void setDays(int d) { days = d; }
    void setPay(double b, int d, double r) { base = b; days = d; rate = r; }

    virtual double pay() const = 0;   // each role works this out differently
    void show() const;
};


class Doctor : public Employee {
    std::string specialty;
    double fee;
    bool available;      // true if accepting appointments

public:
    Doctor() { fee = 0; available = true; }
    Doctor(int i, std::string n, int a, char s, std::string p)
        : Employee(i, n, a, s, p) { fee = 0; available = true; }

    std::string getSpecialty() const { return specialty; }
    double getFee() const { return fee; }
    bool isAvailable() const { return available; }
    void setSpecialty(std::string s) { specialty = s; }
    void setFee(double f) { fee = f; }
    void setAvailable(bool f) { available = f; }

    std::string role() const { return "Doctor"; }
    double pay() const { return base + days * rate + fee * 0.30; }
    void show() const;
};


class Nurse : public Employee {
    std::string ward;
    int nightShifts;

public:
    Nurse() { nightShifts = 0; }
    Nurse(int i, std::string n, int a, char s, std::string p)
        : Employee(i, n, a, s, p) { nightShifts = 0; }

    std::string getWard() const { return ward; }
    int getNightShifts() const { return nightShifts; }
    void setWard(std::string w) { ward = w; }
    void setNightShifts(int n) { nightShifts = n; }

    std::string role() const { return "Nurse"; }
    double pay() const { return base + days * rate + nightShifts * 500.0; }
    void show() const;
};


class Staff : public Employee {
    std::string department;
    double overtime;   // overtime hours

public:
    Staff() { overtime = 0; }
    Staff(int i, std::string n, int a, char s, std::string p)
        : Employee(i, n, a, s, p) { overtime = 0; }

    std::string getDepartment() const { return department; }
    double getOvertime() const { return overtime; }
    void setDepartment(std::string d) { department = d; }
    void setOvertime(double e) { overtime = e; }

    std::string role() const { return "Staff"; }
    // overtime at 1.5x the hourly rate, taking an 8 hour day
    double pay() const { return base + days * rate + overtime * (rate / 8.0) * 1.5; }
    void show() const;
};


class Patient : public Person {
    std::string illness;
    std::string bloodGroup;
    int doctorId;        // id of the assigned doctor, 0 if none

public:
    Patient() { doctorId = 0; }
    Patient(int i, std::string n, int a, char s, std::string p)
        : Person(i, n, a, s, p) { doctorId = 0; }

    std::string getIllness() const { return illness; }
    std::string getBloodGroup() const { return bloodGroup; }
    int getDoctorId() const { return doctorId; }
    void setIllness(std::string i) { illness = i; }
    void setBloodGroup(std::string b) { bloodGroup = b; }
    void setDoctorId(int d) { doctorId = d; }

    std::string role() const { return "Patient"; }
    void show() const;
};


struct Appt {
    int id;
    int patientId;
    int doctorId;
    std::string date;   // yyyy-mm-dd
    int start;          // minutes past midnight
    int minutes;
    std::string status; // Booked / Done / Cancelled

    Appt() { id = 0; patientId = 0; doctorId = 0; start = 0; minutes = 30; status = "Booked"; }

    int end() const { return start + minutes; }

    bool clashes(const Appt& o) const {
        if (date != o.date) return false;
        return start < o.end() && o.start < end();
    }
};

#endif
