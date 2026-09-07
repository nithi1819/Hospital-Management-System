#include "person.h"
#include <iomanip>

using namespace std;

void Person::show() const {
    cout << left << setw(5) << id << setw(20) << name
         << setw(5) << age << setw(4) << sex << setw(14) << phone;
}

void Employee::show() const {
    Person::show();
    cout << setw(6) << days << setw(12) << fixed << setprecision(2) << pay();
}

void Doctor::show() const {
    Employee::show();
    cout << "  " << specialty;
    if (!available) cout << " (off duty)";
}

void Nurse::show() const {
    Employee::show();
    cout << "  ward " << ward << ", " << nightShifts << " nightShifts";
}

void Staff::show() const {
    Employee::show();
    cout << "  " << department << ", " << overtime << " hrs OT";
}

void Patient::show() const {
    Person::show();
    cout << "  " << setw(18) << illness << setw(6) << bloodGroup;
    if (doctorId == 0) cout << "  no doctor";
    else cout << "  doctor " << doctorId;
}
