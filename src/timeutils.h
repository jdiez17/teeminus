#include <time.h>

time_t calc_unix_seconds(struct tm*);
void calc_diff(struct tm* diff, time_t now, time_t then);
