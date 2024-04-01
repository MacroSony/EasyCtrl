#include "mbed.h"
#include "wifi.h"
#include <string> 
/*------------------------------------------------------------------------------
Hyperterminal settings: 115200 bauds, 8-bit data, no parity

This example 
  - connects to a wifi network (SSID & PWD to set in mbed_app.json)
  - displays the IP address and creates a web page
  - then connect on its IP address on the same wifi network with another device
  - Now able to change the led status and read the temperature

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
static WIFI_Status_t SendWebPage(uint8_t ledIsOn, float temperature);
/* Private variables ---------------------------------------------------------*/
BufferedSerial pc(USBTX, USBRX, MBED_CONF_QUECTEL_BG96_BAUDRATE);
static   uint8_t http[1024];
static   uint8_t resp[1024];
uint16_t respLen;
uint8_t  IP_Addr[4]; 
uint8_t  MAC_Addr[6]; 
int32_t Socket = -1;
static   WebServerState_t  State = WS_ERROR;
char     ModuleName[32];
DigitalOut led(LED2);
AnalogIn adc_temp(ADC_TEMP);

int main()
{
    int ret = 0;
    led = 0;
    // pc.baud(115200);
    printf("\n");
    printf("************************************************************\n");
    printf("***   STM32 IoT Discovery kit for STM32L475 MCU          ***\n");
    printf("***         WIFI Web Server demonstration                ***\n\n");
    printf("*** Copy the IP address on another device connected      ***\n");
    printf("*** to the wifi network                                  ***\n");
    printf("*** Read the temperature and update the LED status       ***\n");
    printf("************************************************************\n");
    
    /* Working application */
    ret = wifi_sample_run();
    
    if (ret != 0) {
        return -1;
    }

    
    while(1) {
        WebServerProcess();
    }

}


int wifi_sample_run(void)
{
  
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
        
                printf(">Start HTTP Server... \n");
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
    
    WIFI_ReceiveData(Socket, resp, 1200, &respLen, WIFI_READ_TIMEOUT);
    
    if( respLen > 0)
    {
      if(strstr((char *)resp, "GET")) /* GET: put web page */
      {
        
        if(SendWebPage(LedState, temp) != WIFI_STATUS_OK)
        {
          printf("> ERROR : Cannot send web page\n");
          State = WS_ERROR;
        }
      }
      else if(strstr((char *)resp, "POST"))/* POST: received info */
      {
          if(strstr((char *)resp, "radio"))
          {          
            if(strstr((char *)resp, "radio=0"))
            {
              LedState = 0;
              led = 0;
              printf("turn off the light");
            }
            else if(strstr((char *)resp, "radio=1"))
            {
              LedState = 1;
              led = 1;
              printf("turn on the light");

            } 
            else
            {
                printf("> ERROR : invalic post\n");
            
            }
            
           
            if(SendWebPage(LedState, temp) != WIFI_STATUS_OK)
            {
              printf("> ERROR : Cannot send web page\n");
              State = WS_ERROR;
          }
        }
      }
    }
    if(WIFI_StopServer(Socket) == WIFI_STATUS_OK)
    {
      WIFI_StartServer(Socket, WIFI_TCP_PROTOCOL, "", PORT);
    }
    else
    {
      State = WS_ERROR;  
    }
    break;
  case WS_ERROR:   
  default:
    break;
  }
}


/**
  * @brief  Send HTML page
  * @param  None
  * @retval None
  */
static WIFI_Status_t SendWebPage(uint8_t ledIsOn, float temperature)
{
  uint8_t  temp[50];
  uint16_t SentDataLength;
  WIFI_Status_t ret;
  
  /* construct web page content */
  strcpy((char *)http, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n");
strcat((char *)http, "<html>\r\n<body>\r\n");
strcat((char *)http, "<title>STM32 Web Server</title>\r\n");
strcat((char *)http, "<h2>InventekSys : Web Server using Es-Wifi with STM32</h2>\r\n");
strcat((char *)http, "<br /><hr>\r\n");
strcat((char *)http, "<p><form method=\"POST\">");

if (ledIsOn) {
    strcat((char *)http, "<p><input type=\"radio\" name=\"radio\" value=\"0\" >LED off");
    strcat((char *)http, "<br><input type=\"radio\" name=\"radio\" value=\"1\" checked>LED on");
} else {
    strcat((char *)http, "<p><input type=\"radio\" name=\"radio\" value=\"0\" checked>LED off");
    strcat((char *)http, "<br><input type=\"radio\" name=\"radio\" value=\"1\" >LED on");
}

strcat((char *)http, "</p><p><input type=\"submit\"></form></span>");
strcat((char *)http, "</body>\r\n</html>\r\n");
  
  ret = WIFI_SendData(0, (uint8_t *)http, strlen((char *)http), &SentDataLength, WIFI_WRITE_TIMEOUT); 
  
  if((ret == WIFI_STATUS_OK) && (SentDataLength != strlen((char *)http)))
  {
    ret = WIFI_STATUS_ERROR;
  }
    
  return ret;
}