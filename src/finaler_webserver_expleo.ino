#include <WiFi.h> //standard Wi-Fi lib
#include "esp_wpa2.h"
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <math.h>
#include "Ticker.h"

#define SERIAL_SPEED 115200
#define CAN_PERIOD 20

Ticker ticker_CAN_Message;

#define PIN_T1 26
#define PIN_T2 25
#define PIN_T3 35
#define PIN_T4 34

int BitsT1, BitsT2, BitsT3, BitsT4 = 0;
float VoltsT1, VoltsT2, VoltsT3, VoltsT4 = 0;
float TempT1, TempT2, TempT3, TempT4 = 0;
unsigned long SensorUpdate = 0;
unsigned long SensorUpdate_CAN = 0;
bool DEBUG = true;

//Uncomment for expleo
//#define EAP_IDENTITY "Syildirim" // most of the time same as Username
//#define EAP_USERNAME "Syildirim" // Expleo username
//#define EAP_PASSWORD "Nemyes61!" // Expleo password
//const char *ssid = "ExpleoMobile";    // ssid of WiFi source
int counter = 0;

const char *ssid = "FRITZ!Box 7560 OF";
const char *password = "Trabzon61";


//compressed html webpage
const char webpage[] PROGMEM = R"rawliteral(<!DOCTYPE html><html class='js-focus-visible' lang='en'><title>IceBox Web CLient</title><style> table{position:relative;width:100%;border-spacing:0}tr{border:1px solid white;font-family:'Verdana','Arial',sans-serif;font-size:20px}th{height:20px;padding:3px 15px;background-color:#343a40;color:#FFF !important}td{height:20px;padding:3px 15px}.tabledata{font-size:24px;position:relative;padding-left:5px;padding-top:5px;height:25px;border-radius:5px;color:#FFF;line-height:20px;transition:all 200ms ease-in-out;background-color:#0A0}.tabledata_hot{font-size:24px;position:relative;padding-left:5px;padding-top:5px;height:25px;border-radius:5px;color:#FFF;line-height:20px;transition:all 200ms ease-in-out;background-color:#A00}.tabledata_cold{font-size:24px;position:relative;padding-left:5px;padding-top:5px;height:25px;border-radius:5px;color:#FFF;line-height:20px;transition:all 200ms ease-in-out;background-color:#00A}.fanrpmslider{width:30%;height:15px;outline:none}.bodytext{font-family:'Verdana','Arial',sans-serif;font-size:24px;text-align:left;font-weight:lighter;border-radius:5px;display:inline}.navbar{width:100%;height:50px;margin:0;padding:10px 0;background-color:#FFF;color:#000;border-bottom:5px solid #293578}.fixed-top{position:fixed;top:0;right:0;left:0;z-index:1030}.navtitle{float:left;height:50px;font-family:'Verdana','Arial',sans-serif;font-size:50px;font-weight:bold;line-height:50px;padding-left:20px}.navheading{position:fixed;left:60%;height:50px;font-family:'Verdana','Arial',sans-serif;font-size:20px;font-weight:bold;line-height:20px;padding-right:20px}.navdata{justify-content:flex-end;position:fixed;left:70%;height:50px;font-family:'Verdana','Arial',sans-serif;font-size:20px;font-weight:bold;line-height:20px;padding-right:20px}.category{font-family:'Verdana','Arial',sans-serif;font-weight:bold;font-size:32px;line-height:50px;padding:20px 10px 0 10px;color:#000}.heading{font-family:'Verdana','Arial',sans-serif;font-weight:normal;font-size:28px;text-align:left;padding-left:20px}.btn_OFF{background-color:#fff;border:2px solid #e72020;color:black;padding:10px 20px;text-align:center;text-decoration:none;display:inline-block;font-size:16px;margin:4px 2px;cursor:pointer;transition-duration:500ms}.btn_OFF:hover{background-color:#e72020;color:white}.btn_ON{background-color:#e72020;border:2px solid #e72020;color:white;padding:10px 20px;text-align:center;text-decoration:none;display:inline-block;font-size:16px;margin:4px 2px;cursor:pointer;transition-duration:500ms}.btn_ON:hover{background-color:white;color:black}.check{cursor:pointer;position:relative;font-size:20px}.foot{font-family:'Verdana','Arial',sans-serif;font-size:20px;position:relative;height:30px;text-align:center;color:#AAA;line-height:20px}.container{max-width:1800px;margin:0 auto}table tr:first-child th:first-child{border-top-left-radius:5px}table tr:first-child th:last-child{border-top-right-radius:5px}table tr:last-child td:first-child{border-bottom-left-radius:5px}table tr:last-child td:last-child{border-bottom-right-radius:5px}</style><body style='background-color: #efefef'><header><div class='navbar fixed-top'><div class='container'><div class='navtitle'>IceBox Simulator</div><div class='navdata' id='date'>mm/dd/yyyy</div><div class='navheading'>DATE</div><br><div class='navdata' id='time'>00:00:00</div><div class='navheading'>TIME</div></div></div></header><main class='container' style='margin-top:70px'><div class='category'>Temperature Sensor Readings</div><div style='border-radius: 10px !important;'><table style='width: 50%;'><colgroup><col span='1' style='background-color: rgb(230, 230, 230); width: 20%; color: #000000;'><col span='1' style='background-color: rgb(200, 200, 200); width: 20%; color: #000000;'><col span='1' style='background-color: rgb(180, 180, 180); width: 20%; color: #000000;'><col span='1' style='background-color: rgb(130, 130, 130); width: 20%; color: #000000;'></colgroup><col span='2' style='background-color: #000000; color: #FFFFFF'><col span='2' style='background-color: #000000; color: #FFFFFF'><col span='2' style='background-color: #000000; color: #FFFFFF'><col span='2' style='background-color: #000000; color: #FFFFFF'><tr><th colspan='1'><div class='heading'>Sensor, Pin</div></th><th colspan='1'><div class='heading'>Bits</div></th><th colspan='1'><div class='heading'>Volts</div></th><th colspan='1'><div class='heading'>Temperature</div></th></tr><tr><td><div class='bodytext'>TempSens, 26</div></td><td><div class='tabledata' id='b1'>-</div></td><td><div class='tabledata' id='v1'>-</div></td><td><div class='tabledata' id='t1'>-</div></td></tr><tr><td><div class='bodytext'>TempSens, 25</div></td><td><div class='tabledata' id='b2'>-</div></td><td><div class='tabledata' id='v2'>-</div></td><td><div class='tabledata' id='t2'>-</div></td></tr><tr><td><div class='bodytext'>TempSens, 36</div></td><td><div class='tabledata' id='b3'>-</div></td><td><div class='tabledata' id='v3'>-</div></td><td><div class='tabledata' id='t3'>-</div></td></tr><tr><td><div class='bodytext'>TempSens, 35</div></td><td><div class='tabledata' id='b4'>-</div></td><td><div class='tabledata' id='v4'>-</div></td><td><div class='tabledata' id='t4'>-</div></td></tr></table></div><br><br><div class='category'>Sending Can Messages</div><br><div class='heading'><label class='check'><input type='checkbox' id='CHECK_KLEMME'></label>Status Klemme 15<div class='btn_OFF' id='BTN_KLEMME'>OFF</div></div><br><div class='heading'><label class='check'><input type='checkbox' id='CHECK_SPEED'></label>Geschwindigkeit<label for='SPEED_RANGE'></label><input class='fanrpmslider' id='SPEED_RANGE' max='250' min='0' step='1' type='range'/><output id='VALUE_SPEED'>-</output></div><br><div class='heading'><label class='check'><input type='checkbox' id='CHECK_RPM'></label>Motor RPM<label for='RPM_RANGE'></label><input class='fanrpmslider' id='RPM_RANGE' max='12000' min='0' step='100' type='range'/><output id='VALUE_RPM'>-</output></div></main><script>var Socket;document.getElementById('BTN_KLEMME').addEventListener('click', KLEMME_BUTTON_CHANGE);let msg = {Status: 'OFF', CheckKlemme: 'false', CheckSpeed: 'false', CheckRpm: 'false', Speed: 0, Rpm: 0};function init(){Socket=new WebSocket('ws://' + window.location.hostname + ':81/');Socket.onmessage=function (event) {processCommand(event);};}function KLEMME_BUTTON_CHANGE(){if(document.getElementById('BTN_KLEMME').innerHTML === 'OFF'){document.getElementById('BTN_KLEMME').className = 'btn_ON';document.getElementById('BTN_KLEMME').innerHTML = 'ON';}else{document.getElementById('BTN_KLEMME').className = 'btn_OFF';document.getElementById('BTN_KLEMME').innerHTML = 'OFF';}}function send_Klemme(){if(document.getElementById('CHECK_KLEMME').checked){msg['CheckKlemme'] = 'true';msg['Status'] = document.getElementById('BTN_KLEMME').innerHTML;}else{msg['CheckKlemme'] = 'false';}}let ticker=0;function processCommand(event){var obj=JSON.parse(event.data);var dt=new Date();document.getElementById('time').innerHTML=dt.toLocaleTimeString();document.getElementById('date').innerHTML=dt.toLocaleDateString();document.getElementById('b1').innerHTML=obj.BitsT1;document.getElementById('v1').innerHTML=Math.round(obj.VoltsT1 * 100) / 100;document.getElementById('t1').innerHTML=Math.round(obj.TempT1 - 5);document.getElementById('b2').innerHTML=obj.BitsT2;document.getElementById('v2').innerHTML=Math.round(obj.VoltsT2 * 100) / 100;document.getElementById('t2').innerHTML=Math.round(obj.TempT2 - 5);document.getElementById('b3').innerHTML=obj.BitsT3;document.getElementById('v3').innerHTML=Math.round(obj.VoltsT3 * 100) / 100;document.getElementById('t3').innerHTML=Math.round(obj.TempT3 - 5);document.getElementById('b4').innerHTML=obj.BitsT4;document.getElementById('v4').innerHTML=Math.round(obj.VoltsT4 * 100) / 100;document.getElementById('t4').innerHTML=Math.round(obj.TempT4 - 5);checkBackground();updateSpeed();updateRPM();sendSpeed();sendRpm();send_Klemme();Socket.send(JSON.stringify(msg));}function showMessage(){console.log('The Json object sent to the Board is: ' + JSON.stringify(msg));}setInterval(showMessage,5000);function sendSpeed(){if(document.getElementById('CHECK_SPEED').checked){msg['CheckSpeed'] = 'true';msg['Speed'] = parseInt(document.getElementById('VALUE_SPEED').innerHTML);}else{msg['CheckSpeed'] = 'false';}}function sendRpm(){if(document.getElementById('CHECK_RPM').checked){msg['CheckRpm'] = 'true';msg['Rpm'] = parseInt(document.getElementById('VALUE_RPM').innerHTML);}else{msg['CheckRpm'] = 'false';}}function checkBackground(){const temps = ['t1', 't2', 't3', 't4'];const bits = ['b1', 'b2', 'b3', 'b4'];const volts = ['v1', 'v2', 'v3', 'v4'];var barwidthTemp=0;var barwidthBits=0;var barwidthVolts=0;for(let i=0;i < temps.length;i++){barwidthTemp=document.getElementById(temps[i]).innerHTML / 40.95;barwidthBits=document.getElementById(bits[i]).innerHTML / 40.95;barwidthVolts=document.getElementById(volts[i]).innerHTML / 40.95;if(document.getElementById(temps[i]).innerHTML < 10){document.getElementById(temps[i]).className = 'tabledata_cold';document.getElementById(bits[i]).className = 'tabledata_cold';document.getElementById(volts[i]).className = 'tabledata_cold';}else if(document.getElementById(temps[i]).innerHTML > 60){document.getElementById(temps[i]).className = 'tabledata_hot';document.getElementById(bits[i]).className = 'tabledata_hot';document.getElementById(volts[i]).className = 'tabledata_hot';}else{document.getElementById(temps[i]).className = 'tabledata';document.getElementById(bits[i]).className = 'tabledata';document.getElementById(volts[i]).className = 'tabledata';}}}function updateSpeed(){const value_speed=document.querySelector('#VALUE_SPEED');const input_speed=document.querySelector('#SPEED_RANGE');value_speed.textContent=input_speed.value;input_speed.addEventListener('input_speed', (event) => {value_speed.textContent=event.target.value;});}function updateRPM(){const value_rpm=document.querySelector('#VALUE_RPM');const input_rpm=document.querySelector('#RPM_RANGE');value_rpm.textContent=input_rpm.value;input_rpm.addEventListener('input_speed', (event) => {value_rpm.textContent=event.target.value;})}window.onload=function (event) {init();}</script><footer class='foot' id='temp'>bei fragen seymen.yildirim@expleogroup.com</footer></body></html>)rawliteral";


String jsonString = "";                   // create json String
StaticJsonDocument<200> doc;              // create json document for sending
JsonObject object = doc.to<JsonObject>(); // convert doc to json object

// CAN Serial
#include <ESP32CAN.h>
#include <CAN_config.h>
#include "driver/can.h"

HardwareSerial SerialCAN(2);
CAN_device_t CAN_cfg;

void setup_CAN() {
    CAN_cfg.speed = CAN_SPEED_100KBPS;
    CAN_cfg.tx_pin_id = GPIO_NUM_2;   //2,4 tx to tx
    CAN_cfg.rx_pin_id = GPIO_NUM_15;  //5 rx to rx
    CAN_cfg.rx_queue = xQueueCreate(10, sizeof(CAN_frame_t));
    ESP32Can.CANInit();
}

//WiFi setup function
void setup_WiFi() {
    if (DEBUG) {
        Serial.begin(SERIAL_SPEED);
        Serial.println();
        Serial.print("Connecting to network: ");
        Serial.println(ssid);
    }
    WiFi.disconnect(true); // disconnect form wifi to set new wifi connection
    WiFi.mode(WIFI_STA);   // init wifi mode

    // Example1 (most common): a cert-file-free eduroam with PEAP (or TTLS)
    //WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD); //UNCOMMENT FOR EXPLEO

    WiFi.begin(ssid,password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if (DEBUG) {
            Serial.print(".");
        }
        counter++;
        if (counter >= 60) { // after 30 seconds timeout - reset board
            ESP.restart();
        }
    }
    if (DEBUG) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address set: ");
        Serial.println(WiFi.localIP()); // print LAN IP
    }
}


WebServer server(80);                              // creating webserver, 80 standard port
WebSocketsServer WebSocket = WebSocketsServer(81); // creating websocket server, 81 standard port


void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t);

//Webserver setup function
void setup_Webserver() {
    server.on("/", []() { server.send(200, "text/html", webpage); });
    server.begin();

    WebSocket.begin();
    WebSocket.onEvent(webSocketEvent);
}

//Temperature sensor setup function
void setup_TempSens() {
    pinMode(PIN_T1, INPUT);
    pinMode(PIN_T2, INPUT);
    pinMode(PIN_T3, INPUT);
    pinMode(PIN_T4, INPUT);
}
void setup() {
    // put your setup code here, to run once:
    setup_TempSens();
    setup_WiFi();
    setup_Webserver();
    setup_CAN();
}
//converting bits to temperature

// Nennwiderstand des Thermistors (10k Ohm)
const float nominalResistance = 10000.0;

// Beta-Wert des Thermistors (3976)
const float betaValue = 3976.0;

// Nennwiderstand des Thermistors bei 25 Grad Celsius
const float nominalTemperature = 25.0 + 273.15; // in Kelvin

float temperature(int Pin) {
    int rawADC = analogRead(Pin);
    float voltage = rawADC * (3.3 / 4095.0); // Umwandlung von ADC in Spannung (12-Bit-Auflösung)
    float resistance = (voltage * nominalResistance) / (3.3 - voltage);

    float steinhart;
    steinhart = 1.0 / (1.0 / nominalTemperature + (1.0 / betaValue) * log(resistance / nominalResistance));
    steinhart -= 273.15; // Convert to Celsius

    return steinhart;

}

void loop_TempSens() {
    //BitsT1 = analogRead(PIN_T1);
    //BitsT2 = analogRead(PIN_T2);
    BitsT3 = analogRead(PIN_T3);
    BitsT4 = analogRead(PIN_T4);

    //VoltsT1 = BitsT1*3.3 /4096;
    //VoltsT2 = BitsT2*3.3 /4096;
    VoltsT3 = BitsT3 * 3.3 / 4096;
    VoltsT4 = BitsT4 * 3.3 / 4096;

    //TempT1 = temperature(PIN_T1);
    //TempT2 = temperature(PIN_T2);
    TempT3 = temperature(PIN_T3);
    TempT4 = temperature(PIN_T4);
}


void loop_Write_Json() {

    String jsonString = "";
    StaticJsonDocument<200> doc;
    JsonObject object = doc.to<JsonObject>();
    object["BitsT1"] = 0;
    object["VoltsT1"] = 0;
    object["TempT1"] = 0;

    object["BitsT2"] = 0;
    object["VoltsT2"] = 0;
    object["TempT2"] = 0;

    object["BitsT3"] = BitsT3;
    object["VoltsT3"] = VoltsT3;
    object["TempT3"] = TempT3;

    object["BitsT4"] = BitsT4;
    object["VoltsT4"] = VoltsT4;
    object["TempT4"] = TempT4;
    // convert json object to string
    serializeJson(doc, jsonString);
    if (DEBUG) {
        //Serial.println("Json string sent to the Webserver: \n" + jsonString);  //KOMMENTAR LÖSCHEN UM ZU SEHEN WAS AN WEBSERVER GESCHICKT WIRD
    }
    WebSocket.broadcastTXT(jsonString); // send json string to clients
}

void loop() {
    // put your main code here, to run repeatedly:
    server.handleClient();
    WebSocket.loop();

    unsigned long now = millis();
    if ((unsigned long) (now - SensorUpdate) > 200) // 200 is interval for input update server
    {
        // update values of temp sens and add to json object
        loop_TempSens();
        loop_Write_Json();

        SensorUpdate = now;
    }
}
//send CLAMP_STAT_OFF

//byte array for ALIV_CLMP_STAT_MOTBK_2010(23)
byte ALIV_CLMP_STAT[] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0};
//counter for ALIV_CLMP_STAT
int z = 0;
void sendKlemmeOff() {
    if (DEBUG) {
        Serial.println("Sending klemme off");
    }
    CAN_frame_t rx_frame;
    //receive next CAN frame from queue
    if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {

        //do stuff!
        if (rx_frame.FIR.B.FF == CAN_frame_std)
            printf("New standard frame");
        else
            printf("New extended frame");

        if (rx_frame.FIR.B.RTR == CAN_RTR)
            printf(" RTR from 0x%08x, DLC %d\r\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
        else {
            printf(" from 0x%08x, DLC %d\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
            for (int i = 0; i < 8; i++) {
                printf("%c\t", (char) rx_frame.data.u8[i]);
            }
            printf("\n");
        }
    } else {
        rx_frame.FIR.B.FF = CAN_frame_std;
        rx_frame.MsgID = 1;
        rx_frame.FIR.B.DLC = 8;
        rx_frame.data.u8[0] = 0x40;   //byte 0  = 01000000(bin), key valid, Clamp 15 Off
        rx_frame.data.u8[1] = 0x00;   //byte 1  = 00(dec)       , empty
        rx_frame.data.u8[2] = 0x00;   //byte 2  = 00(dec)       , empty
        rx_frame.data.u8[3] = 0x00;   //byte 3  = 00(dec)       , empty
        rx_frame.data.u8[4] = 0x00;   //byte 4  = 00(dec)       , empty
        rx_frame.data.u8[5] = 0x00;   //byte 5  = 00(dec)       , empty
        rx_frame.data.u8[6] = ALIV_CLMP_STAT[z];   //byte 6  = 0001 0000(bin), ALIV_CLMP_STAT_MOTBK_2010(23)
        rx_frame.data.u8[7] = 0x1E;   //byte 7  = 30(dec)       , CRC_CLMP_STAT_MOTBK_2010(24)


        ESP32Can.CANWriteFrame(&rx_frame);
    }
    ticker_CAN_Message.detach();
    delay(2);
    ticker_CAN_Message.attach_ms(CAN_PERIOD, sendKlemmeOff);
    if (z < sizeof(ALIV_CLMP_STAT)) {
        z++;
    } else { z = 0; }

}

//send CLAMP_STAT_ON
void sendKlemmeOn() {
    if (DEBUG) {
        Serial.println("Sending klemme on");
    }
    CAN_frame_t rx_frame;
    //receive next CAN frame from queue
    if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {

        //do stuff!
        if (rx_frame.FIR.B.FF == CAN_frame_std)
            printf("New standard frame");
        else
            printf("New extended frame");

        if (rx_frame.FIR.B.RTR == CAN_RTR)
            printf(" RTR from 0x%08x, DLC %d\r\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
        else {
            printf(" from 0x%08x, DLC %d\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
            for (int i = 0; i < 8; i++) {
                printf("%c\t", (char) rx_frame.data.u8[i]);
            }
            printf("\n");
        }
    } else {
        rx_frame.FIR.B.FF = CAN_frame_std;
        rx_frame.MsgID = 1;
        rx_frame.FIR.B.DLC = 8;
        rx_frame.data.u8[0] = 0x44;   //byte 0  = 0100 0100(bin), key valid, Clamp 15 On
        rx_frame.data.u8[1] = 0x00;   //byte 1  = 00(dec)       , empty
        rx_frame.data.u8[2] = 0x00;   //byte 2  = 00(dec)       , empty
        rx_frame.data.u8[3] = 0x00;   //byte 3  = 00(dec)       , empty
        rx_frame.data.u8[4] = 0x00;   //byte 4  = 00(dec)       , empty
        rx_frame.data.u8[5] = 0x00;   //byte 5  = 00(dec)       , empty
        rx_frame.data.u8[6] = ALIV_CLMP_STAT[z];   //byte 6  = 0001 0000(bin), ALIV_CLMP_STAT_MOTBK_2010(23)
        rx_frame.data.u8[7] = 0x1E;   //byte 7  = 30(dec)       , CRC_CLMP_STAT_MOTBK_2010(24)


        ESP32Can.CANWriteFrame(&rx_frame);
    }
    ticker_CAN_Message.detach();
    delay(2);
    ticker_CAN_Message.attach_ms(CAN_PERIOD, sendKlemmeOn);
    if (z < sizeof(ALIV_CLMP_STAT)) {
        z++;
    } else { z = 0; }

}

byte ALIV_ENGDAT[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E};

//send RPM
byte eight_lsb_rpm = 0x00;
byte four_msb_rpm = 0x00;
void sendRpm() {
    if (DEBUG) {
        Serial.println("Sending rpm");
    }
    CAN_frame_t rx_frame;
    //receive next CAN frame from queue
    if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {

        //do stuff!
        if (rx_frame.FIR.B.FF == CAN_frame_std)
            printf("New standard frame");
        else
            printf("New extended frame");

        if (rx_frame.FIR.B.RTR == CAN_RTR)
            printf(" RTR from 0x%08x, DLC %d\r\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
        else {
            printf(" from 0x%08x, DLC %d\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
            for (int i = 0; i < 8; i++) {
                printf("%c\t", (char) rx_frame.data.u8[i]);
            }
            printf("\n");
        }
    } else {
        rx_frame.FIR.B.FF = CAN_frame_std;
        rx_frame.MsgID = 1;
        rx_frame.FIR.B.DLC = 8;
        rx_frame.data.u8[0] = 0x46;   //byte 0  CRC_ENGDAT_2_MOTBK_2010 (24)
        rx_frame.data.u8[1] = ALIV_ENGDAT[z];   //byte 1  ST_SDST_MOTBK_2010 (22) ST_KL_50_MOTBK_2010 (23) ALIV_ENGDAT_2_MOTBK_2010 (23)
        rx_frame.data.u8[2] = eight_lsb_rpm;   //byte 2  RPM_ENG_MOTBK_2010 (26)
        rx_frame.data.u8[3] = four_msb_rpm;   //byte 3  BNK_ANG_MOTBK_2010 (24) RPM_ENG_MOTBK_2010 (26)
        rx_frame.data.u8[4] = 0x80;   //byte 4  BNK_ANG_MOTBK_2010 (24)
        rx_frame.data.u8[5] = 0x00;   //byte 5  ANG_THVA_MOTBK_2010 (24)
        rx_frame.data.u8[6] = 0x6A;   //byte 6  ST_SW_BOOT_MOTBK_2010 (22) ST_CR_MOTBK_2010 (24) ST_SEN_OVTU_MOTBK_2010 (23) ST_EMOFF_MOTBK_2010 (24)
        rx_frame.data.u8[7] = 0x00;   //byte 7  DVCH_MOTBK_2010 (23)


        ESP32Can.CANWriteFrame(&rx_frame);
    }
    ticker_CAN_Message.detach();
    delay(2);
    ticker_CAN_Message.attach_ms(CAN_PERIOD, sendRpm);
    if (z < sizeof(ALIV_CLMP_STAT)) {
        z++;
    } else { z = 0; }
}
byte four_lsb_RR = 0x00;
byte eight_msb_RR = 0x00;
byte eight_lsb_FT = 0x00;
byte four_msb_FT = 0x00;
//send Speed
void sendSpeed() {
    if (DEBUG) {
        Serial.println("Sending Speed");
    }
    CAN_frame_t rx_frame;
    //receive next CAN frame from queue
    if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {

        //do stuff!
        if (rx_frame.FIR.B.FF == CAN_frame_std)
            printf("New standard frame");
        else
            printf("New extended frame");

        if (rx_frame.FIR.B.RTR == CAN_RTR)
            printf(" RTR from 0x%08x, DLC %d\r\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
        else {
            printf(" from 0x%08x, DLC %d\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
            for (int i = 0; i < 8; i++) {
                printf("%c\t", (char) rx_frame.data.u8[i]);
            }
            printf("\n");
        }
    } else {
        rx_frame.FIR.B.FF = CAN_frame_std;
        rx_frame.MsgID = 1;
        rx_frame.FIR.B.DLC = 8;
        rx_frame.data.u8[0] = 0x14;   //byte 0 CRC_V_MOTBK_2010
        rx_frame.data.u8[1] = four_lsb_RR | ALIV_ENGDAT[z];   //byte 1 V_WHL_RR_MOTBK_2010 (27) ALIV_V_MOTBK_2010 (23)
        rx_frame.data.u8[2] = eight_msb_RR;   //byte 2 V_WHL_RR_MOTBK_2010 (27)
        rx_frame.data.u8[3] = eight_lsb_FT;   //byte 3 V_WHL_FT_MOTBK_2010 (27)
        rx_frame.data.u8[4] = 0x50 | four_msb_FT;   //byte 4 ST_BRK_FT_MOTBK_2010 (24) ST_BRK_RR_MOTBK_2010 (24) V_WHL_FT_MOTBK_2010 (27)
        rx_frame.data.u8[5] = 0x15;   //byte 5 ST_ABS_MOTBK_2010 (24) ST_SEN_V_FT_MOTBK_2010 (23) ST_SEN_V_RR_MOTBK_2010 (23)
        rx_frame.data.u8[6] = 0xA9;   //byte 6 ST_STASS_MOTBK_2010 (24) MOD_VEH_ABS_CHOV_MOTBK_2010 (26) MOD_VEH_ABS_ACV_MOTBK_2010 (28)
        rx_frame.data.u8[7] = 0x00;   //byte 7 CTR_WMOM_DBC_MOTBK_2010 (22) CTR_DBC_GRAD_MOTBK_2010 (22)


        ESP32Can.CANWriteFrame(&rx_frame);
    }
    ticker_CAN_Message.detach();
    delay(2);
    ticker_CAN_Message.attach_ms(CAN_PERIOD, sendSpeed);
}

//basic websocket function to process input from webclient
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t) {

    switch (type) {
        //code to run if webclient disconnects
        case WStype_DISCONNECTED:
            if (DEBUG) {
                Serial.println("Client disconnect");
            }
            break;
            //code to run if text is received from client
        case WStype_TEXT:
            // try to decipher the JSON string received
            StaticJsonDocument<200> doc;                    // create a JSON container
            DeserializationError error = deserializeJson(doc, payload);
            if (error) {
                if (DEBUG) {
                    Serial.print(("deserializeJson() failed: "));
                    Serial.println(error.f_str());
                }
                return;
            } else {//code to run if json Object is received and has no error
                String received = "";
                String status_klemme = doc["Status"];
                String CheckKlemme = doc["CheckKlemme"];
                String CheckSpeed = doc["CheckSpeed"];
                String CheckRpm = doc["CheckRpm"];
                int Speed_RR = doc["Speed"];
                int Speed_FT = doc["Speed"];
                int Rpm = doc["Rpm"];

                if (DEBUG) {
                    Serial.println("Data from Icebox Client");
                    Serial.println(status_klemme);
                    Serial.println(CheckKlemme);
                    Serial.println(CheckSpeed);
                    Serial.println(CheckRpm);
                    Serial.println(Speed_RR);
                    Serial.println(Rpm);
                    //byte manipulation RPM
                    Rpm /= 5;
                    eight_lsb_rpm = (byte) Rpm;
                    Rpm >>= 8;
                    four_msb_rpm = (byte) Rpm;
                    //byte manipulation speed RR
                    Speed_RR *= 8;
                    Speed_RR <<= 4;
                    four_lsb_RR = (byte)Speed_RR;
                    Speed_RR >>= 8;
                    eight_msb_RR = (byte)Speed_RR;
                    //byte manipulation speed FT
                    Speed_FT *= 8;
                    eight_lsb_FT = (byte) Speed_FT;
                    Speed_FT >>= 8;
                    four_msb_FT = (byte) Speed_FT;

                }
                unsigned long now = millis();
//                if ((unsigned long) (now - SensorUpdate_CAN) > 120) // 200 is interval for input update server
//                {
//                    if (CheckKlemme == "true") {
//                        if (status_klemme == "ON") {
//                            sendKlemmeOn();
//                        } else {
//                            sendKlemmeOff();
//                        }
//                    }
//                    if (CheckSpeed == "true") {
//                        sendSpeed();
//                    }
//                    if (CheckRpm == "true") {
//                        sendRpm();
//                    }
//                    SensorUpdate_CAN = now;
//                }

            }
            break;
    }
}