/*
  PROJET KART IUT : 2017-2018
*/

#include "EnableInterrupt.h"

//--------------------------------------------------------//

// Définition Pinout µC
#define INPUT_CAPTEUR_VITESSE  2 // PD2, pin4  // Entrée dédiée au capteur de vitesse 

#define FEEDBACK_TX       3 // PD3, pin5
#define FEEDBACK_RX       4 // PD4, pin6

#define LUM_BAT         5 // PD5, pin11 // Entrée OE (output enable) du MAX6971 pour le bargraph
#define LUM_V         6 // PD6, pin12 // Entrée OE (output enable) du MAX6971 pour les afficheurs 7 segments

#define AFF_DATA        8 // PB0, pin14 // Entrée CLK du MAX6971
#define AFF_CLK         9 // PB1, pin15 // Entrée DIN du MAX6971
#define AFF_LAT         10  // PB2, pin16 // Entrée LE du MAX6971

#define ADC1          A1  // PC1, pin24 // Entrée analogique récupération tension batterie 1
#define ADC2          A2  // PC2, pin25 // Entrée analogique récupération tension batterie 2
#define ADC3          A3  // PC3, pin26 // Entrée analogique récupération tension batterie 3
#define ADC4          A3  // PC4, pin27 // Entrée analogique récupération tension batterie 4

// Définitions pour le temps
unsigned long TempsEcoule = 0;
unsigned long Count0, Count1;

// Définitions pour la vitesse
#define DIAMETRE_ROUE_MM    866.0f                    // diameter roue en mm
#define DIAMETRE_ROUE_M     (float) ((float)DIAMETRE_ROUE_MM / 1000.0f) // diameter roue en m

typedef struct StructVitesse {
  unsigned char     Actuelle;
  unsigned char     MemoCountDemiTour;
  unsigned char     NbDetectionParSec;
  volatile unsigned char  CountDemiTour;
} StruVitesse;

StruVitesse Vitesse;

// Définitions pour les batteries
#define DefNbBatteries      4     // Nombre de batteries

#define GAIN_AMPLI_OP     0.1f    // Gain de l'ampli op AD628 (Rext1 = 10k, Rext2= ∞)

#define TENSION_CRITIQUE    11.3f   // Tension critique de la batterie                      
#define TENSION_SURCHARGE   15.0f   // Tension de surcharge de la batterie
                        // Seuils au delà desquels la batterie peut être endommagée

#define TENSION_CRITIQUE_LUE  (float) (TENSION_CRITIQUE * GAIN_AMPLI_OP)  // Tension critique de la batterie lue par le µC
#define TENSION_SURCHARGE_LUE (float) (TENSION_SURCHARGE * GAIN_AMPLI_OP) // Tension surcharge de la batterie lue par le µC

typedef struct StructBatterie {
  unsigned int  Lecture_AN;   // Lecture sur le convertisseur Analogique/Numérique sur 1023
  float     Tension_lue;  // Tension batterie en V
  float     Pourcentage;  // Pourcentage correspondant à la tension dans la plage utile
  boolean     Surcharge;    // La batterie a dépassé son seuil de charge maximum
  boolean     Critique;   // La batterie est en dessous de son seuil de tension minimal
} StruBatterie;

StruBatterie  Batterie[DefNbBatteries];
unsigned char PourcentageMini;

// Définitions pour l'affichage
#define TEMPS_ON_AFFICHEUR    0.01f                       // Définit le temps ON de l'afficheur (de 0 à 1)
#define LUMINOSITE_AFFICHEUR  (unsigned char) ((1.0f - TEMPS_ON_AFFICHEUR) * 255) // Définit la luminosité de l'afficheur sur 255
#define TEMPS_ON_BARGRAPH   0.01f                       // Définit le temps ON du bargraph
#define LUMINOSITE_BARGRAPH   (unsigned char) ((1.0f - TEMPS_ON_BARGRAPH) * 255)  // Définit la luminosité du bargraph sur 255

const byte AFFICHEUR_7_SEGMENTS[] = {
  0b00111111,
  0b00000110,
  0b01011011,
  0b01001111,
  0b01100110,
  0b01101101,
  0b01111101,
  0b00000111,
  0b01111111,
  0b01101111,
  0b01110111,
  0b01111100,
  0b00111001,
  0b01011110,
  0b01111001,
  0b01110001
};

/* // Non inversé
const unsigned int  BARGRAPH[] = {
  0b0000000000,
  0b0000000001,
  0b0000000011,
  0b0000000111,
  0b0000001111,
  0b0000011111,
  0b0000111111,
  0b0001111111,
  0b0011111111,
  0b0111111111,
  0b1111111111
};
*/

const unsigned int  BARGRAPH[] = {
  0b0000000000,
  0b1000000000,
  0b1100000000,
  0b1110000000,
  0b1111000000,
  0b1111100000,
  0b1111110000,
  0b1111111000,
  0b1111111100,
  0b1111111110,
  0b1111111111
};

typedef struct StructAffichage {
  unsigned char Digit_1;
  unsigned char Digit_2;
  unsigned int  Bargraph;
} StruAffichage;

StruAffichage Affichage;

//--------------------------------------------------------//

// Fonction d'interruption correspondant au passage de l'état bas à l'état haut de l'entrée liée au capteur de vitesse
void CAPTEUR_INT (void)
{
  Vitesse.CountDemiTour++;
}

// Fonction d'initialisation pour le microcontrôleur
void setup() {
  
  // Initialise la liaison série à 9600 bauds
  Serial.begin(9600);
  
  // Configure l'entrée logique dédiée à la récupération du capteur de vitesse en entrée pull up (mise à 1 par défault)
  pinMode(INPUT_CAPTEUR_VITESSE, INPUT);
  
  // Configure l'interruption qui se déclenchera à chaque fois que l'entrée capteur passe de l'état bas à l'état haut
  enableInterrupt(INPUT_CAPTEUR_VITESSE, CAPTEUR_INT, RISING);
  
  // Configure les leds de feedback
  pinMode(FEEDBACK_TX, OUTPUT);
  digitalWrite(FEEDBACK_TX, HIGH);
  pinMode(FEEDBACK_RX, OUTPUT);
  digitalWrite(FEEDBACK_RX, HIGH);
  
  // Configure les sorties pour le pilotage des MAX6971
  pinMode(AFF_LAT, OUTPUT);
  pinMode(AFF_CLK, OUTPUT);
  pinMode(AFF_DATA, OUTPUT);
  
  pinMode(LUM_BAT, OUTPUT);
  pinMode(LUM_V, OUTPUT);
  analogWrite(LUM_BAT, LUMINOSITE_BARGRAPH);
  analogWrite(LUM_V, LUMINOSITE_AFFICHEUR);
}

// Fonction permettant le calcul de la vitesse en fonction du nombre d'impulsion par secondes
void Lecture_Vitesse()
{
  char tempCount = 0;
  
  tempCount = Vitesse.CountDemiTour;
  
  Vitesse.NbDetectionParSec = (tempCount - Vitesse.MemoCountDemiTour);

  // Calcul détaillé
  //NbTourParSec = (float) (NbDetectionParSec / 2);
  //NbTourParHeure = (NbTourParSec * 3600);   // tour/heure
  //Vitesse = NbTourParHeure * DIAMETRE_ROUE_M; // m/heure
  //Vitesse = Vitesse / 1000; //km/heure

  // Calcul optimisé
  Vitesse.Actuelle = (unsigned char) ((float) Vitesse.NbDetectionParSec * 1.8f * DIAMETRE_ROUE_M);

  Vitesse.MemoCountDemiTour = tempCount;
}

// Fonction permettant de récupérer l'état des différentes batteries
void TensionBatterie()
{
  float     Calcul;
  
  int       i;
  
  // On boucle sur les 4 batteries
  for (i = 0; i < 4; i ++)
  {
    Batterie[i].Surcharge = 0;
    Batterie[i].Critique = 0;

    // On récupére la valeur analogique correspondant à la batterie testée
    switch (i)
    {
      case 0: Batterie[i].Lecture_AN = analogRead(ADC1); break;
      case 1: Batterie[i].Lecture_AN = analogRead(ADC2); break;
      case 2: Batterie[i].Lecture_AN = analogRead(ADC3); break;
      case 3: Batterie[i].Lecture_AN = analogRead(ADC4); break;
    }
    
    // Conversion valeur digitale (0-1023) vers analogique (0-5V)
    Batterie[i].Tension_lue = Batterie[i].Lecture_AN * 5.0f / 1023.0f;
    
    // Vérifie si la batterie est en-dessous du seuil critique
    if (Batterie[i].Tension_lue < TENSION_CRITIQUE_LUE)
    {
      Batterie[i].Critique = 1;
      Batterie[i].Pourcentage = 0.0f;
    }
    
    // Vérifie si la batterie est en surcharge
    if (Batterie[i].Tension_lue > TENSION_SURCHARGE_LUE)
    {
      Batterie[i].Surcharge = 1;
      Batterie[i].Pourcentage = 100.0f;
    }
    
    // Si la batterie n'est ni en surcharge ni au niveau critique on affiche le niveau en %
    if ((Batterie[i].Surcharge != 1) && (Batterie[i].Critique != 1))
    {
      Calcul = Batterie[i].Tension_lue - TENSION_CRITIQUE_LUE;
      Calcul = Calcul / (TENSION_SURCHARGE_LUE - TENSION_CRITIQUE_LUE);
      Batterie[i].Pourcentage = Calcul * 100;
    }
  }
  
  // On prend le niveau minimum des 4 batteries pour l'état du bargraph
  PourcentageMini = min(Batterie[0].Pourcentage, Batterie[1].Pourcentage);
  PourcentageMini = min(PourcentageMini, Batterie[2].Pourcentage);
  PourcentageMini = min(PourcentageMini, Batterie[3].Pourcentage);
  
  
}

// Fonction permettant de tester l'afficheur et le bargraph
void Demo()
{
  unsigned int temp;
  
  Vitesse.Actuelle = (unsigned int) (analogRead(A5) * 100.0f / 1023.0f);
  
  PourcentageMini = (unsigned int) (analogRead(A5) * 100.0f / 1023.0f);
}

// Fonction permettant de convertir les infos pour pouvoir les afficher
void ConvertInfos()
{
  // Convertit la vitesse en donnée pour les afficheurs 7 segments
  if (Vitesse.Actuelle >= 100)
  {
    // Premier Digit
    Affichage.Digit_1 = AFFICHEUR_7_SEGMENTS[9];
      
    // Deuxième Digit
    Affichage.Digit_2 = AFFICHEUR_7_SEGMENTS[9];
  }
  else
  {
    // Premier Digit
    Affichage.Digit_1 = AFFICHEUR_7_SEGMENTS[(unsigned char) (Vitesse.Actuelle % 10)];
      
    // Deuxième Digit
    Affichage.Digit_2 = AFFICHEUR_7_SEGMENTS[(unsigned char) (Vitesse.Actuelle / 10.0f)];
  }
  
  // Convertit l'information de la batterie la plsu faible pour le bargraph
  Affichage.Bargraph = BARGRAPH[PourcentageMini/10];
}

// Fonction permettant de modifier les registres du MAX6971
void writeMAX6971()
{
  unsigned char i;
  
  shiftOut(AFF_DATA, AFF_CLK, MSBFIRST, (unsigned char) (Affichage.Bargraph >> 8));
  shiftOut(AFF_DATA, AFF_CLK, MSBFIRST, (unsigned char) Affichage.Bargraph);
  shiftOut(AFF_DATA, AFF_CLK, MSBFIRST, Affichage.Digit_1);
  shiftOut(AFF_DATA, AFF_CLK, MSBFIRST, Affichage.Digit_2);
  
  digitalWrite(AFF_LAT, HIGH);
  digitalWrite(AFF_LAT, LOW);
}

// Fonction permettant d'afficher les infos sur le port série
void AfficheInfosSerie()
{
  int i;
  
  Serial.println("----------------------------------");
  Serial.print("Temps écoulé depuis le démarrage : ");
  Serial.print(TempsEcoule);
  Serial.print("s");
  Serial.println(",\n");
  
  Serial.print("Vitesse actuelle : ");
  Serial.print(Vitesse.Actuelle);
  Serial.println(",\n");

  Serial.println("Niveaux Batteries :");
  
  for (i = 0; i < 4; i ++)
  {
    Serial.print("La Batterie ");
    Serial.print(i + 1);

    if (Batterie[i].Critique)   { Serial.println(" a atteint un niveau critique! \nS'arreter d'urgence!!\n"); }
    else if (Batterie[i].Surcharge) { Serial.println(" a atteint sa charge max \nVeuillez stopper la charge\n");  }
    else
    {
      Serial.print(" est à ");
      Serial.print(Batterie[i].Pourcentage);
      Serial.println("%\n");
    }
  }
  
  Serial.print("Etat Bargraph : ");
  Serial.print(Affichage.Bargraph, BIN);
  Serial.println("\n");
}

// Equivalent au main sur Arduino
void loop ()
{
  if ((millis() - Count0) > 1000)
  {
    Count0 = millis();
    TempsEcoule ++;
  }
  
  if ((millis() - Count1) > 200)
  {
    Count1 = millis();
    
    // Récupère info vitesse
    Lecture_Vitesse();
    
    // Récupère info tension batterie
    TensionBatterie();
    
    // Mode Démo : permet de tester l'afficheur et le bargraph
    Demo();
    
    // Convertit les infos pour pouvoir les afficher
    ConvertInfos();
    
    // Affiche les infos sur l'afficheur 7 segments et le bargraph
    writeMAX6971();
    
    // Affiche les infos sur le port série
    AfficheInfosSerie();
  }
}
