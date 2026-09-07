# Hospital Management System


## Files

    person.h / person.cpp    the class hierarchy
    hospital.h / hospital.cpp  record store, scheduling, file read/write
    main.cpp                  menu

## Classes

    Person
      Employee        base salary, days worked, per day rate
        Doctor        specialty, consultation fee, available flag
        Nurse         ward, night shifts
        Staff         department, overtime hours
      Patient         illness, blood group, assigned doctor id

`Employee::pay()` is pure virtual. Doctors get 30% of their consultation fee
on top, nurses get 500 per night shift, staff get overtime at 1.5x the hourly
rate. `Hospital::payroll()` is one loop over `Employee*` that adds them all up
without knowing which class each one is.

## Appointments

An appointment is a start time in minutes past midnight plus a length, so
checking two of them for a clash is integer comparison:

    start < other.end() && other.start < end()
Booking is refused if the doctor is busy, the patient is busy,
or the doctor is marked off duty. `freeSlots()` walks 09:00 to 17:00 and returns
the blocks nothing is sitting on.


## Not done yet

Billing and inventory. Search is a linear scan
