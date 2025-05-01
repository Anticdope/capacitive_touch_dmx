#include <Arduino.h>
#include <TeensyDMX.h>

// Use the TeensyDMX “Sender” on Serial1 (DMX TX on pin 1)
namespace teensydmx = ::qindesign::teensydmx;
teensydmx::Sender dmxTx{Serial1};

const uint8_t triggerPin       = 8;    // external open-drain pulls this LOW to trigger
const uint8_t ledPin           = 13;
const uint8_t NUM_RGB_FIX      = 6;    // six RGB fixtures
const uint8_t CH_PER_RGB       = 3;    // each uses R,G,B
const uint8_t CH_LAST_FIX      = 1;    // one single-channel fixture at the end
const uint16_t TOTAL_DMX_CH    = NUM_RGB_FIX * CH_PER_RGB + CH_LAST_FIX;

void setup() {
  // Configure trigger pin as pull-up input
  pinMode(triggerPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWriteFast(ledPin, HIGH);

  // Start DMX transmission
  dmxTx.begin();  // begin asynchronous DMX output :contentReference[oaicite:0]{index=0}

  allOff();
}

static void setRGB(uint8_t fixtureIndex, uint8_t val) {
  // Sets R, G, B = val for fixtureIndex [0..5]
  uint16_t base = fixtureIndex * CH_PER_RGB + 1;
  dmxTx.set(base    , val);
  dmxTx.set(base + 1, val);
  dmxTx.set(base + 2, val);
}

void loop() {

  bool trigger = digitalRead(triggerPin);
  // On falling edge and not yet run:
  if (trigger == LOW) {
    digitalWriteFast(ledPin, LOW);
    // 1) Fixtures 1 & 6 → full white
    setRGB(0, 255);
    setRGB(3, 255);

    delay(500);

    // 2) Fixtures 2 & 5 → full white
    setRGB(1, 255);
    setRGB(4, 255);

    delay(250);

    // 3) Fixtures 3 & 4 → full white
    setRGB(2, 255);
    setRGB(5, 255);

    // 4) Fade last fixture upwards over 1 second
    uint16_t lastCh = NUM_RGB_FIX * CH_PER_RGB + 1;
    for (uint16_t v = 0; v <= 255; v++) {
      dmxTx.set(lastCh, v);
      delay(1000 / 255); // ≈3.9 ms per step → ~1 s total
    }
    delay(3000);
    allOff();
    digitalWriteFast(ledPin, HIGH);
  }
  // Reset on release
  else {
    allOff();
  }
}


  void allOff(){
    for (uint16_t ch = 1; ch <= TOTAL_DMX_CH; ch++) {
      dmxTx.set(ch, 0);
    }
}