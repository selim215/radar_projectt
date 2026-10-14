import processing.serial.*;
import java.awt.event.KeyEvent;
import java.io.IOException;

Serial myPort;
String angle = "";
String distance = "";
String data = "";
int iAngle = 0;
int iDistance = 0;
int index1 = 0;

void setup() {
  size(1200, 700);
  smooth();
  
  // Doğrudan COM5 portuna 9600 baud hızıyla bağlanıyoruz
  myPort = new Serial(this, "COM18", 9600); 
  myPort.bufferUntil('.'); 
}

void draw() {
  fill(98, 245, 31);
  noStroke();
  fill(0, 4); 
  rect(0, 0, width, height - height * 0.065); 
  
  fill(98, 245, 31);
  drawRadar(); 
  drawLine();
  drawObject();
  drawText();
}

void serialEvent(Serial myPort) {
  try {
    data = myPort.readStringUntil('.');
    if (data != null) {
      data = data.trim(); // Boşlukları ve satır sonlarını temizle
      if (data.endsWith(".")) {
        data = data.substring(0, data.length() - 1);
      }
      
      index1 = data.indexOf(",");
      if (index1 > 0) {
        angle = data.substring(0, index1).trim();
        distance = data.substring(index1 + 1).trim();
        
        // Türkçe yerel ayar hatasını engelleyen güvenli sayı dönüşümleri:
        iAngle = Integer.valueOf(angle);
        iDistance = Integer.valueOf(distance);
      }
    }
  } catch(Exception e) {
    // Eksik/bozuk gelen paketleri görmezden gel, programın çökmesini engelle
  }
}

void drawRadar() {
  pushMatrix();
  translate(width / 2, height - height * 0.074);
  noFill();
  strokeWeight(2);
  stroke(98, 245, 31);
  
  // Radar mesafe çemberleri (10 cm, 20 cm, 30 cm, 40 cm)
  arc(0, 0, (width - width * 0.0625), (width - width * 0.0625), PI, TWO_PI);
  arc(0, 0, (width - width * 0.27), (width - width * 0.27), PI, TWO_PI);
  arc(0, 0, (width - width * 0.479), (width - width * 0.479), PI, TWO_PI);
  arc(0, 0, (width - width * 0.687), (width - width * 0.687), PI, TWO_PI);
  
  // Açı kılavuz çizgileri (30°, 60°, 90°, 120°, 150°)
  line(-width / 2, 0, width / 2, 0);
  line(0, 0, (-width / 2) * cos(radians(30)), (-width / 2) * sin(radians(30)));
  line(0, 0, (-width / 2) * cos(radians(60)), (-width / 2) * sin(radians(60)));
  line(0, 0, (-width / 2) * cos(radians(90)), (-width / 2) * sin(radians(90)));
  line(0, 0, (-width / 2) * cos(radians(120)), (-width / 2) * sin(radians(120)));
  line(0, 0, (-width / 2) * cos(radians(150)), (-width / 2) * sin(radians(150)));
  popMatrix();
}

void drawObject() {
  pushMatrix();
  translate(width / 2, height - height * 0.074);
  strokeWeight(9);
  stroke(255, 10, 10); // Tespit edilen cisim için KIRMIZI hedef rengi
  
  float pixsDistance = iDistance * ((height - height * 0.1666) * 0.025);
  
  // 40 cm menzil içindeki nesneleri ekrana kırmızı hedef çizgisi/noktası olarak bas
  if (iDistance > 0 && iDistance < 40) {
    line(pixsDistance * cos(radians(iAngle)), -pixsDistance * sin(radians(iAngle)), 
         (width - width * 0.505) * cos(radians(iAngle)), -(width - width * 0.505) * sin(radians(iAngle)));
  }
  popMatrix();
}

void drawLine() {
  pushMatrix();
  strokeWeight(9);
  stroke(30, 250, 60); // Dönen yeşil radar tarama çizgisi
  translate(width / 2, height - height * 0.074);
  line(0, 0, (height - height * 0.12) * cos(radians(iAngle)), -(height - height * 0.12) * sin(radians(iAngle)));
  popMatrix();
}

void drawText() {
  fill(0);
  noStroke();
  rect(0, height - height * 0.0648, width, height);
  fill(98, 245, 31);
  textSize(24);
  text("Aci: " + iAngle + " °", width * 0.05, height - height * 0.02);
  text("Mesafe: " + (iDistance > 40 || iDistance <= 0 ? "Menzil Disi" : iDistance + " cm"), width * 0.4, height - height * 0.02);
}
