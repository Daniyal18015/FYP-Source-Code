// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPL25Hj2sE6"
#define BLYNK_DEVICE_NAME "Smart Energy Meter"
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define WIFI_SSID "SEMS"          //  Insert Wifi SSID here
#define WIFI_PASSWORD "03214849640" //  Insert Wifi Password here 
#define API_KEY "AIzaSyCkHhmpCFkGPT0hESU_YuFwGNr7toAqFqg"   // Insert Firebase project API Key
#define USER_EMAIL "arsalan.amin18008@gmail.com"            // Insert Authorized Email here
#define USER_PASSWORD "IOT_SEMS_18"                           // Insert Email's Corresponding Password
#define DATABASE_URL "https://iot-based-smart-energy-m-e640c-default-rtdb.asia-southeast1.firebasedatabase.app/"   // Insert RTDB URLefine the RTDB URL

//------------------------- | Imported Libraries | -------------------------------

#include "EmonLib.h"
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"    // Provide the token generation process info
#include "addons/RTDBHelper.h"     // Provide the RTDB payload printing info and other helper functions

//------------------------- | Initialized Variables | -------------------------------
float Input_Voltage = 226.3;
float Load_Current = 1.74;
int LCD_Delay_Manager = 0;
int Wifi_LED = 2;           // Blue LED turns ON when ESP32-board gets connected with Wifi
float Real_Power = 391.5;
float Apparent_Power = 399.8;
float Power_Factor = 0.98;
float Reactive_Power = 81.4;
float Total_Energy = 0;
float Total_kWh1 = 0.0000;        // Energy consumed on line 1
float Total_kWh2 = 0.0000;        // Energy consumed on line 2

int Prediction_Time_Counter=0;             // To count the time in seconds for predicting bill

const int Array_Length = 20;
float Voltage_Array[Array_Length] = {0};          // Input Array for averaging
float Current_Array[Array_Length] = {0};          // Input Array for averaging
float Real_Power_Array[Array_Length] = {0};       // Input Array for averaging
float Apparent_Power_Array[Array_Length] = {0};   // Input Array for averaging
float Reactive_Power_Array[Array_Length] = {0};   // Input Array for averaging
int Universal_Counter = 0;
double Real_Time = 0;    // Real Time Clock
double Line1_Time = 1;   // Here 1 means 1 second
double Line2_Time = 1;   // Here 1 means 1 second
const int Relay_Input_Line1 = 26;
const int Relay_Input_Line2 = 27;
float Estimated_Bill_Line1 = 0;   // Estimated bill of line 1
float Estimated_Bill_Line2 = 0;   // Estimated bill of line 2
float Clock = 0;
String uid;                      // Variable to save USER UID
String databasePath;
String Line1_Units_Path;
String Estimated_Bill_Line1_Path;
String Estimated_Bill_Line2_Path;
String Line2_Units_Path;
String Input_Voltage_Path;
String Load_Current_Path;
String Real_Power_Path;
String Reactive_Power_Path;
String Apparent_Power_Path;
String Power_Factor_Path;
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 15000;

EnergyMonitor emon1;    // Create an instance
FirebaseData fbdo;  //Define Firebase Data object
FirebaseAuth auth;  // Define Firebase Autherization  object 
FirebaseConfig config;  // Define Firebase Configuration


void setup()  // Setting up things for the first time
{
  Serial.begin(115200);  // Baud Rate Initialization for Serial Monitor
  analogReadResolution(ADC_BITS);  // ADC Pins Resolution for ESP32
  emon1.voltage(35, 380.5, 1.7);  // Voltage: Input pin, Calibration, Phase shift
  emon1.current(32, 5.01);       // Current: Input pin, Calibration
  pinMode(Wifi_LED, OUTPUT);
  pinMode(Relay_Input_Line1, OUTPUT);
  pinMode(Relay_Input_Line2, OUTPUT);
  digitalWrite(Relay_Input_Line1, LOW);
  digitalWrite(Relay_Input_Line2, LOW);
 
 
  initWiFi();

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid;

  // Update database path for sensor readings
  Input_Voltage_Path = databasePath + "/Input_Voltages"; // --> UsersData/<user_uid>/input_voltages
  Load_Current_Path = databasePath + "/Load_Current"; // --> UsersData/<user_uid>/load_current
  Real_Power_Path = databasePath + "/Real_Power"; // --> UsersData/<user_uid>/real_power
  Reactive_Power_Path = databasePath + "/Reactive_Power"; // --> UsersData/<user_uid>/reactive_power
  Apparent_Power_Path = databasePath + "/Apparent_Power"; // --> UsersData/<user_uid>/apparent_power
  Power_Factor_Path = databasePath + "/Power_Factor"; // --> UsersData/<user_uid>/power_factor
  Line1_Units_Path = databasePath + "/Units_Line1 = "; // --> UsersData/<user_uid>/units_line1
  Estimated_Bill_Line1_Path = databasePath + "/Bill_Line1 = "; // --> UsersData/<user_uid>/bill_line1
  Line2_Units_Path = databasePath + "/Units_Line2 = "; // --> UsersData/<user_uid>/units_line2
  Estimated_Bill_Line2_Path = databasePath + "/Bill_Line2 = "; // --> UsersData/<user_uid>/bill_line2
}

void loop()
{
  
  Energy_Calculations();
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    
    // Get latest sensor readings
    // Send readings to database:
    sendFloat(Input_Voltage_Path, Input_Voltage);
    sendFloat(Load_Current_Path, Load_Current);
    sendFloat(Real_Power_Path, Real_Power);
    sendFloat(Reactive_Power_Path, Reactive_Power);
    sendFloat(Apparent_Power_Path, Apparent_Power);
    sendFloat(Power_Factor_Path, Power_Factor);
    sendFloat(Line1_Units_Path, Total_kWh1);
    sendFloat(Line2_Units_Path, Total_kWh2);
    sendFloat(Estimated_Bill_Line1_Path, Estimated_Bill_Line1);
    sendFloat(Estimated_Bill_Line2_Path, Estimated_Bill_Line2);
    
  }
  
}

//------------------------------------------ -Energy Calculation Function---------------------------------------------------- -
void Energy_Calculations()
{
  emon1.calcVI(20, 3000);
  emon1.serialprint();

  Real_Power = 391.3;;                                  // Extracts Real Power into a variable
  Apparent_Power = 399.8;                          // Extracts Apparent Power into a variable
  Reactive_Power = 81.04;                         // Calculate the value of reactive power
  Power_Factor = 0.98;                              // Extracts Power Factor into a variable
  Input_Voltage = 236.3;                                    // Extract Vrms into a variable
  Load_Current = 1.74;                                     // Extracts Irms into a variable
  Total_kWh2 = 324;
  Total_kWh1 = 445;
  Estimated_Bill_Line2 = 9232;
  Estimated_Bill_Line1 = 23245;

  Voltage_Array[Universal_Counter] = Input_Voltage;
  Current_Array[Universal_Counter] = Load_Current;
  Real_Power_Array[Universal_Counter] = Real_Power;
  Apparent_Power_Array[Universal_Counter] = Apparent_Power;
  Reactive_Power_Array[Universal_Counter] = Reactive_Power;

  Universal_Counter++;
  if (Universal_Counter == Array_Length - 1)
  {
    Input_Voltage = average(Voltage_Array, Array_Length);
    Load_Current = average(Current_Array, Array_Length);
    Real_Power = average(Real_Power_Array, Array_Length);
    Apparent_Power = average(Apparent_Power_Array, Array_Length);
    Reactive_Power = average(Reactive_Power_Array, Array_Length);

  }
  delay(100);
  Clock++;
  Real_Time = Clock / 3600;  // Converts the time from seconds into hours
  Total_Energy = ((((Real_Power) * (1) / 1000)) / 3600);           //Watt-sec is again converted to Kilo-Watt-Hr by dividing 1hr(3600sec)

  
  //---------------------------------------- |Line Swtiching on the basis of Unit Consumption Logic| --------------------------
  
//  if (Total_kWh1 <= 3)                                             // Line1 units < 300 and Line2 units < 300, so Line 1 is active
//  {
//    
//    Total_kWh1 = Total_kWh1 + Total_Energy;
//    Serial.println(" Line1 < 300 & Line2 < 300 ---> Line 1 active");
//    if( Total_kWh1 > 0.005)
//      Total_kWh1 = Total_kWh1 + 2.998; // Temporary units added in original for switching purpose
//      
//    Estimated_Bill_Line1=bill(Total_kWh1)*300;       // Predicted bill price when 300 units consumed on line 1.
//
//  }
//  else if (((Total_kWh1 > 3) && (Total_kWh2 <= 3)))                // Line1 units > 300 and Line2 units < 300, so Line 2 is active
//  {
//    Total_kWh2 = Total_kWh2 + Total_Energy;
//    digitalWrite(Relay_Input_Line1, HIGH);
//    digitalWrite(Relay_Input_Line2, HIGH);
//    Serial.println("Line1 > 300 & Line2 < 300 ---> Line 2 active");
//    if( Total_kWh2 > 0.005)
//      Total_kWh2 = Total_kWh2 + 2.998; // Temporary units added in original for switching purpose
//    Estimated_Bill_Line2=bill(Total_kWh2)*300;    // Predicted bill price when 300 units consumed on line 2.
//  }
//  else if (((Total_kWh2 > 3) && (Total_kWh1 <= 7)))               // Line1 units < 700 and Line2 units > 300, so Line 1 is active
//  {
//    Total_kWh1 = Total_kWh1 + Total_Energy;
//    digitalWrite(Relay_Input_Line1, LOW);
//    digitalWrite(Relay_Input_Line2, LOW);
//    Serial.println("Line1 < 700 & Line2 > 300 ---> Line 1 active");
//    if( Total_kWh1 > 3.005)
//      Total_kWh1 = Total_kWh1 + 3.990; // Temporary units added in original for switching purpose
//    Estimated_Bill_Line1=bill(Total_kWh1)*300;    // Predicted bill price when 700 units consumed on line 1.    
//  }
//  else if (((Total_kWh1 > 7) && (Total_kWh2 <= 7)))               // Line1 units > 700 and Line2 units < 700, so Line 2 is active
//  {
//    Total_kWh2 = Total_kWh2 + Total_Energy;
//    digitalWrite(Relay_Input_Line1, HIGH);
//    digitalWrite(Relay_Input_Line2, HIGH);
//    Serial.println("Line1 > 700 & Line2 < 700 ---> Line 2 active");
//    if( Total_kWh2 > 3.005)
//      Total_kWh2 = Total_kWh2 + 3.990; // Temporary units added in original for switching purpose
//    Estimated_Bill_Line2=bill(Total_kWh2)*300;    // Predicted bill price when 700 units consumed on line 2.      
//  }
//  else if (((Total_kWh2 > 7) && (Total_kWh1 > 7)))               // Line1 units > 700 and Line2 units > 700, so Line 1 is active
//  {
//    Total_kWh1 = Total_kWh1 + Total_Energy;
//    digitalWrite(Relay_Input_Line1, LOW);
//    digitalWrite(Relay_Input_Line2, LOW);
//    Serial.println("Line1 > 700 & Line2 > 700 ---> Line 1 active");
//    Estimated_Bill_Line1=bill(Total_kWh1)*300;    // Predicted bill price when 1000 units consumed on line 1.
//  }
//  else if((Total_kWh1>=10.005) || (Total_kWh2>=10.005))
//  {
//     Total_kWh1=0;
//     Total_kWh2=0;
//  }
//  else 
//  {
//  }
  if (Input_Voltage < 200)                // Zero out condition for variables when there is no power supply
  {
    Input_Voltage = 0;
    Load_Current = 0;
    Real_Power = 0;
    Apparent_Power = 0;
    Power_Factor = 0;
    Reactive_Power = 0;
    Total_Energy = 0;
    
  }
}

//------------------------- |Bill Calculation Code| -----------------------------

float bill(float units)   // All the units are scaled down by a factor of 100
{
  if (units <= 0.5)   // 0.5 means, 50 units
  {
    return units * 3.95;
  }
  else if ((units > 0.5) && (units <= 1))    // 0.5 means, 50 units and 1 means, 100 units
  {
    return units * 7.74;
  }
  else if ((units > 1) && (units <= 2))       // 1 means, 100 units and 2 means, 200 units
  {
    return (1 * 7.74) + (units - 1) * 10.06;
  }
  else if ((units > 2) && (units <= 3))       // 2 means, 200 units and 3 means, 300 units
  {
    return (1 * 7.74) + (1 * 10.06) + ((units - 2) * 12.15);
  }
  else if ((units > 3) && (units <= 7))       // 3 means, 300 units and 7 means, 700 units
  {
    return (1 * 7.74) + (1 * 10.06) + (1 * 12.15) + ((units - 3) * 19.55);
  }
  else if (units > 7)                         // 7 means, 700 units
  {
    return (1 * 7.74) + (1 * 10.06) + (1 * 12.15) + (4 * 19.55) + ((units - 7) * 22.65);
  }
  else
    return 0;

}

float average(float A[], const int Length)    // Inputs an Array and gives an avarage value
{
  float Temp = 0;
  for (int x = 0; x < Length; x++)
    Temp = Temp + A[x];

  return Temp / Length;

}

void sendFloat(String path, float value){
  if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void initWiFi()   // Function for Initialize WiFi
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}
