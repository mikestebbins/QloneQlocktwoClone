 byte data[2];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  getdat(&data[0]);

  for (int i = 0; i<2; i++)  {
    Serial.println(data[i]);
  }

  data[0] = 0x01;
  data[1] = 0x02;

  for (int i = 0; i<2; i++)  {
    Serial.println(data[i]);
  }

  Serial.println("-----");
  
  delay(500);
}

void getdat(byte *pdata)
{
 pdata[0] = 0x00;
 pdata[1] = 0xFF;
}
