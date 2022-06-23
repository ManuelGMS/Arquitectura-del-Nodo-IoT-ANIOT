// *****************************************************************************
// *** PRACTICA 6 ANIOT
// ***
// *****************************************************************************

#include "main.h"

// *****************************************************************************

// Los LOGs llamarán a esta función.
int vprintf_log(const char *format, va_list arguments)
{
    int ret = 0;

    if (fd_write == NULL){
        printf("Error al abrir fichero para escritura\n");
        ret = -1;
        return ret;
    }

    // Escribimos el texto en el fichero
    ret = vfprintf(fd_write, format, arguments);

    // La funcion se ejecuto (o no) correctamente
    return ret;
}

// Convierte a celsius las lecturas del sensor
float get_temp_celsius(uint8_t *data)
{
    uint16_t data16 = 0;

    // Pone el dato en formato de 16 bits
    data16 = (((data[0] & 0xff) << 8) | (data[1] & 0xff));

    // Calculamos y devolvemos la temperatura
    return ((175.72f * data16) / 65536.0f) - 46.85f;
}

static void i2c_main_init(void)
{
    // Configuracion del I2C
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
    // Declaramos una cola de comandos
    i2c_cmd_handle_t cmd;

    // Creamos una cola de comandos
    cmd = i2c_cmd_link_create();

    // Bit de START: se alerta a los dispositivos secundarios
    i2c_master_start(cmd);
    // Dirección (7b) del secundario y operación (1b). Habilita las confimaciones
    i2c_master_write_byte(cmd, (SECONDARY_ADDR << 1) | WRITE_BIT, ACK_ENABLED);
    // Indica al sensor que debe medir la temperatura. No hold master mode
    i2c_master_write_byte(cmd, 0xF3, ACK_ENABLED);
    // Se libera el uso del bus I2C
    i2c_master_stop(cmd);

    // Ejecutamos todos los comandos encolados.
    i2c_master_cmd_begin(I2C_MAIN_PORT, cmd, pdMS_TO_TICKS(100));

    // Eliminamos la cola de comandos
    i2c_cmd_link_delete(cmd);
}

static void read_i2c_temp(uint8_t *data)
{
    // Declaramos una cola de comandos
    i2c_cmd_handle_t cmd;

    // Creamos otra cola de comandos
    cmd = i2c_cmd_link_create();

    // Bit de START: se alerta a los dispositivos secundarios.
    i2c_master_start(cmd);
    // Direccion (7b) del secundario y operacion (1b). Habilita las confimaciones
    i2c_master_write_byte(cmd, (SECONDARY_ADDR << 1) | READ_BIT, ACK_ENABLED);
    // Leemos el bit mas significativo
    i2c_master_read_byte(cmd, &data[0], ACK_VALUE);
    // Leemos el bit menos significativo
    i2c_master_read_byte(cmd, &data[1], NACK_VALUE);
    // Se libera el uso del bus I2C
    i2c_master_stop(cmd);

    // Ejecutamos todos los comandos encolados
    i2c_master_cmd_begin(I2C_MAIN_PORT, cmd, pdMS_TO_TICKS(100));

    // Eliminamos la cola de comandos
    i2c_cmd_link_delete(cmd);
}

static void sensor_callback(void *args)
{
    // Buffer para almacenar la temperatura
    uint8_t data[2];

    // Ordenamos al sensor medir la temperatura
    measure_i2c_temp();

    // Periodo de espera para que el sensor pueda calcular la temperatura
    vTaskDelay(pdMS_TO_TICKS(50));

    // Obtenemos la read_counter realizada por el sensor
    read_i2c_temp(data);

    // Mostramos la lectura del sensor en grados celsius
    ESP_LOGI(TAG, "Temperatura: %.2f\n", get_temp_celsius(data));
    printf("Escrito en fichero -> Temperatura: %.2f\n", get_temp_celsius(data));
}

static void read_callback(void *args)
{
    int i = 0;

    // Detiene el timer periodico
    esp_timer_stop(timer_sensor);
    esp_timer_delete(timer_sensor);

    // Si no puede abrir el fichero para leer de el, se informa y se detiene el programa
    if (fd_read == NULL) {
        ESP_LOGE(TAG, "Fallo al intentar abrir el fichero para leer de el.");
        printf("Fallo al intentar abrir el fichero para leer de el\n");
        return;
    }

    // Crea un buffer para leer informacion del fichero
    char data[LOG_MAX_LINE_LEN];
    
    // Lee las lineas que toque del fichero
    for (i = 0; i < LOG_LINES_READ; ++i) {
        // Limpiar la casa y coser
        memset(data, '\0', LOG_MAX_LINE_LEN);

        // Leemos una linea del fichero
        if (fgets(data, LOG_MAX_LINE_LEN, fd_read) == NULL){
            ESP_LOGE(TAG, "Error al leer el fichero");
            printf("Error al leer el fichero\n");
        } else {
            // Imprime por serie lo leido del fichero
            printf("Linea %i leida: %s\n", i+1, data);
        }
    }
    printf("\n");
}

wl_handle_t fs_mount(void)
{
   // Configuración de un File System de tipo FAT.
    const esp_vfs_fat_mount_config_t fs_config = {
        // Número maximo de ficheros que pueden abrirse.
        .max_files = 2,
        // Si el FS tipo FAT no puede montarse (es una nueva particion), crea su tabla de particiones y formatea el sistema de ficheros. 
        .format_if_mount_failed = true,
        // Si "format_if_mount_failed = true", y el FS no puede montarse, establece la unidad de almacenamiento en 4096 bytes.
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };

    // Identificador de nuestro sistema de ficheros.
    wl_handle_t fs_handler = WL_INVALID_HANDLE;

    // LOG informativo para ver el estado de la ejecución.
    ESP_LOGI(TAG, "Procediendo al montaje del sistema de ficheros FAT.");

    // Funcion para chequear la correcta ejecucion del comando pasado, se aborta en caso contrario.
    ESP_ERROR_CHECK(
        // Función para registrar el Virtual File System, montarlo e inicializarlo en la SPI FLASH.
        esp_vfs_fat_spiflash_mount(
            // Ruta donde va a montarse el sistema de ficheros.
            "/spiflash",
            // Etiqueta de la partición, ha de aparecer en la tabla de particiones csv.
            "aniotfatfs",
            // Puntero a la estructura que contiene la configuración del sistema de ficheros.
            &fs_config,
            // Puntero al identificador de nuestro sistema de ficheros. 
            &fs_handler
        )
    );
    
    // LOG informativo para ver el estado de la ejecución.
    ESP_LOGI(TAG, "El montaje del sistema de ficheros FAT se realizo correctamente.");

    // Creamos y abrimos el fichero
    ESP_ERROR_CHECK(init_file_descriptors());

    // Función para indicar a dónde deben dirigirse los LOG's (UART0 por defecto).    
    esp_log_set_vprintf(&vprintf_log);

    return fs_handler;
}

void fs_unmount(wl_handle_t fs_handler)
{
    // Restauramos la funcion que por defecto trata los mensajes.
    esp_log_set_vprintf(vprintf);

    // Cerrar ficheros
    ESP_ERROR_CHECK(close_file_descriptors());

    // LOG informativo para ver el estado de la ejecucion
    ESP_LOGI(TAG, "Desmontando el sistema de ficheros.");

    // Desmonta el sistema de ficheros y chequea que se haya podido
    ESP_ERROR_CHECK(esp_vfs_fat_spiflash_unmount("/spiflash", fs_handler));

    // LOG informativo para ver el estado de la ejecucion
    ESP_LOGI(TAG, "Sistema de ficheros desmontado con exito.");
}

int close_file_descriptors()
{
    // Cerramos la comunicacion con los ficheros
    if (fclose(fd_write) == EOF) return -1;
    if (fclose(fd_read) == EOF) return -2;

    return 0;
}

int init_file_descriptors()
{
    // Abrirmos un fichero en modo escritura
    fd_write = fopen(path_to_file, "wb");

    // Si no podemos abrir el fichero para leer de el, se informa y se detiene el programa
    if (fd_write == NULL) {
        ESP_LOGE(TAG, "Fallo al intentar abrir el fichero en modo escritura.");
        return -1;
    }

    // Abrimos un fichero en modo lectura.
    fd_read = fopen(path_to_file, "rb");

    // Si no podemos abrir el fichero para leer de el, se informa y se detiene el programa
    if (fd_read == NULL) {
        ESP_LOGE(TAG, "Fallo al intentar abrir el fichero en modo lectura.");
        return -2;
    }

    // El fichero se abrio correctamente en ambos modos
    return 0;
}

void app_main(void)
{
    // Inicializacion del I2C
    i2c_main_init();

    // Monta el sistema de ficheros
    wl_handle_t fs_handler = fs_mount();

    // Prueba que la escritura/lectura se hace con exito
    printf("\nTEST DE LECTURA/ESCRITURA\n");
    char test_data[6] = "test\0\0";
    if (fwrite(test_data, sizeof(char), 5, fd_write) == 5){
        printf(" * Exito en prueba de escritura. Escrito: %s\n", test_data);
    } else {
        printf(" * Error en la prueba de escritura (puede ser un fallo de la flash)\n");
    }
    memset(test_data, 0, 6*sizeof(char));
    if (fgets(test_data, 4, fd_read) == NULL){
        printf(" * Error en la prueba de lectura (puede ser un fallo de la flash)\n\n");
    } else {
        printf(" * Exito en la prueba de lectura. Leido: %s\n\n", test_data);
    }

    // Timer del sensor
    const esp_timer_create_args_t timer_sensor_args = {
        .callback = &sensor_callback, // funcion callback
        .name = "timertemp", // nombre
        //.skip_unhandled_events = true // para ignorar lo que ocurre en sleep
    };

    // Timer de lectura
    const esp_timer_create_args_t timer_read_args = {
        .callback = &read_callback, // funcion callback
        .name = "timerread", // nombre
    };

    // Crea y arranca timers
    esp_timer_create(&timer_sensor_args, &timer_sensor);
    esp_timer_start_periodic(timer_sensor, CONFIG_SAMPLE_PERIOD*1000); // Cada 10 segundos

    esp_timer_create(&timer_read_args, &timer_read);
    esp_timer_start_once(timer_read, 60000000); // A los 60 segundos

    // Terminar la tarea actual
    vTaskDelete(NULL);
}
