/* ER Command Center - ULTIMATE MASTER VERSION (Realistic Patient Linking) */
/* File Name: hospital_gui.cpp */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

using namespace std;

// ==========================================
// GLOBAL: Times New Roman Font
// ==========================================
Font tnrFont;

// Drop-in replacement for DrawText() using Times New Roman.
// Safe to call even if font failed to load (falls back to default).
void DrawTNR(const char *text, int posX, int posY, int fontSize, Color color) {
    if (tnrFont.texture.id != 0)
        DrawTextEx(tnrFont, text, {(float)posX, (float)posY}, (float)fontSize, 1.0f, color);
    else
        DrawText(text, posX, posY, fontSize, color);
}

// ==========================================
// 1. CORE DATA STRUCTURES & RESOURCES
// ==========================================
struct Patient {
    int id;
    string name;
    int age;
    string symptoms;
    int heartRate, spO2, bpSystolic;
    time_t arrivalTime;
    float triageScore;
    string status; // "Waiting", "In Treatment", "Discharged"
    time_t treatmentStartTime;
};

struct User { string username, password, role; int patientID; };

vector<Patient> patients;
vector<User> users;
int nextId = 1001;

// Resource Management
int totalBeds = 10;
int occupiedBeds = 0;
int totalDoctors = 5;
int availableDoctors = 5;

// Statistics & Analytics
double totalWaitTimeCrit = 0, totalWaitTimeUrg = 0, totalWaitTimeNorm = 0;
int countCrit = 0, countUrg = 0, countNorm = 0;
int patientsTreatedCount = 0;
double longestWaitMinutes = 0;
int hourlyArrivals[24] = {0};

bool simActive = false;
float simTimer = 0;

// ==========================================
// 2. LOGIC & SIMULATION ENGINES
// ==========================================
string getPriorityLabel(float score) {
    if (score >= 30) return "Critical";
    if (score >= 15) return "Urgent";
    return "Normal";
}

Color getScoreColor(float score) {
    if (score >= 30) return MAROON;
    if (score >= 15) return ORANGE;
    return DARKGRAY;
}

void generateRandomPatient() {
    Patient p;
    p.id = nextId++;
    string names[] = {"Lina", "Mark", "Sarah", "John", "Amira", "Youssef", "Elena", "David"};
    p.name = names[rand() % 8] + " (Sim)";
    p.age = 1 + rand() % 90;
    p.symptoms = "Simulated Case";
    p.heartRate = 60 + rand() % 70;
    p.spO2 = 85 + rand() % 15;
    p.bpSystolic = 90 + rand() % 100;
    p.arrivalTime = time(0);
    p.status = "Waiting";
    p.triageScore = 0;
    time_t now = time(0);
    tm *ltm = localtime(&now);
    hourlyArrivals[ltm->tm_hour]++;
    patients.push_back(p);
}

void updateSystem() {
    time_t now = time(0);
    for (auto &p : patients) {
        if (p.status == "Waiting") {
            float score = 0;
            if (p.heartRate > 120 || p.heartRate < 50) score += 15;
            else if (p.heartRate > 100) score += 5;
            if (p.spO2 < 90) score += 25;
            else if (p.spO2 < 95) score += 10;
            if (p.bpSystolic > 180 || p.bpSystolic < 90) score += 15;
            if (p.age < 5 || p.age > 65) score += 5;
            string symp = p.symptoms;
            if (symp.find("chest") != string::npos || symp.find("breath") != string::npos || symp.find("heart") != string::npos) score += 20;
            double waitSecs = difftime(now, p.arrivalTime);
            score += (waitSecs / 60.0);
            p.triageScore = score;
            if (waitSecs / 60.0 > longestWaitMinutes) longestWaitMinutes = waitSecs / 60.0;
        }
        if (p.status == "In Treatment") {
            if (difftime(now, p.treatmentStartTime) > 30) {
                p.status = "Discharged";
                occupiedBeds--;
                availableDoctors++;
                patientsTreatedCount++;
            }
        }
    }
}

int getSmartWaitTime(float patientScore) {
    double avgTimeMinutes = 10.0;
    int totalTreated = countCrit + countUrg + countNorm;
    if (totalTreated > 0)
        avgTimeMinutes = (totalWaitTimeCrit + totalWaitTimeUrg + totalWaitTimeNorm) / totalTreated;
    int peopleAhead = 0;
    for (size_t i = 0; i < patients.size(); i++)
        if (patients[i].status == "Waiting" && patients[i].triageScore >= patientScore) peopleAhead++;
    return peopleAhead * avgTimeMinutes;
}

void exportToCSV() {
    ofstream file("shift_report.csv");
    file << "Report Generated," << time(0) << "\n";
    file << "Metric,Value\n";
    file << "Total Patients Treated," << patientsTreatedCount << "\n";
    file << "Longest Wait (min)," << longestWaitMinutes << "\n";
    file << "Avg Wait Critical (min)," << (countCrit > 0 ? totalWaitTimeCrit/countCrit : 0) << "\n";
    file << "Avg Wait Urgent (min),"   << (countUrg  > 0 ? totalWaitTimeUrg/countUrg   : 0) << "\n";
    file << "Avg Wait Normal (min),"   << (countNorm > 0 ? totalWaitTimeNorm/countNorm : 0) << "\n";
    file.close();
}

// ==========================================
// 3. FILE I/O
// ==========================================
void loadData() {
    ifstream sFile("stats.txt");
    if (sFile.is_open()) {
        sFile >> nextId >> patientsTreatedCount >> longestWaitMinutes
              >> totalWaitTimeCrit >> countCrit >> totalWaitTimeUrg >> countUrg
              >> totalWaitTimeNorm >> countNorm;
        for (int i = 0; i < 24; i++) sFile >> hourlyArrivals[i];
        sFile.close();
    }

    ifstream pFile("patients.txt");
    string line;
    occupiedBeds = 0; availableDoctors = totalDoctors;
    while (getline(pFile, line)) {
        stringstream ss(line); Patient p; string tempStr;
        getline(ss, tempStr, ','); p.id = atoi(tempStr.c_str());
        getline(ss, p.name, ',');
        getline(ss, tempStr, ','); p.age = atoi(tempStr.c_str());
        getline(ss, p.symptoms, ',');
        getline(ss, tempStr, ','); p.heartRate = atoi(tempStr.c_str());
        getline(ss, tempStr, ','); p.spO2 = atoi(tempStr.c_str());
        getline(ss, tempStr, ','); p.bpSystolic = atoi(tempStr.c_str());
        getline(ss, tempStr, ','); p.arrivalTime = (time_t)atol(tempStr.c_str());
        getline(ss, tempStr, ','); p.triageScore = atof(tempStr.c_str());
        getline(ss, p.status, ',');
        getline(ss, tempStr, ','); p.treatmentStartTime = (time_t)atol(tempStr.c_str());
        patients.push_back(p);
        if (p.status == "In Treatment") { occupiedBeds++; availableDoctors--; }
    }

    ifstream uFile("users.txt");
    if (!uFile.is_open() || uFile.peek() == std::ifstream::traits_type::eof()) {
        User d; d.username = "doctor"; d.password = "doc123"; d.role = "doctor"; d.patientID = -1; users.push_back(d);
        User n; n.username = "nurse";  n.password = "nurse123"; n.role = "nurse"; n.patientID = -1; users.push_back(n);
    } else {
        while (getline(uFile, line)) {
            stringstream ss(line); User u; string pIDStr;
            getline(ss, u.username, ','); getline(ss, u.password, ',');
            getline(ss, u.role, ',');     getline(ss, pIDStr, ',');
            if (!pIDStr.empty()) u.patientID = atoi(pIDStr.c_str());
            users.push_back(u);
        }
    }
}

void saveData() {
    ofstream pFile("patients.txt");
    for (size_t i = 0; i < patients.size(); i++) {
        pFile << patients[i].id << "," << patients[i].name << "," << patients[i].age << ","
              << patients[i].symptoms << "," << patients[i].heartRate << ","
              << patients[i].spO2 << "," << patients[i].bpSystolic << ","
              << patients[i].arrivalTime << "," << patients[i].triageScore << ","
              << patients[i].status << "," << patients[i].treatmentStartTime << "\n";
    }
    pFile.close();

    ofstream uFile("users.txt");
    for (size_t i = 0; i < users.size(); i++)
        uFile << users[i].username << "," << users[i].password << ","
              << users[i].role << "," << users[i].patientID << "\n";
    uFile.close();

    ofstream sFile("stats.txt");
    sFile << nextId << "\n" << patientsTreatedCount << "\n" << longestWaitMinutes << "\n"
          << totalWaitTimeCrit << "\n" << countCrit << "\n" << totalWaitTimeUrg << "\n"
          << countUrg << "\n" << totalWaitTimeNorm << "\n" << countNorm << "\n";
    for (int i = 0; i < 24; i++) sFile << hourlyArrivals[i] << " ";
    sFile.close();
}

// ==========================================
// 4. UI COMPONENTS
// ==========================================

// -----------------------------------------------------------
// DrawPasswordBox
//   showPassword = false  ->  mask characters with '*'  (default)
//   showPassword = true   ->  show characters in plain text
// -----------------------------------------------------------
void DrawPasswordBox(Rectangle bounds, string &password, bool &editMode, bool showPassword = false) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        editMode = CheckCollisionPointRec(GetMousePosition(), bounds);

    if (editMode) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (password.length() < 63))
                password += (char)key;
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !password.empty()) password.pop_back();
    }

    Color borderColor = editMode ? (Color){0, 121, 241, 255} : DARKGRAY;
    Color bgColor     = editMode ? RAYWHITE : (Color){245, 245, 245, 255};
    DrawRectangleRec(bounds, bgColor);
    DrawRectangleLinesEx(bounds, 1, borderColor);

    string display = showPassword ? password : string(password.length(), '*');
    if (editMode && ((int)(GetTime() * 2) % 2 == 0)) display += "|";
    DrawTNR(display.c_str(), (int)bounds.x + 8, (int)bounds.y + 6, 20, DARKGRAY);
}

// -----------------------------------------------------------
// DrawEyeButton
//   Draws a small square button with an eye icon.
//   isVisible = true  -> eye open  (password is shown)
//   isVisible = false -> eye with a red slash (password hidden)
//   Returns true on the frame the user clicks it.
// -----------------------------------------------------------
bool DrawEyeButton(Rectangle bounds, bool isVisible) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);

    Color bg = hover ? (Color){200, 220, 255, 255} : (Color){235, 235, 235, 255};
    DrawRectangleRec(bounds, bg);
    DrawRectangleLinesEx(bounds, 1, DARKGRAY);

    int cx = (int)(bounds.x + bounds.width  / 2.0f);
    int cy = (int)(bounds.y + bounds.height / 2.0f);

    // Eye outline (ellipse)
    DrawEllipse(cx, cy, 9, 6, WHITE);
    DrawEllipseLines(cx, cy, 9, 6, DARKGRAY);
    // Pupil
    DrawCircle(cx, cy, 3, DARKBLUE);
    DrawCircle(cx - 1, cy - 1, 1, WHITE); // highlight dot

    if (!isVisible) {
        // Red diagonal slash = password hidden
        DrawLineEx({bounds.x + 4,              bounds.y + bounds.height - 4},
                   {bounds.x + bounds.width - 4, bounds.y + 4},
                   2.0f, RED);
    }

    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void DrawStatBar(int x, int y, string label, float value, float maxVal, Color color, string suffix) {
    DrawTNR(label.c_str(), x, y, 16, DARKGRAY);
    DrawRectangle(x + 100, y, 150, 18, LIGHTGRAY);
    float fillPct = value / maxVal;
    if (fillPct > 1.0f) fillPct = 1.0f;
    if (fillPct < 0.0f) fillPct = 0.0f;
    DrawRectangle(x + 100, y, (int)(150 * fillPct), 18, color);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.1f %s", value, suffix.c_str());
    DrawTNR(buffer, x + 260, y, 16, BLACK);
}

void DrawLiveStatsPanel(int x, int y) {
    GuiPanel((Rectangle){(float)x, (float)y, 330, 480}, "LIVE FLOOR OVERVIEW");

    DrawTNR(TextFormat("BEDS: %d/%d", totalBeds - occupiedBeds, totalBeds),
            x+15, y+40, 18, (occupiedBeds >= totalBeds) ? RED : DARKGREEN);
    DrawTNR(TextFormat("DOCTORS: %d/%d", availableDoctors, totalDoctors),
            x+160, y+40, 18, (availableDoctors == 0) ? RED : DARKBLUE);
    DrawLine(x+15, y+65, x+315, y+65, LIGHTGRAY);

    int critW = 0, urgW = 0, normW = 0;
    for (size_t i = 0; i < patients.size(); i++) {
        if (patients[i].status == "Waiting") {
            if      (patients[i].triageScore >= 30) critW++;
            else if (patients[i].triageScore >= 15) urgW++;
            else                                    normW++;
        }
    }

    DrawTNR("Current Queue Depth", x+15, y+80, 18, BLACK);
    DrawStatBar(x+15, y+110, "Critical", critW, 10.0f, MAROON,   "pts");
    DrawStatBar(x+15, y+140, "Urgent",   urgW,  10.0f, ORANGE,   "pts");
    DrawStatBar(x+15, y+170, "Normal",   normW, 10.0f, DARKGRAY, "pts");
    DrawLine(x+15, y+200, x+315, y+200, LIGHTGRAY);

    DrawTNR("Avg Wait Time (Treated)", x+15, y+215, 18, BLACK);
    float avgC = (countCrit > 0) ? (totalWaitTimeCrit / countCrit) : 0;
    float avgU = (countUrg  > 0) ? (totalWaitTimeUrg  / countUrg)  : 0;
    float avgN = (countNorm > 0) ? (totalWaitTimeNorm / countNorm) : 0;
    DrawStatBar(x+15, y+245, "Critical", avgC, 60.0f,  MAROON,   "min");
    DrawStatBar(x+15, y+275, "Urgent",   avgU, 120.0f, ORANGE,   "min");
    DrawStatBar(x+15, y+305, "Normal",   avgN, 240.0f, DARKGRAY, "min");
    DrawLine(x+15, y+335, x+315, y+335, LIGHTGRAY);

    DrawTNR("Efficiency Metrics", x+15, y+350, 18, BLACK);
    DrawTNR(TextFormat("Total Treated: %d",      patientsTreatedCount), x+15, y+380, 18, DARKGRAY);
    DrawTNR(TextFormat("Longest Wait: %.1f min", longestWaitMinutes),   x+15, y+410, 18, DARKGRAY);
}

// ==========================================
// 5. MAIN LOOP & SCREENS
// ==========================================
enum AppScreen {
    MAIN_MENU, LOGIN_SCREEN, DOCTOR_DASHBOARD,
    NURSE_DASHBOARD, PATIENT_DASHBOARD, REGISTER_ACCOUNT, NURSE_REGISTER_PATIENT
};

int main() {
    InitWindow(800, 600, "Hospital ER Queue System - GUI Edition");
    SetTargetFPS(60);

    // ----------------------------------------------------------
    // Load Times New Roman (32 pt) for all text in the app.
    // Searches the standard Windows Fonts directory first,
    // then a common Linux path.  Falls back to raylib default
    // automatically if neither is found (via DrawTNR wrapper).
    // ----------------------------------------------------------
    tnrFont = LoadFontEx("C:/Windows/Fonts/times.ttf", 32, NULL, 0);
    if (tnrFont.texture.id == 0)
        tnrFont = LoadFontEx("/usr/share/fonts/truetype/msttcorefonts/Times_New_Roman.ttf", 32, NULL, 0);

    if (tnrFont.texture.id != 0) {
        GuiSetFont(tnrFont);
        GuiSetStyle(DEFAULT, TEXT_SIZE, 18);
    }

    loadData();

    AppScreen currentScreen    = MAIN_MENU;
    char   loginUser[64]       = "\0"; bool loginUserEdit = false;
    string loginPass           = "";   bool loginPassEdit = false;
    bool   showLoginPass       = false;               // eye-toggle – login
    string loginErrorMsg       = "";
    int    currentPatientViewID = -1;

    char   accUser[64]         = "\0"; bool accUserEdit      = false;
    string accPass             = "";   bool accPassEdit      = false;
    bool   showRegPass         = false;               // eye-toggle – register
    char   accPatientIDStr[16] = "\0"; bool accPatientIDEdit = false;
    int    accRoleActive       = 0;    string accStatusMsg   = "";

    char patName[128]     = "\0"; bool nameEdit  = false;
    char patSymptoms[256] = "\0"; bool sympEdit  = false;
    int  patAge  = 30; bool ageEdit  = false;
    int  patHR   = 80; bool hrEdit   = false;
    int  patSpO2 = 98; bool spo2Edit = false;
    int  patBP   = 120; bool bpEdit  = false;
    string patStatusMsg = "";

    while (!WindowShouldClose()) {

        // Background simulation
        if (simActive) {
            simTimer += GetFrameTime();
            if (simTimer > 5.0f) { generateRandomPatient(); simTimer = 0; }
        }
        updateSystem();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // -------------------------------------------------------
        // SCREEN: MAIN MENU
        // -------------------------------------------------------
        if (currentScreen == MAIN_MENU) {
            DrawTNR("HOSPITAL ER COMMAND CENTER", 220, 80, 24, DARKBLUE);
            DrawTNR(TextFormat("Total Patients in System: %d", (int)patients.size()), 300, 120, 16, DARKGRAY);

            if (GuiButton((Rectangle){300, 160, 200, 50}, "Login to System"))     currentScreen = LOGIN_SCREEN;
            if (GuiButton((Rectangle){300, 230, 200, 50}, "Register New Account")) currentScreen = REGISTER_ACCOUNT;

            DrawRectangle(250, 320, 300, 240, Fade(LIGHTGRAY, 0.5f));
            DrawTNR("PRO DEMO CONTROLS", 310, 335, 18, MAROON);

            if (GuiButton((Rectangle){280, 370, 240, 40}, "1. Inject 10 Patients")) {
                for (int i = 0; i < 10; i++) generateRandomPatient(); saveData();
            }
            if (GuiButton((Rectangle){280, 420, 240, 40}, "2. Time Warp (+1 Hr)")) {
                for (auto &p : patients) if (p.status == "Waiting") p.arrivalTime -= 3600;
                saveData();
            }
            if (GuiButton((Rectangle){280, 470, 240, 40}, "3. Factory Reset DB")) {
                patients.clear(); nextId = 1001; patientsTreatedCount = 0;
                totalWaitTimeCrit = 0; countCrit = 0;
                totalWaitTimeUrg  = 0; countUrg  = 0;
                totalWaitTimeNorm = 0; countNorm = 0;
                longestWaitMinutes = 0;
                for (int i = 0; i < 24; i++) hourlyArrivals[i] = 0;
                occupiedBeds = 0; availableDoctors = totalDoctors;
                remove("patients.txt"); remove("stats.txt");
            }
        }

        // -------------------------------------------------------
        // SCREEN: LOGIN
        // -------------------------------------------------------
        else if (currentScreen == LOGIN_SCREEN) {
            DrawRectangle(0, 0, 800, 60, DARKBLUE);
            DrawTNR("System Login", 20, 15, 30, WHITE);
            DrawTNR("Username:", 250, 200, 20, DARKGRAY);
            if (GuiTextBox((Rectangle){360, 195, 200, 30}, loginUser, 64, loginUserEdit))
                loginUserEdit = !loginUserEdit;

            DrawTNR("Password:", 250, 250, 20, DARKGRAY);
            // Password box (width = 164) + eye button (width = 32) = 196 px total
            DrawPasswordBox((Rectangle){360, 245, 164, 30}, loginPass, loginPassEdit, showLoginPass);
            if (DrawEyeButton((Rectangle){528, 245, 32, 30}, showLoginPass))
                showLoginPass = !showLoginPass;

            DrawTNR(loginErrorMsg.c_str(), 250, 300, 18, MAROON);

            if (GuiButton((Rectangle){250, 350, 140, 40}, "Login")) {
                bool found = false;
                for (size_t i = 0; i < users.size(); i++) {
                    if (users[i].username == string(loginUser) && users[i].password == loginPass) {
                        found = true;
                        loginErrorMsg = ""; loginPass = ""; showLoginPass = false;
                        if      (users[i].role == "doctor")  currentScreen = DOCTOR_DASHBOARD;
                        else if (users[i].role == "nurse")   currentScreen = NURSE_DASHBOARD;
                        else if (users[i].role == "patient") {
                            currentPatientViewID = users[i].patientID;
                            currentScreen = PATIENT_DASHBOARD;
                        }
                        break;
                    }
                }
                if (!found) loginErrorMsg = "Invalid username or password!";
            }
            if (GuiButton((Rectangle){420, 350, 140, 40}, "Back to Menu")) {
                currentScreen = MAIN_MENU;
                loginErrorMsg = ""; loginUser[0] = '\0'; loginPass = ""; showLoginPass = false;
            }
        }

        // -------------------------------------------------------
        // SCREENS: DOCTOR & NURSE DASHBOARDS
        // -------------------------------------------------------
        else if (currentScreen == DOCTOR_DASHBOARD || currentScreen == NURSE_DASHBOARD) {
            Color  headColor = (currentScreen == DOCTOR_DASHBOARD) ? DARKBLUE : DARKGREEN;
            string headText  = (currentScreen == DOCTOR_DASHBOARD) ? "Doctor Command Center" : "Nurse Station";

            DrawRectangle(0, 0, 800, 60, headColor);
            DrawTNR(headText.c_str(), 20, 15, 30, WHITE);
            DrawRectangle(0, 60, 200, 540, LIGHTGRAY);

            if (currentScreen == DOCTOR_DASHBOARD) {
                if (GuiButton((Rectangle){20, 80, 160, 40}, "Match & Treat Next")) {
                    int bestIdx = -1; float maxS = -1;
                    for (int i = 0; i < (int)patients.size(); i++) {
                        if (patients[i].status == "Waiting" && patients[i].triageScore > maxS) {
                            maxS = patients[i].triageScore; bestIdx = i;
                        }
                    }
                    if (bestIdx != -1 && availableDoctors > 0 && occupiedBeds < totalBeds) {
                        Patient &p = patients[bestIdx];
                        double wait = difftime(time(0), p.arrivalTime) / 60.0;
                        if      (p.triageScore >= 30) { totalWaitTimeCrit += wait; countCrit++; }
                        else if (p.triageScore >= 15) { totalWaitTimeUrg  += wait; countUrg++;  }
                        else                          { totalWaitTimeNorm += wait; countNorm++; }
                        p.status = "In Treatment";
                        p.treatmentStartTime = time(0);
                        availableDoctors--; occupiedBeds++;
                        saveData();
                    }
                }
                if (GuiButton((Rectangle){20, 140, 160, 40}, simActive ? "STOP SIM" : "START SIM"))
                    simActive = !simActive;
                if (GuiButton((Rectangle){20, 200, 160, 40}, "Export Shift CSV"))
                    exportToCSV();
            } else {
                if (GuiButton((Rectangle){20, 100, 160, 40}, "Intake New Patient"))
                    currentScreen = NURSE_REGISTER_PATIENT;
            }

            if (GuiButton((Rectangle){20, 520, 160, 40}, "Logout")) {
                simActive = false; currentScreen = MAIN_MENU;
            }

            DrawTNR("Live Patient Queue:", 220, 80, 20, BLACK);
            int yPos = 120; bool anyoneWaiting = false;
            for (size_t i = 0; i < patients.size(); i++) {
                if (patients[i].status == "Waiting") {
                    anyoneWaiting = true;
                    char buffer[256];
                    snprintf(buffer, sizeof(buffer), "ID: %d | %s | %.1f",
                             patients[i].id, patients[i].name.c_str(), patients[i].triageScore);
                    DrawTNR(buffer, 220, yPos, 18, getScoreColor(patients[i].triageScore));
                    yPos += 30;
                }
            }
            if (!anyoneWaiting) DrawTNR("Empty Queue", 220, 120, 18, DARKGREEN);

            DrawLiveStatsPanel(450, 75);
        }

        // -------------------------------------------------------
        // SCREEN: NURSE INTAKE
        // -------------------------------------------------------
        else if (currentScreen == NURSE_REGISTER_PATIENT) {
            DrawRectangle(0, 0, 800, 60, DARKGREEN);
            DrawTNR("Medical Intake Form", 20, 15, 30, WHITE);

            DrawTNR("Patient Name:", 150, 100, 20, DARKGRAY);
            if (GuiTextBox((Rectangle){300, 95, 250, 30}, patName, 128, nameEdit)) nameEdit = !nameEdit;
            DrawTNR("Symptoms:", 150, 140, 20, DARKGRAY);
            if (GuiTextBox((Rectangle){300, 135, 250, 30}, patSymptoms, 256, sympEdit)) sympEdit = !sympEdit;
            DrawTNR("Age:", 150, 180, 20, DARKGRAY);
            if (GuiSpinner((Rectangle){300, 175, 120, 30}, NULL, &patAge, 0, 120, ageEdit)) ageEdit = !ageEdit;
            DrawTNR("Heart Rate:", 150, 220, 20, DARKGRAY);
            if (GuiSpinner((Rectangle){300, 215, 120, 30}, NULL, &patHR, 0, 300, hrEdit)) hrEdit = !hrEdit;
            DrawTNR("SpO2 %:", 150, 260, 20, DARKGRAY);
            if (GuiSpinner((Rectangle){300, 255, 120, 30}, NULL, &patSpO2, 0, 100, spo2Edit)) spo2Edit = !spo2Edit;
            DrawTNR("Systolic BP:", 150, 300, 20, DARKGRAY);
            if (GuiSpinner((Rectangle){300, 295, 120, 30}, NULL, &patBP, 0, 300, bpEdit)) bpEdit = !bpEdit;

            Color msgColor = (patStatusMsg.find("Success") != string::npos) ? DARKGREEN : MAROON;
            DrawTNR(patStatusMsg.c_str(), 150, 350, 18, msgColor);

            if (GuiButton((Rectangle){200, 400, 140, 40}, "Add to Queue")) {
                string n(patName), s(patSymptoms);
                if (n.empty() || s.empty()) {
                    patStatusMsg = "Error: Name and Symptoms required!";
                } else {
                    Patient newP;
                    int generatedID = nextId++;
                    newP.id = generatedID;
                    newP.name = n; newP.age = patAge; newP.symptoms = s;
                    newP.heartRate = patHR; newP.spO2 = patSpO2; newP.bpSystolic = patBP;
                    newP.arrivalTime = time(0); newP.triageScore = 0; newP.status = "Waiting";
                    patients.push_back(newP); saveData();
                    patStatusMsg = "Success! Give Patient ID: " + to_string(generatedID) + " to the patient.";
                    patName[0] = '\0'; patSymptoms[0] = '\0';
                }
            }
            if (GuiButton((Rectangle){360, 400, 140, 40}, "Back to Dashboard")) {
                currentScreen = NURSE_DASHBOARD; patStatusMsg = "";
            }
        }

        // -------------------------------------------------------
        // SCREEN: REGISTER ACCOUNT
        // -------------------------------------------------------
        else if (currentScreen == REGISTER_ACCOUNT) {
            DrawRectangle(0, 0, 800, 60, DARKBLUE);
            DrawTNR("Register System Account", 20, 15, 30, WHITE);

            DrawTNR("Username:", 200, 160, 20, DARKGRAY);
            if (GuiTextBox((Rectangle){320, 155, 250, 30}, accUser, 64, accUserEdit))
                accUserEdit = !accUserEdit;

            DrawTNR("Password:", 200, 200, 20, DARKGRAY);
            // Password box (width = 214) + eye button (width = 32) = 246 px total
            DrawPasswordBox((Rectangle){320, 195, 214, 30}, accPass, accPassEdit, showRegPass);
            if (DrawEyeButton((Rectangle){538, 195, 32, 30}, showRegPass))
                showRegPass = !showRegPass;

            DrawTNR("Select Role:", 200, 240, 20, DARKGRAY);
            GuiToggleGroup((Rectangle){320, 235, 80, 30}, "Patient;Nurse;Doctor", &accRoleActive);

            if (accRoleActive == 0) {
                DrawTNR("Your Patient ID:", 200, 280, 20, DARKGRAY);
                if (GuiTextBox((Rectangle){340, 275, 120, 30}, accPatientIDStr, 16, accPatientIDEdit))
                    accPatientIDEdit = !accPatientIDEdit;
                DrawTNR("(Provided by Nurse at Intake)", 470, 280, 16, GRAY);
            }

            Color msgColor = (accStatusMsg.find("Success") != string::npos) ? DARKGREEN : MAROON;
            DrawTNR(accStatusMsg.c_str(), 200, 320, 18, msgColor);

            if (GuiButton((Rectangle){250, 360, 140, 40}, "Create Account")) {
                string u(accUser);
                if (u.empty() || accPass.empty()) {
                    accStatusMsg = "Error: Username and Password required!";
                } else {
                    bool exists = false;
                    for (size_t i = 0; i < users.size(); i++)
                        if (users[i].username == u) exists = true;

                    if (exists) {
                        accStatusMsg = "Error: Username is already taken!";
                    } else {
                        User newU; newU.username = u; newU.password = accPass;

                        if (accRoleActive == 0) {
                            newU.role = "patient";
                            int enteredID = atoi(accPatientIDStr);
                            bool idExistsInDB    = false;
                            bool idAlreadyClaimed = false;
                            for (auto &p : patients) if (p.id == enteredID) idExistsInDB = true;
                            for (auto &u : users)    if (u.patientID == enteredID) idAlreadyClaimed = true;

                            if (!idExistsInDB) {
                                accStatusMsg = "Error: ID not found. See Nurse for Intake.";
                            } else if (idAlreadyClaimed) {
                                accStatusMsg = "Error: An account already exists for that ID.";
                            } else {
                                newU.patientID = enteredID;
                                users.push_back(newU); saveData();
                                accStatusMsg = "Success! Account Created.";
                                accUser[0] = '\0'; accPass = ""; accPatientIDStr[0] = '\0';
                                showRegPass = false;
                            }
                        } else {
                            newU.role = (accRoleActive == 1) ? "nurse" : "doctor";
                            newU.patientID = -1;
                            users.push_back(newU); saveData();
                            accStatusMsg = "Success! Account Created.";
                            accUser[0] = '\0'; accPass = ""; showRegPass = false;
                        }
                    }
                }
            }
            if (GuiButton((Rectangle){410, 360, 140, 40}, "Back to Menu")) {
                currentScreen = MAIN_MENU; accStatusMsg = "";
                accUser[0] = '\0'; accPass = ""; accPatientIDStr[0] = '\0'; showRegPass = false;
            }
        }

        // -------------------------------------------------------
        // SCREEN: PATIENT DASHBOARD
        // -------------------------------------------------------
        else if (currentScreen == PATIENT_DASHBOARD) {
            DrawRectangle(0, 0, 800, 60, SKYBLUE);
            DrawTNR("Patient Portal", 20, 15, 30, WHITE);

            Patient p; bool pFound = false;
            for (size_t i = 0; i < patients.size(); i++) {
                if (patients[i].id == currentPatientViewID) { p = patients[i]; pFound = true; break; }
            }

            if (pFound) {
                DrawTNR(TextFormat("Welcome, %s!", p.name.c_str()),               200, 120, 24, DARKBLUE);
                DrawTNR(TextFormat("Patient ID: %d", p.id),                       200, 160, 20, DARKGRAY);
                DrawTNR(TextFormat("Current Triage Score: %.1f", p.triageScore),  200, 190, 20, getScoreColor(p.triageScore));
                DrawTNR(TextFormat("Current Status: %s", p.status.c_str()),       200, 220, 20, DARKGRAY);

                if (p.status == "Waiting") {
                    DrawTNR(TextFormat("Estimated Wait Time: ~%d minutes", getSmartWaitTime(p.triageScore)),
                            200, 280, 22, MAROON);
                } else if (p.status == "In Treatment") {
                    DrawTNR("You are currently with a Doctor.", 200, 280, 22, ORANGE);
                } else {
                    DrawTNR("You have been treated! You can head home.", 200, 280, 22, DARKGREEN);
                }
            } else {
                DrawTNR("Please see the nurse for intake registration.", 200, 150, 20, DARKGRAY);
            }

            if (GuiButton((Rectangle){300, 400, 200, 50}, "Log Out")) {
                currentScreen = MAIN_MENU; currentPatientViewID = -1;
            }
        }

        EndDrawing();
    }

    saveData();
    if (tnrFont.texture.id != 0) UnloadFont(tnrFont);
    CloseWindow();
    return 0;
}