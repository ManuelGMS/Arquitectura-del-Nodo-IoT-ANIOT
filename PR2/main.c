//************************************************************************************************************************************
//**** PRACTICA 2 ANIOT
//****
//************************************************************************************************************************************

#include <time.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "main.h"

//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************

static void sensor_task(void *args)
{

    // Cargamos los argumentos que le son pasados a esta tarea
    struct SensorArgs *sensor_args = (struct SensorArgs *) args;

    struct SensorData data; // Datos leidos por el sensor que le seran enviados a un filtro

    // Bucle para la generación de muestras
    while (1) {

        // Almacena el sello de tiempo
        data.time = time(NULL);

        // Genera un número aleatorio
        data.val = esp_random() % 10;

        // Debug
        printf("EL SENSOR %s HA GENERADO EL DATO %d\n", pcTaskGetName(NULL), data.val);

        // Se encola el dato y se comrueba si ha habido errores
        if(xQueueSendToBack(*(sensor_args->sensor_out), &data, pdMS_TO_TICKS(1000)) != pdTRUE)
            printf("EL SENSOR %s NO PUDO ENCOLAR EL DATO LEIDO\n", pcTaskGetName(NULL));

        // Pasar la tarea al estado de BLOCKED.
        vTaskDelay(pdMS_TO_TICKS(sensor_args->period));

    }

    // Terminar la tarea actual
    vTaskDelete(NULL);

}

//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************

/**
 * Tarea filtro. Esta tarea recibirá las muestras de la tarea anterior. Realizará
 * una media ponderada de las últimas cinco muestras usando los coeficientes
 * (0.05, 0.10, 0.15, 0.25, 0.45). Durante las primeras cuatro muestras no
 * realizará ningún cómputo
 * */
static void filter_task(void *args)
{

    // Cargamos los argumentos que le son pasados a esta tarea
    struct FilterArgs *filter_args = (struct FilterArgs *) args;

    int i = 0; // Iterador para calcular la media
    int log_index = 0; // Índice del registro de datos
    int data_count = 0; // Cuantos datos se han leido
    struct FilterData filterData; // Dato que se envia al controlador
    struct SensorData newSensorData; // Dato que es leido por el sensor
    struct SensorData logSensorData[CONFIG_AVERAGE_LEN]; // Registro con los últimos datos que ha leido el sensor

    // Obtener el nombre de la tarea actual
    const char* taskName = pcTaskGetName(NULL);

    // Cargamos una sola vez el nombre en el dato a enviar al controlador
    filterData.name = (char*) malloc(sizeof(taskName));
    strcpy(filterData.name, taskName);

    while (1) {

        // Intentamos leer de la cola
        if(xQueueReceive(*(filter_args->sensor_out), &newSensorData, pdMS_TO_TICKS(1000)) == pdTRUE) {

            // Debug
            printf("EL FILTRO %s HA LEIDO DE LA COLA %d\n", pcTaskGetName(NULL), newSensorData.val);

            // Incrementa el contador hasta que se tienen tantos datos como para calcular la media ponderada
            data_count = (data_count >= CONFIG_AVERAGE_LEN) ? data_count : data_count + 1;

            // Almacenamos el nuevo dato leido
            logSensorData[log_index] = newSensorData;

            // Actualizar el indice de forma que se mantenga en el rango [0 - (CONFIG_AVERAGE_LEN-1)]
            log_index = (log_index + 1) % CONFIG_AVERAGE_LEN;

            // Se comprueba si se han leído tantos datos como para calcular la media ponderada
            if (data_count >= CONFIG_AVERAGE_LEN) {

                // Establecemos el valor inicial de la media ponderada
                filterData.avg = 0;

                // Cálculo de la media ponderada
                for (i = 0; i < CONFIG_AVERAGE_LEN; ++i)
                    filterData.avg += FILTER_COEFS[i] * logSensorData[(log_index+i) % CONFIG_AVERAGE_LEN].val;

                // El sello del tiempo es el del dato mas nuevo
                filterData.time = newSensorData.time;

                // Intentamos escribir en la cola
                if(xQueueSendToBack(*(filter_args->filter_out), &filterData, pdMS_TO_TICKS(1000)) != pdTRUE)
                    printf("ERROR EN FILTRO %s AL ENCOLAR DATO\n", pcTaskGetName(NULL));

            }

        }

    }

    // Liberamos la memoria del string
    free(filterData.name);

    // Terminar la tarea actual
    vTaskDelete(NULL);

}

//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************

/**
 * Tarea controlador. Esta tarea emula el control de nuestra aplicación. Quedará
 * a la espera de notificaciones por parte de tareas tipo filtro. Cuando le llegue
 * alguna, imprimirá por el puerto serie el nombre de la tarea que le envía el dato,
 * el valor recibido y un timestamp del instante en que se recogió la muestra por
 * parte de la correspondiente tara sensor.
 *
 * Asimismo, la tarea controlador imprimirá periódicamente (cada 10 segundos) una
 * estadística de las tareas del sistema haciendo uso de la llamada
 * vTaskGetRunTimeStats(). Para poder usar dicha función deberás activar una opción
 * en menuconfig.
 * */
static void controller_task(void *args)
{

    // Cargamos los argumentos que le son pasados a esta tarea
    QueueSetHandle_t *queue_set_dir = (QueueSetHandle_t *) args;

    QueueHandle_t queue_handler; // Identificador de alguna cola del conjunto de colas
    struct FilterData filterData; // Para enviar datos al controlador

    // Si hemos configurado el proyecto para generar estadísticas
    #ifdef CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
    char statistics[1024]; // Buffer para almacenar la informacion de las estadísticas
    time_t begin_t = time(NULL), current_t; // Capturamos un instante de tiempo inicial
    #endif

    while (1) {

        // Si hemos configurado el proyecto para generar estadísticas
        #ifdef CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
        // Obtenemos el tiempo actual
        current_t = time(NULL);
        // Si la diferencia de tiempo es de 10 segundos
        if(difftime(current_t, begin_t) >= 10) {
            begin_t = current_t; // Actualizamos la marca de tiempo
            vTaskGetRunTimeStats(statistics); // Cargamos el informe
            printf("ESTADISTICAS DE EJECUCION:\n%s\n", statistics); // Mostramos el informe
        }
        #endif

        // Intentamos obtener el identificador de alguna cola del conjunto de colas
        queue_handler = xQueueSelectFromSet(*(queue_set_dir), 0);

        // Si hemos podido obtener el identificador de una cola y después leer de ella, entonces mostramos el resultado
        if((queue_handler != NULL) && (xQueueReceive(queue_handler, &filterData, 0) == pdTRUE))
            printf("CONTROLLER: %f, %ld, %s\n", filterData.avg, filterData.time, filterData.name);

    }

    // Terminar la tarea actual
    vTaskDelete(NULL);

}

//*************************************************************************************************************************************
//*************************************************************************************************************************************
//*************************************************************************************************************************************

// Punto de entrada, esta tarea siempre se ejecuta en el CORE 0 (PRO)
void app_main(void)
{

    // Contador
    unsigned int i = 0;

    // Identificador del conjunto de colas
    QueueSetHandle_t filter_out_set;

    // Identificadores de las tareas
    TaskHandle_t controller_handle;
    TaskHandle_t sensor_handle[CONFIG_FLOW_COUNT];
    TaskHandle_t filter_handle[CONFIG_FLOW_COUNT];

    // Array de flujos
    struct QueueFlow flow[CONFIG_FLOW_COUNT];

    // Arrays de argumentos a tareas
    struct SensorArgs sensor_args[CONFIG_FLOW_COUNT];
    struct FilterArgs filter_args[CONFIG_FLOW_COUNT];

    // Arrays con los nombres de los sensores y las tareas
    char *sensor_task_name[CONFIG_FLOW_COUNT] = {"sensor1", "sensor2"};
    char *filter_task_name[CONFIG_FLOW_COUNT] = {"filter1", "filter2"};

    // Crea el conjunto de colas
    filter_out_set = xQueueCreateSet(CONFIG_FLOW_COUNT * CONFIG_QUEUE_LEN);

    // Cada vuelta se inicializa un flujo de datos hacia el controlador
    for (i = 0; i < CONFIG_FLOW_COUNT; ++i) {

        // Crea la cola del sensor
        if ((flow[i].sensor_out = xQueueCreate(CONFIG_QUEUE_LEN, sizeof(struct SensorData))) == NULL)
            printf("ERROR AL CREAR LA COLA DEL SENSOR\n");

        // Crea la cola del filtro
        if ((flow[i].filter_out = xQueueCreate(CONFIG_QUEUE_LEN, sizeof(struct FilterData))) == NULL)
            printf("ERROR AL CREAR COLA DEL FILTRO\n");

        // Preparar los argumentos y datos del sensor
        sensor_args[i].period = CONFIG_RANDOM_GEN_PERIOD;
        sensor_args[i].sensor_out = &(flow[i].sensor_out);

        // Preparar los argumentos y datos del filtro
        filter_args[i].sensor_out = &(flow[i].sensor_out);
        filter_args[i].filter_out = &(flow[i].filter_out);

        // Incorpora la cola al conjunto de colas
        xQueueAddToSet(flow[i].filter_out, filter_out_set);

        // Crea la tarea del sensor
        xTaskCreate(&sensor_task, sensor_task_name[i], 3072, &sensor_args[i], 5, &sensor_handle[i]);

        // Crea la tarea del filtro
        xTaskCreatePinnedToCore(&filter_task, filter_task_name[i], 3072, &filter_args[i], 5, &filter_handle[i], i % 2);

    }

    // Crea el controller
    xTaskCreate(&controller_task, "controller", 3072, &filter_out_set, 5, &controller_handle);

    // Terminar la tarea actual
    vTaskDelete(NULL);

}
