// *****************************************************************************
// *** PRACTICA 6 ANIOT
// ***
// *****************************************************************************

#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "string.h"

// *****************************************************************************

#define SECONDARY_ADDR              0x40        // Direccipn del dispositivo secundario (Sensor de temperatura y humedad)
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

#define LOG_LINES_READ    5 // Numero de lines que seran leidas del fichero log
#define LOG_MAX_LINE_LEN  64 // Tamanio maximo de linea en el log

// *****************************************************************************

static const char *TAG = "PR6";

// Ruta a nuestro fichero de texto.
static const char *path_to_file = "/spiflash/logs.txt";

// Descriptores de fichero
FILE *fd_write = NULL;
FILE *fd_read = NULL;

// Timers
esp_timer_handle_t timer_sensor;
esp_timer_handle_t timer_read;

float get_temp_celsius(uint8_t *data);
static void i2c_main_init(void);
static void sensor_callback(void *args);
static void measure_i2c_temp();
static void read_i2c_temp();
static int vprintf_log(const char *format, va_list arguments);
static void read_callback(void *args);
wl_handle_t fs_mount(void);
void fs_unmount(wl_handle_t fs_handler);
int init_file_descriptors();
int close_file_descriptors();
void read_log_file();
