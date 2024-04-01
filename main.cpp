#include "mbed.h"
#include "wifi.h"
#include <cstdio>
#include <string>

#include "VL53L0X.h"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <cstdint>
#include <algorithm>
/*------------------------------------------------------------------------------
Hyperterminal settings: 115200 bauds, 8-bit data, no parity
Need library DISCO_L475VG_IOT01A_wifi
https://os.mbed.com/teams/ST/code/DISCO_L475VG_IOT01A_wifi/
Notice to change app.json to your wifi network
Also copy the returned ip adress to you client
This example
- connects to a wifi network (SSID & PWD to set in mbed_app.json)
- displays the IP address and sent message
- the message counter reset as button pressed


This example uses SPI3 ( PE_0 PC_10 PC_12 PC_11), wifi_wakeup pin (PB_13),
wifi_dataready pin (PE_1), wifi reset pin (PE_8)
------------------------------------------------------------------------------*/


/* Private defines -----------------------------------------------------------*/
#define WIFI_WRITE_TIMEOUT 10000
#define WIFI_READ_TIMEOUT  10000
#define PORT           80


/* Private typedef------------------------------------------------------------*/
typedef enum
{
WS_IDLE = 0,
WS_CONNECTED,
WS_DISCONNECTED,
WS_ERROR,
} WebServerState_t;


/* Private macro -------------------------------------------------------------*/
static int wifi_setup(void);
static void WebServerProcess(void);
static WIFI_Status_t SendMessage(void);
static void Socket_connect(void);
/* Private variables ---------------------------------------------------------*/
BufferedSerial pc(USBTX, USBRX, MBED_CONF_QUECTEL_BG96_BAUDRATE);
static   uint8_t resp[1024];
uint16_t respLen;


uint8_t  IP_Addr[4];
uint8_t  MAC_Addr[6];
int32_t Socket = -1;
static   WebServerState_t  State = WS_ERROR;
char     ModuleName[32];
DigitalOut led(LED3);
int count_sent;
int cmd_to_sent;
int screen_off;
int HDMI_num;
DigitalIn button(BUTTON1);

bool sent;


// Create I2C instance
DevI2C i2c(PB_11, PB_10);

// Create DigitalOut instance 
DigitalOut pin(PC_6);
DigitalOut led1(LED1);

// Create Interrupt from VL53L0X
// InterruptIn interrupt(PC_7);

// Create VL53L0X instance
VL53L0X sensor(&i2c, &pin, PC_7);

// Buffers for gesture recogenition
uint16_t range[50];
uint32_t ambient[50];

// Helper function to refill buffer
template <typename T, std::size_t N>
void fillMax(T (&arr)[N]) {
    std::fill(std::begin(arr), std::end(arr), std::numeric_limits<T>::max());
}

void handleScreen(){
    if(screen_off==1){
           cmd_to_sent = 1;
           screen_off=0;
       }
    else{
           cmd_to_sent = 2;
           screen_off=1;
       }
}
void handleHDMI(){
    if(HDMI_num==3){
           cmd_to_sent = 4;
           HDMI_num=4;
    }
    else{
           cmd_to_sent = 3;
           HDMI_num=3;
    }
}

void handleBrightness(){
    cmd_to_sent = 5;
}

template <std::size_t N>
void detectGesture(uint16_t (&ranges)[N], uint32_t (&ambients)[N]) {
    // uint16_t last_max = 0;
    // uint16_t last_min = std::numeric_limits<uint16_t>::max();
    // uint16_t r_ini = ranges[0];
    int taps = 0;
    int swipes = 0;

    bool tapReady = true;
    bool swipeReady = true;

    uint16_t lastR = ranges[0];
    for(const auto& r:ranges){
        if(r==0 || r == 8190 || r == std::numeric_limits<uint16_t>::max()){
            break;
        }
        // printf("%d\n", r);
        if(r < lastR - 50){
            if(tapReady){
                taps ++;
                tapReady = false;
            }
            lastR = r;
        }else if (r > lastR + 50) {
            tapReady = true;
            lastR = r;
        }
    }

    if(taps == 0){
        for(const auto& a:ambients){
            if(a == 0 || a == std::numeric_limits<uint32_t>::max()){
                break;
            }
            // printf("%d\n", a);
            if(a < 5120){
                if(swipeReady){
                    swipes ++;
                    swipeReady = false;
                }
            }else if (a > 15360) {
                swipeReady = true;
            }
        }
        printf("Swipes: %d\n", swipes);
        if(swipes==1){
            handleBrightness();
        }
        else if(swipes==2){
            handleHDMI();
        } 
    }else if (taps == 1) {
        handleScreen();
    }else{
        printf("Taps: %d\n", taps);
    }
}


int init_sensor(VL53L0X sensor){
   sensor.VL53L0X_on();
    int exit_code = 0;
    // Initialize the sensor
    if (sensor.init(NULL) != 0) {
        printf("Error initializing sensor\n");
        exit_code = -1;
    }
    printf("Sensor Initialized\n");
    // Prepare the sensor for operation
    if (sensor.prepare() != 0) {
        printf("Error preparing sensor\n");
        exit_code = -1;
    }
    printf("Sensor Prepared\n");
    VL53L0X_DEV dev;
    if(sensor.vl53l0x_get_device(&dev)!=0){
        printf("Error get device\n");
        exit_code = -1;
    }
    printf("Device Gotten\n");
    if(sensor.VL53L0X_set_measurement_timing_budget_micro_seconds(dev, 20000)!=0){
        printf("Error Setting Time Budget");
        exit_code = -1;
    }
    if (sensor.start_measurement(OperatingMode::range_single_shot_polling, NULL) != 0) {
        printf("Error starting measurement\n");
        exit_code = -1;
    }
    printf("Measurement Started\n");
    return exit_code;
}


int main()
{
   int ret = 0;
   led = 0;
   // pc.baud(115200);

    ret = wifi_setup();
    if (ret != 0) {
        exit(-1);
    }
    Socket_connect();

    if(init_sensor(sensor)!= 0){
        exit(-1);
    }

    int counter;
    Timer t;
    VL53L0X_RangingMeasurementData_t measurement;
    VL53L0X_HistogramMeasurementData_t histogram;
    while(1) {
        if (cmd_to_sent != 0){
            WebServerProcess();
            if (State==WS_ERROR){
                break;
            }
        }

        if (button == 0) {
            count_sent = 0; // Reset the count when the button is pressed
            printf("Count reset to 0\n");
            if(screen_off==1){
                cmd_to_sent = 1;
                screen_off=0;
            }
            else{
                cmd_to_sent = 2;
                screen_off=1;
            }
            ThisThread::sleep_for(1s); // Wait for 1 second to debounce
        }

        sensor.get_measurement(OperatingMode::range_single_shot_polling, &measurement);
        if(measurement.RangeMilliMeter!=0 && measurement.RangeMilliMeter < 500){

            counter = 0;
            sensor.stop_measurement(OperatingMode::range_single_shot_polling);
            sensor.start_measurement(OperatingMode::range_continuous_polling, NULL);

            t.start();
            printf("Start Measure!\n");
            while(t.read() < 1.0f){
                sensor.get_measurement(OperatingMode::range_continuous_polling, &measurement);
                range[counter] = measurement.RangeMilliMeter;
                ambient[counter] = measurement.AmbientRateRtnMegaCps;
                counter ++;
            }
            printf("Stop Measure!\n");
            t.stop();

            printf("counter: %d\n", counter);

            sensor.stop_measurement(OperatingMode::range_continuous_polling);
            
            detectGesture(range, ambient);

            fillMax(range);
            fillMax(ambient);

            counter = 0;
            t.reset();

            ThisThread::sleep_for(500ms);
            sensor.start_measurement(OperatingMode::range_single_shot_polling, NULL);
        }
   }

}



int wifi_setup(void)
{
   count_sent=0;
   cmd_to_sent=0;
   screen_off=1;
   HDMI_num=4;
   


   /*Initialize and use WIFI module */
   if(WIFI_Init() ==  WIFI_STATUS_OK) {
       printf("ES-WIFI Initialized.\n");


       if(WIFI_GetMAC_Address(MAC_Addr) == WIFI_STATUS_OK) {      
           printf("> es-wifi module MAC Address : %X:%X:%X:%X:%X:%X\n",    
           MAC_Addr[0],
           MAC_Addr[1],
           MAC_Addr[2],
           MAC_Addr[3],
           MAC_Addr[4],
           MAC_Addr[5]);  
           }
       else {
           printf("> ERROR : CANNOT get MAC address\n");
       }
       string ssid = MBED_CONF_APP_WIFI_SSID;
       const char* ssid_c_str = ssid.c_str();


       string password = MBED_CONF_APP_WIFI_PASSWORD;
       const char* password_c_str = password.c_str();


       if(WIFI_Connect(ssid_c_str, password_c_str, WIFI_ECN_WPA2_PSK) == WIFI_STATUS_OK) {
           printf("> es-wifi module connected \n");


           if(WIFI_GetIP_Address(IP_Addr) == WIFI_STATUS_OK) {
               printf("> es-wifi module got IP Address : %d.%d.%d.%d\n",    
               IP_Addr[0],
               IP_Addr[1],
               IP_Addr[2],
               IP_Addr[3]);


           printf(">Start Server... \n");
           printf(">Wait for connection...  \n");
           State = WS_IDLE;
           }
           else {   
               printf("> ERROR : es-wifi module CANNOT get IP address\n");
               return -1;
           }
       } else {
       printf("> ERROR : es-wifi module NOT connected\n");
       return -1;
       }
   } else {
       printf("> ERROR : WIFI Module cannot be initialized.\n");
       return -1;
   }
   return 0;
}


/**
 * @brief  Send message
 * @param  None
 * @retval None
 */
static void Socket_connect(void)
{
        while (State== WS_IDLE){
           Socket = 0;
           WIFI_StartServer(Socket, WIFI_TCP_PROTOCOL, "", PORT);

           if(Socket != -1)
           {
               printf("> HTTP Server Started \n");
               State = WS_CONNECTED;
           }
           else
           {
               printf("> ERROR : Connection cannot be established.\n");
               State = WS_ERROR;
           }
       }
}


/**
 * @brief  Send message
 * @param  None
 * @retval None
 */
static void WebServerProcess(void)
{
   uint8_t LedState = 0;
   float temp;


   switch(State)
   {


       case WS_CONNECTED:
       {
           uint64_t previousTime = us_ticker_read();
           bool errorOccurred = false;


           while(State == WS_CONNECTED && !errorOccurred)
           {
               uint64_t currentTime = us_ticker_read();
               if(currentTime - previousTime >= 1000000) // 100ms in microseconds
               {
                   previousTime = currentTime;
                   if(SendMessage() != WIFI_STATUS_OK)
                   {
                       printf("> ERROR : Cannot send message, please reset.\n");
                       errorOccurred = true;
                       sent = false;
                       led = 1;
                   }else{
                       sent = true;
                   }
                   break;
               }
           }


           if (errorOccurred)
           {
               State = WS_ERROR;
           }
       }
       break;


       case WS_ERROR:
       default:
           break;
   }
}


/**
 * @brief  Send message
 * @param  None
 * @retval None
 */
static WIFI_Status_t SendMessage()
{
   uint16_t SentDataLength;
   WIFI_Status_t ret;
   uint8_t message[1024];


   count_sent++;
   if(cmd_to_sent==0)
   {
       sprintf((char *)message, "message: %d", count_sent);
   }
   else
   {
       sprintf((char *)message, "cmd%d", cmd_to_sent);
       cmd_to_sent=0;
   }


   ret = WIFI_SendData(0, (uint8_t *)message, strlen((char *)message), &SentDataLength, WIFI_WRITE_TIMEOUT);


   if((ret == WIFI_STATUS_OK) && (SentDataLength != strlen((char *)message)))
   {
       ret = WIFI_STATUS_ERROR;
   }


   return ret;
}

