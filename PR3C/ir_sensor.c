//******************************************************************************
//**** PRACTICA 3 (CONTINUACION) ANIOT
//**** 
//******************************************************************************

#include "ir_sensor.h" 

int get_ir_distance(int adc_read){
    // 1149 y 1086
    int i = 0;
    int rdy = 0;

    int dist = 0, dist_low, dist_high;
    int adc_low, adc_high;

    int adc_i_low = 0;
    int adc_i_high = 0;

    // Intenta acotar entre que dos valores de la tabla va a interpolar
    while (i < NUM_READS && rdy == 0){
        if (adc_read < tab_adc[i]) {
            adc_i_high = i;
            adc_i_low = i-1;
            rdy = 1;
        }
        ++i;
    }
    
    if (!rdy) {
        // Ha llegado hasta el final de la tabla -> distancia 0
        dist = tab_distance[NUM_READS-1];
    } else if (adc_i_high == 0) {
        // No ha pasado del principio de la tabla -> distancia infinito
        dist = tab_distance[0];
    } else {
        // Interpolar
        dist_low  = tab_distance[adc_i_low];
        dist_high = tab_distance[adc_i_high];

        adc_low = tab_adc[adc_i_low];
        adc_high = tab_adc[adc_i_high];

        dist = dist_low + ((adc_read-adc_low)*(dist_high-dist_low))/(adc_high-adc_low);
    }

    return dist;
}
