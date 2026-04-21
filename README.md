/*
 Hospital Emergency Room Queue System
 */

#include <iostream>
#include <string>
using namespace std;

const int MAX_PATIENTS = 100;

struct Patient 
{
    int    id;
    string name;
    int    age;
    string symptoms;
    int    priority;  
    string status;   
};

Patient patients[MAX_PATIENTS];
int     patientCount = 0;
int     nextId       = 1001;


string getPriorityLabel(int p) 
{
    if (p == 1) return "Critical";
    if (p == 2) return "Urgent";
    return "Normal";
}

void printLine() 
{
    cout << "----------------------------------------" << endl;
}

int findPatient(int id) {
    for (int i = 0; i < patientCount; i++) 
    {
        if (patients[i].id == id)
            return i;
    }
    return -1;
}

// Returns "doctor", "nurse", "patient", or "fail"
string login() {
    string username, password;
    cout << "Username: "; cin >> username;
    cout << "Password: "; cin >> password;

    if (username == "doctor"  && password == "doc123")   return "doctor";
    if (username == "nurse"   && password == "nurse123") return "nurse";
    if (username == "patient" && password == "pat123")   return "patient";

    return "fail";
}

void registerPatient() 
{
    if (patientCount >= MAX_PATIENTS) 
    {
        cout << "Queue is full!" << endl;
        return;
    }

    Patient p;
    p.id     = nextId++;
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

    patients[patientCount] = p;
    patientCount++;

    cout << "Patient registered! ID: " << p.id << endl;
}

void updatePatient() 
{
    int id;
    cout << "Enter patient ID to update: ";
    cin >> id;

    int i = findPatient(id);
    if (i == -1) {
        cout << "Patient not found." << endl;
        return;
    }

    cout << "Current name    : " << patients[i].name     << endl;
    cout << "Current symptoms: " << patients[i].symptoms << endl;
    cout << "Current priority: " << getPriorityLabel(patients[i].priority) << endl;

    cout << "New priority (1=Critical, 2=Urgent, 3=Normal): ";
    int p; cin >> p;
    if (p >= 1 && p <= 3)
        patients[i].priority = p;

    cout << "Patient updated!" << endl;
}

void removePatient() {
    int id;
    cout << "Enter patient ID to remove: ";
    cin >> id;

    int i = findPatient(id);
    if (i == -1) {
        cout << "Patient not found." << endl;
        return;
    }

    // Shift array left to fill the gap
    for (int j = i; j < patientCount - 1; j++)
        patients[j] = patients[j + 1];

    patientCount--;
    cout << "Patient removed from queue." << endl;
}

void viewAllPatients() {
    if (patientCount == 0) {
        cout << "No patients in queue." << endl;
        return;
    }

    cout << "ID    Name            Age  Priority  Status" << endl;
    printLine();
    for (int i = 0; i < patientCount; i++) {
        cout << patients[i].id       << "   "
             << patients[i].name     << "\t\t"
             << patients[i].age      << "   "
             << getPriorityLabel(patients[i].priority) << "\t"
             << patients[i].status   << endl;
    }
}

void treatNextPatient() {
    int bestIndex = -1;

    for (int i = 0; i < patientCount; i++) {
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
    cout << "Now treating: " << patients[bestIndex].name
         << " (ID: " << patients[bestIndex].id << ")"
         << " - " << getPriorityLabel(patients[bestIndex].priority) << endl;
    cout << "Symptoms: " << patients[bestIndex].symptoms << endl;
}

void searchPatient() 
{
    int id;
    cout << "Enter patient ID: ";
    cin >> id;

    int i = findPatient(id);
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

void viewWaitingTimes() 
{
    if (patientCount == 0) {
        cout << "No patients in queue." << endl;
        return;
    }
    cout << "ID    Name            Priority  Est. Wait" << endl;
    printLine();
    for (int i = 0; i < patientCount; i++) {
        if (patients[i].status == "Waiting") {
            int wait = patients[i].priority * 10; // Simple estimate in minutes
            cout << patients[i].id   << "   "
                 << patients[i].name << "\t\t"
                 << getPriorityLabel(patients[i].priority) << "\t"
                 << wait << " min" << endl;
        }
    }
}

void viewMyStatus() {
    int id;
    cout << "Enter your patient ID: ";
    cin >> id;

    int i = findPatient(id);
    if (i == -1) {
        cout << "ID not found." << endl;
        return;
    }

    int position = 1;
    for (int j = 0; j < i; j++) {
        if (patients[j].status == "Waiting")
            position++;
    }

    printLine();
    cout << "Name          : " << patients[i].name   << endl;
    cout << "Status        : " << patients[i].status << endl;
    cout << "Priority      : " << getPriorityLabel(patients[i].priority) << endl;
    if (patients[i].status == "Waiting") {
        cout << "Queue position: #" << position << endl;
        cout << "Est. wait time: ~" << patients[i].priority * 10 << " min" << endl;
    }
    printLine();
}


void doctorMenu() {
    int choice;
    do {
        cout << "\n=== DOCTOR MENU ===" << endl;
        cout << "1. View All Patients"    << endl;
        cout << "2. Treat Next Patient"   << endl;
        cout << "3. Search Patient by ID" << endl;
        cout << "4. View Waiting Times"   << endl;
        cout << "0. Logout"               << endl;
        cout << "Choice: ";
        cin  >> choice;

        if      (choice == 1) viewAllPatients();
        else if (choice == 2) treatNextPatient();
        else if (choice == 3) searchPatient();
        else if (choice == 4) viewWaitingTimes();
        else if (choice == 0) cout << "Logged out." << endl;
        else                  cout << "Invalid choice." << endl;

    } while (choice != 0);
}

void nurseMenu() {
    int choice;
    do {
        cout << "\n=== NURSE MENU ===" << endl;
        cout << "1. Register New Patient" << endl;
        cout << "2. Update Patient"       << endl;
        cout << "3. Remove Patient"       << endl;
        cout << "4. View All Patients"    << endl;
        cout << "0. Logout"               << endl;
        cout << "Choice: ";
        cin  >> choice;

        if      (choice == 1) registerPatient();
        else if (choice == 2) updatePatient();
        else if (choice == 3) removePatient();
        else if (choice == 4) viewAllPatients();
        else if (choice == 0) cout << "Logged out." << endl;
        else                  cout << "Invalid choice." << endl;

    } while (choice != 0);
}

void patientMenu() {
    int choice;
    do {
        cout << "\n=== PATIENT MENU ===" << endl;
        cout << "1. View My Status" << endl;
        cout << "0. Logout"         << endl;
        cout << "Choice: ";
        cin  >> choice;

        if      (choice == 1) viewMyStatus();
        else if (choice == 0) cout << "Logged out." << endl;
        else                  cout << "Invalid choice." << endl;

    } while (choice != 0);
}

int main() {
    cout << "====================================" << endl;
    cout << "   Hospital ER Queue System"         << endl;
    cout << "====================================" << endl;

    int choice;
    do {
        cout << "\n1. Login" << endl;
        cout << "0. Exit"   << endl;
        cout << "Choice: ";
        cin  >> choice;

        if (choice == 1) {
            string role = login();

            if      (role == "fail")    cout << "Wrong username or password." << endl;
            else if (role == "doctor")  doctorMenu();
            else if (role == "nurse")   nurseMenu();
            else if (role == "patient") patientMenu();
        }

    } while (choice != 0);

    cout << "Goodbye!" << endl;
    return 0;
}
