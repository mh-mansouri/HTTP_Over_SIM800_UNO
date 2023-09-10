//By Mahdi Mansouri
//September 10, 2023
//Sweden, Gothenburg

#include <avr/wdt.h>
#include <SoftwareSerial.h>

#define SERBDRT 9600
#define SERTOUT 5000

#define DLYHLF 500
#define DLYFUL 1000
#define DLY2S 2000
#define DLY3S 3000
#define DLY5S 5000

#define SER_RX 0
#define SER_TX 1

#define SIM800_RX 4
#define SIM800_TX 5
#define SIM800_RS 7

#define waitRedo 1
#define waitReset 5

#define SIGLOW 5
#define SIGHI 35
char chrRead;

const String APN = "Comviq";                                   //you must update it with your SIMcard operator's name
const String URL00 = "http://miliohm.com/miliohmSIM800L.php";  //It is a link that is easily accessible

const bool SSL_EN = false;

SoftwareSerial mySerial(SIM800_RX, SIM800_TX);  // RX, TX

String strIn = "";
String strOut = "";
String Response = "";

int retVal = -1;

enum SIM800STATE { SIMRESERT,
                   HANDSHAKE,
                   SIGNALSTRENGTH,
                   SIMPLUGGED,
                   SIMREG,
                   HTTP_SETPARAM,
                   HTTP_INIT,
                   HTTP_TRANSITION,
                   HTTP_TERM } SIMSTATE = HANDSHAKE;

enum REGSTATUS { NOT_REGISTERED,
                 HOME,
                 NOT_REGISTERED_NEW,
                 DENIED,
                 UNKNOW,
                 ROAMING,
                 SMS_ONLY,
                 SMS_ONLY_ROAMING,
                 EMERGENCY,
                 CSFB_HOME,
                 CSFB_ROAMING
} SIMREGSTATUS = HOME;

enum TECHSTATUS { GSM,
                  GSM_Compact,
                  UTRAN,
                  GSM_w_EGPRS,
                  UTRAN_w_HSDPA,
                  UTRAN_w_HSUPA,
                  UTRAN_w_HSDPA_and_HSUPA,
                  E_UTRAN } SIMTECHSTATE = GSM;

//==================================================SIM_RESET=========================================
int Command_SIMReset() {
  int timeKeeper = 0;
  wdt_reset();

  Serial.print(F("\nSIM800 reset started ..."));
  digitalWrite(SIM800_RS, HIGH);
  delay(DLYFUL);
  wdt_reset();

  digitalWrite(SIM800_RS, LOW);
  delay(DLYFUL);
  wdt_reset();

  Serial.print(F("\nSIM800 reset finished ..."));
  wdt_reset();
  delay(DLYFUL);

  return (HANDSHAKE);
}
//====================================================================================================

//==================================================SIM_HANDSHAKE=====================================
int Command_Handshake() {
  int retIndex = -1;
  int timeKeeper = 0;

  wdt_reset();

  //Serial.println(F("\nAT"));
  mySerial.println("AT");  //Once the handshake test is successful, it will back to OK
  delay(DLYHLF);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  retIndex = Response.indexOf("OK");

  if (retIndex > -1) {
    Serial.println(F("Handshaking successfull!"));
    Serial.println(F("Continues to the next step!"));
    wdt_reset();
    delay(DLYFUL);
    return (SIGNALSTRENGTH);
  } else {
    Serial.println(F("Handshaking failed!"));
    wdt_reset();
    return (retIndex);
  }
}
//====================================================================================================

//==================================================SIGNAL_STRENGTH===================================
int Command_SIGSTRNGTH() {
  int retIndex00 = -1;
  int retIndex01 = -1;
  int retIndexOK = -1;
  int timeKeeper = 0;

  int sigQuality = 0;

  wdt_reset();

  //Serial.println(F("\nAT+CSQ"));
  mySerial.println("AT+CSQ");  //Signal quality test, value range is 0-31 , 31 is the best
  delay(DLYHLF);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  retIndexOK = Response.indexOf("OK");
  retIndex00 = Response.indexOf(":");
  retIndex01 = Response.indexOf(",");
  sigQuality = Response.substring(retIndex00 + 1, retIndex01).toInt();

  if (retIndexOK < 0 || sigQuality > SIGHI || sigQuality < SIGLOW) {
    Serial.println(F("No signal is available!"));
    Serial.println(F("Please check the sim card or the antenna!"));
    wdt_reset();
    delay(DLYFUL);
    return (-1);
  } else {
    Serial.print(F("Signal quality is :"));
    Serial.print(sigQuality);
    Serial.print("\n");
    Serial.println(F("It continues to the next step!"));
    wdt_reset();
    delay(DLYFUL);
    return (SIMPLUGGED);
  }
}
//====================================================================================================

//==================================================SIM_INFO==========================================
int Command_SIMPLUGGED() {
  int retIndex = -1;
  int timeKeeper = 0;

  wdt_reset();

  //Serial.println(F("\nAT+CCID"));
  mySerial.println("AT+CCID");  //Read SIM information to confirm whether the SIM is plugged
  delay(DLYHLF);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  retIndex = Response.indexOf("OK");

  if (retIndex > -1) {
    Serial.println(F("Checking the SIMCARD infromation was successfull!"));
    Serial.println(F("It continues to the next step!"));
    wdt_reset();
    delay(DLYFUL);
    return (SIMREG);
  } else {
    Serial.println(F("Getting the SIMCARD information failed!"));
    Serial.println(F("Retry the SIMCARD reset after some seconds"));
    wdt_reset();
    delay(DLYFUL);

    wdt_reset();
    return (-1);
  }
}
//====================================================================================================

//==================================================SIM_REG===========================================
int Command_SIMREG() {

  int retIndex00 = -1;
  int retIndex01 = -1;
  int timeKeeper = 0;

  int regStatus = -1;
  int techStatus = -1;

  String strSub00 = "";
  String strSub01 = "";

  wdt_reset();

  //Serial.println(F("\nAT+CREG?"));
  mySerial.println("AT+CREG?");  //Check whether it has registered in the network
  delay(DLYHLF);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  retIndex00 = Response.indexOf(",");
  retIndex01 = Response.indexOf("OK");

  if (retIndex01 > -1) {
    strSub00 = Response.substring(retIndex00 - 1, retIndex00);
    strSub01 = Response.substring(retIndex00 + 1, retIndex00 + 2);

    techStatus = strSub00.toInt();
    regStatus = strSub01.toInt();

    if (regStatus != HOME && techStatus < UTRAN) {
      Serial.println(F("Checking the SIMCARD registeration information failed!"));
      wdt_reset();
      delay(DLYFUL);
      return (-1);
    } else {
      Serial.println(F("Checking the SIMCARD registeration infromation was successfull!"));
      Serial.println(F("It continues to the next step!"));
      wdt_reset();
      delay(DLYFUL);
      return (SIMREG);
    }

  } else {
    Serial.println(F("Checking the SIMCARD registeration information failed!"));
    wdt_reset();
    delay(DLYFUL);
    return (-1);
  }
}
//====================================================================================================

//==================================================SET_HTTP_PARAM====================================
int Command_SIMHTPPSETPARAM() {
  int retIndex = -1;
  int timeKeeper = 0;

  wdt_reset();

  //Serial.println(F("\nAT+CFUN?"));
  mySerial.println("AT+CFUN?");  //Check the SIMCARD functionality
  delay(DLYHLF);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  retIndex = Response.indexOf("OK");

  if (retIndex < 0) {
    Serial.println(F("CID setting failed!"));
    wdt_reset();
    delay(DLYFUL);
    return (-1);
  }

  strOut = "AT+SAPBR=3,1,\"APN\",\"" + APN + "\"";
  //Serial.println(strOut);
  mySerial.println(strOut);  //APN setting
  delay(DLYHLF);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  retIndex = Response.indexOf("OK");

  if (retIndex < 0) {
    Serial.println(F("APN setting failed!"));
    wdt_reset();
    delay(DLYFUL);
    return (-1);
  }

  //Do not care if it fails! :)
  //Serial.println(F("\nAT+SAPBR=1,1"));
  mySerial.println("AT+SAPBR=1,1");  //Enable the GPRS
  delay(DLYHLF);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);
  Serial.print(F("\nIt countinues even if the function failed! :)"));

  //Serial.println(F("\nAT+SAPBR=2,1"));
  mySerial.println("AT+SAPBR=2,1");  //Check if got IP Adress
  delay(DLYHLF);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  if (retIndex < 0) {
    Serial.println(F("IP check failed!"));
    wdt_reset();
    delay(DLYFUL);
    return (-1);
  }

  Serial.println(F("HTTP parameters set"));
  Serial.println(F("It continues to the next step!"));
  return (HTTP_INIT);
}
//====================================================================================================

//==================================================HTTP_INIT=========================================
int Command_SIMHTTPINIT(bool SSLENABLER = false) {
  int retIndex = -1;
  int timeKeeper = 0;

  wdt_reset();

  //Serial.println(F("\nAT+HTTPINIT"));
  mySerial.println("AT+HTTPINIT");  //Enable HTTP
  delay(DLYHLF);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  retIndex = Response.indexOf("OK");

  if (retIndex < 0) {
    Serial.println(F("HTTP enabling failed!"));
    return (retIndex);
  }

  if (SSLENABLER) {
    //Serial.println(F("\nAT+HTTPSSL=1"));
    mySerial.println("AT+HTTPSSL=1");  //Enable SSL
    delay(DLYHLF);

    Response = "";
    while (mySerial.available()) {
      chrRead = mySerial.read();
      Response.concat(chrRead);
      wdt_reset();
    }
    Serial.print("\n");
    Serial.println(Response);

    retIndex = Response.indexOf("OK");

    if (retIndex < 0) {
      Serial.println(F("SSL enabling failed!"));
      Serial.println(F("HTTP enabling continues"));
    }
  }

  strOut = "AT+HTTPPARA=\"CID\",1";
  //Serial.println(strOut);
  mySerial.println(strOut);  //Set CID
  delay(DLYHLF);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  retIndex = Response.indexOf("OK");

  if (retIndex < 0) {
    Serial.println(F("CID failed!"));
    return (retIndex);
  }
  return (HTTP_TRANSITION);
}
//====================================================================================================

//==================================================HTTP_TRANSIT======================================
int Command_SIMHTTPTRANS() {

  int retIndex = -1;
  int timeKeeper = 0;

  int commaIndex = -1;
  int webReadSize = -1;

  strOut = "AT+HTTPPARA=\"URL\",\"" + URL00 + "\"";
  //Serial.println(strOut);
  mySerial.println(strOut);  //Set URL
  delay(DLYHLF);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  retIndex = Response.indexOf("OK");
  if (retIndex < 0) {
    Serial.println(F("URL failed!"));
    return (retIndex);
  }

  //Serial.println(F("\nAT+HTTPACTION=0"));
  mySerial.println("AT+HTTPACTION=0");  //Start HTTP GET Session
  delay(DLY2S);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  retIndex = Response.indexOf("200");
  if (retIndex < 0) {
    Serial.println(F("HTTP session failed!"));
    return (retIndex);
  }

  webReadSize = Response.substring(retIndex + 4).toInt();
  Serial.print(F("\nThe webpage reading size is: "));
  Serial.print(webReadSize);
  Serial.print(F(" bytes\n"));

  //Serial.println(F("\nAT+HTTPREAD"));
  mySerial.println("AT+HTTPREAD");  //Read the content of webpage
  delay(DLY5S);                     //Waits 5 seconds
  wdt_reset();

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  retIndex = Response.indexOf(String(webReadSize));
  if (retIndex < 0) {
    Serial.println(F("Reading failed!"));
    return (retIndex);
  }

  Serial.println(F("\nReading done successfully!"));
  return (0);
}
//====================================================================================================

//==================================================HTTP_TERM=========================================
int Command_SIMHTTPTERM() {
  int retIndex = -1;
  int timeKeeper = 0;

  //Serial.println(F("\nAT+HTTPTERM"));
  mySerial.println("AT+HTTPTERM");  //Terminate HTTP
  delay(DLYHLF);

  Response = "";
  while (mySerial.available()) {
    chrRead = mySerial.read();
    Response.concat(chrRead);
    wdt_reset();
  }
  Serial.print("\n");
  Serial.println(Response);

  retIndex = Response.indexOf("OK");

  if (retIndex < 0) {
    Serial.println(F("HTTP termination failed!"));
    wdt_reset();
    delay(DLYFUL);
    return (retIndex);
  }

  return (0);
}
//====================================================================================================

//==================================================SETUP=============================================
void setup() {
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(SERBDRT);
  Serial.setTimeout(SERTOUT);

  Serial.print(F("\nThe Serial port initialized ..."));
  delay(DLYFUL);

  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(SERBDRT);
  mySerial.setTimeout(SERTOUT);

  Serial.print(F("\nThe SIM800 communication port initialized ..."));
  delay(DLYFUL);

  //The SIM800L Reset PIN setup
  pinMode(SIM800_RS, OUTPUT);

  Serial.print(F("\nSIM800 reset pin configured ..."));
  delay(DLYFUL);

  wdt_enable(WDTO_8S);

  Serial.print(F("\nWatchdog timeout set to 8 seconds ..."));
  delay(DLYFUL);

  //Setup SIM800
  Serial.print(F("\nSetting up SIM800 ..."));
  delay(DLYFUL);
  retVal = Command_SIMReset();

  SIMSTATE = HANDSHAKE;
  retVal = Command_Handshake();
  if (retVal > -1) {

    SIMSTATE = SIGNALSTRENGTH;
    retVal = Command_SIGSTRNGTH();
    if (retVal > -1) {

      SIMSTATE = SIMPLUGGED;
      retVal = Command_SIMPLUGGED();
      if (retVal > -1) {

        SIMSTATE = SIMREG;
        retVal = Command_SIMREG();
        if (retVal > -1) {

          SIMSTATE = HTTP_SETPARAM;
          retVal = Command_SIMHTPPSETPARAM();
          if (retVal > -1) {

            SIMSTATE = HTTP_INIT;
            retVal = Command_SIMHTTPINIT(SSL_EN);
            if (retVal > -1) {

              SIMSTATE = HTTP_TRANSITION;
              retVal = Command_SIMHTTPTRANS();

              SIMSTATE = HTTP_TERM;
              retVal = Command_SIMHTTPTERM();
            }
          }
        }
      }
    }
  }
}
//====================================================================================================

//==================================================MAIN_LOOP=========================================
void loop() {

  wdt_reset();
  updateSerial();
}
//====================================================================================================

//==================================================SERIAL_UPDATE=====================================
//It copies any input from the Serial to the mySerialial and vice versa
void updateSerial() {
  delay(DLYHLF);
  wdt_reset();

  while (Serial.available()) {
    mySerial.write(Serial.read());  //Forward what Serial received to Software Serial Port
    wdt_reset();
  }
  while (mySerial.available()) {
    Serial.write(mySerial.read());  //Forward what Software Serial received to Serial Port
    wdt_reset();
  }
}
//====================================================================================================