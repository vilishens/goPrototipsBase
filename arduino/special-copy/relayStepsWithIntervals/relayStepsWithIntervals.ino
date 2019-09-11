#include <time.h>

#define POINT_NAME                      "Testa mezgls"  // iekārtas nosaukums (ne vairāk par 30 simboliem)

#define ACTION_SET_ON                   1
#define ACTION_SET_OFF                  2
#define LAIKA_TEKSTA_FORMATS            "%Y-%m-%d %H:%M:%S"
#define POINT_NAME_MAX_LEN              30

#define ONE_SECOND                      1000 // tūkstoš milisekundes

void actionRelay(void);
void logString(char *str, int len);// char *tm);
void setGpio(int gpio, int action);
void startString(char *str, int len);  
void timeString(char *str, int len);
int relayValue(int action);

struct wire {
    const char *name;
    const int pin;
    const int gpio;
};

struct wire wire1 = {
    "Violetais vads",   // vada nosaukums
    5,                  // kontakta apzīmējuma numurs
    14,                 // GPIO numurs (tas tiek izmantots kodā)
};

struct wire wire2 = {
    "Zilais vads",      // vada nosaukums
    7,                  // kontakta apzīmējums numurs
    13,                 // GPIO numurs (tas tiek izmantots kodā)
};

struct step {
    struct wire *stepWire;
    const int duration;
    const int action;
};

struct step Steps[] = {
    {&wire1,  10 * ONE_SECOND, ACTION_SET_ON},    
    {&wire1,   5 * ONE_SECOND, ACTION_SET_OFF},    
    {&wire2,   4 * ONE_SECOND, ACTION_SET_ON},    
    {&wire2,   2 * ONE_SECOND, ACTION_SET_OFF},    
};

int stepIndex = 0;
int Done = 0;
int cycleCount = 0;
int maxNameLen = 0;

int relayValue(int action) {
    return ACTION_SET_ON == action ? LOW : HIGH;
}

void setGpio(int gpio, int action) {
    digitalWrite(gpio, relayValue(action)); 
}

void actionRelay() {
    int strSize = 256 +1;
    char logStr[strSize];
 
    int gpio = Steps[stepIndex].stepWire->gpio;
    setGpio(gpio, Steps[stepIndex].action); 

    char timeStr[strSize];
    timeString(timeStr, sizeof(timeStr));

    logString(logStr, strSize);//, timeStr);
    Serial.println(logStr); 
    Done = 0;
}

void timeString(char *str, int len) {

    time_t current_time;
    struct tm * time_info;

    time(&current_time);
    time_info = localtime(&current_time);

    strftime(str, len, LAIKA_TEKSTA_FORMATS, time_info);
}

void logString(char *str, int len) {   
    // objecta nosaukums
    char format[56];
    snprintf(format, sizeof(format), "%%%is", maxNameLen);
    char nameStr[128];
    snprintf(nameStr, sizeof(nameStr), format, Steps[stepIndex].stepWire->name);

    // iekartas nosaukums
    char pointStr[POINT_NAME_MAX_LEN + 1]; 
    snprintf(format, sizeof(format), "%%%is", POINT_NAME_MAX_LEN);
    snprintf(pointStr, sizeof(pointStr), format, POINT_NAME);

    // laika teksta rinda
    char tm[256 + 1];
    timeString(tm, sizeof(tm));

    // cikla numurs
    char cycleStr[50];
    snprintf(cycleStr, sizeof(cycleStr), "#%i#", cycleCount); 

    //<solis>/<cikla solu skaits>
    char nbrStr[100];
    snprintf(nbrStr, sizeof(nbrStr), "(%i/%i)", stepIndex + 1, (sizeof(Steps)/sizeof(Steps[0]))); 
            
    snprintf(str, len, "%6s %9s *** %s *** %s \"%s\" (darbibas ilgums %3i; pin %2i; GPIO %2i) ---> %3s"
                            , cycleStr        
                            , nbrStr
                            , tm
                            , pointStr
                            , nameStr
                            , int (Steps[stepIndex].duration/ONE_SECOND)
                            , Steps[stepIndex].stepWire->pin
                            , Steps[stepIndex].stepWire->gpio
                            , ACTION_SET_ON == Steps[stepIndex].action ? "ON" : "OFF"
                            );
}

void stringStart(char *str, int len) {

    char timeStr[56];
    
    timeString(timeStr, sizeof(timeStr));

    char strT[56];
    snprintf(strT, sizeof(strT), "%s - Settings", timeStr);
    int strN = strlen(strT);

    char fillStr[56];
    memset(fillStr, 0, sizeof(fillStr));
    int i = 0;
    for(i = 0; i < strN; i++) {
        fillStr[i] = '*';
    }

    snprintf(str, len, "\n\n%s\n%s\n%s\n", fillStr, strT, fillStr);
}

void setup() {
    Serial.begin(9600);

    stepIndex = 0;
    cycleCount = 0;

    int i = 0;
    for(i = 0; i < (sizeof(Steps)/sizeof(Steps[0])); i++) {
        int gpio = Steps[i].stepWire->gpio;
        pinMode(gpio, OUTPUT);
        digitalWrite(gpio, relayValue(ACTION_SET_OFF));
        int len = strlen(Steps[i].stepWire->name);
        if(len > maxNameLen) {
            maxNameLen = len;
        }
    }

    char str[512];
    stringStart(str, sizeof(str));
    Serial.println(str);
}

void loop() {
    if(!stepIndex) {
        cycleCount++;
    }
    
    actionRelay();

    int duration = Steps[stepIndex].duration;
    int sleep = 100;

    while(0 < duration) {
        delay(sleep);       // *** WeMos R1 D2 (ESP2866)
        duration -= sleep;
    }

    stepIndex++;
    if(stepIndex >= (sizeof(Steps)/sizeof(Steps[0]))) {
        stepIndex = 0;
    }
}
