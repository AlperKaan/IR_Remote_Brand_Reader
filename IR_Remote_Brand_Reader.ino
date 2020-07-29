/* IR receiver VCC--->3.3v
 *             GND--->GND
 *             OUTPUT--->D5
 *             
 * LCD VCC-->5V         
 *     GND-->GND
 *     SDA-->A4
 *     SCL-->A5
 * LED KONTROL
 *     OUTPUT-->D3
 *     OUTPUT-->D6
*/

/*
 * SD card attached to SPI bus as follows:
 * MOSI - pin 11
 * MISO - pin 12
 * SCK  - pin 13
 * CS   - pin 10 (for MKRZero SD: SDCARD_SS_PIN)
*/
 
#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

#define SD_CS_PIN   10
#define IR_PIN      5
#define _AnimStart  12
#define _DEBUG

LiquidCrystal_I2C lcd(0x27,16,2);
IRrecv irrecv(IR_PIN);

decode_results result;
byte AdmCounter, AnimPos = _AnimStart;
unsigned long mMillis;

void setup()
{
    Serial.begin(9600);
    Wire.begin();
    
    lcd.begin(16,2);
    lcd.backlight();

    if (!SD.begin(SD_CS_PIN)) {
        lcdWrite(0,0,"SD Kart Hatali!");
        while (1);
    }
  
    if (!SD.exists("kodlar.txt")) {
        lcdWrite(0,0,"SD Kart Hatali!");
        while (1);
    }

    lcdWrite(0,0,"Sistem");
    lcdWrite(0,1,"Baslatiliyor");
    
    irrecv.enableIRIn();
    delay(500);

    mMillis = 0;

    lcdWriteAdm();
}

void lcdWriteAdm()
{
    lcd.clear();
    lcdWrite(3,0,"COMPANY NAME1");
    lcdWrite(3,1,"COMPANY NAME2");

    AdmCounter = 0;
}

void lcdWrite(int x, int y, char *str)
{
    lcd.setCursor(x, y); 
    lcd.print(str);
}

void readLn(File myFile, char *rCode, char *rBrand)
{
    unsigned long pos = myFile.position();

    byte PInd;
    char temp[81];
    myFile.read(temp, 80);
    for (byte i = 0; i < 80; i++) {
        if (temp[i] == ';') PInd = i; 
        if (temp[i] == '\n') {
            temp[i] = 0;
            break;
        }
    }

    strncpy(rCode, temp, PInd); rCode[PInd] = 0;
    strcpy(rBrand, temp + PInd + 1);
    rBrand[strlen(rBrand)-1] = 0;

    myFile.seek(pos + strlen(temp) + 1);
    return temp;
}

void checkCode(char *result)
{
    char rCode[16], rBrand[60];
    bool isFind = false;
  
    File myFile;
    myFile = SD.open("kodlar.txt", FILE_READ);
    if (myFile) {
        while (myFile.available()) {
            readLn(myFile, rCode, rBrand);
            if (!strcmp(rCode, result)) {
                isFind = true;
                break;
            }

            if (millis() - mMillis > 500) {
               mMillis = millis();
               lcdWrite(AnimPos,0," ");
               if (AnimPos < 15) AnimPos++; else AnimPos = _AnimStart;
               lcdWrite(AnimPos,0,".");
            }
            
        }
    }
    myFile.close();
    
    lcd.clear();
    lcdWrite(0,0,"0X"); lcdWrite(2,0,result);
    if (isFind) 
         lcdWrite(0,1,rBrand); 
    else lcdWrite(0,1,"Bulunamadi");
#ifdef _DEBUG
    Serial.print(result); Serial.print(";");  if (isFind) Serial.println(rBrand); else Serial.println("Bulunamadi");
#endif    
}

void loop() 
{
    if (irrecv.decode(&result)) {

        char resstr[12];
        sprintf(resstr,"%04X",result.value>>16);
        sprintf(resstr + strlen(resstr),"%04X", result.value);
      
#ifdef _DEBUG
        Serial.println(result.value, HEX);
        Serial.println(resstr);
#endif      
        if (strcmp(resstr,"FFFFFFFF")) {
             lcdWrite(0, 0,"Kod araniyor");
             lcdWrite(AnimPos,0,".");
             lcdWrite(0, 1,"Lutfen Bekleyin!");

             mMillis = millis();
             checkCode(resstr);
        }

        irrecv.resume(); 
        
        AdmCounter = 0;
    } 
  
    delay(500);

    if (++AdmCounter >= 10) lcdWriteAdm();
}
