#include "Keyboard.h"

Keyboard::Keyboard(void)
: matrix(), keymap(), hid(), power(), restTimer() {
  idleTime = 0;
  lastBatteryWriteTime = 0;
  int batterLEDOnMinutes = 3;
  batteryLEDOnDuration = batterLEDOnMinutes * 60 * 1000;
}

void Keyboard::begin(void) {
  hid.begin();
  matrix.begin();
  led.begin();
}

void Keyboard::update(void) {
   
  auto update = matrix.scan();
  if (update) {
    idleTime = millis();
    keymap.update(&matrix);
    hid.sendKeys(&keymap);
  }

  led.process();
  indicateBatteryLevel();
  // restCheck();
  sleepCheck();
}

void Keyboard::indicateBatteryLevel(void) {
  if ((millis() - lastBatteryWriteTime) > (batteryWriteIntervalSeconds * 1000)) {
    uint8_t percentage = power.batteryRemainingPercentage();
    hid.sendBattery(percentage);
    lastBatteryWriteTime = millis();
  }
}

void Keyboard::sleepCheck(void) {
  if ((millis() - idleTime) > (sleepMinutes * 60 * 1000)) {
    led.offAll();
    matrix.sleep();    

    uint8_t sd_en;
    sd_softdevice_is_enabled(&sd_en);

    if (sd_en) {
      sd_power_system_off();
    } else {
      NRF_POWER->SYSTEMOFF = 1;
    }
  }
}

void Keyboard::restCheck(void) {

  restTimer.process(idleTime);

  if(restTimer.getNeedsBreak()) {
    if(restTimer.isShortBreakPeriod()) {
      led.flashWithSpeed(LOW); 
    } else {
      led.flashWithSpeed(HIGH);
    }
  } else {
    led.stopFlashing();
  }
 
}
