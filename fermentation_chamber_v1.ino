/*
 * TEMPERATURE CONTROLLED FERMENTATION CHAMBER
 */

#include <cmath>
#include <cstring>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>


/*===================== CONSTANT DEFINITIONS =====================*/
const int ONE_WIRE_BUS = 0; // GPIO 0 NodeMCU pin D3
const int TEMPERATURE_PRECISION = 11; // temperature sensor resolution in bits 0.125C 375ms

const float MIN_TOLERANCE = 0.5; // minimum allowed tolerance for 9bit precision
const int MAX_TOLERANCE = 10; // limited max tolerance

const int READ_FAIL_MAX = 6; // temperature sensor read fail counter
const int SAMPLE_SIZE = 10; // analog input sample size

const int MIN_TEMP = 38; // minimum set temperature in F
const int MAX_TEMP = 95; // maximum set temperature in F

const int IO_REFRESH_INTERVAL = 5 * 1000; // time interval between io refesh calls
const int COOLDOWN_INTERVAL = 2 * 60 * 1000; // hardware cooldown timer
const int CIRCULATION_TIME = 5 * 60 * 1000; // air circulation timer - runtime 5 minutes
const int CIRCULATION_INTERVAL = 55 * 60 * 1000; // air circulation cycle interval - 55 minutes

const int LCD_CYCLE_INTERVAL = 5 * 1000; // update lcd values
const int LCD_TIMEOUT_INTERVAL = 2 * 60 * 1000; // turn off lcd backlight after 5 min

const int REQUEST_INTERVAL = 5 * 1000; // check for any new requested values
const int SYSTEM_INTERVAL = 5 * 1000; // call system monitor output
const int LED_MONITOR_INTERVAL = 5 * 1000; // interval for led pulse during lcd timeout
const int LED_PULSE_INTERVAL = 100; // length of led pulse

// communications
const char* SERVER_NAME = "F-Control";
const char* SERVER_PASSWORD = "<server password here>";
const int PORT = 2701;
IPAddress ip(192, 168, 201, 11);
IPAddress gateway(192, 168, 201, 1);
IPAddress subnet(255, 255, 255, 0);

// relay pins
const int FAN_RELAY = 12; // NodeMCU D6 pin - output to fan trigger
const int COOL_RELAY = 14; // NodeMCU D5 pin - output to cooling trigger
const int HEAT_RELAY = 13; // NodeMCU D7 pin - output to heater trigger

// LCD values: NodeMCU D2 pin GPIO 4 - SDA , NodeMCU D1 pin GPIO 5 - SCL
const int i2cAddr = 0x27; // I2C memory address for NodeMCU
const int en = 2;
const int rw = 1;
const int rs = 0;
const int d4 = 4;
const int d5 = 5;
const int d6 = 6;
const int d7 = 7;
const int bl = 3;
const int LCD_CHAR_LIMIT = 21; // 20 character limit plus null terminator
const char LCD_BLANK_LINE[21] = "                    "; // a null terminated complete lcd line

// Local interface
const int ANALOG_INPUT = A0; // analog input for push buttons

// Monitor LED
const int MONITOR_LED = 15; // NodeMCU D8 pin - ground side 


/*===================== VARIABLE DEFINITIONS =====================*/
char units = 'F'; // temperature units
char currentPage = '_'; // specifies which page should be displayed on lcd
int offset = 2; // offset value to be used depending on which temperature units are used: F=2, C=1
int analogInputs[SAMPLE_SIZE]; // array of analog input samples
int analogIndex = 0; // current index to assign with analog input
int readFailCount = 0; // temperature sensor read fail count - only monitored sensor will affect this number
bool isDisplayOn = false; // true if lcd backlight should be on
bool selectDisplayOn = true; // true if lcd selected to be turned on by user, off if false
bool debug = false; // when true, activate serial and call monitor functions


/*===================== STRUCT DEFINITIONS =====================*/
struct ioComponent {
  bool isEnabled; // true if component is requested to be used for system operation
  char ioType[7]; // "INPUT" or "OUTPUT"
  char type[20]; // such as "TEMPERATURE" or "CIRCULATION"
  char location[50]; // describes component location
  int id; 
  float value; // measured or set value
  float target; // set desired value
  float tolerance; // tolerance +/- to target
  DeviceAddress deviceAddress; // single wire bus address for component
};

struct Request {
  float temperature; // new set temperature
  float tolerance; // new set tolerance
  bool isPending; // true if new requested value pending
  bool isThermowellEnabled; // true if requesting thermowell become enabled
  bool isCoolingEnabled; // true if requesting cooling system become enabled
  bool isHeatingEnabled; // true if requesting heating system become enabled
};

struct Timer {
  uint64_t coolDownStart; // hardware cooldown timer
  uint64_t ioRefreshStart; // input/output refresh call timer
  uint64_t lcdRefreshStart; // lcd refresh timer
  uint64_t lcdTimeoutStart; // lcd timeout (backlight off) timer
  uint64_t requestStart; // request handling timer
  uint64_t circulationStart; // fan recirculation cycle run timer
  uint64_t systemStatusStart; // serial out timer
  uint64_t ledMonitorStart; // led monitor timer
  uint64_t ledMonitorPulse; // led pulse timer
};

struct Credentials {
  char token[200];
  int isAuthed = 0;
};


/*===================== FUNCTION DEFINITIONS =====================*/
// input
void refreshInputs(); // update temperature sensor values
float getTemperatureBySensor(DeviceAddress device); // get temperature from sensor bus by address
bool isValidTemperature(float temperature); // return true if temperature value is a number and within specified plausible range

// local_display
void loadLCDPage(char page); // routing to lcd page loader based on the char page passed in
void loadLCDMainPage(); // load main lcd page 'm'
void loadLCDSetTemperaturePage(); // load set temperature page 'c'
void loadLCDSetTolerancePage(); // load set tolerance page 't'
void loadLCDToggleCoolingEnabled(); // load toggle cooling enabled page 'r'
void loadLCDToggleHeatingEnabled(); // load toggle heating enabled page 'h'
void loadLCDToggleThermowellEnabled(); // load toggle thermowell enabled page 's'
void turnOnScreen(); // turn lcd backlight on
void turnOffScreen(); // load main page and turn lcd backlight off
void printToLCD(const char* line1, const char* line2, const char* line3, const char* line4); // output messages to lcd

// local_interface
void listenForButton(); // if a button is pressed, add input to analogInputs
void submitAnalogInput(); // pass normalized analog input to hanlder and reset analogInputs array and analogIndex
int normalizeAnalogInputs(); // normalize analog input to remove erroneous input values
void handleAnalogInput(int input); // local interface input handler

// monitor
void printSensorSetup(); // serial out sensor setup results
void displaySystemStatus(); // serial out system statuses
void printInterfaceOutput(const char* line1, const char* line2, const char* line3, const char* line4);
void printInterfaceInput(int val); // serial out values from analog inputs
void printIORefresh(); // serial out current input output values

// output
void refreshOutputs(); // if at least one sensor is enabled, call thermostat cycle, otherwise deactivate all cycles
void runThermostatCycle(); // determine what cycle to operate
void runHeatCycle(); // heater output
void runCoolCycle(); // refrigerate output
void runCirculationCycle(); // fan recirculation output
void endCirculationCycle(); // end fan recirculation cycle
void deactivateAllCycles(); // turn all cycles off
void updateOutputValues(); // assign component on/off value with its target

// communication
void handleGetRequest();
void handlePostRequest();
void composeResponse(char* response, bool cpOK, bool tpOK, bool heatOK, bool coolOK, bool fanOK);
bool isIOPlausible(ioComponent& component);


/*==================== OBJECT/STRUCT INITIALIZATIONS ====================*/
OneWire oneWire(ONE_WIRE_BUS); // ds18b20 temperature sensor communication bus
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(i2cAddr, en, rw, rs, d4, d5, d6, d7, bl, POSITIVE); // init LCD
ESP8266WebServer server(PORT);

ioComponent chamberProbe = {
  true,
  "INPUT",
  "TEMPERATURE",
  "FERMENTATION CHAMBER",
  1,
  -99,
  68,
  1.5,
  {0x28, 0xFF, 0x78, 0x2F, 0x21, 0x17, 0x04, 0x2D}
};
ioComponent thermowellProbe = {
  false,
  "INPUT",
  "TEMPERATURE",
  "THERMOWELL",
  2,
  -99,
  68,
  1.5,
  {0x28, 0xFF, 0x4F, 0xFA, 0x80, 0x14, 0x02, 0xAD}
};
ioComponent heating = {
  true,
  "OUTPUT",
  "HEATING",
  "FERMENTATION CHAMBER",
  3,
  0,
  0,
  0,
  {}
};
ioComponent cooling = {
  true,
  "OUTPUT",
  "COOLING",
  "FERMENTATION CHAMBER",
  4,
  0,
  0,
  0,
  {}
};
ioComponent fan = {
  true,
  "OUTPUT",
  "CIRCULATION",
  "FERMENTATION CHAMBER",
  5,
  0,
  0,
  0,
  {}
};

Request request = {
  0,
  0,
  false,
  false,
  true,
  true
};

Timer timer;
Credentials credentials;

ioComponent* monitored; // pointer to sensor that will be used for system operation

void setup() {
  if (debug) {
    Serial.begin(9600);
    Serial.println("Fermentation chamber temperature control set up started");
  }

  pinMode(FAN_RELAY, OUTPUT);
  pinMode(COOL_RELAY, OUTPUT);
  pinMode(HEAT_RELAY, OUTPUT);
  pinMode(MONITOR_LED, OUTPUT);

  digitalWrite(FAN_RELAY, HIGH);
  digitalWrite(COOL_RELAY, HIGH);
  digitalWrite(HEAT_RELAY, HIGH);
  digitalWrite(MONITOR_LED, HIGH);

  lcd.begin(20, 4);
  turnOnScreen();
  lcd.setCursor(6, 1);
  lcd.print("BREW I/O");
  lcd.setCursor(4, 2);
  lcd.print("version  1.0");
  delay(10);

  sensors.begin();
  sensors.setResolution(chamberProbe.deviceAddress, TEMPERATURE_PRECISION);
  sensors.setResolution(thermowellProbe.deviceAddress, TEMPERATURE_PRECISION);

  if (debug) {
    printSensorSetup();
  } else {
    sensors.getDeviceCount();
    sensors.isParasitePowerMode();
    sensors.getAddress(chamberProbe.deviceAddress, 0);
    sensors.getAddress(thermowellProbe.deviceAddress, 1);
    sensors.getResolution(chamberProbe.deviceAddress);
    sensors.getResolution(thermowellProbe.deviceAddress);
  }

  monitored = &chamberProbe; // default to chamberProbe on setup
  request.temperature = chamberProbe.target;
  request.tolerance = chamberProbe.tolerance;    

  WiFi.softAP(SERVER_NAME, SERVER_PASSWORD);
  server.on("/", HTTP_GET, handleGetRequest);
  server.on("/", HTTP_POST, handlePostRequest);
  server.begin();

  uint64_t start = millis();
  timer.ioRefreshStart = start;
  timer.lcdRefreshStart = start;
  timer.lcdTimeoutStart = start;
  timer.requestStart = start;
  timer.circulationStart = start;
  timer.systemStatusStart = start;
  timer.coolDownStart = start;
  timer.ledMonitorStart = start;

  delay(500);
}

void loop() {
  server.handleClient(); // server handler
  
  listenForButton(); // analog input listener

  if (millis() - timer.ioRefreshStart > IO_REFRESH_INTERVAL) {
    sensors.requestTemperatures();
    refreshInputs();
    refreshOutputs();
    timer.ioRefreshStart = millis();
  }

  if (millis() - timer.circulationStart > CIRCULATION_INTERVAL) {
    // only start air circulation if another thermostat cycle is not already running
    if (!cooling.target && !heating.target) {
      fan.target = 1;
      if (!fan.value) runCirculationCycle();
      if ((millis() - timer.circulationStart) > (CIRCULATION_INTERVAL + CIRCULATION_TIME)) {
        fan.target = 0;
        endCirculationCycle();
        timer.circulationStart = millis();
      }
    } 
  }

  // refresh lcd
  if (millis() - timer.lcdRefreshStart > LCD_CYCLE_INTERVAL) {
    if (selectDisplayOn && millis() - timer.lcdTimeoutStart > LCD_TIMEOUT_INTERVAL) {
      selectDisplayOn = false;
      turnOffScreen();
    } else if (selectDisplayOn) {
      loadLCDPage(currentPage);
    } else {
      timer.lcdRefreshStart = millis();
    }
  }

  // process user requests
  if (millis() - timer.requestStart > REQUEST_INTERVAL) {
    if (request.isPending) {
      
      if (request.isThermowellEnabled) {
        float tempT = getTemperatureBySensor(thermowellProbe.deviceAddress);
        if (isValidTemperature(tempT)) {
          thermowellProbe.value = tempT;
          monitored = &thermowellProbe;
          thermowellProbe.isEnabled = true;
        }
      } else {
        monitored = &chamberProbe;
        thermowellProbe.isEnabled = false;
      }
      
      if (request.isCoolingEnabled) {
        cooling.isEnabled = true;
      } else {
        cooling.isEnabled = false;
        cooling.target = 0;
      }

      if (request.isHeatingEnabled) {
        heating.isEnabled = true;
      } else {
        heating.isEnabled = false;
        heating.target = 0;
      }
      
      monitored->target = request.temperature;
      monitored->tolerance = request.tolerance;
      request.isPending = false;
    }
    timer.requestStart = millis();
  }

  if (!isDisplayOn) {
    if (millis() - timer.ledMonitorStart > LED_MONITOR_INTERVAL) {
      digitalWrite(MONITOR_LED, LOW);
      timer.ledMonitorStart = millis();
      timer.ledMonitorPulse = millis();
    }
    if (millis() - timer.ledMonitorPulse > LED_PULSE_INTERVAL) {
      digitalWrite(MONITOR_LED, HIGH);
    }
  }

  if (debug && millis() - timer.systemStatusStart > SYSTEM_INTERVAL) {
    displaySystemStatus();
    timer.systemStatusStart = millis();
  } else if (!debug) {
    timer.systemStatusStart = millis();
  }

}

