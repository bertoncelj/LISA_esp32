#ifndef H_WEBSITE
#define H_WEBSITE

#include <ESP8266WebServer.h>
#include <Arduino.h>
//Declear server
ESP8266WebServer server(80);

/* Set these to your desired credentials. */
bool doneReadAll = false;



void handleData();
void handleRoot();
void handleManualReset();
void handleNotFound();




#endif // WEBSITE
