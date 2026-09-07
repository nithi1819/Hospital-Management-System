#include "hospital.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <sys/stat.h>

using namespace std;

namespace tools {

string trim(string s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

vector<string> split(string s, char sep) {
    vector<string> out;
    string part;
    istringstream in(s);
    while (getline(in, part, sep)) out.push_back(part);
    return out;
}

// read as text and convert, instead of cin >> int, so a typo can't jam the stream.
// atoi returns 0 for junk and gives no way to tell that apart from a real 0,
// so check the characters first and only convert if they are all digits.
int toInt(string s, int fallback) {
    s = trim(s);
    if (s.empty()) return fallback;

    size_t i = 0;
    if (s[0] == '-' || s[0] == '+') i = 1;
    if (i >= s.size()) return fallback;

    for (; i < s.size(); i++)
        if (!isdigit((unsigned char)s[i])) return fallback;

    return atoi(s.c_str());
}

double toDouble(string s, double fallback) {
    s = trim(s);
    if (s.empty()) return fallback;

    size_t i = 0;
    if (s[0] == '-' || s[0] == '+') i = 1;
    if (i >= s.size()) return fallback;

    bool dot = false;
    for (; i < s.size(); i++) {
        if (s[i] == '.') {
            if (dot) return fallback;   // two dots is not a number
            dot = true;
        } else if (!isdigit((unsigned char)s[i])) {
            return fallback;
        }
    }
    return atof(s.c_str());
}

int toMinutes(string hhmm) {
    vector<string> p = split(trim(hhmm), ':');
    if (p.size() != 2) return -1;
    int h = toInt(p[0], -1);
    int m = toInt(p[1], -1);
    if (h < 0 || h > 23 || m < 0 || m > 59) return -1;
    return h * 60 + m;
}

string toClock(int minutes) {
    if (minutes < 0) return "--:--";
    ostringstream o;
    o << setw(2) << setfill('0') << minutes / 60 << ':'
      << setw(2) << setfill('0') << minutes % 60;
    return o.str();
}

bool goodDate(string d) {
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
    for (size_t i = 0; i < d.size(); i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit((unsigned char)d[i])) return false;
    }
    int mo = toInt(d.substr(5, 2));
    int da = toInt(d.substr(8, 2));
    return mo >= 1 && mo <= 12 && da >= 1 && da <= 31;
}

string sanitize(string s) {
    // records are saved pipe separated, so a | in a name would split the row
    for (size_t i = 0; i < s.size(); i++)
        if (s[i] == '|' || s[i] == '\n' || s[i] == '\r') s[i] = ' ';
    return trim(s);
}

} // namespace tools

using namespace tools;

static vector<string> readFile(string path) {
    vector<string> out;
    ifstream in(path.c_str());
    if (!in) return out;
    string line;
    while (getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        out.push_back(line);
    }
    return out;
}

static char normalSex(char c) {
    c = toupper((unsigned char)c);
    if (c == 'M' || c == 'F' || c == 'O') return c;
    return 'U';
}


Hospital::Hospital(string d) {
    dir = d;
    nextDoctorId = nextNurseId = nextStaffId = nextPatientId = nextApptId = 1;
}

bool Hospital::isEmpty() const {
    return doctors.empty() && nurses.empty() && staff.empty() && patients.empty();
}

void Hospital::load() {
    loadDocs();
    loadNurses();
    loadStaff();
    loadPats();
    loadAppts();
}

void Hospital::save() {
    mkdir(dir.c_str(), 0755);
    saveDocs();
    saveNurses();
    saveStaff();
    savePats();
    saveAppts();
}


// ---- doctors ----

void Hospital::loadDocs() {
    doctors.clear();
    nextDoctorId = 1;
    vector<string> lines = readFile(filePath("doctors.txt"));
    for (size_t i = 0; i < lines.size(); i++) {
        vector<string> f = split(lines[i], '|');
        if (f.size() < 11) continue;
        Doctor d(toInt(f[0]), f[1], toInt(f[2]), f[3][0], f[4]);
        d.setSpecialty(f[5]);
        d.setFee(toDouble(f[6]));
        d.setAvailable(f[7] == "1");
        d.setPay(toDouble(f[8]), toInt(f[9]), toDouble(f[10]));
        if (d.getId() >= nextDoctorId) nextDoctorId = d.getId() + 1;
        doctors.push_back(d);
    }
}

void Hospital::saveDocs() {
    ofstream out(filePath("doctors.txt").c_str());
    out << "# id|name|age|sex|phone|specialty|fee|available|base|days|rate\n";
    for (size_t i = 0; i < doctors.size(); i++) {
        const Doctor& d = doctors[i];
        out << d.getId() << '|' << d.getName() << '|' << d.getAge() << '|'
            << d.getSex() << '|' << d.getPhone() << '|' << d.getSpecialty() << '|'
            << d.getFee() << '|' << (d.isAvailable() ? 1 : 0) << '|'
            << d.getBase() << '|' << d.getDays() << '|' << d.getRate() << '\n';
    }
}

int Hospital::addDoctor(string name, int age, char sex, string phone,
                     string specialty, double fee, double base, int days, double rate) {
    name = sanitize(name);
    if (name.empty())        { lastError = "Name is blank."; return 0; }
    if (age < 18 || age > 100) { lastError = "Age must be 18 to 100."; return 0; }
    if (days < 0 || days > 31) { lastError = "Days must be 0 to 31."; return 0; }
    if (fee < 0 || base < 0 || rate < 0) { lastError = "Money cannot be negative."; return 0; }

    Doctor d(nextDoctorId, name, age, normalSex(sex), sanitize(phone));
    d.setSpecialty(sanitize(specialty));
    d.setFee(fee);
    d.setPay(base, days, rate);
    doctors.push_back(d);
    nextDoctorId++;
    return d.getId();
}

Doctor* Hospital::findDoctor(int id) {
    for (size_t i = 0; i < doctors.size(); i++)
        if (doctors[i].getId() == id) return &doctors[i];
    return 0;
}

bool Hospital::removeDoctor(int id) {
    if (findDoctor(id) == 0) { lastError = "No doctor with that id."; return false; }

    // don't leave appointments pointing at a doctor who is gone
    for (size_t i = 0; i < appointments.size(); i++)
        if (appointments[i].doctorId == id && appointments[i].status == "Booked") {
            lastError = "That doctor still has booked appointments.";
            return false;
        }
    for (size_t i = 0; i < patients.size(); i++)
        if (patients[i].getDoctorId() == id) patients[i].setDoctorId(0);

    for (size_t i = 0; i < doctors.size(); i++)
        if (doctors[i].getId() == id) { doctors.erase(doctors.begin() + i); return true; }
    return false;
}


// ---- nurses ----

void Hospital::loadNurses() {
    nurses.clear();
    nextNurseId = 1;
    vector<string> lines = readFile(filePath("nurses.txt"));
    for (size_t i = 0; i < lines.size(); i++) {
        vector<string> f = split(lines[i], '|');
        if (f.size() < 10) continue;
        Nurse n(toInt(f[0]), f[1], toInt(f[2]), f[3][0], f[4]);
        n.setWard(f[5]);
        n.setNightShifts(toInt(f[6]));
        n.setPay(toDouble(f[7]), toInt(f[8]), toDouble(f[9]));
        if (n.getId() >= nextNurseId) nextNurseId = n.getId() + 1;
        nurses.push_back(n);
    }
}

void Hospital::saveNurses() {
    ofstream out(filePath("nurses.txt").c_str());
    out << "# id|name|age|sex|phone|ward|nightShifts|base|days|rate\n";
    for (size_t i = 0; i < nurses.size(); i++) {
        const Nurse& n = nurses[i];
        out << n.getId() << '|' << n.getName() << '|' << n.getAge() << '|'
            << n.getSex() << '|' << n.getPhone() << '|' << n.getWard() << '|'
            << n.getNightShifts() << '|' << n.getBase() << '|' << n.getDays() << '|'
            << n.getRate() << '\n';
    }
}

int Hospital::addNurse(string name, int age, char sex, string phone,
                       string ward, int nightShifts, double base, int days, double rate) {
    name = sanitize(name);
    if (name.empty())          { lastError = "Name is blank."; return 0; }
    if (age < 18 || age > 100) { lastError = "Age must be 18 to 100."; return 0; }
    if (days < 0 || days > 31) { lastError = "Days must be 0 to 31."; return 0; }
    if (nightShifts < 0 || nightShifts > days) { lastError = "Nights cannot be more than days worked."; return 0; }

    Nurse n(nextNurseId, name, age, normalSex(sex), sanitize(phone));
    n.setWard(sanitize(ward));
    n.setNightShifts(nightShifts);
    n.setPay(base, days, rate);
    nurses.push_back(n);
    nextNurseId++;
    return n.getId();
}

Nurse* Hospital::findNurse(int id) {
    for (size_t i = 0; i < nurses.size(); i++)
        if (nurses[i].getId() == id) return &nurses[i];
    return 0;
}

bool Hospital::removeNurse(int id) {
    for (size_t i = 0; i < nurses.size(); i++)
        if (nurses[i].getId() == id) { nurses.erase(nurses.begin() + i); return true; }
    lastError = "No nurse with that id.";
    return false;
}


// ---- staff ----

void Hospital::loadStaff() {
    staff.clear();
    nextStaffId = 1;
    vector<string> lines = readFile(filePath("staff.txt"));
    for (size_t i = 0; i < lines.size(); i++) {
        vector<string> f = split(lines[i], '|');
        if (f.size() < 10) continue;
        Staff s(toInt(f[0]), f[1], toInt(f[2]), f[3][0], f[4]);
        s.setDepartment(f[5]);
        s.setOvertime(toDouble(f[6]));
        s.setPay(toDouble(f[7]), toInt(f[8]), toDouble(f[9]));
        if (s.getId() >= nextStaffId) nextStaffId = s.getId() + 1;
        staff.push_back(s);
    }
}

void Hospital::saveStaff() {
    ofstream out(filePath("staff.txt").c_str());
    out << "# id|name|age|sex|phone|department|overtime|base|days|rate\n";
    for (size_t i = 0; i < staff.size(); i++) {
        const Staff& s = staff[i];
        out << s.getId() << '|' << s.getName() << '|' << s.getAge() << '|'
            << s.getSex() << '|' << s.getPhone() << '|' << s.getDepartment() << '|'
            << s.getOvertime() << '|' << s.getBase() << '|' << s.getDays() << '|'
            << s.getRate() << '\n';
    }
}

int Hospital::addStaff(string name, int age, char sex, string phone,
                       string department, double overtime, double base, int days, double rate) {
    name = sanitize(name);
    if (name.empty())          { lastError = "Name is blank."; return 0; }
    if (age < 18 || age > 100) { lastError = "Age must be 18 to 100."; return 0; }
    if (days < 0 || days > 31) { lastError = "Days must be 0 to 31."; return 0; }
    if (overtime < 0)             { lastError = "Overtime cannot be negative."; return 0; }

    Staff s(nextStaffId, name, age, normalSex(sex), sanitize(phone));
    s.setDepartment(sanitize(department));
    s.setOvertime(overtime);
    s.setPay(base, days, rate);
    staff.push_back(s);
    nextStaffId++;
    return s.getId();
}

Staff* Hospital::findStaff(int id) {
    for (size_t i = 0; i < staff.size(); i++)
        if (staff[i].getId() == id) return &staff[i];
    return 0;
}

bool Hospital::removeStaff(int id) {
    for (size_t i = 0; i < staff.size(); i++)
        if (staff[i].getId() == id) { staff.erase(staff.begin() + i); return true; }
    lastError = "No staff member with that id.";
    return false;
}


// ---- patients ----

void Hospital::loadPats() {
    patients.clear();
    nextPatientId = 1;
    vector<string> lines = readFile(filePath("patients.txt"));
    for (size_t i = 0; i < lines.size(); i++) {
        vector<string> f = split(lines[i], '|');
        if (f.size() < 8) continue;
        Patient p(toInt(f[0]), f[1], toInt(f[2]), f[3][0], f[4]);
        p.setIllness(f[5]);
        p.setBloodGroup(f[6]);
        p.setDoctorId(toInt(f[7]));
        if (p.getId() >= nextPatientId) nextPatientId = p.getId() + 1;
        patients.push_back(p);
    }
}

void Hospital::savePats() {
    ofstream out(filePath("patients.txt").c_str());
    out << "# id|name|age|sex|phone|illness|bloodGroup|doctorId\n";
    for (size_t i = 0; i < patients.size(); i++) {
        const Patient& p = patients[i];
        out << p.getId() << '|' << p.getName() << '|' << p.getAge() << '|'
            << p.getSex() << '|' << p.getPhone() << '|' << p.getIllness() << '|'
            << p.getBloodGroup() << '|' << p.getDoctorId() << '\n';
    }
}

int Hospital::addPatient(string name, int age, char sex, string phone,
                     string illness, string bloodGroup, int doctorId) {
    name = sanitize(name);
    if (name.empty())         { lastError = "Name is blank."; return 0; }
    if (age < 0 || age > 120) { lastError = "Age must be 0 to 120."; return 0; }
    if (doctorId != 0 && findDoctor(doctorId) == 0) { lastError = "No doctor with that id."; return 0; }

    Patient p(nextPatientId, name, age, normalSex(sex), sanitize(phone));
    p.setIllness(sanitize(illness));
    p.setBloodGroup(sanitize(bloodGroup));
    p.setDoctorId(doctorId);
    patients.push_back(p);
    nextPatientId++;
    return p.getId();
}

Patient* Hospital::findPatient(int id) {
    for (size_t i = 0; i < patients.size(); i++)
        if (patients[i].getId() == id) return &patients[i];
    return 0;
}

bool Hospital::removePatient(int id) {
    if (findPatient(id) == 0) { lastError = "No patient with that id."; return false; }

    for (size_t i = 0; i < appointments.size(); ) {
        if (appointments[i].patientId == id) appointments.erase(appointments.begin() + i);
        else i++;
    }
    for (size_t i = 0; i < patients.size(); i++)
        if (patients[i].getId() == id) { patients.erase(patients.begin() + i); return true; }
    return false;
}


// ---- appointments ----

void Hospital::loadAppts() {
    appointments.clear();
    nextApptId = 1;
    vector<string> lines = readFile(filePath("appointments.txt"));
    for (size_t i = 0; i < lines.size(); i++) {
        vector<string> f = split(lines[i], '|');
        if (f.size() < 7) continue;
        Appt a;
        a.id = toInt(f[0]);
        a.patientId = toInt(f[1]);
        a.doctorId = toInt(f[2]);
        a.date = f[3];
        a.start = toInt(f[4]);
        a.minutes = toInt(f[5], 30);
        a.status = f[6];
        if (a.id >= nextApptId) nextApptId = a.id + 1;
        appointments.push_back(a);
    }
}

void Hospital::saveAppts() {
    ofstream out(filePath("appointments.txt").c_str());
    out << "# id|patientId|doctorId|date|start|minutes|status\n";
    for (size_t i = 0; i < appointments.size(); i++) {
        const Appt& a = appointments[i];
        out << a.id << '|' << a.patientId << '|' << a.doctorId << '|' << a.date << '|'
            << a.start << '|' << a.minutes << '|' << a.status << '\n';
    }
}

int Hospital::book(int patientId, int doctorId, string date, string time, int minutes) {
    Patient* p = findPatient(patientId);
    if (p == 0) { lastError = "No patient with that id."; return 0; }
    Doctor* d = findDoctor(doctorId);
    if (d == 0) { lastError = "No doctor with that id."; return 0; }
    if (!d->isAvailable()) { lastError = "Dr " + d->getName() + " is off duty."; return 0; }
    if (!goodDate(date)) { lastError = "Date must be yyyy-mm-dd."; return 0; }

    int start = toMinutes(time);
    if (start < 0) { lastError = "Time must be hh:mm, 24 hour."; return 0; }
    if (minutes < 5 || minutes > 480) { lastError = "Length must be 5 to 480 minutes."; return 0; }
    if (start + minutes > 24 * 60) { lastError = "That would run past midnight."; return 0; }

    Appt a;
    a.patientId = patientId;
    a.doctorId = doctorId;
    a.date = date;
    a.start = start;
    a.minutes = minutes;

    // the actual rule: no overlap, for the doctor or for the patient.
    // half open interval so 10:00-10:30 and 10:30-11:00 do not count as a clash
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i].status != "Booked") continue;
        if (!a.clashes(appointments[i])) continue;

        if (appointments[i].doctorId == doctorId) {
            lastError = "Dr " + d->getName() + " is already booked "
                + toClock(appointments[i].start) + " to " + toClock(appointments[i].end())
                + " on " + date + ".";
            return 0;
        }
        if (appointments[i].patientId == patientId) {
            lastError = p->getName() + " already has an appointment then.";
            return 0;
        }
    }

    a.id = nextApptId;
    appointments.push_back(a);
    nextApptId++;
    return a.id;
}

bool Hospital::setStatus(int id, string status) {
    if (status != "Booked" && status != "Done" && status != "Cancelled") {
        lastError = "Status must be Booked, Done or Cancelled.";
        return false;
    }
    for (size_t i = 0; i < appointments.size(); i++)
        if (appointments[i].id == id) { appointments[i].status = status; return true; }
    lastError = "No appointment with that id.";
    return false;
}

// walks the 9 to 5 day in steps and keeps the blocks nothing is sitting on
vector<string> Hospital::freeSlots(int doctorId, string date, int minutes) const {
    vector<string> out;
    if (minutes < 5 || !goodDate(date)) return out;

    for (int t = 9 * 60; t + minutes <= 17 * 60; t += minutes) {
        Appt probe;
        probe.date = date;
        probe.start = t;
        probe.minutes = minutes;

        bool taken = false;
        for (size_t i = 0; i < appointments.size(); i++) {
            if (appointments[i].doctorId != doctorId) continue;
            if (appointments[i].status != "Booked") continue;
            if (probe.clashes(appointments[i])) { taken = true; break; }
        }
        if (!taken) out.push_back(toClock(t));
    }
    return out;
}


// ---- reports ----

double Hospital::payroll() const {
    vector<const Employee*> all;
    for (size_t i = 0; i < doctors.size(); i++)   all.push_back(&doctors[i]);
    for (size_t i = 0; i < nurses.size(); i++) all.push_back(&nurses[i]);
    for (size_t i = 0; i < staff.size(); i++)  all.push_back(&staff[i]);

    // three different formulas, one loop, no idea which class is which
    double total = 0;
    for (size_t i = 0; i < all.size(); i++) total += all[i]->pay();
    return total;
}

void Hospital::search(string term) const {
    string want = trim(term);
    for (size_t i = 0; i < want.size(); i++) want[i] = tolower((unsigned char)want[i]);

    vector<const Person*> all;
    for (size_t i = 0; i < doctors.size(); i++)   all.push_back(&doctors[i]);
    for (size_t i = 0; i < nurses.size(); i++) all.push_back(&nurses[i]);
    for (size_t i = 0; i < staff.size(); i++)  all.push_back(&staff[i]);
    for (size_t i = 0; i < patients.size(); i++)   all.push_back(&patients[i]);

    int hits = 0;
    for (size_t i = 0; i < all.size(); i++) {
        string name = all[i]->getName();
        for (size_t k = 0; k < name.size(); k++) name[k] = tolower((unsigned char)name[k]);

        bool match = name.find(want) != string::npos
                  || all[i]->getPhone().find(want) != string::npos;
        if (!match) continue;

        cout << setw(9) << left << all[i]->role();
        all[i]->show();
        cout << "\n";
        hits++;
    }
    if (hits == 0) cout << "Nothing matched.\n";
}


void Hospital::loadSamples() {
    addDoctor("Anita Rao", 44, 'F', "9820011223", "Cardiology", 800, 60000, 22, 1200);
    addDoctor("Vikram Shetty", 51, 'M', "9820011224", "Orthopaedics", 650, 55000, 20, 1100);
    addDoctor("Farah Qureshi", 38, 'F', "9820011225", "Paediatrics", 500, 50000, 24, 1000);

    addNurse("Meera Nair", 29, 'F', "9820022331", "ICU", 8, 28000, 26, 600);
    addNurse("Joseph Dsa", 34, 'M', "9820022332", "General", 4, 26000, 24, 550);

    addStaff("Rakesh Patil", 41, 'M', "9820033441", "Maintenance", 12, 22000, 25, 500);
    addStaff("Sunita Kale", 36, 'F', "9820033442", "Front Desk", 6, 24000, 26, 520);

    addPatient("Imran Sheikh", 62, 'M', "9820044551", "Chest pain", "B+", 1);
    addPatient("Lata Kulkarni", 47, 'F', "9820044552", "Knee pain", "O+", 2);
    addPatient("Aarav Menon", 7, 'M', "9820044553", "Fever", "A+", 3);
    addPatient("Divya Rane", 31, 'F', "9820044554", "Checkup", "AB+", 0);
}
