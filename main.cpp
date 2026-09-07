#include <iostream>
#include <iomanip>
#include <string>
#include "hospital.h"

using namespace std;
using namespace tools;

static Hospital hospital;

static void divider(char c = '-') { cout << string(70, c) << "\n"; }

static string ask(string prompt) {
    cout << prompt;
    string s;
    getline(cin, s);
    return trim(s);
}

static int askInt(string prompt, int fallback = 0) {
    return toInt(ask(prompt), fallback);
}

static double askNum(string prompt, double fallback = 0) {
    return toDouble(ask(prompt), fallback);
}

static bool askYes(string prompt) {
    string s = ask(prompt + " (y/n): ");
    return !s.empty() && (s[0] == 'y' || s[0] == 'Y');
}

static char askSex() {
    string s = ask("Sex (M/F/O): ");
    return s.empty() ? 'U' : s[0];
}

static void showError() { cout << "  -> " << hospital.error() << "\n"; }


// ---- listings ----

static void listDoctors() {
    divider();
    if (hospital.allDoctors().empty()) { cout << "No doctors yet.\n"; return; }
    cout << left << setw(5) << "id" << setw(20) << "name" << setw(5) << "age"
         << setw(4) << "sex" << setw(14) << "phone" << setw(6) << "days"
         << setw(12) << "pay" << "\n";
    divider();
    for (size_t i = 0; i < hospital.allDoctors().size(); i++) {
        hospital.allDoctors()[i].show();
        cout << "\n";
    }
    divider();
}

static void listNurses() {
    divider();
    if (hospital.allNurses().empty()) { cout << "No nurses yet.\n"; return; }
    for (size_t i = 0; i < hospital.allNurses().size(); i++) {
        hospital.allNurses()[i].show();
        cout << "\n";
    }
    divider();
}

static void listStaff() {
    divider();
    if (hospital.allStaff().empty()) { cout << "No staff yet.\n"; return; }
    for (size_t i = 0; i < hospital.allStaff().size(); i++) {
        hospital.allStaff()[i].show();
        cout << "\n";
    }
    divider();
}

static void listPatients() {
    divider();
    if (hospital.allPatients().empty()) { cout << "No patients yet.\n"; return; }
    for (size_t i = 0; i < hospital.allPatients().size(); i++) {
        hospital.allPatients()[i].show();
        cout << "\n";
    }
    divider();
}

static void listAppointments() {
    divider();
    if (hospital.allAppointments().empty()) { cout << "No appointments yet.\n"; return; }
    for (size_t i = 0; i < hospital.allAppointments().size(); i++) {
        const Appt& a = hospital.allAppointments()[i];
        Doctor* d = hospital.findDoctor(a.doctorId);
        Patient* p = hospital.findPatient(a.patientId);
        cout << left << setw(5) << a.id << setw(13) << a.date
             << toClock(a.start) << " to " << toClock(a.end()) << "  "
             << setw(18) << (p ? p->getName() : "?")
             << " with " << setw(18) << (d ? d->getName() : "?")
             << a.status << "\n";
    }
    divider();
}


// ---- doctors ----

static void newDoctor() {
    string name = ask("Name: ");
    int age = askInt("Age: ");
    char sex = askSex();
    string phone = ask("Phone: ");
    string specialty = ask("Specialization: ");
    double fee = askNum("Consultation fee: ");
    double base = askNum("Base salary: ");
    int days = askInt("Days worked: ");
    double rate = askNum("Per day rate: ");

    int id = hospital.addDoctor(name, age, sex, phone, specialty, fee, base, days, rate);
    if (id == 0) showError();
    else cout << "  -> Added, id " << id << ".\n";
}

// this is the one that used to make a throwaway Doctor and edit that.
// now it looks the record up and edits the stored one.
static void editDoctor() {
    listDoctors();
    int id = askInt("Doctor id: ");
    Doctor* d = hospital.findDoctor(id);
    if (d == 0) { cout << "  -> No doctor with that id.\n"; return; }

    cout << "Press enter to keep a value.\n";

    string name = ask("Name [" + d->getName() + "]: ");
    if (!name.empty()) d->setName(sanitize(name));

    string phone = ask("Phone [" + d->getPhone() + "]: ");
    if (!phone.empty()) d->setPhone(sanitize(phone));

    string specialty = ask("Specialization [" + d->getSpecialty() + "]: ");
    if (!specialty.empty()) d->setSpecialty(sanitize(specialty));

    double fee = askNum("Fee: ", d->getFee());
    if (fee < 0) { cout << "  -> Fee cannot be negative, keeping the old one.\n"; }
    else d->setFee(fee);

    int days = askInt("Days worked: ", d->getDays());
    if (days < 0 || days > 31) cout << "  -> Days must be 0 to 31, keeping the old value.\n";
    else d->setDays(days);

    d->setAvailable(askYes("Taking appointments?"));
    cout << "  -> Updated.\n";
}

static void doctorMenu() {
    while (true) {
        cout << "\nDoctors\n 1 list  2 add  3 edit  4 remove  0 back\n> ";
        int c = askInt("");
        if (c == 0) return;
        if (c == 1) listDoctors();
        else if (c == 2) newDoctor();
        else if (c == 3) editDoctor();
        else if (c == 4) {
            listDoctors();
            if (!hospital.removeDoctor(askInt("Doctor id to remove: "))) showError();
            else cout << "  -> Removed.\n";
        }
        else cout << "  -> Pick a number from the list.\n";
    }
}


// ---- nurses and staff ----

static void nurseMenu() {
    while (true) {
        cout << "\nNurses\n 1 list  2 add  3 edit  4 remove  0 back\n> ";
        int c = askInt("");
        if (c == 0) return;
        if (c == 1) listNurses();
        else if (c == 2) {
            string name = ask("Name: ");
            int age = askInt("Age: ");
            char sex = askSex();
            string phone = ask("Phone: ");
            string ward = ask("Ward: ");
            int nightShifts = askInt("Night shifts: ");
            double base = askNum("Base salary: ");
            int days = askInt("Days worked: ");
            double rate = askNum("Per day rate: ");
            int id = hospital.addNurse(name, age, sex, phone, ward, nightShifts, base, days, rate);
            if (id == 0) showError();
            else cout << "  -> Added, id " << id << ".\n";
        }
        else if (c == 3) {
            listNurses();
            Nurse* n = hospital.findNurse(askInt("Nurse id: "));
            if (n == 0) { cout << "  -> No nurse with that id.\n"; continue; }
            string name = ask("Name [" + n->getName() + "]: ");
            if (!name.empty()) n->setName(sanitize(name));
            string ward = ask("Ward [" + n->getWard() + "]: ");
            if (!ward.empty()) n->setWard(sanitize(ward));
            int days = askInt("Days worked: ", n->getDays());
            int nightShifts = askInt("Night shifts: ", n->getNightShifts());
            if (days < 0 || days > 31 || nightShifts < 0 || nightShifts > days)
                cout << "  -> Days/nightShifts look wrong, leaving them alone.\n";
            else { n->setDays(days); n->setNightShifts(nightShifts); }
            cout << "  -> Updated.\n";
        }
        else if (c == 4) {
            listNurses();
            if (!hospital.removeNurse(askInt("Nurse id to remove: "))) showError();
            else cout << "  -> Removed.\n";
        }
        else cout << "  -> Pick a number from the list.\n";
    }
}

static void staffMenu() {
    while (true) {
        cout << "\nSupport staff\n 1 list  2 add  3 edit  4 remove  0 back\n> ";
        int c = askInt("");
        if (c == 0) return;
        if (c == 1) listStaff();
        else if (c == 2) {
            string name = ask("Name: ");
            int age = askInt("Age: ");
            char sex = askSex();
            string phone = ask("Phone: ");
            string department = ask("Department: ");
            double overtime = askNum("Overtime hours: ");
            double base = askNum("Base salary: ");
            int days = askInt("Days worked: ");
            double rate = askNum("Per day rate: ");
            int id = hospital.addStaff(name, age, sex, phone, department, overtime, base, days, rate);
            if (id == 0) showError();
            else cout << "  -> Added, id " << id << ".\n";
        }
        else if (c == 3) {
            listStaff();
            Staff* s = hospital.findStaff(askInt("Staff id: "));
            if (s == 0) { cout << "  -> No staff member with that id.\n"; continue; }
            string name = ask("Name [" + s->getName() + "]: ");
            if (!name.empty()) s->setName(sanitize(name));
            string department = ask("Department [" + s->getDepartment() + "]: ");
            if (!department.empty()) s->setDepartment(sanitize(department));
            double overtime = askNum("Overtime hours: ", s->getOvertime());
            if (overtime < 0) cout << "  -> Overtime cannot be negative, leaving it.\n";
            else s->setOvertime(overtime);
            int days = askInt("Days worked: ", s->getDays());
            if (days < 0 || days > 31) cout << "  -> Days must be 0 to 31, leaving it.\n";
            else s->setDays(days);
            cout << "  -> Updated.\n";
        }
        else if (c == 4) {
            listStaff();
            if (!hospital.removeStaff(askInt("Staff id to remove: "))) showError();
            else cout << "  -> Removed.\n";
        }
        else cout << "  -> Pick a number from the list.\n";
    }
}


// ---- patients ----

static void patientMenu() {
    while (true) {
        cout << "\nPatients\n 1 list  2 add  3 edit  4 remove  0 back\n> ";
        int c = askInt("");
        if (c == 0) return;
        if (c == 1) listPatients();
        else if (c == 2) {
            string name = ask("Name: ");
            int age = askInt("Age: ");
            char sex = askSex();
            string phone = ask("Phone: ");
            string illness = ask("Illness: ");
            string bloodGroup = ask("Blood group: ");
            listDoctors();
            int doctorId = askInt("Doctor id (0 for none): ");
            int id = hospital.addPatient(name, age, sex, phone, illness, bloodGroup, doctorId);
            if (id == 0) showError();
            else cout << "  -> Registered, id " << id << ".\n";
        }
        else if (c == 3) {
            listPatients();
            Patient* p = hospital.findPatient(askInt("Patient id: "));
            if (p == 0) { cout << "  -> No patient with that id.\n"; continue; }
            string name = ask("Name [" + p->getName() + "]: ");
            if (!name.empty()) p->setName(sanitize(name));
            string phone = ask("Phone [" + p->getPhone() + "]: ");
            if (!phone.empty()) p->setPhone(sanitize(phone));
            string illness = ask("Illness [" + p->getIllness() + "]: ");
            if (!illness.empty()) p->setIllness(sanitize(illness));
            int doctorId = askInt("Doctor id: ", p->getDoctorId());
            if (doctorId != 0 && hospital.findDoctor(doctorId) == 0) cout << "  -> No such doctor, leaving it.\n";
            else p->setDoctorId(doctorId);
            cout << "  -> Updated.\n";
        }
        else if (c == 4) {
            listPatients();
            if (!hospital.removePatient(askInt("Patient id to remove: "))) showError();
            else cout << "  -> Removed, along with their appointments.\n";
        }
        else cout << "  -> Pick a number from the list.\n";
    }
}


// ---- appointments ----

static void appointmentMenu() {
    while (true) {
        cout << "\nAppointments\n 1 list  2 book  3 free slots  "
                "4 mark done  5 cancel  0 back\n> ";
        int c = askInt("");
        if (c == 0) return;

        if (c == 1) listAppointments();
        else if (c == 2) {
            listPatients();
            int patientId = askInt("Patient id: ");
            listDoctors();
            int doctorId = askInt("Doctor id: ");
            string date = ask("Date (yyyy-mm-dd): ");
            int minutes = askInt("Length in minutes [30]: ", 30);

            vector<string> open = hospital.freeSlots(doctorId, date, minutes);
            if (!open.empty()) {
                cout << "Free: ";
                for (size_t i = 0; i < open.size(); i++) cout << open[i] << " ";
                cout << "\n";
            }
            string time = ask("Start time (hh:mm): ");
            int id = hospital.book(patientId, doctorId, date, time, minutes);
            if (id == 0) showError();
            else cout << "  -> Booked, id " << id << ".\n";
        }
        else if (c == 3) {
            listDoctors();
            int doctorId = askInt("Doctor id: ");
            string date = ask("Date (yyyy-mm-dd): ");
            int minutes = askInt("Slot length [30]: ", 30);
            vector<string> open = hospital.freeSlots(doctorId, date, minutes);
            if (open.empty()) cout << "  -> Nothing available. Check the id and the date.\n";
            else {
                cout << "Free on " << date << ": ";
                for (size_t i = 0; i < open.size(); i++) cout << open[i] << " ";
                cout << "\n";
            }
        }
        else if (c == 4) {
            listAppointments();
            if (!hospital.setStatus(askInt("Appointment id: "), "Done")) showError();
            else cout << "  -> Marked done.\n";
        }
        else if (c == 5) {
            listAppointments();
            if (!hospital.setStatus(askInt("Appointment id: "), "Cancelled")) showError();
            else cout << "  -> Cancelled.\n";
        }
        else cout << "  -> Pick a number from the list.\n";
    }
}


// ---- report ----

static void report() {
    divider('=');
    cout << "Doctors        " << hospital.allDoctors().size() << "\n"
         << "Nurses         " << hospital.allNurses().size() << "\n"
         << "Staff          " << hospital.allStaff().size() << "\n"
         << "Patients       " << hospital.allPatients().size() << "\n"
         << "Appointments   " << hospital.allAppointments().size() << "\n";
    divider();
    cout << "Monthly payroll  " << fixed << setprecision(2) << hospital.payroll() << "\n";
    divider('=');
}


int main() {
    hospital.load();
    if (hospital.isEmpty()) {
        hospital.loadSamples();
        hospital.save();
        cout << "No records found, loaded some sample ones.\n";
    }

    while (true) {
        cout << "\n";
        divider('=');
        cout << "  CITY CARE HOSPITAL\n";
        divider();
        cout << " 1 Doctors      2 Nurses     3 Support staff\n"
                " 4 Patients     5 Appointments\n"
                " 6 Report       7 Search     8 Save\n"
                " 0 Save and quit\n";
        divider('=');

        int c = askInt("> ", -1);

        if (c == 1) doctorMenu();
        else if (c == 2) nurseMenu();
        else if (c == 3) staffMenu();
        else if (c == 4) patientMenu();
        else if (c == 5) appointmentMenu();
        else if (c == 6) report();
        else if (c == 7) hospital.search(ask("Search name or phone: "));
        else if (c == 8) { hospital.save(); cout << "  -> Saved.\n"; }
        else if (c == 0) { hospital.save(); cout << "Saved. Bye.\n"; return 0; }
        else cout << "  -> Pick a number from the list.\n";
    }
}
