#include "rtc.h"
#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/com.h>
#include <stdbool.h>

static rtc_time_t current_time;
static bool time_is_updated = false;
static bool time_is_valid = false;
static int rtc_hook_id = ORIGINAL_RTC_HOOK_ID;
static bool bcdset = false;
static bool hour12 = false;
static int(*rtc_alarm_action)() = NULL;
static int(*rtc_periodic_action)() = NULL;
static unsigned char* rtc_saved_data = NULL;

unsigned char* get_rtc_saved_data()
{
	return rtc_saved_data;
}
int read_rtc_data()
{
	if(rtc_saved_data == NULL)
	{
		rtc_saved_data = malloc(RTC_MEM_SIZE*sizeof(unsigned char));
		if(rtc_saved_data == NULL)
			return 1;
	}
	size_t i;
	for(i = 0; i < RTC_MEM_SIZE ; i++)
	{
		if(read_rtc(RTC_DATA_BEGIN + i, (unsigned long *) (rtc_saved_data+i)))
			return 1;
	}
	return 0;
}
int commit_rtc_data(unsigned char signature)
{
	if(rtc_saved_data == NULL)
		return 1;
	size_t i, accessi;
	unsigned char parity = 0;
	for(i = 0, accessi = 0; accessi < RTC_AVAILABLE_MEM+1; i++, accessi++)
	{
		if(i == RTC_RESTRICTED)
			{continue;}
		parity ^= *(rtc_saved_data+accessi);
		if(write_rtc(RTC_DATA_BEGIN + i, *(rtc_saved_data+accessi)))
			return 1;
	}
	parity ^= signature;
	if(write_rtc(RTC_DATA_BEGIN + i, signature))
		return 1;
	i++;
	if(write_rtc(RTC_DATA_BEGIN + i, parity))
		return 1;
	return 0;
}
int write_rtc_data(unsigned char* buf, size_t numbytes)
{
	if(rtc_saved_data == NULL)
	{
		rtc_saved_data = malloc(RTC_MEM_SIZE*sizeof(unsigned char));
		if(rtc_saved_data == NULL)
			return 1;
	}
	size_t i, accessi;
	for(i = 0, accessi = 0; accessi < RTC_AVAILABLE_MEM && i < numbytes; i++, accessi++)
	{
		if(i == RTC_RESTRICTED)
		{accessi--; continue;}
		rtc_saved_data[i] = buf[accessi];
	}
	return 0;
}
void print_rtc_data()
{
	if(rtc_saved_data == NULL)
		return;
	size_t i;
	for(i = 0; i < RTC_MEM_SIZE ; i++)
	{
		printf("%4u ", *(rtc_saved_data+i));
	}
}
bool verify_rtc_data_source(unsigned char signature)
{
if(rtc_saved_data == NULL)
		return false;
	if(rtc_saved_data[RTC_MEM_SIZE-2] != signature)
		return false;
	size_t i;
	unsigned char parity = 0;
	for(i = 0; i < RTC_MEM_SIZE - 2; i++)
	{
		if(i == RTC_RESTRICTED)
			continue;
		parity ^= *(rtc_saved_data+i);
	}
	parity ^= signature;
	if(parity == rtc_saved_data[RTC_MEM_SIZE- 1])
		return true;
	return false;
}
void set_rtc_alarm_action(int (*new_rtc_alarm_action)() )
{
	rtc_alarm_action = new_rtc_alarm_action;
}
void set_rtc_periodic_action(int (*new_rtc_periodic_action)() )
{
	rtc_periodic_action = new_rtc_periodic_action;
}
void getDateString(char ** str)
{
	char* month;
	switch(current_time.month)
	{
	case 1:
		month = "Jan";
		break;
	case 2:
		month = "Feb";
		break;
	case 3:
		month = "Mar";
		break;
	case 4:
		month = "Apr";
		break;
	case 5:
		month = "May";
		break;
	case 6:
		month = "Jun";
		break;
	case 7:
		month = "Jul";
		break;
	case 8:
		month = "Aug";
		break;
	case 9:
		month = "Sep";
		break;
	case 10:
		month = "Oct";
		break;
	case 11:
		month = "Nov";
		break;
	case 12:
		month = "Dec";
		break;
	default:
		break;
	}
	char* day_of_the_week;
	switch(current_time.day_of_the_week)
	{
	case 1:
		day_of_the_week = "Thu";
		break;
	case 2:
		day_of_the_week = "Fri";
		break;
	case 3:
		day_of_the_week = "Sat";
		break;
	case 4:
		day_of_the_week = "Sun";
		break;
	case 5:
		day_of_the_week = "Mon";
		break;
	case 6:
		day_of_the_week = "Tue";
		break;
	case 7:
		day_of_the_week = "Wed";
		break;
	default:
		break;
	}
	if(* str == NULL)
	*str = (char *) malloc(sizeof(char) * 25);
	sprintf(*str, "%s %s %2d %02d:%02d:%02d 20%02d",
			day_of_the_week,
			month,
			current_time.day_of_the_month,
			current_time.hours,
			current_time.minutes,
			current_time.seconds,
			current_time.year);
}
unsigned long getTimeSeconds()
{
	if(!time_is_valid)
		if(initDate())
			return 0;
	return
			(current_time.year+30) * 365.25 * 24 * 60 * 60 +
			(current_time.month-1) * 30.4375 * 24 * 60 * 60 +
			(current_time.day_of_the_month - 1) * 24 * 60 * 60 +
			current_time.hours * 60 * 60 +
			current_time.minutes * 60 +
			current_time.seconds -70200;
}
unsigned char bcd_to_binary(unsigned char bcdvalue)
{
	char buffer[32];
	sprintf(buffer, "%x", bcdvalue);
	sscanf(buffer, "%d", &bcdvalue);
	return bcdvalue;
}
unsigned char binary_to_bcd(unsigned char binaryvalue)
{
	char buffer[32];
	sprintf(buffer, "%d", binaryvalue);
	sscanf(buffer, "%x", &binaryvalue);
	return binaryvalue;
}
int initDate()
{
	if(time_is_valid)
		return 0;
	unsigned long regB;
	if(read_rtc(RTC_REG_B, &regB))
	{
		printf("rtc:: Error reading Register B\n");
		return 1;
	}
	if(regB & RTC_DATA_MODE)
		bcdset =false;
	bcdset = true;
	if(regB & RTC_24_12)
		hour12 = false;
	hour12 = true;
	return forced_read_date();
}
void process_time(void)
{
	bool pm = false;
	if(hour12)
	{
		if(HOUR_IS_PM(current_time.hours))
			pm = true;
		CLEAR_PM_BIT(current_time.hours);
	}
	if(bcdset)
	{
		current_time.seconds =  bcd_to_binary(current_time.seconds);
		current_time.minutes = bcd_to_binary(current_time.minutes);
		current_time.hours = bcd_to_binary(current_time.hours);
		current_time.day_of_the_week = bcd_to_binary(current_time.day_of_the_week);
		current_time.day_of_the_month = bcd_to_binary(current_time.day_of_the_month);
		current_time.month = bcd_to_binary(current_time.month);
		current_time.year = bcd_to_binary(current_time.year);
	}
	if(pm)
		current_time.hours += 12;
	current_time.hours %= 24;
}
rtc_time_t getTime(void)
{
	time_is_updated = false;
	return current_time;
}
int getTime_is_updated(void)
{
	return time_is_updated;
}
int getTime_is_valid(void)
{
	return time_is_valid;
}
int read_rtc(unsigned long index, unsigned long *value)
{
	if(sys_outb(RTC_ADDR_REG, index) != OK)
		return 1;
	if(sys_inb(RTC_DATA_REG, value) != OK)
		return 1;
	return 0;
}
int write_rtc(unsigned long index, unsigned long value)
{
	if(sys_outb(RTC_ADDR_REG, index) != OK)
		return 1;
	if(sys_outb(RTC_DATA_REG, value) != OK)
		return 1;
	return 0;
}
int rtc_conf()
{
	unsigned long regA;
	unsigned long regB;
	unsigned long regC;
	unsigned long regD;
	if(read_rtc(RTC_REG_A, &regA))
	{
		printf("rtc:: Error reading Register A\n");
		return 1;
	}
	if(read_rtc(RTC_REG_B, &regB))
	{
		printf("rtc:: Error reading Register B\n");
		return 1;
	}
	if(read_rtc(RTC_REG_C, &regC))
	{
		printf("rtc:: Error reading Register B\n");
		return 1;
	}
	if(read_rtc(RTC_REG_D, &regD))
	{
		printf("rtc:: Error reading Register A\n");
		return 1;
	}
	printf("Register A:\n");
	printf("UIP : %d\n", (regA & RTC_UIP)?1:0);
	printf("Register B:\n");
	printf("SET : %d\n", (regB & RTC_SET)?1:0);
	printf("PIE : %d ", (regB & RTC_PERIODIC_INT_EN)?1:0);
	if(regB & RTC_PERIODIC_INT_EN)
		printf("Periodic Interrupts enabled\n");
	else printf("Periodic Interrupts disabled\n");

	printf("AIE : %d ", (regB & RTC_ALARM_INT_EN)?1:0);
	if(regB & RTC_ALARM_INT_EN)
		printf("Alarm Interrupts enabled\n");
	else printf("Alarm Interrupts disabled\n");

	printf("UIE : %d ", (regB & RTC_UPDATE_ENDED_EN)?1:0);
	if(regB & RTC_UPDATE_ENDED_EN)
		printf("Update Ended Interrupts enabled\n");
	else printf("Update Ended Interrupts disabled\n");

	printf("SQWE : %d\n", (regB & RTC_SQW_EN)?1:0);
	printf("DM : %d ", (regB & RTC_DATA_MODE)?1:0);
	if(regB & RTC_DATA_MODE)
		printf("Binary mode\n");
	else printf("BCD mode \n");
	printf("24/12 : %d", (regB & RTC_24_12)?1:0);
	if(regB & RTC_24_12)
		printf("Hours range from 0 to 23\n");
	else printf("Hours range from 1 to 12\n");
	printf("Daylight savings : %d\n", (regB & RTC_DAYLIGHT_SAV)?1:0);
	printf("Valid RAM : %d\n", (regD & RTC_VALID_RAM_TIME)?1:0);
	printf("test: %d\n", bcd_to_binary(0x99));
	return 0;
}
//subscribe rtc interrupts
int rtc_subscribe_int(void)
{
	initDate();
	int returnvalue = BIT(rtc_hook_id);
	int irqpolicy = IRQ_EXCLUSIVE | IRQ_REENABLE;
	if(sys_irqsetpolicy(RTC_IRQ, irqpolicy, &rtc_hook_id)!= OK){
		printf("rtc_subscribe_int(): sys_irqsetpolicy() failed\n");
		return -1;
	}
	if(sys_irqenable(&rtc_hook_id)!=OK){
		printf("rtc_subscribe_int(): sys_irqenable() failed\n");
		return -1;
	}
	unsigned long regC;
	if(read_rtc(RTC_REG_C, &regC))//reset interrupts
		return -1;
	return returnvalue;//return value with bit number (original hook_id) set to 1
}
////unsubscribe rtc interrupts
int rtc_unsubscribe_int(void) {
	if(sys_irqdisable(&rtc_hook_id) != OK){
		printf("rtc_unsubscribe_int(): sys_irqdisable() failed\n");
		return 1;
	}
	if(sys_irqrmpolicy(&rtc_hook_id) != OK){
		printf("rtc_unsubscribe_int(): sys_irqrmpolicy() failed\n");
		return 1;
	}
	return clear_int_bits();
}
int read_date()
{
	unsigned long regB;
	unsigned long saved;
	if(read_rtc(RTC_REG_B, &regB))
		return 1;
	saved = regB;
	regB |= RTC_SET;
	if(write_rtc(RTC_REG_B, regB))
		return 1;
	if(read_rtc(SECONDS_ADDR, &current_time.seconds))
		return 1;
	if(read_rtc(MINUTES_ADDR,&current_time.minutes))
		return 1;
	if(read_rtc(HOURS_ADDR,&current_time.hours))
		return 1;
	if(read_rtc(DAY_OF_WEEK_ADDR,&current_time.day_of_the_week))
		return 1;
	if(read_rtc(DATE_MONTH_ADDR,&current_time.day_of_the_month))
		return 1;
	if(read_rtc(MONTH_ADDR,&current_time.month))
		return 1;
	if(read_rtc(YEAR_ADDR, &current_time.year))
		return 1;
	saved &= ~RTC_SET;
	if(write_rtc(RTC_REG_B, saved))
		return 1;
	process_time();
	time_is_updated = true;
	time_is_valid = true;
	return 0;
}
int forced_read_date()
{
	bool can_read = false;
	int attempts = 100;
	unsigned long regA;
	do
	{
		if(read_rtc(RTC_REG_A, &regA))
			return 1;
		if(!(regA & RTC_UIP)) // not updating
		{
			return read_date();
		}
		attempts--;
		WAIT_MS(50);//update in progress, wait
	}while(!can_read && attempts > 0);
	return 1;
}
int handle_update_int()
{
	return read_date();
}
int handle_alarm_int()
{
	if(rtc_alarm_action == NULL)
		return 1;
	return rtc_alarm_action();
}
int handle_periodic_int()
{
	if(rtc_periodic_action == NULL)
		return 1;
	return rtc_periodic_action();
}
int rtc_ih(void) {
	unsigned long regC;
	int failure = 0;
	if(read_rtc(RTC_REG_C, &regC))
		return 1;
	if(regC & RTC_UPDATE_ENDED_INT_FLAG )
		if(handle_update_int())
			failure = 1;
	if(regC & RTC_ALARM_INT_FLAG)
		if(handle_alarm_int())
			failure = 1;
	if(regC & RTC_PERIODIC_INT_FLAG)
		if(handle_periodic_int())
			failure = 1;
	return failure;
}
int enable_update_ended_int()
{
	unsigned long regB;
	if(read_rtc(RTC_REG_B, &regB))
		return 1;
	regB |= RTC_UPDATE_ENDED_EN;
	if(write_rtc(RTC_REG_B, regB))
		return 1;
}
int clear_int_bits()
{
	unsigned long regB;
	if(read_rtc(RTC_REG_B, &regB))
		return 1;

	regB &= ~(RTC_PERIODIC_INT_EN| RTC_ALARM_INT_EN|RTC_UPDATE_ENDED_EN);
	if(write_rtc(RTC_REG_B, regB))
		return 1;
}
int enable_per_second_alarm(int(*new_action)())
{
	rtc_alarm_action = new_action;
	unsigned long regB;
	if(read_rtc(RTC_REG_B, &regB))
		return 1;
	regB |= RTC_ALARM_INT_EN;
	if(write_rtc(RTC_REG_B, regB))
		return 1;
	unsigned long sec_alarm;
	if(read_rtc(SECONDS_ALARM_ADDR, &sec_alarm))
		return 1;
	sec_alarm = RTC_SET_DONT_CARE(sec_alarm);
	if(write_rtc(SECONDS_ALARM_ADDR, sec_alarm))
		return 1;

	unsigned long min_alarm;
	if(read_rtc(MINUTES_ALARM_ADDR, &min_alarm))
		return 1;
	min_alarm = RTC_SET_DONT_CARE(min_alarm);
	if(write_rtc(MINUTES_ALARM_ADDR, min_alarm))
		return 1;

	unsigned long hour_alarm;
	if(read_rtc(HOURS_ALARM_ADDR, &hour_alarm))
		return 1;
	hour_alarm = RTC_SET_DONT_CARE(hour_alarm);
	if(write_rtc(HOURS_ALARM_ADDR, hour_alarm))
		return 1;
	return 0;
}

int disable_per_second_alarm()
{
	rtc_alarm_action = NULL;
	unsigned long regB;
	if(read_rtc(RTC_REG_B, &regB))
		return 1;
	regB &= ~(RTC_ALARM_INT_EN);
	if(write_rtc(RTC_REG_B, regB))
		return 1;
}


