/*Begining of Auto generated code by Atmel studio */
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

#include <Arduino.h>
#include <SPI.h>
#include "EnableInterrupt.h"

#defineLE       10 //Entree LE du MAX7961


#define INPUT_CAPTEUR_VITESSE    4                      // Entrée dédiée au capteur de vitesse
#define DIAMETRE_ROUE_MM    866.0f                    // diametre roue en mm
#define DIAMETRE_ROUE_M     (float) ((float)DIAMETRE_ROUE_MM / 1000.0f) // diametre roue en m


volatile unsigned char CountDemiTour=0;

unsigned char Vitesse = 0;
unsigned char MemoCountDemiTour=0;
unsigned char NbDetectionParSec=0;
boolean Segment[10];

unsigned long TempsEcoule = 0;
unsigned long CountSeconde1 = 0;
unsigned long CountSeconde2= 0;
float NbTourParSec = 0;
unsigned int NbTourParHeure = 0;

// Fonction d'interruption correspondant au passage de l'état bas à l'état haut de l'entrée liée au capteur de vitesse
void CAPTEUR_INT (void)
{
	CountDemiTour++;
}

// Fonction d'initialisation pour le microcontrôleur
void setup() {

	// Initialise la liaison série à 9600 bauds
	Serial.begin(9600);

	// Configure l'entrée logique dédiée à la récupération du capteur de vitesse en entrée pull up (mise à 1 par défault)
	pinMode(INPUT_CAPTEUR_VITESSE, INPUT_PULLUP);

	// Configure l'interruption qui se déclenchera à chaque fois que l'entrée capteur passe de l'état bas à l'état haut
	enableInterrupt(INPUT_CAPTEUR_VITESSE, CAPTEUR_INT, RISING);
}

// Fonction permettant le calcul de la vitesse en fonction du nombre d'impulsion par secondes
void Lecture_Vitesse ()
{
	char tempCount = 0;

	tempCount = CountDemiTour;
	TempsEcoule ++;

	Serial.print(TempsEcoule);
	Serial.print(", ");

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

void TensionBatterie ()
{
	float batt;          //pourcentage de la valeur de la batterie
	int ten_crit;        //valeur booléene qui sera mise à un si la tension descends en dessous de ten_min
	int surcharge;        //valeur booléene qui sera mise à un si la tension descends en dessus de ten_max
	float ten_min;       //cf surcharge
	float ten_max;      //cf ten_crit
	float differentiel;  //valeur intermediaire pour le calcul de batt
	float inter1;
	float inter2;
	float pourcentage;
	float Vbat[4];		//tension de la batterie brute (tension mesurée)
	float Batt[4];		//pourcentage de la valeur de la batterie
	float Batt_min;
	int   i;

	//plage dans laquelle le niveau de la batterie se trouve
	//(la batterie est dans la 1ere si elle est comprise entre 0 et 10%)
	int TrancheBatt;

	ten_crit = 0;
	surcharge = 0;
	//ten_min =11.3;
	ten_min =1;
	//ten_max = 15.0;
	ten_max =4;

	// On boucle sur les 4 batteries
	for (i = 0; i < 4; i ++)
	{
		// On récupére la valeur analogique correspondant à la batterie testée
		switch (i)
		{
			case 0: Vbat[i] = analogRead(A1); break;
			case 1: Vbat[i] = analogRead(A2); break;
			case 2: Vbat[i] = analogRead(A3); break;
			case 3: Vbat[i] = analogRead(A4); break;
		}

		// Conversion valeur digitale (0-1023) vers analogique (0-5V)
		Vbat[i] = Vbat[i] * 5.0f / 1023.0f;

		//Serial.println(Vbat[i]);

		// Vérifie si la batterie est en-dessous du seuil critique
		if (Vbat[i]<ten_min)
		{
			ten_crit =1;
			//Serial.println("la tension atteint un niveau critique! \nS'arreter d'urgence!!");
		}

		// Vérifie si la batterie est en surcharge
		if (Vbat[i]>ten_max)
		{
			surcharge =1;
			//Serial.println("la tension atteint un niveau critique! \nDebrancher la prise d'urgence!!");
		}

		// Si la batterie n'est ni en surcharge ni au niveau critique on affiche le niveau en %
		if ((surcharge != 1) && (ten_crit != 1))
		{
			inter1 = Vbat[i]-ten_min;
			inter2 = inter1/(ten_max-ten_min);
			Batt[i] = inter2*100;

		//on déclare la variable de tranche
        int TrancheBatt;



				//détermination des tranches de la batterie
			switch Batt
			{

            case (0<Batt<10):
                TrancheBatt = 0;
                break;

            case (10<Batt<20):
                TrancheBatt = 1;
                break;

             case (20<Batt<30):
                TrancheBatt = 2;
                break;

			 case (30<Batt<40):
                TrancheBatt = 3;
                break;

			 case (40<Batt<50):
                TrancheBatt = 4;
                break;

			 case (50<Batt<60):
                TrancheBatt = 5;
                break;

			 case (60<Batt<70):
                TrancheBatt = 6;
                break;

			 case (70<Batt<80):
                TrancheBatt = 7;
                break;

			 case (80<Batt<90):
                TrancheBatt = 8;
                break;

              case (90<Batt<100):
                TrancheBatt = 9;
                break;


			//Nous allons maintenant programmer les leds du bargraphe en fonction des tranches

			   switch TrancheBatt
			{

            case (0):
                writeRegister(0x01);
                break;

            case (1):
                writeRegister(0x02);
                break;

            case (2):
                writeRegister(0x03);
                break;

            case (3):
                writeRegister(0x04);
                break;

			case (4):
                writeRegister(0x05);
                break;

			case (5):
                writeRegister(0x06);
                break;

			case (6):
                writeRegister(0x07);
                break;

			case (7):
                writeRegister(0x08);
                break;

			case (8):
                writeRegister(0x09);
                break;

            case (9):
                writeRegister(0x01);
                break;


			}


			//Serial.print("le niveau de batterie est de ");
			//Serial.print(TrancheBatt);
			//Serial.println("%");
		}
	}

	// On prend le niveau minimum des 4 batteries
	Batt_min = min (Batt[0], Batt[1]);
	Batt_min = min (Batt_min, Batt[2]);
	Batt_min = min (Batt_min, Batt[3]);

	pourcentage = 90.0f;

	for (i = 9; i >= 0; i --)
	{
		Segment[i] = false;

		if (Batt_min > pourcentage) {   Segment[i] = true;  }

		pourcentage -= 10.0f;

		Serial.print(Segment[i]);
	}
	Serial.println(";");
}

// Equivalent au main sur Arduino
void loop()
{
	if ((millis() - CountSeconde1) > 1000)
	{
		CountSeconde1 = millis();
		Lecture_Vitesse();
	}

	if ((millis() - CountSeconde2) > 500)
	{
		CountSeconde2 = millis();
		TensionBatterie();
	}
}

//fonction configurant le MAX7961
void setup() {
    pinMode(LOAD, OUTPUT);
    digitalWrite(LOAD, HIGH);
    SPI.begin();
    SPI.setBitOrder(MSBFIRST); // bits de poids fort en premier


//fonction permettant de modifier les registres du MAX
void writeRegister(byte thisRegister, byte thisValue) // Écrire dans un registre du MAX7219
{
  // Mettre l'entrée LOAD à l'état bas
  digitalWrite(LOAD, LOW);

  SPI.transfer(thisRegister); // Adresse du registre
  SPI.transfer(thisValue); // Contenu du registre

  // Basculer l'entrée LOAD à l'état haut pour verrouiller et transférer les données
  digitalWrite(LOAD, HIGH);
}


