/******************************************************************
 * Teste_Malha_Fechada
 * Controle PID com interface FrontEnd, acionamento de RSS e sensor PT100 via SPI
 * By Franco Picarelli (Original pieces by Brett Beauregard and J. Steinlage) 
 * December 2014
 ******************************************************************/

#include <SPI.h>
#include <PlayingWithFusion_MAX31865.h>              // core library
#include <PlayingWithFusion_MAX31865_STRUCT.h>       // struct library

// Define States
#define Mostura               1
#define Fervura               2

// CS pin used for the connection with the sensor
// other connections are controlled by the SPI library)
const int CS0_PIN = 9;
const int CS1_PIN = 10;
const int Pin_Falha_CH0 = 44;
const int Pin_Falha_CH1 = 45;
const int Pin_Selecao = 11;
const int Output_Mostura = 4;
const int Output_Fervura = 5;

//Define Variables we'll be connecting to
double Setpoint;

double Resist_CH0, SensorTemperaturaFervura, Resist_CH1, SensorTemperaturaMostura;

PWFusion_MAX31865_RTD rtd_ch0(CS0_PIN);
PWFusion_MAX31865_RTD rtd_ch1(CS1_PIN);

// Statemachine State variable and initial value
byte State = Mostura;

void setup()
{
  //initialize the serial link with processing
  Serial.begin(9600);
  
  // setup for the the SPI library:
  SPI.begin();                            // begin SPI
  SPI.setClockDivider(SPI_CLOCK_DIV16);   // SPI speed to SPI_CLOCK_DIV16 (1MHz)
  SPI.setDataMode(SPI_MODE3);             // MAX31865 works in MODE1 or MODE3
  
  // initalize the pins
  pinMode(CS0_PIN, OUTPUT);
  pinMode(CS1_PIN, OUTPUT);
  pinMode(Pin_Falha_CH0, OUTPUT);
  pinMode(Pin_Falha_CH1, OUTPUT);  
  pinMode(Output_Mostura, OUTPUT);
  pinMode(Output_Fervura, OUTPUT);
  pinMode(Pin_Selecao, INPUT);  
  
  rtd_ch0.MAX31865_config();
  rtd_ch1.MAX31865_config();

  
  delay(3000);
}

void serialController(char command)
{
  switch(command)
  {
    case 'm': // Mostura
      State = Mostura;
      Serial.println("State: Mostura");
    break;
    case 'f': // Fervura
      State = Fervura;
      Serial.println("State: Fervura");
    break;
  } 
}

void loop()
{
  Setpoint = 40;
  static struct var_max31865 RTD_CH0;
  static struct var_max31865 RTD_CH1;
  
  RTD_CH0.RTD_type = 1;                         // un-comment for PT100 RTD
  // RTD_CH0.RTD_type = 2;                        // un-comment for PT1000 RTD
  RTD_CH1.RTD_type = 1;                         // un-comment for PT100 RTD
  // RTD_CH0.RTD_type = 2;                        // un-comment for PT1000 RTD
  
  struct var_max31865 *rtd_ptr;
  rtd_ptr = &RTD_CH0;
  rtd_ch0.MAX31865_full_read(rtd_ptr);          // Update MAX31855 readings 
  
  rtd_ptr = &RTD_CH1;
  rtd_ch1.MAX31865_full_read(rtd_ptr);          // Update MAX31855 readings 
  
  // Aquisicao da temperatura do canal 0
  if(0 == RTD_CH0.status)                       // no fault
  {
     Resist_CH0 = (double)RTD_CH0.rtd_res_raw * 400 / 32768; // Resistencia canal 0 em Ohms
     SensorTemperaturaFervura = ((double)RTD_CH0.rtd_res_raw / 32) - 256;    // Temperatura canal 0 em graus Celsius
  }
  else // tratamento de falha do canal 0
  {
      digitalWrite(Pin_Falha_CH0, HIGH);
      
  }  // end of fault handling
  
  // Aquisicao da temperatura do canal 1
  if(0 == RTD_CH1.status)                       // no fault
  {
     Resist_CH1 = (double)RTD_CH1.rtd_res_raw * 400 / 32768; // Resistencia canal 0 em Ohms
     SensorTemperaturaMostura = ((double)RTD_CH1.rtd_res_raw / 32) - 256;    // Temperatura canal 0 em graus Celsius
  }
  else // tratamento de falha do canal 1
  {
      digitalWrite(Pin_Falha_CH1, HIGH);
      
  }  // end of fault handling
  
    switch (State) 
  {
    case Mostura:
          digitalWrite(Output_Fervura, 0);
          if (SensorTemperaturaMostura < Setpoint) {

          digitalWrite(Output_Mostura, 1);      
        }
        else {
          digitalWrite(Output_Mostura, 0);
        }
        Serial.println(SensorTemperaturaMostura);
      break;
    case Fervura:
          digitalWrite(Output_Mostura, 0);
          if (SensorTemperaturaFervura < Setpoint) {

          digitalWrite(Output_Fervura, 1);      
        }
          else {
            digitalWrite(Output_Fervura, 0);
          }
          Serial.println(SensorTemperaturaFervura);
      break;   
    }

  if(Serial.available())
  { 
    serialController(Serial.read());
  }
}
