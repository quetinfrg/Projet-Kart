#define LUM_BAT         5 // PD5, pin11 // Entrée OE (output enable) du MAX6971 pour le bargraph
#define LUM_V         6 // PD6, pin12 // Entrée OE (output enable) du MAX6971 pour les afficheurs 7 segments

#define AFF_DATA        8 // PB0, pin14 // Entrée CLK du MAX6971
#define AFF_CLK         9 // PB1, pin15 // Entrée DIN du MAX6971
#define AFF_LAT         10  // PB2, pin16 // Entrée LE du MAX6971

#define TEMPS_ON_AFFICHEUR    0.05f                       // Définit le temps ON de l'afficheur (de 0 à 1)
#define LUMINOSITE_AFFICHEUR  (unsigned char) ((1.0f - TEMPS_ON_AFFICHEUR) * 255) // Définit la luminosité de l'afficheur sur 255
#define TEMPS_ON_BARGRAPH     0.05f                       // Définit le temps ON du bargraph
#define LUMINOSITE_BARGRAPH   (unsigned char) ((1.0f - TEMPS_ON_BARGRAPH) * 255)  // Définit la luminosité du bargraph sur 255

#define LED_RX  4
#define LED_TX  3

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


unsigned long Count0, Count1;
unsigned char data[3] = {0xFF, 0, 0};
byte cpt = 0;
boolean flash = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(AFF_LAT, OUTPUT);
  pinMode(AFF_CLK, OUTPUT);
  pinMode(AFF_DATA, OUTPUT);

  pinMode(LUM_BAT, OUTPUT);
  pinMode(LUM_V, OUTPUT);
  analogWrite(LUM_BAT, LUMINOSITE_BARGRAPH);
  analogWrite(LUM_V, LUMINOSITE_AFFICHEUR);

  // initialisation de la led
  pinMode(LED_RX, OUTPUT);
  pinMode(LED_TX, OUTPUT);
  digitalWrite(LED_RX, HIGH);
  digitalWrite(LED_TX, HIGH);

  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

void ConvertInfos()
{
  // Convertit la vitesse en donnée pour les afficheurs 7 segments
  if (data[1] >= 100)
  {
    // Premier Digit
    Affichage.Digit_1 = AFFICHEUR_7_SEGMENTS[9];

    // Deuxième Digit
    Affichage.Digit_2 = AFFICHEUR_7_SEGMENTS[9];
  }
  else
  {
    // Premier Digit
    Affichage.Digit_1 = AFFICHEUR_7_SEGMENTS[(unsigned char) (data[1] % 10)];

    // Deuxième Digit
    Affichage.Digit_2 = AFFICHEUR_7_SEGMENTS[(unsigned char) (data[1] / 10.0f)];
  }

  // Convertit l'information de la batterie la plus faible pour le bargraph
  Affichage.Bargraph = BARGRAPH[data[2] / 10];
}

// Fonction permettant de modifier les registres du MAX6971
void writeMAX6971()
{
  shiftOut(AFF_DATA, AFF_CLK, MSBFIRST, (unsigned char) (Affichage.Bargraph >> 8));
  shiftOut(AFF_DATA, AFF_CLK, MSBFIRST, (unsigned char) Affichage.Bargraph);
  shiftOut(AFF_DATA, AFF_CLK, MSBFIRST, Affichage.Digit_1);
  shiftOut(AFF_DATA, AFF_CLK, MSBFIRST, Affichage.Digit_2);

  digitalWrite(AFF_LAT, HIGH);
  digitalWrite(AFF_LAT, LOW);
}

// Fonction permettant de tester l'afficheur et le bargraph
void Demo()
{
  unsigned int temp;
  
  data[1] = (unsigned int) (analogRead(A5) * 100.0f / 1023.0f);
  
  data[2] = (unsigned int) (analogRead(A0) * 100.0f / 1023.0f);
}

void loop() {
  unsigned long realtime = millis();
  // put your main code here, to run repeatedly:

  if (flash == true)
  {
    flash = false;
    digitalWrite(LED_TX, LOW);
    Count1 = realtime;
  }
  else if ((realtime - Count1) > 10)
  {
    digitalWrite(LED_TX, HIGH);
  }

  Demo();

  if ((realtime - Count0) > 100)
  {
    Count0 = realtime;
    // Convertit les infos pour pouvoir les afficher
    ConvertInfos();

    // Affiche les infos sur l'afficheur 7 segments et le bargraph
    writeMAX6971();

    // Envoi des données à la carte stand
    Serial.write(data[0]);
    Serial.write(data[1]);
    Serial.write(data[2]);
    flash = true;
  }
}
