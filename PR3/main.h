//******************************************************************************
//**** PRACTICA 3 ANIOT
//**** 
//******************************************************************************

// FSM
enum enum_state {
    init,
    count,
    stop
};

typedef enum enum_state t_state;

// Prototipos de las funciones

static void reset();

static void timer_callback(void* arg);

static void io_task(void* arg);
static void hall_sampling_task(void* arg);

static void tick_ISR(void* arg);
static void start_stop_ISR(void* arg);
