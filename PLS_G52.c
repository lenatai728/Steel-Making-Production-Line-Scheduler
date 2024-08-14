#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/select.h>

#define resetColor "\033[0m"
#define userColor "\033[1;96m"    // 藍色
#define errorColor "\033[1;31m"   // 紅色
#define successColor "\033[1;32m" // 綠色
#define B "\033[1m"               // 粗體
#define resetB "\033[0m"          // 取消粗體

#pragma region DataStructure
typedef enum
{
    Product_A,
    Product_B,
    Product_C,
    Product_D,
    Product_E,
    Product_F,
    Product_G,
    Product_H,
    Product_I,
    NA
} ProductName;

// 算法類型
typedef enum
{
    FCFS,
    PR,
    SJF,
    Private
} AlgorithmType;
// 工廠
typedef enum
{
    Factory_X,
    Factory_Y,
    Factory_Z
} FactoryName;

typedef struct
{
    int year;
    int month;
    int day;
} Date;

typedef struct
{
    char orderID[6];     // 訂單編號
    Date deliveryDate;   // 交貨日期
    int quantity;        // 數量
    ProductName product; // 產品名稱·
} Order;

// 單日訂單
typedef struct
{
    char orderID[6];     // 訂單編號
    int quantity;        // 數量
    ProductName product; // 產品名稱
    Date dueDate;        // 交貨日期
} DailyOrder;

typedef struct
{
    FactoryName factory;          // 工廠
    int dayCount;                 // 總天數
    DailyOrder dailyOrders[9999]; // 每日訂單
} soloFactorySchedule;

typedef struct
{
    Date period[2];                         // 週期
    AlgorithmType algorithm;                // 算法
    soloFactorySchedule factorySchedule[3]; // 三個工廠
} Schedule;

// input2scheduling
typedef struct
{
    Date period[2];          // 週期
    AlgorithmType algorithm; // 算法
    Order orders[9999];      // 訂單
    int orderCount;          // 訂單數量
    char fileName[100];      // 文件名稱
} input2schedulingData;

typedef struct
{
    int xDay;             // 工廠X的天數
    int yDay;             // 工廠Y的天數
    int zDay;             // 工廠Z的天數
    int xProduct;         // 工廠X的產品數量
    int yProduct;         // 工廠Y的產品數量
    int zProduct;         // 工廠Z的產品數量
    float xUtilization;   // 工廠X的利用率
    float yUtilization;   // 工廠Y的利用率
    float zUtilization;   // 工廠Z的利用率
    float allUtilization; // 總利用率
} Performance;

typedef struct
{
    char orderID[6]; // 訂單編號
    int ifX;         // 是否有工廠X
    int ifY;         // 是否有工廠Y
    int ifZ;         // 是否有工廠Z
    Date xStartDay;  // 工廠X開始日期
    Date yStartDay;  // 工廠Y開始日期
    Date zStartDay;  // 工廠Z開始日期
    Date xEndDay;    // 工廠X結束日期
    Date yEndDay;    // 工廠Y結束日期
    Date zEndDay;    // 工廠Z結束日期
    int xQuantity;   // 工廠X的數量
    int yQuantity;   // 工廠Y的數量
    int zQuantity;   // 工廠Z的數量
} Accept;

#pragma endregion

// 儲存區域
Date wholePeriod[2] = {{0, 0, 0}, {0, 0, 0}}; // 整個週期
Order orders[9999];                           // 訂單
int orderCount = 0;                           // 訂單數量

// 計算兩個日期之間的天數,包括首尾兩天, -1表示日期錯誤
int daysBetween(Date date1, Date date2)
{
    int MonthDays[2][12] = {
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, // 平年
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}  // 閏年
    };

    int YearDays[2] = {365, 366};

    if (date1.year > date2.year || (date1.year == date2.year && date1.month > date2.month) || (date1.year == date2.year && date1.month == date2.month && date1.day > date2.day))
    {
        return -1;
    }
    if (date1.year == date2.year)
    {
        int d1 = 0, d2 = 0; // d1為date1的在該年的第幾天，d2為date2的在該年的第幾天
        for (int i = 1; i < date1.month; i++)
        {
            d1 += MonthDays[(date1.year % 4 == 0 && date1.year % 100 != 0) || date1.year % 400 == 0][i - 1];
        }
        d1 += date1.day;

        for (int i = 1; i < date2.month; i++)
        {
            d2 += MonthDays[(date2.year % 4 == 0 && date2.year % 100 != 0) || date2.year % 400 == 0][i - 1];
        }
        d2 += date2.day;

        return d2 - d1 + 1;
    }
    else
    {
        int d1 = 0, d2 = 0, d3 = 0; // d1為date1的在該年的第幾天，d2為date2的在該年的第幾天
        for (int i = date1.month + 1; i <= 12; i++)
        {
            d1 += MonthDays[(date1.year % 4 == 0 && date1.year % 100 != 0) || date1.year % 400 == 0][i - 1];
        }
        d1 += MonthDays[(date1.year % 4 == 0 && date1.year % 100 != 0) || date1.year % 400 == 0][date1.month - 1] - date1.day;

        for (int i = 1; i < date2.month; i++)
        {
            d2 += MonthDays[(date2.year % 4 == 0 && date2.year % 100 != 0) || date2.year % 400 == 0][i - 1];
        }
        d2 += date2.day;

        for (int i = date1.year + 1; i < date2.year; i++)
        {
            d3 += YearDays[(i % 4 == 0 && i % 100 != 0) || i % 400 == 0];
        }

        return d1 + d2 + d3 + 1;
    }
}
Date addDays(Date date, int days)
{
    Date newDate = date;
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (date.year % 4 == 0 && (date.year % 100 != 0 || date.year % 400 == 0))
    {
        daysInMonth[1] = 29;
    }

    newDate.day += days;

    while (newDate.day > daysInMonth[newDate.month - 1])
    {
        newDate.day -= daysInMonth[newDate.month - 1];
        newDate.month++;

        if (newDate.month > 12)
        {
            newDate.month = 1;
            newDate.year++;

            if (newDate.year % 4 == 0 && (newDate.year % 100 != 0 || newDate.year % 400 == 0))
            {
                daysInMonth[1] = 29;
            }
            else
            {
                daysInMonth[1] = 28;
            }
        }
    }

    return newDate;
}
Date newDate(Date originalDate, int days)
{
    if (days < 0)
    {
        return originalDate;
    }

    Date newDate = originalDate;
    for (int i = 0; i < days; i++)
    {
        int MonthDays[2][12] = {
            {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, // 平年
            {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}  // 閏年
        };
        int year = newDate.year;
        int month = newDate.month;
        int day = newDate.day;
        if (day < MonthDays[(year % 4 == 0 && year % 100 != 0) || year % 400 == 0][month - 1])
        {
            newDate.day++;
        }
        else
        {
            newDate.day = 1;
            if (month < 12)
            {
                newDate.month++;
            }
            else
            {
                newDate.month = 1;
                newDate.year++;
            }
        }
    }
    return newDate;
}
#pragma region Function4input
// 從命令行讀取命令
int splitCommand(char *command, char *c[])
{
    char *token = strtok(command, " ");
    int i = 0;

    while (token != NULL && i < 7)
    {
        c[i++] = token;
        token = strtok(NULL, " ");
    }

    c[i] = NULL;

    // 檢查參數數量是否正確
    if (strcmp(c[0], "addPERIOD") == 0)
    {
        if (i != 3)
        {
            printf(errorColor "Wrong input: Maybe you mean addPERIOD [start date] [end date]?\n" resetColor);
            return 0;
        }
    }
    else if (strcmp(c[0], "addORDER") == 0)
    {
        if (i != 5)
        {
            printf(errorColor "Wrong input: Maybe you mean addORDER [Order Number] [Due Date] [Quantity] [Product Name]?\n" resetColor);
            return 0;
        }
    }
    else if (strcmp(c[0], "addBATCH") == 0)
    {
        if (i != 2)
        {
            printf(errorColor "Wrong input: Maybe you mean addBATCH [batch file]?\n" resetColor);
            return 0;
        }
    }
    else if (strcmp(c[0], "runPLS") == 0)
    {
        if (i != 6)
        {
            printf(errorColor "Wrong input: Maybe you mean runPLS [Algorithm] | printREPORT > [Report file name]?\n" resetColor);
            return 0;
        }
    }
    else if (strcmp(c[0], "exitPLS") == 0)
    {
        if (i != 1)
        {
            printf(errorColor "Wrong input: Maybe you mean exitPLS?\n" resetColor);
            return 0;
        }
    }
    else if (strcmp(c[0], "help") == 0)
    {
        if (i != 1)
        {
            printf(errorColor "Wrong input: Maybe you mean help?\n" resetColor);
            return 0;
        }
    }
    else
    {
        printf(errorColor "Command not found: %s\n" resetColor, c[0]);
        return 0;
    }

    return 1;
}

// 檢查日期格式是否正確,是否存在
int dateCheck(char date[])
{
    if (date[4] == '-' && date[7] == '-' && strlen(date) == 10 && isdigit(date[0]) && isdigit(date[1]) && isdigit(date[2]) && isdigit(date[3]) && isdigit(date[5]) && isdigit(date[6]) && isdigit(date[8]) && isdigit(date[9]))
    {
        int MonthDays[2][12] = {
            {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, // 平年
            {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}  // 閏年
        };
        int year, month, day;
        sscanf(date, "%d-%d-%d", &year, &month, &day);
        if (year < 0 || month < 1 || month > 12 || day < 1 || day > MonthDays[(year % 4 == 0 && year % 100 != 0) || year % 400 == 0][month - 1])
        {
            printf(errorColor "Wrong input: The date entered does not exist\n" resetColor);
            return 0;
        }
        return 1;
    }
    printf(errorColor "Wrong input: Date format should be YYYY-MM-DD\n" resetColor);
    return 0;
}

// 檢查訂單編號是否正確
int orderIDCheck(char *c[])
{
    for (int i = 0; i < 10000; i++)
    {
        if (strcmp(orders[i].orderID, c[1]) == 0)
        {
            printf(errorColor "Wrong input: Order ID already exists\n" resetColor);
            return 0;
        }
    }
    if (strlen(c[1]) == 5 && c[1][0] == 'P' && isdigit(c[1][1]) && isdigit(c[1][2]) && isdigit(c[1][3]) && isdigit(c[1][4]))
    {
        return 1;
    }
    printf(errorColor "Wrong input: Order ID should be PXXXX\n" resetColor);
    return 0;
}

// 檢查數量是否正確
int quantityCheck(char *c[])
{
    int checkNumber = 1;
    for (int i = 0; i < strlen(c[3]); i++)
    {
        if (!isdigit(c[3][i]))
        {
            checkNumber = 0;
            break;
        }
    }
    if (checkNumber)
    {
        return 1;
    }
    printf(errorColor "Wrong input: Quantity should be a number\n" resetColor);
    return 0;
}

// 檢查產品名稱是否正確
int productCheck(char *c[])
{
    if (strcmp(c[4], "Product_A") == 0 || strcmp(c[4], "Product_B") == 0 || strcmp(c[4], "Product_C") == 0 || strcmp(c[4], "Product_D") == 0 || strcmp(c[4], "Product_E") == 0 || strcmp(c[4], "Product_F") == 0 || strcmp(c[4], "Product_G") == 0 || strcmp(c[4], "Product_H") == 0 || strcmp(c[4], "Product_I") == 0)
    {
        return 1;
    }
    printf(errorColor "Wrong input: Product name should be Product_A, Product_B, Product_C, Product_D, Product_E, Product_F, Product_G, Product_H or Product_I\n" resetColor);
    return 0;
}

// addPERIOD命令錯誤預檢測處理
int addPeriodErrorCheck(char *c[])
{
    char date1[11], date2[11];
    strncpy(date1, c[1], 10);
    strncpy(date2, c[2], 10);
    date1[10] = '\0';
    date2[10] = '\0';
    if (dateCheck(date1) && dateCheck(date2))
    {
        int year1, month1, day1, year2, month2, day2;
        sscanf(date1, "%d-%d-%d", &year1, &month1, &day1);
        sscanf(date2, "%d-%d-%d", &year2, &month2, &day2);
        Date date1 = {year1, month1, day1};
        Date date2 = {year2, month2, day2};
        if (daysBetween(date1, date2) < 0)
        {
            printf(errorColor "Wrong input: The end date should be same or later than the start date\n" resetColor);
            return 0;
        }
        return 1;
    }
    return 0;
}

int OrderDateCheck(char *c[])
{
    int year, month, day;
    sscanf(c[2], "%d-%d-%d", &year, &month, &day);
    Date date = {year, month, day};
    if (daysBetween(wholePeriod[0], date) < 0 || daysBetween(date, wholePeriod[1]) < 0)
    {
        printf(errorColor "Wrong input: The delivery date should be in the period\n" resetColor);
        return 0;
    }
    return 1;
}

// addORDER命令錯誤預檢測處理
int addOrderErrorCheck(char *c[])
{
    if (orderIDCheck(c) && dateCheck(c[2]) && quantityCheck(c) && productCheck(c) && OrderDateCheck(c))
    {
        return 1;
    }
    return 0;
}

// addORDER命令核心
int addOrderKernal(char *c[])
{
    int year, month, day;
    int orderquantity;
    sscanf(c[2], "%d-%d-%d", &year, &month, &day);
    orders[orderCount].deliveryDate.year = year;
    orders[orderCount].deliveryDate.month = month;
    orders[orderCount].deliveryDate.day = day;
    sscanf(c[3], "%d", &orderquantity);
    orders[orderCount].quantity = orderquantity;
    if (strcmp(c[4], "Product_A") == 0)
    {
        orders[orderCount].product = Product_A;
    }
    else if (strcmp(c[4], "Product_B") == 0)
    {
        orders[orderCount].product = Product_B;
    }
    else if (strcmp(c[4], "Product_C") == 0)
    {
        orders[orderCount].product = Product_C;
    }
    else if (strcmp(c[4], "Product_D") == 0)
    {
        orders[orderCount].product = Product_D;
    }
    else if (strcmp(c[4], "Product_E") == 0)
    {
        orders[orderCount].product = Product_E;
    }
    else if (strcmp(c[4], "Product_F") == 0)
    {
        orders[orderCount].product = Product_F;
    }
    else if (strcmp(c[4], "Product_G") == 0)
    {
        orders[orderCount].product = Product_G;
    }
    else if (strcmp(c[4], "Product_H") == 0)
    {
        orders[orderCount].product = Product_H;
    }
    else if (strcmp(c[4], "Product_I") == 0)
    {
        orders[orderCount].product = Product_I;
    }
    strcpy(orders[orderCount].orderID, c[1]);

    return 1;
}

// 從batch文件讀取addORDER命令
int addBatchOrder(char *batchFileName)
{
    FILE *fp;
    char line[100];
    char *c[6];
    int lineCount = 0;
    int errorNum = 0;

    fp = fopen(batchFileName, "r");
    if (fp == NULL)
    {
        printf(errorColor "Wrong input: Batch file does not exist\n" resetColor);
        return 0;
    }

    while (fgets(line, sizeof(line), fp))
    {
        line[strcspn(line, "\n")] = '\0'; // 去除換行符
        if (splitCommand(line, c))
        {
            if (strcmp(c[0], "addORDER") == 0)
            {
                printf(resetColor "Line %d: ", lineCount + 1);
                if (addOrderErrorCheck(c))
                {
                    if (orderCount >= 9999)
                    {
                        printf(errorColor "Line %d: Wrong input: Too many orders. The maximum number of orders is 9999" resetColor "\n", lineCount + 1);
                        errorNum++;
                        continue;
                    }
                    addOrderKernal(c);
                    orderCount++;
                    printf(successColor "Order %s of %s with quantity %d and delivery date %s successful" resetColor "\n", orders[orderCount - 1].orderID, c[4], orders[orderCount - 1].quantity, c[2]);
                    lineCount++;
                }
                else
                {
                    errorNum++;
                    lineCount++;
                }
            }
            else
            {
                printf(errorColor "Line %d: Wrong input: Command only can be addORDER: %s" resetColor "\n", lineCount + 1, c[0]);
                errorNum++;
            }
        }
        else
        {
            printf(errorColor "Line %d: Wrong input: Command format error." resetColor "\n", lineCount + 1);
            errorNum++;
        }
    }

    fclose(fp);
    return errorNum;
}

// 檢查runPLS命令錯誤預檢測處理
int runPLSErrorCheck(char *c[])
{
    if (strcmp(c[2], "|") != 0 || strcmp(c[3], "printREPORT") != 0 || strcmp(c[4], ">") != 0)
    {
        printf(errorColor "Wrong input: Maybe you mean runPLS [Algorithm] | printREPORT > [Report file name]?\n" resetColor);
        return 0;
    }
    if (strcmp(c[1], "FCFS") == 0 || strcmp(c[1], "PR") == 0 || strcmp(c[1], "SJF") == 0 || strcmp(c[1], "Private") == 0)
    {
        // 檢測文件名是否是txt文件
        if (strlen(c[5]) < 5 || c[5][strlen(c[5]) - 1] != 't' || c[5][strlen(c[5]) - 2] != 'x' || c[5][strlen(c[5]) - 3] != 't' || c[5][strlen(c[5]) - 4] != '.')
        {
            printf(errorColor "Wrong input: Report file name should be a .txt file\n" resetColor);
            return 0;
        }
        return 1;
    }
    printf(errorColor "Wrong input: Algorithm should be FCFS, RR or SJF\n" resetColor);
    return 0;
}

#pragma endregion

#pragma region Function4scheduling

Schedule fcfs(Order orders[], int orderCount, Date period[])
{
    Schedule schedule;
    schedule.algorithm = FCFS;
    schedule.period[0] = period[0];
    schedule.period[1] = period[1];
    int dailyCapacities[3] = {300, 400, 500};

    int i, f, d;
    for (f = 0; f < 3; f++)
    {
        schedule.factorySchedule[f].factory = (FactoryName)f;
        schedule.factorySchedule[f].dayCount = 0;
        for (d = 0; d < 9999; d++)
        {
            strcpy(schedule.factorySchedule[f].dailyOrders[d].orderID, "");
            schedule.factorySchedule[f].dailyOrders[d].quantity = 0;
            schedule.factorySchedule[f].dailyOrders[d].product = NA;
            schedule.factorySchedule[f].dailyOrders[d].dueDate.year = 0;
            schedule.factorySchedule[f].dailyOrders[d].dueDate.month = 0;
            schedule.factorySchedule[f].dailyOrders[d].dueDate.day = 0;
        }
    }

    int day = 0;
    int currentOrder = 0;
    int remainingQuantity = orders[currentOrder].quantity;
    while (currentOrder < orderCount)
    {
        for (f = 0; f < 3; f++)
        {
            if (remainingQuantity <= 0)
            {
                currentOrder++;
                if (currentOrder >= orderCount)
                {
                    break;
                }

                remainingQuantity = orders[currentOrder].quantity;
            }

            int capacity = dailyCapacities[f];
            if (remainingQuantity <= capacity)
            {
                strcpy(schedule.factorySchedule[f].dailyOrders[day].orderID, orders[currentOrder].orderID);
                schedule.factorySchedule[f].dailyOrders[day].quantity = remainingQuantity;
                schedule.factorySchedule[f].dailyOrders[day].product = orders[currentOrder].product;
                schedule.factorySchedule[f].dailyOrders[day].dueDate = orders[currentOrder].deliveryDate;
                schedule.factorySchedule[f].dayCount++;
                remainingQuantity = 0;
            }
            else
            {
                strcpy(schedule.factorySchedule[f].dailyOrders[day].orderID, orders[currentOrder].orderID);
                schedule.factorySchedule[f].dailyOrders[day].quantity = capacity;
                schedule.factorySchedule[f].dailyOrders[day].product = orders[currentOrder].product;
                schedule.factorySchedule[f].dailyOrders[day].dueDate = orders[currentOrder].deliveryDate;
                schedule.factorySchedule[f].dayCount++;
                remainingQuantity -= capacity;
            }
        }
        day++;
    }
    return schedule;
}

Schedule pr(Order orders[], int orderCount, Date period[])
{
    int i, j;
    const int PRIORITIES[10] = {0, 0, 0, 1, 1, 1, 2, 2, 2};
    int orderPriorities[orderCount];

    for (i = 0; i < orderCount; i++)
    {
        orderPriorities[i] = PRIORITIES[orders[i].product];
    }

    for (i = 0; i < orderCount - 1; i++)
    {
        for (j = 0; j < orderCount - i - 1; j++)
        {
            if (orderPriorities[j] > orderPriorities[j + 1])
            {
                Order temp = orders[j];
                orders[j] = orders[j + 1];
                orders[j + 1] = temp;
            }
        }
    }

    Schedule schedule = fcfs(orders, orderCount, period);
    schedule.algorithm = PR;

    return schedule;
}

Schedule sjf(Order orders[], int orderCount, Date period[])
{
    int i, f, d;
    Schedule schedule;
    schedule.algorithm = SJF;
    schedule.period[0] = period[0];
    schedule.period[1] = period[1];
    int dailyCapacities[3] = {300, 400, 500};

    for (f = 0; f < 3; f++)
    {
        schedule.factorySchedule[f].factory = (FactoryName)f;
        schedule.factorySchedule[f].dayCount = 0;
        for (d = 0; d < 9999; d++)
        {
            strcpy(schedule.factorySchedule[f].dailyOrders[d].orderID, "");
            schedule.factorySchedule[f].dailyOrders[d].quantity = 0;
            schedule.factorySchedule[f].dailyOrders[d].product = NA;
            schedule.factorySchedule[f].dailyOrders[d].dueDate.year = 0;
            schedule.factorySchedule[f].dailyOrders[d].dueDate.month = 0;
            schedule.factorySchedule[f].dailyOrders[d].dueDate.day = 0;
        }
    }

    int day = 0, completedOrders = 0, remainingQuantities[orderCount];
    for (i = 0; i < orderCount; i++)
    {
        remainingQuantities[i] = orders[i].quantity;
    }

    while (completedOrders < orderCount)
    {
        int shortestJobIndex = -1;
        int shortestJobQuantity = 2147483647;

        for (i = 0; i < orderCount; i++)
        {
            if (remainingQuantities[i] > 0 && remainingQuantities[i] < shortestJobQuantity)
            {
                shortestJobIndex = i;
                shortestJobQuantity = remainingQuantities[i];
            }
        }

        if (shortestJobIndex == -1)
        {
            break;
        }

        for (f = 0; f < 3; f++)
        {
            int capacity = dailyCapacities[f];
            if (shortestJobQuantity <= capacity)
            {
                strcpy(schedule.factorySchedule[f].dailyOrders[day].orderID, orders[shortestJobIndex].orderID);
                schedule.factorySchedule[f].dailyOrders[day].quantity = shortestJobQuantity;
                schedule.factorySchedule[f].dailyOrders[day].product = orders[shortestJobIndex].product;
                schedule.factorySchedule[f].dailyOrders[day].dueDate = orders[shortestJobIndex].deliveryDate;
                schedule.factorySchedule[f].dayCount++;
                remainingQuantities[shortestJobIndex] = 0;
                completedOrders++;
            }
            else
            {
                strcpy(schedule.factorySchedule[f].dailyOrders[day].orderID, orders[shortestJobIndex].orderID);
                schedule.factorySchedule[f].dailyOrders[day].quantity = capacity;
                schedule.factorySchedule[f].dailyOrders[day].product = orders[shortestJobIndex].product;
                schedule.factorySchedule[f].dailyOrders[day].dueDate = orders[shortestJobIndex].deliveryDate;
                schedule.factorySchedule[f].dayCount++;
                remainingQuantities[shortestJobIndex] -= capacity;
            }
        }
        day++;
    }

    return schedule;
}

Schedule private(Order orders[], int orderCount, Date period[])
{
    int i, f, d;
    Schedule schedule;
    schedule.algorithm = Private;
    schedule.period[0] = period[0];
    schedule.period[1] = period[1];
    int dailyCapacities[3] = {300, 400, 500};

    for (f = 0; f < 3; f++)
    {
        schedule.factorySchedule[f].factory = (FactoryName)f;
        schedule.factorySchedule[f].dayCount = 0;
        for (d = 0; d < 9999; d++)
        {
            strcpy(schedule.factorySchedule[f].dailyOrders[d].orderID, "");
            schedule.factorySchedule[f].dailyOrders[d].quantity = 0;
            schedule.factorySchedule[f].dailyOrders[d].product = NA;
            schedule.factorySchedule[f].dailyOrders[d].dueDate.year = 0;
            schedule.factorySchedule[f].dailyOrders[d].dueDate.month = 0;
            schedule.factorySchedule[f].dailyOrders[d].dueDate.day = 0;
        }
    }

    int day = 0;
    int completedOrders = 0;
    int remainingQuantities[orderCount];
    int laxities[orderCount];

    for (i = 0; i < orderCount; i++)
    {
        remainingQuantities[i] = orders[i].quantity;
        int daysUntilDue = daysBetween(period[0], orders[i].deliveryDate);
        laxities[i] = daysUntilDue - orders[i].quantity;
    }

    while (completedOrders < orderCount)
    {
        int lowestLaxityIndex = -1;
        int lowestLaxity = 2147483647;

        for (i = 0; i < orderCount; i++)
        {
            if (remainingQuantities[i] > 0 && laxities[i] < lowestLaxity)
            {
                lowestLaxityIndex = i;
                lowestLaxity = laxities[i];
            }
        }

        if (lowestLaxityIndex == -1)
        {
            break;
        }

        for (f = 0; f < 3; f++)
        {
            int capacity = dailyCapacities[f];
            if (remainingQuantities[lowestLaxityIndex] > 0)
            {
                int quantityToAssign = (remainingQuantities[lowestLaxityIndex] <= capacity) ? remainingQuantities[lowestLaxityIndex] : capacity;
                strcpy(schedule.factorySchedule[f].dailyOrders[day].orderID, orders[lowestLaxityIndex].orderID);
                schedule.factorySchedule[f].dailyOrders[day].quantity = quantityToAssign;
                schedule.factorySchedule[f].dailyOrders[day].product = orders[lowestLaxityIndex].product;
                schedule.factorySchedule[f].dailyOrders[day].dueDate = orders[lowestLaxityIndex].deliveryDate;
                schedule.factorySchedule[f].dayCount++;
                remainingQuantities[lowestLaxityIndex] -= quantityToAssign;
                if (remainingQuantities[lowestLaxityIndex] == 0)
                {
                    completedOrders++;
                }
                else
                {
                    laxities[lowestLaxityIndex] = daysBetween(addDays(period[0], day + 1), orders[lowestLaxityIndex].deliveryDate) - remainingQuantities[lowestLaxityIndex];
                }
            }
        }
        day++;
    }

    return schedule;
}

#pragma endregion
#pragma region Function4report

#pragma endregion
#pragma region Function4outputToFilePart

// 輸出報告頭部
void printReportHead(FILE *fp, Schedule schedule)
{
    fprintf(fp, "***PLS Schedule Analysis Report***\n\n");
    if (schedule.algorithm == FCFS)
    {
        fprintf(fp, "Algorithm used: FCFS\n\n");
    }
    else if (schedule.algorithm == PR)
    {
        fprintf(fp, "Algorithm used: PR\n\n");
    }
    else if (schedule.algorithm == SJF)
    {
        fprintf(fp, "Algorithm used: SJF\n\n");
    }
    else if (schedule.algorithm == Private)
    {
        fprintf(fp, "Algorithm used: Private\n\n");
    }
}

void printAccept(FILE *fp, int acceptCount, Accept acceptData[])
{
    fprintf(fp, "There are %d Orders ACCEPTED. Details are as follows:\n\n", acceptCount);
    fprintf(fp, "ORDER NUMBER START          END            DAYS       QUANTITY    PLANT\n");
    fprintf(fp, "===========================================================================\n");
    for (int i = 0; i < acceptCount; i++)
    {
        if (acceptData[i].ifX)
        {
            fprintf(fp, "%s        %d-%02d-%02d     %d-%02d-%02d     %-3d        %8d  Plant_X\n", acceptData[i].orderID, acceptData[i].xStartDay.year, acceptData[i].xStartDay.month, acceptData[i].xStartDay.day, acceptData[i].xEndDay.year, acceptData[i].xEndDay.month, acceptData[i].xEndDay.day, daysBetween(acceptData[i].xStartDay, acceptData[i].xEndDay), acceptData[i].xQuantity);
        }
        if (acceptData[i].ifY)
        {
            fprintf(fp, "%s        %d-%02d-%02d     %d-%02d-%02d     %-3d        %8d  Plant_Y\n", acceptData[i].orderID, acceptData[i].yStartDay.year, acceptData[i].yStartDay.month, acceptData[i].yStartDay.day, acceptData[i].yEndDay.year, acceptData[i].yEndDay.month, acceptData[i].yEndDay.day, daysBetween(acceptData[i].yStartDay, acceptData[i].yEndDay), acceptData[i].yQuantity);
        }
        if (acceptData[i].ifZ)
        {
            fprintf(fp, "%s        %d-%02d-%02d     %d-%02d-%02d     %-3d        %8d  Plant_Z\n", acceptData[i].orderID, acceptData[i].zStartDay.year, acceptData[i].zStartDay.month, acceptData[i].zStartDay.day, acceptData[i].zEndDay.year, acceptData[i].zEndDay.month, acceptData[i].zEndDay.day, daysBetween(acceptData[i].zStartDay, acceptData[i].zEndDay), acceptData[i].zQuantity);
        }
    }
    fprintf(fp, "\n");
    fprintf(fp, "            - End -\n\n");
    fprintf(fp, "===========================================================================\n\n");
}

void printReject(FILE *fp, int rejectCount, Order *rejectOrders)
{
    fprintf(fp, "There are %d Orders REJECTED. Details are as follows:\n\n", rejectCount);
    fprintf(fp, "ORDER NUMBER   PRODUCT NAME     Due Date        QUANTITY\n");
    fprintf(fp, "===========================================================================\n");
    for (int i = 0; i < rejectCount; i++)
    {
        fprintf(fp, "%s          %s        %d-%02d-%02d      %8d\n", rejectOrders[i].orderID, rejectOrders[i].product == Product_A ? "Product_A" : rejectOrders[i].product == Product_B ? "Product_B"
                                                                                                                                               : rejectOrders[i].product == Product_C   ? "Product_C"
                                                                                                                                               : rejectOrders[i].product == Product_D   ? "Product_D"
                                                                                                                                               : rejectOrders[i].product == Product_E   ? "Product_E"
                                                                                                                                               : rejectOrders[i].product == Product_F   ? "Product_F"
                                                                                                                                               : rejectOrders[i].product == Product_G   ? "Product_G"
                                                                                                                                               : rejectOrders[i].product == Product_H   ? "Product_H"
                                                                                                                                                                                        : "Product_I",
                rejectOrders[i].deliveryDate.year, rejectOrders[i].deliveryDate.month, rejectOrders[i].deliveryDate.day, rejectOrders[i].quantity);
    }
    fprintf(fp, "\n");
    fprintf(fp, "            - End -\n\n");
    fprintf(fp, "===========================================================================\n\n");
}
// 輸出表現
void printPerformance(FILE *fp, Performance performance)
{
    fprintf(fp, "***PERFORMANCE\n\n");
    fprintf(fp, "Plant_X:\n");
    fprintf(fp, "          Factory X:                               %d days\n", performance.xDay);
    fprintf(fp, "          Number of products produced:             %8d(in total)\n", performance.xProduct);
    fprintf(fp, "          Utilization of the plant:                %.2f%%\n\n", performance.xUtilization * 100);
    fprintf(fp, "Plant_Y:\n");
    fprintf(fp, "          Factory Y:                               %d days\n", performance.yDay);
    fprintf(fp, "          Number of products produced:             %8d(in total)\n", performance.yProduct);
    fprintf(fp, "          Utilization of the plant:                %.2f%%\n\n", performance.yUtilization * 100);
    fprintf(fp, "Plant_Z:\n");
    fprintf(fp, "          Factory Z:                               %d days\n", performance.zDay);
    fprintf(fp, "          Number of products produced:             %8d(in total)\n", performance.zProduct);
    fprintf(fp, "          Utilization of the plant:                %.2f%%\n\n", performance.zUtilization * 100);
    fprintf(fp, "Overall of utilization:                            %.2f%%\n\n", performance.allUtilization * 100);
}

#pragma endregion

int main()
{

#pragma region Pipe
    int input2scheduling[2];
    int scheduling2report[2];
    int exitNormally[2][2];
    int schedulingFinished[2];
    int reportFinished[2];
    int scheduling2reportForOrder[2];

    // 創建pipe
    if (pipe(input2scheduling) < 0 || pipe(scheduling2report) < 0 || pipe(exitNormally[0]) < 0 || pipe(exitNormally[1]) < 0 || pipe(schedulingFinished) < 0 || pipe(reportFinished) < 0 || pipe(scheduling2reportForOrder) < 0)
    {
        printf(errorColor "Fatal error: Pipe failed\n" resetColor);
        exit(-1);
    }

#pragma endregion

    // 子進程：算法, 報告輸出
    pid_t pid = fork();

    if (pid < 0)
    {
        printf(errorColor "Fatal error: Fork failed\n" resetColor);
        exit(-1);
    }
    else if (pid == 0)
    {

        int exitFlag = 1;
        close(input2scheduling[1]);
        close(scheduling2report[0]);
        close(exitNormally[0][1]);
        close(exitNormally[1][0]);
        close(exitNormally[1][1]);
        close(schedulingFinished[0]);
        close(reportFinished[0]);
        close(reportFinished[1]);
        close(scheduling2reportForOrder[0]);

        while (exitFlag)
        {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(exitNormally[0][0], &rfds);
            FD_SET(input2scheduling[0], &rfds);

            int maxfd = (exitNormally[0][0] > input2scheduling[0]) ? exitNormally[0][0] : input2scheduling[0];

            int retval = select(maxfd + 1, &rfds, NULL, NULL, NULL);
            if (retval == -1)
            {
                perror("select");
                break;
            }

            if (FD_ISSET(exitNormally[0][0], &rfds))
            {
                char buf[10];
                int n = read(exitNormally[0][0], buf, sizeof(buf));
                if (n > 0)
                {
                    exitFlag = 0;
                    break;
                }
            }

            if (FD_ISSET(input2scheduling[0], &rfds))
            {
                input2schedulingData data;
                int total_bytes = 0;
                int bytes_read = 0;

                while (total_bytes < sizeof(data))
                {
                    bytes_read = read(input2scheduling[0], (char *)&data + total_bytes, sizeof(data) - total_bytes);
                    if (bytes_read <= 0)
                    {
                        break;
                    }
                    total_bytes += bytes_read;
                }

                if (total_bytes == sizeof(data))
                {
#pragma region Schedule
                    Schedule DataSchedule;
                    // 處理接收到的完整數據
                    printf(resetColor "period: %d-%d-%d to %d-%d-%d\n", data.period[0].year, data.period[0].month, data.period[0].day, data.period[1].year, data.period[1].month, data.period[1].day);
                    if (data.algorithm == FCFS)
                    {
                        DataSchedule = fcfs(data.orders, data.orderCount, data.period);
                    }
                    else if (data.algorithm == PR)
                    {
                        DataSchedule = pr(data.orders, data.orderCount, data.period);
                    }
                    else if (data.algorithm == SJF)
                    {
                        DataSchedule = sjf(data.orders, data.orderCount, data.period);
                    }
                    else if (data.algorithm == Private)
                    {
                        DataSchedule = private(data.orders, data.orderCount, data.period);
                    }
                    printf(resetColor "orderCount: %d\n", data.orderCount);
                    printf(resetColor "Report will be saved to %s\n", data.fileName);
#pragma endregion

#pragma region Report
                    Order rejectOrders[9999];
                    char rejectOrderNumber[9999][6];
                    int rejectCount = 0;

                    for (int i = 0; i < DataSchedule.factorySchedule[0].dayCount; i++)
                    {
                        if (DataSchedule.factorySchedule[0].dailyOrders[i].product == NA)
                        {
                            continue;
                        }
                        if (daysBetween(newDate(data.period[0], i), DataSchedule.factorySchedule[0].dailyOrders[i].dueDate) < 0)
                        {
                            int findFlag = 0;
                            for (int j = 0; j < rejectCount; j++)
                            {
                                if (strcmp(DataSchedule.factorySchedule[0].dailyOrders[i].orderID, rejectOrderNumber[j]) == 0)
                                {
                                    findFlag = 1;
                                    break;
                                }
                            }
                            if (findFlag == 0)
                            {
                                strcpy(rejectOrderNumber[rejectCount], DataSchedule.factorySchedule[1].dailyOrders[i].orderID);
                                rejectCount++;
                            }
                        }
                    }

                    for (int i = 0; i < rejectCount; i++)
                    {
                        for (int j = 0; j < data.orderCount; j++)
                        {
                            if (strcmp(data.orders[j].orderID, rejectOrderNumber[i]) == 0)
                            {
                                rejectOrders[i] = data.orders[j];
                                break;
                            }
                        }
                    }

                    Order acceptOrders[9999];
                    int acceptCount = 0;

                    for (int i = 0; i < data.orderCount; i++)
                    {
                        int findFlag = 0;
                        for (int j = 0; j < rejectCount; j++)
                        {
                            if (strcmp(data.orders[i].orderID, rejectOrders[j].orderID) == 0)
                            {
                                findFlag = 1;
                                break;
                            }
                        }
                        if (findFlag == 0)
                        {
                            acceptOrders[acceptCount] = data.orders[i];
                            acceptCount++;
                        }
                    }

                    // output
                    for (int i = 0; i < 3; i++)
                    {
                        if (i == 0)
                        {
                            printf("\nPlant_X (300 per day)\n");
                        }
                        else if (i == 1)
                        {
                            printf("\nPlant_Y (400 per day)\n");
                        }
                        else if (i == 2)
                        {
                            printf("\nPlant_Z (500 per day)\n");
                        }
                        printf("%d-%02d-%02d to %d-%02d-%02d\n", data.period[0].year, data.period[0].month, data.period[0].day, data.period[1].year, data.period[1].month, data.period[1].day);
                        printf("   Date            Product Name         Order Number         Quantity(Produced)         Due Date\n");
                        printf("=====================================================================================================\n");
                        for (int j = 0; j < daysBetween(data.period[0], data.period[1]); j++)
                        {
                            if (j < DataSchedule.factorySchedule[i].dayCount)
                            {
                                Date ThisNewDate = newDate(data.period[0], j);
                                if (DataSchedule.factorySchedule[i].dailyOrders[j].product == NA)
                                {
                                    printf("%d-%02d-%02d         NA\n", ThisNewDate.year, ThisNewDate.month, ThisNewDate.day);
                                }
                                // 查找是否被拒絕
                                int rejectFlag = 0;
                                for (int k = 0; k < rejectCount; k++)
                                {
                                    if (strcmp(DataSchedule.factorySchedule[i].dailyOrders[j].orderID, rejectOrderNumber[k]) == 0)
                                    {
                                        rejectFlag = 1;
                                        break;
                                    }
                                }
                                if (rejectFlag)
                                {
                                    printf("%d-%02d-%02d         NA\n", ThisNewDate.year, ThisNewDate.month, ThisNewDate.day);
                                }
                                else
                                {
                                    printf("%d-%02d-%02d         %s            %s                %-18d         %d-%02d-%02d\n", ThisNewDate.year, ThisNewDate.month, ThisNewDate.day, DataSchedule.factorySchedule[i].dailyOrders[j].product == Product_A ? "Product_A" : DataSchedule.factorySchedule[i].dailyOrders[j].product == Product_B ? "Product_B"
                                                                                                                                                                                                                                                                      : DataSchedule.factorySchedule[i].dailyOrders[j].product == Product_C   ? "Product_C"
                                                                                                                                                                                                                                                                      : DataSchedule.factorySchedule[i].dailyOrders[j].product == Product_D   ? "Product_D"
                                                                                                                                                                                                                                                                      : DataSchedule.factorySchedule[i].dailyOrders[j].product == Product_E   ? "Product_E"
                                                                                                                                                                                                                                                                      : DataSchedule.factorySchedule[i].dailyOrders[j].product == Product_F   ? "Product_F"
                                                                                                                                                                                                                                                                      : DataSchedule.factorySchedule[i].dailyOrders[j].product == Product_G   ? "Product_G"
                                                                                                                                                                                                                                                                      : DataSchedule.factorySchedule[i].dailyOrders[j].product == Product_H   ? "Product_H"
                                                                                                                                                                                                                                                                                                                                              : "Product_I",
                                           DataSchedule.factorySchedule[i].dailyOrders[j].orderID, DataSchedule.factorySchedule[i].dailyOrders[j].quantity, DataSchedule.factorySchedule[i].dailyOrders[j].dueDate.year, DataSchedule.factorySchedule[i].dailyOrders[j].dueDate.month, DataSchedule.factorySchedule[i].dailyOrders[j].dueDate.day);
                                }
                            }
                            else
                            {
                                Date ThisNewDate = newDate(data.period[0], j);
                                printf("%d-%02d-%02d         NA\n", ThisNewDate.year, ThisNewDate.month, ThisNewDate.day);
                            }
                        }
                    }

                    Accept accept[9999];
                    // 找出接受訂單的各個工廠的開始日和結束日
                    for (int i = 0; i < acceptCount; i++)
                    {
                        Date thisXStartDay;
                        Date thisXEndDay;
                        Date thisYStartDay;
                        Date thisYEndDay;
                        Date thisZStartDay;
                        Date thisZEndDay;
                        int xFlag = 0;
                        int yFlag = 0;
                        int zFlag = 0;
                        int xQuantity = 0;
                        int yQuantity = 0;
                        int zQuantity = 0;
                        // x工廠
                        for (int j = 0; j < DataSchedule.factorySchedule[0].dayCount; j++)
                        {
                            if (strcmp(DataSchedule.factorySchedule[0].dailyOrders[j].orderID, acceptOrders[i].orderID) == 0)
                            {
                                xQuantity += DataSchedule.factorySchedule[0].dailyOrders[j].quantity;
                                if (xFlag == 0)
                                {
                                    thisXStartDay = newDate(data.period[0], j);
                                    xFlag = 1;
                                    if (j == DataSchedule.factorySchedule[0].dayCount - 1)
                                    {
                                        thisXEndDay = newDate(data.period[0], j);
                                        break;
                                    }
                                    if (strcmp(DataSchedule.factorySchedule[0].dailyOrders[j + 1].orderID, acceptOrders[i].orderID) != 0)
                                    {
                                        thisXEndDay = newDate(data.period[0], j);
                                        break;
                                    }
                                }
                                else
                                {

                                    if (j == DataSchedule.factorySchedule[0].dayCount - 1)
                                    {
                                        thisXEndDay = newDate(data.period[0], j);
                                        break;
                                    }
                                    if (strcmp(DataSchedule.factorySchedule[0].dailyOrders[j + 1].orderID, acceptOrders[i].orderID) != 0)
                                    {
                                        thisXEndDay = newDate(data.period[0], j);
                                        break;
                                    }
                                }
                            }
                        }
                        // y工廠
                        for (int j = 0; j < DataSchedule.factorySchedule[1].dayCount; j++)
                        {
                            if (strcmp(DataSchedule.factorySchedule[1].dailyOrders[j].orderID, acceptOrders[i].orderID) == 0)
                            {
                                yQuantity += DataSchedule.factorySchedule[1].dailyOrders[j].quantity;
                                if (yFlag == 0)
                                {
                                    thisYStartDay = newDate(data.period[0], j);
                                    yFlag = 1;
                                    if (j == DataSchedule.factorySchedule[1].dayCount - 1)
                                    {
                                        thisYEndDay = newDate(data.period[0], j);
                                        break;
                                    }
                                    if (strcmp(DataSchedule.factorySchedule[1].dailyOrders[j + 1].orderID, acceptOrders[i].orderID) != 0)
                                    {
                                        thisYEndDay = newDate(data.period[0], j);
                                        break;
                                    }
                                }
                                else
                                {

                                    if (j == DataSchedule.factorySchedule[1].dayCount - 1)
                                    {
                                        thisYEndDay = newDate(data.period[0], j);
                                        break;
                                    }
                                    if (strcmp(DataSchedule.factorySchedule[1].dailyOrders[j + 1].orderID, acceptOrders[i].orderID) != 0)
                                    {
                                        thisYEndDay = newDate(data.period[0], j);
                                        break;
                                    }
                                }
                            }
                        }
                        // z工廠
                        for (int j = 0; j < DataSchedule.factorySchedule[2].dayCount; j++)
                        {
                            if (strcmp(DataSchedule.factorySchedule[2].dailyOrders[j].orderID, acceptOrders[i].orderID) == 0)
                            {
                                zQuantity += DataSchedule.factorySchedule[2].dailyOrders[j].quantity;
                                if (zFlag == 0)
                                {
                                    thisZStartDay = newDate(data.period[0], j);
                                    zFlag = 1;
                                    if (j == DataSchedule.factorySchedule[2].dayCount - 1)
                                    {
                                        thisZEndDay = newDate(data.period[0], j);
                                        break;
                                    }
                                    if (strcmp(DataSchedule.factorySchedule[2].dailyOrders[j + 1].orderID, acceptOrders[i].orderID) != 0)
                                    {
                                        thisZEndDay = newDate(data.period[0], j);
                                        break;
                                    }
                                }
                                else
                                {

                                    if (j == DataSchedule.factorySchedule[2].dayCount - 1)
                                    {
                                        thisZEndDay = newDate(data.period[0], j);
                                        break;
                                    }
                                    if (strcmp(DataSchedule.factorySchedule[2].dailyOrders[j + 1].orderID, acceptOrders[i].orderID) != 0)
                                    {
                                        thisZEndDay = newDate(data.period[0], j);
                                        break;
                                    }
                                }
                            }
                        }
                        strcpy(accept[i].orderID, acceptOrders[i].orderID);
                        if (xFlag == 1)
                        {
                            accept[i].ifX = 1;
                            accept[i].xStartDay = thisXStartDay;
                            accept[i].xEndDay = thisXEndDay;
                            accept[i].xQuantity = xQuantity;
                        }
                        if (yFlag == 1)
                        {
                            accept[i].ifY = 1;
                            accept[i].yStartDay = thisYStartDay;
                            accept[i].yEndDay = thisYEndDay;
                            accept[i].yQuantity = yQuantity;
                        }
                        if (zFlag == 1)
                        {
                            accept[i].ifZ = 1;
                            accept[i].zStartDay = thisZStartDay;
                            accept[i].zEndDay = thisZEndDay;
                            accept[i].zQuantity = zQuantity;
                        }
                    }

                    // 利用率
                    Performance performance = {0, 0, 0, 0, 0, 0, 0, 0, 0};
                    for (int i = 0; i < daysBetween(data.period[0], data.period[1]); i++)
                    {
                        int rejectFlag = 0;
                        for (int j = 0; j < rejectCount; j++)
                        {
                            if (strcmp(DataSchedule.factorySchedule[0].dailyOrders[i].orderID, rejectOrderNumber[j]) == 0)
                            {
                                rejectFlag = 1;
                                break;
                            }
                        }
                        if (rejectFlag)
                        {
                            continue;
                        }
                        if (DataSchedule.factorySchedule[0].dailyOrders[i].product == NA)
                        {
                            continue;
                        }
                        performance.xDay++;
                        performance.xProduct += DataSchedule.factorySchedule[0].dailyOrders[i].quantity;
                    }
                    performance.xUtilization = (double)performance.xProduct / (300 * daysBetween(data.period[0], data.period[1]));
                    for (int i = 0; i < daysBetween(data.period[0], data.period[1]); i++)
                    {
                        int rejectFlag = 0;
                        for (int j = 0; j < rejectCount; j++)
                        {
                            if (strcmp(DataSchedule.factorySchedule[1].dailyOrders[i].orderID, rejectOrderNumber[j]) == 0)
                            {
                                rejectFlag = 1;
                                break;
                            }
                        }
                        if (rejectFlag || DataSchedule.factorySchedule[1].dailyOrders[i].product == NA)
                        {
                            continue;
                        }

                        performance.yDay++;
                        performance.yProduct += DataSchedule.factorySchedule[1].dailyOrders[i].quantity;
                    }
                    performance.yUtilization = (double)performance.yProduct / (400 * daysBetween(data.period[0], data.period[1]));
                    for (int i = 0; i < daysBetween(data.period[0], data.period[1]); i++)
                    {
                        int rejectFlag = 0;
                        for (int j = 0; j < rejectCount; j++)
                        {
                            if (strcmp(DataSchedule.factorySchedule[2].dailyOrders[i].orderID, rejectOrderNumber[j]) == 0)
                            {
                                rejectFlag = 1;
                                break;
                            }
                        }
                        if (rejectFlag)
                        {
                            continue;
                        }
                        if (DataSchedule.factorySchedule[2].dailyOrders[i].product == NA)
                        {
                            continue;
                        }
                        performance.zDay++;
                        performance.zProduct += DataSchedule.factorySchedule[2].dailyOrders[i].quantity;
                    }
                    performance.zUtilization = (double)performance.zProduct / (500 * daysBetween(data.period[0], data.period[1]));
                    performance.allUtilization = (double)(performance.xProduct + performance.yProduct + performance.zProduct) / (1200 * daysBetween(data.period[0], data.period[1]));
                    performance.xDay = 0;
                    performance.yDay = 0;
                    performance.zDay = 0;
                    for (int i = 0; i < acceptCount; i++)
                    {
                        if (accept[i].ifX)
                        {
                            performance.xDay += daysBetween(accept[i].xStartDay, accept[i].xEndDay);
                        }
                        if (accept[i].ifY)
                        {
                            performance.yDay += daysBetween(accept[i].yStartDay, accept[i].yEndDay);
                        }
                        if (accept[i].ifZ)
                        {
                            performance.zDay += daysBetween(accept[i].zStartDay, accept[i].zEndDay);
                        }
                    }

                    FILE *fp = fopen(data.fileName, "w");
                    if (fp == NULL)
                    {
                        printf(errorColor "Error: Report file failed\n" resetColor);
                        break;
                    }
                    printReportHead(fp, DataSchedule);
                    printAccept(fp, acceptCount, accept);
                    printReject(fp, rejectCount, rejectOrders);
                    printPerformance(fp, performance);
                    fclose(fp);

                    write(schedulingFinished[1], "finished", 9);
                }
                else if (total_bytes == 0)
                {
                    break;
                }
                else
                {
                    printf(errorColor "Error: Incomplete data received\n" resetColor);
                }
            }
        }
        close(input2scheduling[0]);
        close(scheduling2report[1]);
        close(exitNormally[0][0]);
        close(schedulingFinished[1]);
        close(scheduling2reportForOrder[1]);
        exit(0);
    }

    // 關閉pipe
    close(input2scheduling[0]);
    close(scheduling2report[0]);
    close(scheduling2report[1]);
    close(exitNormally[0][0]);
    close(exitNormally[1][0]);
    close(schedulingFinished[1]);
    close(reportFinished[1]);

// input模塊
#pragma region input
    char command[100];
    char *c[7];

    printf("\n~~WELCOME TO PLS~~\n\n");
    while (true)
    {
        printf(resetColor "Please enter:\n" userColor "> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0'; // 去掉換字符
        // 去掉命令前面的 "> " 符號
        if (strncmp(command, "> ", 2) == 0)
        {
            memmove(command, command + 2, strlen(command) - 1);
        }

        if (splitCommand(command, c))
        {
            // 處理命令
            if (strcmp(c[0], "addPERIOD") == 0)
            {
                if (addPeriodErrorCheck(c))
                {
                    if (wholePeriod[0].year != 0)
                    {
                        printf(errorColor "Wrong input: Period already exists\n" resetColor);
                        continue;
                    }
                    int year1, month1, day1, year2, month2, day2;
                    sscanf(c[1], "%d-%d-%d", &year1, &month1, &day1);
                    sscanf(c[2], "%d-%d-%d", &year2, &month2, &day2);
                    wholePeriod[0].year = year1;
                    wholePeriod[0].month = month1;
                    wholePeriod[0].day = day1;
                    wholePeriod[1].year = year2;
                    wholePeriod[1].month = month2;
                    wholePeriod[1].day = day2;

                    printf(successColor "Period From %s to %s successful\n", c[1], c[2]);
                }
            }
            else if (strcmp(c[0], "addORDER") == 0)
            {
                if (wholePeriod[0].year == 0)
                {
                    printf(errorColor "Wrong input: Please add period first\n" resetColor);
                    continue;
                }
                if (addOrderErrorCheck(c))
                {
                    if (orderCount >= 9999)
                    {
                        printf(errorColor "Wrong input: Too many orders. The maximum number of orders is 9999\n" resetColor);
                        continue;
                    }
                    addOrderKernal(c);

                    orderCount++;

                    printf(successColor "Order %s of %s with quantity %d and delivery date %s successful\n", orders[orderCount - 1].orderID, c[4], orders[orderCount - 1].quantity, c[2]);
                }
            }
            else if (strcmp(c[0], "addBATCH") == 0)
            {
                if (wholePeriod[0].year == 0)
                {
                    printf(errorColor "Wrong input: Please add period first\n" resetColor);
                    continue;
                }
                printf(resetColor "Batch file: %s\n", c[1]);
                int errorNum;
                errorNum = addBatchOrder(c[1]);
                if (errorNum == 0)
                {
                    printf(successColor "\nBatch file %s successful\n\n", c[1]);
                }
                else
                {
                    printf(errorColor "\nBatch file %s failed, %d errors\n\n", c[1], errorNum);
                }
            }
            else if (strcmp(c[0], "runPLS") == 0)
            {
                if (wholePeriod[0].year == 0)
                {
                    printf(errorColor "Wrong input: Please add period first\n" resetColor);
                    continue;
                }
                if (orderCount == 0)
                {
                    printf(errorColor "Wrong input: Please add order first\n" resetColor);
                    continue;
                }
                if (runPLSErrorCheck(c))
                {
                    input2schedulingData data;
                    if (strcmp(c[1], "SJF") == 0)
                    {
                        data.algorithm = SJF;
                    }
                    else if (strcmp(c[1], "PR") == 0)
                    {
                        data.algorithm = PR;
                    }
                    else if (strcmp(c[1], "FCFS") == 0)
                    {
                        data.algorithm = FCFS;
                    }
                    else if (strcmp(c[1], "Private") == 0)
                    {
                        data.algorithm = Private;
                    }
                    data.orderCount = orderCount;
                    printf(resetColor "orderCount: %d\n", orderCount);
                    for (int i = 0; i < orderCount; i++)
                    {
                        data.orders[i] = orders[i];
                    }
                    strcpy(data.fileName, c[5]);
                    data.period[0] = wholePeriod[0];
                    data.period[1] = wholePeriod[1];
                    write(input2scheduling[1], &data, sizeof(data));

                    printf(successColor "PLS with %s is running.Please waiting.\n", c[1]);

                    while (true)
                    {
                        char finStr1[9];
                        int fin1 = read(schedulingFinished[0], finStr1, sizeof(finStr1));
                        if (fin1 > 0)
                        {
                            printf(successColor "Scheduling finished, please read the report\n");
                            break;
                        }
                    }
                }
            }
            else if (strcmp(c[0], "help") == 0)
            {
                printf(resetColor B "Commands:\n\n" resetB);
                printf("addPERIOD [start date] [end date]\n");
                printf("addORDER [Order Number] [Due Date] [Quantity] [Product Name]\n");
                printf("addBATCH [batch file]\n");
                printf("runPLS [Algorithm] | printREPORT > [Report file name]\n");
                printf("exitPLS\n");
                printf("help\n\n");
                printf(B "data format\n\n" resetB);
                printf("date format: YYYY-MM-DD\n");
                printf("Product Name: Product_A, Product_B, Product_C, Product_D, Product_E, Product_F, Product_G, Product_H, Product_I\n");
                printf("Algorithm: FCFS, PR, SJF\n");
                printf("Order Number: PXXXX\n");
                printf("Quantity: a positive integer\n");
                printf("Due Date: in the period\n");
                printf("Batch file: a \".dat\" file with addORDER commands\n\n");
            }
            else if (strcmp(c[0], "exitPLS") == 0)
            {
                printf(resetColor "Bye-bye!\n");
                for (int i = 0; i < 2; i++)
                {
                    write(exitNormally[i][1], "exit", 2);
                    close(exitNormally[i][1]);
                }
                break;
            }
        }
    }

    wait(NULL);

    return 0;

#pragma endregion
}