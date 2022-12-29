
#include <SoftwareSerial.h>	    //C++ library for arduino code

int const LDR = A5;                 //LDR input port, analong input A0
int Resistance = 0;                 //LDR analog value input
int const ThresholdResistance = 800;//Darkness level resistance limit
int const StreetLights = 6;         //Output port for street lights
int const LineTracker = 7;          //Input port for IR line tracker
int const YledT = 3;                //Output port for Yellow traffic light
int const RledT = 8;                //Output port for Red traffic light
int const GledT = 5;                //Output port for Green traffic light
int const PedLight = 2;             //Output port for pedestrian crossing light
int LineTrackerState = 0;           //IR line tracker digital input value 
int SignalFlag = 0;                 //Flag to indicate signal close on traffic absence
int SignalsCounter = 0;             //Counter for number of signals
int MissingCarTimer = 0;            //5 second threshold timer for empty road before signal switches
int seconds = 0;                    //Seconds counter
int MaxSeconds = 15;                //Default number of seconds till the signal remains open
int Rseconds = 0;                   //Remaining seconds from the last signal
int const Button = 13;              //Button input port, triggering the system
int ButtonValue = 1;                //Button digital input


//setup() function, runs once every time arduino is provided power to
//setup() and loop() functions are necessary (like main()) for arduino code
void setup() 
{
  pinMode(StreetLights,OUTPUT);	    //Configure streetlight pin for output
  pinMode(LineTracker,INPUT);       //Configure Linetracker pin for input
  pinMode(YledT,OUTPUT);            //Configure LED pins for output
  pinMode(RledT,OUTPUT);
  pinMode(GledT,OUTPUT);
  pinMode(PedLight,OUTPUT);
  pinMode(Button,INPUT);	    //Configure button pin for input
  Serial.begin(9600);		    //Start serial arduino connection with 9600 baud (9600 bits per second serial connection)
}


//Delay function, adds delay for n milliseconds
//And checks street light status for every second of delay specified
void Delay(int n){
  for (int k = 0; k < (n / 1000); k++){  //k is every second (n milliseconds / 1000)
    check_street_lights();
    delay(1000);
  }
}


//Check street light status function
//Based on readings of photoresistance received from LDR
//All analog readings (including LDR's) are in range 0-1024
//LDR provides 1024 when completely dark, 800 is suitable value for turning on street lights

void check_street_lights()
{
  Resistance = analogRead(LDR);
  if (Resistance > ThresholdResistance) //Resistance = current reading, ThresholdResistance = Darkness level limit
    digitalWrite(StreetLights, HIGH);   //Turn lights on
  else
    digitalWrite(StreetLights, LOW);    //Turn lights off
}


//Turn traffic signal red procedure
void switch_off_signal()
{
  digitalWrite(GledT, LOW);	//Turn green light off
  digitalWrite(RledT,HIGH);	//Turn red light on
  Delay(500);			
  digitalWrite(PedLight,HIGH);	//Turn pedestrian light on after half a second
  Delay(5000);			//Remain in this state for 5 seconds
}


//Turn traffic signal green procedure
void switch_signal_on()
{
  digitalWrite(PedLight, LOW);	//Turn pedestrian light off half a second
  Delay(1000);
  digitalWrite(RledT,LOW);	//Turn red light off after a second
  digitalWrite(YledT,HIGH);	//Simultenously, turn yellow light on
  Delay(3000);
  digitalWrite(YledT,LOW);	//Keep wait light (yellow light on for 3 seconds), then turn it off
  digitalWrite(GledT,HIGH);	//Turn green light
}


//loop() function, this will be repeated till arduino is powered off
void loop()
{
  Serial.println("Rseconds: ");
  Serial.println(Rseconds);	      //Print remaining seconds on console

  //start program at button push
  while (ButtonValue != LOW)	      //ButtonValue is the current value of button input
    ButtonValue = digitalRead(Button);//ButtonValue is LOW when button pressed

  Serial.println("continue");	      //Print continue to console when program is started

  SignalsCounter = SignalsCounter + 1;//The signal number being currently toggled
  if (SignalsCounter == 4){	      //If all signals cycled
    SignalsCounter = 0;		      //Reset signal counter and remaining seconds
    Rseconds = 0;
  }
  
  MissingCarTimer = 0;		      //Timer used when signal green and traffic absent, 5 seconds absent and signal toggles
  seconds = MaxSeconds + Rseconds;    //Total seconds for green signal = remaining seconds from previous signal + maximum seconds for each signal
  SignalFlag = 0;		      //Toggle signal flag when traffic absent for 5 seconds
  
  switch_signal_on();		      //Start by toggling signal green
  while (seconds > 1)		      //While there still is time for green signal
  {
    check_street_lights();
    Delay(1000);
    seconds = seconds - 1;	      //Signal time count
    LineTrackerState = digitalRead(LineTracker);
    Serial.println(LineTrackerState);             //LineTrackerState = 1 if no traffic, = 0 if passing traffic
    
    while (LineTrackerState && MissingCarTimer < 5) //While traffic absent for less than 5 seconds
    {
      check_street_lights();
      MissingCarTimer = MissingCarTimer + 1; //Absent traffic time count 
      Delay(1000);
      seconds = seconds - 1;		     //Signal time count

      if (MissingCarTimer == 5){	     //If 5 seconds passed with absent traffic
        SignalFlag = 1;			     //Toggle signal flag
        break;				     //break loop
      }

      LineTrackerState = digitalRead(LineTracker); //Check if traffic still absent

      check_street_lights();
    }
    
    //Loop exists when traffic present
    MissingCarTimer = 0;

    if (SignalFlag == 1){ 		//if signal toggle flag(traffic absent for 5 seconds)
      Rseconds = seconds;		//Add to remaining seconds from toggled signal
      break;
    }
    else{				//Else, maximum time for green signal used by present traffic
      Rseconds = 0;			//Set remaining seconds to zero
    }
  }

  switch_off_signal();			//Turn current signal red and move to next signal
}


//Additional file handling part, unused
//Not included in code

/* ////
void loop(){
  Serial.begin(9600);
  Serial mySerial;
  PrintWriter output;
  void setup() {
    mySerial = new Serial( this, Serial.list()[0], 9600 );
    output = createWriter( "data.txt" );
  }
  void draw() {
      if (mySerial.available() > 0 ) {
           String value = mySerial.readString();
           if ( value != null ) {
                output.println( value );
          }
      }
  }

  void keyPressed() {
      output.flush();  // Writes the remaining data to the file
      output.close();  // Finishes the file
      exit();  // Stops the program
  }
}
*/ ////