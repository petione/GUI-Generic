/*
  Copyright (C) krycha88

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "SuplaWebServer.h"
#include "SuplaDeviceGUI.h"
SuplaWebServer::SuplaWebServer() {

}

void SuplaWebServer::begin() {
  this->createWebServer();

  strcpy(this->www_username, ConfigManager->get(KEY_LOGIN)->getValue());
  strcpy(this->www_password, ConfigManager->get(KEY_LOGIN_PASS)->getValue());

  httpUpdater.setup(&httpServer, UPDATE_PATH, www_username, www_password);
  httpServer.begin();
}

void SuplaWebServer::iterateAlways() {
  httpServer.handleClient();
}

void SuplaWebServer::createWebServer() {
    String path = PATH_START;
  httpServer.on(path, HTTP_GET, std::bind(&SuplaWebServer::handle, this));
  path = PATH_START;
  path += PATH_SET;
  httpServer.on(path, std::bind(&SuplaWebServer::handleSave, this));
  path = PATH_START;
  path += PATH_SERCH;
  httpServer.on(path, std::bind(&SuplaWebServer::handleSearchDS, this));
  path = PATH_START;
  path += PATH_SAVE_DS;
  httpServer.on(path, std::bind(&SuplaWebServer::handleDSSave, this));
  path = PATH_START;
  path += PATH_UPDATE;  
  httpServer.on(path, std::bind(&SuplaWebServer::handleFirmwareUp, this));
  path = PATH_START;
  path += PATH_REBOT;  
  httpServer.on(path, std::bind(&SuplaWebServer::supla_webpage_reboot, this));
  path = PATH_START;
  path += PATH_RELAY;
  httpServer.on(path, std::bind(&SuplaWebServer::handleRelay, this));
  path = PATH_START;
  path += PATH_SAVE_RELAY;
  httpServer.on(path, std::bind(&SuplaWebServer::handleRelaySave, this));
  path = PATH_START;
  path += PATH_CONTROL;
  httpServer.on(path, std::bind(&SuplaWebServer::handleControl, this));
  path = PATH_START;
  path += PATH_SAVE_CONTROL;
  httpServer.on(path, std::bind(&SuplaWebServer::handleControlSave, this));
  path = PATH_START;
  path += PATH_SENSOR;
  httpServer.on(path, std::bind(&SuplaWebServer::handleSensor, this));
  path = PATH_START;
  path += PATH_SAVE_SENSOR;
  httpServer.on(path, std::bind(&SuplaWebServer::handleSensorSave, this));
  path = PATH_START;
  path += PATH_CONFIG;
  httpServer.on(path, std::bind(&SuplaWebServer::handleConfig, this));
  path = PATH_START;
  path += PATH_SAVE_CONFIG;
  httpServer.on(path, std::bind(&SuplaWebServer::handleConfigSave, this));
}

void SuplaWebServer::handle() {

  Serial.println(F("HTTP_GET - metoda handle"));
  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(this->www_username, this->www_password))
      return httpServer.requestAuthentication();
  }
  this->sendContent(supla_webpage_start(0));
}


void SuplaWebServer::handleSave() {
  Serial.println(F("HTTP_POST - metoda handleSave"));
  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(this->www_username, this->www_password))
      return httpServer.requestAuthentication();
  }

  ConfigManager->set(KEY_WIFI_SSID, httpServer.arg(INPUT_WIFI_SSID).c_str());
  ConfigManager->set(KEY_WIFI_PASS, httpServer.arg(INPUT_WIFI_PASS).c_str());
  ConfigManager->set(KEY_SUPLA_SERVER, httpServer.arg(INPUT_SERVER).c_str());
  ConfigManager->set(KEY_SUPLA_EMAIL, httpServer.arg(INPUT_EMAIL).c_str());
  ConfigManager->set(KEY_HOST_NAME, httpServer.arg(INPUT_HOSTNAME).c_str());
  ConfigManager->set(KEY_LOGIN, httpServer.arg(INPUT_MODUL_LOGIN).c_str());
  ConfigManager->set(KEY_LOGIN_PASS, httpServer.arg(INPUT_MODUL_PASS).c_str());
  ConfigManager->set(KEY_MONOSTABLE_TRIGGER, httpServer.arg(INPUT_TRIGGER).c_str());
  
  String button_value;
  for (int i = 0; i < Supla::GUI::button.size(); ++i) {
    String button = INPUT_BUTTON_SET;
    button += i;
    button_value += httpServer.arg(button).c_str();
    ConfigManager->set(KEY_TYPE_BUTTON, button_value.c_str());
  }

   for (int i = 0; i < ConfigManager->get(KEY_MULTI_MAX_DS18B20)->getValueInt(); i++) {
    String ds_key = KEY_DS;
    String ds_name_key = KEY_DS_NAME;
    ds_key += i;
    ds_name_key += i;

    String ds = F("dschlid");
    String ds_name = F("dsnameid");
    ds += i;
    ds_name += i;

    ConfigManager->set(ds_key.c_str(), httpServer.arg(ds).c_str());
    ConfigManager->set(ds_name_key.c_str(), httpServer.arg(ds_name).c_str());
  }

  if (strcmp(httpServer.arg("maxds").c_str(), "") != 0) {
    ConfigManager->set(KEY_MULTI_MAX_DS18B20, httpServer.arg("maxds").c_str());
  }


  switch (ConfigManager->save()) {
    case E_CONFIG_OK:
      Serial.println(F("E_CONFIG_OK: Dane zapisane"));
      this->sendContent(supla_webpage_start(5));
      this->rebootESP();
      break;

    case E_CONFIG_FILE_OPEN:
      Serial.println(F("E_CONFIG_FILE_OPEN: Couldn't open file"));
      httpServer.send(200, "text/html", supla_webpage_start(4));
      break;
  }
}

void SuplaWebServer::handleSearchDS() {
  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(www_username, www_password))
      return httpServer.requestAuthentication();
  }
  this->sendContent(supla_webpage_search(0));
}

void SuplaWebServer::handleDSSave() {
  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(www_username, www_password))
      return httpServer.requestAuthentication();
  }
  for (int i = 0; i < ConfigManager->get(KEY_MULTI_MAX_DS18B20)->getValueInt(); i++) {
    String ds_key = KEY_DS;
    ds_key += i;

    String ds = F("dschlid");
    ds += i;

    String address = httpServer.arg(ds).c_str();
    if (address != NULL) {
      ConfigManager->set(ds_key.c_str(), address.c_str());
    }
  }

  switch (ConfigManager->save()) {
    case E_CONFIG_OK:
      Serial.println(F("E_CONFIG_OK: Config save"));
      this->sendContent(supla_webpage_search(1));
//      this->rebootESP();
      break;
    case E_CONFIG_FILE_OPEN:
      Serial.println(F("E_CONFIG_FILE_OPEN: Couldn't open file"));
      httpServer.send(200, "text/html", supla_webpage_search(2));
      break;
  }
}

void SuplaWebServer::handleFirmwareUp() {
  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(www_username, www_password))
      return httpServer.requestAuthentication();
  }
  httpServer.send(200, "text/html", supla_webpage_upddate());
}

void SuplaWebServer::handleRelay() {
  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(www_username, www_password))
      return httpServer.requestAuthentication();
  }
  httpServer.send(200, "text/html", supla_webpage_relay(0));
}

void SuplaWebServer::handleRelaySave() {
  Serial.println(F("HTTP_POST - metoda handleRelaySave"));

  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(www_username, www_password))
      return httpServer.requestAuthentication();
  }
  String key, input, key_max, input_max;
  int nr, set_input,get_input, current_value, last_value;
  
  last_value = ConfigManager->get(KEY_MAX_RELAY )->getValueInt();
  if(last_value > 0){
    ConfigManager->set(KEY_RELAY_LEVEL, httpServer.arg(INPUT_RELAY_LEVEL).c_str());
  }
  if (strcmp(httpServer.arg(INPUT_MAX_RELAY).c_str(), "") != 0) {
    ConfigManager->set(KEY_MAX_RELAY, httpServer.arg(INPUT_MAX_RELAY).c_str());
  }
  current_value = ConfigManager->get(KEY_MAX_RELAY)->getValueInt();
  if(current_value >= last_value){
    for(nr = 1; nr <= last_value; nr++){
      key = KEY_RELAY;
      input = INPUT_RELAY_GPIO;
      key += nr;
      input += nr;
      set_input = httpServer.arg(input).toInt();
      get_input = ConfigManager->get(key.c_str())->getValueInt();
      if(get_input != set_input){
        if(getBusyGpio(set_input) == false){
          ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());
          setBusyGpio(get_input, false);
          setBusyGpio(set_input, true);
        }
        else {
          Serial.println(F("ERROR!!!"));
          httpServer.send(200, "text/html", supla_webpage_sensor(6));
          return;
        }
      } 
    }
  }
  else if(current_value < last_value){
    for(nr = current_value + 1; nr <= last_value; nr++){
      key = KEY_RELAY;
      key += nr;
      get_input = ConfigManager->get(key.c_str())->getValueInt();
      ConfigManager->set(key.c_str(), "17");
      setBusyGpio(get_input, false);
    }
  }

  switch (ConfigManager->save()) {
    case E_CONFIG_OK:
      Serial.println(F("E_CONFIG_OK: Config save"));
      this->sendContent(supla_webpage_relay(1));
//      this->rebootESP();
      break;
    case E_CONFIG_FILE_OPEN:
      Serial.println(F("E_CONFIG_FILE_OPEN: Couldn't open file"));
      httpServer.send(200, "text/html", supla_webpage_relay(2));
      break;  
  }
}

void SuplaWebServer::handleControl() {
  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(www_username, www_password))
      return httpServer.requestAuthentication();
  }
  httpServer.send(200, "text/html", supla_webpage_control(0));
}

void SuplaWebServer::handleControlSave() {
  Serial.println(F("HTTP_POST - metoda handleControlSave"));

  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(www_username, www_password))
      return httpServer.requestAuthentication();
  }
  String key, input, key_max, input_max;
  int nr, set_input,get_input, current_value, last_value;
  
  last_value = ConfigManager->get(KEY_MAX_BUTTON )->getValueInt();
  if (strcmp(httpServer.arg(INPUT_MAX_BUTTON).c_str(), "") != 0) {
    ConfigManager->set(KEY_MAX_BUTTON, httpServer.arg(INPUT_MAX_BUTTON).c_str());
  }
  current_value = ConfigManager->get(KEY_MAX_BUTTON)->getValueInt();
  if(current_value >= last_value){
    for(nr = 1; nr <= last_value; nr++){
      key = KEY_BUTTON;
      input = INPUT_BUTTON_GPIO;
      key += nr;
      input += nr;
      set_input = httpServer.arg(input).toInt();
      get_input = ConfigManager->get(key.c_str())->getValueInt();
      if(get_input != set_input){
        if(getBusyGpio(set_input) == false){
          ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());
          setBusyGpio(get_input, false);
          setBusyGpio(set_input, true);
        }
        else {
          Serial.println(F("ERROR!!!"));
          httpServer.send(200, "text/html", supla_webpage_sensor(6));
          return;
        }
      } 
    }
  }
  else if(current_value < last_value){
    for(nr = current_value + 1; nr <= last_value; nr++){
      key = KEY_BUTTON;
      key += nr;
      get_input = ConfigManager->get(key.c_str())->getValueInt();
      ConfigManager->set(key.c_str(), "17");
      setBusyGpio(get_input, false);
    }
  }

  last_value = ConfigManager->get(KEY_MAX_LIMIT_SWITCH)->getValueInt();
  if (strcmp(httpServer.arg(INPUT_MAX_LIMIT_SWITCH).c_str(), "") != 0) {
    ConfigManager->set(KEY_MAX_LIMIT_SWITCH, httpServer.arg(INPUT_MAX_LIMIT_SWITCH).c_str());
  }
  current_value = ConfigManager->get(KEY_MAX_LIMIT_SWITCH)->getValueInt();
  if(current_value >= last_value){
    for(nr = 1; nr <= last_value; nr++){
      key = KEY_LIMIT_SWITCH;
      input = INPUT_LIMIT_SWITCH_GPIO;
      key += nr;
      input += nr;
      set_input = httpServer.arg(input).toInt();
      get_input = ConfigManager->get(key.c_str())->getValueInt();
      if(get_input != set_input){
        if(getBusyGpio(set_input) == false){
          ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());
          setBusyGpio(get_input, false);
          setBusyGpio(set_input, true);
        }
        else {
          Serial.println(F("ERROR!!!"));
          httpServer.send(200, "text/html", supla_webpage_sensor(6));
          return;
        }
      } 
    }
  }
  else if(current_value < last_value){
    for(nr = current_value + 1; nr <= last_value; nr++){
      key = KEY_LIMIT_SWITCH;
      key += nr;
      get_input = ConfigManager->get(key.c_str())->getValueInt();
      ConfigManager->set(key.c_str(), "17");
      setBusyGpio(get_input, false);
    }
  }
  
  switch (ConfigManager->save()) {
    case E_CONFIG_OK:
      Serial.println(F("E_CONFIG_OK: Config save"));
      this->sendContent(supla_webpage_control(1));
//      this->rebootESP();
      break;
    case E_CONFIG_FILE_OPEN:
      Serial.println(F("E_CONFIG_FILE_OPEN: Couldn't open file"));
      httpServer.send(200, "text/html", supla_webpage_control(2));
      break;  
  }
}

void SuplaWebServer::handleSensor() {
  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(www_username, www_password))
      return httpServer.requestAuthentication();
  }
  httpServer.send(200, "text/html", supla_webpage_sensor(0));
}

void SuplaWebServer::handleSensorSave() {
  Serial.println(F("HTTP_POST - metoda handleSensorSave"));

  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(www_username, www_password))
      return httpServer.requestAuthentication();
  }
  String key, input, key_max, input_max;
  int nr, set_input,get_input, current_value, last_value;

  last_value = ConfigManager->get(KEY_MAX_DHT11)->getValueInt();
  if (strcmp(httpServer.arg(INPUT_MAX_DHT11).c_str(), "") != 0) {
    ConfigManager->set(KEY_MAX_DHT11, httpServer.arg(INPUT_MAX_DHT11).c_str());
  }
  current_value = ConfigManager->get(KEY_MAX_DHT11)->getValueInt();
  if(current_value >= last_value){
    for(nr = 1; nr <= last_value; nr++){
      key = KEY_DHT11;
      input = INPUT_DHT11_GPIO;
      key += nr;
      input += nr;
      set_input = httpServer.arg(input).toInt();
      get_input = ConfigManager->get(key.c_str())->getValueInt();
      if(get_input != set_input){
        if(getBusyGpio(set_input) == false){
          ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());
          setBusyGpio(get_input, false);
          setBusyGpio(set_input, true);
        }
        else {
          Serial.println(F("ERROR!!!"));
          httpServer.send(200, "text/html", supla_webpage_sensor(6));
          return;
        }
      } 
    }
  }
  else if(current_value < last_value){
    for(nr = current_value + 1; nr <= last_value; nr++){
      key = KEY_DHT11;
      key += nr;
      get_input = ConfigManager->get(key.c_str())->getValueInt();
      ConfigManager->set(key.c_str(), "17");
      setBusyGpio(get_input, false);
    }
  }
  
  last_value = ConfigManager->get(KEY_MAX_DHT22)->getValueInt();
  if (strcmp(httpServer.arg(INPUT_MAX_DHT22).c_str(), "") != 0) {
    ConfigManager->set(KEY_MAX_DHT22, httpServer.arg(INPUT_MAX_DHT22).c_str());
  }
  current_value = ConfigManager->get(KEY_MAX_DHT22)->getValueInt();
  if(current_value >= last_value){
    for(nr = 1; nr <= last_value; nr++){
      key = KEY_DHT22;
      input = INPUT_DHT22_GPIO;
      key += nr;
      input += nr;
      set_input = httpServer.arg(input).toInt();
      get_input = ConfigManager->get(key.c_str())->getValueInt();
      if(get_input != set_input){
        if(getBusyGpio(set_input) == false){
          ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());
          setBusyGpio(get_input, false);
          setBusyGpio(set_input, true);
        }
        else {
          Serial.println(F("ERROR!!!"));
          httpServer.send(200, "text/html", supla_webpage_sensor(6));
          return;
        }
      } 
    }
  }
  else if(current_value < last_value){
    for(nr = current_value + 1; nr <= last_value; nr++){
      key = KEY_DHT22;
      key += nr;
      get_input = ConfigManager->get(key.c_str())->getValueInt();
      ConfigManager->set(key.c_str(), "17");
      setBusyGpio(get_input, false);
    }
  }

  ConfigManager->set(KEY_MULTI_DS, httpServer.arg(INPUT_MULTI_DS_GPIO).c_str());
  
  last_value = ConfigManager->get(KEY_MULTI_MAX_DS18B20)->getValueInt();
  if (strcmp(httpServer.arg("maxds").c_str(), "") != 0) {
    ConfigManager->set(KEY_MULTI_MAX_DS18B20, httpServer.arg("maxds").c_str());    
  }
  if(ConfigManager->get(KEY_MULTI_MAX_DS18B20)->getValueInt() >= last_value){
    int gpio = ConfigManager->get(KEY_MULTI_DS)->getValueInt();
    if(gpio >= 0 && gpio < OFF_GPIO){
      setBusyGpio(gpio, true);
      Supla::GUI::addDS18B20MultiThermometer(gpio);  
    }
    else setBusyGpio(gpio, false);
  }

    key = KEY_SDA;
    input = INPUT_SDA_GPIO;
    set_input = httpServer.arg(input).toInt();
    get_input = ConfigManager->get(key.c_str())->getValueInt();
    if(get_input != set_input){
      if(getBusyGpio(set_input) == false){
        ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());
        setBusyGpio(get_input, false);
        setBusyGpio(set_input, true);
      }
      else {
        Serial.println(F("ERROR!!!"));
        httpServer.send(200, "text/html", supla_webpage_sensor(6));
        return;
      }
    } 
    
    key = KEY_SCL;
    input = INPUT_SCL_GPIO;
    set_input = httpServer.arg(input).toInt();
    get_input = ConfigManager->get(key.c_str())->getValueInt();
    if(get_input != set_input){
      if(getBusyGpio(set_input) == false){
        ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());
        setBusyGpio(get_input, false);
        setBusyGpio(set_input, true);
      }
      else {
        Serial.println(F("ERROR!!!"));
        httpServer.send(200, "text/html", supla_webpage_sensor(6));
        return;
      }
    } 
    
    key = KEY_BME280;
    input = INPUT_BME280;
    ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());

    key = KEY_ALTITUDE_BME280;
    input = INPUT_ALTITUDE_BME280;
    ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());

    key = KEY_TRIG;
    input = INPUT_TRIG_GPIO;
    set_input = httpServer.arg(input).toInt();
    get_input = ConfigManager->get(key.c_str())->getValueInt();
    if(get_input != set_input){
      if(getBusyGpio(set_input) == false){
        ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());
        setBusyGpio(get_input, false);
        setBusyGpio(set_input, true);
      }
      else {
        Serial.println(F("ERROR!!!"));
        httpServer.send(200, "text/html", supla_webpage_sensor(6));
        return;
      }
    } 

    key = KEY_ECHO;
    input = INPUT_ECHO_GPIO;
    set_input = httpServer.arg(input).toInt();
    get_input = ConfigManager->get(key.c_str())->getValueInt();
    if(get_input != set_input){
      if(getBusyGpio(set_input) == false){
        ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());
        setBusyGpio(get_input, false);
        setBusyGpio(set_input, true);
      }
      else {
        Serial.println(F("ERROR!!!"));
        httpServer.send(200, "text/html", supla_webpage_sensor(6));
        return;
      }
    } 

  switch (ConfigManager->save()) {
    case E_CONFIG_OK:
      Serial.println(F("E_CONFIG_OK: Config save"));
      this->sendContent(supla_webpage_sensor(1));
      break;
    case E_CONFIG_FILE_OPEN:
      Serial.println(F("E_CONFIG_FILE_OPEN: Couldn't open file"));
      httpServer.send(200, "text/html", supla_webpage_sensor(2));
      break;  
  }
}

void SuplaWebServer::handleConfig() {
  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(www_username, www_password))
      return httpServer.requestAuthentication();
  }
  httpServer.send(200, "text/html", supla_webpage_config(0));
}

void SuplaWebServer::handleConfigSave() {
  Serial.println(F("HTTP_POST - metoda handleConfigSave"));

  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(www_username, www_password))
      return httpServer.requestAuthentication();
  }

  ConfigManager->set(KEY_CFG_LED_LEVEL, httpServer.arg(INPUT_CFG_LED_LEVEL).c_str());
  
  String key = KEY_CFG_LED;
  String input = INPUT_CFG_LED_GPIO;
  int set_input = httpServer.arg(input).toInt();
  int get_input = ConfigManager->get(key.c_str())->getValueInt();
  if(get_input != set_input){
    if(getBusyGpio(set_input) == false){
      ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());
      setBusyGpio(get_input, false);
      setBusyGpio(set_input, true);
    }
    else {
      Serial.println(F("ERROR!!!"));
      httpServer.send(200, "text/html", supla_webpage_config(6));
      return;
    }
  }
  key = KEY_CFG_BTN;
  input = INPUT_CFG_BTN_GPIO;
  set_input = httpServer.arg(input).toInt();
  get_input = ConfigManager->get(key.c_str())->getValueInt();
  if(get_input != set_input){
    if(getBusyGpio(set_input) == false){
      ConfigManager->set(key.c_str(), httpServer.arg(input).c_str());
      setBusyGpio(get_input, false);
      setBusyGpio(set_input, true);
    }
    else {
      Serial.println(F("ERROR!!!"));
      httpServer.send(200, "text/html", supla_webpage_config(6));
      return;
    }
  }
  switch (ConfigManager->save()) {
    case E_CONFIG_OK:
      Serial.println(F("E_CONFIG_OK: Config save"));
      this->sendContent(supla_webpage_config(1));
//      this->rebootESP();
      break;
    case E_CONFIG_FILE_OPEN:
      Serial.println(F("E_CONFIG_FILE_OPEN: Couldn't open file"));
      httpServer.send(200, "text/html", supla_webpage_config(2));
      break;  
  }
}
 
String SuplaWebServer::supla_webpage_start(int save) {
  String content = F("");

  content += SuplaMetas();
  content += SuplaStyle();
  content += SuplaFavicon();
  content += SuplaSaveResult(save);
  content += SuplaJavaScript();
  content += F("<div class='s'>");
  content += SuplaLogo();
  content += SuplaSummary();
  content += F("<form method='post' action='");
  content += PATH_SET;
  content += F("'>");
  content += F("<div class='w'>");
  content += F("<h3>Ustawienia WIFI</h3>");
  content += F("<i><input name='");
  content += INPUT_WIFI_SSID;
  content += F("' value='");
  content += String(ConfigManager->get(KEY_WIFI_SSID)->getValue());
  content += F("' length=");
  content += MAX_SSID;
  content += F("><label>Nazwa sieci</label></i>");
  content += F("<i><input name='");
  content += INPUT_WIFI_PASS;
  if (ConfigESP->configModeESP != NORMAL_MODE) {
    content += F("' type='password' ");
  }
  content += F("' value='");
  content += String(ConfigManager->get(KEY_WIFI_PASS)->getValue());
  content +=  F("'");

  if (ConfigManager->get(KEY_WIFI_PASS)->getValue() != 0) {
    content += F(">");
  }
  else {
    content += F("'minlength='");
    content += MIN_PASSWORD;
    content += F("' required length=");
    content += MAX_PASSWORD;
    content += F(">");
  }
  content += F("<label>Hasło</label></i>");
  content += F("<i><input name='");
  content += INPUT_HOSTNAME;
  content += F("' value='");
  content += ConfigManager->get(KEY_HOST_NAME)->getValue();
  content += F("' length=");
  content += MAX_HOSTNAME;
  content += F("><label>Nazwa modułu</label></i>");
  content += F("</div>");

  content += F("<div class='w'>");
  content += F("<h3>Ustawienia SUPLA</h3>");
  content += F("<i><input name='");
  content += INPUT_SERVER;
  content += F("' length=");
  content += MAX_SUPLA_SERVER;
  String def = DEFAULT_SERVER;
  String def_2 = String(ConfigManager->get(KEY_SUPLA_SERVER)->getValue());
  if (def == def_2){
    content += F(" placeholder='");
  }
  else{
    content += F(" value='");
  }
  content += def_2;
  content += F("'><label>Adres serwera</label></i>");
  content += F("<i><input name='");
  content += INPUT_EMAIL;
  content += F("' length=");
  content += MAX_EMAIL;
  def = DEFAULT_EMAIL;
  def_2 = String(ConfigManager->get(KEY_SUPLA_EMAIL)->getValue());
  if (def == def_2){
    content += F(" placeholder='");
  }
  else{
    content += F(" value='");
  }
  content += def_2;
  content += F("'><label>Email</label></i>");
  content += F("</div>");

  content += F("<div class='w'>");
  content += F("<h3>Ustawienia administratora</h3>");
  content += F("<i><input name='");
  content += INPUT_MODUL_LOGIN;
  content += F("' value='");
  content += String(ConfigManager->get(KEY_LOGIN)->getValue());
  content += F("' length=");
  content += MAX_MLOGIN;
  content += F("><label>Login</label></i>");
  content += F("<i><input name='");
  content += INPUT_MODUL_PASS;
  if (ConfigESP->configModeESP != NORMAL_MODE) {
    content += F("' type='password' ");
  }
  content += F("' value='");
  content += String(ConfigManager->get(KEY_LOGIN_PASS)->getValue());
  content += F("'");
  content += F("'minlength='");
  content += MIN_PASSWORD;
  content += F("' required length=");
  content += MAX_MPASSWORD;
  content += F(">");
  content += F("<label>Hasło</label></i>");
  content += F("</div>");
  //****************************************************************************
  content += F("<div class='w'>");
  content += F("<h3>Ustawienia GPIO</a></h3>");
  content += F("<i>");
  content += F("<button><a href='");
  content += PATH_START;
  content += PATH_RELAY;
  content += F("'>PRZEKAŹNIKI</a></button>");
  content += F("<label>Ustaw</label></i>");
  content += F("<i>");
  content += F("<button><a href='");
  content += PATH_START;
  content += PATH_CONTROL;
  content += F("'>PRZYCISKI</a></button>");
  content += F("<label>Ustaw</label></i>");
  content += F("<i>");
  content += F("<button><a href='");
  content += PATH_START;
  content += PATH_SENSOR;
  content += F("'>SENSORY</a></button>");
  content += F("<label>Ustaw</label></i>");
  content += F("<i>");
  content += F("<button><a href='");
  content += PATH_START;
  content += PATH_CONFIG;
  content += F("'>LED, BUTTON CONFIG</a></button>");
  content += F("<label>Ustaw</label></i>");
  content += F("</div>");
  //DS****************************************************************************
//  if (!Supla::GUI::sensorDS.empty() || ConfigManager->get(KEY_MULTI_DS)->getValueInt() < OFF_GPIO) {
  if (!Supla::GUI::sensorDS.empty()) {
    content += F("<div class='w'>");
    content += F("<h3>Temperatura</h3>");
    for (int i = 0; i < ConfigManager->get(KEY_MULTI_MAX_DS18B20)->getValueInt(); i++) {
      String ds_key = KEY_DS;
      String ds_name_key = KEY_DS_NAME;
      ds_key += i;
      ds_name_key += i;

      double temp = Supla::GUI::sensorDS[i]->getValue();
      content += F("<i><input name='dsnameid");
      content += i;
      content += F("' value='");
      content += String(ConfigManager->get(ds_name_key.c_str())->getValue());
      content += F("' maxlength=");
      content += MAX_DS18B20_NAME;
      content += F("><label>");
      content += F("Nazwa ");
      content += i + 1;
      content += F("</label></i>");
      content += F("<i><input name='dschlid");
      content += i;
      content += F("' value='");
      content += String(ConfigManager->get(ds_key.c_str())->getValue());
      content += F("' maxlength=");
      content += MAX_DS18B20_ADDRESS_HEX;
      content += F("><label>");
      if (temp != -275)content += temp;
      else content += F("--.--");
      content += F(" <b>&deg;C</b> ");
      content += F("</label></i>");
    }
    content += F("</div>");
  }
  //*****************************************************************************************
if (!Supla::GUI::button.empty()) {
    content += F("<div class='w'>");
    content += F("<h3>Ustawienia Przycisków</h3>");  
//    //button***************************************************************************************
    
      for (int i = 0; i < Supla::GUI::button.size(); i++) {
        int select_button = ConfigManager->get(KEY_TYPE_BUTTON)->getValueElement(i);
        content += F("<i><label>");
        content += i + 1;
        content += F(". Przycisk typ</label><select name='");
        content += INPUT_BUTTON_SET;
        content += i;
        content += F("'>");
  
        for (int suported_button = 0; suported_button < sizeof(Supported_Button) / sizeof(char*); suported_button++) {
          content += F("<option value='");
          content += suported_button;
          if (select_button == suported_button) {
            content += F("' selected>");
          }
          else content += F("'>");
          content += (Supported_Button[suported_button]);
        }
        content += F("</select></i>");
      }
  
      //monostable triger
      content += F("<i><label>");
      content += F("Reakcja na");
      content += F("</label><select name='");
      content += INPUT_TRIGGER;
      content += F("'>");
      int select_trigger = ConfigManager->get(KEY_MONOSTABLE_TRIGGER)->getValueInt();
      for (int suported_trigger = 0; suported_trigger < sizeof(Supported_MonostableTrigger) / sizeof(char*); suported_trigger++) {
        content += F("<option value='");
        content += suported_trigger;
        if (select_trigger == suported_trigger) {
          content += F("' selected>");
        }
        else content += F("'>");
        content += (Supported_MonostableTrigger[suported_trigger]);
      }
      content += F("</select></i>");
    content += F("</div>");
  }
//  *****************************************************************************************

  content += F("<button type='submit'>Zapisz</button></form>");
  content += F("<br>");
  content += F("<a href='");
  content += PATH_START;
  content += PATH_UPDATE;
  content += F("'><button>Aktualizacja</button></a>");
  content += F("<br><br>");
  content += F("<form method='post' action='");
  content += PATH_REBOT;
  content += F("'>");
  content += F("<button type='submit'>Restart</button></form></div>");
  content += SuplaCopyrightBar();
  return content;
}

String SuplaWebServer::supla_webpage_search(int save) {
  String content = "";
  uint8_t count = 0;
  int pin = Supla::GUI::sensorDS[0]->getPin();

  OneWire ow(pin);
  DallasTemperature sensors;
  DeviceAddress address;
  char strAddr[64];

  content += SuplaMetas();
  content += SuplaStyle();
  content += SuplaFavicon();
  content += SuplaSaveResult(save);
  content += SuplaJavaScript();
  content += F("<div class='s'>");
  content += SuplaLogo();
  content += SuplaSummary();
  content += F("<center>");
  content += F("<div class='w'>");
  content += F("<h3>Temperatura</h3>");
//  for (int i = 0; i < Supla::GUI::sensorDS.size(); i++) {
  for (int i = 0; i < ConfigManager->get(KEY_MULTI_MAX_DS18B20)->getValueInt(); i++) {
    double temp = Supla::GUI::sensorDS[i]->getValue();
    String ds_key = KEY_DS;
    ds_key += i;

    content += F("<i><input name='ds18b20_");
    content += i;
    content += F("' value='");
    content += String(ConfigManager->get(ds_key.c_str())->getValue());
    content += F("' maxlength=");
    content += MAX_DS18B20_ADDRESS_HEX;
    content += F(" readonly><label>");
    if (temp != -275)content += temp;
    else content += F("--.--");
    content += F(" <b>&deg;C</b> ");
    content += F("</label>");
    content += F("<label style='left:80px'>GPIO: ");
    content += pin;
    content += F("</label></i>");
  }
  content += F("</div>");

  content += F("<form method='post' action='");
  content += PATH_SAVE_DS;
  content += F("'>");
  content += F("<div class='w'>");
  content += F("<h3>Znalezione DS18b20</h3>");
  sensors.setOneWire(&ow);
  sensors.begin();
  if (sensors.isParasitePowerMode()) {
    supla_log(LOG_DEBUG, "OneWire(pin %d) Parasite power is ON", pin);
  } else {
    supla_log(LOG_DEBUG, "OneWire(pin %d) Parasite power is OFF", pin);
  }
  // report parasite power requirements
  for (int i = 0; i < sensors.getDeviceCount(); i++) {
    if (!sensors.getAddress(address, i)) {
      supla_log(LOG_DEBUG, "Unable to find address for Device %d", i);
    } else {
      sprintf(
        strAddr,
        "%02X%02X%02X%02X%02X%02X%02X%02X",
        address[0],
        address[1],
        address[2],
        address[3],
        address[4],
        address[5],
        address[6],
        address[7]);
      supla_log(LOG_DEBUG, "Index %d - address %s", i, strAddr);

      content += F("<i><input name='dschlid");
      content += count;

      content += F("' value='");
      content += String(strAddr);
      content += F("' maxlength=");
      content += MAX_DS18B20_ADDRESS_HEX;
      content += F(" readonly><label></i>");

      count++;
    }
    delay(0);
  }

  if (count == 0)
    content += F("<i><label>brak podłączonych czujników</label></i>");

  content += F("</div>");
  content += F("</center>");
  content += F("<button type='submit'>Zapisz znalezione DSy</button></form>");
  content += F("<br><br>");
  content += F("<a href='/'><button>Powrót</button></a></div>");
  content += SuplaCopyrightBar();

  return content;
}

String SuplaWebServer::supla_webpage_upddate() {
  String content = "";

  content += SuplaMetas();
  content += SuplaStyle();
  content += F("<div class='s'>");
  content += SuplaLogo();
  content += SuplaSummary();
  content += F("<div class='w'>");
  content += F("<h3>Aktualizacja oprogramowania</h3>");
  content += F("<br>");
  content += F("<center>");
  content += F("<iframe src=");
  content += UPDATE_PATH;
  content += F(">Twoja przeglądarka nie akceptuje ramek! width='200' height='100' frameborder='100'></iframe>");
  content += F("</center>");
  content += F("</div>");
  content += F("<a href='/'><button>Powrót</button></a></div>");
  content += SuplaCopyrightBar();

  return content;
}

String SuplaWebServer::supla_webpage_relay(int save) {
  String key;
  int selected, suported, nr;
  
  String pagerelay = "";
  pagerelay += SuplaMetas();
  pagerelay += SuplaStyle();
  pagerelay += SuplaSaveResult(save);
  pagerelay += F("</div>");
  pagerelay += SuplaJavaScript(PATH_RELAY);
  pagerelay += F("<div class='s'>");
  pagerelay += SuplaLogo();
  pagerelay += SuplaSummary();
  pagerelay += F("<form method='post' action='");
  pagerelay += PATH_SAVE_RELAY;
  pagerelay += F("'><div class='w'><h3>Ustawienie GPIO dla przekaźników</h3>");
  pagerelay += F("<i><label>ILOŚĆ</label><input name='");
  pagerelay += INPUT_MAX_RELAY;
  pagerelay += F("' type='number' placeholder='0' step='1' min='0' max='");
  pagerelay += MAX_GPIO - ConfigManager->get(KEY_MAX_BUTTON)->getValueInt() - ConfigManager->get(KEY_MAX_LIMIT_SWITCH)->getValueInt() - ConfigManager->get(KEY_MAX_DHT11)->getValueInt() - ConfigManager->get(KEY_MAX_DHT22)->getValueInt();
  pagerelay += F("' value='");
  pagerelay += String(ConfigManager->get(KEY_MAX_RELAY)->getValue());
  pagerelay += F("'></i>");
  if(ConfigManager->get(KEY_MAX_RELAY)->getValueInt() > 0){
    pagerelay += F("<i><label>");
    pagerelay += F("Sterowanie stanem</label><select name='");
    pagerelay += INPUT_RELAY_LEVEL;
    pagerelay += F("'>");
    key = KEY_RELAY_LEVEL;
    selected = ConfigManager->get(key.c_str())->getValueInt();
    for (suported = 0; suported < sizeof(Supported_Level) / sizeof(char*); suported++) {
        pagerelay += F("<option value='");
        pagerelay += suported;
        if (selected == suported) {
          pagerelay += F("' selected>");
        }
        else pagerelay += F("'>");
        pagerelay += (Supported_Level[suported]);
    }
    pagerelay += F("</select></i>");
  }
  for(nr = 1; nr <= ConfigManager->get(KEY_MAX_RELAY)->getValueInt(); nr++){
    pagerelay += F("<i><label>");
    pagerelay += nr;
    pagerelay += F(". PRZEKAŹNIK</label><select name='");
    pagerelay += INPUT_RELAY_GPIO;
    pagerelay += nr;
    pagerelay += F("'>");
    key = KEY_RELAY;
    key += nr;
    selected = ConfigManager->get(key.c_str())->getValueInt();
    for (suported = 0; suported < sizeof(Supported_Gpio) / sizeof(char*); suported++) {
      if(getBusyGpio(suported) == false || selected == suported){
          pagerelay += F("<option value='");
          pagerelay += suported;
          if (selected == suported) {
            pagerelay += F("' selected>");
          }
          else pagerelay += F("'>");
          pagerelay += (Supported_Gpio[suported][0]);
      }
    }
    pagerelay += F("</select></i>");    
  }
  pagerelay += F("</div><button type='submit'>Zapisz</button></form>");
  pagerelay += F("<br><br>");
  pagerelay += F("<a href='/'><button>Powrót</button></a></div>");
  return pagerelay;
}

String SuplaWebServer::supla_webpage_control(int save) {
  int nr, suported, selected;
  String pagebutton,key;
  
  pagebutton += SuplaMetas();
  pagebutton += SuplaStyle();
  pagebutton += SuplaSaveResult(save);
  pagebutton += F("</div>");
  pagebutton += SuplaJavaScript(PATH_CONTROL);
  pagebutton += F("<div class='s'>");
  pagebutton += SuplaLogo();
  pagebutton += SuplaSummary();
  pagebutton += F("<form method='post' action='");
  pagebutton += PATH_SAVE_CONTROL;
  pagebutton += F("'><div class='w'><h3>Ustawienie GPIO dla przycisków</h3>");
  pagebutton += F("<i><label>ILOŚĆ</label><input name='");
  pagebutton += INPUT_MAX_BUTTON;
  pagebutton += F("' type='number' placeholder='0' step='1' min='0' max='");
  pagebutton += MAX_GPIO - ConfigManager->get(KEY_MAX_RELAY)->getValueInt() - ConfigManager->get(KEY_MAX_LIMIT_SWITCH)->getValueInt() - ConfigManager->get(KEY_MAX_DHT11)->getValueInt() - ConfigManager->get(KEY_MAX_DHT22)->getValueInt();
  pagebutton += F("' value='");
  pagebutton += String(ConfigManager->get(KEY_MAX_BUTTON)->getValue());
  pagebutton += F("'></i>");
  for(nr = 1; nr <= ConfigManager->get(KEY_MAX_BUTTON)->getValueInt(); nr++){
    pagebutton += F("<i><label>");
    pagebutton += nr;
    pagebutton += F(". PRZYCISK</label><select name='");
    pagebutton += INPUT_BUTTON_GPIO;
    pagebutton += nr;
    pagebutton += F("'>");
    key = KEY_BUTTON;
    key += nr;
    selected = ConfigManager->get(key.c_str())->getValueInt();
    for (suported = 0; suported < sizeof(Supported_Gpio) / sizeof(char*); suported++) {
      if(Supported_Gpio[suported][1] == "0" || selected == suported){
        pagebutton += F("<option value='");
        pagebutton += suported;
        if (selected == suported) {
          pagebutton += F("' selected>");
        }
        else pagebutton += F("'>");
        pagebutton += (Supported_Gpio[suported][0]);
      }
    }
    pagebutton += F("</select></i>");    
  }
  pagebutton += F("</div>");
  pagebutton += F("<div class='w'><h3>Ustawienie GPIO dla cz. otwarcia</h3>");
  pagebutton += F("<i><label>ILOŚĆ</label><input name='");
  pagebutton += INPUT_MAX_LIMIT_SWITCH;
  pagebutton += F("' type='number' placeholder='0' step='1' min='0' max='");
  pagebutton += MAX_GPIO - ConfigManager->get(KEY_MAX_RELAY)->getValueInt()- ConfigManager->get(KEY_MAX_BUTTON)->getValueInt() - ConfigManager->get(KEY_MAX_DHT11)->getValueInt() - ConfigManager->get(KEY_MAX_DHT22)->getValueInt();
  pagebutton += F("' value='");
  pagebutton += String(ConfigManager->get(KEY_MAX_LIMIT_SWITCH)->getValue());
  pagebutton += F("'></i>");
  for(nr = 1; nr <= ConfigManager->get(KEY_MAX_LIMIT_SWITCH)->getValueInt(); nr++){
    pagebutton += F("<i><label>");
    pagebutton += nr;
    pagebutton += F(". KRAŃCÓWKA</label><select name='");
    pagebutton += INPUT_LIMIT_SWITCH_GPIO;
    pagebutton += nr;
    pagebutton += F("'>");
    key = KEY_LIMIT_SWITCH;
    key += nr;
    selected = ConfigManager->get(key.c_str())->getValueInt();
    for (suported = 0; suported < sizeof(Supported_Gpio) / sizeof(char*); suported++) {
      if(Supported_Gpio[suported][1] == "0" || selected == suported){
        pagebutton += F("<option value='");
        pagebutton += suported;
        if (selected == suported) {
          pagebutton += F("' selected>");
        }
        else pagebutton += F("'>");
        pagebutton += (Supported_Gpio[suported][0]);
      }
    }
    pagebutton += F("</select></i>");    
  }
  pagebutton += F("</div><button type='submit'>Zapisz</button></form>");
  pagebutton += F("<br><br>");
  pagebutton += F("<a href='/'><button>Powrót</button></a></div>");
  return pagebutton;
}

String SuplaWebServer::supla_webpage_sensor(int save) {
  int nr, suported, selected;
  String page,key;
  
  page += SuplaMetas();
  page += SuplaStyle();
  page += SuplaSaveResult(save);
  page += F("</div>");
  page += SuplaJavaScript(PATH_SENSOR);
  page += F("<div class='s'>");
  page += SuplaLogo();
  page += SuplaSummary();
  page += F("<form method='post' action='");
  page += PATH_SAVE_SENSOR;
  page += F("'><div class='w'><h3>Ustawienie GPIO dla DHT11</h3>");
  page += F("<i><label>ILOŚĆ</label><input name='");
  page += INPUT_MAX_DHT11;
  page += F("' type='number' placeholder='0' step='1' min='0' max='");
  page += MAX_GPIO - ConfigManager->get(KEY_MAX_RELAY)->getValueInt()- ConfigManager->get(KEY_MAX_BUTTON)->getValueInt()- ConfigManager->get(KEY_MAX_DHT22)->getValueInt();
  page += F("' value='");
  page += String(ConfigManager->get(KEY_MAX_DHT11)->getValue());
  page += F("'></i>");
  for(nr = 1; nr <= ConfigManager->get(KEY_MAX_DHT11)->getValueInt(); nr++){
    page += F("<i><label>");
    page += nr;
    page += F(". DHT11</label><select name='");
    page += INPUT_DHT11_GPIO;
    page += nr;
    page += F("'>");
    key = KEY_DHT11;
    key += nr;
    selected = ConfigManager->get(key.c_str())->getValueInt();
    for (suported = 0; suported < sizeof(Supported_Gpio) / sizeof(char*); suported++) {
      if(getBusyGpio(suported) == false || selected == suported){
        page += F("<option value='");
        page += suported;
        if (selected == suported) {
          page += F("' selected>");
        }
        else page += F("'>");
        page += (Supported_Gpio[suported][0]);
      }
    }
    page += F("</select></i>");    
  }
  page += F("</div>");
  page += F("<div class='w'><h3>Ustawienie GPIO dla DHT22</h3>");
  page += F("<i><label>ILOŚĆ</label><input name='");
  page += INPUT_MAX_DHT22;
  page += F("' type='number' placeholder='0' step='1' min='0' max='");
  page += MAX_GPIO - ConfigManager->get(KEY_MAX_RELAY)->getValueInt()- ConfigManager->get(KEY_MAX_BUTTON)->getValueInt() - ConfigManager->get(KEY_MAX_DHT11)->getValueInt();
  page += F("' value='");
  page += String(ConfigManager->get(KEY_MAX_DHT22)->getValue());
  page += F("'></i>");
  for(nr = 1; nr <= ConfigManager->get(KEY_MAX_DHT22)->getValueInt(); nr++){
    page += F("<i><label>");
    page += nr;
    page += F(". DHT22</label><select name='");
    page += INPUT_DHT22_GPIO;
    page += nr;
    page += F("'>");
    key = KEY_DHT22;
    key += nr;
    selected = ConfigManager->get(key.c_str())->getValueInt();
    for (suported = 0; suported < sizeof(Supported_Gpio) / sizeof(char*); suported++) {
      if(getBusyGpio(suported) == false || selected == suported){
        page += F("<option value='");
        page += suported;
        if (selected == suported) {
          page += F("' selected>");
        }
        else page += F("'>");
        page += (Supported_Gpio[suported][0]);
      }
    }
    page += F("</select></i>");    
  }
  page += F("</div>");

  page += F("<div class='w'><h3>Ustawienie GPIO dla Multi DS18B20</h3>");
  page += F("<i><label>");
  page += F("MULTI DS18B20</label><select name='");
  page += INPUT_MULTI_DS_GPIO;
  page += F("'>");
  key = KEY_MULTI_DS;
  selected = ConfigManager->get(key.c_str())->getValueInt();
  for (suported = 0; suported < sizeof(Supported_Gpio) / sizeof(char*); suported++) {
    if(getBusyGpio(suported) == false || selected == suported){
      page += F("<option value='");
      page += suported;
      if (selected == suported) {
        page += F("' selected>");
      }
      else page += F("'>");
      page += (Supported_Gpio[suported][0]);
    }
  }
  page += F("</select></i>");
  if (ConfigManager->get(KEY_MULTI_DS)->getValueInt() < OFF_GPIO) {
    page += F("<i><label>ILOŚĆ</label><input name='maxds' type='number' placeholder='1' step='1' min='1' max='16' value='");
    page += String(ConfigManager->get(KEY_MULTI_MAX_DS18B20)->getValue());
    page += F("'></i>");
  }
  page += F("</div>");

  page += F("<div class='w'><h3>Ustawienie GPIO dla i2c</h3>");
  page += F("<i><label>");
  page += F("SDA</label><select name='");
  page += INPUT_SDA_GPIO;
  page += F("'>");
  key = KEY_SDA;
  selected = ConfigManager->get(key.c_str())->getValueInt();
  for (suported = 0; suported < sizeof(Supported_Gpio) / sizeof(char*); suported++) {
    if(getBusyGpio(suported) == false || selected == suported){
      page += F("<option value='");
      page += suported;
      if (selected == suported) {
        page += F("' selected>");
      }
      else page += F("'>");
      page += (Supported_Gpio[suported][0]);
    }
  }
  page += F("</select></i>");
  page += F("<i><label>");
  page += F("SCL</label><select name='");
  page += INPUT_SCL_GPIO;
  page += F("'>");
  key = KEY_SCL;
  selected = ConfigManager->get(key.c_str())->getValueInt();
  for (suported = 0; suported < sizeof(Supported_Gpio) / sizeof(char*); suported++) {
    if(getBusyGpio(suported) == false || selected == suported){
      page += F("<option value='");
      page += suported;
      if (selected == suported) {
        page += F("' selected>");
      }
      else page += F("'>");
      page += (Supported_Gpio[suported][0]);
    }
  }
  page += F("</select></i>");
  if(ConfigManager->get(KEY_SDA)->getValueInt() != OFF_GPIO && ConfigManager->get(KEY_SCL)->getValueInt() != OFF_GPIO){
    page += F("<i><label>");
    page += F("BME280 adres</label><select name='");
    page += INPUT_BME280;
    page += F("'>");
    key = KEY_BME280;
    selected = ConfigManager->get(key.c_str())->getValueInt();
    for (suported = 0; suported < sizeof(Supported_Bme280) / sizeof(char*); suported++) {
        page += F("<option value='");
        page += suported;
        if (selected == suported) {
          page += F("' selected>");
        }
        else page += F("'>");
        page += (Supported_Bme280[suported]);
    }
    page += F("</select></i>");
    if(selected){
        page += F("<i><input name='");
        page += INPUT_ALTITUDE_BME280;
        page += F("' value='");
        page += ConfigManager->get(KEY_ALTITUDE_BME280)->getValue();
        page += F("' ");
        page += F("><label>Wysokość m n.p.m.</label></i>");    
      }
    }
  page += F("</div>");

  
    page += F("<div class='w'><h3>Ustawienie GPIO dla HC-SR04</h3>");
  page += F("<i><label>");
  page += F("TRIG</label><select name='");
  page += INPUT_TRIG_GPIO;
  page += F("'>");
  key = KEY_TRIG;
  selected = ConfigManager->get(key.c_str())->getValueInt();
  for (suported = 0; suported < sizeof(Supported_Gpio) / sizeof(char*); suported++) {
    if(getBusyGpio(suported) == false || selected == suported){
      page += F("<option value='");
      page += suported;
      if (selected == suported) {
        page += F("' selected>");
      }
      else page += F("'>");
      page += (Supported_Gpio[suported][0]);
    }
  }
  page += F("</select></i>");
  page += F("<i><label>");
  page += F("ECHO</label><select name='");
  page += INPUT_ECHO_GPIO;
  page += F("'>");
  key = KEY_ECHO;
  selected = ConfigManager->get(key.c_str())->getValueInt();
  for (suported = 0; suported < sizeof(Supported_Gpio) / sizeof(char*); suported++) {
    if(getBusyGpio(suported) == false || selected == suported){
      page += F("<option value='");
      page += suported;
      if (selected == suported) {
        page += F("' selected>");
      }
      else page += F("'>");
      page += (Supported_Gpio[suported][0]);
    }
  }
  page += F("</select></i>");
  page += F("</div>");
  
  page += F("<button type='submit'>Zapisz</button></form>");
  page += F("<br><br>");

  if (!Supla::GUI::sensorDS.empty() || ConfigManager->get(KEY_MULTI_DS)->getValueInt() < OFF_GPIO) {
    page += F("<a href='");
    page += PATH_START;
    page += PATH_SERCH;
    page += F("'><button>Szukaj DS</button></a>");
    page += F("<br><br>");
  }

  page += F("<a href='/'><button>Powrót</button></a></div>");
  return page;
}

String SuplaWebServer::supla_webpage_config(int save) {
  
  int selected, suported;
  String page = "";
  page += SuplaMetas();
  page += SuplaStyle();
  page += SuplaSaveResult(save);
  page += F("</div>");
  page += SuplaJavaScript(PATH_CONFIG);
  page += F("<div class='s'>");
  page += SuplaLogo();
  page += SuplaSummary();
  page += F("<form method='post' action='");
  page += PATH_SAVE_CONFIG;
  page += F("'><div class='w'><h3>Ustawienie GPIO dla CONFIG</h3>");
  page += F("<i><label>");
  page += F("LED</label><select name='");
  page += INPUT_CFG_LED_GPIO;
  page += F("'>");
  selected = ConfigManager->get(KEY_CFG_LED)->getValueInt();
  for (suported = 0; suported < sizeof(Supported_Gpio) / sizeof(char*); suported++) {
    if(getBusyGpio(suported) == false || selected == suported){
        page += F("<option value='");
        page += suported;
        if (selected == suported) {
          page += F("' selected>");
        }
        else page += F("'>");
        page += (Supported_Gpio[suported][0]);
    }
  }
  page += F("</select></i>");

  if(selected != 17){
    page += F("<i><label>");
    page += F("Sterowanie stanem</label><select name='");
    page += INPUT_CFG_LED_LEVEL;
    page += F("'>");
    selected = ConfigManager->get(KEY_CFG_LED_LEVEL)->getValueInt();
    for (suported = 0; suported < sizeof(Supported_Level) / sizeof(char*); suported++) {
        page += F("<option value='");
        page += suported;
        if (selected == suported) {
          page += F("' selected>");
        }
        else page += F("'>");
        page += (Supported_Level[suported]);
    }
    page += F("</select></i>");
  }  
  page += F("<i><label>");
  page += F("BUTTON</label><select name='");
  page += INPUT_CFG_BTN_GPIO;
  page += F("'>");
  selected = ConfigManager->get(KEY_CFG_BTN)->getValueInt();
  for (suported = 0; suported < sizeof(Supported_Gpio) / sizeof(char*); suported++) {
    if(getBusyGpio(suported) == false || selected == suported){
      page += F("<option value='");
      page += suported;
      if (selected == suported) {
        page += F("' selected>");
      }
      else page += F("'>");
      page += (Supported_Gpio[suported][0]);
    }
  }
  page += F("</select></i>");
  page += F("</div><button type='submit'>Zapisz</button></form>");
  page += F("<br><br>");
  page += F("<a href='/'><button>Powrót</button></a></div>");
  return page;
}

void SuplaWebServer::supla_webpage_reboot() {
  if (ConfigESP->configModeESP == NORMAL_MODE) {
    if (!httpServer.authenticate(www_username, www_password))
      return httpServer.requestAuthentication();
  }
  httpServer.send(200, "text/html", supla_webpage_start(2));
  this->rebootESP();
}

const String SuplaWebServer::SuplaMetas() {
  return F("<!DOCTYPE HTML><meta http-equiv='content-type' content='text/html; charset=UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no'>\n");
}

const String SuplaWebServer::SuplaStyle() {
  String gui_color = "";
  if (ConfigESP->configModeESP == NORMAL_MODE) {
    gui_color = "#005c96";
  }
  else gui_color = "#00D151";
  
  String style = F("<style>a{text-decoration: none;}body{font-size:14px;font-family:'HelveticaNeue','Helvetica Neue','HelveticaNeueRoman','HelveticaNeue-Roman','Helvetica Neue Roman','TeXGyreHerosRegular','Helvetica','Tahoma','Geneva','Arial',sans-serif;font-weight:400;font-stretch:normal;background:");
  style += gui_color;
  style += F(";color:#fff;line-height:20px;padding:0}.s{width:460px;margin:0 auto;margin-top:calc(50vh - 340px);border:solid 3px #fff;padding:0 10px 10px;border-radius:3px}#l{display:block;max-width:150px;height:155px;margin:-80px auto 20px;background:");
  style += gui_color;
  style += F(";padding-right:5px}#l path{fill:#000}.w{margin:3px 0 16px;padding:5px 0px;border-radius:3px;background:#fff;box-shadow:0 1px 3px rgba(0,0,0,.3)}h1,h3{margin:10px 8px;font-family:'HelveticaNeueLight','HelveticaNeue-Light','Helvetica Neue Light','HelveticaNeue','Helvetica Neue','TeXGyreHerosRegular','Helvetica','Tahoma','Geneva','Arial',sans-serif;font-weight:300;font-stretch:normal;color:#000;font-size:23px}h1{margin-bottom:14px;color:#fff}span{display:block;margin:10px 7px 14px}i{display:block;font-style:normal;position:relative;border-bottom:solid 1px ");
  style += gui_color;
  style += F(";height:42px}i:last-child{border:none}label{position:absolute;display:inline-block;top:0px;left:8px;color:");
  style += gui_color;
  style += F(";line-height:41px;pointer-events:none}input,select{width:calc(100% - 145px);border:none;font-size:16px;line-height:40px;border-radius:0;letter-spacing:-.5px;background:#fff;color:#000;padding-left:144px;-webkit-appearance:none;-moz-appearance:none;appearance:none;outline:none!important;height:40px}select{padding:0px;float:right;margin:1px 3px 1px 2px}button{width:100%;border:0;background:#000;padding:5px 10px;font-size:16px;line-height:40px;color:white;border-radius:3px;box-shadow:0 1px 3px rgba(0,0,0,.3);cursor:pointer}.c{background:#FFE836;position:fixed;width:100%;line-height:80px;color:#000;top:0;left:0;box-shadow:0 1px 3px rgba(0,0,0,.3);text-align:center;font-size:26px;z-index:100}@media all and (max-height: 920px){.s{margin-top:80px}}@media all and (max-width: 900px){.s{width:calc(100% - 20px);margin-top:40px;border:none;padding:0 8px;border-radius:0px}#l{max-width:80px;height:auto;margin:10px auto 20px}h1,h3{font-size:19px}i{border:none;height:auto}label{display:block;margin:4px 0 12px;color:");
  style += gui_color;
  style += F(";font-size:13px;position:relative;line-height:18px}input,select{width:calc(100% - 10px);font-size:16px;line-height:28px;padding:0px 5px;border-bottom:solid 1px ");
  style += gui_color;
  style += F("}select{width:100%;float:none;margin:0}}</style>\n");
  return style;  
}

const String SuplaWebServer::SuplaFavicon() {
//  return F("<link rel='icon' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgBAMAAACBVGfHAAAAB3RJTUUH5AUUCCQbIwTFfgAAAAlwSFlzAAALEgAACxIB0t1+/AAAAARnQU1BAACxjwv8YQUAAAAwUExURf7nNZuNIOvWMci2KWRbFJGEHnRpFy8rCdrGLSAdBgwLAod6G7inJkI8DVJLEKeYIg6cTsoAAAGUSURBVHjaY2CAAFUGNLCF4QAyl4mhmP8BB4LPcWtdAe+BEBiX9QD77Kzl24GKHCAC/OVZH5hkVyUCFQlCRJhnKjAwLVlVb8lQDOY/ZFrG8FDVQbVqbU8BWODc3BX8dbMMGJhfrUyAaOla+/dAP8jyncsbgJTKgVP/b+pOAUudegAkGMsrGZhE1EFyDGwLwNaucmZyl1TgKTdg4JvAwMBzn3txeKWrMwP7wQcMWiAtf2c9YDjUfYBJapsDw66bm4AiUesOnJty0/O9iwLDPI5EhhCD6/q3Chk4dgCleJYpAEOmfCkDB+sbsK1886YBRfgWMTBwbi896wR04YZuAyAH6OmzDCbr3RgYsj6A1HEBPXCfgWHONgaG6eUBII0LFTiA7jn+iIF/MbMTyEu3lphtAJtpvl4BTLPNWgVSySA+y28aWIDdyGtVBgNH5psshVawwHGGO+arLr7MYFoJjZr/zBPYj85a1sC4ulwAIsIdcJzh2qt1WReYBWBR48gxgd1ziQIi6hTYEsxR45pZwRU9+oWgNAB1F3c/H6bYqgAAAABJRU5ErkJggg==' type='image/x-png' />\n");
  return F("");
}

const String SuplaWebServer::SuplaLogo() {
//  return F("<a href='/'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg></a>\n");
  return F("");
}

const String SuplaWebServer::SuplaSummary() {
  String content = "";
  content += F("<h1>");
  content += String(ConfigManager->get(KEY_HOST_NAME)->getValue());
  content += F(" </h1>");
  content += F("<span>LAST STATE: ");
  content += String(ConfigESP->getLastStatusSupla());
  content += F("<br>");
  content += F("Firmware: SuplaDevice ");
  content += String(Supla::Channel::reg_dev.SoftVer);
  content += F("<br>");
  content += F("GUID: ");
  content += String(ConfigManager->get(KEY_SUPLA_GUID)->getValueHex(SUPLA_GUID_SIZE));
  content += F("<br>");
  content += F("MAC: ");
  content += String(ConfigESP->getMacAddress(true));
  content += F("</span>\n");
  return content;
}

const String SuplaWebServer::SuplaJavaScript(String java_return) {
    String java_script = F("<script type='text/javascript'>setTimeout(function(){var element=document.getElementById('msg');if( element != null){element.style.visibility='hidden';location.href='");
    java_script += java_return;
    java_script += F("';}},3200);</script>\n");
    return java_script;
}

const String SuplaWebServer::SuplaCopyrightBar() {
//  return F("<a target='_blank' rel='noopener noreferrer' href='https://forum.supla.org/viewtopic.php?f=11&t=5276'><span style='color: #ffffff !important;'>https://forum.supla.org/</span></a>\n");
  return F("");
}

const String SuplaWebServer::SuplaSaveResult(int save) {
    if (save == 0) return F("");
    String saveresult = "";
    saveresult += F("<div id=\"msg\" class=\"c\">");
    if (save == 1) {
      saveresult += F("Dane zapisane");
    } 
    else if (save == 2) {
      saveresult += F("Restart modułu");
    } 
    else if (save == 3) {
      saveresult += F("Dane wymazane - należy zrobić restart urządzenia");
    } 
    else if (save == 4) {
      saveresult += F("Błąd zapisu - nie można odczytać pliku - brak partycji FS.");
    }  
    else if (save == 5) {
      saveresult += F("Dane zapisane - restart modułu.");
    }
   else if (save == 6) {
      saveresult += F("Błąd zapisu - złe dane.");
    }
    saveresult += F("</div>");
    return saveresult;
  }
  
void SuplaWebServer::rebootESP() {
  delay(3000);
  WiFi.forceSleepBegin();
  wdt_reset();
  ESP.restart();
  while (1)wdt_reset();
}

void SuplaWebServer::sendContent(const String content) {
  httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer.send(200, "text/html", " ");
  httpServer.sendContent(content);
}

void SuplaWebServer::redirectToIndex() {
  httpServer.sendHeader("Location", "/", true);
  httpServer.send(302, "text/plain", "");
  httpServer.client().stop();
}

void SuplaWebServer::setBusyGpio(int gpio, int busy){
  if (busy == true && gpio < 17) Supported_Gpio[gpio][1] = "1";
  else Supported_Gpio[gpio][1] = "0";
}

int SuplaWebServer::getBusyGpio(int gpio){
  if (Supported_Gpio[gpio][1] == "0") return false;
  else return true;
}
  
