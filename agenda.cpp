#include "agenda.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;
const char* format = "%Y-%m-%d %H:%M:%S";

int resp, login_resp;
string name, pass, passw, confirma, name_event,month, day, year, event_anotation;

void login();
void create_user();
void logged();
void delete_user();
void show_events();
void add_events();
void delete_events();
void byebye();
void logged();

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
    time_t dateObj;
    event_repetions event_rep;

    Event() {}

    Event(string n, event_repetions rep, time_t date)
    {
        name = n;
        event_rep = rep;
        dateObj = date;
    }

    json toJson() const
    {
        json j;
        j["name"] = name;
        j["date"] = dateObj;
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

        for (const auto& event : events)
        {
            j["events"].push_back(event.toJson());
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

        for (const auto& user : Users)
        {
            j["users"].push_back(user.toJson());
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
bool USER_LoadAll(void);

bool EVENT_Create(User& user);
bool EVENT_Delete(User& user);
void EVENT_Print(Event event);
void EVENT_PrintAll(User user);

/*****************************************************************************************************************************/
Settings SETTINGS;

/*
 * USERS FUNCTIONS
 */
bool USER_LoadAll(void)
{
    ifstream file("settings.json");
    if (!file)
    {
        cerr << "Erro ao abrir o arquivo settings.json" << endl;
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
            newUser.events.push_back(Event(event["name"], static_cast<event_repetions>(event["event_rep"]), event["date"]));
        }

        //cout << SETTINGS.toJson().dump(4);
        
        SETTINGS.Users.push_back(newUser);
    }
    
    //cout << SETTINGS.toJson().dump(4);

    return true;
}

bool USER_Delete()
{
    return false;
}

bool USER_Save()
{
    for (int i = 0; i < SETTINGS.Users.size(); i++)
    {
	    //cout<< "Compare: " + user.name + " to " + username << endl;
	    //cout<< "Compare: " + user.pass + " to " + password << endl;
        if(SETTINGS.Users[i].name == SETTINGS.lastUser.name && SETTINGS.Users[i].pass == SETTINGS.lastUser.pass)
        {
            SETTINGS.Users[i] = SETTINGS.lastUser;            
            break;
        }
    }

    ofstream file("settings.json");
    if (!file)
    {
        cerr << "Erro ao abrir o arquivo settings.json para escrita" << endl;
        return false;
    }

    file << SETTINGS.toJson().dump(4);
    file.close();

    return true;
}

bool USER_Add()
{
    User user;
    char opc = 'Y';
    cout << "Insert your name (Ex.: Zeh Linguiça): ";
    cin >> user.name;

    cout << "Insert your pass (Ex.: strongpass): ";
    cin >> user.pass;

    cout << "==> New user created!!" << endl;

    while (opc == 'Y' || opc == 'y')
    {
        cout << "==> Do you like to add a new event? (y/n): ";
        cin >> opc;

        if (opc == 'Y' || opc == 'y')
        {
            EVENT_Create(user);
        }
        else if (opc == 'N' || opc == 'n')
        {
            break;
        }
    }

    SETTINGS.Users.push_back(user);
    USER_Save();
    return true;
}

/*****************************************************************************************************************************/

void Header(bool isLogged)
{
    // - Tela não logado
    //      - Cadastrar
    //      - Logar
    //      - Deletar user

    // - Tela logado
    //      - Adicionar Evento
    //      - Deletar Evento
    //      - Ver Eventos
    //      - Deslogar

    if (isLogged)
    {
        // Implementação da interface quando o usuário está logado
    }
}

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

void EVENT_PrintAll(User user)
{
    cout << "==== " << user.name << " ====" << endl;
    for (const Event& event : user.events)
    {
        EVENT_Print(event);
    }
}

void EVENT_Print(Event event)
{
    cout << "==============================" << endl;
    cout << "Name: " << event.name << endl;
    cout << "Date: " << DateTime(event.dateObj, format) << endl;
    cout << "Rep.: " << EVENT_GetRepStr(event.event_rep) << endl;
    cout << "==============================" << endl;
}

bool EVENT_Create(User& user)
{
    char opc;
    int opcEv;
    Event event;
    string date, hour;

    cout << "Insert event's name (Ex.: Dog's birthday): ";
    cin >> event.name;

    cout << "Insert event's date (Ex.: 2024/05/19): ";
    cin >> date;

    cout << "Insert event's hour (Ex.: 21:05:00): ";
    cin >> hour;

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
            cout << "Invalid option! Setting not repeat for event." << endl;
            event.event_rep = EVENT_REP_NOT;
            break;
        }
    }
    else if (opc == 'N' || opc == 'n')
    {
        event.event_rep = EVENT_REP_NOT;
    }

    date.append(" ");
    date.append(hour);
    event.dateObj = parseDateTime(date, format);

    cout << "==> Event Added!" << endl;
    user.events.push_back(event);

    USER_Save();

    return true;
}

bool EVENT_Delete(User& user)
{
    int opc;
    for (size_t i = 0; i < user.events.size(); ++i)
    {
        cout << i + 1 << ")" << endl;
        EVENT_Print(user.events[i]);
        cout << endl;
    }

    cout << "Which event do you like to delete? (Ex.: 3): ";
    cin >> opc;

    if (opc < 1 || opc > user.events.size())
    {
        cout << "Invalid option!" << endl;
        return false;
    }

    user.events.erase(user.events.begin() + opc - 1);

    USER_Save();

    cout << "==> Event deleted!" << endl;

    return true;
}

/*USER FUNCTIONS**********************************/
//USER NOT LOGGED
void create_user(){
	cout<<("==========================================\n");
	cout<<("WHAT'S YOUR NAME: ");
	cin>>(name);
	cout<<("WHAT'S PASSW (4 DIG): ");
	cin>>(pass);
	cout<<("\n");
	cout<<("==========================================\n");				
	}
	
	
void delete_user(){
	cout<<("==========================================\n");
	cout<<("=====  -------- D-E-L-E-T-E -------- =====\n");
	cout<<("==========================================\n");
	cout<<("=========  THIS FUNCTION WILL    =========\n");
	cout<<("=========  DELETE ALL EVENTS...  =========\n");
	cout<<("=========  ARE YOU SURE? Y/N  =========\n");
	cout<<("==========================================\n");
	cout<<("TYPE Y FOR YES AND N FOR NO:");
	cin>>confirma;
	
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

void login(){
	cout<<("==========================================\n");
	cout<<("WHAT'S YOUR NAME: ");
	cin>>(name);
	cout<<("WHAT'S PASSW (4 DIG): ");
	cin>>(passw);
	cout<<("\n");
	cout<<("==========================================\n");

    if(User_GetUser(SETTINGS.lastUser, name, passw))
    {
	    cout << "==> You are logged!" << endl;
	    logged();
    }
    else
    {
	    cout << "==> Username or password incorrect!" << endl;
    }
}	

//USER ACCOUNT

//ESSA PARTE DEVE MOSTRAR TODOS AS TAREFAS ADICIONADAS NA LISTA
void show_events(){

}


//ESSA PARTE DEVERA SER RESPONSAVEL POR ADICIONAR EVENTO 
void add_events()
{
	cout<<("==========================================\n");
	cout<<("EVENT NAME: ");
	cin>>(name_event);
	cout<<("\n");
	cout<<("NOTES ABOUT EVENT:");
	cin>>(event_anotation);
	cout<<("\n");
	cout<<("DATE: M/D/Y");
	cin>>(month, day, year);
	cout<<("==========================================\n");				
}

//ESSA PARTE DEVERA SER RESPONSAVEL POR DELETAR EVENTO 
void delete_events()
{
	cout<<("==========================================\n");
	cout<<("EVENT NAME: ");
	cin>>(name_event);
	cout<<("\n");
}

//ESSA PARTE DEVERA SER RESPONSAVEL POR DESCONECTAR-SE DA AGENDA 
void byebye(){
	printf("==> SEE YOU LATER ...\n");
}


void logged()
{
    bool disconnect = false;

    while(!disconnect)
    {
        cout<<("==========================================\n");
        cout<<("=====    Y-O-U-R--ARE--L-O-G-G-E-D   =====\n");
        cout<<("==========================================\n");
        cout<<("=========  1 - SHOW EVENTS:      =========\n");
        cout<<("=========  2 - ADD EVENTS:       =========\n");
        cout<<("=========  3 - DELETE EVENTS:    =========\n");
        cout<<("==========================================\n");
        cout<<("=========  4 - DISCONNECT:       =========\n");
        cout<<("==========================================\n");
        cout<<("\n");
        cout<<(" WHAT'S YOUR CHOICE:");
        cin>>(login_resp);

        switch(login_resp){
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
            disconnect = true;
            break;
        }
    }
}

void notLogged()
{
    bool close = false;

    while(!close)
    {
        cout<<("==========================================\n");
        cout<<("=====    Y-O-U-R--NOT--L-O-G-G-E-D   =====\n");
        cout<<("==========================================\n");
        cout<<("=========   1 - CREATE USER:     =========\n");
        cout<<("=========   2 - DELETE USER:     =========\n");
        cout<<("=========   3 - LOGIN:           =========\n");
        cout<<("=========   4 - EXIT:           =========\n");
        cout<<("==========================================\n");
        cout<<("\n");
        cout<<(" WHAT'S YOUR CHOICE:");
        cin>>(resp);
        
        switch(resp){
        case 1:
            //create_user();
            USER_Add();
            break;
        case 2:
            delete_user();
            break;
        case 3: 
            login();
            break;
        case 4:
            byebye();
            close = true;
            break;
        }
    }
}
/*****************************************************************************************************************************/
int main()
{
    USER_LoadAll();

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
