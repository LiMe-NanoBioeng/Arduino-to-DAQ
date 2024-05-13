// Arduino 基本動作Firmware
// 20190219 Release1
// by Hiroyuki OGAWA

// 20190227 Release1.1 AnalogInputで1チャンネルのみ指定ができない問題を修正
//by Junichi Murai　20240430

//difine ten valves 
const int Valvepins[]={3,7,34,1,33,25,24,23,22,21};
const int numValves = sizeof(Valvepins);
int val = 0;

//　setup for ten valves
void setup() {
  for(int i =0; i<numValves;i++){
   pinMode(Valvepins[i],OUTPUT);
  

  Serial.begin(9600); //open serialport by 9600bps

   digitalWrite(Valvepins[i],HIGH);
   }

}

void loop() {

checkUserInteraction();

}

void checkUserInteraction(){

  while (Serial.available() > 0){ 
    // 受信したデータが存在する
    char temp = Serial.read();

    // 1文字目を読み込む
    char tempIO = Serial.read();

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
          AnalogIN();
          break;
        case 'O':
        // 読み込みデータが　O の場合
          AnalogOUT();
          break;
        }
    }
    //
    else {
      Serial.flush();
      break;
    }
  }
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
      digitalWrite(Valvepins[vNumD-1], HIGH);
      break;
    case 'L':
      // 読み込みデータが　L の場合
      digitalWrite(Valvepins[vNumD-1], LOW);
      break;
  }
}


// "AI1:3"というフォーマットの文字列をベースとして設定する
//AnalogINの処理
void AnalogIN(){

  // 3文字目は開始バルブ番号
  int vNumAS = Serial.parseInt();
  
  // 4文字目はコロンの判定
  // str from_to = Serial.read();
    
  if ( Serial.read() == ':'){
   
   // 5文字目は終了バルブ番号
      int vNumAE = Serial.parseInt();
      for (int i = vNumAS; i<= vNumAE-1; i++){
        val = analogRead(i-1);
        Serial.print(val);
        Serial.print(", ");         
        }
        val = analogRead(vNumAE-1);
        Serial.println(val);
   }
   else {
        val = analogRead(vNumAS-1);    
        Serial.println(val);
   }
      
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
      aoDuty = aoDuty / 5000 * 255;
      analogWrite(vNumA, aoDuty);
      break;

    case 'q':
      analogWrite(vNumA,0); //出力OFF
    break;
  
  }
}
