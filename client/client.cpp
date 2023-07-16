#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include "../common/socket/local_sock.h"
#include "../common/status_codes.h"
#include "../common/socket/sockets.h"
#include "../common/Board.h"

using std::cout;
using std::endl;

// Function prototypes
int init_client(char * server_name);
void ServerDisconnected();
bool TakeTurn(Board & board);
std::string char2String(char * message);

int client_sock = 0;

int main(int argc, char ** argv) {

    char buffer[BUF_SIZE];
    bool game_over = false;
    bool ingame = false;
    bool isLogin = false;
    char temp;
    int row, col;
    StatusCode status;
    Board board;

    // Check command line for hostname
    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s <server name>\n", argv[0]);
        exit(1);
    }

    // Initialize the client and get the client's socket
    client_sock = init_client(argv[1]);

    // Send user messages to the server
    while(!game_over)
    {
        // Wait for a response from the server
        if(!ReceiveStatus(client_sock, &status))
            ServerDisconnected();

        // Respond to the status
        switch(status)
        {
            case REGISTERED:
                cout << "Player name registered with server" << endl;
                break;
            case CREATED:
                cout << "New game created. You are X's. Waiting for a player to connect..." << endl;
                ingame = true;
                board = Board();
                board.SetType(EXXES);
                break;
            case JOINED:
                cout << "You joined the game. You are O's. Other player's turn." << endl;
                ingame = true;
                board = Board();
                board.SetType(OHS);
                break;
            case OTHER_JOINED:
                cout << "Somebody joined your game!" << endl;
                board = Board();
                TakeTurn(board);
                break;
            case MOVE:
                // Receive the row and column
                if(!ReceiveInt(client_sock, &row))
                    ServerDisconnected();

                if(!ReceiveInt(client_sock, &col))
                    ServerDisconnected();

                printf("Received a move: row %d, col %d\n", row, col);
                board.OtherMakeMove(row, col);

                // Check if receiving move is loss or tie
                if(board.IsLost(row, col))
                {
                    // You lost D:!!!
                    cout << "You lost! D:" << endl;
                    ingame = false;
                }
                else if (board.IsDraw())
                {
                    cout << "You tied" << endl;
                    ingame = false;
                }
                else
                {
                    // Have the player take their turn
                    game_over = TakeTurn(board);
                    if (game_over)
                    {
                        ingame = false;
                        game_over = false;
                    }
                }

                break;
            case LIST:
                temp = 'D';

                // Print out the list of games from the server
                while(temp != '\0')
                {
                    if(read(client_sock, &temp, 1) > 0)
                        cout << temp;
                    else
                        ServerDisconnected();
                }

                break;
            case PLAYER_DISCONNECT:
                ServerDisconnected();
                break;
            case OTHER_DISCONNECT:
                cout << "Other player disconnected :(" << endl;
                cout << "You win by default, congrats!" << endl;
                ingame = false;
                break;
            case GAME_EXISTS:
                cout << "That game already has two players connected. Try again." << endl;
                break;
            case INVALID_CMD:
                cout << "Invalid Command" << endl;
                break;
            case NOTLOGIN:
                cout << "Please login or signin: " << endl;
                break;
            case DIFFERENCE_RANK:
                cout << "You can't entry this room because difference ranking" << endl;
                break;
            default:
                cout << "Unrecognized response from the server" << endl;
                exit(1);
        }

        if (!isLogin)
        {

            std::string input = "";

            while (input != "login" && input != "signup")
            {
                cout << "Input command (login or signup): ";

                // Read a line of text from the user
                fgets(buffer, BUF_SIZE - 1, stdin);
                int i = 0;
                for (i = 0; buffer[i] != '\n'; ++i)
                {

                }
                buffer[i] = '\0';   
                input = buffer;
            }

            
            // Write it out to the server
            SendString(client_sock, buffer);

            char msg[1024];
            ReceiveString(client_sock, msg);
            std::string message = char2String(msg);
            std::cout << (message == "username") << " " << message.size() << " " << std::endl;
            if (message == "username")
            {
                cout << "Input username: ";
                char username[1024];
                std::cin >> username;

                SendString(client_sock, username);
                ReceiveString(client_sock, msg);
                message = char2String(msg);

                if (message == "UNACTIVE")
                {
                    cout << "Your account was disabled" << endl;
                    close(client_sock);
                    exit(1);
                }
                else if (message == "password")
                {
                    while (message == "password")
                    {
                        cout << "Input password: ";
                        char password[1024];
                        std::cin >> password;

                        SendString(client_sock, password);
                        ReceiveString(client_sock, msg);
                        message = char2String(msg);
                    }

                    if (message == "LOCKED")
                    {
                        cout << "You took wrong password 3 times" << endl;
                        cout << "Your account will be disabled" << endl;
                        cout << "Please contact admin to reactive your account" << endl;
                        close(client_sock);
                        exit(1);
                    }
                    else if (message == "Login sucessfully")
                    {
                        isLogin = true;
                        cout << "Your account information:" << endl;
                        ReceiveString(client_sock, msg);
                        cout << char2String(msg) << endl;
                    }
                    else if (message == "Signup sucessfully")
                    {
                        cout << "Your account has been created" << endl;
                    }
                }
                else if (message == "EXISTS")
                {
                    cout << "This account already exists" << endl;
                    close(client_sock);
                    exit(1);
                }
            }
            else if (message == "NOTFOUND")
            {
                cout << "Your account not exist" << endl;
                close(client_sock);
                exit(1);
            }

        }
        if(!ingame && isLogin) // Send commands if not in game yet
        {
            std::string input = "";
            do 
            {
                // flush any data from the internal buffers
                int c;
                while((c = getchar()) != '\n' && c != EOF); 
                cout << "\nEnter a command: ";

                // Read a line of text from the user
                fgets(buffer, BUF_SIZE - 1, stdin);
                int i = 0;
                for (i = 0; buffer[i] != '\n'; ++i)
                {

                }
                buffer[i] = '\0';
                input = buffer;
                
            } while (input.size() == 0);
            SendString(client_sock, buffer);    
        }
    }

    cout << "Thanks for playing!" << endl;

    return 0;
}

/**
 * Create the client socket and connect to the server
 */
int init_client(char * server_name)
{
    int client_sock = 0;    // This client's connection to the server
    struct addrinfo * aip;  // Server's address information
    struct addrinfo hint;   // Used when determining the server address info
    int err = 0;            // Used for capturing error messages

    // Setup hint structure for getaddrinfo
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;   // Use IPv4
    hint.ai_socktype = SOCK_STREAM; // Use TCP

    // Get server address info
    if((err = getaddrinfo(server_name, PORT_NUM, &hint, &aip)) != 0)
    {
        fprintf(stderr, "Failed to get server address info: %s", gai_strerror(err));
        exit(1);
    }

    // Create socket connection to server
    if((client_sock = socket(aip->ai_family, aip->ai_socktype, 0)) < 0)
    {
        perror("Failed to create socket");
        exit(1);
    }

    // Connect to the server
    if(connect(client_sock, aip->ai_addr, aip->ai_addrlen) < 0)
    {
        perror("Failed to connect to server");
        exit(1);
    }

    printf("Connected to server successfully!\n");

    return client_sock;
}

// Have the player take their turn
bool TakeTurn(Board & board)
{
    bool game_over = false;
    bool input_good = false;
    int row = 0, col = 0;

    // Display the board
    board.DrawBoard();

    while(!input_good)
    {
        printf("Enter move (row col): ");

        // Make sure two integers were inputted
        if(scanf("%d %d", &row, &col) == 2)
        {
            if(row < 0 || row > MAX_ROWS)
                printf("Invalid row input. Try again.\n");
            else if (col < 0 || col > MAX_COLS)
                printf("Invalid column input. Try again.\n");
            else if(!board.IsBlank(row, col))
                printf("That cell isn't blank. Try again.\n");
            else
                input_good = true;
        }
        else
            printf("Invalid move input. Try again.\n");

        // flush any data from the internal buffers
        int c;
        while((c = getchar()) != '\n' && c != EOF);
    }

    board.PlayerMakeMove(row, col);
    cout << "Sending move to server" << endl;

    // Send the move to the server
    if(!SendStatus(client_sock, MOVE))
        ServerDisconnected();

    if(!SendInt(client_sock, row))
        ServerDisconnected();

    if(!SendInt(client_sock, col))
        ServerDisconnected();

    // Check for win/draw
    if(board.IsWon(row, col))
    {
        cout << "YOU WIN!!!!" << endl;
        game_over = true;
        SendStatus(client_sock, WIN);
    }
    else if (board.IsDraw())
    {
        cout << "You tied ._." << endl;
        game_over = true;
        SendStatus(client_sock, DRAW);
    }

    return game_over;
}

// Tell the user they've been disconnected then quit
void ServerDisconnected()
{
    printf("You've been disconnected from the server D:\n");
    close(client_sock);
    exit(1);
}

std::string char2String(char *message)
{
    std::string msg = "";
    for (int i = 0; message[i] != '\0' && int(message[i]) != 127; ++i)
    {
        msg += message[i];
    }
    return msg;
}