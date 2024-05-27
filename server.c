#include "common.h"

//Handler for server to count number of user 1 messages to process
void user1_sig_handler(int sig) {
    ++msg_num_usr1;
}

//Handler for server to count number of user 2 messages to process
void user2_sig_handler(int sig) {
    ++msg_num_usr2;
}

//Handler for server to count number of user 3 messages to process
void user3_sig_handler(int sig) {
    ++msg_num_usr3;
}


/*Program Starts here: Initialize all 6 signals used along with their respective signal handlers. After registering
signals, create 6 pipes, 3 for user to server, and three for server to user. Fork 3 children processes all stemming
from the parent process which we refer to as the server. Close all pipes not used by each user (ie, close pipe 2
for user 1 and 3) and also close all pipe ends respective to each user and the server.

SIGACTION --> Register signal handler with a corresponding Signal and essentially links them!
*/
int main() {
    server_id = getpid();
    struct sigaction sa1;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = SA_RESTART;
    sa1.sa_handler = user1_sig_handler;
    
    if (sigaction(SIGRTMIN, &sa1, NULL) != 0) {
        perror("sigaction failed");
        return 1;
    }

    struct sigaction sa2;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = SA_RESTART;
    sa2.sa_handler = user2_sig_handler;
    
    if (sigaction(SIGRTMIN + 1, &sa2, NULL) != 0) {
        perror("sigaction failed");
        return 1;
    }

    struct sigaction sa3;
    sigemptyset(&sa3.sa_mask);
    sa3.sa_flags = SA_RESTART;
    sa3.sa_handler = user3_sig_handler;
    
    if (sigaction(SIGRTMIN + 2, &sa3, NULL) != 0) {
        perror("sigaction failed");
        return 1;
    }

    struct sigaction sa_sv1;
    sigemptyset(&sa_sv1.sa_mask);
    sa_sv1.sa_flags = SA_RESTART;
    sa_sv1.sa_handler = user1_handle_new_msg_from_srv;
    
    if (sigaction(SIGRTMIN + 3, &sa_sv1, NULL) != 0) {
        perror("sigaction failed");
        return 1;
    }

    struct sigaction sa_sv2;
    sigemptyset(&sa_sv2.sa_mask);
    sa_sv2.sa_flags = SA_RESTART;
    sa_sv2.sa_handler = user2_handle_new_msg_from_srv;
    
    if (sigaction(SIGRTMIN + 4, &sa_sv2, NULL) != 0) {
        perror("sigaction failed");
        return 1;
    }

    struct sigaction sa_sv3;
    sigemptyset(&sa_sv3.sa_mask);
    sa_sv3.sa_flags = SA_RESTART;
    sa_sv3.sa_handler = user3_handle_new_msg_from_srv;

    if (sigaction(SIGRTMIN + 5, &sa_sv3, NULL) != 0) {
        perror("sigaction failed");
        return 1;
    }

    //create pipe for user to server for all three users, visa versa
    for(int i = 0; i < NUM_USERS; i++) {
        pipe(server_to_user[i]);
        pipe(user_to_server[i]);
    }

    //User 1 initializing (closing unused pipe and pipe ends, generate messages to be processed by write handler)
    user1_id = fork();
    if (user1_id == 0) {
        close(server_to_user[0][1]);
        close(user_to_server[0][0]);
        for(int i = 0; i < NUM_USERS; i++) {
            if(i != 0) {
                close(server_to_user[i][0]);
                close(server_to_user[i][1]);
                close(user_to_server[i][0]);
                close(user_to_server[i][1]);
            }
        }
        int num_messages_1 = 3;
        char user1_messages[3][128] ={"user_1_sent_msg_to_user_1: All men are mortal.", "user_1_sent_msg_to_user_2: Socrates is a man.", "user_1_sent_msg_to_user_3: Therefore, Socrates is mortal."};
        user_write_handler(user_to_server[0][1], 0, user1_messages, num_messages_1, server_id);
        exit(0);

    } else {
        close(server_to_user[0][0]);
        close(user_to_server[0][1]);

        //User 2 initializing (closing unused pipe and pipe ends, generate messages to be processed by write handler)
        user2_id = fork();
        if (user2_id == 0) {
            close(server_to_user[1][1]);
            close(user_to_server[1][0]);
            for(int i = 0; i < NUM_USERS; i++) {
                if(i != 1) {
                    close(server_to_user[i][0]);
                    close(server_to_user[i][1]);
                    close(user_to_server[i][0]);
                    close(user_to_server[i][1]);
                }
            }
            int num_messages_2 = 2;
            char user2_messages[2][128] = {"user_2_sent_msg_to_user_1: We know what we are.", "user_2_sent_msg_to_user_3: But know not what we may be."};
            user_write_handler(user_to_server[1][1], 1, user2_messages, num_messages_2, server_id);
            exit(0);

        } else {
            close(server_to_user[1][0]);
            close(user_to_server[1][1]);

            //User 3 initializing (closing unused pipe and pipe ends, generate messages to be processed by write handler)
            user3_id = fork();
            if (user3_id == 0) {
                close(server_to_user[2][1]);
                close(user_to_server[2][0]);
                for(int i = 0; i < NUM_USERS; i++) {
                    if(i != 2) {
                        close(server_to_user[i][0]);
                        close(server_to_user[i][1]);
                        close(user_to_server[i][0]);
                        close(user_to_server[i][1]);
                    }
                }
                int num_messages_3 = 1;
                char user3_messages[1][128] = {"user_3_sent_msg_to_user_1: We wish you a prosperous Year of Dragon!"};
                user_write_handler(user_to_server[2][1], 2, user3_messages, num_messages_3, server_id);
                exit(0);
            } else {
                close(server_to_user[2][0]);
                close(user_to_server[2][1]);
            }
        }
    }
    
    server_read_handler();

    //Next three lines reap the children processes so computer resources are conserved
    wait(NULL);
    wait(NULL);
    wait(NULL);
    return 0;
}

//Converts character to int then subtracts 1 because id is decimal value - 1.
int char_to_id(char ch) {
    return ch - '0' - 1;
}

/*This method does the relaying of message: Recieves the message and extracts the proper char to determine which
user will actually be recieving the message and who the message was sent from. Will write to the write end of the
pipe that is to recieve the message and will later be read by that user!

WRITE --> Writes to the pipe where the user will eventually read the message to successfully transmit message!
KILL --> When signal sent, message count will increase, telling user how many messages to account for from server!
*/
int relay_message(char message[]) {
    errno = 0;
    int write_err = 0;
    int k_err = 0;
    int id_to_send = char_to_id(message[24]);
    //-1 will return if there was an error, if interrupted by signal, will write until write success
    if ((write_err = write(server_to_user[id_to_send][1], message, 128)) == -1) {
        if(errno == EINTR) {
            while(1) {
                if(write(server_to_user[id_to_send][1], message, 128) != -1) {
                    break;
                }
            }
        } else {
            printf("Write Err: %d\n", write_err);
            perror("server failed to write to pipe");
            return 1;
        }
    }
    if(id_to_send == 0) {
        k_err = kill(user1_id, SIGRTMIN + 3);
    } else if(id_to_send == 1) {
        k_err = kill(user2_id, SIGRTMIN + 4);
    } else if( id_to_send == 2) {
        k_err = kill(user3_id, SIGRTMIN + 5);
    }
    if(k_err == -1) {
        perror("Kill fail");
        return 1;
    }
    return 0;
}

/*This is the server processing (Think CPU of the program): Recieves the messages sent from each and processes it
to determine which user the message is being sent to (done in relay_message() helper) Monitors number of messages
processed and once finished processing and relaying all user messages, server job is finished.

WRITE --> Writes to the pipe where the server will eventually read the message for further processing!
KILL --> When signal sent, message count will increase, telling user how many messages to account for from server!
READ --> Will return 0 if trying to read from a pipe where write end is closed. This indicates that it is done
         reading messages from the pipe and will exit the infinite loop!
*/
int server_read_handler() {
    char message[128];
    int err1 = 1;
    int err2 = 1;
    int err3 = 1;
    int k_err = 0;
    int write_err = 0;
    
    errno = 0;
    struct timespec req, rem; // * advanced sleep which will not be interfered by signals
    req.tv_sec = 2;  // The time to sleep in seconds
    req.tv_nsec = 0; // Additional time to sleep in nanoseconds
    while(nanosleep(&req, &rem) == -1) {
        if(errno == EINTR) {
            req = rem;
        }
    }

    while(1) {
        //User 1 Message Processing
        if(msg_num_usr1 > 0) {
            errno = 0;
            err1 = read(user_to_server[0][0], message, 128);
            if (err1 == -1) {
                if(errno == EINTR) {
                    continue;
                } else {
                    perror("Error");
                    return 1;
                }
            } else if (err1 == 0) {
                --msg_num_usr1;
            } else {
                relay_message(message);      
                --msg_num_usr1;
            }
        } 
        //User 2 Message Processing
        if(msg_num_usr2 > 0) {
            errno = 0;
            err2 = read(user_to_server[1][0], message, 128);
            if (err2 == -1) {
                if(errno == EINTR) {
                    continue;
                } else {
                    perror("Error");
                    return 1;
                }
            } else if(err2 == 0) {
                --msg_num_usr2;
            } else {
                relay_message(message);
                --msg_num_usr2;
            }
        } 
        //User 3 Message Processing
        if(msg_num_usr3 > 0) {
            errno = 0;
            err3 = read(user_to_server[2][0], message, 128);
            if (err3 == -1) {
                if(errno == EINTR) {
                    continue;
                } else {
                    perror("Error");
                    return 1;
                }
            } else if (err3 == 0) {
                --msg_num_usr3;
            } else {
                relay_message(message);
                --msg_num_usr3;
            }
        }
        //Once all users are done relaying messages, close write pipes and send one more signal so user knows to stop reading!
        if(err1 == 0 && err2 == 0 && err3 == 0) {   
            close(server_to_user[0][1]);
            kill(user1_id, SIGRTMIN + 3);  
            close(server_to_user[1][1]);
            kill(user2_id, SIGRTMIN + 4);  
            close(server_to_user[2][1]);
            kill(user3_id, SIGRTMIN + 5);  
            break;
        }
    }
    return 0;
}