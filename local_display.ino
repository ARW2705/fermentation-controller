/*=========================
      LOCAL DISPLAY     
=========================*/

/*
 * LCD page router
 * 
 * params: char
 * page - character that corresponds to the requested page to display
 * 
 * return: none
 */
void loadLCDPage(char page) {
  turnOnScreen();
  switch(page) {
    case 'm':
      loadLCDMainPage();
      break;
    case 'c':
      loadLCDSetTemperaturePage();
      break;
    case 't':
      loadLCDSetTolerancePage();
      break;
    case 'r':
      loadLCDToggleCoolingEnabled();
      break;
    case 'h':
      loadLCDToggleHeatingEnabled();
      break;
    case 's':
      loadLCDToggleThermowellEnabled();
      break;
    default:
      loadLCDMainPage();
      break;
  }
  Serial.flush();
}

/*
 * Load the Main/Home screen
 * 
 * params: none
 * 
 * return: none
 */
void loadLCDMainPage() {
  /*
   * Set:70.0F    +/-1.5F
   * << COOLING  CYCLE >>
   * Chamber        74.5F
   * Thermowell     75.0F
   */
  char line1[LCD_CHAR_LIMIT] = "Set:";
  char line2[LCD_CHAR_LIMIT] = "";
  char line3[LCD_CHAR_LIMIT] = "Chamber        ";
  char line4[LCD_CHAR_LIMIT] = "Thermowell     ";

  char _setTemperature[5];
  char _setTolerance[4];
  char _units[2] = {units};
  sprintf(_setTemperature, "%.1f", monitored->target);
  sprintf(_setTolerance, "%.1f", monitored->tolerance);

  strcat(line1, _setTemperature);
  strcat(line1, _units);
  strcat(line1, "    +/-");
  strcat(line1, _setTolerance);
  strcat(line1, _units);
 
  int coolDownRemaining = (COOLDOWN_INTERVAL - (millis() - timer.coolDownStart)) / 1000;
  if (coolDownRemaining > 0 && coolDownRemaining < 120) {
    char _coolDown[4];
    sprintf(_coolDown, "%d", coolDownRemaining);
    strcpy(line2, "<< COOLDOWN  ");
    if (coolDownRemaining < 10) {
      strcat(line2, "  ");
    } else if (coolDownRemaining < 100) {
      strcat(line2, " ");
    } 
    strcat(line2, _coolDown);
    strcat(line2, "s >>");
  } else if (cooling.target) {
    strcpy(line2, "<< COOLING  CYCLE >>");
  } else if (heating.target) {
    strcpy(line2, "<< HEATING  CYCLE >>");
  } else if (fan.target) {
    strcpy(line2, "< AIR  CIRCULATION >");
  } else {
    strcpy(line2, ">> TEMPERATURE OK <<");
  }

  char _chamberValue[5];
  sprintf(_chamberValue, "%.1f", chamberProbe.value);
  strcat(line3, _chamberValue);
  strcat(line3, _units);

  char _thermowellValue[5];
  sprintf(_thermowellValue, "%.1f", thermowellProbe.value);

  if (thermowellProbe.isEnabled) {
    strcat(line4, _thermowellValue);
    strcat(line4, _units);
  } else {
    strcat(line4, "  OFF");
  }

  if (debug) printInterfaceOutput(line1, line2, line3, line4);

  printToLCD(line1, line2, line3, line4);

  currentPage = 'm';
}

/*
 * Load the Set Temperature screen
 * 
 * params: none
 * 
 * return: none
 */
void loadLCDSetTemperaturePage() {
  /*
   * **SET  TEMPERATURE**
   *   Currently: 70.0F
   *   Change to: 72.0F
   *   
   */
  char line1[LCD_CHAR_LIMIT] = "**SET  TEMPERATURE**";
  char line2[LCD_CHAR_LIMIT] = "  Currently: ";
  char line3[LCD_CHAR_LIMIT] = "  Change to: ";
  
  char _currentSetTemperature[5];
  char _newSetTemperature[5];
  char _units[2] = {units};
  sprintf(_currentSetTemperature, "%.1f", monitored->target);
  sprintf(_newSetTemperature, "%.1f", request.temperature);
  
  strcat(line2, _currentSetTemperature);
  strcat(line2, _units);
  strcat(line2, "  ");
  
  strcat(line3, _newSetTemperature);
  strcat(line3, _units);
  strcat(line3, "  ");

  if (debug) printInterfaceOutput(line1, line2, line3, "");

  printToLCD(line1, line2, line3, LCD_BLANK_LINE);

  currentPage = 'c';
}

/*
 * Load the Set Tolerance screen
 * 
 * params: none
 * 
 * return: none
 */
void loadLCDSetTolerancePage() {
  /*
   * ***SET  TOLERANCE***
   *   Currently: 1.5F
   *   Change to: 2.0F
   *   
   */
  char line1[LCD_CHAR_LIMIT] = "***SET  TOLERANCE***";
  char line2[LCD_CHAR_LIMIT] = "  Currently: ";
  char line3[LCD_CHAR_LIMIT] = "  Change to: ";

  char _currentTolerance[4];
  char _newTolerance[4];
  char _units[2] = {units};
  sprintf(_currentTolerance, "%.1f", monitored->tolerance);
  sprintf(_newTolerance, "%.1f", request.tolerance);

  strcat(line2, _currentTolerance);
  strcat(line2, _units);
  strcat(line2, "   ");
  
  strcat(line3, _newTolerance);
  strcat(line3, _units);
  strcat(line3, "   ");

  if (debug) printInterfaceOutput(line1, line2, line3, "");

  printToLCD(line1, line2, line3, LCD_BLANK_LINE);

  currentPage = 't';
}

/*
 * Load the Cooling system activation toggle screen
 * 
 * params: none
 * 
 * return: none
 */
void loadLCDToggleCoolingEnabled() {
  /*
   * ACTIVATE COOLING SYS
   * 
   *   Cooling  Enabled   
   *    
   */
  char line1[LCD_CHAR_LIMIT] = "ACTIVATE COOLING SYS";
  char line3[LCD_CHAR_LIMIT] = "";
  
  if (request.isCoolingEnabled) {
    strcpy(line3, "  Cooling  Enabled  ");
  } else {
    strcpy(line3, "  Cooling Disabled  ");
  }

  if (debug) printInterfaceOutput(line1, "", line3, "");

  printToLCD(line1, LCD_BLANK_LINE, line3, LCD_BLANK_LINE);
  
  currentPage = 'r';
}

/*
 * Load the Heating system activation toggle screen
 * 
 * params: none
 * 
 * return: none
 */
void loadLCDToggleHeatingEnabled() {
  /*
   * ACTIVATE HEATING SYS  
   * 
   *   Heating  Enabled   
   *    
   */
  char line1[LCD_CHAR_LIMIT] = "ACTIVATE HEATING SYS";
  char line3[LCD_CHAR_LIMIT] = "";
  
  if (request.isHeatingEnabled) {
    strcpy(line3, "  Heating  Enabled  ");
  } else {
    strcpy(line3, "  Heating Disabled  ");
  }

  if (debug) printInterfaceOutput(line1, "", line3, "");

  printToLCD(line1, LCD_BLANK_LINE, line3, LCD_BLANK_LINE);
  
  currentPage = 'h';
}

/*
 * Load the Thermowell activation toggle screen
 * 
 * params: none
 * 
 * return: none
 */
void loadLCDToggleThermowellEnabled() {
  /*
   * ACTIVATE  THERMOWELL
   * 
   *    Sensor Enabled   
   *    
   */
  char line1[LCD_CHAR_LIMIT] = "ACTIVATE  THERMOWELL";
  char line3[LCD_CHAR_LIMIT] = "";
  
  if (request.isThermowellEnabled) {
    strcpy(line3, "   Sensor Enabled   ");
  } else {
    strcpy(line3, "  Sensor  Disabled  ");
  }

  if (debug) printInterfaceOutput(line1, "", line3, "");

  printToLCD(line1, LCD_BLANK_LINE, line3, LCD_BLANK_LINE);

  currentPage = 's';
}

/*
 * Print char arrays to LCD
 * 
 * params: char*, char*, char*, char*
 * line1 - pointer to pre-constructed char array with top line content
 * line2 - pointer to pre-constructed char array with top-middle line content
 * line3 - pointer to pre-constructed char array with bottom-middle line content
 * line4 - pointer to pre-constructed char array with bottom line content
 * 
 * return: none
 */
void printToLCD(const char* line1, const char* line2, const char* line3, const char* line4) {
  delay(100);
  
  lcd.setCursor(0, 0);
  lcd.print(line1);

  lcd.setCursor(0, 1);
  lcd.print(line2);

  lcd.setCursor(0, 2);
  lcd.print(line3);

  lcd.setCursor(0, 3);
  lcd.print(line4);
}

/*
 * Turn on LCD backlight and refresh
 * 
 * params: none
 * 
 * reutrn: none
 */
void turnOnScreen() {
  timer.lcdRefreshStart = millis();
  if (!isDisplayOn) {
    Serial.println("Turning screen on");
    isDisplayOn = true;
    lcd.clear();
    lcd.noCursor();
    lcd.backlight();
    timer.lcdRefreshStart -= LCD_CYCLE_INTERVAL;
    timer.lcdTimeoutStart = millis();
  }
}

/*
 * Clear the LCD and turn off backlight
 * 
 * params: none
 * 
 * return: none
 */
void turnOffScreen() {
  if (isDisplayOn) printToLCD(LCD_BLANK_LINE, LCD_BLANK_LINE, LCD_BLANK_LINE, LCD_BLANK_LINE);
  isDisplayOn = false;
  lcd.noBacklight();
}
