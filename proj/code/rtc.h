#ifndef RTC_H
#define RTC_H

#include "real_time_clock.h"
#include <stdbool.h>
#include <stdlib.h>

/**
 * @defgroup rtc rtc
 * @{
 *
 * implementation of RTC functionality
 */

typedef struct
{
	unsigned long seconds;
	unsigned long minutes;
	unsigned long hours;
	unsigned long day_of_the_week;
	unsigned long day_of_the_month;
	unsigned long month;
	unsigned long year;

}rtc_time_t;


///@brief get the data that is saved to the rtc
///@return ptr to data
unsigned char* get_rtc_saved_data();

///@brief read rtc data
///@return zero on sucess, non-zero otherwise
int read_rtc_data();

///@brief commit the data to the rtc
///@param signature identifier of calling program
///@return zero on sucess, non-zero otherwise
int commit_rtc_data(unsigned char signature);

///@brief write data to rtc module
///@param buf buffer contain the data
///@param numbytes number of bytes written
///@return zero on sucess, non-zero otherwise
int write_rtc_data(unsigned char* buf, size_t numbytes);

///@brief print rtc data on the screen
void print_rtc_data();

///@brief verifies that the data in the rtc was written by calling program by verifying signature and parity
///@param signature signature of calling program
///@return true if data contained in the rtc was written by the calling program
bool verify_rtc_data_source(unsigned char signature);

///@brief set the rtc action that the rtc will perform on alarm
///@param new_rtc_alarm_action new action to perform on a rtc alarm interrupt
void set_rtc_alarm_action(int (*new_rtc_alarm_action)() );

///@brief set the rtc action that the rtc will perform on priodic int
///@param new_rtc_periodic_action new action to perform on a rtc periodic interrupt
void set_rtc_periodic_action(int (*new_rtc_periodic_action)() );

///@brief get the date in a string
///@param str return valur
void getDateString(char ** str);

///@brief get time in seconds
///@return time in secnds
unsigned long getTimeSeconds();

///@brief bcd to binary
///@param bcdvalue value to convert
///@return binary value
unsigned char bcd_to_binary(unsigned char bcdvalue);

///@brief initialize the date
///@return zero on success, non-zero otherwise
int initDate();

///@brief process the values read from the rtc
///if values are in BCD, convert to binary, if in 12 mode convert to 24
void process_time(void);

///@brief get current time
///@return rtc time
rtc_time_t getTime(void);

///@brief indicates whether time has been updated, since it was read
///@return time has been updated
int getTime_is_updated(void);

///@brief indicates whether the time is valid
///@return validity of time
int getTime_is_valid(void);

///@brief read from rtc
///@param index register to read
///@param value return value
///@return zero on success, non-zero otherwise
int read_rtc(unsigned long index, unsigned long *value);

///@brief write to rtc
///@param index register to write
///@param value value to write
///@return zero on success, non-zero otherwise
int write_rtc(unsigned long index, unsigned long value);

///@brief print the rtc's configuation
///@return zero on success, non-zero otherwise
int rtc_conf();

///@brief subscribe rtc interrupts
///@return irq_set on success else negative value
int rtc_subscribe_int(void);

///@brief unsubscribe rtc interrupts
///@return zero on success, non-zero otherwise
int rtc_unsubscribe_int(void);

///@brief read the date, without checking UIP bit
///@return zero on success, non-zero otherwise
int read_date();

///@brief forced read date, checking UIP periodically
///@return zero on success, non-zero otherwise
int forced_read_date();

///@brief update interruption handler
///@return zero on success, non-zero otherwise
int handle_update_int();

///@brief alarm interruption handler
///@return zero on success, non-zero otherwise
int handle_alarm_int();

///@brief alarm periodic handler
///@return zero on success, non-zero otherwise
int handle_periodic_int();

///@brief generic rtc interrupt handler
///@return zero on success, non-zero otherwise
int rtc_ih(void);

///@brief enable update ended interrupts
///@return zero on success, non-zero otherwise
int enable_update_ended_int();

///@brief clear all interrupt bits
///@return zero on success, non-zero otherwise
int clear_int_bits();

///@brief enable the rtc's alarm on a per second basis
///@param new_action action to perform on a rtc alarm
///@return success of operation
int enable_per_second_alarm(int(*new_action)());

///@brief disable the rtc's alarm ints
///@return success of operation
int disable_per_second_alarm();

/** @} end of cahr_queue */

#endif
