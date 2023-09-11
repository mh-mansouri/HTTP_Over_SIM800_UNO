Many believe the Internet of Things, or IoT, is the next wave of digitization. With IoT, machines and devices can connect online and exchange data using cloud technology. So, connecting to the Internet is the foundation of IoT technology. Although these days we see many public places equipped with free WiFi, regarding data security, it is not advised to connect your devices to those hubs, and the safety and security issues are even much more restricted when we are talking about IoT devices because most of them contain or transfer data that are related to our personal life or private or industrial signals that are assumed to be highly protected against piercing. 
So, GSM is considered one of the most reliable connections for IoT devices because it is available almost everywhere. IoT devices usually send data of fewer than kilobytes every second. So, such a connection provides security and is cost-efficient due to bypassing the installation of WiFi hubs. However, a GSM connection brings issues and difficulties from the PCB design to the coding. 
Among many GSM modules, SIM800 is popular due to the low price, easiness of installation, and comparable straightforward coding. SIM800 is a quad-band GSM/GPRS module that works on 850MHz GSM, 900MHz EGSM, 1800MHz DCS, and 1900MHz PCS. It also features GPRS multi-slot class 12/class 10 (optional) and supports CS-1, CS-2, CS-3, and CS-4 GPRS coding schemes.
This article explains establishing an HTTP connection over SIM800 using Arduino UNO. Regarding the code simplicity, it could be used for most of the AVR microcontrollers.
I have split the connection procedure into nine steps: resetting the SIM800 module and the HTTP termination. The code provides an example of how to prepare your SIM800 through stages of handshaking and getting the SIMCard information until the way to read a web page's contents. The functions are listed as follows according to the sequence of calling in my code:

int Command_SIMReset();
int Command_Handshake();
int Command_SIGSTRNGTH();
int Command_SIMPLUGGED();
int Command_SIMREG();
int Command_SIMHTPPSETPARAM();
int Command_SIMHTTPINIT(bool SSLENABLER = false);
int Command_SIMHTTPTRANS();
int Command_SIMHTTPTERM();

I have also provided some enumerators in the code to distinguish or update the codes from the different states of the SIMCARD registration and technical status, named REGSTATUS and TECHSTATUS.
