/****************  PROGRAMMER – OLED‑Only (no Wi‑Fi)  ****************/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>

/* ---------- I²C & OLED ---------- */
#define SDA_PIN   21
#define SCL_PIN   22
#define OLED_ADDR 0x3C
#define OLED_RST  16          // tie panel RST here; comment out if the pad isn’t broken‑out

Adafruit_SSD1306 display(128, 64, &Wire, OLED_RST);
bool hasOled = false;

/* ---------- buttons ---------- */
#define BTN_UP     32
#define BTN_DOWN   33
#define BTN_SELECT 25
Bounce btnUp = Bounce(), btnDn = Bounce(), btnSel = Bounce();

/* ---------- mode list ---------- */
struct Mode { const char *name; };
Mode modes[] = {
  {"Pixel (54 ch)"},
  {"1‑Ch White"},
  {"3‑Ch RGB"}
};
const uint8_t MODE_COUNT = sizeof(modes) / sizeof(modes[0]);
int currentIndex = 0;

/* ---------- helpers ---------- */
void splash() {
  if (!hasOled) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 16);
  display.println(F("BOOT OK"));
  display.display();
  delay(800);
}
void drawUI(bool logSerial = false) {
  if (hasOled) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(F("Select"));
    display.setTextSize(1);
    for (uint8_t i = 0; i < MODE_COUNT; i++) {
      display.print(i == currentIndex ? F("> ") : F("  "));
      display.println(modes[i].name);
    }
    display.display();
  }
  if (logSerial) {
    Serial.println("------ UI ------");
    for (uint8_t i = 0; i < MODE_COUNT; i++) {
      Serial.printf("%c %u: %s\n",
                    (i == currentIndex ? '>' : ' '), i, modes[i].name);
    }
    Serial.println("----------------");
  }
}

/* ---------- Arduino setup ---------- */
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== OLED‑only Programmer ===");

#ifdef OLED_RST                        // give the panel a hard HW reset
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(10);
  digitalWrite(OLED_RST, HIGH);
#endif

  /* I²C & OLED */
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);              // 100 kHz I²C
  Wire.setClockStretchLimit(2000000); // <-- 2 ms stretch allowance

  hasOled = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

  if (hasOled) {
    Serial.println("[OLED] 128×64 ready");
    splash();
  } else {
    Serial.println("[OLED] init failed – running headless");
  }

  /* buttons */
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  btnUp.attach(BTN_UP);  btnUp.interval(10);
  btnDn.attach(BTN_DOWN);btnDn.interval(10);
  btnSel.attach(BTN_SELECT);btnSel.interval(10);

  drawUI(true);
}

/* ---------- main loop ---------- */
void loop() {
  btnUp.update(); btnDn.update(); btnSel.update();

  if (btnUp.fell()) {
    currentIndex = (currentIndex + MODE_COUNT - 1) % MODE_COUNT;
    Serial.println("[BTN] UP");
    drawUI(true);
  }
  if (btnDn.fell()) {
    currentIndex = (currentIndex + 1) % MODE_COUNT;
    Serial.println("[BTN] DOWN");
    drawUI(true);
  }
  if (btnSel.fell()) {
    Serial.printf("[BTN] SELECT – would upload \"%s\"\n",
                  modes[currentIndex].name);

    if (hasOled) {
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print(F("Selected: "));
      display.println(modes[currentIndex].name);
      display.display();
    }
    delay(800);
    drawUI();
  }
}
