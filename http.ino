/*==========================
      HTTP FUNCTIONS     
==========================*/

/*
 * Handle http get request
 * 
 * params: none
 * 
 * return: none
 */
void handleGetRequest() {
  int numComponents = 0;
  bool cp = false; // chamber probe
  bool tp = false; // thermowell probe
  bool ht = false; // heating
  bool cl = false; // cooling
  bool fn = false; // fans
  
  // check each component for validity and count total number of valid components
  if (isIOPlausible(chamberProbe)) {
    cp = true;
    numComponents++;
  }
  if (isIOPlausible(thermowellProbe)) {
    tp = true;
    numComponents++;
  }
  if (isIOPlausible(heating)) {
    ht = true;
    numComponents++;
  }
  if (isIOPlausible(cooling)) {
    cl = true;
    numComponents++;
  }
  if (isIOPlausible(fan)) {
    fn = true;
    numComponents++;
  }

  // allocate memory for message depending on how many components are available
  char* chamberStatus = (char*)malloc(sizeof(char) * (numComponents * 200));
  composeResponse(chamberStatus, cp, tp, ht, cl, fn);
  server.send(200, "text/json", chamberStatus);
  free(chamberStatus);
}

/*
 * Handle http post request
 * 
 * params: none
 * 
 * return: none
 */
void handlePostRequest() {
  // decode json post request
  String data = server.arg("plain");
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(data);

  if (!root.success()) {
    server.send(500, "text/plain", "Failed to parse json");
    return;
  } else {
    int id = root["id"];
    // update values if present
    switch (id) {
      case 1:
        if (root.containsKey("isEnabled")) {
          chamberProbe.isEnabled = root["isEnabled"];
        }
        if (root.containsKey("target")) {
          chamberProbe.target = root["target"];
        }
        if (root.containsKey("location")) {
          memset(chamberProbe.location, '\0', 50);
          strcpy(chamberProbe.location, root["location"]);
        }
        if (root.containsKey("tolerance")) {
          chamberProbe.tolerance = root["tolerance"];
        }
        break;
      case 2:
        if (root.containsKey("isEnabled")) {
          thermowellProbe.isEnabled = root["isEnabled"];
        }
        if (root.containsKey("target")) {
          thermowellProbe.target = root["target"];
        }
        if (root.containsKey("location")) {
          memset(thermowellProbe.location, '\0', 50);
          strcpy(thermowellProbe.location, root["location"]);
        }
        if (root.containsKey("tolerance")) {
          thermowellProbe.tolerance = root["tolerance"];
        }
        break;
      case 3:
        if (root.containsKey("isEnabled")) {
          heating.isEnabled = root["isEnabled"];
        }
        if (root.containsKey("location")) {
          memset(heating.location, '\0', 50);
          strcpy(heating.location, root["location"]);
        }
        break;
      case 4:
        if (root.containsKey("isEnabled")) {
          cooling.isEnabled = root["isEnabled"];
        }
        if (root.containsKey("location")) {
          memset(cooling.location, '\0', 50);
          strcpy(cooling.location, root["location"]);
        }
        break;
      case 5:
        if (root.containsKey("isEnabled")) {
          fan.isEnabled = root["isEnabled"];
        }
        if (root.containsKey("location")) {
          memset(fan.location, '\0', 50);
          strcpy(fan.location, root["location"]);
        }
        break;
      default:
        break;
    }
  }
}

/*
 * Create http response message
 * 
 * params: char*, bool, bool, bool, bool, bool
 * response - pointer to char array that will contain the constructed response
 * cpOK - true if chamber probe is enabled and value is plausible
 * tpOK - true if thermowell probe is enabled and value is plausible
 * heatOK - true if heating element is enabled and value is plausible
 * coolOK - true if cooling system is enabled and value is plausible
 * fanOK - true if ventilation system is enabled and value is plausible
 * 
 * return: none
 */
void composeResponse(char* response, bool cpOK, bool tpOK, bool heatOK, bool coolOK, bool fanOK) {
  char id[3];
  char value[5];
  char target[5];
  char tolerance[5];
  strcpy(response, "{");
  
  if (cpOK) {
    strcat(response, "\"chamberProbe\": {");
    strcat(response, "\"isEnabled\": true, \"ioType\": \"");
    strcat(response, chamberProbe.ioType);
    strcat(response, "\", \"type\": \"");
    strcat(response, chamberProbe.type);
    strcat(response, "\", \"location\": \"");
    strcat(response, chamberProbe.location);
    strcat(response, "\", \"id\": ");
    sprintf(id, "%1d", chamberProbe.id);
    strcat(response, id);
    strcat(response, ", \"value\": ");
    sprintf(value, "%.1f", chamberProbe.value);
    strcat(response, value);
    strcat(response, ", \"target\": ");
    sprintf(target, "%.1f", chamberProbe.target);
    strcat(response, target);
    strcat(response, ", \"tolerance\": ");
    sprintf(tolerance, "%.1f", chamberProbe.tolerance);
    strcat(response, tolerance);
    strcat(response, "}");
  }
  
  if (tpOK) {
    strcat(response, "\"thermowellProbe\": {");
    strcat(response, "\"isEnabled\": true, \"ioType\": \"");
    strcat(response, thermowellProbe.ioType);
    strcat(response, "\", \"type\": \"");
    strcat(response, thermowellProbe.type);
    strcat(response, "\", \"location\": \"");
    strcat(response, thermowellProbe.location);
    strcat(response, "\", \"id\": ");
    sprintf(id, "%1d", chamberProbe.id);
    strcat(response, id);
    strcat(response, ", \"value\": ");
    sprintf(value, "%.1f", thermowellProbe.value);
    strcat(response, value);
    strcat(response, ", \"target\": ");
    sprintf(target, "%.1f", thermowellProbe.target);
    strcat(response, target);
    strcat(response, ", \"tolerance\": ");
    sprintf(tolerance, "%.1f", thermowellProbe.tolerance);
    strcat(response, tolerance);
    strcat(response, "}");
  }
  
  if (heatOK) {
    strcat(response, "\"heating\": {");
    strcat(response, "\"isEnabled\": true, \"ioType\": \"");
    strcat(response, heating.ioType);
    strcat(response, "\", \"type\": \"");
    strcat(response, heating.type);
    strcat(response, "\", \"location\": \"");
    strcat(response, heating.location);
    strcat(response, "\", \"id\": ");
    sprintf(id, "%1d", chamberProbe.id);
    strcat(response, id);
    strcat(response, ", \"value\": ");
    sprintf(value, "%.1f", heating.target);
    strcat(response, value);
    strcat(response, ", \"target\": ");
    sprintf(target, "%.1f", heating.value);
    strcat(response, target);
    strcat(response, "}");
  }
  
  if (coolOK) {
    strcat(response, "\"cooling\": {");
    strcat(response, "\"isEnabled\": true, \"ioType\": \"");
    strcat(response, cooling.ioType);
    strcat(response, "\", \"type\": \"");
    strcat(response, cooling.type);
    strcat(response, "\", \"location\": \"");
    strcat(response, cooling.location);
    strcat(response, "\", \"id\": ");
    sprintf(id, "%1d", chamberProbe.id);
    strcat(response, id);
    strcat(response, ", \"value\": ");
    sprintf(value, "%.1f", cooling.value);
    strcat(response, value);
    strcat(response, ", \"target\": ");
    sprintf(target, "%.1f", cooling.target);
    strcat(response, target);
    strcat(response, "}");
  }
  
  if (fanOK) {
    strcat(response, "\"fan\": {");
    strcat(response, "\"isEnabled\": true, \"ioType\": \"");
    strcat(response, fan.ioType);
    strcat(response, "\", \"type\": \"");
    strcat(response, fan.type);
    strcat(response, "\", \"location\": \"");
    strcat(response, fan.location);
    strcat(response, "\", \"id\": ");
    sprintf(id, "%1d", chamberProbe.id);
    strcat(response, id);
    strcat(response, ", \"value\": ");
    sprintf(value, "%.1f", fan.value);
    strcat(response, value);
    strcat(response, ", \"target\": ");
    sprintf(target, "%.1f", fan.target);
    strcat(response, target);
    strcat(response, "}");
  }

  strcat(response, "}");
}

/*
 * Check if input/output component is enabled and value is plausible
 * 
 * params: object ref
 * component - reference to an ioComponent
 * 
 * return: boolean
 * - true if component is both enabled and has a plausible value
 */
bool isIOPlausible(ioComponent& component) {
  if (component.isEnabled) {
    if (!strcmp(component.ioType, "INPUT") 
        && component.value > (MIN_TEMP - 5) 
        && component.value < (MAX_TEMP + 5)) {
      return true;
    }
    else if (!strcmp(component.ioType, "OUTPUT") 
              && component.value > -1 
              && component.value < 2) {
      return true;
    }
  }
  return false;
}
