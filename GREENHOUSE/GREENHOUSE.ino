#include <SoftwareSerial.h>
#include <SIM900.h> 
#include <sms.h>  

#define PRINT_TEMPERATURE(sensor,cvalue,ovalue)   Serial.print(#sensor" => ");\
                     Serial.print(" "#cvalue" : ");\
                     Serial.print(cvalue);\
                     Serial.print("*C");\
                     Serial.print(" "#ovalue" : ");\
                     Serial.print(ovalue);\
                     Serial.print("*C      ");\
                     Serial.print(millis());\
                     Serial.println(" milliseconds\n\n");
#define PRINT_MOISTURE(sensor,cvalue,ovalue)   Serial.print(#sensor" => ");\
                     Serial.print(" "#cvalue" : ");\
                     Serial.print(cvalue);\
                     Serial.print("% ");\
                     Serial.print(" "#ovalue" : ");\
                     Serial.print(ovalue);\
                     Serial.print("%      ");\
                     Serial.print(millis());\
                     Serial.println(" milliseconds\n\n");                    
                     
#define CALCULATE_TEMPERATURE(analogValue) (((float) analogValue *0.48828125f) -2.0f)
#define CALCULATE_MOISTURE(analogValue) ((((float) analogValue )*0.09765625f))

#define ON  (1)
#define OFF (0)

#define ThresholdTemperatureLevel (28.0f)
#define CriticalTemperatureLevel (32.0f)
#define ThresholdMoistureLevel (70.0f)
#define CriticalMoistureLevel  (75.0f)

int TEMPERATURESENSOR = A0;  
int MOISTURESENSOR    = A1;
int FAN_CONNECTION    = 12;
int PUMP_CONNECTION   = 13;
char Number[] ="+256702396221";
char mBuffer[100];
  
float OldTemperatureLevel,OldMoistureLevel;
long lastMeasuredtempTime,lastMeasuredmoistTime;
unsigned char FANSTATUS = OFF, PUMPSTATUS = OFF, sendTEMPALART = OFF,sendMOISALART = OFF; 

void setup() 
{
   Serial.begin(9600);
  Serial.println("WELCOME TO: AUTOMATED GREENHOUSE IRRIGATION SYSTEM\n\n"); 
  Serial.println("Initailising Components:\n");
  // initialize the digital pin as an output.
  Serial.println("MODEM\n");
  Serial.println("Testing GSM/GPRS Modem...");
  if( gsm.begin(9600) ){
   Serial.println("Connected");
  }
  else{
   Serial.println("Failed to connect"); 
  }
   
  Serial.println("ACTUATORS\n1. FAN");
 
  pinMode(FAN_CONNECTION, OUTPUT);
  SwitchFAN(OFF); 
   Serial.println("2. PUMP");
   // initialize the digital pin as an output.
  pinMode(PUMP_CONNECTION, OUTPUT);
  SwitchPUMP(OFF);
  //delay (10000);
  //sendTEMPALART = ON;
   
}

void loop() 
{
    sendTEMPALART = getTemperature();
    sendMOISALART = getMoisture();
    sendSMS();
}

//-------------------------------------------------------------------------------------------------------------------------

unsigned char getTemperature()
{
  unsigned char sendHighTempAlart = OFF;
  float CurrentTemperatureLevel;
  if( ( millis() - lastMeasuredtempTime ) > 5000)
  {
      CurrentTemperatureLevel = CALCULATE_TEMPERATURE(analogRead(TEMPERATURESENSOR));
      
      if(CurrentTemperatureLevel != OldTemperatureLevel)
      {
        
          if( ( ( CurrentTemperatureLevel - ThresholdTemperatureLevel ) >= 1.0f) && (FANSTATUS == OFF))
          {
             //Turn ON fan here 
              SwitchFAN(ON);   
          }
          else if ( ( ( ThresholdTemperatureLevel - CurrentTemperatureLevel ) >=  1.0f ) && (FANSTATUS == ON))
          {
            //Turn OFF fan here 
             SwitchFAN(OFF);   
          } 
          
          sendHighTempAlart = OFF;
      }
      else if( CurrentTemperatureLevel >= CriticalTemperatureLevel)
      {
        // SEND SMS here
        sendHighTempAlart = ON;
      }
  lastMeasuredtempTime = millis(); 
  PRINT_TEMPERATURE(TEMPERATURESENSOR,CurrentTemperatureLevel,OldTemperatureLevel); 
  OldTemperatureLevel = CurrentTemperatureLevel;
   
  }
    
  
  return sendHighTempAlart;
}


//----------------------------------------------------------------------------------------------------------------------

unsigned char getMoisture()
{
  unsigned char sendHighMoistureAlart = OFF; 
  float CurrentMoistureLevel;
  if( ( millis() - lastMeasuredmoistTime ) > 5000)
  {
      CurrentMoistureLevel = CALCULATE_MOISTURE(analogRead(MOISTURESENSOR));
      
      if(CurrentMoistureLevel != OldMoistureLevel)
      {
        
          if( ( CurrentMoistureLevel > ThresholdMoistureLevel ) && (PUMPSTATUS == ON))
          {
             //Turn ON pump here
             SwitchPUMP(OFF); 
              
          }
          else if ( (  CurrentMoistureLevel < 30 ) && (PUMPSTATUS == OFF))
          {
            //Turn OFF pump here
             SwitchPUMP(ON);
          } 
          
          sendHighMoistureAlart = OFF;
      }
      else if( CurrentMoistureLevel > CriticalMoistureLevel)
      {
        // SEND SMS here
        sendHighMoistureAlart = ON;
      }
  lastMeasuredmoistTime = millis(); 
  PRINT_MOISTURE(MOISTURESENSOR,CurrentMoistureLevel,OldMoistureLevel); 
  OldMoistureLevel = CurrentMoistureLevel;
   
  }
    
  
  return sendHighMoistureAlart;
}

//-------------------------------------------------------------------------------------------------------------------------



void SwitchFAN(unsigned char State)
{
     Serial.print(" SWITCHING - FAN: ");
     if(State == OFF)
    {
      digitalWrite(FAN_CONNECTION, LOW); 
      Serial.println("OFF"); 
    }
    else
    {
      digitalWrite(FAN_CONNECTION, HIGH); 
      Serial.println("ON");
    }
    
      FANSTATUS = State; 
}

//-------------------------------------------------------------------------------------------------------------------------

void SwitchPUMP(unsigned char State)
{ 
     Serial.print(" SWITCHING - PUMP: ");
     if(State == OFF)
    {
      digitalWrite(PUMP_CONNECTION, LOW); 
      Serial.println("OFF"); 
    }
    else
    {
      digitalWrite(PUMP_CONNECTION, HIGH); 
      Serial.println("ON");
    }
    
      PUMPSTATUS = State; 
}

//-------------------------------------------------------------------------------------------------------------------------
 
unsigned char sendSMS()
{
  
  char tempBuffer[10];
  char mstatus = 0;
  
  
  //Check if Over temperature alert message is ready for sending 
  if(sendTEMPALART == ON)
  {
   Serial.print("SENDING SMS ALERT FOR - ");
   Serial.println("OVER TEMPERATURE"); 
   ultoa(OldTemperatureLevel,tempBuffer,10);
   //create message
   strcpy(mBuffer,"OVER TEMPERATURE ALERT: Value => ");
   strcat(mBuffer,tempBuffer);
   strcat(mBuffer,"*C ");
   //send message
   mstatus = mSMS.SendSMS(Number, mBuffer);
   //check if message is send successfully otherwise reschedule sending 
   sendTEMPALART =  OFF; 
   // print sent message on debug window 
   Serial.print("To: "); 
   Serial.print(Number); 
   Serial.println("  MSG: "); 
   Serial.println( mBuffer); 
   
  }
 
  //Check if excessive Moisture alert message is ready for sending 
  mstatus = 0;
  if(sendMOISALART == ON)
  { 
   Serial.print("SENDING SMS ALART FOR - ");
   Serial.println("EXCESSIVE MOISTURE");
   ultoa(OldMoistureLevel,tempBuffer,10);
   //create message
   strcpy(mBuffer,"EXCESSIVE MOISTURE ALERT: Value => ");
   strcat(mBuffer,tempBuffer);
   strcat(mBuffer,"% ");
    
   //send message
   mstatus = mSMS.SendSMS(Number, mBuffer);
   //check if message is send successfully otherwise reschedule sending 
   sendMOISALART = OFF  ; 
   // print sent message on debug window
   Serial.print("To: "); 
   Serial.print(Number); 
   Serial.println("  MSG: "); 
   Serial.println( mBuffer); 
  }
}

 
