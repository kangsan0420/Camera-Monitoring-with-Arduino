//========= global Variables ==========
byte Pin[] = {22, 23, 24, 25, 26, 27};
short pinNum = 6;

byte MaxPin = 28;
byte MinPin = 29;
short VoltMin = 100;
short VoltMax = 300;
short Voltage = 0;
short Current = 0;

short cStatus = 0x00;
short cConfig = 0x00;
short reserved = 0xFF;
bool on = LOW;
byte dt = 10;

byte thres = 2;
#define lst_size 20
short V[lst_size] = {};
short C[lst_size] = {};
short Vt, Ct;
byte i = 0;
//=====================================

void setup() {
  initGpio();

  int BAUDRATE = 9600;
  digitalWrite(13, HIGH);
  Serial.begin(BAUDRATE);
  while (!(Serial));
  delay(500);
  digitalWrite(13, LOW);
}

void loop() {
  cStatus = 0;
  V[i] = (analogRead(A4)-400)*10;
  C[i] = analogRead(A0);
  i = (i+1)%10;
  Vt = average(V);
  Ct = average(C);

  if(abs(Voltage - Vt) > thres)
    Voltage = Vt;
  if(abs(Current - Ct) > thres)
    Current = Ct;
  overVoltDetect();

  char data[50];
  if (Serial.available() > 0) {
    short sig = read16();
    if ((sig >> 8) == 0x01) // initialize mode
    {
      send16(Voltage);
      send16(VoltMax);
      send16(VoltMin);
    }
    else if ((sig >> 8) == 0x02) // configurate mode
    {
      VoltMin = read16();
      VoltMax = read16();
      send16(sig);
    }
    else if ((sig >> 8) == 0x03) // communicate mode
    {
      send16(cStatus);
      send16(Voltage);
      send16(Current);
      lightLED(sig);
    }
    else // flag error
    {
      send16(reserved);
      send16(sig);
      send16(reserved);
    }
  }
}

//======================================================

void send16(short bits) {
  Serial.write(byte(bits >> 8));
  Serial.write(byte(bits));
}

short read16() {
  byte byte_ms = Serial.read();
  Serial.write(byte_ms);
  delay(5);
  byte byte_ls = Serial.read();
  Serial.write(byte_ls);
  short int16 = byte_ms * 256 + byte_ls;
  delay(5);
  return int16;
}

void lightLED(byte bits) {
  for (int i = 0; i < pinNum; i++)
  {
    if (((bits >> i) & 0x01) == 1)
      on = HIGH;
    else
      on = LOW;
    digitalWrite(Pin[i], on);
  }
}

void overVoltDetect() {
  if (Voltage > VoltMax) {
    digitalWrite(MaxPin, HIGH);
    cStatus += 2;
  } else
    digitalWrite(MaxPin, LOW);
  if (Voltage < VoltMin) {
    digitalWrite(MinPin, HIGH);
    cStatus += 1;
  } else
    digitalWrite(MinPin, LOW);
}

void initGpio() {
  for (int i = 0 ; i < pinNum ; i++) {
    pinMode(Pin[i], OUTPUT);
    digitalWrite(Pin[i], LOW);
  }
  pinMode(MaxPin, OUTPUT);
  pinMode(MinPin, OUTPUT);
}

float average(short* lst){
  float tmp = 0;
  float len = lst_size;
  for(int i=0;i<lst_size;i++){
    tmp += lst[i]*(lst_size-i)/len;
  }
  return tmp/len;
}
