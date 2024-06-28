#include <SPI.h>
#include <SD.h>

// Define CS pins for each SPI interface
#define MAXCS1 25
#define MAXCS2 26
#define SD_CS 5
#define LED_PIN 2

// Define shared SPI pins
// #define MAXSCLK 18
// #define MAXDO 19
// #define MAXMOSI 23

byte dummyData[] = {0x00, 0x00, 0x00, 0x00};
SPIClass *MAX1 = NULL;

int received_data1[4];
hw_timer_t * timer = NULL;
int LED_STATE=LOW;
// int received_data2[4];
int TemperatureFlag1 = 0, TemperatureFlag2 = 0, intr_cntr = 0,temperatureFlag=0;\
char CTEMP,htemp;
// hw_timer_t * timer = NULL;
// int LED_STATE = LOW;
// int TemperatureFlag1 = 0, TemperatureFlag2 = 0, intr_cntr = 0;
File dataFile;

void IRAM_ATTR onTimer() {   
  intr_cntr++;
  if (intr_cntr == 500) {
    TemperatureFlag1 = 1;
  }
  if (intr_cntr == 1000) {
    TemperatureFlag2 = 1;
    intr_cntr = 0;
  }
}

void setup() {
    Serial.begin(115200); 

    MAX1 = new SPIClass(VSPI);
    MAX1->begin();

    pinMode(MAXCS1, OUTPUT);
    digitalWrite(MAXCS1, HIGH); // Deselect the first MAX31855
    pinMode(MAXCS2, OUTPUT);
    digitalWrite(MAXCS2, HIGH); // Deselect the second MAX31855
    pinMode(LED_PIN, OUTPUT);

    // Initialize SD card
    if (!SD.begin(SD_CS)) {
        Serial.println("Initialization failed!");
        return;
    }
    Serial.println("Initialization done.");

    timer = timerBegin(0,80,true);             // timer 0, prescalar: 80, UP counting
    timerAttachInterrupt(timer, &onTimer, true);   // Attach interrupt
    timerAlarmWrite(timer, 1000, true);     // Match value= 1000000 for 1 sec. delay.
    timerAlarmEnable(timer);
}

void loop() {
    if (TemperatureFlag1 == 1) {
        Serial.print("Sensor 1 data: ");
        logData("Sensor 1", MAX1, MAXCS1);
        Serial.print("\n");

        TemperatureFlag1 = 0; // Reset the flag
        LED_STATE = !LED_STATE;         
        digitalWrite(LED_PIN, LED_STATE);
    }

    if (TemperatureFlag2 == 1) {
        Serial.print("Sensor 2 data: ");
        logData("Sensor 2", MAX1, MAXCS2);
        Serial.print("\n");
        TemperatureFlag2 = 0; // Reset the flag
    }
}

void logData(const char* sensorName, SPIClass *spi, int csPin) {
    spi->beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW); // Pull CS low to prepare for transfer
    for (int i = 0; i < 4; i++) {
        received_data1[i] = spi->transfer(dummyData[i]);
        Serial.print(received_data1[i], HEX); // Print received data
        Serial.print(" ");
    }
    digitalWrite(csPin, HIGH); // Pull CS high to signify end of data transfer
    spi->endTransaction();

    dataFile = SD.open("datalog.txt", FILE_APPEND);
    if (dataFile) {
        dataFile.print(sensorName);
        dataFile.print(": ");
        for (int i = 0; i < 4; i++) {
            dataFile.print(received_data1[i], HEX);
            dataFile.print(" ");
        }
        dataFile.println();
        dataFile.close();
        Serial.println("Data logged.");
    } else {
        Serial.println("Error opening datalog.txt");
    }
}
