#pragma once

#include "../sys_init.h"
#include "../control/control.h"

void init_webserver(void);

void setupNetworkAPI(void);
void setupWebServer(void);
void setupTimeAPI(void);
void setupControlAPI(void);

void handleWebServer(void);
void handleRoot(void);
void handleFile(const char *path);

// Global variables
extern WebServer server;