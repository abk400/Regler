#pragma once
#include "esp_headers.h"
class Storage
{
public:
    static Storage *instance();
    void erase();
    void write(std::string key, std::string value);
    std::string read(std::string key);
    void write_int(std::string key, int32_t v);
    int32_t read_int(std::string key);

    void write_EE(std::string key, const void *value, size_t length);
    void clear_key(std::string key);
private:
    void init();
    Storage();
    static Storage * self;    
    // NVS handler
    nvs_handle my_handle;
};
