// Arduino 基本動作Firmware
// 20190219 Release1
// by Hiroyuki OGAWA

// 20190227 Release1.1 AnalogInputで1チャンネルのみ指定ができない問題を修正
// Hiro 202404 made PID, I2C, Pulse functions
//by Junichi Murai　20240513  modify for ten valves
//by :K2(EEprotocol) 20240530 modify for pressure regulator pin 

// feadback control
bool fdmode = false;
bool fmeter = true;
float pre_measurement=0.0;
float setpoint = 0.0;
float preT=0.0;
float integralv=0.0;
float Kp=0.1;
float Ki=0.001;
float Kd=0.0;

int aoDuty=0.0;
//int vNumA =11;
int vNumA =9;//9/9/2024 K2
//difine ten valves 
//const int Valvepins[]={1,22,21,15,20,16,19,14,18};
const int Valvepins[]={1,7,15,16,14,22,21,20,19,18,23};
const int numValves = 11;
int val = 0;
//const int pressReg=4;

//I2C 
#include <Wire.h> // Arduino library for I2C
const int ADDRESS = 0x40; // Standard address for Liquid Flow Sensors

// EEPROM Addresses for factor and unit of calibration fields 0,1,2,3,4.
const uint16_t SCALE_FACTOR_ADDRESSES[] = {0x2B6, 0x5B6, 0x8B6, 0xBB6, 0xEB6};
const uint16_t UNIT_ADDRESSES[] =         {0x2B7, 0x5B7, 0x8B7, 0xBB7, 0xEB6};

// Flow Units and their respective codes.
const char    *FLOW_UNIT[] = {"nl/min", "ul/min", "ml/min", "ul/sec", "ml/h"};
const uint16_t FLOW_UNIT_CODES[] = {2115, 2116, 2117, 2100, 2133};

uint16_t scale_factor;
const char *unit;
int starttime;
//EOF I2C
//　setup for ten valves
void setup() {
  int ret;
  uint16_t user_reg;
  uint16_t scale_factor_address;
  
  uint16_t unit_code;

  byte crc1;
  byte crc2;
  
  for(int i =0; i<numValves;i++){
   pinMode(Valvepins[i],OUTPUT);
   digitalWrite(Valvepins[i],HIGH);
   }
 Serial.begin(9600); //open serialport by 9600bps#NEVER CHANGE FROM 9600. Be patient however slow it is... 
 Wire.begin();       // join i2c bus (address optional for master)
 starttime=millis();
  do {
    delay(1000); // Error handling for example: wait a second, then try again
    if (millis()-starttime > 10000){
      fmeter=false;
      break;
      }
    //Serial.println(millis()-starttime);
    // Soft reset the sensor
    Wire.beginTransmission(ADDRESS);
    Wire.write(0xFE);
    ret = Wire.endTransmission();
    if (ret != 0) {
      //Serial.println("Error while sending soft reset command, retrying...");
      continue;
    }
    delay(50); // wait long enough for reset

    // Read the user register to get the active configuration field
    Wire.beginTransmission(ADDRESS);
    Wire.write(0xE3);
    ret = Wire.endTransmission();
    if (ret != 0) {
      //Serial.println("Error while setting register read mode");
      continue;
    }

    Wire.requestFrom(ADDRESS, 2);
    if (Wire.available() < 2) {
      //Serial.println("Error while reading register settings");
      continue;
    }
    user_reg  = Wire.read() << 8;
    user_reg |= Wire.read();

    // The active configuration field is determined by bit <6:4>
    // of the User Register
    scale_factor_address = SCALE_FACTOR_ADDRESSES[(user_reg & 0x0070) >> 4];

    // Read scale factor and measurement unit
    Wire.beginTransmission(ADDRESS);
    Wire.write(0xFA); // Set EEPROM read mode
    // Write left aligned 12 bit EEPROM address
    Wire.write(scale_factor_address >> 4);
    Wire.write((scale_factor_address << 12) >> 8);
    ret = Wire.endTransmission();
    if (ret != 0) {
      //Serial.println("Error during write EEPROM address");
      continue;
    }

    // Read the scale factor and the adjacent unit
    Wire.requestFrom(ADDRESS, 6);
    if (Wire.available() < 6) {
      //Serial.println("Error while reading EEPROM");
      continue;
    }
    scale_factor = Wire.read() << 8;
    scale_factor |= Wire.read();
    crc1         = Wire.read();
    unit_code    = Wire.read() << 8;
    unit_code   |= Wire.read();
    crc2         = Wire.read();

    switch (unit_code) {
      case 2115:
        {
          unit = FLOW_UNIT[0];
        }
        break;
      case 2116:
        {
          unit = FLOW_UNIT[1];
        }
        break;
      case 2117:
        {
          unit = FLOW_UNIT[2];
        }
        break;
      case 2100:
        {
          unit = FLOW_UNIT[3];
        }
        break;
      case 2133:
        {
          unit = FLOW_UNIT[4];
        }
        break;
      default:
        Serial.println("Error: No matching unit code");
        break;
    }


  //I2C
  //Wire.begin();
  //do {
    // Soft reset the sensor
    Wire.beginTransmission(ADDRESS);
    Wire.write(0xFE);
    ret = Wire.endTransmission();
    if (ret != 0) {
      Serial.println("Error while sending soft reset command, retrying...");
      //fmeter=false;
    }
     delay(50); // wait long enough for chip reset to complete
   // To perform a measurement, first send 0xF1 to switch to measurement mode,
  // then read 2 bytes + 1 CRC byte from the sensor.
  Wire.beginTransmission(ADDRESS);
  Wire.write(0xF1);
  ret = Wire.endTransmission();
  if (ret != 0) {
      Serial.println("Error while changing to measurement mode, retrying...");
      //fmeter=false;
    }
  } while (ret != 0);
  
 
  // EOF I2C
}
bool is_first_cycle = true; //add JM
void loop() {
 checkUserInteraction(aoDuty); 
 if (fdmode==true){
  aoDuty=PID();
 }
}
//bool is_first_cycle = true; //add JM

int PID(){
  float measurement;
  float error, e_prev, P, D, max_val,dt;
  float MV,curT;
  //Serial.println(integralv);
  max_val=10;
  curT=millis();
  dt = (curT-preT)/1000; 
  
  //add JM
  if (is_first_cycle){
    measurement=0.0;
    is_first_cycle=false;
  }else{
    measurement=I2CIN();
  }
  
  //measurement=I2CIN();
  error=setpoint - measurement;
  e_prev=setpoint -pre_measurement;
  P=Kp*error;
  integralv = integralv+Ki*error*dt;
  D =Kd*(error-e_prev)/dt;
  MV=P+integralv+D;
  //aoDuty = max(min(MV / 1000 * 255.0,254),0);
  aoDuty = max(min(MV / 1000 * 255.0,254),1);
  analogWrite(vNumA, aoDuty);
  integralv = max(min(MV,1000),0);
  preT=curT;
  pre_measurement = measurement;
  return(aoDuty);
  }
void checkUserInteraction(int aoDuty){
  //int ret;
  float converted_flowrate;
  String out;
  while (Serial.available() > 0){ // if data is available 
    //String line=Serial.readStringUntil('\n');
    //Serial.println(line);
    char temp = Serial.read(); // read the first char
    char tempIO = Serial.read(); // read the second char
    if ( temp == 'D'){
      switch(tempIO){
        case 'I':
        // 読み込みデータが　I の場合
          DigitalIN();
          break;
        case 'O':
        // 読み込みデータが　O の場合
          DigitalOUT();
          break;
        }
     }

    else if ( temp == 'A'){
      switch(tempIO){
        case 'I':
        // 読み込みデータが　I の場合
          out = AnalogIN();
          Serial.println(out);

          break;
        case 'O':
        // 読み込みデータが　O の場合
          AnalogOUT();
          break;
        }
    }

    else if (temp == 'I'){
      switch(tempIO){
        case 'I':
          if (fmeter ==true){
            converted_flowrate=I2CIN();
          }else{
            converted_flowrate=0.0;
          }
          Serial.println(converted_flowrate);
          break;
        case 'O':
          break;
      }
    }
    else if (temp== 'F'){
      fdmode=true;
      vNumA=Serial.parseInt();
      setpoint=Serial.parseFloat();
      Kp=Serial.parseFloat();
      Ki=Serial.parseFloat();
      Kd=Serial.parseFloat();
      preT=millis();
      pre_measurement=0.0;
      integralv=0.0;
      is_first_cycle = true; //add JM
      break;
    }
    else if (temp=='B'){ // 
      fdmode=false;
      pre_measurement=0.0;
      integralv=0.0;
      is_first_cycle = true; //add JM
      //aoDuty=1;//8/9/2024 :K2add
      //digitalWrite(Valvepins[8],HIGH);
      //delay(10);
      //digitalWrite(Valvepins[8],LOW);
      break;
    }
    else if (temp=='P'){

      DigitalPulse();
//      vNumA=Serial.parseInt();
//      setpoint=Serial.parseFloat();
//      Kp=Serial.parseFloat();
//      Ki=Serial.parseFloat();
//      Kd=Serial.parseFloat();
      break;
    }
    else if (temp=='R'){ // read duty
      Serial.println(aoDuty);
    }
    else if (temp=='S'){  // return ready 
      Serial.print('R');
    }
    //
    else {
      //Serial.flush();
      break;
    }
  }
}
  float I2CIN(){

    int ret;
    uint16_t raw_sensor_value;
    int16_t signed_sensor_value;
    float converted_flowrate;

  // To perform a measurement, first send 0xF1 to switch to measurement mode,
  // then read 2 bytes + 1 CRC byte from the sensor.
  //Wire.beginTransmission(ADDRESS);
  //Wire.write(0xF1);
  //ret = Wire.endTransmission();
  //if (ret != 0) {
    //Serial.println("Error during write measurement mode command");

  //} else {
    Wire.requestFrom(ADDRESS, 2);       // reading 2 bytes ignores the CRC byte
    if (Wire.available() < 2) {
      //Serial.println("Error while reading flow measurement");

    } else {
      raw_sensor_value  = Wire.read() << 8; // read the MSB from the sensor
      raw_sensor_value |= Wire.read();      // read the LSB from the sensor

      //Serial.print("raw value from sensor: ");
      //Serial.print(raw_sensor_value);

      signed_sensor_value = (int16_t) raw_sensor_value;
      converted_flowrate = (float) signed_sensor_value / (float) scale_factor;
      //Serial.print(", signed value: ");
      
      return converted_flowrate;
    //}
  }

  //delay(1000); // milliseconds delay between reads (for demo purposes)
}



void DigitalIN(){
  //Digitalinの処理
  //Serial.write('N');
  //Serial.write('\n');
}



// "DO1H"というフォーマットの文字列をベースとして設定する
void DigitalOUT(){

  // 3文字目はバルブ番号
  int vNumD = Serial.parseInt();
  
  // 4文字目はHかL
  char onOFF = Serial.read();

  switch(onOFF){
    case 'H':
      // 読み込みデータが　H の場合
      digitalWrite(Valvepins[vNumD], HIGH);
      break;
    case 'L':
      // 読み込みデータが　L の場合
      digitalWrite(Valvepins[vNumD], LOW);
      break;
  }
  Serial.println('R');
}

void DigitalPulse(){
  float curT;
  // 3文字目は開始バルブ番号
  int vNumD = Serial.parseInt();
  
  // 4文字目はコロンの判定
  char from_to = Serial.read();
  int vNumAE = Serial.parseInt();
  float delayTime = Serial.parseFloat();
  float duration = Serial.parseFloat();
  int vNumRead =Serial.parseInt(); // channel number of the photosensor
  int threshold =Serial.parseInt(); // threshold to stop
  curT=(float) millis()/1000;
  Serial.println('B');
//     Serial.print(delayTime+duration);
//     Serial.print( ',');
//     Serial.println((float) millis());
  digitalWrite(Valvepins[10], HIGH);
  while (delayTime > (float) millis()/1000-curT){
    // just a timer
    //Serial.println(val);
  }
  val = analogRead(vNumRead);// stop threshold
  
  // set the pins high
  //digitalWrite(Valvepins[10], HIGH);
  if ( from_to == ':'){
      for (int i = vNumD; i<= vNumAE; i++){
        digitalWrite(Valvepins[i], HIGH);      
        }
    }
    else if (from_to == ','){
        digitalWrite(Valvepins[vNumD], HIGH);
        digitalWrite(Valvepins[vNumAE],HIGH);
    }
    else {
        digitalWrite(Valvepins[vNumD], HIGH);
    }  
  while ((delayTime+duration > (float) millis()/1000-curT) && (val < threshold)){
    val = analogRead(vNumRead);// stop threshold
    //Serial.println(val);
  }
  if ( from_to == ':'){
      for (int i = vNumD; i<= vNumAE; i++){
        digitalWrite(Valvepins[i], LOW);      
        }
    }
    else if (from_to == ','){
        digitalWrite(Valvepins[vNumD], LOW);
        digitalWrite(Valvepins[vNumAE],LOW);
    }
    else {
        digitalWrite(Valvepins[vNumD], LOW);
    }
  digitalWrite(Valvepins[10], LOW);
  Serial.println('R');
    
}
// "AI1:3" or "AI1,3"というフォーマットの文字列をベースとして設定する
//AnalogINの処理
String AnalogIN(){
  String out;
  int val;
  // 3文字目は開始バルブ番号
  int vNumAS = Serial.parseInt();
  
  // 4文字目はコロンの判定
  char from_to = Serial.read();   
  if ( from_to == ':'){
    int vNumAE = Serial.parseInt();
   
   // 5文字目は終了バルブ番号
      for (int i = vNumAS; i<= vNumAE; i++){
        //Serial.print(val);
        //Serial.print(", ");
        if (i>vNumAS){
          val = analogRead(i);
          out= out + "," + val;
        } 
        else{
          val = analogRead(i);
          out = val;
          //out=val;
        }
        }
        //val = analogRead(vNumAE);
   }
   else if (from_to == ','){
        int vNumAE = Serial.parseInt();
        val = analogRead(vNumAS);
        out = val;    
        //Serial.print(val);
        //Serial.print(", ");
        //out = val;
        val=analogRead(vNumAE);
        out = out + "," + val ;
    }
   else {
        val =analogRead(vNumAS);
        out = val;    
   }
 //Serial.println(out);
   return out;   
}


//AnalogOUTの処理
void AnalogOUT(){
  
  // 3文字目は開始バルブ番号
  int vNumA = Serial.parseInt();
  //Serial.write(vNumA);
  //Serial.write('\n');

  // 4文字目はbかv
  char cselect = Serial.read();
  //Serial.write(cselect);
  //Serial.write('\n');

  // 5文字目以降はデューティ比
  float aoDuty = Serial.parseInt();
    // 小数を扱いたいのでfloat関数を使用

  //行末はNewlin(LF=ラインフィード)か？
  if (Serial.read() != '\n'){
    Serial.flush();
    return;
  }
    
  switch(cselect){
    case 'b':
      // 読み込みデータが　b の場合
      analogWrite(vNumA,aoDuty);
      break;
      
    case 'v':
      // 読み込みデータが　v(mV) の場合
      //aoDuty = aoDuty;
      analogWrite(vNumA, aoDuty);
      break;

    case 'q':
      analogWrite(vNumA,0); //出力OFF
    break;
  
  }
}
