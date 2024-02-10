long long int avg = 0;
long long int count = 0;
String inString = "";
void setup() {
  Serial.begin(9600);
  pinMode(A0,INPUT);
  pinMode(D1,OUTPUT);
  inString.reserve(200);
}

void loop() {
  while (Serial.available() > 0) {
    int inChar = Serial.read();
    if (isDigit(inChar)) {
      inString += (char)inChar;
    }
    if (inChar == '\n') {
      analogWriteFreq(inString.toInt(););
      Serial.print("Frequency : ");
      Serial.println(inString.toInt());
      inString = "";
    }
  }
  int val = 0;
  val = analogRead(A0);
  count++;
  avg += val;
  if(val >= 18*3 - 2)
    analogWrite(D1,0);
  else analogWrite(D1,50*(!(freq == 0)));
  if(count >= 1000)
  {
    Serial.println(int(avg/count));
    count = 0;
    avg = 0;
  }
}
