#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <windows.h>
#include <map>
#include <ctime>

using namespace std;


#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

struct TimetableEntry
{
    string day;
    string city;
    int start_time;
    int end_time;
};

vector<string> get_friends(const char* username)
{
    vector<string> friends;
    ifstream infile("friends.csv");
    string line;

    while (getline(infile, line))
    {
        size_t commaPos = line.find(',');
        string user = line.substr(0, commaPos);
        string friendName = line.substr(commaPos + 1);

        if (user == username)
        {
            friends.push_back(friendName);
        }
    }

    infile.close();
    return friends;
}

vector<TimetableEntry> get_timetable(const char* username)
{
    vector<TimetableEntry> timetable;
    ifstream infile("timetable.csv");
    string line;

    while (getline(infile, line))
    {
        stringstream ss(line);
        string user, day, city, start_str, end_str;
        int start_time, end_time;

        getline(ss, user, ',');
        getline(ss, day, ',');
        getline(ss, city, ',');
        getline(ss, start_str, ',');
        getline(ss, end_str, ',');

        stringstream start_ss(start_str);
        stringstream end_ss(end_str);
        start_ss >> start_time;
        end_ss >> end_time;

        if (user == username)
        {
            timetable.push_back({day, city, start_time, end_time});
        }
    }

    infile.close();
    return timetable;
}

void check_overlap_and_suggest_outing(const char* username)
{
    vector<string> friends = get_friends(username);
    vector<TimetableEntry> user_timetable = get_timetable(username);
    map<string, vector<string> > group_outings;

    for (size_t i = 0; i < friends.size(); ++i)
    {
        vector<TimetableEntry> friend_timetable = get_timetable(friends[i].c_str());

        for (size_t j = 0; j < user_timetable.size(); ++j)
        {
            for (size_t k = 0; k < friend_timetable.size(); ++k)
            {
                if (user_timetable[j].day == friend_timetable[k].day &&
                        user_timetable[j].city == friend_timetable[k].city)
                {
                    int overlap_start = max(user_timetable[j].start_time, friend_timetable[k].start_time);
                    int overlap_end = min(user_timetable[j].end_time, friend_timetable[k].end_time);

                    if (overlap_start < overlap_end)
                    {
                        cout << "You can meet with " << friends[i] << " in " << user_timetable[j].city
                             << " on " << user_timetable[j].day << " from " << overlap_start
                             << " to " << overlap_end << ".\n";
                        stringstream ss_start, ss_end;
                        ss_start << overlap_start;
                        ss_end << overlap_end;
                        string key = user_timetable[j].day + "," + user_timetable[j].city + "," +
                                     ss_start.str() + "," + ss_end.str();
                        group_outings[key].push_back(friends[i]);
                    }
                }
            }
        }
    }
}
int check_user(const char *username, const char *password)
{
    ifstream file("users.csv");
    if (!file.is_open())
    {
        cerr << "Error opening file." << endl;
        return 0;
    }

    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        string user, pass;

        if (getline(ss, user, ',') && getline(ss, pass, ','))
        {
            if (!pass.empty() && pass[pass.size() - 1] == '\n')
            {
                pass.erase(pass.size() - 1);
            }
            if (user == username && pass == password)
            {
                file.close();
                return 1;
            }
        }
    }

    file.close();
    return 0;
}


int is_username_taken(const char *username)
{
    ifstream file("users.csv");
    if (!file.is_open())
    {
        cerr << "Error opening file." << endl;
        return 0;
    }

    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        string user;

        if (getline(ss, user, ','))
        {
            if (user == username)
            {
                file.close();
                return 1;
            }
        }
    }

    file.close();
    return 0;
}

void signup()
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];

    cout << "Enter username: ";
    cin >> username;

    if (is_username_taken(username))
    {
        cout << "Username already taken. Please choose another." << endl;
        return;
    }

    cout << "Enter password: ";
    cin >> password;
    ofstream outfile("users.csv", ios::app);

    if (outfile.is_open())
    {
        outfile << username << "," << password << "\n";
        outfile.close();
        cout << "User created successfully!" << endl;
    }
    else
    {
        cout << "Failed to open the file." << endl;
    }
}
void send_friend_request(const char* from, const char* to)
{
    if (strcmp(from, to) == 0)
    {
        cout << "You cannot send a friend request to yourself." << endl;
        return;
    }

    if (!is_username_taken(to))
    {
        cout << "The user '" << to << "' does not exist." << endl;
        return;
    }

    ofstream outfile("requests.csv", ios::app);

    if (outfile.is_open())
    {
        outfile << from << "," << to << "\n";
        outfile.close();
        cout << "Friend request sent from " << from << " to " << to << "!" << endl;
    }
    else
    {
        cout << "Failed to open the file." << endl;
    }
}
void view_friend_requests(const char* username)
{
    ifstream infile("requests.csv");
    string line;
    vector<string> requests;

    while (getline(infile, line))
    {
        size_t commaPos = line.find(',');
        string from = line.substr(0, commaPos);
        string to = line.substr(commaPos + 1);

        if (to == username)
        {
            requests.push_back(from);
        }
    }

    infile.close();

    if (requests.empty())
    {
        cout << "No friend requests." << endl;
    }
    else
    {
        cout << "Friend requests:" << endl;
        for (size_t i = 0; i < requests.size(); ++i)
        {
            cout << "- " << requests[i] << endl;
        }
    }
}

void respond_friend_request(const char* from, const char* to, bool accept)
{
    ifstream infile("requests.csv");
    ofstream tempFile("temp.csv");
    string line;
    bool request_found = false;

    while (getline(infile, line))
    {
        size_t commaPos = line.find(',');
        string reqFrom = line.substr(0, commaPos);
        string reqTo = line.substr(commaPos + 1);

        if (reqFrom == from && reqTo == to)
        {
            request_found = true;
            if (accept)
            {
                ofstream friendsFile("friends.csv", ios::app);
                if (friendsFile.is_open())
                {
                    friendsFile << from << "," << to << "\n";
                    friendsFile << to << "," << from << "\n";
                    friendsFile.close();
                }
            }
        }
        else
        {
            tempFile << line << "\n";
        }
    }

    infile.close();
    tempFile.close();

    remove("requests.csv");
    rename("temp.csv", "requests.csv");

    if (request_found)
    {
        if (accept)
        {
            cout << "Friend request accepted." << endl;
        }
        else
        {
            cout << "Friend request rejected." << endl;
        }
    }
    else
    {
        cout << "No such friend request found." << endl;
    }
}

void view_friends(const char* username)
{
    ifstream infile("friends.csv");
    string line;
    vector<string> friends;

    while (getline(infile, line))
    {
        size_t commaPos = line.find(',');
        string user = line.substr(0, commaPos);
        string friendName = line.substr(commaPos + 1);

        if (user == username)
        {
            friends.push_back(friendName);
        }
    }

    infile.close();

    if (friends.empty())
    {
        cout << "No friends found." << endl;
    }
    else
    {
        cout << "Friends:" << endl;
        for (size_t i = 0; i < friends.size(); ++i)
        {
            cout << "- " << friends[i] << endl;
        }
    }
}

string get_tomorrow_date() {
    time_t now = time(0);
    tm* local_time = localtime(&now);
    local_time->tm_mday += 1;
    time_t tomorrow_time = mktime(local_time);
    tm* tomorrow_date = localtime(&tomorrow_time);
    int year = 1900 + tomorrow_date->tm_year;
    int month = 1 + tomorrow_date->tm_mon;
    int day = tomorrow_date->tm_mday;
    stringstream ss;
    ss << year << "-" << month << "-" << day;
    return ss.str();
}

void update_timetable(const char *username)
{
    int choice;
    struct TimetableEntry
    {
        string day;
        string city;
        int start_time;
        int end_time;
    };

    cout << "Update timetable for:\n";
    cout << "1. Tomorrow\n";
    cout << "2. This Weekend\n";
    cout << "3. Next Week\n";
    cout << "Enter Choice: ";
    cin >> choice;
    cin.ignore();

    ofstream outfile("timetable.csv", ios::app);
    if (!outfile.is_open())
    {
        cerr << "Error opening file." << endl;
        return;
    }

    switch (choice)
    {
    case 1:
    {
        char city[MAX_USERNAME_LENGTH];
        int start_time, end_time;
        string date= get_tomorrow_date();
        cout << "Enter the city for tomorrow: ";
        cin >> city;
        cout << "Enter start time (10:00-16:00): ";
        cin >> start_time;
        cout << "Enter end time (10:00-16:00): ";
        cin >> end_time;
        outfile << username << ","<< date << "," << city << "," << start_time << "," << end_time << "\n";
        break;
    }
    case 2:
    {
        char city[MAX_USERNAME_LENGTH];
        int start_time, end_time;
        string days[] = {"Saturday", "Sunday"};
        for (int i = 0; i < 2; ++i)
        {
            cout << "Enter the city for " << days[i] << ": ";
            cin >> city;
            cout << "Enter start time (10:00-16:00): ";
            cin >> start_time;
            cout << "Enter end time (10:00-16:00): ";
            cin >> end_time;
            outfile << username << "," << days[i] << "," << city << "," << start_time << "," << end_time << "\n";
        }
        break;
    }
    case 3:
    {
        char city[MAX_USERNAME_LENGTH];
        int start_time, end_time;
        string days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
        for (int i = 0; i < 7; ++i)
        {
            cout << "Enter the city for " << days[i] << ": ";
            cin >> city;
            cout << "Enter start time (10:00-16:00): ";
            cin >> start_time;
            cout << "Enter end time (10:00-16:00): ";
            cin >> end_time;
            outfile << username << "," << days[i] << "," << city << "," << start_time << "," << end_time << "\n";
        }
        break;
    }
    default:
        cout << "Invalid choice." << endl;
        break;
    }

    outfile.close();
    cout << "Timetable updated successfully!" << endl;
}

void menu(const char *username)
{
    int choice;
    while (1)
    {
        cout << "1. Send Friend Request\n";
        cout << "2. View Friend Requests\n";
        cout << "3. View Friends\n";
        cout << "4. Respond to Friend Request\n";
        cout << "5. Update Timetable\n";
        cout << "6. Check Friends\n";
        cout << "7. Log Out\n";
        cout << "Enter Choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice)
        {
        case 1:
        {
            system("cls");
            char to[MAX_USERNAME_LENGTH];
            cout << "Enter username to send request to: ";
            cin >> to;
            send_friend_request(username, to);
            break;
        }
        case 2:
        {
            system("cls");
            view_friend_requests(username);
            break;
        }
        case 3:
        {
            system("cls");
            view_friends(username);
            break;
        }
        case 4:
        {
            char from[MAX_USERNAME_LENGTH];
            int response;
            cout << "Enter username of the friend request to respond to: ";
            cin >> from;
            cout << "Enter 1 to accept or 0 to reject the friend request: ";
            cin >> response;
            respond_friend_request(from, username, response == 1);
            break;
        }
        case 5:
        {
            system("cls");
            update_timetable(username);
            break;
        }
        case 6:
        {
            system("cls");
            check_overlap_and_suggest_outing(username);
            break;
        }
        case 7:
            system("cls");
            cout << "Bye, bye!\n";
            return;
        default:
            cout << "Invalid choice." << endl;
            break;
        }
    }
}

void login()
{
    char username[30];
    char password[30];

    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    if (check_user(username, password))
    {
        cout << "Login successful!" << endl;
        menu(username);
    }
    else
    {
        cout << "Invalid username or password!" << endl;
    }
}

int main()
{
    int choice;
    while (1)
    {
        cout<<"1. Sign Up\n";
        cout<<"2. Login\n";
        cout<<"3. Exit\n";
        printf("Enter your choice: ");
        cin>>choice;
        switch (choice)
        {
        case 1:
            system("cls");
            signup();
            break;
        case 2:
            system("cls");
            login();
            break;
        case 3:
            return 0;
        default:
            printf("Invalid choice. Please enter again.\n");
        }
    }
    return 0;
}
