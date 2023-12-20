#ifndef SIM_BP_H
#define SIM_BP_H

typedef struct bp_params{
    unsigned long int K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int N;
    char*             bp_name;
}bp_params;

typedef struct branch_prediction_data{
    unsigned long int counter;
}branch_prediction_data;

typedef struct set_index_data_t{
    unsigned long int set;
    unsigned long int index;
}set_index_data_t;

typedef struct display_data_t
{
    unsigned long int set;
    unsigned long int index;
    unsigned long int predictions;
    unsigned long int mispredictions;
    
}display_data_t;


// Put additional data structures here as per your requirement

#endif
