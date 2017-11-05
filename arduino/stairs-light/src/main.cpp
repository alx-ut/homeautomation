#include <Arduino.h>
#include <ArduinoLog.h>
#include<UltraDistSensor.h>

const int LIGHT_PIN = 2;

const int LDR_PIN = A1;
const int POT_PIN = A3;

const int SENSOR1_ECHO_PIN = 5;
const int SENSOR1_TRIG_PIN = 6;

const int SENSOR2_ECHO_PIN = 7;
const int SENSOR2_TRIG_PIN = 8;


const int distance1_threshold = 50;
const int distance2_threshold = 50;

const long light_time = 180000;

UltraDistSensor sensor1;
UltraDistSensor sensor2;


bool needLightState = false;
bool barrierState = false;
long lastBarrierDetectTime = 0;


bool oldNeedLight = false;
long lastLightOnTime;

long lastSensorRerreshTime = 0;
int lastSensorRefresh = 0;

int distance1;
int distance2;

bool forceRecheck;


void setup() {
    Serial.begin(9600);

    Log.begin(LOG_LEVEL_VERBOSE, &Serial);

    pinMode(POT_PIN, INPUT);
    pinMode(LDR_PIN, INPUT);
    pinMode(LIGHT_PIN, OUTPUT);
    digitalWrite(LIGHT_PIN,LOW);

    sensor1.attach(SENSOR1_TRIG_PIN,SENSOR1_ECHO_PIN);
    sensor2.attach(SENSOR2_TRIG_PIN,SENSOR2_ECHO_PIN);
    Log.notice(  "Initialize sensors" CR);
}



void updateState() {        
    bool needLight = barrierState && needLightState;
    if (needLight) {
        lastLightOnTime = millis();
    } else if (!forceRecheck) {
        needLight = millis() - lastLightOnTime < light_time;
    }
    forceRecheck = false;
    if (oldNeedLight != needLight) {
        oldNeedLight = needLight;
        Log.notice("Light state changed : %t" CR, needLight);
    }
    if (needLight) {
        digitalWrite(LIGHT_PIN, HIGH);
    } else {
        digitalWrite(LIGHT_PIN, LOW);
        lastLightOnTime = 0;
    }
}


void loop() {    
    int threshold = analogRead(POT_PIN);    
    int lightness = analogRead(LDR_PIN);    

    if (threshold < 20 || threshold > 1000) {
        forceRecheck = true;
    }

    needLightState = (lightness > threshold);    

    if (millis() - lastSensorRerreshTime > 500) { // delay between UltraDistSensor measurement
        if (lastSensorRefresh == 0) {
            distance1 = sensor1.distanceInCm();
            lastSensorRefresh = 1;
        } else {
            distance2 = sensor2.distanceInCm();
            lastSensorRefresh = 0;            
        }
        lastSensorRerreshTime = millis();
        Log.notice("Lights : %d, %d" CR, lightness, threshold);
        Log.notice("Distance : %d, %d" CR, distance1, distance2);
    }    

    if (distance1 < distance1_threshold || distance2 < distance2_threshold) {
        barrierState = true;
        lastBarrierDetectTime = millis();
    } else {
        barrierState = false;
    }

    updateState();          
}
