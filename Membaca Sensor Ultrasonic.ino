#include <LiquidCrystal_I2C.h>

int trigPin = 7;
int echoPin = 6;
int ledMerah = 4;
int ledKuning = 3;
int ledHijau = 2;
int piezoPin = 10;

LiquidCrystal_I2C lcd(0x27, 16, 2);

float *dataBuffer;
int kapasitas = 30;
int jumlahData = 0;

long durasi;
float jarak;

extern int __heap_start;
extern int *__brkval;
int freeMemory() {
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledMerah, OUTPUT);
  pinMode(ledKuning, OUTPUT);
  pinMode(ledHijau, OUTPUT);
  pinMode(piezoPin, OUTPUT);

  lcd.init();
  lcd.backlight();

  dataBuffer = (float*) malloc(kapasitas * sizeof(float));

  Serial.println("=== SISTEM MONITOR JARAK ===");
  Serial.print("Free RAM awal: ");
  Serial.println(freeMemory());
  Serial.print("Berhasil alokasi ");
  Serial.print(kapasitas * sizeof(float));
  Serial.println(" bytes");
  Serial.print("Free RAM setelah alokasi: ");
  Serial.println(freeMemory());
}

float bacaJarak() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2); 
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  durasi = pulseIn(echoPin, HIGH);
  jarak = durasi * 0.034 / 2;
  return jarak;
}

float hitungRata() {
  float total = 0;
  for (int i = 0; i < jumlahData; i++) {
    total = total + dataBuffer[i];
  }
  if (jumlahData == 0) return 0;
  return total / jumlahData;
}

void updateIndikator(float d) {
  digitalWrite(ledMerah, LOW);
  digitalWrite(ledKuning, LOW);
  digitalWrite(ledHijau, LOW);
  noTone(piezoPin);

  if (d < 10) {
    digitalWrite(ledMerah, HIGH);
    tone(piezoPin, 10);
    delay(100);
    noTone(piezoPin);
    delay(100);
  } else if (d <= 30) {
    digitalWrite(ledKuning, HIGH);
  } else {
    digitalWrite(ledHijau, HIGH);
  }
}

void loop() {
  float d = bacaJarak();

  if (jumlahData < kapasitas) {
    dataBuffer[jumlahData] = d;
    jumlahData++;
  } else {
    for (int i = 0; i < kapasitas - 1; i++) {
      dataBuffer[i] = dataBuffer[i+1];
    }
    dataBuffer[kapasitas - 1] = d;
  }

  float rata = hitungRata();

  static int sampleIndex = 0;
  Serial.println();
  Serial.print("--- Sample ");
  Serial.print(sampleIndex);
  Serial.println(" ---");

  Serial.print("Data buffer (");
  Serial.print(jumlahData);
  Serial.print("/");
  Serial.print(kapasitas);
  Serial.print("): ");
  for (int i = 0; i < jumlahData; i++) {
    Serial.print(dataBuffer[i], 1);
    if (i < jumlahData - 1) Serial.print(" ");
  }
  Serial.println();

  Serial.print("Rata-rata: ");
  Serial.print(rata, 1);
  Serial.println(" cm");

  Serial.print("Free RAM: ");
  Serial.println(freeMemory());

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Avg:");
  lcd.print(rata,1);
  lcd.print("cm");
  lcd.setCursor(0,1);
  lcd.print("Free:");
  lcd.print(freeMemory());

  updateIndikator(rata);

  sampleIndex++;
  delay(1000);
}
