#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <windows.h>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <ctime>
#include <set>
#include <algorithm>
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

class Restaurant
{
public:
    Restaurant(const string& name) : name(name) {}
    string getName() const { return name; }
private:
    string name;
};

class User {
public:
    User(const std::string& name) : username(name) {}

    void addFriend(const std::string& friendName) {
        friends.push_back(friendName);
    }

    // Overload the << operator to handle User objects
    friend std::ostream& operator<<(std::ostream& os, const User& user);

private:
    std::string username;
    std::vector<std::string> friends;
};

// Implementation of the overloaded operator
std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& vec) {
    for (size_t i = 0; i < vec.size(); i++) {
        os << "- " << vec[i] << "\n";
    }
    return os;
}

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
            timetable.push_back(TimetableEntry{day, city, start_time, end_time});
        }
    }

    infile.close();
    return timetable;
}

bool is_conflicting(const vector<TimetableEntry>& timetable, const string& day, int start_time, int end_time)
{
    for (size_t i = 0; i < timetable.size(); ++i)
    {
        if (timetable[i].day == day &&
            ((start_time >= timetable[i].start_time && start_time < timetable[i].end_time) ||
             (end_time > timetable[i].start_time && end_time <= timetable[i].end_time) ||
             (start_time <= timetable[i].start_time && end_time >= timetable[i].end_time)))
        {
            return true;
        }
    }
    return false;
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

void view_friends(const char* username) {
    std::ifstream infile("friends.csv");
    if (!infile.is_open()) {
        std::cerr << "Failed to open friends.csv" << std::endl;
        return;
    }

    std::string line;
    std::vector<std::string> friends;
    std::string user(username); // Convert to string for comparison

    while (getline(infile, line)) {
        size_t commaPos = line.find(',');
        if (commaPos == std::string::npos) {
            continue; // Skip malformed lines
        }

        std::string userFromFile = line.substr(0, commaPos);
        std::string friendName = line.substr(commaPos + 1);

        if (userFromFile == user) {
            friends.push_back(friendName);
        }
    }

    infile.close();

    if (friends.empty()) {
        std::cout << "No friends found for " << username << "." << std::endl;
    } else {
        std::cout << "Friends of " << username << ":" << std::endl;
        std::cout << friends; // Utilizing the overloaded operator<<
    }
}

void update_timetable(const char* username)
{
    vector<TimetableEntry> timetable = get_timetable(username);

    cout << "Enter the day: ";
    string day;
    cin >> day;

    cout << "Enter the city: ";
    string city;
    cin >> city;

    cout << "Enter the start time (in 24-hour format, e.g., 1300 for 1 PM): ";
    int start_time;
    cin >> start_time;

    cout << "Enter the end time (in 24-hour format, e.g., 1500 for 3 PM): ";
    int end_time;
    cin >> end_time;

    if (is_conflicting(timetable, day, start_time, end_time))
    {
        cout << "Error: Timetable entry conflicts with an existing entry." << endl;
        return;
    }

    ofstream outfile("timetable.csv", ios::app);
    if (outfile.is_open())
    {
        outfile << username << "," << day << "," << city << "," << start_time << "," << end_time << "\n";
        outfile.close();
        cout << "Timetable updated successfully!" << endl;
    }
    else
    {
        cout << "Failed to open the file." << endl;
    }
}

void compare_favorite_restaurants(const char* username)
{
    vector<string> friends = get_friends(username);
    if (friends.empty())
    {
        cout << "You have no friends to compare with." << endl;
        return;
    }

    char friendUsername[MAX_USERNAME_LENGTH];
    cout << "Enter friend's username to compare favorite restaurants with: ";
    cin >> friendUsername;

    bool friendExists = false;
    for (size_t i = 0; i < friends.size(); ++i)
    {
        if (friends[i] == friendUsername)
        {
            friendExists = true;
            break;
        }
    }

    if (!friendExists)
    {
        cout << "The entered username is not in your friends list." << endl;
        return;
    }

    ifstream file("favorites.csv");
    if (!file.is_open())
    {
        cerr << "Error opening favorites file." << endl;
        return;
    }

    vector<string> userFavorites;
    vector<string> friendFavorites;

    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        string user, restaurant;
        getline(ss, user, ',');
        getline(ss, restaurant, ',');

        if (user == username)
        {
            userFavorites.push_back(restaurant);
        }
        else if (user == friendUsername)
        {
            friendFavorites.push_back(restaurant);
        }
    }

    file.close();

    vector<string> commonRestaurants;
    for (size_t i = 0; i < userFavorites.size(); ++i)
    {
        for (size_t j = 0; j < friendFavorites.size(); ++j)
        {
            if (userFavorites[i] == friendFavorites[j])
            {
                // Check if it's already in commonRestaurants to avoid duplicates
                if (find(commonRestaurants.begin(), commonRestaurants.end(), userFavorites[i]) == commonRestaurants.end())
                {
                    commonRestaurants.push_back(userFavorites[i]);
                }
            }
        }
    }

    if (commonRestaurants.empty())
    {
        cout << "No common favorite restaurants found." << endl;
    }
    else
    {
        cout << "Common favorite restaurants:" << endl;
        for (size_t i = 0; i < commonRestaurants.size(); ++i)
        {
            cout << "- " << commonRestaurants[i] << endl;
        }
    }
}
void add_favorite_restaurant(const char* username)
{
    string restaurant;
    cout << "Enter the name of the restaurant to add to your favorites: ";
    getline(cin, restaurant);

    ofstream outfile("favorites.csv", ios::app);
    if (!outfile.is_open())
    {
        cerr << "Error opening favorites file." << endl;
        return;
    }

    outfile << username << "," << restaurant << "\n";
    outfile.close();

    cout << "Restaurant added to your favorites successfully!" << endl;
}

// Function to display the user's favorite restaurants
void view_favorite_restaurants(const char* username)
{
    ifstream infile("favorites.csv");
    if (!infile.is_open())
    {
        cerr << "Error opening favorites file." << endl;
        return;
    }

    vector<string> favoriteRestaurants;
    string line;

    while (getline(infile, line))
    {
        stringstream ss(line);
        string user, restaurant;
        getline(ss, user, ',');
        getline(ss, restaurant, ',');

        if (user == username)
        {
            favoriteRestaurants.push_back(restaurant);
        }
    }

    infile.close();

    if (favoriteRestaurants.empty())
    {
        cout << "You have no favorite restaurants." << endl;
    }
    else
    {
        cout << "Your favorite restaurants:" << endl;
        for (size_t i = 0; i < favoriteRestaurants.size(); ++i)
        {
            cout << "- " << favoriteRestaurants[i] << endl;
        }
    }
}
void menu(const char *username) {
    int choice;
    while (1) {
        cout << "1. Send Friend Request\n";
        cout << "2. View Friend Requests\n";
        cout << "3. View Friends\n";
        cout << "4. Respond to Friend Request\n";
        cout << "5. Update Timetable\n";
        cout << "6. Check Friends\n";
        cout << "7. Compare Favorite Restaurants\n";
        cout << "8. Add Favorite Restaurant\n";
        cout << "9. View Favorite Restaurants\n";
        cout << "10. Log Out\n";
        cout << "Enter Choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: {
                system("cls");
                char to[MAX_USERNAME_LENGTH];
                cout << "Enter username to send request to: ";
                cin >> to;
                send_friend_request(username, to);
                break;
            }
            case 2: {
                system("cls");
                view_friend_requests(username);
                break;
            }
            case 3: {
                system("cls");
                view_friends(username);
                break;
            }
            case 4: {
                char from[MAX_USERNAME_LENGTH];
                int response;
                cout << "Enter username of the friend request to respond to: ";
                cin >> from;
                cout << "Enter 1 to accept or 0 to reject the friend request: ";
                cin >> response;
                respond_friend_request(from, username, response == 1);
                break;
            }
            case 5: {
                system("cls");
                update_timetable(username);
                break;
            }
            case 6: {
                system("cls");
                check_overlap_and_suggest_outing(username);
                break;
            }
            case 7: {
                system("cls");
                compare_favorite_restaurants(username);
                break;
            }
            case 8: {
                system("cls");
                add_favorite_restaurant(username);
                break;
            }
            case 9: {
                system("cls");
                view_favorite_restaurants(username);
                break;
            }
            case 10:
                system("cls");
                cout << "Bye, bye!\n";
                return;
            default:
                cout << "Invalid choice." << endl;
                break;
        }
    }
}

int main()
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];

    int choice;
    while (true)
    {
        cout << "1. Signup\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        if (choice == 1)
        {
            signup();
        }
        else if (choice == 2)
        {
            cout << "Enter username: ";
            cin >> username;
            cout << "Enter password: ";
            cin >> password;

            if (check_user(username, password))
            {
                cout << "Login successful!" << endl;
                menu(username); // Call the menu function for the logged-in user
            }
            else
            {
                cout << "Login failed. Invalid username or password." << endl;
            }
        }
        else if (choice == 3)
        {
            break;
        }
        else
        {
            cout << "Invalid choice. Please try again." << endl;
        }
    }

    return 0;
}
