#include <LiquidCrystal_I2C.h>
#include <dht.h>
#include <RTClib.h>

#pragma region Változók és szenzorok

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD objektum létrehozása, az LCD címének, a karaktereinek és a sorainak megadása
RTC_DS3231 rtc; // Létrehozzuk az RTC objektumot
dht DHT; // DHT-11 objektum létrehozása

#define dataPin 22 // DHT11 PIN
#define echoPin 52 // HC-SR04 ECHOPIN
#define trigPin 53 //HC-SR04 TRIGPIN
#define fanPinOne A0
#define fanPinTwo A1

const int buzzer = 24;
int temp; // változó a hőmérsékletnek
int hum; // változó a pratartalomnak
int second; // változó az RTC másodpercének
int hour; // változó az RTC órájának
int minute; // változó az RTC percének
#pragma endregion

void setup()
{
  pinMode(Relay_Pin, OUTPUT); //Relé PIN megadása kimenetként (22-es digitalpin)
  pinMode(fanPinOne,OUTPUT);
  pinMode(fanPinTwo,OUTPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600); // initialize serial
  lcd.init(); // LCD bekapcsolása
  lcd.backlight(); // LCD háttérvilágítás
  rtc.begin(); // RTC bekapcsolása
  delay(500); // fél másodperc várakozás
}

void loop()
{
  LCDTime(); // idő megjelenítése az LCD-n
  TemperatureAndHumidity(); //hőmérséklet és páratartalaom megjelenítése az LCD-nű
  FanMode();
  Buzz();
  
  if (Serial.available())
  {
    char input = Serial.read();
    if (input == 'u') UpdateRTC(); //RTC frissétése a soros monitoron keresztül az 'u' megnyomása utána

  }
}

#pragma region Metódusok
void TemperatureAndHumidity() //Hőmérsékelt és páratartalom megjelenítése
{
  int readData = DHT.read11(dataPin); //A szenzor adatainak a beolvasása
  temp = DHT.temperature; //Hőmérséklet értékének a kiolvasása
  hum = DHT.humidity; //Páratartalom értékének a kiolvasása

  //A kiolvasott értékek megjelenítése az LCD-n
  lcd.setCursor(4, 0); // A kurzor beállítása
  lcd.print("Temp: ");
  lcd.print(temp); //Hőmérsékelt kiiratása
  lcd.print((char)223); //A ° jel megjelenítése
  lcd.print('C');
  lcd.setCursor(4, 1); // A kurzor beállítása
  lcd.print("Hum: ");
  lcd.print(hum); // A páratartalom kiiratása 
  lcd.print(" %");

  DelayAndClear(2000);
}
void UpdateRTC() //RTC frissitése a soros monitoron keresztül
{
  lcd.clear(); // LCD törlése
  lcd.setCursor(0, 0); //Kurzor beállítása
  lcd.print("Edit Mode..."); //Kiratjuk a szerkesztő módot

  const char txt[6][15] =
  {
  "year [4-digit]", "month [1~12]", "day [1~31]",
  "hours [0~23]", "minutes [0~59]", "seconds [0~59]"
  }; // A dátum és az idő formátumának a megadása

  String str = ""; //Egy STRING objektum létrehozása
  long newDate[6]; //Tömb az új dátumnak

  while (Serial.available())
  {
    Serial.read(); // Soros puffer törlése
  }

  for (int i = 0; i < 6; i++) //Bekérjük a soros monitoron keresztül az új dátumot és időt
  {

    Serial.print("Enter ");
    Serial.print(txt[i]);
    Serial.print(": ");

    while (!Serial.available()) // Várunk az adatokra
    {
      ;
    }

    str = Serial.readString(); // Adatbeolvasás
    newDate[i] = str.toInt(); // A beolvasott adatokat számmá konvertáljuk és mentjük

    Serial.println(newDate[i]); // A beolvasott adatok megjelenítése
  }

  rtc.adjust(DateTime(newDate[0], newDate[1], newDate[2], newDate[3], newDate[4], newDate[5])); //Az RTC frissítése a megadott adatok alapján
  Serial.println("RTC Updated!"); // Visszajelzés a sikeres frissítésről
}
void LCDTime() //A dátum és az idő kiiratása az LCD-re
{
  DateTime rtcTime = rtc.now(); //DateTime objektum létrehozása

  second = rtcTime.second(); //másodperc
  minute = rtcTime.minute(); //perc
  hour = rtcTime.hour(); // óra
  int dayoftheweek = rtcTime.dayOfTheWeek(); //a hét napja
  int day = rtcTime.day(); // nap
  int month = rtcTime.month(); // hónap
  int year = rtcTime.year(); // év

  lcd.setCursor(0, 0); //kurzor beállítása

  // dátum és az idő kiiratása év-hónap-nap és hét napja rövidítve formátumban

  lcd.print(year);
  lcd.print(".");
  if (month < 10) lcd.print("0"); // ha a szám kisebb, mint 10 elé írunk egy nullát (0)
  lcd.print(month);
  lcd.print(".");
  if (day < 10) lcd.print("0"); // ha a szám kisebb, mint 10 elé írunk egy nullát (0)
  lcd.print(day);

  lcd.setCursor(13, 0); //kurzor beállítása

  const char dayInWords[7][4] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" }; // a hét napjai rövidítve, angol formátumban (I2C LCD nem tud ékezetet kezelni)
  lcd.print(dayInWords[dayoftheweek]); // a hét napjai rövidítve kiiratása

  lcd.setCursor(0, 1); //kurzor beállítása

  // az óra-perc-másodperc kiiratása  
  if (hour < 10) lcd.print("0"); // ha a szám kisebb, mint 10 elé írunk egy nullát (0)
  lcd.print(hour);
  lcd.print(":");
  if (minute < 10) lcd.print("0"); // ha a szám kisebb, mint 10 elé írunk egy nullát (0)
  lcd.print(minute);
  lcd.print(":");
  if (second < 10) lcd.print("0"); // ha a szám kisebb, mint 10 elé írunk egy nullát (0)
  lcd.print(second);
  DelayAndClear(2000); //Késleltetés és a kijelző letörlése
}

void FanMode()
{
   digitalWrite(fanPinOne, HIGH);
  if (temp>=25)
    {
      digitalWrite(fanPinTwo, HIGH);
    }

   if (28 >= temp)
    {
      digitalWrite(fanPinTwo, LOW);
    }

}

void Buzz()
{
  if(temp>=30)
  {
    tone(buzzer, 2500);
  }
  else
  {
    noTone(buzzer); 
  }
}

void DelayAndClear(int ms) //Késleltetés és az LCD törlése megadott milisecundom értékkel
{
  delay(ms);
  lcd.clear();
}
