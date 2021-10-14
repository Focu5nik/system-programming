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

enum cur_status current_status = INIT;
char* pre_CMD;

enum test_status validate_message( struct Message *msg )
{
    
    // TODO: Implement me

    enum Direction	direction = msg->direction;
    char* text_message; 
    text_message = msg->text_message;
    strcpy(text_message, msg->text_message);
    if (current_status != WAITING_DATA) {
        pre_CMD = "";
    }
    if (direction == A_TO_B) {
        if (current_status == INIT) {
            if (!strcmp(text_message, "CONNECT")) {
                current_status = CONNECTING;
                pre_CMD = "";
                return MESSAGE_VALID;
            }
        }
        else if (current_status == CONNECTED) {
            if (!strcmp(text_message, "GET_VER")) {
                current_status = WAITING_VER;
                return MESSAGE_VALID;
            }
            else if (!strcmp(text_message, "GET_DATA")) {
                current_status = WAITING_DATA;
                pre_CMD = "GET_DATA";
                return MESSAGE_VALID;
            }
            else if (!strcmp(text_message, "GET_FILE")) {
                current_status = WAITING_DATA;
                pre_CMD = "GET_FILE";
                return MESSAGE_VALID;
            }
            else if (!strcmp(text_message, "GET_COMMAND")) {
                current_status = WAITING_DATA;
                pre_CMD = "GET_COMMAND";
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
        }
    }
    else if (current_status == CONNECTING) {
        if (!strcmp(text_message, "CONNECT_OK")) {
            current_status = CONNECTED;
            return MESSAGE_VALID;
        }
    }
    else if (current_status == WAITING_VER) {
        if (!strncmp(text_message, "VERSION ", 8)) {
            text_message += 8;
            if (*text_message > 48 && *text_message < 58) {
                text_message++;
                for (; *text_message != '\0'; text_message++)
                    if (*text_message < 48 || *text_message > 57) {
                        current_status = INIT;
                        return MESSAGE_INVALID;
                    }
                current_status = CONNECTED;
                return MESSAGE_VALID;
            }            
        }
    }
    else if (current_status == WAITING_DATA && (strcmp(pre_CMD, "") != 0)) {
        int sizeCMD = strlen(pre_CMD);
        int sizeMSG = strlen(text_message);
        if ( !strncmp(text_message, pre_CMD, sizeCMD) &&
             !strncmp(text_message + sizeMSG - sizeCMD, pre_CMD, sizeCMD) &&
             !strncmp(text_message + sizeCMD, " ", 1) &&
             !strncmp(text_message + sizeMSG - sizeCMD - 1, " ", 1) ) {
            text_message += sizeCMD + 1;
            int l = strlen(text_message) - sizeCMD * 2 - 2;
            for (int i = 0; i < l; ++i) {
                if (!data[*(text_message + i)]) {
                    current_status = INIT;
                    return MESSAGE_INVALID;
                }
                /*if ( !((*(text_message + i) > 96 && *(text_message + i) < 123)) && 
                     !((*(text_message + i) > 47 && *(text_message + i) < 58))  &&
                     (*(text_message + i) != 46) ){
                    current_status = INIT;
                    return MESSAGE_INVALID;
                }*/
            }
            current_status = CONNECTED;
            return MESSAGE_VALID;         
        }
    }
    else if (current_status ==  WAITING_B64_DATA) {
        if (!strncmp(text_message, "B64: ", 5)) {
            text_message += 5;
            int sizeMSG = strlen(text_message);
            if (sizeMSG % 4 == 0) {
                if (!strncmp(text_message + sizeMSG - 2, "=", 1) ) {
                    if (!strncmp(text_message + sizeMSG - 1, "=", 1)) {
                        sizeMSG -= 2;
                    }
                }
                else if ( !strncmp(text_message + sizeMSG - 1, "=", 1) ) {
                    sizeMSG--;
                }
                for (int i = 0; i < sizeMSG; ++i) {
                    if (!base64[*(text_message + i)]) {
                        current_status = INIT;
                        return MESSAGE_INVALID;
                    }
                    /*if (!((*(text_message + i) > 96 && *(text_message + i) < 123)) &&
                        !((*(text_message + i) > 64 && *(text_message + i) < 91)) &&
                        !((*(text_message + i) > 47 && *(text_message + i) < 58)) &&
                        (*(text_message + i) != 43) &&
                        (*(text_message + i) != 47)) {
                        current_status = INIT;
                        return MESSAGE_INVALID;
                    }*/
                }                
                current_status = CONNECTED;
                return MESSAGE_VALID;
            }            
        }
    }
    else if (current_status == DISCONNECTING) {
        if (!strcmp(text_message, "DISCONNECT_OK")) {
            current_status = INIT;
            return MESSAGE_VALID;
        }
    } 
    current_status = INIT;
    return MESSAGE_INVALID;
}
