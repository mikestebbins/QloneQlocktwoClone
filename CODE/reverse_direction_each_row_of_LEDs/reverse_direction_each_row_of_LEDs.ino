void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
}

void loop() {

  bool reversed = false;

  int j = 0;
  for (int i = 0; i < 114; i++)  {
    if (reversed == false)  {
      Serial.print(i);
      Serial.print("\t");
    }
    else {
      int temp = i + 10 - (2*j);
      Serial.print(temp);
      Serial.print("\t");
      j++;
    }
    
    if (i%11 == 10)  {
      reversed = !reversed;
      j = 0;
      Serial.println("");
    }    
  }
  
  Serial.println("");
  Serial.println("----------------");
  delay(1000);
}
