/*
    This sketch demonstrates how to scan WiFi networks.
    The API is almost the same as with the WiFi Shield library,
    the most obvious difference being the different file you need to include:
*/
#include "ESP8266WiFi.h"
#include <time.h>
#include <WiFiUdp.h>

//***** Connection settings - START
#define LOCAL_UDP_PORT                  49250
#define REMOTE_UDP_ADDRESS              "192.168.1.9"
#define REMOTE_UDP_PORT                 49750
#define REMOTE_UDP_STATION              "bazargans"
#define REMOTE_UDP_PASS                  "Mi0dirg8ss"
//***** Connection settings - FINISHI

#define ONE_SECOND                      1000    // tūkstoš milisekundes
#define CONNECTION_LIMIT                20      // 10 sekundes (20x) savienojumam
#define TIME_TEXT_FORMAT                "%Y-%m-%d %H:%M:%S"

IPAddress string2IP(const char *strIP); 
int connectClient();
int incommingMsg();
int outcommingMsg();
int setWifiClient();
int stationIndex(const char *station);
void msgSetupFailed();
void msgSetupOk();
void timeString(char *str, int len) ;
void wifiInit();

WiFiUDP Udp;
int Ok;
char InMsg[512];
char OutMsg[512];

void setup() {

    Ok = 0;
    
    Serial.begin(9600);
    
    Serial.printf("\n\n");
    
    wifiInit();

    if(setWifiClient()) {
        Ok = 1;
    }    

    Serial.printf("\n##### SETUP #####\n");
    if(Ok) {
         msgSetupOk();
    }
    else {
        msgSetupFailed();
    }    
}

void loop() {
    if (Ok) {

        memset(OutMsg, 0, sizeof(OutMsg));
        
        if(incommingMsg()) {
            
            Serial.println("### SANEMU msg ###");     

            strcpy(OutMsg, " *SANEMU * ");
            

            strncat(OutMsg, InMsg, sizeof(OutMsg));    
        } else {
            strcpy(OutMsg," --> Pitanga"); 
        }


        Serial.println(OutMsg);     

        char timeStr[100];
        timeString(timeStr, sizeof(timeStr));
         
        Serial.printf("\"%s\" --- \"%s\"\n", timeStr, OutMsg);     


        char gims[512];
 
        snprintf(gims, sizeof(gims), "%s%s", timeStr, OutMsg); 

        strcpy(OutMsg, gims);



        if(strlen(OutMsg)) {
            if(!outcommingMsg(OutMsg)){
                Serial.printf("Failure! Output message failed!\n");     
            }
        }    
    } else {
        Serial.println("Connection failed...");
    }
    // Wait a bit before scanning again
    delay(ONE_SECOND * 2);
}

void wifiInit() {
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(ONE_SECOND / 10);
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
    if (0 > stationIndex(REMOTE_UDP_STATION)) {
        Serial.printf("Failure! Couldn't find the station specified --- \"%s\"\n"
                        , REMOTE_UDP_STATION);
        return 0; 
    }

    // Connect to the station    
    WiFi.begin(REMOTE_UDP_STATION, REMOTE_UDP_PASS);

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
        Serial.printf("Failure! Couldn't connect to the station --- \"%s\"\n"
                        , REMOTE_UDP_STATION);
        return 0;                        
    }
   
    return 1;
}

int incommingMsg() {
    
    memset(InMsg, 0, sizeof(InMsg));
    
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
        Serial.printf("Received %d bytes from %s:%d\n"
            , packetSize
            , Udp.remoteIP().toString().c_str()
            , Udp.remotePort());
        int len = Udp.read(InMsg, sizeof(InMsg));
        if (len > 0)
        {
            InMsg[len] = 0;
        }
        Serial.printf("UDP packet contents: %s\n", InMsg);
        return len;
    } 

    return 0;         
}

int outcommingMsg(const char *out) {

    if(!strlen(out)) {
        return 1;
    }

    // send a reply, to the remote address and port
    IPAddress ip = string2IP(REMOTE_UDP_ADDRESS);
    
    if(!Udp.beginPacket(ip, REMOTE_UDP_PORT)) {
        // there is a problem with the supplied IP address or port 
        Serial.printf("Failure! Sending problen with the address %s:%i\n"
                        , REMOTE_UDP_ADDRESS, REMOTE_UDP_PORT);
        return 0;
    }
    
    if(!Udp.write(out)) {
        // there is a problem with the supplied IP address or port 
        Serial.printf("Failure!Sending problen with data writing\n");
        return 0;      
    }
    
    if (!Udp.endPacket()) {
        // couldn't finish off the packet and send it 
        Serial.printf("Failure! Sending couldn't finish off the packet and send it\n");
        return 0;      
    }    

    return 1;
}


IPAddress string2IP(const char *strIP) 
{
    IPAddress ip(0,0,0,0);

    if (ip.fromString(strIP)) 
    {
        int ip0, ip1, ip2, ip3; 
        sscanf(strIP, "%i.%i.%i.%i",&ip0, &ip1
                                   , &ip2, &ip3);
        ip = IPAddress(ip0, ip1, ip2, ip3);                                                
    }
    
    return ip;    
}

void timeString(char *str, int len) {

    time_t current_time;
    struct tm * time_info;

    time(&current_time);
    time_info = localtime(&current_time);

    strftime(str, len, TIME_TEXT_FORMAT, time_info);
}

int setWifiClient() {
    IPAddress ip(0,0,0,0);
    if (!ip.fromString(REMOTE_UDP_ADDRESS)) {
        Serial.printf("Failure! Wrong the remote UDP address --- %s\n"
                        , REMOTE_UDP_ADDRESS);
        return 0;
    }

    if(!connectClient()) {
        return 0;
    }    
        
    // Starts WiFiUDP socket, listening at local port PORT 
    if(!Udp.begin(LOCAL_UDP_PORT)) {
        Serial.printf("Failure! Couldn't start to listen the local port --- %i\n"
                        , LOCAL_UDP_PORT);
        return 0;
    }

    return 1;
}

void msgSetupFailed() {
    Serial.printf("Board Setup Failure!\n\n\n");
}

void msgSetupOk() {

    IPAddress localIp = WiFi.localIP();
    int locN = strlen(localIp.toString().c_str());
    int remN = strlen(REMOTE_UDP_ADDRESS);
    int maxN = locN > remN ? locN : remN; 

    char forma[200];
    snprintf(forma, sizeof(forma), "    %%s: %%%is:%%i\n", maxN);     

    Serial.printf(forma, " Local:", localIp.toString().c_str(), LOCAL_UDP_PORT);
    Serial.printf(forma, "Remote:", REMOTE_UDP_ADDRESS, REMOTE_UDP_PORT);
    Serial.printf("\n\n");
}

