#include "common.h"

//Handler for user 1 to count number of messages to recieve from server
void user1_handle_new_msg_from_srv(int sig) {
    ++usr1_msg_num_srv;
}

//Handler for user 2 to count number of messages to recieve from server
void user2_handle_new_msg_from_srv(int sig) {
    ++usr2_msg_num_srv;
}

//Handler for user 3 to count number of messages to recieve from server
void user3_handle_new_msg_from_srv(int sig) {
    ++usr3_msg_num_srv;
}

/*This is what each user individually executes: Executes in the child process and sends the messages to the server
once server has processed and send back the messages, each user will process the messages it recieved and output

KILL --> When signal sent, message count will increase, telling server how many messages to account for per user!
READ --> Will return 0 if trying to read from a pipe where write end is closed. This indicates that it is done
         reading messages from the pipe and will exit the infinite loop!
*/
int user_write_handler(int write_pipe, int user_id, char messages[][128], int num_messages, pid_t pid) {
    char messagePad[128];

    for(int j = 0; j < num_messages; j++) {
        
        //next two for loops are to masked will null padding so 128 bytes are consistently sent and recieved
        for(int i = 0; i < 128; i++) {
            messagePad[i] = 0;
        }

        for(int i = 0; i < strlen(messages[j]); i++) {
            messagePad[i] = messages[j][i];
        }

        int err = 0;
        errno = 0;
        if (write(user_to_server[user_id][1], messagePad, 128) == -1) { 
            //-1 will return if there was an error, if interrupted by signal, will write until write success
            if(errno == EINTR){
                while(1) {
                    if(write(user_to_server[user_id][1], messagePad, 128) != -1) {
                        break;
                    }
                }
            } else {
                perror("child failed to write to pipe");
                exit(1);
            }  
        }
        printf("%s\n", messagePad);
        err = kill(pid, SIGRTMIN + user_id);

        if(err == -1) {
            perror("Kill failed");
            return 1;
        }
    }

    //Final kill sent after close so that server knows this user has sent all meesages it wanted to
    int k_err = 0;
    close(user_to_server[user_id][1]);
    k_err = kill(pid, SIGRTMIN + user_id);
    if(k_err == -1) {
        perror("Kill failed");
        exit(1);
    }

    char message_from_s[128];
    int err1 = 1;
    int err2 = 1;
    int err3 = 1;
    //User 2 Message Receiving
    if(user_id == 0) {
        while(1) {
            if(usr1_msg_num_srv > 0) {
                errno = 0;
                err1 = read(server_to_user[0][0], message_from_s, 128);
                //-1 will return if there was an error, if interrupted by signal, will read until read success
                if (err1 == -1) {
                    if(errno == EINTR) {
                        continue;
                    } else {
                        perror("Error");
                        return 1;
                    }
                } else if (err1 == 0) {
                    --usr1_msg_num_srv;
                    exit(0);
                } else {
                    //Next two for loops are to mask and differentiate the recieved messages from sent ones
                    char maskMSG[128];
                    for(int i = 0; i < 128; i++) {
                        maskMSG[i] = 0;
                    }

                    for(int i = 27; i < 128; i++) {
                        maskMSG[i - 27] = message_from_s[i];
                    }

                    printf("user_1_received_msg_from_user_%c: %s\n", message_from_s[5], maskMSG);
                    --usr1_msg_num_srv;
                }
            }
        }
    //User 2 Message Receiving
    } else if(user_id == 1) {
        while(1) {
            if(usr2_msg_num_srv > 0) {
                errno = 0;
                err2 = read(server_to_user[1][0], message_from_s, 128);
                //-1 will return if there was an error, if interrupted by signal, will read until read success
                if (err2 == -1) {
                    if(errno == EINTR) {
                        continue;
                    } else {
                        perror("Error");
                        return 1;
                    }
                } else if (err2 == 0) {
                    --usr2_msg_num_srv;
                    exit(0);
                } else {  
                    //Next two for loops are to mask and differentiate the recieved messages from sent ones 
                    char maskMSG[128];
                    for(int i = 0; i < 128; i++) {
                        maskMSG[i] = 0;
                    }

                    for(int i = 27; i < 128; i++) {
                        maskMSG[i - 27] = message_from_s[i];
                    }

                    printf("user_2_received_msg_from_user_%c: %s\n", message_from_s[5], maskMSG);
                    --usr2_msg_num_srv;
                }
            }
        }
    //User 2 Message Receiving
    } else if(user_id == 2) {
        //recieve_msg(usr3_msg_num_srv, user_id, message_from_s);
        while(1) {
            if(usr3_msg_num_srv > 0) {
                errno = 0;
                err3 = read(server_to_user[2][0], message_from_s, 128);
                //-1 will return if there was an error, if interrupted by signal, will read until read success
                if (err3 == -1) {
                    if(errno == EINTR) {
                        continue;
                    } else {
                        perror("Error");
                        return 1;
                    }
                } else if (err3 == 0) {
                    --usr3_msg_num_srv;
                    exit(0);
                } else {  
                    //Next two for loops are to mask and differentiate the recieved messages from sent ones 
                    char maskMSG[128];
                    for(int i = 0; i < 128; i++) {
                        maskMSG[i] = 0;
                    }

                    for(int i = 27; i < 128; i++) {
                        maskMSG[i - 27] = message_from_s[i];
                    }

                    printf("user_3_received_msg_from_user_%c: %s\n", message_from_s[5], maskMSG);
                    --usr3_msg_num_srv;
                }
            }
        }
    }
    return 0;
}
