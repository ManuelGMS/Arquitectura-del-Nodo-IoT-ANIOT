// *****************************************************************************
// *** PRACTICA 4 ANIOT
// *** 
// *****************************************************************************

#include <stdio.h>
#include "driver/i2c.h"

#define SECONDARY_ADDR              0x40        // Dirección del dispositivo secundario (Sensor temp & humedad)
#define DISABLE                     0           // Para deshabilitar algún elemento o indicar que no se hace uso de él    
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
    // Configuración del I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MAIN,
        .sda_io_num = I2C_MAIN_SDA_GPIO_PIN,
        .scl_io_num = I2C_MAIN_SCL_GPIO_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MAIN_FREQ_HZ,
    };

    // Creamos la configuración para un puerto I2C
    i2c_param_config(I2C_MAIN_PORT, &conf);
    
    // Instalamos la configuración creada
    i2c_driver_install(I2C_MAIN_PORT, conf.mode, DISABLE, DISABLE, DISABLE);
}

static void read_callback(void *args)
{
    // Buffer para almacenar la temperatura.
    uint8_t data[2];

    // Declaramos una cola de comandos.
    i2c_cmd_handle_t cmd;

    // MEDICION ****************************************************************

    // Creamos una cola de comandos.
    cmd = i2c_cmd_link_create();

    // Bit de START: se alerta a los dispositivos secundarios.
    i2c_master_start(cmd);  

    // Dirección (7b) del secundario y operación (1b). Habilita las confimaciones.
    i2c_master_write_byte(cmd, (SECONDARY_ADDR << 1) | WRITE_BIT, ACK_ENABLED); 

    // Indica al sensor que debe medir la temperatura. No hold master mode
    i2c_master_write_byte(cmd, 0xF3, ACK_ENABLED);

    // Se libera el uso del bus I2C. 
    i2c_master_stop(cmd);  

    // Ejecutamos todos los comandos encolados.
    i2c_master_cmd_begin(I2C_MAIN_PORT, cmd, pdMS_TO_TICKS(100));

    // Eliminamos la cola de comandos.
    i2c_cmd_link_delete(cmd);

    // ESPERA ******************************************************************
    
    // Periodo de espera para que el sensor pueda calcular la temperatura.
    vTaskDelay(pdMS_TO_TICKS(50));

    // LECTURA *****************************************************************

    // Creamos otra cola de comandos.
    cmd = i2c_cmd_link_create();

    // Bit de START: se alerta a los dispositivos secundarios.
    i2c_master_start(cmd);

    // Dirección (7b) del secundario y operación (1b). Habilita las confimaciones.
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

    // IMPRIME *****************************************************************

    printf("Temperatura: %.2f\n", get_temp_celsius(data));

}

void app_main(void)
{
    // Identificador del timer
    esp_timer_handle_t timer;

    // Inicialización del dispositivo principal.
    i2c_main_init();

    // Configuración del timer para las mediciones
    const esp_timer_create_args_t timer_args = {
        .callback = &read_callback, // funcion callback
        .name = "timertemp" // nombre 
    };

    // Crea y arranca el timer periodico (periodo ajustable con menuconfig)
    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, CONFIG_SAMPLE_PERIOD*1000);

    // Terminar la tarea actual
    vTaskDelete(NULL);
}
