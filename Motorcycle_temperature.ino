// Nokia 5110 LCD-Display (84x48 Bildpunkte)
// Setup to controll fan on rd350 -85 (works on different temperature sensors but need to change
// THERMISTORNOMINAL AND BCOEFFICIENT, (SERIESRESISTOR and ADCREF/VIN)
// Using Arduino Nano and 5110 display.

#include <SPI.h>

#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// D7 - Serial clock out (CLK oder SCLK)
// D6 - Serial data out (DIN)
// D5 - Data/Command select (DC oder D/C)
// D4 - LCD chip select (CE oder CS)
// D3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

//Contrast and brightness
#define CONTRAST 40
#define LEDPIN 9
#define BRIGHTNESS 80 //0-100%

// which analog pin to connect
#define THERMISTORPIN A0         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 455      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 10
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 4150
// the value of the 'other' resistor
#define SERIESRESISTOR 4700    
int samples[NUMSAMPLES];
int samples2[NUMSAMPLES];

// Values for voltage divider
#define ADCREF 3.3
#define VIN 5

// Fan Relay
#define RELAYPIN 2
#define FANON 102
#define FANOFF 98

// DEBUG
#define DEBUG false


float maxTemp=-50.0;

void setup()   {

  // Display initialisieren
  display.begin();

  // Kontrast setzen
  display.setContrast(CONTRAST);
  display.clearDisplay();   // clears the screen and buffer

  // Set Brightness
  pinMode(LEDPIN, OUTPUT);
  analogWrite(LEDPIN, (-(BRIGHTNESS-100))*2.55);

  analogReference(EXTERNAL); //Uses external voltage supplied to Aref

  // FAN Relay
  pinMode(RELAYPIN, OUTPUT);

  welcomeText();
}

void loop() {
  uint8_t i;
  uint8_t j;
  float average = 0;
  float average2 = 0;

  for (i=0; i<NUMSAMPLES; i++) {
    // take N samples in a row, with a slight delay
    for (j=0; j< NUMSAMPLES; j++) {
     samples2[j] = analogRead(THERMISTORPIN);
     delay(10);
    }
   
    // average all the samples out
    for (j=0; j< NUMSAMPLES; j++) {
     average2 += samples2[j];
    }
    average2 /= NUMSAMPLES;
    samples[i] = average2;
  }
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
  
  // convert the value to voltag
  float voltage = average;
  voltage = ADCREF / 1023 * voltage;

  // convert the value to resistance
  float ohm = voltage;
  ohm = VIN / ohm - 1;
  ohm = SERIESRESISTOR / ohm;

  float steinhart = ohm;
  if (steinhart == 0) steinhart = 1;

  steinhart = steinhart / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C

  if (steinhart > FANON) digitalWrite(RELAYPIN, HIGH);
  else if (steinhart < FANOFF) digitalWrite(RELAYPIN, LOW);

  if (steinhart > maxTemp) maxTemp = steinhart; 
  
  if (DEBUG) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(BLACK);
    
    display.print("ADC:      ");
    display.println(average,0);
  
    display.print("Voltage: ");
    display.println(voltage,2);
    
    display.print("Ohm:     ");
    display.println(ohm,0);

    display.print("Temp: ");
    display.print(steinhart,1);
    display.println(" C");

    display.print("Max:  ");
    display.print(maxTemp,1);
    display.println(" C");
  }
  else {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(BLACK);

    display.println(steinhart,1);

    display.print("M:");
    display.println(maxTemp,1);
  }
  display.display();
}

void welcomeText(void){
  display.setTextSize(1);
  set_text(5,0,"Temperature!",BLACK);
  delay(500);
  
  display.drawLine(7,11,77,11,BLACK);
  display.display();
  delay(500);
  
  display.drawCircle(8,23,5,BLACK);
  display.display();
  delay(500);
  
  display.fillCircle(11,26,5,BLACK);
  display.display();
  delay(500);
  
  display.drawRect(25,18,10,10,BLACK);
  display.display();
  delay(500);

  display.fillRect(28,21,10,10,BLACK);
  display.display();
  delay(500);

  display.drawRoundRect(47,18,10,10,2,BLACK);
  display.display();
  delay(500);
  
  display.fillRoundRect(50,21,10,10,2,BLACK);
  display.display();
  delay(500);
  
  display.drawTriangle(68,18,68,28,78,23,BLACK);
  display.display();
  delay(500);

  display.fillTriangle(71,21,71,31,81,26,BLACK);
  display.display();
  delay(500);
  
//  // Ein kleines bisschen Scroll-Text-Magie
//  int x=0;
//  for(int i=0;i<(5.6*8);i++){
//    set_text(x,40,"blog.simtronyx.de",BLACK);
//    delay(i==0?1000:100);
//    if(i<(5.6*8)-1)set_text(x,40,"blog.simtronyx.de",WHITE);
//    if((i)<(2.74*8))x-=1;else x+=1;
//  }
//  delay(250);
  
  display.clearDisplay();
}

void set_text(int x,int y,String text,int color){
  
  display.setTextColor(color); // Textfarbe setzen, also Schwarz oder Weiss
  display.setCursor(x,y);      // Startpunkt-Position des Textes
  display.println(text);       // Textzeile ausgeben
  display.display();           // Display aktualisieren
}
