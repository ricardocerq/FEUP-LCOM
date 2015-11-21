#ifndef REAL_TIME_CLOCK_H
#define REAL_TIME_CLOCK_H

#define RTC_IRQ 8

#define BIT(n) (0x01<<(n))
#define SEP_BIT(num, n) ((BIT(n)&num)>>n) //returns bit n of num
#define SEP_BITS(num, a, b) ((num & (( (1<<(b-a+1)) - 1)<<a)) >> a) //returns bits a to b of num
#define WAIT_MS(ms) tickdelay(micros_to_ticks(ms*1000))

#define RTC_SET_DONT_CARE(val) (val|0xC0)


#define ORIGINAL_RTC_HOOK_ID 5
#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71

#define HOUR_IS_PM(hour) ((hour & 0x80)? 1 : 0)
#define CLEAR_PM_BIT(hour) (hour &= 0x7F)

/*Addresses*/
#define SECONDS_ADDR 0
#define SECONDS_ALARM_ADDR 1
#define MINUTES_ADDR 2
#define MINUTES_ALARM_ADDR 3
#define HOURS_ADDR 4
#define HOURS_ALARM_ADDR 5
#define DAY_OF_WEEK_ADDR 6
#define DATE_MONTH_ADDR 7
#define MONTH_ADDR 8
#define YEAR_ADDR 9
#define RTC_REG_A 10
#define RTC_REG_B 11
#define RTC_REG_C 12
#define RTC_REG_D 13

#define RTC_DATA_BEGIN 14

#define RTC_MEM_SIZE 114
#define RTC_AVAILABLE_MEM 111 // 1 byte restricted 1 byte reserved for signature, 1 byte reserved for parity
#define RTC_MEM_RANGE 112
#define RTC_RESTRICTED 36 // writing to this position alters year, day of week

#define RTC_DELAY 244


/*register A*/
#define RTC_UIP BIT(7)
#define RTC_DV2 BIT(6)
#define RTC_DV1 BIT(5)
#define RTC_DV0 BIT(4)
#define RTC_RS3 BIT(3)
#define RTC_RS2 BIT(2)
#define RTC_RS1 BIT(1)
#define RTC_RS0 BIT(0)

/*register B*/
#define RTC_SET BIT(7)
#define RTC_PERIODIC_INT_EN BIT(6)
#define RTC_ALARM_INT_EN BIT(5)
#define RTC_UPDATE_ENDED_EN BIT(4)
#define RTC_SQW_EN BIT(3)
#define RTC_DATA_MODE BIT(2)
#define RTC_24_12 BIT(1)
#define RTC_DAYLIGHT_SAV BIT(0)

/*register C*/
#define RTC_IRQF BIT(7)
#define RTC_PERIODIC_INT_FLAG BIT(6)
#define RTC_ALARM_INT_FLAG BIT(5)
#define RTC_UPDATE_ENDED_INT_FLAG BIT(4)

/*register D*/
#define RTC_VALID_RAM_TIME BIT(7)


#endif
