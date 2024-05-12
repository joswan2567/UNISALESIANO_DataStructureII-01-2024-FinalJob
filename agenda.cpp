/*
* Projeto 2: Sistema de Gerenciamento de Calendário
* Crie um sistema que permita aos usuários agendar eventos em um calendário.
* Use set ou unordered_set para armazenar os eventos de forma ordenada ou desordenada, respectivamente.
* Implemente recursos como notificações de eventos, repetição de eventos e visualização de agenda.
*/
#include "agenda.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <vector>
//#include <unordered_set>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

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
        tm evDate;
        event_repetions event_rep;

        Event(string n, event_repetions rep)
        {
            name = n;
            event_rep = rep;
        }

        json toJson() const
        {
            json j;
            j["name"] = name;
            j["date"] = evDate.tm_hour;
            j["event_rep"] = event_rep;
            return j;
        }
};

class User
{
    public:
        string name;
        string pass;
        // unordered_set<Event, EventHash> events;
        vector<Event> events;

    User(){}

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
        vector<User> Users;

    Settings(){}

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

Settings SETTINGS;

void LoadUsers()
{
    ifstream file("settings.json");
    json jsonSettings;
    file >> jsonSettings;
    file.close();

    for (const auto& user : jsonSettings["Users"])
    {
        User newUser = User(user["name"], user["pass"]); 

        for (const auto& event : jsonSettings["events"])
        {
            // user.events.insert(event["name"], event["event_rep"]);
            newUser.events.push_back({event["name"], event["event_rep"]});
        }
        SETTINGS.Users.push_back(newUser);
        cout << newUser.toJson().dump(4) << endl;
    }
    
    cout << SETTINGS.toJson().dump(4) << endl;

}


void SaveUser(User &user)
{
    ofstream file("settings.json");

    SETTINGS.Users.push_back(user);

    file << SETTINGS.toJson().dump(4);

    file.close();
}

bool AddUser()
{
    User newUser;
    char opc;
    cout << "Insert your name(Ex.: Zeh Linguiça):";
    cin >> newUser.name;

    cout << "Insert your pass(Ex.: strongpass):";
    cin >> newUser.pass;
    
    cout << "==> New user added!! Do you like add new event? (y/n)";
    cin >> opc;

    if(opc == 'Y' || opc == 'y')
    {

    }
    else if(opc == 'N' || opc == 'n')
    {

    }

    SaveUser(newUser);
}

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

    if(isLogged)
    {

    }
}

int main()
{
    User user;

    user.name = "Jose";
    user.pass = "123456789";
    user.events.push_back(Event("Teste01", EVENT_REP_DAILY));
    user.events.push_back(Event("Teste02", EVENT_REP_DAILY));

    SaveUser(user);

    user.name = "Pedro";
    user.pass = "123456789";
    user.events.push_back(Event("Teste03", EVENT_REP_DAILY));
    user.events.push_back(Event("Teste04", EVENT_REP_DAILY));

    cout << user.toJson().dump(4) << endl;

    SaveUser(user);

    LoadUsers();

    return 0;
}