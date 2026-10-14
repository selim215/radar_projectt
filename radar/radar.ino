#include <Servo.h>
#include <HCSR04.h>

// Çıkış pinleri
#define servoPin 9
#define buzzerAktif 8  // Pin 8: Aktif Buzzer (Sadece HIGH/LOW ile çalışır)
#define buzzerPasif 3  // Pin 3: Pasif Buzzer (Frekans ile melodi çalar)
#define kirmiziLed 7
#define sariLed 6
#define yesilLed 5

// Ultrasonik sensör (Trig: 13, Echo: 12)
UltraSonicDistanceSensor distanceSensor(13, 12);

// Servo motor
Servo radarServo;

int mesafe = 0;

// Türk Marşı Nota Frekansları (Hz) ve Baz Süreleri (ms)
const int notalar[] = {
  494, 440, 415, 440, 523, 0,
  587, 523, 494, 523, 659, 0,
  698, 659, 622, 659, 784, 698, 659, 587, 523, 494, 440
};

const int bazSureler[] = {
  100, 100, 100, 100, 200, 80,
  100, 100, 100, 100, 200, 80,
  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 250
};

const int toplamNota = sizeof(notalar) / sizeof(notalar[0]);
int aktifNotaIndex = 0;
unsigned long sonNotaZamani = 0;

void setup() {
  Serial.begin(9600);

  radarServo.attach(servoPin);

  pinMode(buzzerAktif, OUTPUT);
  pinMode(buzzerPasif, OUTPUT);
  pinMode(kirmiziLed, OUTPUT);
  pinMode(sariLed, OUTPUT);
  pinMode(yesilLed, OUTPUT);

  digitalWrite(buzzerAktif, LOW);
  digitalWrite(buzzerPasif, LOW);
  digitalWrite(kirmiziLed, LOW);
  digitalWrite(sariLed, LOW);
  digitalWrite(yesilLed, HIGH);
}

// Pin 3'te melodiyi çalan, Pin 8'de ritim/uyarı vuruşu yapan fonksiyon
void hibritBuzzerCal(int frekans, int sureMs, int guncelMesafe) {
  // Eylemsiz/Sus (Rest) notası
  if (frekans == 0) {
    digitalWrite(buzzerAktif, LOW);
    digitalWrite(buzzerPasif, LOW);
    delay(sureMs);
    return;
  }

  unsigned long periyotUs = 1000000UL / frekans;
  unsigned long yarimPeriyot = periyotUs / 2;
  unsigned long donguSayisi = ((unsigned long)sureMs * 1000UL) / periyotUs;

  // Aktif buzzer kritik mesafede (<=12cm) her notanın başında 15ms vurur,
  // orta mesafede (13-28cm) her 2 notada bir vurur.
  bool aktifBuzzerVursun = (guncelMesafe <= 12) || (aktifNotaIndex % 2 == 0);
  unsigned long aktifBuzzerBitisUs = 15000UL; // 15 ms kısa bip darbesi
  unsigned long gecenSureUs = 0;

  if (aktifBuzzerVursun) {
    digitalWrite(buzzerAktif, HIGH);
  }

  for (unsigned long i = 0; i < donguSayisi; i++) {
    // Pasif buzzer (Pin 3) için frekans üretimi
    digitalWrite(buzzerPasif, HIGH);
    delayMicroseconds(yarimPeriyot);
    digitalWrite(buzzerPasif, LOW);
    delayMicroseconds(yarimPeriyot);

    // Aktif buzzer (Pin 8) darbe süresi dolunca sustur (melodiyi bastırmaması için)
    gecenSureUs += periyotUs;
    if (gecenSureUs >= aktifBuzzerBitisUs) {
      digitalWrite(buzzerAktif, LOW);
    }
  }

  digitalWrite(buzzerAktif, LOW);
  digitalWrite(buzzerPasif, LOW);
}

// Durum, LED ve Hıza Duyarlı Müzik Kontrol Fonksiyonu
void tehlikeKontrol(int d) {
  // 1. GÜVENLİ DURUM: 28 cm'den uzak veya cisim yok
  if (d <= 0 || d > 28) {
    digitalWrite(kirmiziLed, LOW);
    digitalWrite(sariLed, LOW);
    digitalWrite(yesilLed, HIGH);
    digitalWrite(buzzerAktif, LOW);
    digitalWrite(buzzerPasif, LOW);
    return;
  }

  // LED Durumlarını Güncelle
  if (d <= 12) {
    // KRİTİK TEHLİKE (1 - 12 cm)
    digitalWrite(kirmiziLed, HIGH);
    digitalWrite(sariLed, LOW);
    digitalWrite(yesilLed, LOW);
  } else {
    // ORTA TEHLİKE (13 - 28 cm)
    digitalWrite(kirmiziLed, LOW);
    digitalWrite(sariLed, HIGH);
    digitalWrite(yesilLed, LOW);
  }

  // Mesafeye göre hız hesabı (yaklaştıkça hızlanır, uzaklaştıkça yavaşlar)
  int sinirliMesafe = constrain(d, 2, 28);
  int notaArasiBekleme = map(sinirliMesafe, 2, 28, 15, 220); 
  float hizCarpani = map(sinirliMesafe, 2, 28, 35, 130) / 100.0;

  if (millis() - sonNotaZamani >= (unsigned long)notaArasiBekleme) {
    int guncelNotaSuresi = bazSureler[aktifNotaIndex] * hizCarpani;
    
    hibritBuzzerCal(notalar[aktifNotaIndex], guncelNotaSuresi, d);
    
    aktifNotaIndex = (aktifNotaIndex + 1) % toplamNota;
    sonNotaZamani = millis();
  }
}

void radarAdimi(int aci, int beklemeMs) {
  radarServo.write(aci);
  delay(beklemeMs);

  mesafe = distanceSensor.measureDistanceCm();
  tehlikeKontrol(mesafe);

  Serial.print(aci);
  Serial.print(",");
  Serial.print(mesafe);
  Serial.print(".");
}

void loop() {
  // 0 dereceden 180 dereceye doğru tarama
  for (int aci = 0; aci <= 180; aci++) {
    radarAdimi(aci, 10);
  }

  // 180 dereceden 0 dereceye geri dönüş taraması
  for (int aci = 180; aci >= 0; aci--) {
    radarAdimi(aci, 30);
  }
}