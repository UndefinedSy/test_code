#include <assert.h>
#include <cstdio>
#include <iostream>
#include <regex>
#include <string>

/**
 * Check whether the input date is valid.
 * @param Date Target Datetime string to verify.
 * @return true if the format is valid. 
 */
static bool
IsDateValid(const std::string& Date)
{
    std::regex DateRegex("[0-9]{4}-[0-9]{1,2}-[0-9]{1,2}");
    return std::regex_match(Date, DateRegex);
}

/**
 * Check if the input year is a leap year.
 * @param Year Target year to verify.
 * @return true if the input year is a leap year. 
 */
static bool
IsLeapYear(int Year)
{
    if (Year % 4 == 0 
        && Year % 100 != 0 
        || Year % 400 == 0)
		return true;
	return false;
}

/**
 * Get the day count after the change of the month.
 * * Will not change `Day` if it is less than the DayCount in target month. (e.g. 2021-02-15 ==> 2021-03-15)
 * * Will set `Day` to the last day of the target month otherwise. (e.g. 2021-02-28 ==> 2021-03-31)
 * @param Year Year after the MonthOffset.
 * @param Month Month after the MonthOffset.
 * @param Day The original input Day.
 * @return The day count after the change of the month.
 */
static int
GetDay(int Year, int Month, int Day)
{
    static int DayCounts[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (Day < DayCounts[Month])
        return Day;

    if (Month == 2 && IsLeapYear(Year))
        return 29;
    return DayCounts[Month];
}

/**
 * Calculate the Datetime after an offset of month.
 * @param DateBase The base datetime to calc.
 * @param MonthOffset The offset of the month.
 * @param Result(OUT param)
 *  * The target datetime if no error occurred.
 *  * Error message if something wrong.
 *      * MonthOffset < 0
 *      * DateBase is invalid
 * @return True if there is no error occurred.
 */
bool
CalcDateWithMonthOffset(const std::string& DateBase, int MonthOffset,
                        std::string& Result)
{
    if (MonthOffset <= 0)
    {
        Result = "The param `MonthOffset` should be > 0.";
        return false;
    }

    if (!IsDateValid(DateBase))
    {
        Result = "The param `DateBase` should be formatted as \"YYYY-MM-DD\".";
        return false;
    }

    int Year, Month, Day;
    sscanf(DateBase.c_str(), "%d-%d-%d", &Year, &Month, &Day);

    Month += MonthOffset;
    if (Month > 12)
    {
        Year += Month / 12;
        Month %= 12;
    }

    Day = GetDay(Year, Month, Day);

    char Res[11];
    sprintf(Res, "%d-%d-%d", Year, Month, Day);
    Result = Res;
    return true;
}

int main()
{
    std::string Result;

    printf("Input Param: {DateBase: %s, MonthOffset: %d}", "2019-4-15", 13);
    assert(CalcDateWithMonthOffset("2019-4-15", 13, Result));
    printf(" Return: %s\n", Result.c_str());

    printf("Input Param: {DateBase: %s, MonthOffset: %d}", "2019-1-31", 1);
    assert(CalcDateWithMonthOffset("2019-1-31", 1, Result));
    printf(" Return: %s\n", Result.c_str());

    printf("Input Param: {DateBase: %s, MonthOffset: %d}", "2019-1-31", 2);
    assert(CalcDateWithMonthOffset("2019-1-31", 2, Result));
    printf(" Return: %s\n", Result.c_str());

    printf("Input Param: {DateBase: %s, MonthOffset: %d}", "2019-1-31", 3);
    assert(CalcDateWithMonthOffset("2019-1-31", 3, Result));
    printf(" Return: %s\n", Result.c_str());

    printf("Input Param: {DateBase: %s, MonthOffset: %d}", "2019-12-31", 2);
    assert(CalcDateWithMonthOffset("2019-12-31", 2, Result));
    printf(" Return: %s\n", Result.c_str());

    printf("Input Param: {DateBase: %s, MonthOffset: %d}", "2019-12-31", 26);
    assert(CalcDateWithMonthOffset("2019-12-31", 26, Result));
    printf(" Return: %s\n", Result.c_str());

    printf("Input Param: {DateBase: %s, MonthOffset: %d}", "d2019-4-15", 1);
    assert(!CalcDateWithMonthOffset("d2019-4-15", 1, Result));
    printf(" Return: %s\n", Result.c_str());

    printf("Input Param: {DateBase: %s, MonthOffset: %d}", "2019-4-15", -1);
    assert(!CalcDateWithMonthOffset("2019-4-15", -1, Result));
    printf(" Return: %s\n", Result.c_str());

    return 0;
}