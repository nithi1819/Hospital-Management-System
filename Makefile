hms: person.cpp hospital.cpp main.cpp person.h hospital.h
	g++ -std=c++11 -Wall person.cpp hospital.cpp main.cpp -o hms

clean:
	rm -f hms
