#ifndef FORCE_H
#define FORCE_H


#define LOADCELL_DOUT_PIN       7
#define LOADCELL_SCK_PIN        6

void init_scale();
void start_capture();
void end_capture();
void get_version();
void get_calibration_factor();
void set_calibration_factor(String inputString);
void calibrate(String inputString);
void tare();
void get_tare();
void set_tare(String inputString);
String get_command_argument(String inputString);
void get_transmission_format();
bool is_capturing();
void scale_loop();
void stop_capturing();


#endif