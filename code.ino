//Scenariusz
//1. Butelki są na jednym poziomie i poruszają się jednostajnym tempem.
//2. Butelki są na dwuch poziomach i poruszają się jednostajnym tempem
//3. Butelki są tylko na górnym poziomie.
//4. Butelki nie idą, ale zlicza dolny poziom.
//5. Super wymieszany



//Algorytm.
//

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

#ifndef STASSID
#define STASSID "HMI_Enterence"
#define STAPSK  "trudnehasloduzymiliterami"
#endif

int sensor_A = D0;
int sensor_B = D1;
int sensor_C = D2;
int sensor_D = D3;

int led_A = D5;
int led_B = D6;
int led_C = D7;
int led_D = D8;

const String sensor_on_enterance = "13"; //Na wejściu
const String sensor_on_exit = "14"; //Na wyjściu
const int    numer_licznika   =  13 ;


int koncowka_ip = 4 + numer_licznika; //lenistwo do 100 są rezerwowane na liczniki. Czyli możemy zainstalować 60 urządzeń.



long interval = 10000; //Co ile ma wysyłać stan //10 sekund.
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

const String server_adress = "http://10.186.10.2/test/"; //Aktualnie jest serwer
const String php_key_target = "key.php";
const String field_for_key = "?key=";
const String php_target = "insert.php"; //nazwa skryptu uzupelniajacego. Trzeba pamiętać o "?"
const String field_one = "?nazwa=";
const String field_two = "&dane=";

int wykryty_tryb = 0; //0 -- start; 1 -- jedna warstwa dolna; 2 -- dwie warstwy

int flaga_gora_wejscie = 0;
int flaga_dol_wejscie = 0;
int flaga_gora_wyjscie = 0;
int flaga_dol_wyjscie = 0;

static unsigned int licznik_wejsc = 0;
static unsigned int pamiec_licznika_wejsc = 0;
static unsigned int licznik_wyjsc = 0;
static unsigned int pamiec_licznika_wyjsc = 0;

//Dla tych dwóch stanów trzeba użyć wspólnego debounca. Tak aby jeden debounc nie pozwalał załączyć się zbyt szybko dwum stanom czujnika.

void ICACHE_RAM_ATTR gora_up_wejscie() { //Ta funkcja jest realizowana przez drugi wątek esp.
  {
    //dodałem debouncing bo zliczało nieraz 3 krotnie więcej niż powinno.
    static unsigned long last_interrupt_time_for_A = 0;
    static unsigned long interrupt_time_for_A = millis();
    // If interrupts come faster than 1000ms, assume it's a bounce and ignore
    if (interrupt_time_for_A - last_interrupt_time_for_A > 300 and flaga_gora_wejscie != 1) //Trzeba upewnić się że był stan niski. Przynajmiej przez 300 ms.
    {
      flaga_gora_wejscie = 1;
      licznik_wejsc++;
    }
    last_interrupt_time_for_A = interrupt_time_for_A;
  }
}



void ICACHE_RAM_ATTR gora_down_wejscie() { //Ta funkcja jest realizowana przez drugi wątek esp.
  {
    //dodałem debouncing bo zliczało nieraz 3 krotnie więcej niż powinno.
    static unsigned long last_interrupt_time_for_A = 0;
    static unsigned long interrupt_time_for_A = millis();
    if (interrupt_time_for_A - last_interrupt_time_for_A > 300)
    {
      flaga_gora_wejscie = 0;
    }
    last_interrupt_time_for_A = interrupt_time_for_A;
  }
}
/////////////////////////////////////TU DUŁ WEJŚCIA
void ICACHE_RAM_ATTR dol_up_wejscie() { //Ta funkcja jest realizowana przez drugi wątek esp.
  {
    //dodałem debouncing bo zliczało nieraz 3 krotnie więcej niż powinno.
    static unsigned long last_interrupt_time_for_B = 0;
    static unsigned long interrupt_time_for_B = millis();

    if (interrupt_time_for_B - last_interrupt_time_for_B > 2000 and flaga_gora_wejscie == 0)
    { if (pamiec_licznika_wejsc < licznik_wejsc) {
        flaga_dol_wejscie = 1;
        licznik_wejsc++;
        pamiec_licznika_wejsc = licznik_wejsc;
      }
    }
    last_interrupt_time_for_B = interrupt_time_for_B;
  }
}

void ICACHE_RAM_ATTR dol_up_wejscie() { //Ta funkcja jest realizowana przez drugi wątek esp.
  {
    //dodałem debouncing bo zliczało nieraz 3 krotnie więcej niż powinno.
    static unsigned long last_interrupt_time_for_B = 0;
    static unsigned long interrupt_time_for_B = millis();

    if (interrupt_time_for_B - last_interrupt_time_for_B > 300)
    {
      flaga_dol_wejscie = 1;
      licznik_wejsc++;
    }
    last_interrupt_time_for_B = interrupt_time_for_B;
  }
}

void ICACHE_RAM_ATTR gora_up_wyjscie() { //Ta funkcja jest realizowana przez drugi wątek esp.
  {
    //dodałem debouncing bo zliczało nieraz 3 krotnie więcej niż powinno.
    static unsigned long last_interrupt_time_for_C = 0;
    static unsigned long interrupt_time_for_C = millis();
    // If interrupts come faster than 1000ms, assume it's a bounce and ignore
    if (interrupt_time_for_C - last_interrupt_time_for_C > 300 and flaga_gora_wyjscie != 1) //Trzeba upewnić się że był stan niski. Przynajmiej przez 300 ms.
    {
      flaga_gora_wyjscie = 1;
      licznik_wyjsc++;
    }
    last_interrupt_time_for_C = interrupt_time_for_C;
  }
}



void ICACHE_RAM_ATTR gora_down_wyjscie() { //Ta funkcja jest realizowana przez drugi wątek esp.
  {
    //dodałem debouncing bo zliczało nieraz 3 krotnie więcej niż powinno.
    static unsigned long last_interrupt_time_for_C = 0;
    static unsigned long interrupt_time_for_C = millis();
    if (interrupt_time_for_C - last_interrupt_time_for_C > 300)
    {
      flaga_gora_wyjscie = 0;
    }
    last_interrupt_time_for_C = interrupt_time_for_C;
  }
}
/////////////////////////////////////TU DUŁ WEJŚCIA
void ICACHE_RAM_ATTR dol_up_wyjscie() { //Ta funkcja jest realizowana przez drugi wątek esp.
  {
    //dodałem debouncing bo zliczało nieraz 3 krotnie więcej niż powinno.
    static unsigned long last_interrupt_time_for_D = 0;
    static unsigned long interrupt_time_for_D = millis();

    if (interrupt_time_for_D - last_interrupt_time_for_D > 2000 and flaga_gora_wyjscie == 0)
    { if (pamiec_licznika_wyjsc < licznik_wyjsc) {
        flaga_dol_wyjscie = 1;
        licznik_wyjsc++;
        pamiec_licznika_wyjsc = licznik_wyjsc;
      }
    }
    last_interrupt_time_for_D = interrupt_time_for_D;
  }
}

void ICACHE_RAM_ATTR dol_up_wyjscie() { //Ta funkcja jest realizowana przez drugi wątek esp.
  {
    //dodałem debouncing bo zliczało nieraz 3 krotnie więcej niż powinno.
    static unsigned long last_interrupt_time_for_D = 0;
    static unsigned long interrupt_time_for_D = millis();

    if (interrupt_time_for_D - last_interrupt_time_for_D > 300)
    {
      flaga_dol_wyjscie = 1;
      licznik_wyjsc++;
    }
    last_interrupt_time_for_D = interrupt_time_for_D;
  }
}






void setup() {

  pinMode(led_neta, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(czujnik, INPUT); //nie moze byc pullup
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  Serial.println();
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  Serial.print("yo im ");
  Serial.println(numer_licznika);
  Serial.println("https://github.com/MarcinanBarbarzynca/ESP8266-PNP-Sensor-counter");
  WiFi.mode(WIFI_STA);

  //Statyczne IP Tu musisz zmienić na swoje ustawienia sieci
  IPAddress ip(10, 186, 10, koncowka_ip); //34-->1 35-->2 36-->3 37--> 4
  IPAddress gateway(10, 186, 10, 3);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, gateway, subnet);


  WiFiMulti.addAP(STASSID, STAPSK);


  attachInterrupt(digitalPinToInterrupt(czujnik), dodaj_l_jeden, FALLING); //Ten interrupt musi znaleźć się na końcu całego setupa


}

void loop() {
  if (digitalRead(sensor_A) == HIGH) {
    digitalWrite(led_A, HIGH);
  } else {
    digitalWrite(led_A, LOW);
  }
  if (digitalRead(sensor_B) == HIGH) {
    digitalWrite(led_B, HIGH);
  } else {
    digitalWrite(led_B, LOW);
  }

  if (digitalRead(sensor_C) == HIGH) {
    digitalWrite(led_C, HIGH);
  } else {
    digitalWrite(led_C, LOW);
  }

  if (digitalRead(sensor_D) == HIGH) {
    digitalWrite(led_D, HIGH);
  } else {
    digitalWrite(led_D, LOW);
  }



  if ((WiFiMulti.run() == WL_CONNECTED)) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      Serial.print("[HTTP] begin... \n");
      WiFiClient client;
      HTTPClient http;


      int key = random(1000000, 9999999);
      String url_for_key = server_adress + php_key_target + field_for_key + key;
      Serial.println(url_for_key);
      Serial.println();

      String complete_adress_enterance = server_adress + php_target + field_one + sensor_on_enterance + field_two + licznik_wejsc;
      String complete_adress_exit = server_adress + php_target + field_one + sensor_on_exit + field_two + licznik_wyjsc;
      Serial.println(complete_adress);
      Serial.println();


      if (http.begin(client, url_for_key)) {
        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTP] GET... code: %d\n", httpCode);

          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();
            Serial.print("Otrzymany klucz: ");
            Serial.println(payload);
            http.end(); //Zamykanie połączenia a potem ponowne otwieranie w celu przesłania danych.
            if (payload.toInt() == key) {
              if (http.begin(client, complete_adress_enterance)) {
                Serial.print("[HTTP] GET...\n");
                // start connection and send HTTP header
                int httpCode = http.GET();

                // httpCode will be negative on error
                if (httpCode > 0) {
                  // HTTP header has been send and Server response header has been handled
                  Serial.printf("[HTTP] GET... code: %d\n", httpCode);

                  // file found at server
                  if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                    String payload = http.getString();
                    licznik_impulsow = 0;
                    if (flip_flop_neta == 0) {
                      digitalWrite(led_neta, HIGH);
                      flip_flop_neta = 1;
                    } else {
                      digitalWrite(led_neta, LOW);
                      flip_flop_neta = 0;
                    }
                    Serial.println(payload);
                    http.begin(client, complete_adress_exit);
                    http.end();
                  }
                } else {
                  Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
                }


              } else {
                Serial.printf("[HTTP} Unable to connect\n");
              }
            }


          }
        } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
      } else {
        Serial.printf("[HTTP} Unable to connect\n");
      }






    }
  }

  //delay(10000);
}
