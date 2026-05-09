/* Hospital Emergency Room Queue System */
/* cs_project.cpp */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <cstdlib>

using namespace std;

// --- Data Structures ---
struct Patient { 
    int id; 
    string name; 
    int age; 
    string symptoms; 
    int priority;
    string status;
};

struct User {
    string username;
    string password;
    string role;
    int patientID; 
};

// --- Global Variables ---
vector<Patient> patients;
vector<User> users;

int nextId = 1001;
int loggedInPatientID = -1;

double totalTreatmentTimeSeconds = 0.0;
int patientsTreatedCount = 0;

// --- Helper Functions ---
string getPriorityLabel(int p) { 
    if (p == 1) return "Critical"; 
    if (p == 2) return "Urgent"; 
    return "Normal"; 
}

void printLine() { 
    cout << "--------------------------------------------------" << endl; 
}

int findPatientIndex(int id) { 
    for (size_t i = 0; i < patients.size(); i++) { 
        if (patients[i].id == id) return i; 
    } 
    return -1; 
}

// --- Data Persistence ---
void saveData() {
    ofstream pFile("patients.txt");
    for (size_t i = 0; i < patients.size(); i++) {
        pFile << patients[i].id << "," << patients[i].name << "," << patients[i].age << "," 
              << patients[i].symptoms << "," << patients[i].priority << "," << patients[i].status << "\n";
    }
    pFile.close();

    ofstream uFile("users.txt");
    for (size_t i = 0; i < users.size(); i++) {
        uFile << users[i].username << "," << users[i].password << "," << users[i].role << "," << users[i].patientID << "\n";
    }
    uFile.close();

    ofstream sFile("stats.txt");
    sFile << nextId << "\n" << totalTreatmentTimeSeconds << "\n" << patientsTreatedCount << "\n";
    sFile.close();
}

void loadData() {
    ifstream sFile("stats.txt");
    if (sFile.is_open()) {
        sFile >> nextId;
        sFile >> totalTreatmentTimeSeconds;
        sFile >> patientsTreatedCount;
        sFile.close();
    } else {
        User defaultDoc;
        defaultDoc.username = "doctor"; defaultDoc.password = "doc123"; defaultDoc.role = "doctor"; defaultDoc.patientID = -1;
        users.push_back(defaultDoc);

        User defaultNurse;
        defaultNurse.username = "nurse"; defaultNurse.password = "nurse123"; defaultNurse.role = "nurse"; defaultNurse.patientID = -1;
        users.push_back(defaultNurse);
    }

    ifstream uFile("users.txt");
    string line;
    while (getline(uFile, line)) {
        stringstream ss(line);
        User u;
        string pIDStr;
        getline(ss, u.username, ',');
        getline(ss, u.password, ',');
        getline(ss, u.role, ',');
        getline(ss, pIDStr, ',');
        if (!pIDStr.empty()) u.patientID = atoi(pIDStr.c_str());
        users.push_back(u);
    }
    
    ifstream pFile("patients.txt");
    while (getline(pFile, line)) {
        stringstream ss(line);
        Patient p;
        string ageStr, priStr, idStr;
        
        getline(ss, idStr, ','); p.id = atoi(idStr.c_str());
        getline(ss, p.name, ',');
        getline(ss, ageStr, ','); p.age = atoi(ageStr.c_str());
        getline(ss, p.symptoms, ',');
        getline(ss, priStr, ','); p.priority = atoi(priStr.c_str());
        getline(ss, p.status, ',');
        
        patients.push_back(p);
    }
}

// --- Smart Wait Time Logic ---
int getSmartWaitTime(int patientPriority) {
    double avgTimeMinutes = 10.0; 
    if (patientsTreatedCount > 0) {
        avgTimeMinutes = (totalTreatmentTimeSeconds / patientsTreatedCount) / 60.0;
    }
    int peopleAhead = 0;
    for (size_t i = 0; i < patients.size(); i++) {
        if (patients[i].status == "Waiting" && patients[i].priority <= patientPriority) {
            peopleAhead++;
        }
    }
    return peopleAhead * avgTimeMinutes; 
}

// --- User Management ---
void registerAccount() {
    User newUser;
    cout << "\nEnter new username: ";
    cin >> newUser.username;
    
    for (size_t i = 0; i < users.size(); i++) {
        if (users[i].username == newUser.username) {
            cout << "Username already exists. Please try another." << endl;
            return;
        }
    }
    
    cout << "Enter new password: ";
    cin >> newUser.password;
    cout << "Enter role (doctor, nurse, patient): ";
    cin >> newUser.role;
    
    if (newUser.role == "patient") {
        cout << "\n--- Patient Self-Registration ---" << endl;
        newUser.patientID = nextId++;
        
        Patient p;
        p.id = newUser.patientID;
        p.status = "Waiting";
        p.priority = 3; 
        
        cout << "Enter your full name: ";
        cin.ignore();
        getline(cin, p.name);
        cout << "Enter your age: ";
        cin >> p.age;
        cout << "Enter your symptoms: ";
        cin.ignore();
        getline(cin, p.symptoms);
        
        patients.push_back(p);
        cout << "\nRegistration Complete! YOUR ID IS: " << p.id << endl;
        cout << "Please see a nurse to confirm your priority level." << endl;

    } else if (newUser.role == "doctor" || newUser.role == "nurse") {
        newUser.patientID = -1;
    } else {
        cout << "Invalid role. Account creation failed." << endl;
        return;
    }
    
    users.push_back(newUser);
    saveData(); 
    if (newUser.role != "patient") cout << "Account registered successfully!" << endl;
}

string login() { 
    string username, password; 
    cout << "Username: "; 
    cin >> username; 
    cout << "Password: "; 
    cin >> password;

    for (size_t i = 0; i < users.size(); i++) {
        if (users[i].username == username && users[i].password == password) {
            if (users[i].role == "patient") loggedInPatientID = users[i].patientID; 
            return users[i].role;
        }
    }
    return "fail";
}

// --- Nurse Functions ---
void registerPatient() { 
    Patient p;
    p.id = nextId++;
    p.status = "Waiting";

    cout << "Enter patient name: ";
    cin.ignore();
    getline(cin, p.name);
    cout << "Enter age: ";
    cin >> p.age;
    cout << "Enter symptoms: ";
    cin.ignore();
    getline(cin, p.symptoms);
    cout << "Enter priority (1=Critical, 2=Urgent, 3=Normal): ";
    cin >> p.priority;

    while (p.priority < 1 || p.priority > 3) {
        cout << "Invalid! Enter 1, 2, or 3: ";
        cin >> p.priority;
    }

    patients.push_back(p);
    saveData();
    cout << "Patient registered! ID: " << p.id << endl;
}

void updatePatient() { 
    int id; 
    cout << "Enter patient ID to update: "; 
    cin >> id;
    
    int i = findPatientIndex(id);
    if (i == -1) {
        cout << "Patient not found." << endl;
        return;
    }

    cout << "Current priority: " << getPriorityLabel(patients[i].priority) << endl;
    cout << "New priority (1=Critical, 2=Urgent, 3=Normal): ";
    int p; cin >> p;
    if (p >= 1 && p <= 3) {
        patients[i].priority = p;
        saveData();
        cout << "Patient updated!" << endl;
    }
}

void removePatient() { 
    int id; 
    cout << "Enter patient ID to remove: "; 
    cin >> id;
    
    for (size_t i = 0; i < patients.size(); i++) {
        if (patients[i].id == id) {
            patients.erase(patients.begin() + i);
            break;
        }
    }
    
    saveData();
    cout << "Patient removed from queue (if they existed)." << endl;
}

// --- Shared Functions ---
void viewAllPatients() { 
    if (patients.empty()) { 
        cout << "No patients in system." << endl; 
        return; 
    }
    
    cout << "ID    Name            Age  Priority  Status" << endl;
    printLine();
    for (size_t i = 0; i < patients.size(); i++) {
        cout << patients[i].id << "   " << patients[i].name << "\t\t" << patients[i].age << "   "
             << getPriorityLabel(patients[i].priority) << "\t" << patients[i].status << endl;
    }
}

void searchPatient() { 
    int id; 
    cout << "Enter patient ID: "; 
    cin >> id;
    
    int i = findPatientIndex(id);
    if (i == -1) {
        cout << "Patient not found." << endl;
        return;
    }

    printLine();
    cout << "ID      : " << patients[i].id       << endl;
    cout << "Name    : " << patients[i].name     << endl;
    cout << "Age     : " << patients[i].age      << endl;
    cout << "Symptoms: " << patients[i].symptoms << endl;
    cout << "Priority: " << getPriorityLabel(patients[i].priority) << endl;
    cout << "Status  : " << patients[i].status   << endl;
    printLine();
}

void viewWaitingTimes() { 
    bool anyWaiting = false;
    cout << "ID Name Priority Smart Est. Wait" << endl; 
    printLine(); 
    
    for (size_t i = 0; i < patients.size(); i++) {
        if (patients[i].status == "Waiting") { 
            anyWaiting = true;
            cout << patients[i].id << " " << patients[i].name << "\t\t" << getPriorityLabel(patients[i].priority) 
                 << "\t~" << getSmartWaitTime(patients[i].priority) << " min" << endl; 
        } 
    } 
    if (!anyWaiting) cout << "No patients are currently waiting." << endl;
}

// --- Doctor Functions ---
void treatNextPatient() { 
    int bestIndex = -1;
    for (size_t i = 0; i < patients.size(); i++) {
        if (patients[i].status == "Waiting") {
            if (bestIndex == -1 || patients[i].priority < patients[bestIndex].priority)
                bestIndex = i;
        }
    }

    if (bestIndex == -1) {
        cout << "No patients waiting." << endl;
        return;
    }

    patients[bestIndex].status = "Treated";
    cout << "Now treating: " << patients[bestIndex].name << " - " << getPriorityLabel(patients[bestIndex].priority) << endl;
    
    time_t startTime = time(0);
    cout << "\n[Treatment in progress...]" << endl;
    cout << "Press ENTER when treatment is complete...";
    
    cin.ignore(10000, '\n'); 
    cin.get(); 
    
    time_t endTime = time(0);
    double duration = difftime(endTime, startTime); 
    
    totalTreatmentTimeSeconds += duration;
    patientsTreatedCount++;
    saveData(); 
    
    cout << "Treatment complete! Time taken: " << duration << " seconds." << endl;
}

void viewTreatmentStats() {
    printLine();
    cout << "--- TREATMENT STATISTICS ---" << endl;
    cout << "Total Patients Treated : " << patientsTreatedCount << endl;
    
    if (patientsTreatedCount > 0) {
        double avgTime = totalTreatmentTimeSeconds / patientsTreatedCount;
        cout << "Total Treatment Time   : " << totalTreatmentTimeSeconds << " seconds" << endl;
        cout << "Average Treatment Time : " << avgTime << " seconds/patient" << endl;
    } else {
        cout << "No patients treated yet." << endl;
    }
    printLine();
}

// --- Patient Functions ---
void viewMyStatus() { 
    if (loggedInPatientID == -1) {
        cout << "Error: No patient record linked to this account." << endl;
        return;
    }

    int i = findPatientIndex(loggedInPatientID);
    if (i == -1) {
        cout << "ID not found. Please ask a nurse for assistance." << endl;
        return;
    }

    printLine();
    cout << "Name          : " << patients[i].name   << endl;
    cout << "Status        : " << patients[i].status << endl;
    cout << "Priority      : " << getPriorityLabel(patients[i].priority) << endl;
    if (patients[i].status == "Waiting") {
        cout << "Est. wait time: ~" << getSmartWaitTime(patients[i].priority) << " min" << endl;
    }
    printLine();
}

// --- Menus ---
void doctorMenu() { 
    int choice; 
    do { 
        cout << "\n=== DOCTOR MENU ===" << endl; 
        cout << "1. View All Patients\n2. Treat Next Patient\n3. Search Patient by ID\n4. View Waiting Times\n5. View Treatment Statistics\n0. Logout\nChoice: "; 
        cin >> choice;
        if      (choice == 1) viewAllPatients();
        else if (choice == 2) treatNextPatient();
        else if (choice == 3) searchPatient();
        else if (choice == 4) viewWaitingTimes();
        else if (choice == 5) viewTreatmentStats();
    } while (choice != 0);
}

void nurseMenu() { 
    int choice; 
    do { 
        cout << "\n=== NURSE MENU ===" << endl; 
        cout << "1. Register New Patient\n2. Update Patient\n3. Remove Patient\n4. View All Patients\n0. Logout\nChoice: "; 
        cin >> choice;
        if      (choice == 1) registerPatient();
        else if (choice == 2) updatePatient();
        else if (choice == 3) removePatient();
        else if (choice == 4) viewAllPatients();
    } while (choice != 0);
}

void patientMenu() { 
    int choice; 
    do { 
        cout << "\n=== PATIENT MENU ===" << endl; 
        cout << "1. View My Status\n0. Logout\nChoice: "; 
        cin >> choice;
        if (choice == 1) viewMyStatus();
    } while (choice != 0);
}

// --- Main Program ---
int main() { 
    loadData(); 
    
    cout << "====================================" << endl; 
    cout << "      Hospital ER Queue System      " << endl; 
    cout << "====================================" << endl;

    int choice;
    do {
        cout << "\n1. Login\n2. Register New Account\n0. Exit\nChoice: ";
        cin >> choice;

        if (choice == 1) {
            string role = login();
            if      (role == "fail")    cout << "Wrong username or password." << endl;
            else if (role == "doctor")  doctorMenu();
            else if (role == "nurse")   nurseMenu();
            else if (role == "patient") patientMenu();
        } 
        else if (choice == 2) {
            registerAccount();
        }
    } while (choice != 0);

    saveData(); 
    cout << "Goodbye!" << endl;
    return 0;
}
