/* 
 * SPLPv1.c
 * The file is part of practical task for System programming course. 
 * This file contains validation of SPLPv1 protocol. 
 */


//#error Specify your name and group
/*
  Белотелов Евгений Витальевич
  № 14
*/



/*
---------------------------------------------------------------------------------------------------------------------------
# |      STATE      |         DESCRIPTION       |           ALLOWED MESSAGES            | NEW STATE | EXAMPLE
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
1 | INIT            | initial state             | A->B     CONNECT                      |     2     |
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
2 | CONNECTING      | client is waiting for con-| A<-B     CONNECT_OK                   |     3     |
  |                 | nection approval from srv |                                       |           |                      
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
3 | CONNECTED       | Connection is established | A->B     GET_VER                      |     4     |                     
  |                 |                           |        -------------------------------+-----------+----------------------
  |                 |                           |          One of the following:        |     5     |                      
  |                 |                           |          - GET_DATA                   |           |                      
  |                 |                           |          - GET_FILE                   |           |                      
  |                 |                           |          - GET_COMMAND                |           |
  |                 |                           |        -------------------------------+-----------+----------------------
  |                 |                           |          GET_B64                      |     6     |                      
  |                 |                           |        ------------------------------------------------------------------
  |                 |                           |          DISCONNECT                   |     7     |                                 
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
4 | WAITING_VER     | Client is waiting for     | A<-B     VERSION ver                  |     3     | VERSION 2                     
  |                 | server to provide version |          Where ver is an integer (>0) |           |                      
  |                 | information               |          value. Only a single space   |           |                      
  |                 |                           |          is allowed in the message    |           |                      
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
5 | WAITING_DATA    | Client is waiting for a   | A<-B     CMD data CMD                 |     3     | GET_DATA a GET_DATA 
  |                 | response from server      |                                       |           |                      
  |                 |                           |          CMD - command sent by the    |           |                      
  |                 |                           |           client in previous message  |           |                      
  |                 |                           |          data - string which contains |           |                      
  |                 |                           |           the following allowed cha-  |           |                      
  |                 |                           |           racters: small latin letter,|           |                     
  |                 |                           |           digits and '.'              |           |                      
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
6 | WAITING_B64_DATA| Client is waiting for a   | A<-B     B64: data                    |     3     | B64: SGVsbG8=                    
  |                 | response from server.     |          where data is a base64 string|           |                      
  |                 |                           |          only 1 space is allowed      |           |                      
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
7 | DISCONNECTING   | Client is waiting for     | A<-B     DISCONNECT_OK                |     1     |                      
  |                 | server to close the       |                                       |           |                      
  |                 | connection                |                                       |           |                      
---------------------------------------------------------------------------------------------------------------------------

IN CASE OF INVALID MESSAGE THE STATE SHOULD BE RESET TO 1 (INIT)

*/


#include "splpv1.h"
#include "string.h"
#include "stdlib.h"




/* FUNCTION:  validate_message
 * 
 * PURPOSE:  
 *    This function is called for each SPLPv1 message between client 
 *    and server
 * 
 * PARAMETERS:
 *    msg - pointer to a structure which stores information about 
 *    message
 * 
 * RETURN VALUE:
 *    MESSAGE_VALID if the message is correct 
 *    MESSAGE_INVALID if the message is incorrect or out of protocol 
 *    state
 */

enum cur_status
{
    INIT,
    CONNECTING,
    CONNECTED,
    WAITING_VER,
    WAITING_DATA,
    WAITING_B64_DATA,
    DISCONNECTING
};

const char data[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
const char base64[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
                        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
                        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };


const char* commands[] = { "GET_DATA", "GET_FILE", "GET_COMMAND" };

enum cur_status current_status = INIT;
int pre_CMD = -1;

enum test_status validate_message( struct Message *msg )
{
    
    // TODO: Implement me

    char* text_message; 
    text_message = msg->text_message;
    if (msg->direction == A_TO_B) {
       if (current_status == CONNECTED) {
            if (!strcmp(text_message, "GET_FILE")) {
                current_status = WAITING_DATA;
                pre_CMD = 1;
                return MESSAGE_VALID;
            }
            else if (!strcmp(text_message, "GET_DATA")) {
                current_status = WAITING_DATA;
                pre_CMD = 0;
                return MESSAGE_VALID;
            }
            else if (!strcmp(text_message, "GET_COMMAND")) {
                current_status = WAITING_DATA;
                pre_CMD = 2;
                return MESSAGE_VALID;
            }
            else if (!strcmp(text_message, "GET_B64")) {
                current_status = WAITING_B64_DATA;
                return MESSAGE_VALID;
            }
            else if (!strcmp(text_message, "DISCONNECT")) {
                current_status = DISCONNECTING;
                return MESSAGE_VALID;
            }
            else if (!strcmp(text_message, "GET_VER")) {
                current_status = WAITING_VER;
                return MESSAGE_VALID;
            }            
       }
       else if (current_status == INIT) {
           if (!strcmp(text_message, "CONNECT")) {
               current_status = CONNECTING;
               return MESSAGE_VALID;
           }
       }
    }
    else if (current_status == WAITING_DATA && pre_CMD != -1) {
        int sizeCMD = strlen(commands[pre_CMD]);
        if (!strncmp(text_message, commands[pre_CMD], sizeCMD)) {
            int sizeMSG = strlen(text_message);
            char* temp = text_message + sizeMSG;
            if (!strncmp(temp - sizeCMD, commands[pre_CMD], sizeCMD)) {
                if (!strncmp(text_message + sizeCMD, " ", 1)) {
                    if (!strncmp(temp - sizeCMD - 1, " ", 1)) {
                        text_message += sizeCMD + 1;
                        for (; data[*text_message]; ++text_message);
                        if (strlen(text_message + sizeCMD +1) == 0) {
                            current_status = CONNECTED;
                            return MESSAGE_VALID;
                        }
                    }
                }
            }
        }
    }
    else if (current_status == WAITING_B64_DATA) {
        if (!strncmp(text_message, "B64: ", 5)) {
            text_message += 5;
            char* temp = text_message;
            for (; base64[*text_message]; ++text_message);
            char equality = 0;
            for (; (equality < 2) && (text_message[equality] == '='); ++equality);
            if ((text_message - temp + equality) % 4 == 0 && !text_message[equality]) {
                current_status = CONNECTED;
                return MESSAGE_VALID;
            }
        }
    }
    else if (current_status == CONNECTING) {
        if (!strcmp(text_message, "CONNECT_OK")) {
            current_status = CONNECTED;
            return MESSAGE_VALID;
        }
    }
    else if (current_status == DISCONNECTING) {
        if (!strcmp(text_message, "DISCONNECT_OK")) {
            current_status = INIT;
            return MESSAGE_VALID;
        }
    }
    else if (current_status == WAITING_VER) {
        if (!strncmp(text_message, "VERSION ", 8)) {
            text_message += 8;
            if (*text_message > 48 && *text_message < 58) {
                text_message++;
                for (; *text_message != '\0'; text_message ++)
                    /*text_message += 8;
                    int l = strlen(text_message);
                    for (int i = 0; i < l; ++i) {
                        if (!data[*(text_message + i)]) {
                            current_status = INIT;
                            return MESSAGE_INVALID;
                        }
                    }*/
                    if (*text_message < 48 || *text_message > 57) {
                        current_status = INIT;
                        return MESSAGE_INVALID;
                    }
                current_status = CONNECTED;
                return MESSAGE_VALID;
            }
        }
    }
    current_status = INIT;
    return MESSAGE_INVALID;
}

//
//int CurrentState = 1;
//int WhatCommand;
//
//const char base64[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
//
//const char data[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
//
//const char* commands[] = { "GET_DATA", "GET_FILE", "GET_COMMAND" };
//
//
//enum test_status validate_message(struct Message* msg) {
//    char* message = msg->text_message;
//    if (msg->direction == A_TO_B) {
//        if (CurrentState == 1 && !strcmp(message, "CONNECT")) {
//            CurrentState = 2;
//            return MESSAGE_VALID;
//        }
//        else if (CurrentState == 3) {
//            if (!strcmp(message, "GET_VER")) {
//                CurrentState = 4;
//                return MESSAGE_VALID;
//            }
//            else if (!strcmp(message, "GET_DATA")) {
//                WhatCommand = 0;
//                CurrentState = 5;
//                return MESSAGE_VALID;
//            }
//            else if (!strcmp(message, "GET_FILE")) {
//                WhatCommand = 1;
//                CurrentState = 5;
//                return MESSAGE_VALID;
//            }
//            else if (!strcmp(message, "GET_COMMAND")) {
//                WhatCommand = 2;
//                CurrentState = 5;
//                return MESSAGE_VALID;
//            }
//            else if (!strcmp(message, "GET_B64")) {
//                CurrentState = 6;
//                return MESSAGE_VALID;
//            }
//            else if (!strcmp(message, "DISCONNECT")) {
//                CurrentState = 7;
//                return MESSAGE_VALID;
//            }
//        }
//    }
//    else {
//        if (CurrentState == 2 && !strcmp(message, "CONNECT_OK")) {
//            CurrentState = 3;
//            return MESSAGE_VALID;
//        }
//        else if (CurrentState == 7 && !strcmp(message, "DISCONNECT_OK")) {
//            CurrentState = 1;
//            return MESSAGE_VALID;
//        }
//        else if (CurrentState == 4 && !strncmp(message, "VERSION ", 8)) {
//            message += 8;
//            if (*message > 48 && *message < 58) {
//                for (++message; *message != '\0'; message++)
//                    if (*message < 48 || *message > 57) {
//                        CurrentState = 1;
//                        return MESSAGE_INVALID;
//                    }
//                CurrentState = 3;
//                return MESSAGE_VALID;
//            }
//        }
//        else if (CurrentState == 5) {
//            int l = strlen(commands[WhatCommand]);
//            if (!strncmp(message, commands[WhatCommand], l))
//            {
//                message += l;
//                if (*message == ' ')
//                {
//                    message++;
//                    char* s;
//                    for (; data[*message + 128]; ++message);
//                    s = (*message == ' ') ? message + 1 : NULL;
//                    if (s && !strcmp(s, commands[WhatCommand])) {
//                        CurrentState = 3;
//                        return MESSAGE_VALID;
//                    }
//                }
//            }
//        }
//        else if (CurrentState == 6 && !strncmp(message, "B64: ", 5)) {
//            message += 5;
//            char* begin = message;
//            for (; base64[*message + 128]; ++message);
//            char check = 0;
//            for (; (check < 2) && (message[check] == '='); ++check);
//            if ((message - begin + check) % 4 == 0 && !message[check]) {
//                CurrentState = 3;
//                return MESSAGE_VALID;
//            }
//        }
//    }
//    CurrentState = 1;
//    return MESSAGE_INVALID;
//}