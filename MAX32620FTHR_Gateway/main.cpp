#include "mbed.h"
#include "LoRa.h"
#include "UARTSerial.h"

DigitalOut  idle(LED2);
DigitalOut  receiving(LED1);

Serial pc(USBTX, USBRX);
Serial _serial(P3_1, P3_0,9600);


int counter = 0;

void onReceive(int packetSize);

main()
{
    // setup
    pc.printf("LoRa Callback Rx . . .\n");
    if (!LoRa.begin(915E6))
    {
        pc.printf("LoRa Radio not Found!\n");
    } else { 
        pc.printf("LoRa Radio Started.\n");
    }
    idle = true;
    LoRa.onReceive(onReceive);
    LoRa.receive();
    
    //loop
    while(1)
    {
        // do nothing
    }
}

void onReceive(int packetSize)
{
    idle = false;
    char msg[45];
    pc.printf("PACKET RECEIVED, SIZE:%d \n", packetSize);
    // pc.printf("MSG[0]: %c \n", msg[0]);
    // pc.printf("MSG[1]: %c \n", msg[1]);
    // pc.printf("MSG[2]: %c \n", msg[2]);
    // pc.printf("MSG[3]: %c \n", msg[3]);
    // pc.printf("MSG[4]: %c \n", msg[4]);
    for (int i = 0; i<packetSize; i++)
    {   
        if ( i < 45) 
        {
            msg[i] = (char)LoRa._getc();
        }
        receiving = !receiving;
    }
    if (msg[0] == '1')
    {
        // indicates this message comes from the canary!
        pc.printf("received from 0\n");
        _serial.printf("%s\n", msg);
    } 
    else {
        pc.printf("dont know who this was from\n");    
    }
    pc.printf("MSG: %s, ",msg);
    pc.printf("RSSI:  %d\n", LoRa.packetRssi());
    // _serial.printf("%s\n", msg);
    receiving = false;
    idle = true;
}

