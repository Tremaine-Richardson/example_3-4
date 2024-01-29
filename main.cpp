//=====[Libraries]=============================================================


#include "mbed.h"
#include "arm_book_lib.h"


//=====[Defines]===============================================================


#define TIME_INCREMENT_MS                       10
#define HEADLIGHT_ON_DELAY 			            1000
#define HEADLIGHT_OFF_DELAY			            2000
#define DEBOUNCE_BUTTON_TIME_MS                 40
#define HEADLIGHT_OFF_THRESHOLD                 .3
#define HEADLIGHT_ON_THRESHOLD                  .6
#define DUSK_THRESHOLD                          .2
#define DAYLIGHT_THRESHOLD                      .5


//=====[Declaration of public data types]======================================


typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_FALLING,
    BUTTON_RISING
} buttonState_t;


//=====[Declaration and initialization of public global objects]===============


DigitalIn driverSeat(D2);
DigitalIn ignitionSwitch(D5);

DigitalOut rightHeadlight(D3);
DigitalOut leftHeadlight(D4);
DigitalOut engineOnLED(LED2); 

AnalogIn lightSensor(A1);
AnalogIn potentiometer(A0); 


//=====[Declaration and initialization of public global variables]=============


float lightSensorReading = 0.0;
float potentiometerReading = 0.0;
int accumulatedTimeDelay = 0;
float sensorReading = 0.0;

bool engineRunning = OFF;

int accumulatedDebounceButtonTime     = 0;
int numberOfEnterButtonReleasedEvents = 0;
buttonState_t ignitionSwitchState;

//=====[Declarations (prototypes) of public functions]=========================


void inputsInit();
void outputsInit();

void ignitionStateUpdate();
void headlightStateUpdate();

void ignitionSwitchInit();
bool ignitionSwitchUpdate();


//=====[Main function, the program entry point after power on or reset]========


int main()
{
   inputsInit();
   outputsInit();
   while (true) {
       ignitionStateUpdate();
       headlightStateUpdate();
       delay(TIME_INCREMENT_MS);
   }
}


//=====[Implementations of public functions]===================================


void inputsInit()
{
   driverSeat.mode(PullDown);
   ignitionSwitch.mode(PullUp);
}


void outputsInit()
{
   rightHeadlight = OFF;
   leftHeadlight = OFF;
   engineOnLED = OFF;
}


void ignitionStateUpdate()
{
    bool ignitionSwitch = ignitionSwitchUpdate();
   if( driverSeat && ignitionSwitch ) {
     if(!engineRunning){
         // Start the engine
         engineOnLED = ON;
         engineRunning = true;
     } else{
         engineOnLED = OFF;
         engineRunning = false;
     }
   }

   if (engineRunning && !driverSeat){
       if ( ignitionSwitch ){
       engineOnLED = OFF; 
       engineRunning = false;
       }
       // Keep engine running in absence of driver
   }
}

void headlightStateUpdate() 
{
    potentiometerReading = potentiometer.read();
    lightSensorReading = lightSensor.read();
    if ( engineRunning ) {
        if ( potentiometerReading > HEADLIGHT_OFF_THRESHOLD ) {  
            if ( potentiometerReading < HEADLIGHT_ON_THRESHOLD ) {  //Mode is AUTO
                 accumulatedTimeDelay = accumulatedTimeDelay + TIME_INCREMENT_MS;
                if ( lightSensorReading > DUSK_THRESHOLD ) {
                    if ( lightSensorReading < DAYLIGHT_THRESHOLD ) { //Neither Daylight or Dusk
                        accumulatedTimeDelay = 0;
                    }
                    else { //Daylight
                        if ( accumulatedTimeDelay >= HEADLIGHT_OFF_DELAY) {
                        rightHeadlight = OFF;
                        leftHeadlight = OFF;
                        }
                    }
                }
                else { //Dusk
                    if (accumulatedTimeDelay >= HEADLIGHT_ON_DELAY) {
                        rightHeadlight = ON;
                        leftHeadlight = ON;
                    }
                }
            }
            else { //Mode is ON
                accumulatedTimeDelay = 0;
                rightHeadlight = ON;
                leftHeadlight = ON;
             }
    }
    else { //Mode is OFF
            accumulatedTimeDelay = 0;
            rightHeadlight = OFF;
            leftHeadlight = OFF;
        }
    }
    else {
        rightHeadlight = OFF;
        leftHeadlight = OFF;
    }
}

void ignitionSwitchInit()
{
    if( ignitionSwitch == 1) {
        ignitionSwitchState = BUTTON_UP;
    } else {
        ignitionSwitchState = BUTTON_DOWN;
    }
}


bool ignitionSwitchUpdate()
{
    bool ignitionSwitchReleasedEvent = false;
    switch( ignitionSwitchState ) {


    case BUTTON_UP:
        if( ignitionSwitch == 0 ) {
            ignitionSwitchState = BUTTON_FALLING;
            accumulatedDebounceButtonTime = 0;
        }
        break;


    case BUTTON_FALLING:
        if( accumulatedDebounceButtonTime >= DEBOUNCE_BUTTON_TIME_MS ) {
            if( ignitionSwitch == 0 ) {
                ignitionSwitchState = BUTTON_DOWN;
            } else {
                ignitionSwitchState = BUTTON_UP;
            }
        }
        accumulatedDebounceButtonTime = accumulatedDebounceButtonTime + TIME_INCREMENT_MS;
        break;


    case BUTTON_DOWN:
        if( ignitionSwitch == 1 ) {
            ignitionSwitchState = BUTTON_RISING;
            accumulatedDebounceButtonTime = 0;
        }
        break;


    case BUTTON_RISING:
        if( accumulatedDebounceButtonTime >= DEBOUNCE_BUTTON_TIME_MS ) {
            if( ignitionSwitch == 1 ) {
                ignitionSwitchState = BUTTON_UP;
                ignitionSwitchReleasedEvent = true;
            } else {
                ignitionSwitchState = BUTTON_DOWN;
            }
        }
        accumulatedDebounceButtonTime = accumulatedDebounceButtonTime +
                                        TIME_INCREMENT_MS;
        break;


    default:
        ignitionSwitchInit();
        break;
    }
    return ignitionSwitchReleasedEvent;
}
