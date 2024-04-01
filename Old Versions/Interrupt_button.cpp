#include "mbed.h"
#include "wifi.h"
#include <string> 
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
static int wifi_sample_run(void);
static void WebServerProcess(void);
static WIFI_Status_t SendMessage(void);
/* Private variables ---------------------------------------------------------*/
BufferedSerial pc(USBTX, USBRX, MBED_CONF_QUECTEL_BG96_BAUDRATE);
static   uint8_t resp[1024];
uint16_t respLen;


uint8_t  IP_Addr[4]; 
uint8_t  MAC_Addr[6]; 
int32_t Socket = -1;
static   WebServerState_t  State = WS_ERROR;
char     ModuleName[32];
DigitalOut led(LED2);
int count_sent;
int cmd_to_sent;
int screen_off;
DigitalIn button(BUTTON1);
InterruptIn button(BUTTON1);

void button_pressed() {
    count_sent = 0; // Reset the count when the button is pressed
    printf("Count reset to 0\n");
    if(screen_off == 1) {
        cmd_to_sent = 1;
        screen_off = 0;
    } else {
        cmd_to_sent = 2;
        screen_off = 1;
    }
    ThisThread::sleep_for(1s); // Wait for 1 second to debounce
    WebServerProcess();
}

int main()
{
    int ret = 0;
    led = 0;
    // pc.baud(115200);
    printf("\n");
    printf("************************************************************\n");
    printf("***   STM32 IoT Discovery kit for STM32L475 MCU          ***\n");
    printf("***         WIFI Web Server demonstration                ***\n\n");
    printf("************************************************************\n");
    
    /* Working application */
    ret = wifi_sample_run();
    
    if (ret != 0) {
        return -1;
    }

    button.fall(&button_pressed);
    ThisThread::sleep_for(osWaitForever);

}


int wifi_sample_run(void)
{
    count_sent=0;
    cmd_to_sent=0;
    screen_off=1;
  
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
        } else {
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
                printf("copy this to your client\n"); 
        
                printf(">Start Server... \n");
                printf(">Wait for connection...  \n");
                State = WS_IDLE;
            } else {    
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
  * @brief  Send HTML page
  * @param  None
  * @retval None
  */
static void WebServerProcess(void)
{
    uint8_t LedState = 0;
    float temp;
    
    switch(State)
    {
        case WS_IDLE:
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
            break;
        
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
                        printf("> ERROR : Cannot send message\n");
                        errorOccurred = true;
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
  if(cmd_to_sent==0){
  sprintf((char *)message, "Hello from Disco L475VG IoT01 over WiFi! Count: %d", count_sent);
  }
  else{
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
