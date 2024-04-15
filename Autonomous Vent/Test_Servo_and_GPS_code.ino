#include<Adafruit_GPS.h>//Install the adafruit GPS library
#include<SoftwareSerial.h>//Load the software serial library
#include "Servo.h"// Include the servo library:
SoftwareSerial mySerial(3,2);//Initialize the software serial port (We are creating our own serial minitor to work with the GPS)
Adafruit_GPS GPS(&mySerial);//Creates the GPS object

// Define the servo pin:
#define servoPin 9

// Create a new servo object:
Servo myservo;

String NMEA1;//Variable for firts NMEA sentance (We only need to use the  GPGGA  & GPRMC NMEA lines)
String NMEA2;  //We will use this variable to hold our second NMEA sentence- we are using 2 NMEA lines so we can get both GPGGA and GPRMC not only the GPRMC

float altitudeGPS;
float latitudeGPS, longitudeGPS;  // EDIT
const float targetHight=651.0; ///blacksburg altitude is 618Meters above sea level- target altitude need s to be added to that // GPS works on meters
const float top=0.0, bottom=0.0, left=0.0, right=0.0; // EDIT

char c; //This character only purpose is to we can raed the characters from the string NMEA lines

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);//Turns on normal serial monitor 
  GPS.begin(9600);// Turn on GPS at 9600 baud
  GPS.sendCommand("$PGCMD,33,0*6D");//Turns off antenna update data that tends to interfere with the GPGGA & GPRMC lines of data we need
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ); //Set update rate to 10HZ
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);//Request that the GPS only send the GPRMC & GPGGA NMEA data lines
  delay(250); 
   // Attach the Servo variable to a pin:
  myservo.attach(servoPin);
   pinMode(13, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  //  digitalWrite(ENABLE_PIN, HIGH);
 readGPS();
 Serial.println(GPS.altitude);

 Serial.println(GPS.latitude);  // EDIT
 Serial.println(GPS.longitude); // EDIT
 
 altitudeGPS=GPS.altitude; // Traget altitude has to adjusted as VT has an altitude of 604 meters above sea level.
 
 latitudeGPS=GPS.latitude;    // EDIT
 longitudeGPS=GPS.longitude;  // EDIT

convertCoordinate(&latitudeGPS);  //EDIT  - Convert DDMM.MMMM to Decimal Degrees
 convertCoordinate(&longitudeGPS); // EDIT
  
 Serial.println(altitudeGPS);
 
 Serial.println(latitudeGPS);   // EDIT
 Serial.println(longitudeGPS);  // EDIT

  if(altitudeGPS>targetHight){
  myservo.write(90);
  delay(1000);
  myservo.write(180);
  delay(1000);
  myservo.write(0);
  delay(1000);
 }

  // EDIT
 if     (latitudeGPS>top   || latitudeGPS<bottom) {}
 else if(longitudeGPS<left || longitudeGPS>right) {}
 else {
  myservo.write(90);
 }

}

void readGPS(){
  clearGPS();
  while(!GPS.newNMEAreceived()){//loop until we get a proper NMEA sentance 
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());//Parse the last good NMEA sentace
  NMEA1=GPS.lastNMEA(); //saved the 1 good sentace
  while(!GPS.newNMEAreceived()) {  //Go out and get the second NMEA sentence, should be different type than the first one read above.
  c=GPS.read();
  }
GPS.parse(GPS.lastNMEA());
NMEA2=GPS.lastNMEA();
  Serial.println(NMEA1);
  Serial.println(NMEA1);
  Serial.println("");
}
void clearGPS(){ // clear old and corrupt data from serial port
  while(!GPS.newNMEAreceived()){//loop until we get a proper NMEA sentance 
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());//Parse the last good NMEA sentace
  while(!GPS.newNMEAreceived()){//loop until we get a proper NMEA sentance 
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());//Parse the last good NMEA sentace
  while(!GPS.newNMEAreceived()){//loop until we get a proper NMEA sentance 
    c=GPS.read();
  }
  GPS.parse(GPS.lastNMEA());//Parse the last good NMEA sentace
}

// Geofence
void convertCoordinate(float *coordinate) {
  int degrees = *coordinate / 100;              //degrees
  float minutes = *coordinate - 100 * degrees;  // minutes
  float minutes2 = minutes - (int)minutes;
  minutes /= 60;                                // minutes to degrees
  minutes2 /= 3600;                              // minutes to degrees
  *coordinate = degrees + minutes + minutes2;
}
