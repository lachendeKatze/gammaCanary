#include "mbed.h"
#include "LoRa.h"
#include "MBed_Adafruit_GPS.h"
#include "MAX17055.h"

Serial pc (USBTX, USBRX);       // debug serial port
DigitalOut led1(LED1);          // debug blinker

/******************************************************************************/
/*************************** Power & Fuel Gauge *******************************/                                 
/******************************************************************************/
DigitalOut powerHLD(P2_2, 1); //keep power on when connected to battery
I2C i2c(I2C2_SDA, I2C2_SCL);
MAX17055 max17055(i2c);
int gaugeStatus;
int stateOfCharge, timeToEmpty;
float batteryVoltage, batteryCurrent, boardTemperature, tteReport;
#define FUEL_REFRESH 30 // 120 secs, 2 minutes for now, testing . . .

Timer fuelTimer;
Timer batteryEmergencyPause; // avoid sending alert right after starting unit up!
bool startBatteryEmergency;
bool inBatteryEmergencyState;
#define EMERBEGIN 180000 // 3 minutes  in millissecond after setup runs
void readFuelGauge(void);
/************************* End Power & Fuel Guage *****************************/

/******************************************************************************/
/***************************** LoRa TxRx **************************************/
/******************************************************************************/
#define GATEWAYID           0x30 // LoRa Gateway ID
#define LOCALADDRESSID      0x31 // this gamma canary ID
#define GPSDATA             0x40
#define CPMDATA             0x41
#define FUELDATA            0x42
#define LOWBATTERY          0x43
// #define CPMALERT            0x44

// data transmission/report intervals
// #define LOCATION_INTERVAL 15.0      // GPS location every 120 secs
// #define CPM_INTERVAL  60.0          // CPM every 60 secs
// #define FUEL_INTERVAL 90.0          // Fuel Gauge every 90 secs

// data transmission/report intervals
float LOCATION_INTERVAL = 15.0;         // GPS location every 120 secs
float CPM_INTERVAL      = 60.0;         // CPM every 60 secs
float FUEL_INTERVAL     = 90.0;         // Fuel Gauge every 90 secs

volatile int locationCountDown, cpmCountDown, fuelCountDown; 

Ticker transmissionTicker;
void transmitLoRaTicker(void)
{
    if (locationCountDown > 0) locationCountDown--;
    if (cpmCountDown > 0) cpmCountDown--;
    if (fuelCountDown >0) fuelCountDown--;
} 

void transmitLocation(void);
void transmitCPM(void);
void transmitFuel(void);
void batteryLowTransmit();
void onReceive(int packetSize);
/*************************** End LoRa *****************************************/

/******************************************************************************/
/******************************** GPS *****************************************/
/******************************************************************************/
double currentLatitude, currentLongitude, previousLatitude, previousLongitude;
Serial * _gpsSerial;

#define GPS_REFRESH 5000 // 2000 millisecs???
Timer gpsTimer; // GPS data collection/refresh timer

void transformGPSData(double lat, char p, double lon, char m);
// GPS command list at http://www.adafruit.com/datasheets/PMTK_A08.pdf
/**************************** End GPS *****************************************/

/******************************************************************************/
/*********************** Begin Radiation Monitoring ***************************/
/******************************************************************************/
// PMOD Port 1 pins for radiation sensor
volatile int signalCounter, noiseCounter;
InterruptIn signalPin(P1_0);
InterruptIn noisePin(P1_1);
// interrupt service routines(ISR) which count are called whenever signal or noise pins are triggered
void signalISR(){ signalCounter++; led1 = !led1;}
void noiseISR(){ noiseCounter++; } 

// #define CPMALERTLEVEL   50 // set very low or testing purposes.
// #DEFINE CPMMAXALERTS     3 // only three at a time to not bomb SMS network!
#define CPM_REFRESH     60 // 60 seconds in a minute
Timer cpmTimer; // CPM data collection, determine total current counts per minute

int currentSignalCount, currentNoiseCount, previousCPM, currentCPM;
int previousTotalSignalCount, previousTotalNoiseCount;

void calculateCPM();
/*********************** End Radiation Monitoring *****************************/


int main()
{
    /****************************** Setup *************************************/
    pc.printf(" ********* gamma canary v001 ********* \n");
    
    /** 0. Confirm running on battery power **/
    if (powerHLD.is_connected()) {pc.printf("Battery power connected...\n");}
    
    /** 1. LoRa TxRx initialization **/
    if (!LoRa.begin(915E6)) {pc.printf("LoRa not found!");} 
    else {  pc.printf("LoRa started...\n");}
    // LoRa.onReceive(onReceive);
    // LoRa.receive();
    transmissionTicker.attach(&transmitLoRaTicker,1.0);
    
    /** 2. GPS/NMEA Initialization **/
    currentLongitude = 0.00;
    previousLongitude = currentLongitude;
    currentLatitude = 0.00;
    previousLatitude = currentLatitude;
    
    _gpsSerial = new Serial(P3_1,P3_0); //serial object for use w/ GPS
    Adafruit_GPS myGPS(_gpsSerial); 
    myGPS.begin(9600);  
    char c;
    myGPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //these commands are defined in MBed_Adafruit_GPS.h; a link is provided there for command creation
    myGPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    myGPS.sendCommand(PGCMD_ANTENNA);
    gpsTimer.start();
    pc.printf("GPS setup done...\n");
    
    //** 3. Radiation Detecton - CPM(Counts Per Minute)
    currentSignalCount = 0;
    previousTotalSignalCount = 0;
    currentNoiseCount = 0;
    previousTotalNoiseCount = 0;
    currentCPM = 0;
    previousCPM = 0;
    signalPin.rise(&signalISR);     // attach ISR function to the rising edge
    noisePin.rise(&noiseISR);       // attach ISR function to the rising edge
    cpmTimer.start();
    pc.printf("Starting radiation monitoring...\n");

    /** 4. Fuel Gauge/Battery Monitoring **/
    max17055.init(0.05f);
    max17055.status(&gaugeStatus);
    fuelTimer.start();
    startBatteryEmergency = false;
    inBatteryEmergencyState = false;
    batteryEmergencyPause.start();
    pc.printf("MAX17055 status: %04X\r\n", (uint16_t)gaugeStatus);
    wait(3.0);
    pc.printf("Setup complete...\n");
    
    /***************************** End Setup **********************************/
    // loop()
    while(1)
    {
        
        /** GPS/Location Data Collection"loop" here **/
        c = myGPS.read();   //queries the GPS
        if ( myGPS.newNMEAreceived() ) 
        {
            if ( !myGPS.parse(myGPS.lastNMEA()) ) {
                continue;   
            }    
        }
        
        //check if enough time has passed to warrant printing GPS info to screen
        //note if refresh_Time is too low or pc.baud is too low, GPS data may be lost during printing
        if (gpsTimer.read_ms() >= GPS_REFRESH) 
        {
            gpsTimer.reset();
            if (myGPS.fix) {
                // pc.printf("Location: %5.2f%c, %5.2f%c\n", myGPS.latitude, myGPS.lat, myGPS.longitude, myGPS.lon);
                // previousLatitude = currentLatitude;
                // previousLongitude = currentLongitude;            
                // currentLatitude = myGPS.latitude;
                // currentLongitude = myGPS.longitude;
                transformGPSData(myGPS.latitude, myGPS.lat, myGPS.longitude, myGPS.lon);
            }
        }        
        
        if (cpmTimer.read() >= CPM_REFRESH)
        {
            cpmTimer.reset();
            calculateCPM();    
        }
        
        if (fuelTimer.read() >= FUEL_REFRESH)
        {
            fuelTimer.reset();
            readFuelGauge();    
        }
        
        // special 1 time timer to begin battery monitoring for emergency lo battery warning
        
        if( batteryEmergencyPause.read_ms() >= EMERBEGIN )
        {
            batteryEmergencyPause.stop();
            startBatteryEmergency = true;  
        }
        
        /** LoRa Data Transmision **/
        if (locationCountDown == 0) 
        { 
            locationCountDown = LOCATION_INTERVAL;
            transmitLocation();      
        }
        if (cpmCountDown == 0) 
        {
            cpmCountDown = CPM_INTERVAL;
            transmitCPM();    
        }
        if (fuelCountDown == 0) 
        {
            fuelCountDown = FUEL_INTERVAL;
            transmitFuel();    
        }
        
    } // end loop
    
} 

/** GPS Functions **/
void transformGPSData(double lat, char parallel, double lon, char meridian)
{
    float degrees = floor(lat/100.00);
    double minutes = lat - (100.00*degrees);
    minutes /= 60.00;
    degrees += minutes;
    if (parallel=='S') { degrees *= -1.00; }
    
    double dLatitude = degrees;
    
    degrees = floor(lon/100.00);
    minutes = lon - (100.00*degrees);
    minutes /= 60.00;
    degrees += minutes;
    if (meridian == 'W') { degrees *= -1.00; }
    
    double dLongitude = degrees;
    currentLatitude = dLatitude;
    currentLongitude = dLongitude;
    pc.printf("LAT: %f ", dLatitude);
    pc.printf("LON: %f\n", dLongitude);
}

/** CPM functions **/
void calculateCPM()
{   
    
    currentSignalCount = signalCounter - previousTotalSignalCount;
    previousTotalSignalCount = signalCounter;
    
    currentNoiseCount = noiseCounter - previousTotalNoiseCount;
    previousTotalNoiseCount = noiseCounter;
    
    previousCPM = currentCPM;
    currentCPM = currentSignalCount - currentNoiseCount;
    
    pc.printf("signal: %d, noise: %d, ", currentSignalCount, currentNoiseCount);
    pc.printf("Current CPM: %d\n", currentCPM);
   
}

/** Fuel Guage Functions **/
void readFuelGauge()
{
    max17055.v_cell(&batteryVoltage);
    printf("%6.3fV ", batteryVoltage);
    
    max17055.current(&batteryCurrent);
    printf("%6.3fI ", batteryCurrent);
    
    max17055.temp(&boardTemperature);
    printf("%6.3fC ", boardTemperature);
    
    max17055.reportSOC(&stateOfCharge);
    if (startBatteryEmergency and !inBatteryEmergencyState)
    {
        pc.printf("emergency battery monitoring set\n");
        if (stateOfCharge < 51.0)
        {  
           inBatteryEmergencyState = true; // prevents sending messages over and over again, future versions to reset this when bat > 50 again;   
           batteryLowTransmit();
        }
    }
    printf(" SOC:%d\\% ",stateOfCharge);
    
    // max17055.tte1(&timeToEmpty);
    // tteReport = ((5.625)*timeToEmpty)/120.0;
    // printf(" TTE:%6.2fhours \n", tteReport);
    
}

/** LoRa TxRx Functions **/
void transmitLocation()
{
    // transmite GPS/Location data, lat/lon
    LoRa.beginPacket();
    
    // Packet Header 
    LoRa.printf("%c",LOCALADDRESSID); LoRa.printf("**");    
    LoRa.printf("%c",GPSDATA); LoRa.printf("**");    
    
    // Packet Contents
    LoRa.printf("%f",currentLatitude);LoRa.printf("**");
    LoRa.printf("%f",currentLongitude);LoRa.printf("**");
    
    LoRa.endPacket();
    pc.printf("LoRa GPS Packet Sent\n");    
}

void transmitCPM()
{
    // Transmit Radiation data: CPM = Counts Per Minute, current reading
    LoRa.beginPacket();
    
    // Packet Header 
    LoRa.printf("%c",LOCALADDRESSID); LoRa.printf("**");    
    LoRa.printf("%c",CPMDATA); LoRa.printf("**");    
    
    // Packet Contents
    LoRa.printf("%d",currentCPM);LoRa.printf("**");
    LoRa.printf("%d", "-1");LoRa.printf("**");
    
    LoRa.endPacket();
    pc.printf("LoRa CPM Packet Sent\n");  
}
void transmitFuel()
{
    // Transimet Battery/Fuel Gauge data: voltage, current, temp 
    LoRa.beginPacket();
    
    // Packet Header 
    LoRa.printf("%c",LOCALADDRESSID); LoRa.printf("**");    
    LoRa.printf("%c",FUELDATA); LoRa.printf("**");    
    
    LoRa.printf("%d",stateOfCharge);LoRa.printf("**"); 
    LoRa.printf("%f",tteReport);LoRa.printf("**"); // time to empty in hours
    LoRa.printf("%f",boardTemperature);LoRa.printf("**");
       
    // Packet Contents
    
    LoRa.endPacket();
    pc.printf("LoRa Fuel Gauge Packet Sent\n");
}

void batteryLowTransmit() 
{
    /** broadcast warning low battery/increased transmisson intervals **/
    
    // extend transmission intervals to conserve power
    LOCATION_INTERVAL  = 150.0;     
    CPM_INTERVAL       = 600.0;          
    FUEL_INTERVAL      = 900.0;          
    
    // Transimet Battery/Fuel Gauge data: voltage, current, temp 
    LoRa.beginPacket();
    
    // Packet Header 
    LoRa.printf("%c",LOCALADDRESSID); LoRa.printf("**");    
    LoRa.printf("%c",LOWBATTERY); LoRa.printf("**");    
    
    LoRa.printf("%d",stateOfCharge);LoRa.printf("**"); 
    
    LoRa.endPacket();
    pc.printf("LoRa Low Battery Packet Sent\n");
    
}

void onReceive(int packetSize)
{
    char msg[45];
    pc.printf("PACKET RECEIVED, SIZE:%d \n", packetSize);
    for (int i = 0; i<packetSize; i++)
    {   if ( i < 45) 
        {
            msg[i] = (char)LoRa._getc();
        }
    }
    pc.printf("RSSI:  %d, ", LoRa.packetRssi()); pc.printf("MSG:%s \n",msg); 
    if (msg[0] == LOCALADDRESSID)
    {
        pc.printf("received from %x\n", msg[0]);
    } 
    else {
        pc.printf("dont know who, %x is, ignore msg\n", msg[0]);    
    }
}
