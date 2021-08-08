#include <Arduino.h>
#include <VescUart.h>
#include <LiquidCrystal.h>

#include "defines.h"

/** Inicializing libraries */
VescUart UART;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte amhour[] = {B00100, B01010, B01110, B01010, B00000, B01010, B01110, B01010};
byte kmh[] = {B10010, B10100, B11000, B10100, B10010, B00100, B00111, B00101};
byte km[] = {B10010, B10100, B11000, B10100, B10010, B11111, B10101, B10101};
byte bat[] = {B00000, B01110, B10001, B10001, B11111, B11111, B11111, B11111};
byte percentage[] = {B00000, B11001, B11010, B00100, B01011, B10011, B00000, B00000};
byte spark[] = {B11100, B01110, B00111, B01110, B11100, B01100, B00110, B00011};
byte motoramp[] = {B10001, B11011, B10101, B10001, B01110, B01010, B01110, B01010};
byte batteryamp[] = {B11000, B10100, B11100, B10100, B11010, B00101, B00111, B00101};
byte dutycycle[] = {B11000, B10100, B10100, B11000, B00111, B01000, B01000, B00111};
byte mosfettemp[] = {B11011, B10101, B10001, B10001, B00000, B00111, B00010, B00010};
byte degrees[] = {B00000, B11000, B11000, B00111, B01000, B01000, B01000, B00111};

float temFET, tempMotor, avgMotorCurrent, avgInputCurrent, dutyCycleNow, inpVoltage, ampHours, ampHoursCharged, velocity, batterypercent, distance;
long rpm, tachometer, tachometerAbs;
bool statelightTOP = false;
bool statelightBOT = false;
bool stateSensorPAS = false;
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long previousMillisValues = 0;
int count = 0;
float cadenceRev = 0;
float cadenceRPM = 0;

void getVESCvalues()
{
  if (UART.getVescValues())
  {
    /** Get all data from UART */
    temFET = UART.data.tempFET;
    // tempMotor = UART.data.tempMotor;
    avgMotorCurrent = UART.data.avgMotorCurrent;
    avgInputCurrent = UART.data.avgInputCurrent;
    dutyCycleNow = UART.data.dutyCycleNow * 100;
    inpVoltage = UART.data.inpVoltage;
    // ampHours = UART.data.ampHours;
    // ampHoursCharged = UART.data.ampHoursCharged;
    rpm = (UART.data.rpm) / 15; // Replace 15 for the number of motor poles
    // tachometer = (UART.data.tachometer) / 45;       // '45' is the number of poles multiply by 3
    tachometerAbs = (UART.data.tachometerAbs) / 45; // '45' is the number of poles multiply by 3
  }
  else
  {
    // lcd.clear();
    // lcd.setCursor(0, 0);
    // lcd.print("No hay valores");
  }
  // delay(50);
}

void setup()
{

  previousMillis = millis();
  previousMillisValues = millis();

  Serial.begin(115200);
  UART.setSerialPort(&Serial);

  /** LCD Settings */
  pinMode(contrast, OUTPUT);
  analogWrite(contrast, valueofContrast);
  lcd.begin(cols, rows);

  /** Sensor PAS pin */
  pinMode(sensorPin, INPUT);

  /** Button setting (internal pullup, D8, D7) */
  PORTB |= B00000001; //d8 HIGH to enable pullup
  PORTD |= B10000000; //d7 HIGH to enable pullup

  /** PCINT settings */
  PCICR |= B00000101;  //enable PCIE0 and PCIE2
  PCMSK0 |= B00000001; //enable interrupt PCINT0 (pin 8)
  PCMSK2 |= B10000000; //enable interrupt PCINT23 (pin7)

  /** 2n2222 settings */
  DDRB |= B00000110; //set pins 9 and 10 as OUTPUT

  // cli(); //stop interrupts
  // /** Reset control register */
  // TCCR1A = 0;
  // TCCR1B = 0;
  // /** Set prescalar to desired value --> 256 */
  // TCCR1A = B00000100;
  // TCNT1 = 0; //reset timer back to 0;
  // /** Enable compare match mode on register A, set OCIE1A to 1*/
  // TIMSK1 |= B00000010;
  // OCR1B = 31250;

  // sei(); //enable back interrupts

  lcd.setCursor(4, 1);
  lcd.print("Quero e-Bike");
  delay(3000);
  lcd.clear();

  lcd.createChar(0, motoramp);
  lcd.createChar(1, batteryamp);
  lcd.createChar(2, dutycycle);
  lcd.createChar(3, percentage);
  lcd.createChar(4, bat);
  lcd.createChar(5, mosfettemp);
  lcd.createChar(6, degrees);
  lcd.createChar(7, kmh);

  // /** First row */
  lcd.setCursor(_motorCurrent, 0);
  lcd.write(byte(0)); // motor amp
  lcd.print(":");
  // lcd.setCursor(_motorCurrent + 6, 0);
  // lcd.print("A");
  lcd.setCursor(_batteryPercentage, 0);
  lcd.write(byte(4)); // battery
  lcd.print(":");
  lcd.setCursor(_batteryPercentage + 5, 0);
  lcd.write(byte(3)); // percent

  // /** Second row */
  lcd.setCursor(_velocity, 1);
  lcd.print("V");
  lcd.print(":");
  // lcd.setCursor(_velocity + 4, 1);
  // lcd.write(byte(7)); //kmh
  lcd.setCursor(_mosfetTem, 1);
  lcd.write(byte(5)); // Mosfet tem
  lcd.print(":");
  // lcd.setCursor(_mosfetTem + 6, 1);
  // lcd.write(byte(6)); //degrees

  // /** Third row */
  lcd.setCursor(_battCurrent, 2);
  lcd.write(byte(1)); //battery amp
  lcd.print(":");
  // lcd.setCursor(_battCurrent + 6, 2);
  // lcd.print("A");
  lcd.setCursor(_distance, 2);
  lcd.print("D");
  lcd.print(":");
  // lcd.setCursor(_distance + 6, 2);
  // lcd.print("K");

  // /** Fourth row */
  lcd.setCursor(_dutyCicle, 3);
  lcd.write(byte(2)); //dutycycle
  lcd.print(":");
  // lcd.setCursor(_dutyCicle + 6, 3);
  // lcd.write(byte(4)); //percent
  // getVESCvalues();
}

void printFloats(float _variable, byte _define, byte _row, char const *_x, byte _y)
{

  if (_variable >= 0)
  {
    if ((String(int(_variable))).length() == 1)
    {
      lcd.setCursor(_define + 2, _row);
      lcd.print(" ");
      lcd.print(_variable); //Motor Current
      lcd.setCursor(_define + 6, _row);
      if (_y == 8)
      {
        lcd.print(_x);
      }
      else
      {
        lcd.write(byte(_y));
      }
    }
    else if ((String(int(_variable))).length() == 2)
    {
      lcd.setCursor(_define + 2, _row);
      lcd.print(_variable); //Motor Current
      lcd.setCursor(_define + 6, _row);
      if (_y == 8)
      {
        lcd.print(_x);
      }
      else
      {
        lcd.write(byte(_y));
      }
    }
    else
    {
      // Serial.print("Error al imprimir " + String(variable));
    }
  }
}

void printInts(float _variable, byte _define, byte _row)
{

  // if ((String(int(_variable))).length() == 3)
  // {
  //   lcd.setCursor(_define + 2, _row);
  //   lcd.print(int(_variable)); // Battery Percent
  // }
  if ((String(int(_variable))).length() == 2)
  {
    lcd.setCursor(_define + 2, _row);
    lcd.print(int(_variable)); // Battery Percent
  }
  else if ((String(int(_variable))).length() == 1)
  {
    lcd.setCursor(_define + 2, _row);
    lcd.print(" ");
    lcd.print(int(_variable)); // Battery Percent
  }
  else
  {
    // Serial.println("Error al imprimir " + String(variable));
  }
}

int sensorRead()
{
  byte value;
  int _throttle = 127;
  unsigned int valueRAW;
  currentMillis = millis();

  valueRAW = analogRead(sensorPin);
  if (valueRAW <= 63)
  {
    value = 0;
    stateSensorPAS = false;
  }
  else if (valueRAW > 63)
  {
    value = 1;
  }

  if (value == 1 && stateSensorPAS == false)
  {
    count += 1;
    stateSensorPAS = true;
  }

  if (currentMillis > (previousMillis + interval))
  {

    if(count >= 3){
      _throttle = 255;
    }

    count = 0;
    previousMillis = millis();
  }

  return _throttle;
}

void setNunchuckValue(int _throttle)
{
  UART.nunchuck.valueY = _throttle;
  UART.setNunchuckValues();
  // if(_cadence >= 10) {
  //   UART.setDuty(0.5);
  //   // UART.setRPM(4822);
  // } else if (_cadence < 10){
  //   // UART.setDuty(0.0);
  //   // UART.setRPM(0);
  // }
  lcd.setCursor(0,1);
  lcd.print(_throttle);
}

void loop()
{

  int throttle = sensorRead();

  if (millis() > (previousMillisValues + intervalValues))
  {
    getVESCvalues();
    /** Parameters to calculate */
    velocity = rpm * 60 * 2 * 3.142 * (0.66 / 2) * 1E-3; //rpm * 2PI * 60 * wheel radius * 10E-3
    distance = tachometerAbs * 3.142 * 1E-3 * 0.66;
    batterypercent = (inpVoltage - 33) * (100 - 0) / (40.5 - 33) + 0; // change the values of 33 and 40.5 by the min and max voltage value of your battery.
    /** Display values to lcd
   * If there is unit, write 8 in byte.
  */
    printFloats(avgMotorCurrent, _motorCurrent, 0, "A", 8);
    printFloats(temFET, _mosfetTem, 1, "A", 6);
    printFloats(avgInputCurrent, _battCurrent, 2, "A", 8);
    printFloats(distance, _distance, 2, "K", 8);
    printFloats(dutyCycleNow, _dutyCicle, 3, "A", 3);
    printInts(batterypercent, _batteryPercentage, 0);
    printInts(velocity, _velocity, 1);

    // lcd.setCursor(_mosfetTem+2,1);
    // lcd.print(temFET);

    previousMillisValues = millis();
  }
    setNunchuckValue(throttle);

  /** BARRA DEL DUTY CICLE */
  // byte unitDC = dutyCycleNow / 10;

  // for (byte i = 0; i <= unitDC; i++)
  // {
  //   lcd.setCursor(i + _dutyCicle, 3);
  //   lcd.print("-");
  // }

  // for (byte i = unitDC; i <= 0; i--)
  // {
  //   lcd.setCursor(i + _dutyCicle, 3);
  //   lcd.print(" ");
  // }
}

ISR(PCINT0_vect)
{
  /** 
   * If the Button is on LOW --> Light enable if it disable and light disable if it enable
                         HIGH --> Do nothing
`*/
  if (!bit_is_set(PINB, 0))
  {
    if (statelightTOP == false)
    {
      statelightTOP = true;
      PORTB |= B00000100; // PIN 10 HIGH
    }
    else if (statelightTOP == true)
    {
      statelightTOP = false;
      PORTB &= !B00000100; // PIN 10 LOW
    }
  }
}

ISR(PCINT2_vect)
{
  /** 
   * If the Button is on LOW --> Light enable if it disable and light disable if it enable
                         HIGH --> Do nothing
`*/
  if (!bit_is_set(PIND, 7))
  {
    if (statelightBOT == false)
    {
      statelightBOT = true;
      PORTB |= B00000010; // PIN 9 HIGH
      Serial.println("Luz pin 7 encendida");
    }
    else if (statelightBOT == true)
    {
      statelightBOT = false;
      PORTB &= !B00000010; // PIN 9 LOW
      Serial.println("Luz pin 7 apagada");
    }
  }
}

// ISR(TIMER1_COMPA_vect)
// {
//   // getVESCvalues();
//   TCNT1 = 0; //reset timer back to 0;
// }