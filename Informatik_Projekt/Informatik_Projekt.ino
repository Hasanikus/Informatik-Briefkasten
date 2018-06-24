#include <SoftwareSerial.h>
#include "DumbServer.h"
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myservo;
long transponder=0; //Speichern der Nummer des Transponders 
int bias=0;         
boolean locked=true;
boolean extra=true;
boolean light=true;   //ist dafür zuständig dass nur einmal hochgezählt wird
boolean extra2=true;  //Während die Tür mit Karte geöffnet wird, geht bis es geschlossen der Client nicht
boolean extra3=true;  //Ist dafür da während die Tür geöffnet wird egal auf welcher Methode, zählt währenddessen der Zähler für Briefeinwürfe nicht hoch
/*long validkey=1778890;*/  //Code der Keycards
long validkey=1403390;
SoftwareSerial esp_serial(3, 2); //pins 3 und 2 sind für die verbindung zwischen wlan-shield und arduino
EspServer esp_server;

int a = 0;          //Hier wird a deklariert. a ist sozusagen die Anzahl der Briefeinwürfe die zum Clienten gesendet wird
int lightPin = A5;  //Pin für Photoresistor

int value;          //der abgelesene Wert vom lightPin

void setup()
{
  Serial.begin(9600);     
  esp_serial.begin(9600);  // zum clienten wird mit einer baudrate von 9600 gesendet

  Serial.println("Starting server...");
  esp_server.begin(&esp_serial, "arduino", "password", 30303); //Server wird auf dem Netzwerk gestartet
  Serial.println("...server is running");

  char ip[16];            
  esp_server.my_ip(ip, 16);   //IP wird ermittelt

  Serial.print("My ip: ");  //IP wird ausgegeben am serial monitor
  Serial.println(ip);
   SPI.begin();           //Verbindung für den Kartenleser.  Serial Peripheral Interface
mfrc522.PCD_Init();
Serial.begin(9600);
myservo.attach(7);  //Servo benutzt pin 7 

}

void loop()
{
value = analogRead(lightPin);  //wert vom pin wird abgelesen

    if (value <= 500){
    light = true;         //erlaubt wieder zugang zur unteren Schleife
  }
  else if (value > 500 && light == true && extra3 == true){
    a++;
    esp_server.println(a);  //Die Zahl für die Anzahl der Briefeinwürfe wird zum CLienten gesendet
    light = false;          //ist dafür dass es nicht wieder in die Schleife geht und nochmal hochzählt
  }

if ( ! mfrc522.PICC_IsNewCardPresent())
{
transponder =0;
bias++;
}
else{
  mfrc522.PICC_ReadCardSerial();    
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
  transponder=((transponder+mfrc522.uid.uidByte[i])*10); 
  }
  bias=0;
}
if(locked==true&&transponder==validkey&&extra==true){//wenn karte rangehalten wird und vorher nicht
  myservo.write(69);            //mit client geöffnet wurde, öffnet er
  locked=false;
  extra2=false;   //verhindert Öffnen/Schließen mit Client
  extra3=false;
}

else if(locked == false&&bias>=2){ //schließen
  myservo.write(156);
  locked = true;
  light = false;
  a = 0;        //beim Schließen wird die Anzahl der Briefeinwürfe wieder auf 0 gesetzt und im Anschluss zum CLienten gesendet
  extra2=true;  //aktiviert wieder Öffnen/Schließen mit Client
  extra3=true;
  esp_server.println(a);
}

Serial.print("Wert:");// Hier werden value, status und code ausgegeben
Serial.println(value);
Serial.println(transponder);
Serial.print("Status:");
Serial.println(locked);

 while(esp_server.available()) {    //Sobald ein Signal vom Clienten gesendet wird
    // Read one line of commands
    String command= esp_server.readStringUntil('\n');  //Liest den command bis \n ab

    if(command == "on" && extra2 == true){   //Wenn command = on und extra2 = true wird tür geöffnet
      myservo.write(69);                     //
      extra = false;                        //extra wird false gesetzt damit die obige schleife nicht mit validkey durchgeführt werden kann
      locked = true;                        //locked wird true gesetzt damit sich es nicht wieder direkt schließt
      extra3=false;
    }
    else if(command == "off" && extra2 == true){//Schließt sich wieder
      extra = true;
      locked = false;
      light = false;
      extra3=true;
    }
  }
}
