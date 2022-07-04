#include "oled-wing-adafruit.h"
#include "SparkFun_VCNL4040_Arduino_Library.h"
#include <Wire.h>
#include <blynk.h>

VCNL4040 proximitySensor;
OledWingAdafruit display;

SYSTEM_THREAD(ENABLED)

#define blue D8
#define green D7
#define red D6
#define button D5
#define tempSensor A4
#define potentiometer A5

bool mode = true;
bool calibrate = false;
bool minLight = true;
bool maxLight = false;
bool done = false;
bool notifyBright = false;
bool notifyLow =false;
int minValue;
int maxValue;
unsigned int ambientValue;
float longitude;
float latitude;
float altitude;


BLYNK_WRITE(V1) {
    longitude = param[1].asFloat();
    latitude = param[0].asFloat();
    altitude = param[2].asFloat();
}

BLYNK_WRITE(V2)
{
    Blynk.run(V1);
    displaySetup();
    display.println("Darwin's Iphone");
    display.print("Lat: ");
    display.println(latitude, 7);
    display.print("Lon: ");
    display.println(longitude, 7);
    display.print("Altitute: ");
    display.println(altitude, 2);
    display.display();
    delay(2000);
}

void calibration()
{
    if (digitalRead(button) == HIGH)
    {
        if (minLight == true)
        {
            minLight = false;
            maxLight = true;
            done = false;
            delay(200);
        }
        else if (maxLight == true)
        {
            minLight = false;
            maxLight = false;
            done = true;
            delay(200);
        }
        else if (done == true)
        {
            calibrate = true;
        }
    }
}


void displaySetup()
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
}

void getAmbient()
{
    displaySetup();
    display.print("Ambient light level: ");
    display.println(ambientValue);
    display.display();
}

void setup()
{
    display.setup();
    displaySetup();
    display.setTextColor(WHITE);
    display.display();
    Wire.begin();
    proximitySensor.begin();
    proximitySensor.powerOffProximity();
    proximitySensor.powerOnAmbient();
    pinMode(blue, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(red, OUTPUT);
    pinMode(button, INPUT);
    pinMode(potentiometer, INPUT);
    Blynk.begin("Qoy41XMwJ0CLD926YLzIdrkFZaW1IvQX", IPAddress(167, 172, 234, 162), 8080);
}

void loop()
{
    Blynk.run();
    display.loop();
    ambientValue = proximitySensor.getAmbient();
    Blynk.virtualWrite(V0, ambientValue);
    uint64_t reading = analogRead(tempSensor);
    double voltage = (reading * 3.3) / 4095.0;
    double temperature = (voltage - 0.5) * 100;
    int farenheit = temperature * 1.8 + 32;
    Blynk.virtualWrite(V4, temperature);
    Blynk.virtualWrite(V5, farenheit);

    if (display.pressedA())
    {
        mode = true;
    }
    else if (display.pressedB())
    {
        mode = false;
    }
    else if (display.pressedC())
    {
        mode = false;
        calibrate = false;
        minLight = true;
        digitalWrite(red, LOW);
        digitalWrite(green, LOW);
        digitalWrite(blue, LOW);
    }
    if (mode == true)
    {
        digitalWrite(red, LOW);
        digitalWrite(green, LOW);
        digitalWrite(blue, LOW);
        displaySetup();
        display.print(temperature);
        display.println("C");
        display.print(farenheit);
        display.print("F");
        display.display();
        delay(200);
    }
    else if (mode == false)
    {
        if (calibrate == false)
        {
            calibration();
            if (minLight == true)
            {
                int unmapped_value = analogRead(potentiometer);
                int value = map(unmapped_value, 0, 4095, 0, 65535);
                minValue = value;
                displaySetup();
                display.println("Minimum Ambient Light:");
                display.print(value);
                display.display();
            }
            else if (maxLight == true)
            {
                int unmapped_value = analogRead(potentiometer);
                int value = map(unmapped_value, 0, 4095, 0, 65535);
                maxValue = value;
                displaySetup();
                display.println("Maximum Ambient Light:");
                display.print(value);
                display.display();
            }
        }
        else if (calibrate == true)
        {
            getAmbient();
            if (notifyBright == true) {
                Blynk.notify("Might want to check this out, ambient light is too high!");
            } else if (notifyLow == true) {
                Blynk.notify("Watch out, ambient light too low");
            }
            if (ambientValue < maxValue && ambientValue > minValue)
            {
                digitalWrite(red, LOW);
                digitalWrite(green, HIGH);
                digitalWrite(blue, LOW);
                notifyBright = false;
                notifyLow = false;
            }
            else if (ambientValue > maxValue)
            {
                digitalWrite(red, HIGH);
                digitalWrite(green, LOW);
                digitalWrite(blue, LOW);
                notifyBright = true;
                notifyLow = false;
            }
            else if (ambientValue < minValue)
            {
                digitalWrite(red, LOW);
                digitalWrite(green, LOW);
                digitalWrite(blue, HIGH);
                notifyBright = false;
                notifyLow = true;
            }
        }
    }
}