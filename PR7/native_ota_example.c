//////////////////////////////////////////////////////////
/// Práctica 7 ANIOT
//////////////////////////////////////////////////////////

/* OTA example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "esp_timer.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "protocol_examples_common.h"
#include "errno.h"
#include <driver/adc.h>

#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif

#define BUFFSIZE 1024
#define HASH_LEN 32 /* SHA-256 digest length */

static void ota_example_task(void *pvParameter);

static const char *TAG = "native_ota_example";
/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

#define OTA_URL_SIZE 256

// Identificador del timer
esp_timer_handle_t timer;


static void timer_callback(void* arg)
{
    // Si se supera el umbral
    if (hall_sensor_read() >= 100){
        xTaskCreate(&ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);
        
        // Detiene el timer para que no se intente actualizar varias veces a la vez
    	esp_timer_stop(timer);
    }
}

static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

static void __attribute__((noreturn)) task_fatal_error(void)
{
    ESP_LOGE(TAG, "Exiting task due to fatal error...");
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}

static void print_sha256 (const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s: %s", label, hash_print);
}

static void infinite_loop(void)
{
    int i = 0;
    ESP_LOGI(TAG, "When a new firmware is available on the server, press the reset button to download it");
    while(1) {
        ESP_LOGI(TAG, "Waiting for a new firmware ... %d", ++i);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void ota_example_task(void *pvParameter)
{
    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;

    ESP_LOGI(TAG, "Starting OTA example");

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);

    esp_http_client_config_t config = {
        .url = CONFIG_EXAMPLE_FIRMWARE_UPG_URL,
        .cert_pem = (char *)server_cert_pem_start,
        .timeout_ms = CONFIG_EXAMPLE_OTA_RECV_TIMEOUT,
        .keep_alive_enable = true,
    };

#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL_FROM_STDIN
    char url_buf[OTA_URL_SIZE];
    if (strcmp(config.url, "FROM_STDIN") == 0) {
        example_configure_stdin_stdout();
        fgets(url_buf, OTA_URL_SIZE, stdin);
        int len = strlen(url_buf);
        url_buf[len - 1] = '\0';
        config.url = url_buf;
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong firmware upgrade image url");
        abort();
    }
#endif

#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");
        task_fatal_error();
    }
    err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        task_fatal_error();
    }
    esp_http_client_fetch_headers(client);

    update_partition = esp_ota_get_next_update_partition(NULL);
    assert(update_partition != NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);

    int binary_file_length = 0;
    /*deal with all receive packet*/
    bool image_header_was_checked = false;
    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        if (data_read < 0) {
            ESP_LOGE(TAG, "Error: SSL data read error");
            http_cleanup(client);
            task_fatal_error();
        } else if (data_read > 0) {
            if (image_header_was_checked == false) {
                esp_app_desc_t new_app_info;
                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                    // check current version with downloading
                    memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                    ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);

                    esp_app_desc_t running_app_info;
                    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
                    }

                    const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                    esp_app_desc_t invalid_app_info;
                    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                        ESP_LOGI(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
                    }

                    // check current version with last invalid partition
                    if (last_invalid_app != NULL) {
                        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                            ESP_LOGW(TAG, "New version is the same as invalid version.");
                            ESP_LOGW(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                            ESP_LOGW(TAG, "The firmware has been rolled back to the previous version.");
                            http_cleanup(client);
                            infinite_loop();
                        }
                    }
#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECK
                    if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
                        ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
                        http_cleanup(client);
                        infinite_loop();
                    }
#endif

                    image_header_was_checked = true;

                    err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
                    if (err != ESP_OK) {
                        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                        http_cleanup(client);
                        esp_ota_abort(update_handle);
                        task_fatal_error();
                    }
                    ESP_LOGI(TAG, "esp_ota_begin succeeded");
                } else {
                    ESP_LOGE(TAG, "received package is not fit len");
                    http_cleanup(client);
                    esp_ota_abort(update_handle);
                    task_fatal_error();
                }
            }
            err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                http_cleanup(client);
                esp_ota_abort(update_handle);
                task_fatal_error();
            }
            binary_file_length += data_read;
            ESP_LOGD(TAG, "Written image length %d", binary_file_length);
        } else if (data_read == 0) {
           /*
            * As esp_http_client_read never returns negative error code, we rely on
            * `errno` to check for underlying transport connectivity closure if any
            */
            if (errno == ECONNRESET || errno == ENOTCONN) {
                ESP_LOGE(TAG, "Connection closed, errno = %d", errno);
                break;
            }
            if (esp_http_client_is_complete_data_received(client) == true) {
                ESP_LOGI(TAG, "Connection closed");
                break;
            }
        }
    }
    ESP_LOGI(TAG, "Total Write binary data length: %d", binary_file_length);
    if (esp_http_client_is_complete_data_received(client) != true) {
        ESP_LOGE(TAG, "Error in receiving complete file");
        http_cleanup(client);
        esp_ota_abort(update_handle);
        task_fatal_error();
    }

    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
        } else {
            ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        http_cleanup(client);
        task_fatal_error();
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        http_cleanup(client);
        task_fatal_error();
    }
    ESP_LOGI(TAG, "Prepare to restart system!");
    esp_restart();
    return ;
}

static bool diagnostic_is_ok(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type    = GPIO_INTR_DISABLE;
    io_conf.mode         = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en   = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "Diagnostics (5 sec)...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    bool diagnostic_is_ok = gpio_get_level(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);

    gpio_reset_pin(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);
    return diagnostic_is_ok;
}

void app_main(void)
{
    // Estructura que define una particion.
    esp_partition_t partition;

    // Código hash de 32 bytes/elementos.
    uint8_t sha_256[HASH_LEN] = {0};

    // Dirección donde comienza la partición en la memoria flash.
    partition.address   = ESP_PARTITION_TABLE_OFFSET; 
    // Tamaño de la partición en bytes.
    partition.size      = ESP_PARTITION_TABLE_MAX_LEN;
    // Tipo de partición: app (partición que almacenará la aplicación) / data (partición que almacena los datos, no la aplicación).
    partition.type      = ESP_PARTITION_TYPE_DATA;
    // Obtiene el codigo hash de la partición aplicando el algoritmo SHA-256 (codificación de 64 letras/números ==> 256 bits o 32 bytes).
    esp_partition_get_sha256(&partition, sha_256);
    // Muestra el código hash.
    print_sha256(sha_256, "SHA-256 for the partition table: ");

    // Dirección donde comienza la partición en la memoria flash.
    partition.address   = ESP_BOOTLOADER_OFFSET;
    // Tamaño de la partición en bytes.
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    // Tipo de partición: app (partición que almacenará la aplicación) / data (partición que almacena los datos, no la aplicación).
    partition.type      = ESP_PARTITION_TYPE_APP;
    // Obtiene el codigo hash de la partición aplicando el algoritmo SHA-256 (codificación de 64 letras/números ==> 256 bits o 32 bytes).
    esp_partition_get_sha256(&partition, sha_256);
    // Muestra el código hash.
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    // OTA (Over The Air) permite que un dispositivo se actualice a sí mismo en base a los datos que recibe a través de la tecnología Wi-Fi o Bluetooth.
    esp_ota_img_states_t ota_state;

    // Obtiene la tabla de particiones de la aplicación que se está ejecutando.
    const esp_partition_t *running = esp_ota_get_running_partition();

    // Obtiene el codigo hash de la tabla de particiones de la aplicación que se ejecuta.
    esp_partition_get_sha256(running, sha_256);
    // Muestra el código hash.
    print_sha256(sha_256, "SHA-256 for current firmware: ");

    // Analisis de la partición OTA.
    switch (esp_ota_get_state_partition(running, &ota_state))
    {
        case ESP_ERR_NOT_FOUND: printf("ESP_ERR_NOT_FOUND: La tabla de partición no tiene otadata o no se puede averiguar el estado de la particion.\n"); break;
        case ESP_ERR_INVALID_ARG: printf("ESP_ERR_INVALID_ARG: Aguno de los argumentos de la funcion era NULL.\n"); break;
        case ESP_ERR_NOT_SUPPORTED: printf("ESP_ERR_NOT_SUPPORTED: La particion no es de tipo OTA.\n"); break;
        default: // ESP_OK
            if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
                if (diagnostic_is_ok()) {
                    ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution ...");
                    esp_ota_mark_app_valid_cancel_rollback();
                } else {
                    ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version ...");
                    esp_ota_mark_app_invalid_rollback_and_reboot();
                }
            }
        break;
    }
    

    /*
    Inicializa la partición NVS por defecto.
    La partición NVS se utiliza para almacenar datos, por ejemplo, los provenientes del WiFi, los datos de calibración (PHY) 
    del dispositivo, datos de configuración personalizados, certificados en la nube, etc. NVS soporta la encriptación de datos.
    */
    esp_err_t err = nvs_flash_init();

    /*
    Comprobamos si se han producido errores.
    ESP_ERR_NVS_NO_FREE_PAGES: La partición NVS no contiene páginas vacías. Esto puede ocurrir si la partición NVS fue truncada.
    ESP_ERR_NVS_NEW_VERSION_FOUND: La partición NVS contiene datos en un nuevo formato que no es reconocido.
    */
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Si se da alguno de estos errores hay que borrar la partición NVS y volver a llamar a nvs_flash_init.
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    
    // Inicializa la pila TCP/IP.
    ESP_ERROR_CHECK(esp_netif_init());
    // El bucle de eventos permite registrar manejadores ante la ocurrencia de ciertos eventos.
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Se produce un bloqueo hasta que se establece la conexión y se obtiene la dirección IP.
    ESP_ERROR_CHECK(example_connect());
    
    // Establece el modo de ahorro de energía.
    esp_wifi_set_ps(WIFI_PS_NONE);
    
    // Habilita el ADC1 y establce la cantidad de bits para representar la señal
    adc1_config_width(ADC_WIDTH_BIT_12);

    // Configuracion del timer
    const esp_timer_create_args_t timer_args = {
        .callback = &timer_callback, // funcion callback
        .name = "hall_sensor" // el nombre es opcional, pero ayuda a depurar
    };
    
    // Crea el timer
    esp_timer_create(&timer_args, &timer);

    // Arranca el timer forever
    esp_timer_start_periodic(timer, 100000);

}
