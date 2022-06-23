//************************************************************************************************************************************
//**** PRACTICA 2 ANIOT
//**** 
//************************************************************************************************************************************

#define CONFIG_FLOW_COUNT 2

// Un flujo lo componen un sensor y un filtro
struct QueueFlow {
    QueueHandle_t sensor_out;
    QueueHandle_t filter_out;
};

// Argumentos que recibe una tarea de tipo sensor
struct SensorArgs {
    int period; // Cada n (period) milisegundos se toman muestras.
    QueueHandle_t *sensor_out; // Identificador de la cola en la que tiene que guardar los datos.
};

// Argumentos que recibe una tarea de tipo filtro
struct FilterArgs {
    QueueHandle_t *sensor_out; // Identificador de la cola de la que tiene que leer los datos.
    QueueHandle_t *filter_out; // Identificador de la cola en la que tiene que guardar los datos.
};

// Datos de la cola del sensor
struct SensorData {
    time_t time;
    uint32_t val;
};

// Datos de la cola del filtro
struct FilterData {
    char* name;
    time_t time;
    float avg;
};

// Coeficientes del filtro
const float FILTER_COEFS[CONFIG_AVERAGE_LEN] = {0.05, 0.10, 0.15, 0.25, 0.45};
