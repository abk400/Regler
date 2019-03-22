#pragma once

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp32-hal-ledc.h"
#include "esp_spi_flash.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <nvs_flash.h>

#include "esp_partition.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"

#include <esp_err.h>           // ESP32 ESP-IDF
#include <esp_log.h>           // ESP32 ESP-IDF
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>

#define NOT !
#define AND &&
#define OR ||
