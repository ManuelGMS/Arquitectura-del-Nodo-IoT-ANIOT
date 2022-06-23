// *****************************************************************************
// *** PRACTICA 5 ANIOT
// ***
// *** OBSERVACION: Este codigo es modular. Con los flags de la linea 23 en
// *** adelante puede configurarse el programa para responder a las distintas
// *** cuestiones planteadas en el enunciado de la practica.
// *****************************************************************************

#include <stdio.h>
#include "driver/i2c.h"
#include "esp_sleep.h"
#include "esp_bt.h" // Si da problemas, en el fichero "sdkconfig.esp32dev" poner CONFIG_BT_ENABLED=y (ya que por defecto esta en "not set").
#include "esp_bt_main.h"
#include "esp_wifi.h"
#include "esp_pm.h"
#include "esp32/pm.h"
#include "esp_log.h"

// *****************************************************************************

#define DEEP_SLEEP     0 // Habilita el modo "ligth sleep" (0) o el modo "deep sleep" (1)
#define ENABLE_PM      0 // (1) Configuracion manual del Power Manager (Requiere habilitar "Dynamic frequency scaling" y "Tickless Idle Support" en menuconfig)
#define SHOW_WUP_CAUSE 0 // (1) Consulta la causa del wakeup
#define USE_TIMER      0 // Muestreo con un bucle (0) o muestreo con un temporizador (1)

// Codigos de configuracion
//
// 0 0 0 0 -> 5.3.1.1 (ligth sleep)
// 1 0 0 0 -> 5.3.1.2 (deep sleep)
// 0 1 1 0 -> 5.3.2 (gestor pm + get_wakeup_cause)
// X X X 1 -> 5.3.3 (cambiar delay por timer)

#define WAKEUP_TIMEOUT 10000000 // Tiempo del timer de WAKEUP en microsegundos

// *****************************************************************************

#define SECONDARY_ADDR              0x40        // Dirección del dispositivo secundario (Sensor de temperatura y humedad)
#define DISABLE                     0           // Para deshabilitar algun elemento o indicar que no se hace uso de el
#define READ_BIT                    1           // Operación de lectura
#define WRITE_BIT                   0           // Operación de escritura
#define ACK_ENABLED                 1           // Habilita el uso de las señales de confirmación
#define ACK_VALUE                   0           // Valor para ACK
#define NACK_VALUE                  1           // Valor para NACK

#define I2C_MAIN_SCL_GPIO_PIN       22          // Pin GPIO por defecto para la señal de reloj
#define I2C_MAIN_SDA_GPIO_PIN       21          // Pin GPIO por defecto para la señal de datos
#define I2C_MAIN_PORT               0           // Puerto I2C del dispositivo principal
#define I2C_MAIN_FREQ_HZ            400000      // Frecuencia del reloj: 400 KHz
#define I2C_MODE_MAIN               1           // Modo: dispositivo primario

// *****************************************************************************

static const char *TAG = "PR5";

float get_temp_celsius(uint8_t *data);
static void i2c_main_init(void);
static void read_callback(void *args);
static void measure_i2c_temp();
static void read_i2c_temp();
static void enter_light_sleep();
static void enter_deep_sleep();
void show_wakeup_cause(esp_sleep_wakeup_cause_t cause);

// *****************************************************************************

// Convierte a celsius las lecturas del sensor
float get_temp_celsius(uint8_t *data)
{
    uint16_t data16 = 0;

    // Pone el dato en formato de 16 bits
    data16 = (((data[0] & 0xff) << 8) | (data[1] & 0xff));

    // Calculamos y devolvemos la temperatura.
    return ((175.72f * data16) / 65536.0f) - 46.85f;
}

static void i2c_main_init(void)
{
    // Configuracion del I2C.
    i2c_config_t conf = {
        .mode = I2C_MODE_MAIN,
        .sda_io_num = I2C_MAIN_SDA_GPIO_PIN,
        .scl_io_num = I2C_MAIN_SCL_GPIO_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MAIN_FREQ_HZ,
    };

    // Creamos la configuracion para un puerto I2C.
    i2c_param_config(I2C_MAIN_PORT, &conf);

    // Instalamos la configuracion creada.
    i2c_driver_install(I2C_MAIN_PORT, conf.mode, DISABLE, DISABLE, DISABLE);
}

static void measure_i2c_temp()
{
    // Declaramos una cola de comandos.
    i2c_cmd_handle_t cmd;

    // Creamos una cola de comandos.
    cmd = i2c_cmd_link_create();

    // Bit de START: se alerta a los dispositivos secundarios.
    i2c_master_start(cmd);
    // Dirección (7b) del secundario y operación (1b). Habilita las confimaciones.
    i2c_master_write_byte(cmd, (SECONDARY_ADDR << 1) | WRITE_BIT, ACK_ENABLED);
    // Indica al sensor que debe medir la temperatura. No hold master mode.
    i2c_master_write_byte(cmd, 0xF3, ACK_ENABLED);
    // Se libera el uso del bus I2C.
    i2c_master_stop(cmd);

    // Ejecutamos todos los comandos encolados.
    i2c_master_cmd_begin(I2C_MAIN_PORT, cmd, pdMS_TO_TICKS(100));

    // Eliminamos la cola de comandos.
    i2c_cmd_link_delete(cmd);
}

static void read_i2c_temp(uint8_t *data)
{
    // Declaramos una cola de comandos.
    i2c_cmd_handle_t cmd;

    // Creamos otra cola de comandos.
    cmd = i2c_cmd_link_create();

    // Bit de START: se alerta a los dispositivos secundarios.
    i2c_master_start(cmd);
    // Direccion (7b) del secundario y operacion (1b). Habilita las confimaciones.
    i2c_master_write_byte(cmd, (SECONDARY_ADDR << 1) | READ_BIT, ACK_ENABLED);
    // Leemos el bit mas significativo.
    i2c_master_read_byte(cmd, &data[0], ACK_VALUE);
    // Leemos el bit menos significativo.
    i2c_master_read_byte(cmd, &data[1], NACK_VALUE);
    // Se libera el uso del bus I2C.
    i2c_master_stop(cmd);

    // Ejecutamos todos los comandos encolados.
    i2c_master_cmd_begin(I2C_MAIN_PORT, cmd, pdMS_TO_TICKS(100));

    // Eliminamos la cola de comandos.
    i2c_cmd_link_delete(cmd);
}

static void enter_light_sleep()
{
    // Configurar el timer como mecanismo de wake up
    esp_sleep_enable_timer_wakeup(WAKEUP_TIMEOUT);

    // Desactivar los modulos necesarios
    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_ENABLED) esp_bluedroid_disable();
    if (esp_bt_controller_get_status() != ESP_BT_CONTROLLER_STATUS_ENABLED) esp_bt_controller_disable();
    esp_wifi_stop();

    // Entra en modo light sleep
    ESP_LOGI(TAG, "Durmiendo suave...\n");
    esp_light_sleep_start();
}

static void enter_deep_sleep()
{
    // Configurar el timer como mecanismo de wake up
    esp_sleep_enable_timer_wakeup(WAKEUP_TIMEOUT);

    // Desactivar los modulos necesarios
    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_ENABLED) esp_bluedroid_disable();
    if (esp_bt_controller_get_status() != ESP_BT_CONTROLLER_STATUS_ENABLED) esp_bt_controller_disable();
    esp_wifi_stop();

    // Entra en modo deep sleep
    ESP_LOGI(TAG, "Durmiendo profundo...\n");
    esp_deep_sleep_start();
}

void show_wakeup_cause(esp_sleep_wakeup_cause_t cause)
{
    switch(cause){
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_UNDEFINED\n");
        break;
        case ESP_SLEEP_WAKEUP_ALL:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_ALL\n");
        break;
        case ESP_SLEEP_WAKEUP_EXT0:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_EXT0\n");
        break;
        case ESP_SLEEP_WAKEUP_EXT1:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_EXT1\n");
        break;
        case ESP_SLEEP_WAKEUP_TIMER:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_TIMER\n");
        break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_TOUCHPAD\n");
        break;
        case ESP_SLEEP_WAKEUP_ULP:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_ULP\n");
        break;
        case ESP_SLEEP_WAKEUP_GPIO:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_GPIO\n");
        break;
        case ESP_SLEEP_WAKEUP_UART:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_UART\n");
        break;
        case ESP_SLEEP_WAKEUP_WIFI:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_WIFI\n");
        break;
        case ESP_SLEEP_WAKEUP_COCPU:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_COCPU\n");
        break;
        case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG\n");
        break;
        case ESP_SLEEP_WAKEUP_BT:
            ESP_LOGI(TAG, "Causa del ultimo wakeup: ESP_SLEEP_WAKEUP_BT\n");
        break;
    }
}

static void read_callback(void *args)
{
    #if USE_TIMER == 0
        // Buffer para almacenar la temperatura.
        uint8_t data[2];
        // Contador de mediciones de temperatura.
        int read_count = 0;

        while(1) {
            // Ordenamos al sensor medir la temperatura.
            measure_i2c_temp();

            // Periodo de espera para que el sensor pueda calcular la temperatura.
            vTaskDelay(pdMS_TO_TICKS(50));

            // Obtenemos la lectura realizada por el sensor.
            read_i2c_temp(data);

            // Mostramos la lectura del sensor en grados celsius.
            ESP_LOGI(TAG, "Temperatura: %.2f\n", get_temp_celsius(data));

            // Tras 5 mediciones de temperatura dormimos al sistema.
            if ((++read_count%5) == 0) {

                // Modo "ligth sleep" o modo "deep sleep".
                #if DEEP_SLEEP == 0 && ENABLE_PM == 0
                    enter_light_sleep();
                #elif ENABLE_PM == 0
                    enter_deep_sleep();
                #endif

                // Consulta la causa del wakeup.
                #if SHOW_WUP_CAUSE == 1 && DEEP_SLEEP == 0 && ENABLE_PM == 0
                    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
                    show_wakeup_cause(cause);
                #endif
            }
            // Espera correspondiente al periodo de muestreo.
            vTaskDelay(pdMS_TO_TICKS(CONFIG_SAMPLE_PERIOD-50));

            // Consulta la causa del wakeup en modo automatico (PM).
            #if SHOW_WUP_CAUSE == 1 && ENABLE_PM == 1
                esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
                show_wakeup_cause(cause);
            #endif
        }
        // Terminar la tarea actual
        vTaskDelete(NULL);

    #else

        // Buffer para almacenar la temperatura.
        uint8_t data[2];
        // Contador de mediciones de temperatura.
        static int read_count = 0;

        // Consulta la causa del wakeup en modo automatico (PM).
        #if SHOW_WUP_CAUSE == 1 && ENABLE_PM == 1
            esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
            show_wakeup_cause(cause);
        #endif

        // Ordenamos al sensor medir la temperatura.
        measure_i2c_temp();

        // Periodo de espera para que el sensor pueda calcular la temperatura.
        vTaskDelay(pdMS_TO_TICKS(50));

        // Obtenemos la read_counter realizada por el sensor.
        read_i2c_temp(data);

        // Mostramos la lectura del sensor en grados celsius.
        ESP_LOGI(TAG, "Temperaturas: %.2f\n", get_temp_celsius(data));

        // Tras 5 mediciones de temperatura dormimos al sistema.
        if ((++read_count%5) == 0) {
            // Modo "ligth sleep" o modo "deep sleep".
            #if DEEP_SLEEP == 0 && ENABLE_PM == 0
                enter_light_sleep();
            #elif ENABLE_PM == 0
                enter_deep_sleep();
            #endif

            // Consulta la causa del wakeup.
            #if SHOW_WUP_CAUSE == 1 && DEEP_SLEEP == 0 && ENABLE_PM == 0
                esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
                show_wakeup_cause(cause);
            #endif
        }
    #endif
}

void app_main(void)
{
    #if SHOW_WUP_CAUSE == 1 && DEEP_SLEEP == 1 && ENABLE_PM == 0
        esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
        show_wakeup_cause(cause);
    #endif

    // Inicializacion del dispositivo principal.
    i2c_main_init();

    #if ENABLE_PM == 1
        // Configura el power manager de ESP-IDF
        esp_pm_config_esp32_t pm_config;
        pm_config.max_freq_mhz = 240;
        pm_config.min_freq_mhz = 40;
        pm_config.light_sleep_enable = true;
        esp_pm_configure(&pm_config);
    #endif

    #if USE_TIMER == 0
        // Crea la tarea
        xTaskCreate(&read_callback, "Read_task", 2048, NULL, 5, NULL);
    #else
        // Identificador del timer
        esp_timer_handle_t timer;
        // Configuración del timer para las mediciones
        const esp_timer_create_args_t timer_args = {
            .callback = &read_callback, // funcion callback
            .name = "timertemp", // nombre
            .skip_unhandled_events = true // para ignorar lo que ocurre en sleep
        };
        // Crea y arranca el timer periodico (periodo ajustable con menuconfig)
        esp_timer_create(&timer_args, &timer);
        esp_timer_start_periodic(timer, CONFIG_SAMPLE_PERIOD*1000);
    #endif

    // Terminar la tarea actual.
    vTaskDelete(NULL);
}
