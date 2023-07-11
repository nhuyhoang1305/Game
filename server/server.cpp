#include <iostream>
#include <algorithm>
#include <list>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <bits/local_lim.h>
#include <map>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include "../common/socket/local_sock.h"
#include "../common/Player.h"
#include "../common/Game.h"
#include "../common/status_codes.h"
#include "../common/socket/sockets.h"

using std::cout;
using std::endl;
using std::list;
using std::find;
using std::remove_if;
using std::find_if;
using std::map;
using std::vector;


/** Function prototypes **/
char * allocate_hostname();
int init_server();
void * handle_client(void * arg);
bool ProcessCommand(char buffer[], Player * player, bool & client_connected);
void sig_handler(int signum);
void DisconnectPlayer(const Player * player);
void ListGames(const Player * player);
void CreateJoinGame(Player * player, string game_name);
void LoadPlayersFromDB();
void SavePlayersToDB();
string char2String(char *message);

/** Global Variables **/
int server_sock = 0;    // Server socket

list<Game *> game_list;    // List of open games
map<string, Player *> players; // List of player in database

// Mutex to make sure operations on games list are atomic
pthread_mutex_t games_lock = PTHREAD_MUTEX_INITIALIZER;

int main() {

    printf("Load players from db....\n");
    LoadPlayersFromDB();
    printf("Has %ld players\n", players.size());


    // Signals used to kill the server gracefully
    if(signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("Can't catch SIGINT");
        exit(1);
    }

    if(signal(SIGTERM, sig_handler) == SIG_ERR)
    {
        perror("Can't catch SIGTERM");
        exit(1);
    }

    // Initialize the server and get the server's socket
    server_sock = init_server();

    int new_socket = 0;

    // Infinitely accept clients and spawning threads
    while(true)
    {
        // Wait for a client, and then accept
        if((new_socket = accept(server_sock, NULL, NULL)) < 0)
        {
            perror("Failed to accept client");
            close(server_sock);
            exit(1);
        }

        cout << "New client connected" << endl;

        // Spawn thread to handle the client
        pthread_t threadid;
        pthread_create(&threadid, NULL, handle_client, (void *)&new_socket);
    }

    return 0;
}

/**
 * Allocates space for a hostname and fills it with the system's hostname
 *
 * @return The system hostname. Must be freed at a later point with "free"
 */
char * allocate_hostname() {
    char * host = 0;
    long maxsize = 0;

    // Get the maximum hostname size
    if((maxsize = sysconf(_SC_HOST_NAME_MAX)) < 0)
        maxsize = HOST_NAME_MAX;

    // Allocate memory for the hostname
    if((host = (char *)malloc((size_t)maxsize)) == NULL)
    {
        perror("Can't allocate hostname");
        exit(1);
    }

    // Actually get the hostname
    if(gethostname(host, (size_t)maxsize) < 0)
    {
        perror("Couldn't retrieve hostname");
        exit(1);
    }

    return host;
}

/**
 * Initialize the server: create the socket, bind, and start listening.
 *
 * @return Returns the server's socket
 */
int init_server()
{
    int orig_sock = 0; // The server's socket
    char * hostname;    // The system hostname

    struct addrinfo * aip;   // Server's address info
    struct addrinfo hint;   // Used when determining the server info

    int err = 0;    // Generic error code
    int reuse = 1;

    // Setup hint structure for getaddrinfo
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;   // Use IPv4
    hint.ai_socktype = SOCK_STREAM; // Use TCP

    // Get the system's address info
    hostname = allocate_hostname();
    if((err = getaddrinfo(hostname, PORT_NUM, &hint, &aip)) != 0)
    {
        fprintf(stderr, "Failed to get address info: %s", gai_strerror(err));
        free(hostname);
        exit(1);
    }
    free(hostname);

    // Create the socket
    if((orig_sock = socket(aip->ai_addr->sa_family, aip->ai_socktype, 0)) < 0)
    {
        perror("Failed to create socket");
        exit(1);
    }

    // Let us reuse an old address
    if(setsockopt(orig_sock, SOL_SOCKET, SO_REUSEADDR, &reuse,  sizeof(int)) < 0)
    {
        perror("Failed to setup socket for address reuse");
        exit(1);
    }

    // Bind the created socket to the address information
    if(bind(orig_sock, aip->ai_addr, aip->ai_addrlen) < 0)
    {
        perror("Failed to bind socket");
        close(orig_sock);
        exit(1);
    }

    // Free up the generated address info
    freeaddrinfo(aip);

    // Start listening on the socket
    if(listen(orig_sock, MAX_BACKLOG) < 0)
    {
        perror("Failed to start listening");
        close(orig_sock);
        exit(1);
    }

    return orig_sock;
}

/**
 * Handles a single client connection
 */
void * handle_client(void * arg)
{
    int client_sock = *(int *)arg;
    char buffer[BUF_SIZE];
    bool client_connected = true;
    char temp = '\0';
    int row = 0, col = 0;
    int i = 0;
    bool isLogin = false;

    // Create the player
    Player *player;

    // Always handle the client
    while(client_connected)
    {

        // Login required
        if (!isLogin)
        {
            char msg[1024];
            SendStatus(client_sock, NOTLOGIN);
            ReceiveString(client_sock, msg);
            string message = char2String(msg);
            if (message == "LOGIN")
            {

                char *username = (char *) "username";
                SendString(client_sock, username);
                ReceiveString(client_sock, msg);

                message = char2String(msg);
                cout << message << " " << players.count(message) << endl;
                if (players.count(message))
                {
                    player = players[message];
                    

                    if (player->GetStatus() == 0)
                    {
                        SendString(client_sock, (char *)"UNACTIVE");
                        close(client_sock);
                    }
                    else
                    {
                        //SendString(client_sock, (char *) "password");
                        int loginAttempt = 0;
                        
                        while (loginAttempt < 3 && !isLogin)
                        {
                            SendString(client_sock, (char *) "password");
                            ReceiveString(client_sock, msg);
                            message = char2String(msg);
                            if (message == player->GetPassword())
                            {
                                SendString(client_sock, (char *) "Login sucessfully");
                                SendString(client_sock, (char *) player->ToString().c_str());
                                player->SetSocket(client_sock);
                                isLogin = true;
                                break;
                            }

                            ++loginAttempt;
                        }

                        if (!isLogin)
                        {
                            // Lock account
                            SendString(client_sock, (char *) "LOCKED");
                            player->SetStatus(0);
                            SavePlayersToDB();
                            close(client_sock);
                            return (void *) 0;
                        }
                    }

                }
                else
                {
                    cout << "Username: " << message << " not exists" << endl;
                    SendString(client_sock, (char *)"NOTFOUND");
                    close(client_sock);
                    return (void *) 0;
                }

            }
        }
        else
        {
            // Process commands or pass game data
            if(player->GetMode() == COMMAND)
            {

                ReceiveString(client_sock, buffer);
                cout << "Received command \"" << buffer << "\" from " << player->GetUserName() << endl;

                // If there's an invalid command, tell the client
                if(!ProcessCommand(buffer, player, client_connected))
                    SendStatus(player->GetSocket(), INVALID_CMD);
            }
            else if (player->GetMode() == INGAME)
            {
                // Get the game the player is a part of
                pthread_mutex_lock(&games_lock);
                auto game = find_if(game_list.begin(), game_list.end(),
                        [player] (Game * game) { return game->HasPlayer(player); });
                auto end = game_list.end();
                pthread_mutex_unlock(&games_lock);

                // Something horrible has gone wrong
                if(game == end)
                    cout << "Somehow Player " << player->GetUserName() << " isn't a part of a game but is INGAME" << endl;
                else
                {
                    StatusCode status;
                    client_connected = ReceiveStatus(player->GetSocket(), &status);

                    // If the player is still connected, then perform the move
                    if(client_connected)
                    {
                        switch(status)
                        {
                            case MOVE:
                                // Pass the row and column right along
                                ReceiveInt(player->GetSocket(), &row);
                                ReceiveInt(player->GetSocket(), &col);
                                cout << "Received moved from " << player->GetName()
                                    << ": row=" << row << ", col=" << col << endl;

                                SendStatus((*game)->GetOtherPlayer(player)->GetSocket(), MOVE);
                                SendInt((*game)->GetOtherPlayer(player)->GetSocket(), row);
                                client_connected = SendInt((*game)->GetOtherPlayer(player)->GetSocket(), col);
                                cout << "Sent move to " << (*game)->GetOtherPlayer(player)->GetUserName() << endl;

                                break;
                            case WIN:
                                cout << player->GetUserName() << " won a game against " << (*game)->GetOtherPlayer(player)->GetUserName() << endl;
                                player->SetScore(player->GetScore() + 5);
                                player->SetMode(COMMAND);
                                (*game)->GetOtherPlayer(player)->SetMode(COMMAND);
                                (*game)->GetOtherPlayer(player)->SetScore((*game)->GetOtherPlayer(player)->GetScore() - 5);
                                break;
                            case DRAW:
                                cout << player->GetUserName() << " tied against " << (*game)->GetOtherPlayer(player)->GetUserName() << endl;
                                player->SetMode(COMMAND);
                                (*game)->GetOtherPlayer(player)->SetMode(COMMAND);
                                break;
                            default:
                                client_connected = SendStatus(player->GetSocket(), INVALID_CMD);
                        }
                    }
                }
            }
        }
        
    }

    // The client disconnected on us D:
    cout << "Player \"" << player->GetUserName() << "\" has disconnected" << endl;
    SavePlayersToDB();
    DisconnectPlayer(player);
    close(client_sock);

    return (void *)0;
}

/**
 * Processes a command coming from the client
 *
 * @param buffer The command sent from the user
 * @param player A reference to the player who sent the command
 * @param game A pointer to the game the player will be a part of
 *
 * @return True if the command was valid, false otherwise
 */
bool ProcessCommand(char buffer[], Player * player, bool & client_connected)
{
    char s_command[BUF_SIZE], s_arg[BUF_SIZE];
    string command, arg;
    bool valid_cmd = true;

    // Convert the input command into separate command and argument
    int num = sscanf(buffer, "%s %s", s_command, s_arg);
    command = s_command;
    arg = s_arg;

    // Scanf needs to have captured either one or two strings
    if(command == "join" && num == 2)
    {
        CreateJoinGame(player, arg);
    }
    else if (command == "register" && num == 2)
    {
        // Set the player's name
        player->SetName(arg);
        SendStatus(player->GetSocket(), REGISTERED);
        cout << "Registered player name \"" << arg << "\"" << endl;
    }
    else if (command == "list" && num == 1)
    {
        ListGames(player);
        cout << player->GetName() << " listed all open games" << endl;
    }
    else if (command == "leave" && num == 1)
    {
        SendStatus(player->GetSocket(), PLAYER_DISCONNECT);
        client_connected = false;
    }
    else
        valid_cmd = false;

    return valid_cmd;
}

// Create or join a player to a game
void CreateJoinGame(Player * player, string game_name)
{
    pthread_mutex_lock(&games_lock);

    auto iter = find_if(game_list.begin(), game_list.end(),
        [game_name] (Game * game) { return game->GetName() == game_name; });

    // Check if the game already exists
    if(iter != game_list.end())
    {
        // Check if the game already has two players
        if((*iter)->GetPlayer2()->GetSocket() == -1)
        {
            // Check if the rank is difference
            if ((*iter)->GetPlayer1()->GetRank() != player->GetRank())
            {
                SendStatus(player->GetSocket(), DIFFERENCE_RANK);
            }
            else
            {
                // Same rank => can battle
                // If not, join the two players together and notify them
                (*iter)->SetPlayer2(player);
                player->SetMode(INGAME);
                SendStatus(player->GetSocket(), JOINED);
                SendStatus((*iter)->GetPlayer1()->GetSocket(), OTHER_JOINED);
            }
        }
        else    // Otherwise, tell the player that game already exists
            SendStatus(player->GetSocket(), GAME_EXISTS);
    }
    else
    {
        // Create a new game and add it to the list
        Game * game = new Game(game_name, player);
        game_list.push_back(game);
        SendStatus(player->GetSocket(), CREATED);
        player->SetMode(INGAME);
    }

    pthread_mutex_unlock(&games_lock);
}

// Send a list of all open games to the player
void ListGames(const Player * player)
{
    int num_open_games = 0;

    // Send a list of all of the games
    SendStatus(player->GetSocket(), LIST);
    write(player->GetSocket(), "---All Open Games---\n", 21);

    pthread_mutex_lock(&games_lock);
    for(auto iter = game_list.begin(); iter != game_list.end(); ++iter)
    {
        // Only print out the games that don't have a player 2
        if((*iter)->GetPlayer2()->GetSocket() == -1) {
            write(player->GetSocket(), (*iter)->ToString().c_str(), (*iter)->ToString().length());
            write(player->GetSocket(), "\n", 1);
            ++num_open_games;
        }
    }
    pthread_mutex_unlock(&games_lock);

    // If there are no open games, tell the client
    if(num_open_games == 0)
        write(player->GetSocket(), "No open games\n", 14);

    // Tell the client we're done sending
    write(player->GetSocket(), "\0", 1);
}

// Remove the player from any games they were a part of
void DisconnectPlayer(const Player * player)
{
            
    // Remove player from any games they were a part of and notify the other player
    pthread_mutex_lock(&games_lock);

    // Inform the other player that this player disconnected
    for(auto iter = game_list.begin(); iter != game_list.end(); ++iter)
    {
        if((*iter)->GetPlayer1() == player)
        {
            (*iter)->GetPlayer1()->SetScore((*iter)->GetPlayer1()->GetScore() - 5);
            (*iter)->GetPlayer2()->SetScore((*iter)->GetPlayer2()->GetScore() + 5);
            (*iter)->GetPlayer2()->SetMode(COMMAND);
            SendStatus((*iter)->GetPlayer2()->GetSocket(), OTHER_DISCONNECT);
        }
        else if ((*iter)->GetPlayer2() == player)
        {
            (*iter)->GetPlayer1()->SetScore((*iter)->GetPlayer1()->GetScore() + 5);
            (*iter)->GetPlayer2()->SetScore((*iter)->GetPlayer2()->GetScore() - 5);
            (*iter)->GetPlayer1()->SetMode(COMMAND);
            SendStatus((*iter)->GetPlayer1()->GetSocket(), OTHER_DISCONNECT);
        }
    }

    // Remove any games the player was a part of
    game_list.remove_if([player] (Game * game) { return game->HasPlayer(player); });

    pthread_mutex_unlock(&games_lock);
}

/**
 * Gracefully cleanup the sockets upon catching a signal
 */
void sig_handler(int signum)
{
    switch(signum)
    {
        case SIGTERM:
        case SIGINT:
            close(server_sock);

            // Dynamically delete every game in the list and close every connected client
            for(auto iter = game_list.begin(); iter != game_list.end(); ++iter)
            {
                close((*iter)->GetPlayer1()->GetSocket());
                close((*iter)->GetPlayer2()->GetSocket());

                // Remove the game
                delete *iter;
            }

            game_list.clear();
            SavePlayersToDB();
            printf("Exiting server\n");
            exit(0);
            break;
        default: printf("Unrecognized signal captured: %d", signum);
    }
}

/*
* Load list player from DB
*/
void LoadPlayersFromDB()
{
    // File pointer
    std::fstream fin;
  
    // Open an existing file
    fin.open(".//accounts.csv", std::ios::in);

    // Get the roll number
    // of which the data is required
    int rollnum, roll2, count = 0;
  
    // Read the Data from the file
    // as String Vector
    vector<string> row;
    string line, word;

    while (!fin.eof()) {
  
        row.clear();
  
        // read an entire row and
        // store it in a string variable 'line'
        getline(fin, line);
        
        // used for breaking words
        std::stringstream s(line);
  
        // read every column data of a row and
        // store it in a string variable, 'word'
        while (std::getline(s, word, ',')) {
  
            // add all the column data
            // of a row to a vector
            row.push_back(word);
        }

        Player * player = new Player();
        player->SetID(std::stoi(row[0]));
        player->SetUserName(row[1]);
        player->SetPassword(row[2]);
        player->SetRank(row[3]);
        player->SetStatus(std::stoi(row[4]));
        player->SetScore(std::stoi(row[5]));


        players[player->GetUserName()] = player;
    }
    fin.close();
}

string char2String(char *message)
{
    string msg = "";
    for (int i = 0; message[i] != '\0' && int(message[i]) != 127; ++i)
    {
        msg += message[i];
    }

    return msg;
}

void SavePlayersToDB()
{
    // File pointer
    std::fstream fout;

  
    // Create a new file to store updated data
    fout.open("accountsnew.csv", std::ios::out);
  
    for (const auto &kv : players)
    {
        fout << kv.second->GetID() << ",";
        fout << kv.second->GetUserName() << ",";
        fout << kv.second->GetPassword() << ",";
        fout << kv.second->GetRank() << ",";
        fout << kv.second->GetStatus() << ",";
        fout << kv.second->GetScore() << ",\n";
    }
    
    //fin.close();
    fout.close();
  
    // removing the existing file
    remove("accounts.csv");
  
    // renaming the updated file with the existing file name
    rename("accountsnew.csv", "accounts.csv");
}