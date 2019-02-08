void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("hack for spotlight LED");

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

}

int id = -1;
char a = 'n';
char b = 'n';

void loop() {
    if (Serial.available() >= 2){
    // LED_id
    a = Serial.read(); // 1 or 0 on/off
    b = Serial.read(); // colon separator
    id = Serial.parseInt();
    //Serial.read(); //flush newline char

    Serial.println(a);
    Serial.println(b);
    Serial.println(id);

    if(id >= 0 && id <= 24)
      led_display(id, a == '1');

    while(Serial.available())
      Serial.read(); //disgusting blocking code to deal with flushing serial line

  }

}

void led_display(int i, bool on)
{
  if (on)
    digitalWrite(13, LOW);
  else
    digitalWrite(13, HIGH);

  if(i == 25) //all off condition
    digitalWrite(13, HIGH);
}
