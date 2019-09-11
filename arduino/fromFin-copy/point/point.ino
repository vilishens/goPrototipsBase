/*
    This sketch demonstrates how to scan WiFi networks.
    The API is almost the same as with the WiFi Shield library,
    the most obvious difference being the different file you need to include:
*/
#include "ESP8266WiFi.h"
#include <time.h>
#include <WiFiUdp.h>

//***** Settings - UDP connection - START
#define POINT_NAME                      "granato"       // the board name of the link
#define STATION_NAME                    "SITKO"         // the name of the supervisor station
#define UDP_PORT                        49750           // the predifined UDP port of the link
#define WIFI_HOTSPOT_NAME               "bazargans"     // the WiFi hotspot name
#define WIFI_HOTSPOT_PASS               "Mi0dirg8ss"    // the WiFi hotspot password
//***** Settings - UDP connection - END

//***** Settings - unified UDP messages - START
#define MSG_INPUT_HELLO_FROM_STATION        0x00000001  // input    <station name><msgCd><msgNbr><station UTC seconds><station offset seconds><stationIP><stationPort>
#define MSG_INPUT_SET_RELAY_GPIO            0x00000004  // input    <station name><msgCd><msgNbr><Gpio><set value>
#define MSG_RESPONSE_HELLO_FROM_POINT       0x00000002  // response <point name><msgCd><pointIP><pointPort>
#define MSG_RESPONSE_SUCCESS                0x00000008  // response <point name><msgSuccessCd><msgNbr> 
#define MSG_RESPONSE_FAILED                 0x00000010  // response <point name><msgFailureCd><msgNbr>

#define MSG_INPUT_LIST_LAST_INDEX               2

struct msgData {
    int msgCd;
    int fieldCount;    
};
struct msgData msgInputList[MSG_INPUT_LIST_LAST_INDEX + 1];

int msgFoundMsg(int msgCd);
int handleInput(char *msg, int ind);
int messageNumber = -1;
int pointSigned = 0;
int lastMsg = -1;

void msgInputListInit();
void msgSetupFailed();
void msgSetupOk();
//***** Settings - unified UDP messages - END

#define FIELD_SEPARATOR                 ":::"
#define ONE_SECOND                      1000    // tūkstoš milisekundes
#define CONNECTION_LIMIT                20      // savienojumu reizes savienojumam (10 sekundes (20x))
#define TIME_TEXT_FORMAT                "%Y-%m-%d %H:%M:%S"

#define RC_SUCCESS                      0
#define RC_FAILED                       1

#define UDP_PORT_MIN                    49152
#define UDP_PORT_MAX                    65535




int string2IP(IPAddress *ip, const char *strIP); 
int cmdIndex(int cmd);
int connectClient();
//int foundCmd(int cmdCd);
//int handleInput(char *msg, int ind);
int incommingMsg();
int outcommingMsg(char *data);
int sendResponse();
int msgHelloFromStation(char *msg, int ind);
int msgSetRelayGpio(char *msg, int ind);
int setWifiClient();
char *getMsgField(char *msg, int nbr);
int stationIndex(const char *station);
void responseHelloFromStation();
void responseResult(int msgNbr, int result);
void setSecondTime(int secs);
void timeString(char *str, int len) ;
void timeZoneString(char *str, int len);
void wifiInit();
/***********
*** GPIO ***
************/
#define GPIO_ON                   1
#define GPIO_OFF                  2
int relayValue(int action);
void setRelayGpio(int gpio, int action);

WiFiUDP UDPwifi;
int Ok;
char UDPin[512];
char UDPout[512];
char UDPremoteAddress[256];
int UDPremotePort;
int stationOffsetSeconds = 0;

//#define MSG_LIST_LAST_INDEX               2
//struct cmdData msgList[MSG_LIST_LAST_INDEX + 1];

void setup() {
    Ok = 0;
    
    Serial.begin(9600); 
    Serial.println("\n");

    int attempt = 0;

    while(!Ok) {
        wifiInit();
        msgInputListInit();

        memset(UDPremoteAddress, 0, sizeof(UDPremoteAddress));
        if(setWifiClient()) {
            Ok = 1;
        }    

        attempt++;
        Serial.print("\n##### SETUP ");
        Serial.print(attempt);
        Serial.println(" #####");

        Ok ? msgSetupOk() : msgSetupFailed();

        if(!Ok) {
            // Wait before the next attempt
            delay(10 * ONE_SECOND);
            
            Serial.println();
            Serial.println();
            Serial.println();
        }
    }        
}

void loop() {
    if (Ok) {

        int timeChrs = 180;
        char strTime[timeChrs];
        messageNumber = -1;
        
        if(incommingMsg()) {
            memset(UDPout, 0, sizeof(UDPout));
        }    
        
        if(strlen(UDPin)) {

            timeString(strTime, timeChrs);
            
            Serial.print(strTime);
            Serial.print(" ### "); 
            Serial.print("INPUT received \""); 
            Serial.print(UDPin);
            Serial.println("\"");

            if(outcommingMsg(UDPin)) {
                Serial.print(strTime);
                Serial.print(" ### "); 
                Serial.println("Failure! Output message failed!\n");     
            } else if(strlen(UDPout)) {
                if(sendResponse()) {
                    Serial.print(strTime);
                    Serial.print(" ### "); 
                    Serial.print("Failure! The response message \"");
                    Serial.print(UDPout);
                    Serial.println("\" failed!\n");
                } else if (MSG_INPUT_HELLO_FROM_STATION == lastMsg) {
                    pointSigned = 5;
                }
            }
        }     
    } else {
        Serial.println("Connection failed...");
    }
    // Wait a bit before scanning again
    delay(ONE_SECOND / 10);
}


void responseHelloFromStation() {
    memset(UDPout, 0, sizeof(UDPout));

    snprintf(UDPout, sizeof(UDPout), "%s%s%i%s%i%s%s%s%i"
                                    , POINT_NAME
                                    , FIELD_SEPARATOR                                
                                    , MSG_RESPONSE_HELLO_FROM_POINT
                                    , FIELD_SEPARATOR
                                    , messageNumber
                                    , FIELD_SEPARATOR                               
                                    , WiFi.localIP().toString().c_str()
                                    , FIELD_SEPARATOR
                                    , UDP_PORT);
}

void responseResult(int result) {
    memset(UDPout, 0, sizeof(UDPout));

    snprintf(UDPout, sizeof(UDPout), "%s%s%i%s%i"
                                    , POINT_NAME
                                    , FIELD_SEPARATOR
                                    , (RC_SUCCESS == result) ? MSG_RESPONSE_SUCCESS : MSG_RESPONSE_FAILED                                    
                                    , FIELD_SEPARATOR                                  
                                    , messageNumber);
}

int sendResponse() {
    // send a reply, to the remote address and port
    IPAddress ip;
    if(string2IP(&ip, UDPremoteAddress)) {
        if(strlen(UDPremoteAddress)) {
            Serial.printf("Failure! The invalid remote address \"%s\"\n", UDPremoteAddress);
        } else {
            Serial.printf("Failure! The remote address hasn't set yet\n");    
        } 
        return 1;        
    }

    if((UDP_PORT_MIN > UDPremotePort) || (UDPremotePort > UDP_PORT_MAX)) {
        Serial.printf("Failure! The invalid remote port %i (%i - %i)\n"
                        , UDPremotePort, UDP_PORT_MIN, UDP_PORT_MAX);
        return 1;        
    } 
    
    if(!UDPwifi.beginPacket(ip, UDPremotePort)) {
        // there is a problem with the supplied IP address or port 
        Serial.printf("Failure! Sending problen with the address %s:%i\n"
                        , UDPremoteAddress, UDPremotePort);
        return 1;
    }

    Serial.print("### RESPONSE: ");
    Serial.print(UDPout);
    Serial.println();
    Serial.println("\n");
    
    if(!UDPwifi.write(UDPout)) {
        // there is a problem with the supplied IP address or port 
        Serial.printf("Failure!Sending problen with data writing\n");
        return 1;      
    }
    
    if (!UDPwifi.endPacket()) {
        // couldn't finish off the packet and send it 
        Serial.printf("Failure! Sending couldn't finish off the packet and send it\n");
        return 1;      
    }    
    
    return 0;
}

void setSecondTime(int secs) {
    struct timeval tv = { secs, 0 };
    settimeofday(&tv, NULL);
}

int msgHelloFromStation(char *msg, int ind) {

    char *str4 = getMsgField(msg,  4);  // station time UTC seconds
    char *str5 = getMsgField(msg,  5);  // station time offset seconds
    char *str6 = getMsgField(msg,  6);  // station IP address
    char *str7 = getMsgField(msg, -7);  // station UDP port

    Serial.print(" TIME: \"");
    Serial.print(str4 ? str4 : "XXXXX");
    Serial.print("\" ");
    Serial.println(str4 ? strlen(str4) : 1234567);
    Serial.print("  OFF: \"");
    Serial.print(str5 ? str5 : "XXXXX");
    Serial.print("\" ");
    Serial.println(str5 ? strlen(str5) : 1234567);
    Serial.print("   IP: \"");
    Serial.print(str6 ? str6 : "XXXXX");
    Serial.print("\" ");
    Serial.println(str6 ? strlen(str6) : 1234567);
    Serial.print(" PORT: \"");
    Serial.print(str7 ? str7 : "XXXXX");
    Serial.print("\" ");
    Serial.println(str7 ? strlen(str7) : 1234567);

    int rc = RC_SUCCESS;
    if(!str4 || !str5 || !str6 || !str7) {
        rc = RC_FAILED; 
    }

    if(rc == RC_SUCCESS) {
        // set the point time the same as the station has        
        setSecondTime(strtol(str4, NULL, 0));
        // remember the station time offset seconds
        stationOffsetSeconds = (int) strtol(str5, NULL, 0);
        // set the station IP address
        snprintf(UDPremoteAddress, sizeof(UDPremoteAddress), "%s", str6);
        // set the station port
        UDPremotePort = (int) strtol(str7, NULL, 0);

        // prepare the response    
        responseHelloFromStation();
    }    

    if(str4) free(str4);
    if(str5) free(str5);
    if(str6) free(str6);
    if(str7) free(str7);

    return rc;    
}

int msgSetRelayGpio(char *msg, int ind) {

    char *str4 = getMsgField(msg,  4);
    char *str5 = getMsgField(msg, -5);

    Serial.print(" GPIO: \"");
    Serial.print(str4 ? str4 : "XXXXX");
    Serial.print("\" ");
    Serial.println(str4 ? strlen(str4) : 1234567);
    Serial.print("  SET: \"");
    Serial.print(str5 ? str5 : "XXXXX");
    Serial.print("\" ");
    Serial.println(str5 ? strlen(str5) : 1234567);

    int rc = RC_SUCCESS;
    if(!str4 || !str5) {
        rc = RC_FAILED; 
    }

    if(rc == RC_SUCCESS) {
        int gpio_n = (int) strtol(str4, NULL, 0);
        int value = (int) strtol(str5, NULL, 0);

        setRelayGpio(gpio_n, value);
    }    

    if(str4) free(str4);
    if(str5) free(str5);

    return rc;    
}

int handleInput(char *msg, int ind) {

    int rc = RC_FAILED;
    lastMsg = -1;

    if(!pointSigned && (msgInputList[ind].msgCd != MSG_INPUT_HELLO_FROM_STATION)) {
        // don't even try to make the response message if there is no remote address and the message isn't Hello from Station 
        // (the point didn't receive or loose the station data) 
        // it is required to get Hello from Station message to start handle incomming messages  
        Serial.printf("Failure! The point isn't signed yet\n");    
        
        return RC_FAILED;        
    }

    lastMsg = msgInputList[ind].msgCd;
    
    switch(msgInputList[ind].msgCd) {
        case MSG_INPUT_HELLO_FROM_STATION:
            // this is a start message to establish the connection
            // between a station and a point  
            memset(UDPremoteAddress, 0, sizeof(UDPremoteAddress));
            rc = msgHelloFromStation(msg, ind);

            return rc;
            break;
        case MSG_INPUT_SET_RELAY_GPIO:
            rc = msgSetRelayGpio(msg, ind);
            break;    
        default:
            Serial.print("ERROR --- Input message \"");
            Serial.print(msgInputList[ind].msgCd);    
            Serial.println("\" doesn't have logic");
    }

    responseResult(rc);

    return rc;
}

char *getMsgField(char *msg, int nbr) {

    int len = strlen(msg);
    int i;

    int count = nbr >= 0 ? nbr : -nbr;  // counter of found separators, negative means read till the end  
    
    int f_start = 0;
    int f_end = 0;
    int found_n = 0;
    char *str = NULL;
    
    for(i = 0; i < len; ++i) {
        int is_separ = !strncmp(msg + i, FIELD_SEPARATOR, strlen(FIELD_SEPARATOR));

        int f_get = 0;
        if(is_separ) {   
            // the separator is found      
            found_n++;
            f_start = (f_end == 0 ? 0 : f_end + strlen(FIELD_SEPARATOR));
            f_end = i;
        }

        if(is_separ && (found_n == count)) {
            // the required count of the separator has found
            f_get = 1;                      
        } else if(is_separ && (nbr < 0) && (found_n == (-nbr - 1))) {
            // read the last field from the separator till the end of the message
            f_start = (f_end == 0 ? 0 : f_end + strlen(FIELD_SEPARATOR));
            f_end = len;
            f_get = 1;    
        }

        if(f_get) {
            int new_size = f_end - f_start + 1;           
            str =(char *) malloc(new_size * sizeof(char));

            if(str) {
                memset(str, 0, new_size);
                strncpy(str, msg + f_start, f_end - f_start);
            }

            return str;      
        }
    }

    return str;
}

void msgInputListInit() {
    msgInputList[0].msgCd      = MSG_INPUT_HELLO_FROM_STATION;  
    msgInputList[0].fieldCount = 5; 

    msgInputList[1].msgCd      = MSG_INPUT_SET_RELAY_GPIO;
    msgInputList[1].fieldCount = 4;
    
    msgInputList[MSG_INPUT_LIST_LAST_INDEX].msgCd      = -7;  
    msgInputList[MSG_INPUT_LIST_LAST_INDEX].fieldCount = -5;   
}

int msgFoundMsg(int msgCd) {

    int i = 0;
    for(i = 0; msgInputList[i].msgCd >= 0; i++) {
        if(msgCd == msgInputList[i].msgCd) {
            return i;
        }
    }

    return -5;
}

void wifiInit() {
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(ONE_SECOND / 10);
}

int setWifiClient() {
    if(!connectClient()) {
        return 0;
    }    
        
    // Starts WiFiUDP socket, listening at local port PORT 
    if(!UDPwifi.begin(UDP_PORT)) {
        Serial.printf("Failure! Couldn't start to listen the point port --- %i\n"
                        , UDP_PORT);
        return 0;
    }

    return 1;
}

int stationIndex(const char *station) {
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
        String name = WiFi.SSID(i);
        if (!strcmp(name.c_str(), station)) {
            return i;           
        }
    }

    return -1;
}

int connectClient() {

    // Check the station in available connections
    if (0 > stationIndex(WIFI_HOTSPOT_NAME)) {
        Serial.printf("Failure! Couldn't find the WIFI hotspot specified --- \"%s\"\n"
                        , WIFI_HOTSPOT_NAME);
        return 0; 
    }

    // Connect to the station    
    WiFi.begin(WIFI_HOTSPOT_NAME, WIFI_HOTSPOT_PASS);

    int limit = CONNECTION_LIMIT;
    int cd = WL_CONNECT_FAILED;
    Serial.print("Connecting: ");
    while ((cd != WL_CONNECTED) && limit) {
        cd = WiFi.status();
        if (cd != WL_CONNECTED) {
            Serial.print(".");
            delay(ONE_SECOND / 2);
            limit--;
        }    
    }
    Serial.println();

    if (cd != WL_CONNECTED) {   
        Serial.printf("Failure! Couldn't connect to the WIFI hotspot --- \"%s\"\n"
                        , WIFI_HOTSPOT_NAME);
        return 0;                        
    }
   
    return 1;
}

void msgSetupFailed() {
    Serial.println("Board Setup Failure!\n\n\n");
}

void msgSetupOk() {

    IPAddress localIp = WiFi.localIP();

    Serial.print("    The point \"");
    Serial.print(POINT_NAME);
    Serial.print("\" (");
    Serial.print(localIp.toString().c_str());
    Serial.print(":");
    Serial.print(UDP_PORT);    
    Serial.print(") connected to the WIFI hotspot \"");    
    Serial.print(WIFI_HOTSPOT_NAME);
    Serial.print("\"");    
    Serial.print("\n\n\n");

    return;
}

int incommingMsg() {
    
    memset(UDPin, 0, sizeof(UDPin));
    
    int packetSize = UDPwifi.parsePacket();
    if (packetSize)
    {
//        Serial.printf("Received %d bytes from %s:%d\n"
//            , packetSize
//            , UDPwifi.remoteIP().toString().c_str()
//            , UDPwifi.remotePort());
        int len = UDPwifi.read(UDPin, sizeof(UDPin));
        if (len > 0)
        {
            UDPin[len] = 0;
        }
        return len;
    } 

    return 0;         
}

void timeZoneString(char *str, int len) {
    // (tz +/-HH:MM)

    int hr = 0;
    int mins = 0;

    if (stationOffsetSeconds) {
        mins = stationOffsetSeconds / 60;
        hr = mins / 60;
        mins = mins % 60;
    }    

    snprintf(str, len, "(tz %s%02i:%02i)"
        , !stationOffsetSeconds ? "" : (stationOffsetSeconds > 0) ? "+" : "-"
        , hr
        , mins);     
}

void timeString(char *str, int len) {
    char str1[len];
    char str2[len];

    time_t seconds;             // Unix seconds
    struct tm * time_info;
    struct timeval tv;

    gettimeofday(&tv, NULL); 
    seconds=tv.tv_sec;
    time_info = localtime(&seconds);

    // string of the current time
    strftime(str1, len, TIME_TEXT_FORMAT, time_info);
    // string of the current time time zone
    timeZoneString(str2, len);

    snprintf(str, len, "%s %s", str1, str2); 
}

int outcommingMsg(char *data) {

    char errMsg[512 + 1];
    memset(UDPout, sizeof(UDPout), 0);

    if(!strlen(data)) {
        // nothing to send but it's not an error
        return RC_SUCCESS;
    }

    int rcSuccess = 0;
    char *station = NULL, *msgCd = NULL, *msgNbr = NULL;

    // get the sender station name
    station = getMsgField(data,  1);
    if(!station) {
        return RC_FAILED;
    }    

    snprintf(errMsg, sizeof(errMsg), "\"%s\" is not the right station for this point", station);
    int msgFound = !strcmp(station, STATION_NAME);
    free(station);

    if(!msgFound) {
        Serial.println(errMsg);
          
        return RC_FAILED;
    }
               
    msgCd = getMsgField(data,  2);
    if(!msgCd) {
        return RC_FAILED;
    }    

    int msgInd = msgFoundMsg((int) strtol(msgCd, NULL, 0));
    snprintf(errMsg, sizeof(errMsg), "\"%s\" message is not in the accepted message type list", msgCd);
    free(msgCd);

    if(0 > msgInd) {
        Serial.println(errMsg);
        
        return RC_FAILED;
    }

    msgNbr = getMsgField(data,  3);
    if(!msgNbr) {
        snprintf(errMsg, sizeof(errMsg), "\"%s\" message has invalid form couldn't find the message number", data);
        Serial.println(errMsg);
        
        return RC_FAILED;        
    }

    messageNumber = (int) strtol(msgNbr, NULL, 0);
    free(msgNbr);

    return handleInput(data, msgInd);
}

int string2IP(IPAddress *ip, const char *strIP) 
{
    *ip = IPAddress(0,0,0,0);
    if (!(*ip).fromString(strIP)) {
        return RC_FAILED; 
    }
        
    int ip0, ip1, ip2, ip3; 
    sscanf(strIP, "%i.%i.%i.%i",&ip0, &ip1
                                   , &ip2, &ip3);
    *ip = IPAddress(ip0, ip1, ip2, ip3); 

    return RC_SUCCESS;                                               
}

//***********************************************
//*********** Relay GPIO ************************
//*********************************************** 
void setRelayGpio(int gpio, int action) {
    pinMode(gpio, OUTPUT);
    digitalWrite(gpio, relayValue(action)); 
}

int relayValue(int action) {
    return GPIO_ON == action ? LOW : HIGH;
}

//    int gpio = Steps[stepIndex].stepWire->gpio;
//    setGpio(gpio, Steps[stepIndex].action); 


//        pinMode(gpio, OUTPUT);
//        digitalWrite(gpio, relayValue(ACTION_SET_OFF));

