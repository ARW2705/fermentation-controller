/*==========================
      SYSTEM MONITOR     
==========================*/

/*
 * Serial print the char arrays that would be printed to the LCD
 * 
 * params: char*, char*, char*, char*
 * line1 - pointer to pre-constructed char array with top line content
 * line2 - pointer to pre-constructed char array with top-middle line content
 * line3 - pointer to pre-constructed char array with bottom-middle line content
 * line4 - pointer to pre-constructed char array with bottom line content
 * 
 * return: none
 */
void printInterfaceOutput(const char* line1, const char* line2, const char* line3, const char* line4) {
  if (strcmp(line1, "")) Serial.println(line1);
  if (strcmp(line2, "")) Serial.println(line2);
  if (strcmp(line3, "")) Serial.println(line3);
  if (strcmp(line4, "")) Serial.println(line4);
}

/*
 * Serial print analog input
 * 
 * params: int
 * val - normalized analog input
 * 
 * return: none
 */
void printInterfaceInput(int val) {
  Serial.println("\n*** Local interface input ***\n");
    switch(val) {
    case 200:
      // toggle display on/off
      // black button pressed 3.51kohm
      Serial.print("\nBlack button: ");
      break;
    case 400:
      // change lcd page
      // white button pressed 1.5kohm
      Serial.print("\nWhite button: ");
      break;
    case 600: 
      // increment values or toggle thermowell enabling
      // red button pressed 670ohm
      Serial.print("\nRed button: ");
      break;
    case 800: 
      // decrement values or toggle thermowell enabling
      // blue button pressed 270ohm
      Serial.print("\nBlue button:");
      break;
    default:
      Serial.print("\nButton state is invalid: ");
      Serial.println(val);
      Serial.println();
      break;
  }
}

/*
 * Serial print input/output refresh values
 * 
 * params: none
 * 
 * return: none
 */
void printIORefresh() {
  Serial.println("=============== IO VALUES ===============");
  Serial.println("\nINPUTS:");
  if (!chamberProbe.isEnabled && !thermowellProbe.isEnabled) {
    Serial.println("No sensors enabled, outputs will be deactivated");
  }
  if (chamberProbe.isEnabled) {
    Serial.print("Chamber temperature: ");
    Serial.print(chamberProbe.value);
    Serial.println(units);
    Serial.print("Chamber target: ");
    Serial.print(chamberProbe.target);
    Serial.println(units);
    Serial.print("Chamber tolerance: ");
    Serial.print(chamberProbe.tolerance);
    Serial.println(units);
    Serial.println("-----------------------------------------");
  }

  if (thermowellProbe.isEnabled) {
    Serial.print("Thermowell temperature: ");
    Serial.print(thermowellProbe.value);
    Serial.println(units);
    Serial.print("Thermowell target: ");
    Serial.print(thermowellProbe.target);
    Serial.println(units);
    Serial.print("Thermowell tolerance: ");
    Serial.print(thermowellProbe.tolerance);
    Serial.println(units);
    Serial.println("-----------------------------------------");
  }

  uint64_t _now = millis();
  Serial.println("\nOUTPUTS:");
  Serial.print("Cooldown remaining: ");
  int remaining = COOLDOWN_INTERVAL - (_now - timer.coolDownStart);
  if (remaining < 1) Serial.print("0");
  else Serial.print(remaining / 1000);
  Serial.println("s");
  Serial.println("-------------------------------------------");

  Serial.print("Current cycle: ");
  if (cooling.value) Serial.println("COOLING");
  else if (heating.value) Serial.println("HEATING");
  else if (fan.value) Serial.println("CIRCULATION");
  else Serial.println("NONE");

  Serial.println("-------------------------------------------");
  Serial.print("Cooling current status: ");
  if (cooling.value) Serial.println("ON");
  else Serial.println("OFF");
  Serial.print("Cooling target status: ");
  if (cooling.target) Serial.println("ON");
  else Serial.println("OFF");
  Serial.println("-------------------------------------------");
  
  Serial.print("Heating current status: ");
  if (heating.value) Serial.println("ON");
  else Serial.println("OFF");
  Serial.print("Heating target status: ");
  if (heating.target) Serial.println("ON");
  else Serial.println("OFF");
  Serial.println("-------------------------------------------");

  Serial.print("Fan current status: ");
  if (fan.value) Serial.println("ON");
  else Serial.println("OFF");
  Serial.print("Fan target status: ");
  if (fan.target) Serial.println("ON");
  else Serial.println("OFF");

  Serial.println();
  Serial.print("Available memory: ");
  Serial.println(ESP.getFreeHeap());
  Serial.println();

  Serial.println("\n============= END IO VALUES =============\n");
  Serial.flush();
}

/*
 * Setup and serial print the setup status
 * 
 * params: none
 * 
 * return: none
 */
void printSensorSetup() {
  Serial.println("Locating sensors...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" sensors");

  Serial.print("Parasite power: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  if (!sensors.getAddress(chamberProbe.deviceAddress, 0)) Serial.println("Cannot find address for Chamber Probe");
  if (!sensors.getAddress(thermowellProbe.deviceAddress, 1)) Serial.println("Cannot find address for Thermowell Probe");

  Serial.print("Chamber Probe Resolution: ");
  Serial.print(sensors.getResolution(chamberProbe.deviceAddress), DEC);
  Serial.println();

  Serial.print("Thermowell Probe Resolution: ");
  Serial.print(sensors.getResolution(thermowellProbe.deviceAddress), DEC);
  Serial.println();
  Serial.flush();
}


/*
 * Serial print of system status entry point
 * 
 * params: none
 * 
 * return: none
 */
void displaySystemStatus() {
  Serial.println("\n>>>>>> System Status <<<<<<\n");
  printIORefresh();
}

