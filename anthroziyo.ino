#include <FS.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "Free_Fonts.h"
#include <TFT_eSPI.h>
#include <TFT_eWidget.h>
#include <HX711_ADC.h>
#include <EEPROM.h>
#include "SRF05.h"
#include <BluetoothSerial.h>
// #include <Adafruit_Thermal.h>

// #if !defined(CONFIG_BT_SPP_ENABLED)
// #error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
// #endif

#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false

// Keyboard start position, key sizes and spacing
#define KEY_X1 67 // Centre of key
#define KEY_Y1 118 
#define KEY_W1 65 // Width and height
#define KEY_H1 35
#define KEY_SPACING_X1 5 // X and Y gap
#define KEY_SPACING_Y1 5
#define KEY_TEXTSIZE1 1

// Keypad start position, key sizes and spacing
#define KEY_X 105 // Centre of key
#define KEY_Y 118 
#define KEY_W 110 // Width and height
#define KEY_H 35
#define KEY_SPACING_X 25 // X and Y gap
#define KEY_SPACING_Y 5
#define KEY_TEXTSIZE 1

// Numeric display box size and location
#define DISP_X 5
#define DISP_Y 40
#define DISP_W 469 //320
#define DISP_H 47 //50
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_GREEN //TFT_YELLOW // TFT_CYAN
#define RefreshTime 200

// Using two fonts since numbers are nice when bold
#define LABEL1_FONT &FreeSansOblique12pt7b // Key label font 1
#define LABEL2_FONT &FreeSansBold12pt7b    // Key label font 2

#define BUTTON_W 200
#define BUTTON_H 60

#define BUTTON_W1 80
#define BUTTON_H1 60

#define BUTTON_W2 120
#define BUTTON_H2 60

#define BUTTON_W3 160
#define BUTTON_H3 60

#define BOY 0
#define GIRL 1

#define INDONESIA 0
#define ENGLISH 1

#define PAUD 0
#define POSYANDU 1
#define PUSKESMAS 2

// Number length, buffer for storing it and character index
#define NUM_LEN 10
#define NUM_LEN2 5
#define NUM_LEN3 19
#define NUM_LEN4 8
#define NUM_LEN5 14

// We have a status line for messages
#define STATUS_X 240 // Centred on this
#define STATUS_Y 90 //95
#define n_data 50 //4

char bodyWeight[NUM_LEN2 + 1] = "", numberBuffer[NUM_LEN + 1] = "", tanggalLahir[NUM_LEN + 1] = "", calibrate[NUM_LEN2 + 1] = "", name[NUM_LEN3 + 1] = "", address[NUM_LEN3 + 1] = "", 
phoneNumber[NUM_LEN5 + 1] = "", apiKey[NUM_LEN4 + 1] = "", placeName[NUM_LEN3 + 1] = "";

const PROGMEM int buzzer = 12, trigger = 33, echo = 32, HX711_dout = 26, HX711_sck = 27; //mcu > HX711 sck pin
const char * ssid = "anthroziyo"; const char * password = "anthro123";

unsigned long previousMillis = 0, currentMillis, interval = 2000, weightInterval = 2000, prevMillis = 0, lastFrame;
const PROGMEM int calVal_eepromAdress = 0, tareOffsetVal_eepromAdress = 4;
uint8_t numberIndex = 0, indexWeight = 0, indexLahir = 0, indexCalibrate = 0, indexName = 0, indexAddress = 0, indexWA = 0, indexAPI = 0, indexPlace = 0;

boolean language, gender, method;
bool connected;
int i_data = 0, j_data = 0, k_data = 0, displayState = 0, VSensor = 25, type, usia = 0, days = 0, usiaHari, usiaBulan, usiaBulanTemp, regresi, simpangan = 0, currentDate, currentMonth, currentYear, 
tempTime = 0, birthDate, birthMonth, birthYear, inputValue = 0, ref_voltage = 5.0;

float jarak, waktu, newDistance, ave, newAve, newAver, newAverage, tinggiBadan, adcValue = 0.0, voltage = 0.0, data[n_data], alpha = 0.2, filteredDistance, length, beratBadan, bb, sd_TBU, m_TBU, 
zscore_TBU, l_BBU, m_BBU, s_BBU, zscore_BBU, l_BBTB, m_BBTB, s_BBTB, zscore_BBTB, imt, l_IMTU, m_IMTU, s_IMTU, zscore_IMTU;
String numberBufferr, kategori_TBU, kategori_TBUU, kategori_BBU, kategori_BBUU, kategori_BBTB, kategori_BBTBB, kategori_IMTU, kategori_IMTUU, urlWA, pesanWA, typee, genderr, pesanSpreadsheets, pesanPrinter;
String placeNameStr, nameStr, addressStr;
// String GOOGLE_SCRIPT_ID = "AKfycbzbUBTtBNx5L0cS87ZQDQFTX12rriRp5hqzZiN74qX3R8PmDW2NBxD0EqPGC2zyz1MyaA";
String GOOGLE_SCRIPT_ID = "AKfycbyxobfLA27Pz9CeLjC5HfPihkHBFnInEWZZz0fQqmEL3fCKjvPXenOvHsOrPHIq-4qfRg"; //lama
// String GOOGLE_SCRIPT_ID = "AKfycbyzNW4w1lXX60vTx8cJHqKTSgQKC4FV4XNgxxRFG3pJejt0pcVeDq_-aWR7CyWzdjzdcA"; //baru
// String GOOGLE_SCRIPT_ID = "AKfycbzyr22Jfo_JQC9BcupDnjzLAlcDxi1du3Mxcxc1Zj2Apfv8uRAJ_M0Ypj6If8HsYc0mBg";

TFT_eSPI tft = TFT_eSPI();

ButtonWidget btnSTART = ButtonWidget(&tft);
ButtonWidget btnCAL = ButtonWidget(&tft);
ButtonWidget btnWA = ButtonWidget(&tft);
ButtonWidget btnBOY = ButtonWidget(&tft);
ButtonWidget btnGIRL = ButtonWidget(&tft);
ButtonWidget btnRCM = ButtonWidget(&tft);
ButtonWidget btnSTD = ButtonWidget(&tft);
ButtonWidget btnCALCULATE = ButtonWidget(&tft);
ButtonWidget btnRES = ButtonWidget(&tft);
ButtonWidget btnPRINT = ButtonWidget(&tft);
ButtonWidget btnBackCAL = ButtonWidget(&tft);
ButtonWidget btnSAVE = ButtonWidget(&tft);
ButtonWidget btnINDO = ButtonWidget(&tft);
ButtonWidget btnENG = ButtonWidget(&tft);
ButtonWidget btnSetOFFSET = ButtonWidget(&tft);
ButtonWidget btnSaveWEIGHT = ButtonWidget(&tft);
ButtonWidget btnPAUD = ButtonWidget(&tft);
ButtonWidget btnPOSYANDU = ButtonWidget(&tft);
ButtonWidget btnPUSKESMAS = ButtonWidget(&tft);
ButtonWidget btnUbahWIFI = ButtonWidget(&tft);
ButtonWidget btnBackUbahWIFI = ButtonWidget(&tft);

ButtonWidget* btn[] = {&btnSTART,&btnCAL,&btnWA,&btnBOY,&btnGIRL,&btnRCM,&btnSTD,&btnCALCULATE,
                       &btnRES,&btnPRINT,&btnBackCAL,&btnSAVE,&btnINDO,&btnENG,&btnSetOFFSET,&btnSaveWEIGHT,
                       &btnPAUD,&btnPOSYANDU,&btnPUSKESMAS, &btnUbahWIFI, &btnBackUbahWIFI};
uint8_t buttonCount = sizeof(btn) / sizeof(btn[0]);


// Create 15 keys for the keypad
char keyLabel[15][5] PROGMEM = {"New", "Del", "Send", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-", "0", "Back" }, 
keyLabel2[15][5] PROGMEM = {"New", "Del", "Send", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "0", "Back" },
keyLabel3[15][5] PROGMEM = {"New", "Del", "Send", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "0", "-" },
keyboardLabel[30][5] PROGMEM = {"Q", "W", "E", "R", "T", "Y",
                          "U", "I", "O", "P",  "A", "S",
                          "D", "F", "G", "H", "J", "K",
                          "L", "Z", "X", "C", "V", "Del",
                          "B", "N", "M", " ", "New", "Send"};
uint16_t keyColor[15] PROGMEM = {TFT_RED, TFT_BROWN, TFT_DARKGREEN,
                        TFT_BLUE, TFT_BLUE, TFT_BLUE,
                        TFT_BLUE, TFT_BLUE, TFT_BLUE,
                        TFT_BLUE, TFT_BLUE, TFT_BLUE,
                        TFT_BLUE, TFT_BLUE, TFT_DARKGREY
                        }, 
keyColor3[15] PROGMEM = {TFT_RED, TFT_BROWN, TFT_DARKGREEN,
                TFT_BLUE, TFT_BLUE, TFT_BLUE,
                TFT_BLUE, TFT_BLUE, TFT_BLUE,
                TFT_BLUE, TFT_BLUE, TFT_BLUE,
                TFT_BLUE, TFT_BLUE, TFT_BLUE
                }, 
keyboardColor[30] PROGMEM = {TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BLUE,
                    TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BLUE,
                    TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BLUE,
                    TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_BROWN,
                    TFT_BLUE, TFT_BLUE, TFT_BLUE, TFT_GREY, TFT_RED, TFT_DARKGREEN
                    };

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[15], key1[15],key2[15],key3[15], key4[15], key5[15], keyboard[30], keyboard1[30], keyboard2[30];

WiFiClientSecure client;
SRF05 SRF(trigger, echo);
HX711_ADC LoadCell(HX711_dout, HX711_sck);
BluetoothSerial SerialBT;
// Adafruit_Thermal printer(&SerialBT);
String slaveName = "RPP02N", myName = "ESP32-BT-Master";

// #include "variabel.h"
#include "TBU.h"
#include "BBU.h"
#include "BBTB.h"
#include "IMTU.h"
// #include "tambahan.h"

//-------------Init BUTTON------------
void initButtonINDO(){
  touch_calibrate();
  tft.fillScreen(TFT_CYAN);
  tft.setCursor(145,tft.height()/2-70); //60
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.print("CHOOSE LANGUAGE");
  // tft.setFreeFont(&FreeSansBold9pt7b);
  uint16_t x5 = (tft.width() - BUTTON_W) / 2;
  uint16_t y5 = tft.height() / 2 - BUTTON_H - 10;
  btnINDO.initButtonUL(x5, y5+40, BUTTON_W, BUTTON_H, TFT_DARKGREEN, TFT_WHITE, TFT_DARKGREEN, "Indonesia", 1);
  btnINDO.setPressAction(btnINDO_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnINDO.drawSmoothButton(false, 3, TFT_BLACK);
}

void initButtonENG(){
  // tft.setFreeFont(&FreeSansBold9pt7b);
  uint16_t x5 = (tft.width() - BUTTON_W) / 2;
  uint16_t y5 = tft.height() / 2 - BUTTON_H + 60;
  btnENG.initButtonUL(x5, y5+40, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "English", 1);
  btnENG.setPressAction(btnENG_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnENG.drawSmoothButton(false, 3, TFT_BLACK);
}

void initButtonSTART(){
  tft.setFreeFont(&FreeSansBold9pt7b);
  if(millis()>50){
    if (bacaTegangan() > 3.40){
      tft.fillScreen(TFT_CYAN);
      tft.setCursor(150,tft.height()/2-100);
      tft.setTextColor(TFT_BLACK);
      tft.setTextSize(1);
      
      if(language == ENGLISH){
        tft.print("Battery = ");
      }else if(language == INDONESIA){
        tft.print("Baterai = ");
      }
      
      tft.setCursor(240,tft.height()/2-100);
      tft.print(bacaTegangan());
      tft.setCursor(280,tft.height()/2-100);
      tft.print("V");
      // Serial.println(bacaTegangan());
    }
    else if ( bacaTegangan() >= 3.00 && bacaTegangan() <= 3.40){
      tft.fillScreen(TFT_YELLOW);
      tft.setCursor(150,tft.height()/2-100);
      tft.setTextColor(TFT_BLACK);
      tft.setTextSize(1);
      if(language == ENGLISH){
        tft.print("Battery = ");
      }else if(language == INDONESIA){
        tft.print("Baterai = ");
      }
      
      tft.setCursor(240,tft.height()/2-100); //160
      tft.print(bacaTegangan());
      tft.setCursor(280,tft.height()/2-100); //220
      tft.print("V");
      //Serial.println(bacaTegangan());
    }
    else if( bacaTegangan() < 3.00){
      tft.fillScreen(TFT_RED);
      tft.setCursor(110,tft.height()/2-100); //60
      tft.setTextColor(TFT_BLACK);
      tft.setTextSize(1);
      if(language == ENGLISH){
        tft.print("Battery = ");
        tft.setCursor(210,tft.height()/2-100); //160
        tft.print("LOW BATTERY !!");
      }else if(language == INDONESIA){
        tft.print("Baterai = ");
        tft.setCursor(210,tft.height()/2-100); //160
        tft.print("BATERAI LEMAH !!");
      }
      
      //Serial.println(bacaTegangan());    
    }
  }

  uint16_t x = (tft.width() - BUTTON_W) / 2;
  uint16_t y = tft.height() / 2 - BUTTON_H - 50;//10;
  if(language == ENGLISH){
    btnSTART.initButtonUL(x, y+40, BUTTON_W, BUTTON_H, TFT_DARKGREEN, TFT_WHITE, TFT_DARKGREEN, "Start", 1);
  }else if(language == INDONESIA){
    btnSTART.initButtonUL(x, y+40, BUTTON_W, BUTTON_H, TFT_DARKGREEN, TFT_WHITE, TFT_DARKGREEN, "Mulai", 1);
  }
  
  btnSTART.setPressAction(btnSTART_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnSTART.drawSmoothButton(false, 3, TFT_BLACK);
}

void initButtonCAL(){
  tft.setFreeFont(&FreeSansBold9pt7b);
  uint16_t x = (tft.width() - BUTTON_W) / 2;
  uint16_t y = tft.height() / 2 - BUTTON_H + 15; //60;
  if(language == ENGLISH){
    btnCAL.initButtonUL(x, y+40, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Calibrate", 1);
  }else if(language == INDONESIA){
    btnCAL.initButtonUL(x, y+40, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Kalibrasi", 1);
  }
  
  btnCAL.setPressAction(btnCAL_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnCAL.drawSmoothButton(false, 3, TFT_BLACK);
}

void initButtonWA(){
  tft.setFreeFont(&FreeSansBold9pt7b);
  uint16_t x = (tft.width() - BUTTON_W) / 2;
  uint16_t y = tft.height() / 2 - BUTTON_H + 80; //60;
  // if(language == ENGLISH){
  btnWA.initButtonUL(x, y+40, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "WhatsApp", 1);
  // }else if(language == INDONESIA){
  //   btnWA.initButtonUL(x, y+40, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Kalibrasi", 1);
  // }
  
  btnWA.setPressAction(btnWA_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnWA.drawSmoothButton(false, 3, TFT_BLACK);
}

void initButtonBOY(){
  tft.fillScreen(TFT_CYAN);
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  if(language == ENGLISH){
    tft.setCursor(150,tft.height()/2-70); //60
    tft.print("CHOOSE GENDER !");
  }else if(language == INDONESIA){
    tft.setCursor(138,tft.height()/2-70); //60
    tft.print("PILIH JENIS KELAMIN !");
  }
  
  uint16_t x1 = (tft.width() - BUTTON_W) / 2;
  uint16_t y1 = tft.height() / 2 - BUTTON_H - 10;
  if(language == ENGLISH){
    btnBOY.initButtonUL(x1, y1+40, BUTTON_W, BUTTON_H1, TFT_DARKGREEN, TFT_WHITE, TFT_DARKGREEN, "Boy", 1);
  }else if(language == INDONESIA){
    btnBOY.initButtonUL(x1, y1+40, BUTTON_W, BUTTON_H1, TFT_DARKGREEN, TFT_WHITE, TFT_DARKGREEN, "Laki-laki", 1);
  }
  
  btnBOY.setPressAction(btnBOY_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnBOY.drawSmoothButton(false, 3, TFT_BLACK);
}

void initButtonGIRL(){
  uint16_t x1 = (tft.width() - BUTTON_W) / 2;
  uint16_t y1 = tft.height() / 2 - BUTTON_H + 60;
  if(language == ENGLISH){
    btnGIRL.initButtonUL(x1, y1+40, BUTTON_W, BUTTON_H1, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Girl", 1);
  }else if(language == INDONESIA){
    btnGIRL.initButtonUL(x1, y1+40, BUTTON_W, BUTTON_H1, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Perempuan", 1);
  }
  btnGIRL.setPressAction(btnGIRL_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnGIRL.drawSmoothButton(false, 3, TFT_BLACK);
}

void initButtonTYPE(){
  tft.fillScreen(TFT_CYAN);
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  // if(language == ENGLISH){
  //   tft.setCursor(150,tft.height()/2-70); //60
  //   tft.print("CHOOSE GENDER !");
  // }else if(language == INDONESIA){
  //   tft.setCursor(138,tft.height()/2-70); //60
  //   tft.print("PILIH JENIS KELAMIN !");
  // }
  
  uint16_t x1 = (tft.width() - BUTTON_W) / 2;
  uint16_t y1 = tft.height() / 2 - BUTTON_H - 40;
  // if(language == ENGLISH){
    btnPAUD.initButtonUL(x1, y1, BUTTON_W, BUTTON_H1, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Paud", 1);
  // }else if(language == INDONESIA){
  //   btnPAUD.initButtonUL(x1, y1+40, BUTTON_W, BUTTON_H1, TFT_DARKGREEN, TFT_WHITE, TFT_DARKGREEN, "Laki-laki", 1);
  // }
  btnPAUD.setPressAction(btnPAUD_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnPAUD.drawSmoothButton(false, 3, TFT_BLACK);

  y1 = tft.height() / 2 - BUTTON_H + 30;
  // if(language == ENGLISH){
    btnPOSYANDU.initButtonUL(x1, y1, BUTTON_W, BUTTON_H1, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Posyandu", 1);
  // }else if(language == INDONESIA){
  //   btnGIRL.initButtonUL(x1, y1+40, BUTTON_W, BUTTON_H1, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Perempuan", 1);
  // }
  btnPOSYANDU.setPressAction(btnPOSYANDU_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnPOSYANDU.drawSmoothButton(false, 3, TFT_BLACK);

  y1 = tft.height() / 2 - BUTTON_H + 100;
  // if(language == ENGLISH){
    btnPUSKESMAS.initButtonUL(x1, y1, BUTTON_W, BUTTON_H1, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Puskesmas", 1);
  // }else if(language == INDONESIA){
  //   btnGIRL.initButtonUL(x1, y1+40, BUTTON_W, BUTTON_H1, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Perempuan", 1);
  // }
  btnPUSKESMAS.setPressAction(btnPUSKESMAS_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnPUSKESMAS.drawSmoothButton(false, 3, TFT_BLACK);
}

void initButtonWEIGHT(){
  // tft.fillScreen(TFT_BROWN);
  tft.fillScreen(TFT_CYAN);
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setCursor(30,40);
  tft.setTextSize(1);
  if(language == ENGLISH){
    tft.print("BODY WEIGHT MEASUREMENT");
  }else if(language == INDONESIA){
    tft.print("PENGUKURAN BERAT BADAN");
  }
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setTextSize(1);
  
  uint16_t x = (tft.width() - BUTTON_W2) / 2 - 80;
  uint16_t y = tft.height() / 2 - BUTTON_H2 + 80;
  if(language == ENGLISH){
    btnSetOFFSET.initButtonUL(x, y, BUTTON_W2, BUTTON_H2, TFT_WHITE, TFT_RED, TFT_WHITE, "Set to 0", 1);
  }else if(language == INDONESIA){
    btnSetOFFSET.initButtonUL(x, y, BUTTON_W2, BUTTON_H2, TFT_WHITE, TFT_RED, TFT_WHITE, "Atur ke 0", 1);
  }

  btnSetOFFSET.setPressAction(btnSetOFFSET_pressAction);
  btnSetOFFSET.drawSmoothButton(false, 3, TFT_BLACK);

  x = (tft.width() - BUTTON_W2) / 2 + 80;
  y = tft.height() / 2 - BUTTON_H2 + 80;
  if(language == ENGLISH){
    btnSaveWEIGHT.initButtonUL(x, y, BUTTON_W2, BUTTON_H2, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Save", 1);
  }else if(language == INDONESIA){
    btnSaveWEIGHT.initButtonUL(x, y, BUTTON_W2, BUTTON_H2, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Simpan", 1);
  }
  
  btnSaveWEIGHT.setPressAction(btnSaveWEIGHT_pressAction);
  //btnR.setReleaseAction(btnR_releaseAction);
  btnSaveWEIGHT.drawSmoothButton(false, 3, TFT_BLACK);
}

void initButtonCALCU(){
  uint16_t x = (tft.width() - BUTTON_W3) / 2;
  uint16_t y = tft.height() / 2;
  if(language == ENGLISH){
    btnCALCULATE.initButtonUL(x, y-20, BUTTON_W3, BUTTON_H3, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Calculate", 1);
  }else if(language == INDONESIA){
    btnCALCULATE.initButtonUL(x, y-20, BUTTON_W3, BUTTON_H3, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Kalkulasi", 1);
  }
  
  btnCALCULATE.setPressAction(btnCALCULATE_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnCALCULATE.drawSmoothButton(false, 3, TFT_BLACK);
}

void initButtonsRES(){
  uint16_t x = tft.width()/2+40;
  uint16_t y = 40;
  btnRES.initButtonUL(x, y, BUTTON_W3-20, BUTTON_H, TFT_RED, TFT_RED, TFT_WHITE, "SAVE", 1);
  btnRES.setPressAction(btnRES_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnRES.drawSmoothButton(false, 3, TFT_BLACK);

  x = tft.width()/2+40;
  y = 110;
  btnPRINT.initButtonUL(x, y, BUTTON_W3-20, BUTTON_H, TFT_DARKGREEN, TFT_DARKGREEN, TFT_WHITE, "PRINT", 1);
  btnPRINT.setPressAction(btnPRINT_pressAction);
  //btnPRINT.setReleaseAction(btnPRINT_releaseAction);
  btnPRINT.drawSmoothButton(false, 3, TFT_BLACK);
}

void initButtonSAVE(){
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setTextSize(1);
  
  uint16_t x = (tft.width() - BUTTON_W2) / 2 - 80;
  uint16_t y = tft.height() / 2 - BUTTON_H2 + 80;
  if(language == ENGLISH){
    btnBackCAL.initButtonUL(x, y, BUTTON_W2, BUTTON_H2, TFT_WHITE, TFT_RED, TFT_WHITE, "Back", 1);
  }else if(language == INDONESIA){
    btnBackCAL.initButtonUL(x, y, BUTTON_W2, BUTTON_H2, TFT_WHITE, TFT_RED, TFT_WHITE, "Kembali", 1);
  }

  btnBackCAL.setPressAction(btnBackCAL_pressAction);
  btnBackCAL.drawSmoothButton(false, 3, TFT_BLACK);

  x = (tft.width() - BUTTON_W2) / 2 + 80;
  y = tft.height() / 2 - BUTTON_H2 + 80;
  if(language == ENGLISH){
    btnSAVE.initButtonUL(x, y, BUTTON_W2, BUTTON_H2, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Save", 1);
  }else if(language == INDONESIA){
    btnSAVE.initButtonUL(x, y, BUTTON_W2, BUTTON_H2, TFT_WHITE, TFT_DARKGREEN, TFT_WHITE, "Simpan", 1);
  }
  
  btnSAVE.setPressAction(btnSave_pressAction);
  //btnR.setReleaseAction(btnR_releaseAction);
  btnSAVE.drawSmoothButton(false, 3, TFT_BLACK);
}

//-------------BUTTON PressAction----------
void btnINDO_pressAction(void){
  if(btnINDO.justPressed()){
    //Serial.println("INDONESIA");
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.fillScreen(TFT_CYAN);
    tft.setCursor(tft.width()/2-85, tft.height()/2);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    tft.print("Indonesia");
    delay(2000);

    language = INDONESIA;
    initButtonSTART();
    initButtonCAL();
    initButtonWA();
    displayState = 1;
  }
}

void btnENG_pressAction(void){
  if(btnENG.justPressed()){
    //Serial.println("ENGLISH");
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.fillScreen(TFT_CYAN);
    tft.setCursor(tft.width()/2-65, tft.height()/2);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    tft.print("English");
    delay(2000);

    language = ENGLISH;
    initButtonSTART();
    initButtonCAL();
    initButtonWA();
    displayState = 1;
  }
}
//----------BARUUU---------

void btnSTART_pressAction(void){
  if(btnSTART.justPressed()){
    tft.fillScreen(TFT_CYAN);
    tft.setCursor(tft.width()/2-85,tft.height()/2);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    if(language == ENGLISH){
      tft.print("Starting.");
      delay(1000);
      tft.setCursor(tft.width()/2-85,tft.height()/2);
      tft.print("Starting..");
      delay(1000);
      tft.setCursor(tft.width()/2-85,tft.height()/2);
      tft.print("Starting...");
      delay(1000);
    }else if(language == INDONESIA){
      tft.print("Memulai.");
      delay(1000);
      tft.setCursor(tft.width()/2-85,tft.height()/2);
      tft.print("Memulai..");
      delay(1000);
      tft.setCursor(tft.width()/2-85,tft.height()/2);
      tft.print("Memulai...");
      delay(1000);
    }

    initButtonBOY();
    initButtonGIRL();
    displayState = 2;
  }
}

void btnCAL_pressAction(void){
  if(btnCAL.justPressed()){
    tft.fillScreen(TFT_CYAN);
    tft.setCursor(100,tft.height()/2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_BLACK);
    if(language == ENGLISH){
      tft.print("Current Offset Value :");
    }else if(language == INDONESIA){
      tft.print("Nilai Simpangan Saat ini :");
    }
    
    tft.setTextSize(1);
    tft.setCursor(tft.width()/2+90,tft.height()/2);
    tft.print(simpangan);
    delay(3000);

    tft.fillRect(0, 0, 480, 320, TFT_CYAN);
    tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
    tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);
    drawKeypadCAL();
    displayState = 18;
  }
}

void btnUbahWIFI_pressAction(void){
  if(btnUbahWIFI.justPressed()){
    displayState = 7;
    drawKeypadWA();
  }
}

void btnBackUbahWIFI_pressAction(void){
  if(btnBackUbahWIFI.justPressed()){
    initButtonSTART();
    initButtonCAL();
    initButtonWA();
    displayState = 1;
  }
}

void btnWA_pressAction(void){
  if(btnWA.justPressed()){
    EEPROM.get(100,phoneNumber);
    EEPROM.get(200,apiKey);

    displayState = 21;
    initButtonUbahWIFI();
    // displayState = 7;
    // drawKeypadWA();
  }
}

void initButtonUbahWIFI(){
  tft.fillScreen(TFT_CYAN);
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setTextSize(1);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(100, 80);
  tft.print("CURRENT WIFI INFORMATION");
  tft.setCursor(35, 140);
  tft.print("Phone");
  tft.setCursor(tft.width()/2-100, 140);
  tft.print(":");
  tft.setCursor(tft.width()/2-90, 140);
  tft.print(phoneNumber);
  tft.setCursor(35, 170);
  tft.print("API Key");
  tft.setCursor(tft.width()/2-100, 170);
  tft.print(":");
  tft.setCursor(tft.width()/2-90, 170);
  tft.print(apiKey);


  uint16_t x = tft.width()/2-160;
  uint16_t y = tft.height() - 70; //80; //110;
  btnBackUbahWIFI.initButtonUL(x, y, BUTTON_W3-10, BUTTON_H-15, TFT_BLUE, TFT_BLUE, TFT_WHITE, "BACK", 1);
  btnBackUbahWIFI.setPressAction(btnBackUbahWIFI_pressAction);
  //btnDELETE.setReleaseAction(btnDELETE_releaseAction);
  btnBackUbahWIFI.drawSmoothButton(false, 3, TFT_BLACK);

  x = tft.width()/2+20; //tft.width()/2+25;
  // y = tft.height() - 80; //30;
  btnUbahWIFI.initButtonUL(x, y, BUTTON_W3-10, BUTTON_H-15, TFT_RED, TFT_RED, TFT_WHITE, "CHANGE", 1);
  btnUbahWIFI.setPressAction(btnUbahWIFI_pressAction);
  //btnSTART.setReleaseAction(btnSTART_releaseAction);
  btnUbahWIFI.drawSmoothButton(false, 3, TFT_BLACK);
}

void btnBOY_pressAction(void){
  if(btnBOY.justPressed()){
    //Serial.println("BOY");
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.fillScreen(TFT_CYAN);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    if(language == ENGLISH){
      tft.setCursor(tft.width()/2-35, tft.height()/2);
      tft.print("Boy");
    }else if(language == INDONESIA){
      tft.setCursor(tft.width()/2-80, tft.height()/2);
      tft.print("Laki-laki");
    }
    delay(2000);

    gender = BOY;
    displayState = 3;
    initButtonTYPE();
  }
}

void btnGIRL_pressAction(void){
  if(btnGIRL.justPressed()){
    //Serial.println("GIRL");
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.fillScreen(TFT_CYAN);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    if(language == ENGLISH){
      tft.setCursor(tft.width()/2-35, tft.height()/2);
      tft.print("Girl");
    }else if(language == INDONESIA){
      tft.setCursor(tft.width()/2-100, tft.height()/2);
      tft.print("Perempuan");
    }
    delay(2000);

    gender = GIRL;
    displayState = 3;
    initButtonTYPE();
  }
}

void btnPAUD_pressAction(void){
  if(btnPAUD.justPressed()){
    //Serial.println("BOY");
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.fillScreen(TFT_CYAN);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    // if(language == ENGLISH){
      tft.setCursor(tft.width()/2-45, tft.height()/2);
      tft.print("Paud");
    delay(2000);

    type = PAUD;
    displayState = 4;
    // drawKeypadWEIGHT();
    drawKeyboard1();
  }
}

void btnPOSYANDU_pressAction(void){
  if(btnPOSYANDU.justPressed()){
    //Serial.println("BOY");
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.fillScreen(TFT_CYAN);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    // if(language == ENGLISH){
      tft.setCursor(tft.width()/2-90, tft.height()/2);
      tft.print("Posyandu");
    // }else if(language == INDONESIA){
    //   tft.setCursor(tft.width()/2-80, tft.height()/2);
    //   tft.print("Laki-laki");
    // }
    delay(2000);

    type = POSYANDU;
    displayState = 4;
    // drawKeypadWEIGHT();
    drawKeyboard1();
  }
}

void btnPUSKESMAS_pressAction(void){
  if(btnPUSKESMAS.justPressed()){
    //Serial.println("BOY");
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.fillScreen(TFT_CYAN);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    // if(language == ENGLISH){
      tft.setCursor(tft.width()/2-100, tft.height()/2);
      tft.print("Puskesmas");
    // }else if(language == INDONESIA){
    //   tft.setCursor(tft.width()/2-80, tft.height()/2);
    //   tft.print("Laki-laki");
    // }
    delay(2000);

    type = PUSKESMAS;
    displayState = 4;
    drawKeyboard1();
  }
}

void btnCALCULATE_pressAction(void){
  if(btnCALCULATE.justPressed()){
    tft.fillScreen(TFT_CYAN);
    tft.setTextColor(TFT_BLACK);
    // tft.setFreeFont(LABEL2_FONT);
    // tft.setFreeFont(&FreeSansBold9pt7b);
    tft.setFreeFont(LABEL2_FONT);
    tft.setTextSize(1);
        
    tft.setCursor(60,tft.height()/2);
    if(language == ENGLISH){
      tft.print("Height (cm) : ");
    }else if(language == INDONESIA){
      tft.print("Tinggi (cm) : ");
    }
 
    // if(usiaBulan>24 && method == RECUMBENT){
    //   tinggiBadan = newAverage + 0.7 + simpangan;
    // }
    // else if(usiaBulan<24 && method == STANDING){
    //   tinggiBadan = newAverage - 0.7 + simpangan;
    // }
    // else{
      tinggiBadan = newAverage;
    // }

    //Serial.println(tinggiBadan);
    tft.setCursor(tft.width()/2,tft.height()/2);
    tft.setTextSize(2);
    tft.print(tinggiBadan);

    usiaBulan = usiaBulanTemp;
    length = tinggiBadan;
    beratBadan = bb;
    // beratBadan = atof(bodyWeight);

    calculate_TBU();
    calculate_BBU();
    calculate_BBTB();
    calculate_IMTU();

    displayState = 14;
    delay(2000);
  }
}

void btnRES_pressAction(void){
  if(btnRES.justPressed()){
    ESP.restart();
  }
}

void btnPRINT_pressAction(void){
  SerialBT.begin(myName, true);
  // tft.fillScreen(TFT_CYAN);
  // //Serial.printf("The device \"%s\" started in master mode, make sure slave BT device is on!\n", myName.c_str());
  // tft.setCursor(tft.width()/2 - 140, tft.height()/2);
  // tft.setTextColor(TFT_RED);
  // if(language == ENGLISH){
  //   tft.print("Please Turn On Printer");
  // }else if(language == INDONESIA){
  //   tft.print("Mohon Nyalakan Printer");
  // }
  // delay(4000);
  connected = SerialBT.connect(slaveName);
  //Serial.printf("Connecting to slave BT device named \"%s\"\n", slaveName.c_str());

  if(connected) { //Serial.println("Connected Successfully!");
  } else {
    while(!SerialBT.connected(10000)) { 
      //Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app.");
    }
  }
  sendPrinter();
}

void btnSetOFFSET_pressAction(void){
  if(btnSetOFFSET.justPressed()){
    refreshOffsetValueAndSaveToEEprom();
    tft.setCursor(tft.width()/2-100,tft.height()/2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_BLACK);
    if(language == ENGLISH){
      tft.print("Set Offset Value to 0");
      delay(1000);
    }else if(language == INDONESIA){
      tft.print("Atur Simpangan ke 0");
      delay(1000);
    }
    
    initButtonWEIGHT();
    displayState = 12;
    // tft.fillRect(0, 0, 480, 320, TFT_BLUE);
    // tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
    // tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);
    // drawKeypadCAL();

    // displayState = 13;
  }
}

void btnSaveWEIGHT_pressAction(void){
  if(btnSaveWEIGHT.justPressed()){
    tft.fillScreen(TFT_CYAN);
    tft.setCursor(155,tft.height()/2);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    tft.print(bb, 1);
    tft.setCursor(tft.width()/2+40, tft.height()/2);
    tft.print("kg");
    delay(2000);
    tft.fillScreen(TFT_CYAN);
    tft.setCursor(30,40);
    tft.setTextSize(1);
    if(language == ENGLISH){
      tft.print("Height Measurement");
      tft.setCursor(tft.width()/2-135,tft.height()/2+80);
      tft.setTextSize(1);
      tft.setTextColor(TFT_BLACK);
      tft.print("CLICK TO START CALCULATE");
    }else if(language == INDONESIA){
      tft.print("Pengukuran Tinggi Badan");
      tft.setCursor(tft.width()/2-135,tft.height()/2+80);
      tft.setTextSize(1);
      tft.setTextColor(TFT_BLACK);
      tft.print("KLIK UNTUK MULAI KALKULASI");
      tft.setCursor(50,tft.height()/2+130);
      tft.setTextColor(TFT_RED);
      // tft.print("Tinggi Tidak Berubah-ubah = Wi-Fi Belum Connect");
    }
    
    initButtonCALCU();
    displayState = 13;
    movingAverage_Clear();
    // movingAverage1_Clear();
  }
}

void btnBackCAL_pressAction(void){
  if(btnBackCAL.justPressed()){
    tft.fillRect(0, 0, 480, 320, TFT_CYAN);
    tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
    tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);
    drawKeypadCAL();
    displayState = 18;
  }
}

void btnSave_pressAction(void){
  if(btnSAVE.justPressed()){
    delay(1000);
    ESP.restart();
  }
}


float bacaTegangan(){
  inputValue = analogRead(VSensor);
  adcValue = (inputValue * ref_voltage)/1024.0;
  Serial.print("Input Voltage = ");
  Serial.println(voltage,2);

  return adcValue;
}


//-----------------Draw Keypad-----------------
void drawKeypadCAL(){
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(LABEL2_FONT);
  tft.setTextSize(1);
  if(language == ENGLISH){
    tft.setCursor(55, 30);
    tft.print("Enter Offset Value");
    tft.setCursor(280, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(ex: 0.75)");
  }else if(language == INDONESIA){
    tft.setCursor(40, 30);
    tft.print("Masukkan Simpangan");
    tft.setCursor(312, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(cth: 0.75)");
  }
  
  // Draw the keys
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 3; col++) {
      uint8_t b = col + row * 3;

      if (b < 3 || b == 14) tft.setFreeFont(LABEL1_FONT);
      else tft.setFreeFont(LABEL2_FONT);

      key3[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                        KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        KEY_W, KEY_H, TFT_WHITE, keyColor3[b], TFT_WHITE,
                        keyLabel3[b], KEY_TEXTSIZE);
      key3[b].drawButton();
    }
  }
}

void drawkeypadBIRTH(){
  tft.fillRect(0, 0, 480, 320, TFT_CYAN);
  tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
  tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);

  tft.setCursor(47, 30);
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(LABEL2_FONT);
  tft.setTextSize(1);
  if(language == ENGLISH){
    tft.print("Enter Birth Date");
    tft.setCursor(242, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(ex: 7-4-2024)");
  }else if(language == INDONESIA){
    tft.print("Tanggal Lahir");
    tft.setCursor(220, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(cth: 7-4-2024)");
  }
  
  // Draw the keys
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 3; col++) {
      uint8_t b = col + row * 3;

      if (b < 3 || b == 14) tft.setFreeFont(LABEL1_FONT);
      else tft.setFreeFont(LABEL2_FONT);

      key2[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                        KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
                        keyLabel[b], KEY_TEXTSIZE);
      key2[b].drawButton();
    }
  }
}

void drawKeyboard1(){
  tft.fillRect(0, 0, 480, 320, TFT_CYAN);
  tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
  tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(LABEL2_FONT);
  tft.setTextSize(1);
  if(language == ENGLISH){
    if(type == PAUD){
      tft.setCursor(40, 30);
      tft.print("Paud's Name");
      tft.setCursor(200, 17);
      tft.setTextColor(TFT_BLACK);
      tft.setTextFont(0);
      tft.setTextDatum(TC_DATUM);
      tft.setTextSize(2);
      tft.print("(ex: MELATI_BANGLI)");
    }else if(type == POSYANDU){
      tft.setCursor(20, 30);
      tft.print("Posyandu's Name");
      tft.setCursor(240, 17);
      tft.setTextColor(TFT_BLACK);
      tft.setTextFont(0);
      tft.setTextDatum(TC_DATUM);
      tft.setTextSize(2);
      tft.print("(ex: MELATI_BANGLI)");
    }else if(type == PUSKESMAS){
      tft.setCursor(15, 30);
      tft.print("Puskesmas's Name");
      tft.setCursor(245, 17);
      tft.setTextColor(TFT_BLACK);
      tft.setTextFont(0);
      tft.setTextDatum(TC_DATUM);
      tft.setTextSize(2);
      tft.print("(ex: MELATI_BANGLI)");
    }
  }else if(language == INDONESIA){
    if(type == PAUD){
      tft.setCursor(45, 30);
      tft.print("Nama Paud");
      tft.setCursor(185, 17);
      tft.setTextColor(TFT_BLACK);
      tft.setTextFont(0);
      tft.setTextDatum(TC_DATUM);
      tft.setTextSize(2);
      tft.print("(cth: MELATI_BANGLI)");
    }else if(type == POSYANDU){
      tft.setCursor(25, 30);
      tft.print("Nama Posyandu");
      tft.setCursor(220, 17);
      tft.setTextColor(TFT_BLACK);
      tft.setTextFont(0);
      tft.setTextDatum(TC_DATUM);
      tft.setTextSize(2);
      tft.print("(cth: MELATI_BANGLI)");
    }else if(type == PUSKESMAS){
      tft.setCursor(20, 30);
      tft.print("Nama Puskesmas");
      tft.setCursor(230, 17);
      tft.setTextColor(TFT_BLACK);
      tft.setTextFont(0);
      tft.setTextDatum(TC_DATUM);
      tft.setTextSize(2);
      tft.print("(cth: MELATI_BANGLI)");
    }
  }

  // Draw the keys
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 6; col++) {
      uint8_t b = col + row * 6;

      if (b > 26 || b == 23) tft.setFreeFont(LABEL1_FONT);
      else tft.setFreeFont(LABEL2_FONT);

      keyboard1[b].initButton(&tft, KEY_X1 + col * (KEY_W1 + KEY_SPACING_X1),
                        KEY_Y1 + row * (KEY_H1 + KEY_SPACING_Y1), // x, y, w, h, outline, fill, text
                        KEY_W1, KEY_H1, TFT_WHITE, keyboardColor[b], TFT_WHITE,
                        keyboardLabel[b], KEY_TEXTSIZE1);
      keyboard1[b].drawButton();
    }
  }
}

void drawKeyboard2(){
  tft.fillRect(0, 0, 480, 320, TFT_CYAN);
  tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
  tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(LABEL2_FONT);
  tft.setTextSize(1);
  if(language == ENGLISH){
    tft.setCursor(50, 30);
    tft.print("Address");
    tft.setCursor(150, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(ex: JALAN_AKASIA_SOLO)");
  }else if(language == INDONESIA){
    tft.setCursor(50, 30);
    tft.print("Alamat");
    tft.setCursor(150, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(cth: JALAN_AKASIA_SOLO)");
  }

  // Draw the keys
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 6; col++) {
      uint8_t b = col + row * 6;

      if (b > 26 || b == 23) tft.setFreeFont(LABEL1_FONT);
      else tft.setFreeFont(LABEL2_FONT);

      keyboard2[b].initButton(&tft, KEY_X1 + col * (KEY_W1 + KEY_SPACING_X1),
                        KEY_Y1 + row * (KEY_H1 + KEY_SPACING_Y1), // x, y, w, h, outline, fill, text
                        KEY_W1, KEY_H1, TFT_WHITE, keyboardColor[b], TFT_WHITE,
                        keyboardLabel[b], KEY_TEXTSIZE1);
      keyboard2[b].drawButton();
    }
  }
}

void drawKeyboard(){
  tft.fillRect(0, 0, 480, 320, TFT_CYAN);
  tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
  tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(LABEL2_FONT);
  tft.setTextSize(1);
  if(language == ENGLISH){
    tft.setCursor(35, 30);
    tft.print("Toddler's Name");
    tft.setCursor(225, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(ex: ANANDA_AZRIEL)");
  }else if(language == INDONESIA){
    tft.setCursor(40, 30);
    tft.print("Nama Balita");
    tft.setCursor(200, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(cth: ANANDA_AZRIEL)");
  }

  // Draw the keys
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 6; col++) {
      uint8_t b = col + row * 6;

      if (b > 26 || b == 23) tft.setFreeFont(LABEL1_FONT);
      else tft.setFreeFont(LABEL2_FONT);

      keyboard[b].initButton(&tft, KEY_X1 + col * (KEY_W1 + KEY_SPACING_X1),
                        KEY_Y1 + row * (KEY_H1 + KEY_SPACING_Y1), // x, y, w, h, outline, fill, text
                        KEY_W1, KEY_H1, TFT_WHITE, keyboardColor[b], TFT_WHITE,
                        keyboardLabel[b], KEY_TEXTSIZE1);
      keyboard[b].drawButton();
    }
  }
}

void drawKeypadWA(){
  tft.fillRect(0, 0, 480, 320, TFT_CYAN);
  tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
  tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(LABEL2_FONT);
  tft.setTextSize(1);
  if(language == ENGLISH){
    tft.setCursor(50, 30);
    tft.print("WA Number");
    tft.setCursor(195, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(ex: 6285755807245)");
  }else if(language == INDONESIA){
    tft.setCursor(50, 30);
    tft.print( "Nomor WA");
    tft.setCursor(180, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(cth: 6285755807245)");
  }

  // Draw the keys
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 3; col++) {
      uint8_t b = col + row * 3;

      if (b < 3 || b == 14) tft.setFreeFont(LABEL1_FONT);
      else tft.setFreeFont(LABEL2_FONT);

      key4[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                        KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
                        keyLabel2[b], KEY_TEXTSIZE);
      key4[b].drawButton();
    }
  }
}

void drawKeypadAPI(){
  tft.fillRect(0, 0, 480, 320, TFT_CYAN);
  tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
  tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(LABEL2_FONT);
  tft.setTextSize(1);
  if(language == ENGLISH){
    tft.setCursor(50, 30);
    tft.print("API Number");
    tft.setCursor(195, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(ex: 999888)");
  }else if(language == INDONESIA){
    tft.setCursor(50, 30);
    tft.print("Nomor API");
    tft.setCursor(185, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(cth: 999888)");
  }

  // Draw the keys
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 3; col++) {
      uint8_t b = col + row * 3;

      if (b < 3 || b == 14) tft.setFreeFont(LABEL1_FONT);
      else tft.setFreeFont(LABEL2_FONT);

      key5[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                        KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
                        keyLabel2[b], KEY_TEXTSIZE);
      key5[b].drawButton();
    }
  }
}

void drawKeypadDATE(){
  tft.fillRect(0, 0, 480, 320, TFT_CYAN);
  tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
  tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);
  
  tft.setCursor(25, 30);
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(LABEL2_FONT);
  tft.setTextSize(1);
  if(language == ENGLISH){
    tft.print("Enter Today's Date");
    tft.setCursor(255, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(ex: 7-4-2024)");
  }else if(language == INDONESIA){
    tft.print("Tanggal Sekarang");
    tft.setCursor(250, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(2);
    tft.print("(cth: 7-4-2024)");
  }

  // Draw the keys
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 3; col++) {
      uint8_t b = col + row * 3;

      if (b < 3 || b == 14) tft.setFreeFont(LABEL1_FONT);
      else tft.setFreeFont(LABEL2_FONT);

      key1[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                        KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
                        keyLabel[b], KEY_TEXTSIZE);
      key1[b].drawButton();
    }
  }
}
//---------------END of Draw Keypad------------

void touch_calibrate(){
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!LittleFS.begin()) {
    Serial.println("Formating file system");
    LittleFS.format();
    LittleFS.begin();
  }

  // check if calibration file exists and size is correct
  if (LittleFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL) {
      // Delete if we want to re-calibrate
      LittleFS.remove(CALIBRATION_FILE);
    } else {
      File f = LittleFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

void status(const char *msg, uint16_t color){
  tft.setTextPadding(240);
  if(color != 0){
    tft.setTextColor(TFT_BLACK, color);
  }else{
    tft.setTextColor(color, color);
  }
  tft.setTextFont(0);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(1);
  tft.drawString(msg, STATUS_X, STATUS_Y);
}

void drawBitmap(const uint16_t *bitmap){
  int h=320, w=480, row, col, buffidx=0;
  for (row=0; row<h; row++){
    for (col=0; col<w; col++){
      tft.drawPixel(col, row, pgm_read_word(bitmap + buffidx));
      buffidx++;
    }
  }
}

void refreshOffsetValueAndSaveToEEprom() {
  long _offset = 0;
  Serial.println("Calculating tare offset value...");
  LoadCell.tare(); // calculate the new tare / zero offset value (blocking)
  _offset = LoadCell.getTareOffset(); // get the new tare / zero offset value
  EEPROM.put(tareOffsetVal_eepromAdress, _offset); // save the new tare / zero offset value to EEprom
#if defined(ESP8266) || defined(ESP32)
  EEPROM.commit();
#endif
  LoadCell.setTareOffset(_offset); // set value as library parameter (next restart it will be read from EEprom)
  Serial.print("New tare offset value:");
  Serial.print(_offset);
  Serial.print(", saved to EEprom adr:");
  Serial.println(tareOffsetVal_eepromAdress);
}

void sendSpreadsheets(String params) {
  HTTPClient httpp;  // Declare object of class HTTPClient
  String urlSpreadsheets="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+params;
  httpp.begin(urlSpreadsheets);  // begin the HTTPClient object with generated url
  tft.setCursor(65, tft.height()/2);
  tft.setTextColor(TFT_BLACK);
  if(language == ENGLISH){
    tft.print("Sending Data to Spreadsheets...");
  }else if(language == INDONESIA){
    tft.print("Kirim Data ke Spreadsheets...");
  }
  
  int httpCode1 = httpp.GET();  
  httpp.end();
  if(language == ENGLISH){
    tft.setCursor(170, tft.height()/2+40); tft.print("Data Sent"); delay(2000); 
  }else if(language == INDONESIA){
    tft.setCursor(140, tft.height()/2+40); tft.print("Data Terkirim"); delay(2000); 
  }
  
  delay(1000);
  displayState = 16;
}

void sendPrinter(){
  String str1 = "      ANTHRO ZIYO H-2.2\n";
  String str2 = "         " + numberBufferr + "\n\n";
  String stripe = "--------------------------------\n\n";
  String space = "\n";
  for (int i = 0; i < space.length(); i++) {
    SerialBT.write(space[i]);
  }
  for (int i = 0; i < space.length(); i++) {
    SerialBT.write(space[i]);
  }
  for (int i = 0; i < str1.length(); i++) {
    SerialBT.write(str1[i]);
  }
  for (int i = 0; i < str2.length(); i++) {
    SerialBT.write(str2[i]);
  }
  for (int i = 0; i < stripe.length(); i++) {
    SerialBT.write(stripe[i]);
  }
  for (int i = 0; i < pesanPrinter.length(); i++) {
    SerialBT.write(pesanPrinter[i]);
  }
  for (int i = 0; i < stripe.length(); i++) {
    SerialBT.write(stripe[i]);
  }
  for (int i = 0; i < space.length(); i++) {
    SerialBT.write(space[i]);
  }
  for (int i = 0; i < space.length(); i++) {
    SerialBT.write(space[i]);
  }
  for (int i = 0; i < space.length(); i++) {
    SerialBT.write(space[i]);
  }
}

String urlencode(String str) { // Function used for encoding the url
  String encodedString="";
  char c, code0, code1, code2;
  for (int i =0; i < str.length(); i++){
    c=str.charAt(i);
    if (c == ' '){
      encodedString+= '+';
    } else if (isalnum(c)){
      encodedString+=c;
    } else{
      code1=(c & 0xf)+'0';
      if ((c & 0xf) >9){ code1=(c & 0xf) - 10 + 'A'; } 
      c=(c>>4)&0xf; code0=c+'0';
      if (c > 9){ code0=c - 10 + 'A'; }
      code2='\0'; encodedString+='%'; encodedString+=code0; encodedString+=code1;
    }
    yield();
  }
  return encodedString;
}

float read_US(int trigger, int echo){
  long duration;
  float distance;
  int i;

  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  duration = pulseIn(echo, HIGH); //uS
  distance = duration * 0.034 / 2;
  return distance;
}

float movingAverage(float in_data, int debug){
  float average, sum;

  data[i_data] = in_data;
  if (data[n_data - 1] == 0){ sum = 0;
    for (int i = 0; i <= i_data; i++){ sum += data[i]; }
    average = sum / (i_data + 1);
  }
  else{ sum = 0;
    for (int i = 0; i < n_data; i++){ sum += data[i]; }
    average = sum / n_data; }

  i_data++;
  if (i_data >= n_data) i_data = 0;
  Serial.print("Average: "); Serial.println(average); return average;
}

float movingAverage1(float in_data, int debug){
  float average1, sum;

  data[i_data] = in_data;

  if (data[n_data - 1] == 0){ sum = 0;
    for (int i = 0; i <= i_data; i++){ sum += data[i]; }
    average1 = sum / (i_data + 1);
  } else{ sum = 0;
    for (int i = 0; i < n_data; i++){ sum += data[i]; }
    average1 = sum / n_data;
  }

  i_data++;
  if (i_data >= n_data) i_data = 0;

  Serial.print("Average: "); Serial.println(average1);return average1;
}

float movingAverage2(float in_data, int debug){
  float average2;
  float sum;

  data[i_data] = in_data;

  if (data[n_data - 1] == 0){
    sum = 0;

    for (int i = 0; i <= k_data; i++){
      sum += data[i];
    }
    average2 = sum / (i_data + 1);
  }
  else{
    sum = 0;
    for (int i = 0; i < n_data; i++){
      sum += data[i];
    }
    average2 = sum / n_data;
  }

  i_data++;
  if (i_data >= n_data)
    i_data = 0;

  // Serial.print("Average: "); Serial.println(average2);
  return average2;
}

void movingAverage_Clear(){
  //Make sure All Array is 0
  for (int i = 0; i < n_data; i++) {
    data[i] = 0;
  }
}

void movingAverage1_Clear(){
  //Make sure All Array is 0
  for (int i = 0; i < n_data; i++) {
    data[i] = 0;
  }
}

void movingAverage2_Clear(){
  //Make sure All Array is 0
  for (int i = 0; i < n_data; i++) {
    data[i] = 0;
  }
}

bool isLeapYear(int year){
  if(year % 4 != 0){
    return false;
  } else if (year % 100 != 0 ){
    return true;
  } else if (year % 400 != 0) {
    return false;
  } else {
    return true;
  }
}

int getDaysInMonth (int month, int year){
  if (month == 2){
    return isLeapYear(year) ? 29 :28;
  } else if (month ==  4 || month == 6 || month == 9 || month == 11){
    return 30;
  } else{
    return 31;
  }
}

void setup() {
  Serial.begin(9600);
  LoadCell.begin();
  float calibrationValue = 34612.71;
  EEPROM.begin(512);
  EEPROM.get(69,simpangan);
  // EEPROM.get(calVal_eepromAdress, calibrationValue);
  EEPROM.get(100,phoneNumber);
  EEPROM.get(200,apiKey);
  long tare_offset = 0;
  EEPROM.get(tareOffsetVal_eepromAdress, tare_offset);
  LoadCell.setTareOffset(tare_offset);
  boolean _tare = false;
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  LoadCell.start(stabilizingtime, _tare);
  tft.begin();
  tft.setRotation(1);
  tft.setFreeFont(&FreeSansBold9pt7b); //FF18
  pinMode(buzzer, OUTPUT);
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  if (LoadCell.getTareTimeoutFlag()) {
    //Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    //Serial.println("Startup is complete");
  }
  movingAverage2_Clear();

  initButtonINDO();
  initButtonENG();
  // displayState = 0;
  // initButtonSTART();
  // initButtonCAL();
}

void loop() {
  while((millis()-lastFrame )<20);
  lastFrame = millis();
  
  static uint32_t scanTime = millis();
  static uint32_t scanTime1 = millis();
  static uint32_t scanTime2 = millis();
  uint16_t t_x = 9999, t_y = 9999;    // To store the touch coordinates
  uint16_t t_x2 = 9999, t_y2 = 9999;
  uint16_t t_x4 = 9999, t_y4 = 9999;
  uint16_t t_x5 = 9999, t_y5 = 9999;
  uint16_t t_x6 = 9999, t_y6 = 9999;
  uint16_t t_x7 = 9999, t_y7 = 9999;
  uint16_t t_x8 = 9999, t_y8 = 9999;
  bool pressed = tft.getTouch(&t_x, &t_y);
  bool pressed2 = tft.getTouch(&t_x2, &t_y2);
  bool pressed4 =tft.getTouch(&t_x4, &t_y4);
  bool pressed5 =tft.getTouch(&t_x5, &t_y5);
  bool pressed6 =tft.getTouch(&t_x6, &t_y6);
  bool pressed7 =tft.getTouch(&t_x7, &t_y7);
  bool pressed8 =tft.getTouch(&t_x8, &t_y8);
  char *token ;
  char *token2;
  const char delimiter[] = "-";

//-------------Display STATE------------
  switch(displayState){
  
  case 0:
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed5 = tft.getTouch(&t_x, &t_y);
    scanTime = millis();
    for (uint8_t b = 12; b <= 13; b++) {
      if (pressed5) {
        if (btn[b]->contains(t_x, t_y)) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }
  break; 
  break;

  case 1:
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();
    for (uint8_t b = 0; b <= 2; b++) {
      if (pressed) {
        if (btn[b]->contains(t_x, t_y)) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }
  break; 
  break;

  case 2:
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();
    for (uint8_t b = 3; b <= 4; b++) {
      if (pressed) {
        if (btn[b]->contains(t_x, t_y)) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }
  break; 
  break;

  case 3:
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();
    for (uint8_t b = 16; b <= 18; b++) {
      if (pressed) {
        if (btn[b]->contains(t_x, t_y)) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }
  break; 
  break;

  case 4:
  for (uint8_t b = 0; b < 30; b++) {
    if (pressed8 && keyboard1[b].contains(t_x8, t_y8)) {
      keyboard1[b].press(true);  // tell the button it is pressed
    } else {
      keyboard1[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 30; b++) {

    if (b > 26 || b == 23) tft.setFreeFont(LABEL1_FONT);
    else tft.setFreeFont(LABEL2_FONT);

    if (keyboard1[b].justReleased()) keyboard1[b].drawButton();     // draw normal

    if (keyboard1[b].justPressed()) {
      keyboard1[b].drawButton(true);  // draw invert

      // if a numberpad button, append the relevant # to the 
      if (b < 28 && b != 23) {
        if (indexPlace < NUM_LEN3) {
          placeName[indexPlace] = keyboardLabel[b][0];
          indexPlace++;
          placeName[indexPlace] = 0; // zero terminate
        }
        status("", 0); // Clear the old status
      }

      // Del button, so delete last char
      if (b == 23) {
        placeName[indexPlace] = 0;
        if (indexPlace > 0) {
          indexPlace--;
          placeName[indexPlace] = 0;//' ';
        }
        status("", 0); // Clear the old status
      }

      if (b == 29) {
        if(language == ENGLISH){
          status("Sent value to serial port", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Mengirim ke serial port", TFT_CYAN);
        }
        //Serial.println(placeName);
        tft.fillScreen(TFT_CYAN);
        tft.setTextSize(4);
        tft.setTextColor(TFT_BLACK);
        int placeDigit = strlen(placeName);
        tft.setCursor(tft.width()/2-placeDigit*12, tft.height()/2-20);
        tft.print(placeName);
        delay(3000);

        displayState = 5;
        drawKeyboard();
        break;
      }
      
      if (b == 28) {
        if(language == ENGLISH){
          status("Value cleared", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Data dihapus", TFT_CYAN);
        }
        
        indexPlace = 0; // Reset index to 0
        placeName[indexPlace] = 0; // Place null in buffer
      }

      // if (b == 14) {
      //   displayState = 2; 
      //   initButtonBOY();
      //   initButtonGIRL();
      // }

      if (b != 29){
      // Update the number display field
      tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
      tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
      tft.setTextColor(DISP_TCOLOR);     // Set the font colour
      tft.setTextSize(1);

      // Draw the string, the value returned is the width in pixels
      int xwidth = tft.drawString(placeName, DISP_X + 4, DISP_Y + 12);

      // Now cover up the rest of the line up by drawing a black rectangle.  No flicker this way
      // but it will not work with italic or oblique fonts due to character overlap.
      tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);

      delay(10); // UI debouncing
      }
    }
  }
  break;

  case 5:
  for (uint8_t b = 0; b < 30; b++) {
    if (pressed && keyboard[b].contains(t_x, t_y)) {
      keyboard[b].press(true);  // tell the button it is pressed
    } else {
      keyboard[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 30; b++) {

    if (b > 26 || b == 23) tft.setFreeFont(LABEL1_FONT);
    else tft.setFreeFont(LABEL2_FONT);

    if (keyboard[b].justReleased()) keyboard[b].drawButton();     // draw normal

    if (keyboard[b].justPressed()) {
      keyboard[b].drawButton(true);  // draw invert

      // if a numberpad button, append the relevant # to the 
      if (b < 28 && b != 23) {
        if (indexName < NUM_LEN3) {
          name[indexName] = keyboardLabel[b][0];
          indexName++;
          name[indexName] = 0; // zero terminate
        }
        status("", 0); // Clear the old status
      }

      // Del button, so delete last char
      if (b == 23) {
        name[indexName] = 0;
        if (indexName > 0) {
          indexName--;
          name[indexName] = 0;//' ';
        }
        status("", 0); // Clear the old status
      }

      if (b == 29) {
        if(language == ENGLISH){
          status("Sent value to serial port", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Mengirim ke serial port", TFT_CYAN);
        }
        //Serial.println(name);
        tft.fillScreen(TFT_CYAN);
        tft.setTextSize(4);
        tft.setTextColor(TFT_BLACK);
        int nameDigit = strlen(name);
        tft.setCursor(tft.width()/2-nameDigit*12, tft.height()/2-20);
        tft.print(name);
        delay(3000);

        displayState = 6;
        drawKeyboard2();
        break;
      }
      
      if (b == 28) {
        if(language == ENGLISH){
          status("Value cleared", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Data dihapus", TFT_CYAN);
        }
        
        indexName = 0; // Reset index to 0
        name[indexName] = 0; // Place null in buffer
      }

      if (b != 29){
      // Update the number display field
      tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
      tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
      tft.setTextColor(DISP_TCOLOR);     // Set the font colour
      tft.setTextSize(1);

      // Draw the string, the value returned is the width in pixels
      int xwidth = tft.drawString(name, DISP_X + 4, DISP_Y + 12);

      // Now cover up the rest of the line up by drawing a black rectangle.  No flicker this way
      // but it will not work with italic or oblique fonts due to character overlap.
      tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);

      delay(10); // UI debouncing
      }
    }
  }
  break;

  case 6:
  for (uint8_t b = 0; b < 30; b++) {
    if (pressed && keyboard2[b].contains(t_x, t_y)) {
      keyboard2[b].press(true);  // tell the button it is pressed
    } else {
      keyboard2[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 30; b++) {

    if (b > 26 || b == 23) tft.setFreeFont(LABEL1_FONT);
    else tft.setFreeFont(LABEL2_FONT);

    if (keyboard2[b].justReleased()) keyboard2[b].drawButton();     // draw normal

    if (keyboard2[b].justPressed()) {
      keyboard2[b].drawButton(true);  // draw invert

      // if a numberpad button, append the relevant # to the 
      if (b < 28 && b != 23) {
        if (indexAddress < NUM_LEN3) {
          address[indexAddress] = keyboardLabel[b][0];
          indexAddress++;
          address[indexAddress] = 0; // zero terminate
        }
        status("", 0); // Clear the old status
      }

      // Del button, so delete last char
      if (b == 23) {
        address[indexAddress] = 0;
        if (indexAddress > 0) {
          indexAddress--;
          address[indexAddress] = 0;//' ';
        }
        status("", 0); // Clear the old status
      }

      if (b == 29) {
        if(language == ENGLISH){
          status("Sent value to serial port", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Mengirim ke serial port", TFT_CYAN);
        }
        //Serial.println(name);
        tft.fillScreen(TFT_CYAN);
        tft.setTextSize(4);
        tft.setTextColor(TFT_BLACK);
        int addressDigit = strlen(address);
        tft.setCursor(tft.width()/2-addressDigit*12, tft.height()/2-20);
        tft.print(address);
        delay(3000);

        // displayState = 7;
        // drawKeypadWA();
        displayState = 9;
        drawKeypadDATE();
        break;
      }
      
      if (b == 28) {
        if(language == ENGLISH){
          status("Value cleared", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Data dihapus", TFT_CYAN);
        }
        
        indexAddress = 0; // Reset index to 0
        address[indexAddress] = 0; // Place null in buffer
      }
      
      if (b != 29){
      // Update the number display field
      tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
      tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
      tft.setTextColor(DISP_TCOLOR);     // Set the font colour
      tft.setTextSize(1);

      // Draw the string, the value returned is the width in pixels
      int xwidth = tft.drawString(address, DISP_X + 4, DISP_Y + 12);

      // Now cover up the rest of the line up by drawing a black rectangle.  No flicker this way
      // but it will not work with italic or oblique fonts due to character overlap.
      tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);

      delay(10); // UI debouncing
      }
    }
  }
  break;

  case 7: 
  for (uint8_t b = 0; b < 15; b++) {
    if (pressed6 && key4[b].contains(t_x6, t_y6)) {
      key4[b].press(true);  // tell the button it is pressed
    } else {
      key4[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 15; b++) {

    if (b < 3 || b == 14) tft.setFreeFont(LABEL1_FONT);
    else tft.setFreeFont(LABEL2_FONT);

    if (key4[b].justReleased()) key4[b].drawButton();     // draw normal

    if (key4[b].justPressed()) {
      key4[b].drawButton(true);  // draw invert

      // if a numberpad button, append the relevant # to the 
      if (b >= 3 && b != 14) {
        if (indexWA < NUM_LEN5) {
          phoneNumber[indexWA] = keyLabel[b][0];
          indexWA++;
          phoneNumber[indexWA] = 0; // zero terminate
        }
        status("", 0); // Clear the old status
      }

      // Del button, so delete last char
      if (b == 1) {
        phoneNumber[indexWA] = 0;
        if (indexWA > 0) {
          indexWA--;
          phoneNumber[indexWA] = 0;//' ';
        }
        status("", 0); // Clear the old status
      }

      if (b == 2) {
        if(language == ENGLISH){
          status("Sent value to serial port", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Mengirim ke serial port", TFT_CYAN);
        }
        
        //Serial.println(phoneNumber);
        tft.fillScreen(TFT_CYAN);
        tft.setCursor(tft.width()/2-150, tft.height()/2-20);
        tft.setTextSize(4);
        tft.setTextColor(TFT_BLACK);
        int phoneDigit = strlen(phoneNumber);
        tft.setCursor(tft.width()/2-phoneDigit*12, tft.height()/2-20);
        tft.print(phoneNumber);
        EEPROM.put(100,phoneNumber);
        EEPROM.commit();  
        //EEPROM

        delay(3000);

        displayState = 8;
        drawKeypadAPI();
        break;
      }
      
      if (b == 0) {
        if(language == ENGLISH){
          status("Value cleared", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Data dihapus", TFT_CYAN);
        }
        
        indexWA = 0; // Reset index to 0
        phoneNumber[indexWA] = 0; // Place null in buffer
      }

      if (b == 14) {
        // displayState = 6; 
        // drawKeyboard2();
        initButtonUbahWIFI();
        displayState = 21;
      }

      if (b != 14){
      // Update the number display field
      tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
      tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
      tft.setTextColor(DISP_TCOLOR);     // Set the font colour

      // Draw the string, the value returned is the width in pixels
      int xwidth = tft.drawString(phoneNumber, DISP_X + 4, DISP_Y + 12);
      tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);

      delay(10); // UI debouncing
      }
    }
  }
  break;

  case 8: 
  for (uint8_t b = 0; b < 15; b++) {
    if (pressed7 && key5[b].contains(t_x7, t_y7)) {
      key5[b].press(true);  // tell the button it is pressed
    } else {
      key5[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 15; b++) {

    if (b < 3 || b == 14) tft.setFreeFont(LABEL1_FONT);
    else tft.setFreeFont(LABEL2_FONT);

    if (key5[b].justReleased()) key5[b].drawButton();     // draw normal

    if (key5[b].justPressed()) {
      key5[b].drawButton(true);  // draw invert

      // if a numberpad button, append the relevant # to the 
      if (b >= 3 && b != 14) {
        if (indexAPI < NUM_LEN4) {
          apiKey[indexAPI] = keyLabel[b][0];
          indexAPI++;
          apiKey[indexAPI] = 0; // zero terminate
        }
        status("", 0); // Clear the old status
      }

      // Del button, so delete last char
      if (b == 1) {
        apiKey[indexAPI] = 0;
        if (indexAPI > 0) {
          indexAPI--;
          apiKey[indexAPI] = 0;//' ';
        }
        status("", 0); // Clear the old status
      }

      if (b == 2) {
        if(language == ENGLISH){
          status("Sent value to serial port", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Mengirim ke serial port", TFT_CYAN);
        }
        
        //Serial.println(phoneNumber);
        tft.fillScreen(TFT_CYAN);
        tft.setCursor(tft.width()/2-90, tft.height()/2-20);
        tft.setTextSize(4);
        tft.setTextColor(TFT_BLACK);
        int apiDigit = strlen(apiKey);
        tft.setCursor(tft.width()/2-apiDigit*12, tft.height()/2-20);
        tft.print(apiKey);

        EEPROM.put(200,apiKey);
        EEPROM.commit(); 

        delay(2000);
        ESP.restart();

        // displayState = 9;
        // drawKeypadDATE();
        //EEPROM
        break;
      }
      
      if (b == 0) {
        if(language == ENGLISH){
          status("Value cleared", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Data dihapus", TFT_CYAN);
        }
        
        indexAPI = 0; // Reset index to 0
        apiKey[indexAPI] = 0; // Place null in buffer
      }

      if (b == 14) {
        displayState = 7;
        drawKeypadWA();
      }

      if (b != 14){
      // Update the number display field
      tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
      tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
      tft.setTextColor(DISP_TCOLOR);     // Set the font colour

      // Draw the string, the value returned is the width in pixels
      int xwidth = tft.drawString(apiKey, DISP_X + 4, DISP_Y + 12);
      tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);

      delay(10); // UI debouncing
      }
    }
  }
  break;

  case 9: 
  for (uint8_t b = 0; b < 15; b++) {
    if (pressed2 && key1[b].contains(t_x2, t_y2)) {
      key1[b].press(true);  // tell the button it is pressed
    } else {
      key1[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 15; b++) {

    if (b < 3 || b == 14) tft.setFreeFont(LABEL1_FONT);
    else tft.setFreeFont(LABEL2_FONT);

    if (key1[b].justReleased()) key1[b].drawButton();     // draw normal

    if (key1[b].justPressed()) {
      key1[b].drawButton(true);  // draw invert

      // if a numberpad button, append the relevant # to the 
      if (b >= 3 && b != 14) {
        if (numberIndex < NUM_LEN) {
          numberBuffer[numberIndex] = keyLabel[b][0];
          numberIndex++;
          numberBuffer[numberIndex] = 0; // zero terminate
        }
        status("", 0); // Clear the old status
      }

      // Del button, so delete last char
      if (b == 1) {
        numberBuffer[numberIndex] = 0;
        if (numberIndex > 0) {
          numberIndex--;
          numberBuffer[numberIndex] = 0;//' ';
        }
        status("", 0); // Clear the old status
      }

      if (b == 2) {
        if(language == ENGLISH){
          status("Sent value to serial port", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Mengirim ke serial port", TFT_CYAN);
        }
        
        //Serial.println(numberBuffer);
        tft.fillScreen(TFT_CYAN);
        tft.setCursor(tft.width()/2-120, tft.height()/2-20);
        tft.setTextSize(4);
        tft.setTextColor(TFT_BLACK);
        tft.print(numberBuffer);
        delay(3000);
        displayState = 10;
        drawkeypadBIRTH();
        break;
      }
      
      if (b == 0) {
        if(language == ENGLISH){
          status("Value cleared", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Data dihapus", TFT_CYAN);
        }
        
        numberIndex = 0; // Reset index to 0
        numberBuffer[numberIndex] = 0; // Place null in buffer
      }

      if (b == 14) {
        displayState = 6;
        drawKeyboard2();
      }

      if (b != 14){
      // Update the number display field
      tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
      tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
      tft.setTextColor(DISP_TCOLOR);     // Set the font colour

      // Draw the string, the value returned is the width in pixels
      int xwidth = tft.drawString(numberBuffer, DISP_X + 4, DISP_Y + 12);
      numberBufferr = numberBuffer;
      tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);

      delay(10); // UI debouncing
      }
    }
  }
  break;

  case 10:
  for (uint8_t b = 0; b < 15; b++) {
    if (pressed4 && key2[b].contains(t_x4, t_y4)) {
      key2[b].press(true);  // tell the button it is pressed
    } else {
      key2[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 15; b++) {

    if (b < 3 || b == 14) tft.setFreeFont(LABEL1_FONT);
    else tft.setFreeFont(LABEL2_FONT);

    if (key2[b].justReleased()) key2[b].drawButton();     // draw normal

    if (key2[b].justPressed()) {
      key2[b].drawButton(true);  // draw invert

      // if a numberpad button, append the relevant # to the tanggalLahir
      if (b >= 3 && b != 14) {
        if (indexLahir < NUM_LEN) {
          tanggalLahir[indexLahir] = keyLabel[b][0];
          indexLahir++;
          tanggalLahir[indexLahir] = 0; // zero terminate
        }
        status("", 0); // Clear the old status
      }

      // Del button, so delete last char
      if (b == 1) {
        tanggalLahir[indexLahir] = 0;
        if (indexLahir > 0) {
          indexLahir--;
          tanggalLahir[indexLahir] = 0;//' ';
        }
        status("", 0); // Clear the old status
      }
      if (b == 2) {
        if(language == ENGLISH){
          status("Sent value to serial port", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Mengirim ke serial port", TFT_CYAN);
        }
        
        //Serial.println(tanggalLahir);
        displayState = 11;
        delay(1000);
        tft.fillScreen(TFT_CYAN);
        tft.setCursor(tft.width()/2-120, tft.height()/2-20);
        tft.setTextSize(4);
        tft.setTextColor(TFT_BLACK);
        tft.print(tanggalLahir);
        //initButtons4();
        delay(3000);
        break;
      }
      if (b == 0) {
        if(language == ENGLISH){
          status("Value cleared", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Data dihapus", TFT_CYAN);
        }
        
        indexLahir = 0; // Reset index to 0
        tanggalLahir[indexLahir] = 0; // Place null in buffer
      }
      if (b == 14) {
        displayState = 9; 
        drawKeypadDATE();
      }
      if (b != 14){
      // Update the number display field
      tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
      tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
      tft.setTextColor(DISP_TCOLOR);     // Set the font colour

      // Draw the string, the value returned is the width in pixels
      int xwidth = tft.drawString(tanggalLahir, DISP_X + 4, DISP_Y + 12);

      tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);
      delay(10); // UI debouncing
      }
    }
  }
  break;

  case 11: // Calculate umur
  char parsedStrings[5][20];
  token = strtok(numberBuffer,delimiter);
  strncpy(parsedStrings[0], token, sizeof(parsedStrings[0]));
  for (int i = 1 ; i< 3; i++){
    token = strtok(NULL,delimiter);
    strncpy(parsedStrings[i],token, sizeof(parsedStrings[i]));
  }

  currentDate = atoi(parsedStrings[0]);
  currentMonth = atoi(parsedStrings[1]);
  currentYear = atoi(parsedStrings[2]);

  char parsedStrings2[5][20];
  token2 = strtok(tanggalLahir,delimiter);
  strncpy(parsedStrings2[0], token2, sizeof(parsedStrings2[0]));
  for (int i = 1 ; i< 3; i++){
  token2 = strtok(NULL,delimiter);
    strncpy(parsedStrings2[i],token2, sizeof(parsedStrings2[i]));
  }

  birthDate = atoi(parsedStrings2[0]);
  birthMonth = atoi(parsedStrings2[1]);
  birthYear = atoi(parsedStrings2[2]);

  for( int year = birthYear; year < currentYear ; year++){
    if(isLeapYear(year)){
      days += 366;
    }else{
      days += 365;
    }
  }

  for(int month = 1 ; month < birthMonth ; month++){
    days -= getDaysInMonth(month, birthYear);
  }
  days -= birthDate;

  for(int month = 1 ; month < currentMonth ; month++){
    days += getDaysInMonth(month, currentYear);
  }

  days += currentDate;
  usiaHari = days;
  usiaBulan = usiaHari/30.4375;
  usiaBulanTemp = usiaBulan;
  
  tft.fillScreen(TFT_CYAN);
  tft.setCursor(tft.width()/2 -110, tft.height()/2-40 );
  tft.setTextSize(3);
  tft.setTextColor(TFT_BLACK);
  if(language == ENGLISH){
    tft.print("Age: ");
    tft.setCursor(tft.width()/2 - 20, tft.height()/2-40 );
    tft.print(usiaBulanTemp);
    tft.setCursor(tft.width()/2, tft.height()/2-40 );
    tft.print(" Month");
    delay(1000);
    tft.setCursor(tft.width()/2-90,tft.height()/2);
    tft.print("Loading...");
    delay(1000);
  }else if(language == INDONESIA){
    tft.print("Umur: ");
    tft.setCursor(tft.width()/2 - 20, tft.height()/2-40 );
    tft.print(usiaBulanTemp);
    tft.setCursor(tft.width()/2, tft.height()/2-40 );
    tft.print(" Bulan");
    delay(1000);
    tft.setCursor(tft.width()/2-90,tft.height()/2);
    tft.print("Memuat...");
    delay(1000);
  }

  tft.fillScreen(TFT_CYAN);
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setCursor(30,40);
  tft.setTextSize(1);
  // tft.print("Berdiri diatas timbangan !");
    // tft.setCursor(tft.width()/2-135,tft.height()/2+80);
    // tft.setTextSize(1);
    // tft.setTextColor(TFT_WHITE);
    // tft.print("Klik simpan untuk lanjut");
  SRF.setCorrectionFactor(1.035);
  // initButtonSTD();
  initButtonWEIGHT();
  displayState = 12;
  break;  

  case 12: //ukur berat
  static boolean newDataReady = 0;
  if (LoadCell.update() != 0) newDataReady = true;
  if (newDataReady) {
    float i = LoadCell.getData();
    // if(i != 0){
      // bb = movingAverage2(i, 0);
      bb = i;
    // }
    if (millis() > prevMillis + weightInterval) {
      // Serial.print("Load_cell output val: ");
      //Serial.print(i); Serial.print("  ||  "); Serial.print(bb); Serial.print("  ||  "); Serial.println(bb, 1);
      tft.fillRect((tft.width()-120)/2-12, tft.height()/2-75, 140, 40, TFT_YELLOW);
      if (bb >= 100){
        tft.setCursor((tft.width()-120)/2 + 10, tft.height()/2-43);
        tft.setTextSize(2);
        tft.setTextColor(TFT_BLACK, TFT_BROWN);
        tft.print(bb, 1);
      }else if (bb < 10){
        tft.setCursor((tft.width()-120)/2 + 30, tft.height()/2-43);
        tft.setTextSize(2);
        tft.setTextColor(TFT_BLACK, TFT_BROWN);
        tft.print(bb, 1);
      }else{
        tft.setCursor((tft.width()-120)/2 + 20, tft.height()/2-43);
        tft.setTextSize(2);
        tft.setTextColor(TFT_BLACK, TFT_BROWN);
        tft.print(bb, 1);
      }
      newDataReady = 0;
      prevMillis = millis();
    }
  } 

  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    //Serial.println(pressed);
    scanTime = millis();
    for (uint8_t b = 14; b <= 15; b++) {
      if (pressed) {
        if (btn[b]->contains(t_x, t_y)) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }
  break;
  break;

  case 13: //ukur tinggi
  newDistance = read_US(trigger, echo);
  ave = 0.0404*newDistance - 0.077;
  newAve = movingAverage(newDistance, 0);
  newAver = newAve + ave;
  newAverage = movingAverage1(newAver+simpangan, 0);

  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    //Serial.print(newDistance); Serial.print(" ");
    //Serial.println(newAverage);
    tft.fillRect((tft.width()-120)/2-12, tft.height()/2-75, 140, 40, TFT_YELLOW);
    if (newAverage >= 100){
      tft.setCursor((tft.width()-120)/2 + 10, tft.height()/2-43);
      tft.setTextSize(2);
      tft.setTextColor(TFT_BLACK, TFT_BROWN);
      tft.print(newAverage, 1);
    }else if (newAverage+simpangan < 10){
      tft.setCursor((tft.width()-120)/2 + 30, tft.height()/2-43);
      tft.setTextSize(2);
      tft.setTextColor(TFT_BLACK, TFT_BROWN);
      tft.print(newAverage, 1);
    }else{
      tft.setCursor((tft.width()-120)/2 + 20, tft.height()/2-43);
      tft.setTextSize(2);
      tft.setTextColor(TFT_BLACK, TFT_BROWN);
      tft.print(newAverage, 1);
    }
  }

  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis(); 
    if (pressed) {
      if (btnCALCULATE.contains(t_x, t_y)) {
        btnCALCULATE.press(true);
        btnCALCULATE.pressAction();
      }
      else {
      btnCALCULATE.press(false);
      btnCALCULATE.releaseAction();
      }
    }
  }
  break;

  //calculate Zscore
  case 14:
  tft.fillScreen(TFT_CYAN);
  tft.setTextSize(2);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  tft.setTextSize(1);

  while (WiFi.status() != WL_CONNECTED) {  // Wait until WiFi is connected
    // tft.fillRect(tft.width()/2 - 127, tft.height()/2-100, 245, 30, TFT_RED);
    tft.setCursor(tft.width()/2 - 140, tft.height()/2);
    tft.setTextColor(TFT_RED);
    if(language == ENGLISH){
      tft.print("Please Turn On Wi-Fi");
    }else if(language == INDONESIA){
      tft.print("Mohon Nyalakan Wi-Fi");
    }
  }
  
  tft.fillScreen(TFT_CYAN);
  if(type == 0){
    typee = "PAUD";
  }else if(type == 1){
    typee = "POSYANDU";
  }else if(type == 2){
    typee = "PUSKESMAS";
  }

  if(gender == BOY){
    genderr = "Laki-Laki";
  }else if(gender == GIRL){
    genderr = "Perempuan";
  }
    
  pesanWA = "Hasil Pengukuran Anthro Ziyo \n \n" + typee + " " + String(placeName) + "\n" + String(name) 
  + "\n" + String(address) + "\nUmur : " + String(usiaBulan) + " Bulan" + "\nJenis Kelamin : " + genderr 
  + "\nBerat Badan (kg) : " + String(bb) + "\nTinggi Badan (cm) : " + String(length) 
  + "\nZ-Score BB/U : " + String(zscore_BBU) + "  " + kategori_BBUU 
  + "\nZ-Score TB/U : " + String(zscore_TBU) + "  " + kategori_TBUU 
  + "\nZ-Score BB/TB : " + String(zscore_BBTB) + "  " + kategori_BBTBB 
  + "\nZ-Score IMT/U : " + String(zscore_IMTU) + "  " + kategori_IMTUU; 

  pesanPrinter = typee + " " + String(placeName) + "\n\n" + String(name) 
    + "\n" + String(address) + "\nUmur : " + String(usiaBulan) + " Bulan" + "\nJenis Kelamin : " + genderr //+ "\nTanggal : " + String(numberBuffer) 
    + "\n\nBerat Badan (kg)  : " + String(bb) + "\nTinggi Badan (cm) : " + String(length) 
    + "\n\nBB/U  : " + String(zscore_BBU) + "  (" + kategori_BBUU + ")"
    + "\nTB/U  : " + String(zscore_TBU) + "  (" + kategori_TBUU + ")"
    + "\nBB/TB : " + String(zscore_BBTB) + "  (" + kategori_BBTBB + ")" 
    + "\nIMT/U : " + String(zscore_IMTU) + "  (" + kategori_IMTUU + ")\n\n"; 
  sendWA(pesanWA);
  displayState = 15;
  break;
    
  //Output
  case 15:

  if(type == 0){
    typee = "PAUD";
  }else if(type == 1){
    typee = "POSYANDU";
  }else if(type == 2){
    typee = "PUSKESMAS";
  }

  if(gender == BOY){
    genderr = "Boy";
  }else if(gender == GIRL){
    genderr = "Girl";
  }
  placeNameStr = String(placeName); nameStr = String(name); addressStr = String(address);
  pesanSpreadsheets = "Place=" + typee + "&Place_Name=" + urlencodee(placeNameStr) + "&Name=" + urlencodee(nameStr)
  + "&Address=" + urlencodee(addressStr) + "&Age(month)=" + String(usiaBulan) + "&Gender=" + genderr
  + "&WA_Number=" + String(phoneNumber) + "&API_Key=" + String(apiKey)
  + "&Body_Weight=" + String(bb) + "&Height=" + String(length) + "&WAZ=" + String(zscore_BBU)
  + "&HAZ=" + String(zscore_TBU) + "&WLZ=" + String(zscore_BBTB) 
  + "&BMIZ=" + String(zscore_IMTU);

  sendSpreadsheets(pesanSpreadsheets);
  delay(1000);
  // displayState = 16;
  break;

  case 16:
  //output
  tft.fillScreen(TFT_CYAN);
  tft.setCursor(75, tft.height()/2);
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(LABEL2_FONT);
  tft.setTextSize(2);
  if(language == ENGLISH){
    tft.print("Calculating..");
    delay(2000);
    
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.fillScreen(TFT_CYAN);
    tft.setTextSize(1);
    tft.setTextColor(TFT_BLACK);

    tft.setCursor(35, 50);
    tft.print(name);

    tft.setCursor(35, 70);
    tft.print(address);

    tft.setCursor(35, 90);
    tft.print(phoneNumber);

    tft.setCursor(35, 110);
    tft.print(apiKey);

    tft.setCursor(35, 130);
    tft.print("Age :");
    tft.setCursor(tft.width()/2-157, 130);
    tft.print(usiaBulan);
    if (usiaBulan < 10){
      tft.setCursor(tft.width()/2-137, 130);
      tft.print("month");
    }else{
      tft.setCursor(tft.width()/2-127, 130);
      tft.print("month");
    }

    tft.setCursor(35, 150);
    tft.print("Gender :");
    tft.setCursor(tft.width()/2-130, 150);
    if(gender == BOY){
      tft.print("Boy");
    }
    else if (gender == GIRL){
      tft.print("Girl");
    }

    tft.setCursor(35, 170); //160
    tft.print("Weight (kg) :");
    tft.setCursor(tft.width()/2-90, 170);
    tft.print(bb);

    tft.setCursor(35, 190); //130
    tft.print("Height (cm) :");
    tft.setCursor(tft.width()/2-90, 190);
    tft.print(tinggiBadan);

    tft.setCursor(35, 210); //220
    tft.print("Z-Score WAZ");
    tft.setCursor(tft.width()/2-80, 210);
    tft.print(":");
    tft.setCursor(tft.width()/2-70, 210);
    tft.print(zscore_BBU);
    tft.setCursor(tft.width()/2-13, 210);
    tft.print(kategori_BBU);

    tft.setCursor(35, 230); //190
    if(usiaBulan < 24){
      tft.print("Z-Score LAZ");
    }else if(usiaBulan >= 24){
      tft.print("Z-Score HAZ");
    }
    tft.setCursor(tft.width()/2-80, 230);
    tft.print(":");
    tft.setCursor(tft.width()/2-70, 230);
    tft.print(zscore_TBU);
    tft.setCursor(tft.width()/2-13, 230);
    tft.print(kategori_TBU);

    tft.setCursor(35, 250);
    if(usiaBulan < 24){
      tft.print("Z-Score WLZ");
    }else if(usiaBulan >= 24){
      tft.print("Z-Score WHZ");
    }
    tft.setCursor(tft.width()/2-80, 250);
    tft.print(":");
    tft.setCursor(tft.width()/2-70, 250);
    tft.print(zscore_BBTB);
    tft.setCursor(tft.width()/2-13, 250);
    tft.print(kategori_BBTB);

    tft.setCursor(35, 270);
    tft.print("Z-Score BMIZ");
    tft.setCursor(tft.width()/2-80, 270);
    tft.print(":");
    tft.setCursor(tft.width()/2-70, 270);
    tft.print(zscore_IMTU);
    tft.setCursor(tft.width()/2-13, 270); //+25
    tft.print(kategori_IMTU);
  }
  else if(language == INDONESIA){
    tft.print("Kalkulasi..");
    delay(2000);
    
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.fillScreen(TFT_CYAN);
    tft.setTextSize(1);
    tft.setTextColor(TFT_BLACK);

    tft.setCursor(35, 50);
    tft.print(name);

    tft.setCursor(35, 70);
    tft.print(address);

    tft.setCursor(35, 90);
    tft.print(phoneNumber);

    tft.setCursor(35, 110);
    tft.print(apiKey);

    tft.setCursor(35, 130);
    tft.print("Umur :");
    
    tft.setCursor(tft.width()/2-142, 130);
    tft.print(usiaBulan);
    if (usiaBulan < 10){
      tft.setCursor(tft.width()/2-122, 130);
      tft.print("bulan");
    }else{
      tft.setCursor(tft.width()/2-112, 130);
      tft.print("bulan");
    }

    tft.setCursor(35, 150);
    tft.print("Jenis Kelamin :");
    tft.setCursor(tft.width()/2-70, 150);
    if(gender == BOY){
      tft.print("Laki-laki");
    }
    else if (gender == GIRL){
      tft.print("Perempuan");
    }

    tft.setCursor(35, 170); //160
    tft.print("Berat (kg)    :");
    tft.setCursor(tft.width()/2-90, 170);
    tft.print(bb);

    tft.setCursor(35, 190); //130
    tft.print("Tinggi (cm) :");
    tft.setCursor(tft.width()/2-90, 190);
    tft.print(tinggiBadan);

    tft.setCursor(35, 210); //220
    tft.print("Z-Score BBU");
    tft.setCursor(tft.width()/2-80, 210);
    tft.print(":");
    tft.setCursor(tft.width()/2-70, 210);
    tft.print(zscore_BBU);
    tft.setCursor(tft.width()/2-13, 210);
    tft.print(kategori_BBU);

    tft.setCursor(35, 230); //190
    if(usiaBulan < 24){
      tft.print("Z-Score PBU");
    }else if(usiaBulan >= 24){
      tft.print("Z-Score TBU");
    }
    tft.setCursor(tft.width()/2-80, 230);
    tft.print(":");
    tft.setCursor(tft.width()/2-70, 230);
    tft.print(zscore_TBU);
    tft.setCursor(tft.width()/2-13, 230);
    tft.print(kategori_TBU);

    tft.setCursor(35, 250);
    if(usiaBulan < 24){
      tft.print("Z-Score BBPB");
    }else if(usiaBulan >= 24){
      tft.print("Z-Score BBTB");
    }
    tft.setCursor(tft.width()/2-80, 250);
    tft.print(":");
    tft.setCursor(tft.width()/2-70, 250);
    tft.print(zscore_BBTB);
    tft.setCursor(tft.width()/2-13, 250);
    tft.print(kategori_BBTB);

    tft.setCursor(35, 270);
    tft.print("Z-Score IMTU");
    tft.setCursor(tft.width()/2-80, 270);
    tft.print(":");
    tft.setCursor(tft.width()/2-70, 270);
    tft.print(zscore_IMTU);
    tft.setCursor(tft.width()/2-13, 270); //+25
    tft.print(kategori_IMTU);
  }

  initButtonsRES();
  displayState = 17;
  break;

  case 17:
  // if (millis() - scanTime >= 50) {
  //   // Pressed will be set true if there is a valid touch on the screen
  //   bool pressed = tft.getTouch(&t_x, &t_y);
  //   scanTime = millis(); 
  //   if (pressed) {
  //     if (btnRES.contains(t_x, t_y)) {
  //       btnRES.press(true);
  //       btnRES.pressAction();
  //     }
  //     else {
  //       btnRES.press(false);
  //       btnRES.releaseAction();
  //     }
  //   }  
  // }
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();
    for (uint8_t b = 8; b <= 9; b++) {
      if (pressed4) {
        if (btn[b]->contains(t_x4, t_y4)) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }
  break;

  case 18:
  for (uint8_t b = 0; b < 15; b++) {
    if (pressed && key3[b].contains(t_x, t_y)) {
      key3[b].press(true);  // tell the button it is pressed
    } else {
      key3[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 15; b++) {

    if (b < 3) tft.setFreeFont(LABEL1_FONT);
    else tft.setFreeFont(LABEL2_FONT);

    if (key3[b].justReleased()) key3[b].drawButton();

    if (key3[b].justPressed()) {
      key3[b].drawButton(true);
      if (b >= 3) {
        if (indexCalibrate < NUM_LEN2) {
          calibrate[indexCalibrate] = keyLabel3[b][0];
          indexCalibrate++;
          calibrate[indexCalibrate] = 0; // zero terminate
        }
        status("", 0); // Clear the old status
      }
      if (b == 1) {
        calibrate[indexCalibrate] = 0;
        if (indexCalibrate > 0) {
          indexCalibrate--;
          calibrate[indexCalibrate] = 0;//' ';
        }
        status("", 0); // Clear the old status
      }
      if (b == 2) {
        if(language == ENGLISH){
          status("Sent value to serial port", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Mengirim ke serial port", TFT_CYAN);
        }
        
        //Serial.println(calibrate);
        simpangan = atof(calibrate);
        EEPROM.put(69,simpangan);
        EEPROM.commit();        
        displayState = 19;
      }
      if (b == 0) {
        if(language == ENGLISH){
          status("Value cleared", TFT_CYAN);
        }else if(language == INDONESIA){
          status("Data dihapus", TFT_CYAN);
        }
        
        indexCalibrate = 0; // Reset index to 0
        calibrate[indexCalibrate] = 0; // Place null in buffer
      }
    
      // Update the number display field
      tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
      tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
      tft.setTextColor(DISP_TCOLOR);     // Set the font colour

      // Draw the string, the value returned is the width in pixels
      int xwidth = tft.drawString(calibrate, DISP_X + 4, DISP_Y + 12);
      tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);
      
      delay(10); // UI debouncing
    }
  }
  break;

  case 19:
  tft.fillScreen(TFT_CYAN);
  tft.setFreeFont(LABEL1_FONT);
  tft.setTextSize(1);
  tft.setTextColor(TFT_BLACK);
  if(language == ENGLISH){
    tft.setCursor(tft.width()/2-90, 50);
    tft.print("Offset : ");
  }else if(language == INDONESIA){
    tft.setCursor(tft.width()/2-150, 50);
    tft.print("Simpangan : ");
  }
  
  tft.setCursor(tft.width()/2+10, 50);
  tft.print(simpangan);
  
  tft.setTextSize(1);
  tft.setFreeFont(LABEL2_FONT);
  tft.setCursor(80,tft.height()/2-30);
  if(language == ENGLISH){
    tft.print("Height (cm) : ");
  }else if(language == INDONESIA){
    tft.print("Tinggi (cm) : ");
  }
  

  initButtonSAVE();
  displayState = 20;
  movingAverage_Clear();
  break;

  case 20:
  newDistance = read_US(trigger, echo);
  ave = 0.0404*newDistance - 0.077;
  newAve = movingAverage(newDistance, 0);
  newAver = newAve + ave;
  newAverage = movingAverage1(newAver+simpangan, 0);

  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    //Serial.print(newDistance); Serial.print(" ");
    //Serial.println(newAverage);
    tft.fillRect(tft.width()/2+20, tft.height()/2-60, 120, 40, TFT_YELLOW);
    tft.setCursor(tft.width()/2+25, tft.height()/2-30);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    tft.print(newAverage, 1);
  }

  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed4 = tft.getTouch(&t_x4, &t_y4);
    scanTime = millis();
    for (uint8_t b = 10; b <= 11; b++) {
      if (pressed4) {
        if (btn[b]->contains(t_x4, t_y4)) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }
  break;

  case 21:
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    bool pressed4 = tft.getTouch(&t_x4, &t_y4);
    scanTime = millis();
    for (uint8_t b = 19; b <= 20; b++) {
      if (pressed4) {
        if (btn[b]->contains(t_x4, t_y4)) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }
  break;

  }
}

//-------------END of Display State------------
//-----------------END of LOOP-----------------

//-------------Calculate Variable--------------
//-----------END of Calculate Variable---------

float getFakeTemperature() {
  return micros()%20;
}
String fakeFunc1() {
  return "HELLOW";
}
float fakeFunc2() {
  return millis()%100;
}

void sendWA(String message) {      // user define function to send meassage to WhatsApp app
  //adding all number, your api key, your message into one complete urlWA
  // EEPROM.get4
  EEPROM.get(100,phoneNumber);
  EEPROM.get(200,apiKey);
  urlWA = "https://api.callmebot.com/whatsapp.php?phone=+" + String(phoneNumber) + "&text=" + urlencode(message) + "&apikey=" + String(apiKey);

  // postData(); // calling postData to run the above-generated url once so that you will receive a message.
  int httpCode;     // variable used to get the responce http code after calling api
  HTTPClient http;  // Declare object of class HTTPClient
  http.begin(urlWA);  // begin the HTTPClient object with generated url
  tft.setCursor(75, tft.height()/2-80);
  tft.setTextColor(TFT_BLACK);
  if(language == ENGLISH){
    tft.print("Sending Data to WhatsApp...");
  }else if(language == INDONESIA){
    tft.print("Kirim Data ke WhatsApp...");
  }
  httpCode = http.POST(urlWA); // Finaly Post the URL with this function and it will store the http code
  if (httpCode == 200) { 
    //Serial.println("Sent ok."); 
    if(language == ENGLISH){
      tft.setCursor(170, tft.height()/2-40); tft.print("Data Sent"); delay(2000); 
    }else if(language == INDONESIA){
      tft.setCursor(140, tft.height()/2-40); tft.print("Data Terkirim"); delay(2000); 
    } // Check if the responce http code is 200
  } else {
    Serial.println("Error."); tft.setCursor(170, tft.height()/2 - 40); tft.print("ERROR"); delay(2000);
  } // if response HTTP code is not 200 it means there is some error.
  http.end();          // After calling API end the HTTP client object.
  // displayState = 15;
  delay(1000);
}

// void urlencodee(String &str) {
//   str.replace(" ", "%20");
//   str.replace("&", "%26");
//   str.replace("#", "%23");
//   str.replace("=", "%3D");
//   str.replace("+", "%2B");
//   str.replace(",", "%2C");
//   str.replace("/", "%2F");
//   str.replace("?", "%3F");
//   str.replace(":", "%3A");
//   str.replace(";", "%3B");
//   str.replace("<", "%3C");
//   str.replace(">", "%3E");
//   str.replace("@", "%40");
//   str.replace("[", "%5B");
//   str.replace("]", "%5D");
//   str.replace("\\", "%5C");
//   str.replace("^", "%5E");
//   str.replace("_", "%5F");
//   str.replace("`", "%60");
//   str.replace("{", "%7B");
//   str.replace("|", "%7C");
//   str.replace("}", "%7D");
//   str.replace("~", "%7E");
// }

String urlencodee(String &str) {
  String tempStr = str;
  tempStr.replace(" ", "%20");
  tempStr.replace("&", "%26");
  tempStr.replace("#", "%23");
  tempStr.replace("=", "%3D");
  tempStr.replace("+", "%2B");
  tempStr.replace(",", "%2C");
  tempStr.replace("/", "%2F");
  tempStr.replace("?", "%3F");
  tempStr.replace(":", "%3A");
  tempStr.replace(";", "%3B");
  tempStr.replace("<", "%3C");
  tempStr.replace(">", "%3E");
  tempStr.replace("@", "%40");
  tempStr.replace("[", "%5B");
  tempStr.replace("]", "%5D");
  tempStr.replace("\\", "%5C");
  tempStr.replace("^", "%5E");
  tempStr.replace("_", "%5F");
  tempStr.replace("`", "%60");
  tempStr.replace("{", "%7B");
  tempStr.replace("|", "%7C");
  tempStr.replace("}", "%7D");
  tempStr.replace("~", "%7E");
  return tempStr;
}
