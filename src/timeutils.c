#include "timeutils.h"

// From https://github.com/jerith/pebble-beapoch
time_t calc_unix_seconds(struct tm *tick_time) {
    // This function is necessary because mktime() doesn't work (probably
    // because there's no native timezone support) and just calling time()
    // gives us no guarantee that the seconds haven't changed since tick_time
    // was made.

    int years_since_epoch;
    int days_since_epoch;

    // This uses a naive algorithm for calculating leap years and will
    // therefore fail in 2100.
    years_since_epoch = tick_time->tm_year - 70;
    days_since_epoch = (years_since_epoch * 365 +
                        (years_since_epoch + 1) / 4 +
                        tick_time->tm_yday);
    return (days_since_epoch * 86400 +
            tick_time->tm_hour * 3600 +
            tick_time->tm_min * 60 +
            tick_time->tm_sec);
}

void calc_diff(struct tm* diff, time_t now, time_t then) {
    time_t unix_difference = (then > now) ? then - now : now - then; // count down or up
    diff->tm_hour = unix_difference / 3600;
    diff->tm_min  = (unix_difference % 3600) / 60;
    diff->tm_sec  = unix_difference % 60;
}
