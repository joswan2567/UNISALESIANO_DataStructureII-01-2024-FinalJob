#include "agenda.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <csignal>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;
const char* format = "%Y-%m-%d %H:%M:%S";
bool isLogged = false;

// int resp, login_resp;
// string name, pass, passw, confirma, name_event,month, day, year, event_anotation;

void USER_Login();
void logged();
void byebye();

/*
 * CLASSES
 */
typedef enum __event_repetions
{
    EVENT_REP_NOT = 0,
    EVENT_REP_DAILY,
    EVENT_REP_WEEKLY,
    EVENT_REP_MONTHLY,

} event_repetions;

class Event
{
public:
    string name;
    string note;
    time_t dateObj;
    event_repetions event_rep;

    Event() {}

    Event(string n, string nt, event_repetions rep, time_t date)
    {
        name = n;
        note = nt;
        event_rep = rep;
        dateObj = date;
    }

    json toJson() const
    {
        json j;
        j["name"] = name;
        j["date"] = dateObj;
        j["note"] = note;
        j["event_rep"] = event_rep;
        return j;
    }
};

class User
{
public:
    string name;
    string pass;
    vector<Event> events;

    User() {}

    User(string n, string p)
    {
        name = n;
        pass = p;
    }

    json toJson() const
    {
        json j;
        j["name"] = name;
        j["pass"] = pass;

        if(events.size() == 0)
        {
            j["events"] = json::array();
        }
        else
        {
            for (const auto& event : events)
            {
                j["events"].push_back(event.toJson());
            }
        }
        return j;
    }
};

class Settings
{
public:
    User lastUser;
    vector<User> Users;

    Settings() {}

    Settings(User user)
    {
        Users.push_back(user);
    }

    json toJson() const
    {
        json j;
        if(Users.size() == 0)
        {
            j["users"] = json::array();
        }
        else
        {
            for (const auto& user : Users)
            {
                j["users"].push_back(user.toJson());
            }
        }
        return j;
    }
};

/*****************************************************************************************************************************/
/*
 * PROTOTYPE FUNCTIONS
 */
bool USER_Add(User& user);
bool USER_Save(void);
bool SETTINGS_LoadUsersAll(void);

bool EVENT_Create(User& user);
bool EVENT_Delete(User& user);
void EVENT_Print(Event event);
void EVENT_PrintAll(User &user);

/*****************************************************************************************************************************/
Settings SETTINGS;

/*
 * USERS FUNCTIONS
 */
void SETTINGS_ClearScreen() 
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
bool SETTINGS_Save()
{
    try
    {
        ofstream file("settings.json");
        if (!file)
        {
            cerr << "\n*** Settings file not founded! \n" << endl;
        }
        else
        {
            //cout << " Saved this in file:" +  SETTINGS.toJson().dump(4);
            file << SETTINGS.toJson().dump(4);
            file.close();
            return true;
        }
    }    
    catch(const std::exception& e)
    {
        cout << "\nErr open settings file: ";
        cout << e.what() << '\n' << endl;
    }
    
    ofstream file("settings.json");
    file << SETTINGS.toJson().dump(4);
    file.close();

    return false;
}

bool SETTINGS_LoadUsersAll(void)
{
    ifstream file("settings.json");
    if (!file)
    {
        cerr << "======================" << endl;
        cerr << "Not founded user file!" << endl;
        cerr << "======================" << endl;
        return false;
    }

    json jsonSettings;
    file >> jsonSettings;
    file.close();

    for (const auto& user : jsonSettings["users"])
    {
        User newUser(user["name"], user["pass"]);

        for (const auto& event : user["events"])
        {
            newUser.events.push_back(Event(event["name"], event["note"], static_cast<event_repetions>(event["event_rep"]), event["date"]));
        }

        //cout << SETTINGS.toJson().dump(4);
        
        SETTINGS.Users.push_back(newUser);
    }
    
    //cout << SETTINGS.toJson().dump(4);

    return true;
}


bool User_GetUser(User &getUser, string username, string password)
{
    for (const auto& user : SETTINGS.Users)
    {
	    //cout<< "Compare: " + user.name + " to " + username << endl;
	    //cout<< "Compare: " + user.pass + " to " + password << endl;
        if(user.name == username && user.pass == password)
        {
            getUser = user;
            return true;
        }
    }

    return false;
} 

bool USER_GetUserByUsername(User &getUser, string username)
{
    for (const auto& user : SETTINGS.Users)
    {
	    //cout<< "Compare: " + user.name + " to " + username << endl;
	    //cout<< "Compare: " + user.pass + " to " + password << endl;
        if(user.name == username)
        {
            getUser = user;
            return true;
        }
    }

    return false;
} 

bool SETTINGS_RemoveUser(User &rmvUser)
{
    for (int i = 0; SETTINGS.Users.size(); i++)
    {
	    //cout<< "Compare: " + user.name + " to " + username << endl;
	    //cout<< "Compare: " + user.pass + " to " + password << endl;
        if(SETTINGS.Users[i].name == rmvUser.name && SETTINGS.Users[i].pass == rmvUser.pass)
        {
            SETTINGS.Users.erase(SETTINGS.Users.begin() + i);
            SETTINGS_Save();
            return true;
        }
    }

    return false;
} 

void USER_Login()
{
    string name, pass;
    cout << "\n====== Login ======\n\n";
	cout<<("Insert username: ");
	cin>>(name);
	cout<<("Insert your password: ");
	cin>>(pass);
	cout<<("\n");
	cout<<("==========================================\n");

    if(User_GetUser(SETTINGS.lastUser, name, pass))
    {
	    cout << "==> You are logged!" << endl;
	    logged();
    }
    else
    {
	    cout << "==> Username or password incorrect!" << endl;
    }
}	


bool USER_Delete()
{
    User user;
    string username, passwd;
    cout << "\n====== User Delete ======\n\n";
	cout<<("Insert username: ");
	cin>>(username);
    if(!USER_GetUserByUsername(user, username))
    {
        cout << "\n*** User not founded!\n" << endl;
        return false;
    }
    cout<< "User founded, When deleting the user, all registered events associated\nwith that user will be lost, continue?\nEnter this user's password to confirm the deletion: ";
    cin>>(passwd);

    if(user.pass == passwd)
    {
        if(SETTINGS_RemoveUser(user))
        {
            cout<< "\n==> User deleted with success!!\n " << endl;
            return true;
        }
        cout<< "\n*** Err in delete user!!\n " << endl;
    }
    else
    {
        cout << "\n*** Password wrong! Is your '" + user.name + "'?\n" << endl;
    }
    return false;
}

bool USER_Save()
{
    for (int i = 0; i < SETTINGS.Users.size(); i++)
    {
	    //cout<< "Compare: " + SETTINGS.Users[i].name + " to " +  SETTINGS.lastUser.name << endl;
	    //cout<< "Compare: " + SETTINGS.Users[i].pass + " to " +  SETTINGS.lastUser.pass << endl;
        if(SETTINGS.Users[i].name == SETTINGS.lastUser.name && SETTINGS.Users[i].pass == SETTINGS.lastUser.pass)
        {
            //cout << "==> Saved with success!\n" << endl;
            SETTINGS.Users[i] = SETTINGS.lastUser;            
            SETTINGS_Save();
            return true;
        }
    }

    cout << "\n ***Not save!\n" << endl;
    return false;
}

bool USER_Add()
{
    cout << "\n====== User Add ======\n\n";
    User user;
    char opc = 'Y';
    cout << "Insert your name (Ex.: Zeh Linguiça): ";
    cin >> user.name;

    cout << "Insert your pass (Ex.: strongpass): ";
    cin >> user.pass;

    cout << "==> New user created!!" << endl;
    SETTINGS.Users.push_back(user);
    SETTINGS.lastUser = user;
    USER_Save();

    while (opc == 'Y' || opc == 'y')
    {
        cout << "==> Do you like to add a new event? (y/n): ";
        cin >> opc;

        if (opc == 'Y' || opc == 'y')
        {
            EVENT_Create(SETTINGS.lastUser);
            USER_Save();
        }
        else if (opc == 'N' || opc == 'n')
        {
            break;
        }
    }

    cout << "==> You are logged!" << endl;
	logged();
    return true;
}

/*****************************************************************************************************************************/
/*
 * EVENTS FUNCTIONS
 */
time_t parseDateTime(const string& datetimeString, const string& format)
{
    struct tm tmStruct = {0};

    if (sscanf(datetimeString.c_str(), "%d/%d/%d %d:%d:%d",
               &tmStruct.tm_year,
               &tmStruct.tm_mon,
               &tmStruct.tm_mday,
               &tmStruct.tm_hour,
               &tmStruct.tm_min,
               &tmStruct.tm_sec) != 6)
    {
        cerr << "Erro ao analisar a data/hora." << endl;
        return -1;
    }

    tmStruct.tm_year -= 1900;
    tmStruct.tm_mon -= 1;

    return mktime(&tmStruct);
}

string DateTime(time_t time, const string& format)
{
    char buffer[90];
    struct tm* timeinfo = localtime(&time);
    strftime(buffer, sizeof(buffer), format.c_str(), timeinfo);
    return string(buffer);
}

string EVENT_GetRepStr(event_repetions eventRep)
{
    switch (eventRep)
    {
    case EVENT_REP_DAILY:
        return "Daily";
    case EVENT_REP_MONTHLY:
        return "Monthly";
    case EVENT_REP_WEEKLY:
        return "Weekly";
    case EVENT_REP_NOT:
        return "Not repeating";
    default:
        return "";
    }
}

void EVENT_PrintAll(User& user)
{
    char opc;
    cout << "\n====== All events ======\n\n";
    cout << "==== " << user.name << "' events ====" << endl;

    if(user.events.size() == 0)
    {
        cout << "\n*** Not founded events! \n" << endl;
        cout << "Do you like ad some?(Y/N): ";
        cin >> opc;

        if(opc == 'Y' || opc == 'y')
        {
            EVENT_Create(user);
            return;
        }
    }

    for (const Event& event : user.events)
    {
        EVENT_Print(event);
    }
}

void EVENT_Print(Event event)
{
    cout << "==============================" << endl;
    cout << "Name: " << event.name << endl;
    cout << "Note: " << event.note << endl;
    cout << "Date: " << DateTime(event.dateObj, format) << endl;
    cout << "Rep.: " << EVENT_GetRepStr(event.event_rep) << endl;
    cout << "==============================" << endl;
}


bool EVENT_isLeapYear(int year)
{
    if (year % 4 != 0) 
    {
        return false;
    } 
    else if (year % 100 != 0) 
    {
        return true;
    } 
    else if (year % 400 != 0) 
    {
        return false;
    } 
    else 
    {
        return true;
    }
}

bool EVENT_IsHourValid(const std::string& timeStr)
{
    int hours, minutes;
    if (timeStr.size() != 5 || timeStr[2] != ':')
    {
        return false;
    }
    
    try 
    {
        hours = std::stoi(timeStr.substr(0, 2));
        minutes = std::stoi(timeStr.substr(3, 2));
    }
    catch (std::exception& e)
    {
        return false;
    }
    
    if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59)
    {
        return false;
    }
    
    return true;
}

bool EVENT_CheckDate(int year, int month, int day)
{
    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    
    if (year < 1 || 
        month < 1 ||
        month > 12 ||
        day < 1) 
    {
        return false;
    }    
    
    if (month == 2 && EVENT_isLeapYear(year)) 
    {
        daysInMonth[1] = 29;
    }
    
    if (day > daysInMonth[month - 1])
    {
        return false;
    }
    
    return true;
}

bool EVENT_IsDateValid(const string dateStr)
{
    int year, month, day;
    if (dateStr.size() != 10 || dateStr[4] != '/' || dateStr[7] != '/')
    {
        return false;
    }
    
    try
    {
        year = stoi(dateStr.substr(0, 4));
        month = stoi(dateStr.substr(5, 2));
        day = stoi(dateStr.substr(8, 2));

        return EVENT_CheckDate(year, month, day);
    } 
    catch (exception& e) 
    {
        cout << e.what() << endl;
        return false;
    }
    
    return true;
}


bool EVENT_Create(User& user)
{
    char opc;
    int opcEv, year, month, day;
    Event event;
    string date, hour;

    cout << "\n====== Create event ======\n\n";
    cout << "Insert event's name (Ex.: Dog's birthday): ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, event.name);
    do
    {        
        cout << "Insert event's date (Frmt.: yyyy/mm/dd | Ex.: 2023/05/15): ";
        cin >> date;

        if(EVENT_IsDateValid(date))
        {
            break;
        }
        else
        {
            printf("\n*** Date not valid!\n");
        }
    } 
    while (true);

    do
    {        
        cout << "Insert event's hour (Frmt.: hh:mm | Ex.: 21:30): ";
        cin >> hour;

        if(EVENT_IsHourValid(hour))
        {
            break;
        }
        else
        {
            printf("\n*** Hour not valid!\n");
        }
    } 
    while (true);

    cout << "This event repeats? (Y/N): ";
    cin >> opc;
    if (opc == 'Y' || opc == 'y')
    {
        cout << "How frequently?" << endl;
        cout << "1 - Daily" << endl;
        cout << "2 - Weekly" << endl;
        cout << "3 - Monthly" << endl;
        cout << "Input an option: ";
        cin >> opcEv;

        switch (opcEv)
        {
        case 1:
            event.event_rep = EVENT_REP_DAILY;
            break;
        case 2:
            event.event_rep = EVENT_REP_WEEKLY;
            break;
        case 3:
            event.event_rep = EVENT_REP_MONTHLY;
            break;
        default:
            cout << "\n*** Invalid option! Setting not repeat for event.\n" << endl;
            event.event_rep = EVENT_REP_NOT;
            break;
        }
    }
    else if (opc == 'N' || opc == 'n')
    {
        event.event_rep = EVENT_REP_NOT;
    }

    cout << "Insert note about event (Ex.: Event in my house): ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, event.note);

    date.append(" " + hour + ":00");
    event.dateObj = parseDateTime(date, format);

    cout << "==> Event Added!" << endl;
    user.events.push_back(event);

    USER_Save();

    return true;
}

bool EVENT_Delete(User& user)
{
    int opc;
    cout << "\n====== Delete event ======\n\n";

    if(user.events.size() == 0)
    {
        cout << "\n*** Not founded events! \n" << endl;
        return false;
    }
    
    for (size_t i = 0; i < user.events.size(); ++i)
    {
        cout << i + 1 << ") ";
        EVENT_Print(user.events[i]);
        cout << endl;
    }

    cout << "Which event do you like to delete? (Ex.: 3): ";
    cin >> opc;

    if (opc < 1 || opc > user.events.size())
    {
        cout << "\n*** Invalid option!\n" << endl;
        return false;
    }

    user.events.erase(user.events.begin() + opc - 1);

    USER_Save();

    cout << "==> Event deleted!" << endl;

    return true;
}

void byebye()
{
	printf("==> SEE YOU LATER ...\n");
    exit(1);
}

void logged()
{
    bool disconnect = false;
    isLogged = true;
    int opc;

    while(!disconnect)
    {
        cout<<("==========================================\n");
        cout<<("===== Welcome " + SETTINGS.lastUser.name +" to your notepad!\n");
        cout<<("==========================================\n");
        cout<<("=========  1 - See all events    =========\n");
        cout<<("=========  2 - Add event         =========\n");
        cout<<("=========  3 - Delete a event    =========\n");
        cout<<("=========  4 - Logout            =========\n");
        cout<<("==========================================\n");
        cout<<("\n");
        cout<<("Do you like to do?(Ex.: 2):");
        cin>>(opc);

        SETTINGS_ClearScreen();

        switch(opc){
        case 1:
            //show_events();
            EVENT_PrintAll(SETTINGS.lastUser);
            break;
        case 2:
            //add_events();
            EVENT_Create(SETTINGS.lastUser);
            break;
        case 3: 
            //delete_events();
            EVENT_Delete(SETTINGS.lastUser);
            break;
        case 4:
            byebye();
            isLogged = false;
            disconnect = true;
            break;
        }
    }
}

void notLogged()
{
    bool close = false;
    int opc;

    while(!close)
    {
        cout<<("==========================================\n");
        cout<<("=====    Welcome to MyEvents 2.0 1!  =====\n");
        cout<<("==========================================\n");
        cout<<("=========   1 - Create user -    =========\n");
        cout<<("=========   2 - Remove user -    =========\n");
        cout<<("=========   3 -   Login     -    =========\n");
        cout<<("=========   4 -   Exit:     -    =========\n");
        cout<<("==========================================\n");
        cout<<("\n");
        cout<<("Do you like today?(Ex. 1):");
        cin>>(opc);
        
        SETTINGS_ClearScreen();
        switch(opc)
        {
            case 1:
                //create_user();
                USER_Add();
                break;
            case 2:
                USER_Delete();
                break;
            case 3: 
                USER_Login();
                break;
            case 4:
                byebye();
                close = true;
                break;
        }
    }
}
void signalHandler(int signum)
{
    std::cout << "\nISR (Ctrl+C). Function Exit..." << std::endl;

    // if(isLogged)
    // {
    //     cout<<("==========================================\n");
    //     cout<<("===== Welcome " + SETTINGS.lastUser.name +" to your notepad!\n");
    //     cout<<("==========================================\n");
    //     cout<<("=========  1 - See all events    =========\n");
    //     cout<<("=========  2 - Add event         =========\n");
    //     cout<<("=========  3 - Delete a event    =========\n");
    //     cout<<("=========  4 - Logout            =========\n");
    //     cout<<("==========================================\n");
    //     cout<<("\n");
    //     cout<<("Do you like to do?(Ex.: 2):");
    // }
    // else
    // {
    //     cout<<("==========================================\n");
    //     cout<<("=====    Welcome to MyEvents 2.0 1!  =====\n");
    //     cout<<("==========================================\n");
    //     cout<<("=========   1 - Create user -    =========\n");
    //     cout<<("=========   2 - Remove user -    =========\n");
    //     cout<<("=========   3 -   Login     -    =========\n");
    //     cout<<("=========   4 -   Exit:     -    =========\n");
    //     cout<<("==========================================\n");
    //     cout<<("\n");
    //     cout<<("Do you like today?(Ex. 1):");
    // }

    exit(1);
}
/*****************************************************************************************************************************/
int main()
{
    signal(SIGINT, signalHandler);

    SETTINGS_LoadUsersAll();

    notLogged();

    /*** TESTE DE IMPLEMENTAÇÂO ***/

    // User user;

    // user.name = "Jose";
    // user.pass = "123456789";
    // user.events.push_back(Event("Teste01", EVENT_REP_DAILY, 123465497));
    // user.events.push_back(Event("Teste02", EVENT_REP_DAILY, 123132465));

    // SETTINGS.Users.push_back(user);
    // USER_Save();

    // user.name = "Pedro";
    // user.pass = "123456789";
    // user.events.push_back(Event("Teste03", EVENT_REP_DAILY, 14487));
    // user.events.push_back(Event("Teste04", EVENT_REP_DAILY, 154678984));

    // SETTINGS.Users.push_back(user);
    // USER_Save();

    // EVENT_Create(user);
    // EVENT_Delete(user);

    // USER_Save();
    /*** TESTE DE IMPLEMENTAÇÂO ***/

    return 0;
}
