//******************************************************************************
//**** PRACTICA 3 ANIOT
//**** 
//******************************************************************************

#include <stdio.h>
#include <driver/adc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/touch_pad.h"

#include "main.h"

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

// Variables globales

// Estado
t_state state;

// Minutos y segundos del cronómetro
int min, seg;

// Sincronizacion
SemaphoreHandle_t sync = NULL;

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

static void hall_sampling_task(void* arg)
{
    // Habilita el ADC1 y establce la cantidad de bits para representar la señal
    adc1_config_width(ADC_WIDTH_BIT_12);

    // Bucle infinito
    while(1) {
        // Si se supera el humbral accionar el reset
        if(hall_sensor_read() >= 100) reset();
        // Muestreamos cada medio segundo
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

static void timer_callback(void* arg)
{
    // Variables para la copia local
    int sync_seg = 0, sync_min = 0;

    // Lee cuenta actual para imprimir luego
    if (xSemaphoreTakeFromISR(sync, NULL)) {
        sync_seg = seg;
        sync_min = min;
    }

    // Devuelve el semaforo
    xSemaphoreGiveFromISR(sync, NULL);

    // El valor de esta variable persiste entre llamadas a la función
    static bool level = false;

    // Cambia el estado logico del PIN de salida
    level = !level;

    // Establece el nivel logico (0 o 1) de tension (0 o 5) en el pin
    gpio_set_level((gpio_num_t) arg, level);

    // Imprime
    printf("CRONO %.2d:%.2d\n", sync_min, sync_seg);
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

static void tick_ISR(void* arg)
{
    // Toma el semaforo
    if (xSemaphoreTakeFromISR(sync, NULL)) {
        switch (state) {
            case init: break;
            case count:
                min = seg == 59 ? min + 1 : min;
                seg = (seg + 1) % 60;
            break;
            case stop: break;
            default: abort(); break;
        }
    }

    // Devuelve el semaforo
    xSemaphoreGiveFromISR(sync, NULL);
}

// ISR del boton start/stop.
static void start_stop_ISR(void* arg)
{
    // Borra la interrupción causada por el touch pad
    touch_pad_clear_status();

    // Toma el semaforo
    if (xSemaphoreTakeFromISR(sync, NULL)){
        switch (state) {
            case init: state = count; break;
            case count: state = stop; break;
            case stop: state = count; break;
            default: abort(); break;
        }
    }

    // Devuelve el semaforo
    xSemaphoreGiveFromISR(sync, NULL);
}

// Funcion para el reset (la ISR va aparte)
static void reset()
{
    // Toma el semaforo
    if (xSemaphoreTakeFromISR(sync, NULL)){
        // Reseteamos el contador
        seg = 0;
        min = 0;
        switch (state) {
            case init: break;
            case count: state = init; break;
            case stop: state = init; break;
            default: abort(); break;
        }
    }

    // Devuelve el semaforo
    xSemaphoreGiveFromISR(sync, NULL);
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

void app_main(void)
{
    // Estado inicial
    state = init;
    min = 0;
    seg = 0;

    // Variable para leer el valor de los touch pad
    uint16_t tp_value;

    // Crea el semaforo para sincronizar el acceso a variables globales
    sync = xSemaphoreCreateCounting(1, 0);

    // Identificador del timer
    esp_timer_handle_t timer;

    // Configuración del timer
    const esp_timer_create_args_t timer_args = {
        .callback = &timer_callback, // funcion callback
        .arg = (void*) GPIO_NUM_18, // argumento a pasar a la función callcack
        .name = "cronometro" // el nombre es opcional, pero ayuda a depurar
    };

    // Declaramos una variable para configurar los pines
    gpio_config_t io_conf;

    // Configuramos los pines de salida.
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE; // Se indica el tipo de interrupcion, en este caso, no queremos generar una interrupcion.
    io_conf.mode = GPIO_MODE_OUTPUT; // Indicamos que es un pin de salida.
    io_conf.pin_bit_mask = (1ULL<<GPIO_NUM_18); // Indicamos a que pines afecta la configuracion (GPIO 18).
    io_conf.pull_down_en = 0; // Desabilita la resistencia de pull-down.
    io_conf.pull_up_en = 0; // Desabilita la resistencia de pull-up.
    gpio_config(&io_conf); // Establece la configuracion del PIN.

    // Configuramos los pines de entrada.
    io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE; // Flanco de subida o bajada.
    io_conf.pin_bit_mask = (1ULL<<GPIO_NUM_19); // Indicamos a que pines afecta la configuracion (GPIO 19).
    io_conf.mode = GPIO_MODE_INPUT; // Indicamos que es un pin de entrada.
    io_conf.pull_down_en = 0; // Desabilita la resistencia de pull-down.
    io_conf.pull_up_en = 0; // Desabilita la resistencia de pull-up.
    gpio_config(&io_conf); // Establece la configuracion del PIN.

    // Habilita el uso de diferentes ISR para el tratamiento de interrupciones
    gpio_install_isr_service(0);

    // Añade un manejador de interrupciones al PIN de entrada
    gpio_isr_handler_add(GPIO_NUM_19, tick_ISR, (void*) GPIO_NUM_19);

    // Inicialización del touch pad
    touch_pad_init();

    /*
    Hay que indicar si se realizarán mediciones sobre el panel táctil
    con la ayuda de un temporizador HW (MODE_TIMER) o SW (MODE_SW).
    */
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);

    /*
    Iniciamos el touch pad 0 sin umbral de interrupción
    NOTA (pines del touch pad):
    TOUCH_0 -> GPIO_4
    TOUCH_2 -> GPIO_2
    TOUCH_3 -> GPIO_15
    TOUCH_4 -> GPIO_13
    TOUCH_5 -> GPIO_12
    TOUCH_6 -> GPIO_14
    TOUCH_7 -> GPIO_27
    TOUCH_8 -> GPIO_33
    TOUCH_9 -> GPIO_32
    */
    touch_pad_config(TOUCH_PAD_NUM0, 0);

    /*
    Inicia un filtro para procesar el ruido con el fin de evitar falsos
    disparos al detectar un cambio de capacitancia. Esto utiliza el timer.
    Requiere indicar el número de milisegundos cada cual se calibrará el filtro.
    */
    touch_pad_filter_start(10);

    // Lee el valor que emite el PIN por defecto
    touch_pad_read_filtered(TOUCH_PAD_NUM0, &tp_value);

    // El umbral de interrupción es 2/3 del valor inicial
    touch_pad_set_thresh(TOUCH_PAD_NUM0, tp_value * 2 / 3);

    // Función que tratará las interrupciones causadas por los touch pad
    touch_pad_isr_register(start_stop_ISR, NULL);

    // Habilita el tratamiento de las interrupciones del touch pad
    touch_pad_intr_enable();

    // Crea una tarea para tomar muestras del campo magnetico
    xTaskCreate(&hall_sampling_task, "hall_sampling_task", 2048, NULL, 5, NULL);

    // Crea el timer
    esp_timer_create(&timer_args, &timer);

    // Arranca el timer forever
    esp_timer_start_periodic(timer, 1000000);

    // Termina la tarea actual
    vTaskDelete(NULL);
}
