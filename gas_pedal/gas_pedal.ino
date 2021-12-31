#define pinIn 15
#define pinInBlack 16
#define pinOut 3

//#define manual
#define pedal

#define fullThrottle 330
#define noThrottle 470

#define noThrottleBlack 335
#define fullThrottleBlack 404


int inputBlack = 0;
int val = 0;
int valBlack = 0;

void setup() {
  pinMode(pinIn, INPUT);
  pinMode(pinOut, OUTPUT);
  Serial.begin(115200);
  Serial.println("Setup succesful!");
  Serial.println(" ");
}

void loop() {
  String inputStr = "";
  int inputInt = 0;
  int oldInt = 0;
  #ifdef manual
    if(Serial.available() > 0){
      inputStr = Serial.readStringUntil("\n");
      inputInt = inputStr.toInt();
      if(inputInt < 0){
        inputInt = 0;
      }
      else if(inputInt  > 100){
        inputInt = 100;
      }
        Serial.print("Gas auf ");
    Serial.print(inputInt);
    Serial.println(" Prozent gesetzt");
    val = map(inputInt, 0, 100, 0,255);
    Serial.println(val);
    }
  
    
  #endif
  #ifdef pedal
    inputInt = analogRead(pinIn);
    inputBlack = analogRead(pinInBlack);
    val = map(inputInt, fullThrottle, noThrottle, 255, 0);    //Geschw Begrenzung (60 war ganz okay)
   // val = map(inputInt, fullThrottle, noThrottle, 100, 0);    //Geschw Begrenzung (60 war ganz okay)
    valBlack = map(inputBlack, fullThrottleBlack, noThrottleBlack, 100, 0);
    // Serial.println(val);
    // Serial.println(valBlack);
    // Serial.println("-------------------------------------");
    if(val > 255){
      val = 255;
    }
    if(val < 0){
      val = 0;
    }
    
  #endif
  
  //Serial.println(analogRead(pinInBlack));
  analogWrite(pinOut, val);
  delay(100);
}
