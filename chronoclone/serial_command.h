#ifndef SERIAL_COMMAND_H
#define SERIAL_COMMAND_H

#define PORT_SCANNING             'J'
#define GET_VERSION               'V'
#define GET_DEBOUNCE_TIME         'a'
#define SET_DEBOUNCE_TIME         'b'
#define GET_STATUS                'E'
#define FRAME_CHANGE_START        'X'
#define RESPONSE_ERROR            '-1'
#define C1                        'r'
#define C2                        'R'
#define C3                        's'
#define C4                        'S'
#define C5                        't'
#define C6                        'T'
#define C7                        'u'
#define C8                        'U'
#define C9                        'y'
#define C10                       'Y'
#define C11                       'w'
#define C12                       'W'
#define C13                       'x'
#define C14                       'X'
#define C15                       'Z'
//#define CHANGEBAUDRATE9600        'c'
//#define CHANGEBAUDRATE115200      'C'


#define UART_WAITING_COMMAND        0
#define UART_WAITING_DEBOUNCEVALUE  1

//#define BAUDRATE9600                0
//#define BAUDRATE115200              1


#endif