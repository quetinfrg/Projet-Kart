/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>

/*End of auto generated code by Atmel studio */


//Beginning of Auto generated function prototypes by Atmel Studio
//End of Auto generated function prototypes by Atmel Studio

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

#define INPUT_CAPTEUR_VITESSE    4											// Entrée dédiée au capteur de vitesse
#define DIAMETRE_ROUE_MM		866.0f										// diameter roue en mm
#define DIAMETRE_ROUE_M			(float) ((float)DIAMETRE_ROUE_MM / 1000.0f) // diameter roue en m


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
}

//Fonction d'initialisation pour le microcontrôleur
void setup() {
	
	// Initialise la liaison série à 9600 bauds
	Serial.begin(9600);
	
	// Configure l'entrée logique dédiée à la récupération du capteur de vitesse en entrée pull up (mise à 1 par défault)
	pinMode(INPUT_CAPTEUR_VITESSE, INPUT_PULLUP);
	
	// Configure l'interruption qui se déclenchera à chaque fois que l'entrée capteur passe de l'état bas à l'état haut
	enableInterrupt(INPUT_CAPTEUR_VITESSE, CAPTEUR_INT, RISING);
}


void Lecture_Vitesse ()
{
	
	
	
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

		// Calcul détaillé
		//NbTourParSec = (float) (NbDetectionParSec / 2);
		//NbTourParHeure = (NbTourParSec * 3600);   // tour/heure
		//Vitesse = NbTourParHeure * DIAMETRE_ROUE_M; // m/heure
		//Vitesse = Vitesse / 1000; //km/heure
		
		// Calcul optimisé
		Vitesse = (unsigned char) ((float) NbDetectionParSec * 1.8f * DIAMETRE_ROUE_M);
		
		Serial.print(Vitesse);
		Serial.println(", ");
		
		MemoCountDemiTour = tempCount;
	}
}

void TensionBatterie ()
{
	float batt;				   //la valeur finale qui contiendra la valeur de la batterie
	float ten_mes;			  //valeur qui contiendra la valeur de la tension mesurée à l'instant t
	int ten_crit;		     //valeur booléene qui sera mise à un si la tension descends en dessous de ten_min
	int surcharge;	     	//valeur booléene qui sera mise à un si la tension descends en dessus de ten_max
	float ten_min;		   //cf surcharge
	float ten_max;		  //cf ten_crit
	float differentiel;  //valeur intermediaire pour le calcul de batt
	float inter1;
	float inter2;
	float inter3;


	ten_crit = 0;
	surcharge = 0;
	ten_min =11.7;
	ten_max = 15.0;

	scanf("%f", &ten_mes);

	if (ten_mes<ten_min)
	{
		ten_crit =1;
		printf("la tension atteint un niveau critique! \nS'arreter d'urgence!!");
	}


	if (ten_mes>ten_max)
	{
		surcharge =1;
		printf("la tension atteint un niveau critique! \nDebrancher la prise d'urgence!!");
	}


	if ((surcharge != 1) && (ten_crit != 1))
	{
		inter1 = ten_mes-ten_min;
		inter2 = inter1/10;
		inter3 = inter2/0.33;
		batt = inter3*100;


		printf("le niveau de batterie est de %f %!", batt);
	}
}

