#include "storage.h"
Storage * Storage::self;

Storage *Storage::instance()
{
    if (NOT self ) {
        self = new Storage();
        self->init();
    }
    return self;
}

void Storage::erase()
{
    printf("\nErasing all the VFS partition...");
    fflush(stdout);
    
    esp_err_t err = nvs_erase_all(my_handle);
    if(err != ESP_OK) {
        printf(" error! (%04X)\n", err);
        return;
    }
    err = nvs_commit(my_handle);	
    if(err != ESP_OK) {
        printf(" error in commit! (%04X)\n", err);
        return;
    }
    printf(" done!\n");
}

void Storage::write(std::string key, std::string value)
{
    esp_err_t err = nvs_set_str(my_handle, key.c_str(), value.c_str());
    if(err != ESP_OK) {
        printf("\nError in nvs_set_str! (%04X)\n", err);
        return;
    }
    err = nvs_commit(my_handle);
    if(err != ESP_OK) {
        printf("\nError in commit! (%04X)\n", err);
        return;
    }
    printf("\nValue %s stored in NVS with key %s\n", key.c_str(), value.c_str());
}

void Storage::write_int(std::string key, int32_t v)
{
    const char* parameter1 = key.c_str();    
    int32_t value = v;
    
    esp_err_t err = nvs_set_i32(my_handle, parameter1, value);
    if(err != ESP_OK) {
        printf("\nError in nvs_set_i32! (%04X)\n", err);
        return;
    }
    err = nvs_commit(my_handle);
    if(err != ESP_OK) {
        printf("\nError in commit! (%04X)\n", err);
        return;
    }
    printf("\nValue %d stored in NVS with key %s\n", value, parameter1);
}

void Storage::write_EE(std::string key, const void *value, size_t length)
{
    const char* parameter1 = key.c_str();
//    int32_t value = v;

    esp_err_t err = nvs_set_blob(my_handle, parameter1, value, length);
    if(err != ESP_OK) {
        printf("\nError in nvs_set_blob! (%04X)\n", err);
        return;
    }
    err = nvs_commit(my_handle);
    if(err != ESP_OK) {
        printf("\nError in commit! (%04X)\n", err);
        return;
    }
    printf("\nEE values stored in NVS with key %s\n", parameter1);
}

void Storage::clear_key(std::string key)
{
    const char* parameter1 = key.c_str();
//    int32_t value = v;

    esp_err_t err = nvs_erase_key(my_handle, parameter1);
    if(err != ESP_OK) {
        printf("\nError in nvs_erase_key! (%04X)\n", err);
        return;
    }
    err = nvs_commit(my_handle);
    if(err != ESP_OK) {
        printf("\nError in commit! (%04X)\n", err);
        return;
    }
    printf("\nKey %s erased in NVS\n", parameter1);
}

std::string Storage::read(std::string key)
{
    
    if(key.empty()) {
        printf("\nNo key provided!\n");
        return "";
    }
    size_t string_size;
    esp_err_t err = nvs_get_str(my_handle, key.c_str(), NULL, &string_size);
    if(err != ESP_OK) {
        printf("\nError in nvs_get_str to get string size! (%04X)\n", err);
        return "";
    }
    char* value = (char *)malloc(string_size);
    err = nvs_get_str(my_handle, key.c_str(), value, &string_size);
    if(err != ESP_OK) {
        if(err == ESP_ERR_NVS_NOT_FOUND) printf("\nKey %s not found\n", key.c_str());
        printf("\nError in nvs_get_str to get string! (%04X)\n", err);
        return "";
    }
    printf("\nValue stored in NVS for key %s is %s\n", key.c_str(), value);
    return value;
}

int32_t Storage::read_int(std::string key)
{
    const char* parameter = key.c_str();
    
    int32_t value = 0;
    esp_err_t err = nvs_get_i32(my_handle, parameter, &value);
    if(err != ESP_OK) {
        if(err == ESP_ERR_NVS_NOT_FOUND) printf("\nKey %s not found\n", parameter);
        else printf("\nError in nvs_get_i32! (%04X)\n", err);
        return -1;
    }
    printf("\nValue stored in NVS for key %s is %d\n", parameter, value);
    return value;
}

void* Storage::read_EE(std::string key)
{
    esp_err_t err;
    const char* parameter = key.c_str();

    size_t required_size;
    err = nvs_get_blob(my_handle, parameter, NULL, &required_size);
    void* array = malloc(required_size);
    err = nvs_get_blob(my_handle, parameter, array, &required_size);
    if(err != ESP_OK) {
        if(err == ESP_ERR_NVS_NOT_FOUND) printf("\nKey %s not found\n", parameter);
        else printf("\nError in nvs_get_blob! (%04X)\n", err);
        return -1;
    }
    printf("\nGet array stored in NVS for key %s\n", parameter);
    return value;
}

void Storage::init()
{
    esp_err_t err = nvs_flash_init();
    // if it is invalid, try to erase it
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
		
		printf("Got NO_FREE_PAGES error, trying to erase the partition...\n");
		
		// find the NVS partition
        const esp_partition_t* nvs_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);      
		if(!nvs_partition) {
			
			printf("FATAL ERROR: No NVS partition found\n");
			while(1) vTaskDelay(10 / portTICK_PERIOD_MS);
		}
		
		// erase the partition
        err = (esp_partition_erase_range(nvs_partition, 0, nvs_partition->size));
		if(err != ESP_OK) {
			printf("FATAL ERROR: Unable to erase the partition\n");
			while(1) vTaskDelay(10 / portTICK_PERIOD_MS);
		}
		printf("Partition erased!\n");
		
		// now try to initialize it again
		err = nvs_flash_init();
		if(err != ESP_OK) {
			
			printf("FATAL ERROR: Unable to initialize NVS\n");
			while(1) vTaskDelay(10 / portTICK_PERIOD_MS);
		}
	}
	printf("NVS init OK!\n");
	err = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (err != ESP_OK) {
		
		printf("FATAL ERROR: Unable to open NVS\n");
		while(1) vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	printf("NVS open OK\n");
}

Storage::Storage()
{
    
}
