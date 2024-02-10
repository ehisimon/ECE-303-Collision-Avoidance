#include <Arduino.h>
#include <LiquidCrystal.h>
#include "DHT.h"
//#include "IRremote.h"
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 6
#define SS_PIN 53
#define DHTPIN 10
#define DHTTYPE DHT11
#define ONE 16724175   // Define the key 1
#define TWO 16718055   // Define the key 2
#define THREE 16743045 // Define the key 3
#define FOUR 16716015  // Define the key 4
#define FIVE 16726215  // Define the key 5
#define SIX 16734885   // Define the key 6
#define SEVEN 16728765 // Define the key 7
#define EIGHT 16730805 // Define the key 8
#define NINE 16732845  // Define the key 9
#define ZERO 16738455  // Define the key 0

// instances
// IRrecv irrecv();
MFRC522 mfrc522(SS_PIN, RST_PIN);
DHT dht(DHTPIN, DHTTYPE);

// lcd
LiquidCrystal lcd(1, 30, 40, 38, 36, 9);

// headlights
int headlight1 = 11;
int headlight_intensity;

// variable definitions
const int trigPin = 2;
const int echoPin = 4;
int far = 100;
int safe = 10;
const int m1 = 7;
const int m2 = 5;
long duration;
int distance;
const int water = A0;
const int buzzer = 8;
int checkaccess = 0;

void setup()
{
  // set pinmodes
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(m1, OUTPUT);
  pinMode(m2, OUTPUT);
  pinMode(headlight1, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // ir remote
  //irrecv.enableIRIn();

  // rfid
  SPI.begin();        // Initiate  SPI bus
  mfrc522.PCD_Init(); // Initiate MFRC522

  // initialize dht
  dht.begin();

  // lcd
  pinMode(24, OUTPUT);
  analogWrite(24, 120);
  lcd.begin(16, 2);

  // serial comm
  Serial.begin(9600);
}

void loop()
{
  if (checkaccess == 0)
  {
    // rfid
    //  Look for new cards
    if (!mfrc522.PICC_IsNewCardPresent())
    {
      return;
    }
    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
    {
      return;
    }
    // uid check
    String access = "";
    // byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      access.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      access.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    access.toUpperCase();

    if (access.substring(1) == "A3 9B C7 05")
    {

      // sound buzzer once
      tone(buzzer, 1000);
      delay(1000);
      noTone(buzzer);
      delay(1000);
      checkaccess = 1;
      Serial.println("Access granted");
    }
    else
    {
      tone(buzzer, 1000);
    }
  }
  else
  {
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // object distance
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    String dist = String(distance);
    delay(2000);

    // motor speed
    float motor_speed = map(distance, 0, 100, 0, 255);
    if (distance > far)
    {
      analogWrite(m2, 0);
      analogWrite(m1, 255);
      motor_speed = analogRead(m1);
    }
    if ((distance < far) && (distance >= safe))
    {
      analogWrite(m1, motor_speed);
    }
    if (distance < safe)
    {
      // motor off
      analogWrite(m1, 0);
      analogWrite(m2, 0);
      motor_speed = 0;
    }
    float MSpercent = (motor_speed / 255) * 100;
    String motorS = String(MSpercent);

    // temp sensor
    float temperature = dht.readTemperature();
    String tempS = String(temperature);

    // water lvl sensor
    float water_lvl = analogRead(water);
    String WL = String(water_lvl);

    // print temp, motor speed and water lvl to lcd
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write("TEMP: ");
    lcd.print(tempS);
    lcd.setCursor(0, 1);
    lcd.write("Motor S.: ");
    lcd.print(motorS);
    delay(1000);
    lcd.clear();
    lcd.write("Cool lvl: ");
    lcd.print(WL);

    /*
    //LED - IR remote
    if (irrecv.decode(&results))
    {
      // Serial.println(results.value);
      if (results.value == ONE)
      {                               // Check if the key 1 is pressed
        digitalWrite(headlight1, HIGH); // Turn on the headlight 1
        headlight_intensity = 100;

      }
      else if (results.value == TWO)
      {                                // Check if the key 2 is pressed
        analogWrite(headlight1, 150); // Turn on the headlight 1 (dim)
        headlight_intensity = 50;

      }
      else if (results.value == THREE)
      {                                // Check if the key 3 is pressed
        digitalWrite(headlight1, LOW); // Turn off the headlight 1
        headlight_intensity = 0;

      }
      irrecv.resume(); // Receive the next value
    }*/
    String headL = String(headlight_intensity);
    

    // output to matlab[distance, temp, Mspeed, CoolLvl]
    String output = dist + "," + tempS + "," + motorS + "," + WL;
    // exception handling
    if (distance < 100)
    {
      String out = "00" + output;
      Serial.println(out);
    }
    else
    {
      Serial.println(output);
    }
  }
}