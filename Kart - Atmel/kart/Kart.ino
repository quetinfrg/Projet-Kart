/*
  Analog Input

  Demonstrates analog input by reading an analog sensor on analog pin 0 and
  turning on and off a light emitting diode(LED) connected to digital pin 13.
  The amount of time the LED will be on and off depends on the value obtained
  by analogRead().

  The circuit:
  - potentiometer
    center pin of the potentiometer to the analog input 0
    one side pin (either one) to ground
    the other side pin to +5V
  - LED
    anode (long leg) attached to digital output 13
    cathode (short leg) attached to ground

  - Note: because most Arduinos have a built-in LED attached to pin 13 on the
    board, the LED is optional.

  created by David Cuartielles
  modified 30 Aug 2011
  By Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/AnalogInput
*/

#include "EnableInterrupt.h"

#define INPUT_CAPTEUR_VITESSE    4                 // Entrée dédiée au capteur de vitesse
#define DIAMETRE_ROUE_MM      866.0f                // diameter roue en mm
#define DIAMETRE_ROUE_M       (float) ((float)DIAMETRE_ROUE_MM / 1000.0f) // diameter roue en m 


volatile unsigned char CountDemiTour=0;

unsigned char Vitesse = 0;
unsigned char MemoCountDemiTour=0;
unsigned char NbDetectionParSec=0;

unsigned long TempsEcoule = 0;
unsigned long CountSeconde = 0;
float NbTourParSec = 0;
unsigned int NbTourParHeure = 0;

void CAPTEUR_INT (void)
{
  CountDemiTour++;  
  //Serial.println(CountDemiTour);
}

void setup() {
  // start serial connection
  Serial.begin(9600);
  
  // Configure input en pull up
  pinMode(INPUT_CAPTEUR_VITESSE, INPUT_PULLUP);
  
  // Configure interruption
  enableInterrupt(INPUT_CAPTEUR_VITESSE, CAPTEUR_INT, RISING);
}

void loop() {
  char tempCount = 0;
  
  if ((millis() - CountSeconde) > 1000)
  {
    tempCount = CountDemiTour;
    TempsEcoule ++;

    Serial.print(TempsEcoule);
    Serial.print(", ");
    
    CountSeconde = millis();
    NbDetectionParSec = (tempCount - MemoCountDemiTour);

    //Serial.print("NbDetectionParSec:");
    //NbTourParSec = (float) (NbDetectionParSec / 2);
    //NbTourParHeure = (NbTourParSec * 3600);   // tour/heure
    //Vitesse = NbTourParHeure * DIAMETRE_ROUE_M; // m/heure
    //Vitesse = Vitesse / 1000; //km/heure
    
    Vitesse = (unsigned char) ((float) NbDetectionParSec * 1.8f * DIAMETRE_ROUE_M);
    //Serial.print("Vitesse: ");
    
    Serial.print(Vitesse);
    Serial.println(", ");
    //Serial.println(" km/h");
    
    
    
    MemoCountDemiTour = tempCount;
  }
}
