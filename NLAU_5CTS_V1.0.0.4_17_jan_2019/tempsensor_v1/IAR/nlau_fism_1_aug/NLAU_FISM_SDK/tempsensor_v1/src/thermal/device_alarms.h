






#define BATTERY_JSON    1
#define SENSOR_JSON     2

void check_sensor_alarm();
void send_temp_alarm(int sensor);
void check_time_alarm();
void alarm_packet(char *alarm_type);
void read_alarm_packets();
void check_battery_alarm();