/*
#include "param.h"


void handleData() {
    char temp[400];

    snprintf(temp, 400,
           " %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
        lisa_U1, lisa_U2, lisa_Upov, lisa_ANG, lisa_ANG1,
        lisa_U3,lisa_U4, lisa_ANG2, lisa_ANG3,
        lisa_Vbat, lisa_temp
        );
    server.send(200, "text/html", temp);
}

void handleRoot() {
    //next from curr checkARR
    fillST(READ, WEB_REQ, READ, arrLisaKeyRX);
  if ( doneReadAll == false) {
    return;
  }
  digitalWrite(led, 1);
  char temp[1000];


snprintf(temp, 1000, 
"<html>\
<head>\
<title>Page Title</title>\
</head>\
<body>\
<h1 style=\"color:red;\">ALISA DATA</h1>\
<h3 style=\"color:blue;\">-----------  Measurements ----------</h3>\
<h3>Voltage U1: %d </h3>\
<h3>Voltage U2: %d </h3>\
<h3>Voltage Uavg: %d </h3>\
<h3>Angle <span style=\"color:red;\">ANG</span>: %d </h3>\
<h3>Angle <span style=\"color:red;\">ANG1</span>: %d </h3>\
<h3 style=\"color:blue;\">----------- Sensitive Measurements ----------</h3>\
<h3>Voltage U3: %d </h3>\
<h3>Voltage U4: %d </h3>\
<h3>Angle <span style=\"color:red;\">ANG2</span>: %d </h3>\
<h3>Angle <span style=\"color:red;\">ANG3</span>: %d </h3>\
<h3 style=\"color:blue;\">----------- General Inforamtion ----------</h3>\
<h3>Battery Voltage: %d mV</h3>\
<h3>Temperature: %d C</h3>\
</body>\
</html>\
",
    lisa_U1, lisa_U2, lisa_Upov, lisa_ANG, lisa_ANG1,
    lisa_U3,lisa_U4, lisa_ANG2, lisa_ANG3,
    lisa_Vbat, lisa_temp
);

  server.send(200, "text/html", temp);
  digitalWrite(led, 0);
  doneReadAll = false;
}

void handleManualReset(){
        server.send(200, "text/plain", "manual reset ");

        delay(5000);
        ESP.restart();  
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}
*/
