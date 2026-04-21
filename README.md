/*
CS1 Project
Group 9
*/

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <fstream>
#include <iomanip>
#include <algorithm>

using namespace std;

struct Patient 
{
    int id;
    string name;
    int age;
    string symptoms;
    int priority; // 1 = Critical, 2 = Urgent, 3 = Normal
    string status; // e.g., "Waiting", "Being Treated", "Treated" 

    bool operator>(const Patient& other) const 
    {
        return priority > other.priority;
    }
};

vector<Patient> allPatients;
priority_queue<Patient, vector<Patient>, greater<Patient>> triageQueue;

void mainMenu();
void login();
void doctorDashboard();
void nurseDashboard();
void patientDashboard(int patientID);
void saveToFile();
void loadFromFile();

void saveToFile() 
{
    ofstream outFile("patients.txt");
    for (const auto& p : allPatients) 
    {
        outFile << p.id << "|" << p.name << "|" << p.age << "|" << p.symptoms << "|" 
                << p.priority << "|" << p.status << endl;
    }
    outFile.close();
}

void loadFromFile() 
{
    ifstream inFile("patients.txt");
    if (!inFile) return;

    allPatients.clear();
    while (!triageQueue.empty()) triageQueue.pop();

    string line;
    while (getline(inFile, line)) 
    {
      
    }
    inFile.close();
}


void nurseDashboard() 
{
    int choice;
    do {
        cout << "\n--- Nurse Dashboard ---" << endl;
        cout << "1. Register Patient\n2. Update Patient Information\n3. Remove Patient\n4. Logout" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) 
        {
            Patient p;
            cout << "Enter ID: "; cin >> p.id;
            cout << "Enter Name: "; cin.ignore(); getline(cin, p.name);
            cout << "Enter Age: "; cin >> p.age;
            cout << "Enter Symptoms: "; cin.ignore(); getline(cin, p.symptoms);
            cout << "Enter Priority (1-Critical, 2-Urgent, 3-Normal): "; cin >> p.priority;
            p.status = "Waiting";
            
            allPatients.push_back(p);
            triageQueue.push(p); [cite: 89]
            cout << "Patient registered successfully!" << endl;
        } 
        else if (choice == 3) 
        {
            cout << "Feature: Patient removed from queue." << endl; [cite: 61]
        }
    } while (choice != 4);
}

void doctorDashboard() 
{
    int choice;
    do 
    {
        cout << "\n--- Doctor Dashboard ---" << endl;
        cout << "1. View Patients\n2. Treat Next Patient\n3. View Waiting Time\n4. Logout" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) 
        {
            cout << "\n--- Patient List (Priority Order) ---" << endl;
            priority_queue<Patient, vector<Patient>, greater<Patient>> temp = triageQueue;
            while (!temp.empty()) 
            {
                Patient p = temp.top();
                cout << "ID: " << p.id << " | Name: " << p.name << " | Priority: " << p.priority << endl;
                temp.pop();
            }
        } 
        else if (choice == 2) {
            if (!triageQueue.empty()) 
            {
                Patient p = triageQueue.top();
                triageQueue.pop(); [cite: 42]
                cout << "Treating Patient: " << p.name << " (Priority: " << p.priority << ")" << endl;
            } else 
            {
                cout << "No patients in queue." << endl;
            }
        }
    } while (choice != 4);
}

void patientDashboard(int patientID) 
{
    cout << "\n--- Patient Status ---" << endl;
    bool found = false;
    for (const auto& p : allPatients) 
    {
        if (p.id == patientID) 
        {
            cout << "Name: " << p.name << endl;
            cout << "Status: " << p.status << endl;
            cout << "Priority Level: " << p.priority << endl;
            cout << "Estimated Wait: " << (p.priority * 15) << " minutes." << endl;
            found = true;
            break;
        }
    }
    if (!found) cout << "Patient ID not found." << endl;
}

void login() 
{
    int role;
    cout << "\nSelect Role:\n1. Doctor\n2. Nurse\n3. Patient\nEnter: ";
    cin >> role;

    switch (role) {
        case 1: doctorDashboard(); break; [cite: 54]
        case 2: nurseDashboard(); break; [cite: 58]
        case 3: 
            int id;
            cout << "Enter your Patient ID: "; cin >> id;
            patientDashboard(id); break; [cite: 63]
        default: cout << "Invalid role." << endl;
    }
}

int main() {
    cout << "Welcome to the Hospital Emergency Room Queue System" << endl; [cite: 3]
    while (true) {
        cout << "\n1. Login\n2. Exit\nEnter choice: ";
        int choice;
        cin >> choice;
        if (choice == 1) login();
        else break;
    }
    return 0;
}
