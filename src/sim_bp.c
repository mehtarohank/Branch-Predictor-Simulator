#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sim_bp.h"

branch_prediction_data *bimodal_table, *gshare_table,*hybrid_table;

display_data_t display_data;
unsigned long int gbh_register;
set_index_data_t bimodal_set_index,gshare_set_index,hybrid_set_index;

void seprateindexAddr(unsigned long int addr,bp_params params);
void bimodal_GshareFunc(char outcome,bp_params params,char* bp_name);
void hybridFunc(char outcome,bp_params params);
void allocatingDynamicMemoryFunc(char* bp_name);
void display(bp_params params);


/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim bimodal 6 gcc_trace.txt
    argc = 4
    argv[0] = "sim"
    argv[1] = "bimodal"
    argv[2] = "6"
    ... and so on
*/
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file
    
    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.bp_name  = argv[1];
    
    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        trace_file      = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);

        bimodal_set_index.set = (unsigned int)pow(2, params.M2);

        allocatingDynamicMemoryFunc("bimodal");
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);

        gshare_set_index.set = (unsigned int)pow(2, params.M1);

         allocatingDynamicMemoryFunc("gshare");
    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);
    
        hybrid_set_index.set = (unsigned int)pow(2, params.K);
        bimodal_set_index.set = (unsigned int)pow(2, params.M2);
        gshare_set_index.set = (unsigned int)pow(2, params.M1);

        allocatingDynamicMemoryFunc("bimodal");
        allocatingDynamicMemoryFunc("gshare");
        allocatingDynamicMemoryFunc("hybrid");

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }
    
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }

    char str[2];
    while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        outcome = str[0];
        seprateindexAddr(addr,params);
        display_data.predictions++;
     
        if (outcome == 't')
        {
           // printf("%lx %s\n", addr, "t");           // Print and test if file is read correctly
        }
        else if (outcome == 'n')
        {
           // printf("%lx %s\n", addr, "n");          // Print and test if file is read correctly
        }

        if(strcmp(params.bp_name, "bimodal") == 0)
        {
            bimodal_GshareFunc(outcome,params,"bimodal");
        }
        else if(strcmp(params.bp_name, "gshare") == 0)
        {
            bimodal_GshareFunc(outcome,params,"gshare");
        }
        else
        {
            hybridFunc(outcome,params);
        }
    }
    display(params);

    return 0;
}

void seprateindexAddr(unsigned long int addr,bp_params params)
{
    addr >>=2;
    
    if(strcmp(params.bp_name, "bimodal") == 0)  
    {
        int mask = (1 << params.M2) - 1;
        bimodal_set_index.index = addr & mask;
    }
    else if(strcmp(params.bp_name, "gshare") == 0) 
    {
        int mask = (1 << params.M1) - 1;
        unsigned long int tempindex = addr & mask;

        if(params.N != 0)
        {
            unsigned long int tempghb = gbh_register << (params.M1 - params.N);
            gshare_set_index.index = tempghb ^ tempindex;
        }
        else
        {
            gshare_set_index.index = tempindex;
        }
    }
    else if(strcmp(params.bp_name, "hybrid") == 0)
    {
        int mask = (1 << params.M2) - 1;
        bimodal_set_index.index = addr & mask;
        mask = 0;

        mask = (1 << params.M1) - 1;
        unsigned long int tempindex = addr & mask;
        if(params.N != 0)
        {
            unsigned long int tempghb = gbh_register << (params.M1 - params.N);
            gshare_set_index.index = tempghb ^ tempindex;
        }
        else
        {
            gshare_set_index.index = tempindex;
        }
        mask = 0;

        mask = (1 << params.K) - 1;
        hybrid_set_index.index = addr & mask;
    }
}

void hybridFunc(char outcome,bp_params params)
{
    int bimodal_correct = 0,bimodal_incorrect = 0,gshare_correct = 0,gshare_incorrect = 0;
    int c_flag = 0;

    if(outcome == 't')
    {
       if(bimodal_table[bimodal_set_index.index].counter < 2)    
       {
             bimodal_incorrect = 1;
       }
       if (bimodal_table[bimodal_set_index.index].counter >= 2) 
       {
             bimodal_correct = 1;
       }
       if(gshare_table[gshare_set_index.index].counter < 2)    
       {
             gshare_incorrect = 1;
       }
       if (gshare_table[gshare_set_index.index].counter >= 2) 
       {
             gshare_correct = 1;
       }

    }
    else if(outcome == 'n')
    {
         if (bimodal_table[bimodal_set_index.index].counter >= 2)
         {
            bimodal_incorrect = 1;
         }
         if (bimodal_table[bimodal_set_index.index].counter < 2) 
         {
            bimodal_correct = 1;
         }
         if (gshare_table[gshare_set_index.index].counter >= 2)
         {
            gshare_incorrect = 1;
         }
         if (gshare_table[gshare_set_index.index].counter < 2) 
         {
            gshare_correct = 1;
         }
    }

    /*Make an overall prediction. Use index to get the branch chooser counter from the chooser table. 
    If the chooser counter value is greater than or equal to 2, then use the prediction that was obtained from 
    the gshare predictor otherwise use the prediction that was obtained from the bimodal predictor.*/

    if(hybrid_table[hybrid_set_index.index].counter >= 2)
    {
        c_flag = 1; // choose gshare
    }
    else if(hybrid_table[hybrid_set_index.index].counter < 2)
    {
        c_flag = 0; // choose bimodal
    }
       
    //Update the selected branch predictor based on the branch actual outcome
    if(c_flag == 0) //update bimodal
    {
        bimodal_GshareFunc(outcome,params,"bimodal");  

        if(params.N !=0)
        {
           // Update the global branch history register
           // Note that the gshare global branch history register must always be updated, even if bimodal was selected
            gbh_register = (gbh_register >> 1) | ((outcome == 't' ? 1 : 0) << (params.N - 1));
        }
    }
    else //update gshare
    {
        bimodal_GshareFunc(outcome,params,"gshare"); 
    }

    //Update the branch chooser counter
    if (bimodal_correct == 1 && gshare_incorrect == 1) // bimodal correct gshare incorrect
    {
        if(hybrid_table[hybrid_set_index.index].counter != 0)
        {
            hybrid_table[hybrid_set_index.index].counter -=1;
        }
    }
    if (bimodal_incorrect == 1 && gshare_correct == 1) // bimodal incorrect gshare correct
    {
        if(hybrid_table[hybrid_set_index.index].counter != 3)
        {
            hybrid_table[hybrid_set_index.index].counter +=1;
        }
    }
}

void bimodal_GshareFunc(char outcome,bp_params params,char* bp_name)
{  
    if(strcmp(bp_name, "bimodal") == 0)  
    {
        if(outcome == 't')
        {
            if (bimodal_table[bimodal_set_index.index].counter< 2)    
            {
                display_data.mispredictions++;
            }           
            if (bimodal_table[bimodal_set_index.index].counter < 3) 
            {
                bimodal_table[bimodal_set_index.index].counter++;
            }  
        }
        else if(outcome == 'n')
        {
            if (bimodal_table[bimodal_set_index.index].counter >= 2)
            {
                display_data.mispredictions++;
            }
            if (bimodal_table[bimodal_set_index.index].counter > 0) 
            {
                bimodal_table[bimodal_set_index.index].counter--;
            } 
        }       
    }
    else if(strcmp(bp_name, "gshare") == 0) 
    {
        if(outcome == 't')
        {
            if (gshare_table[gshare_set_index.index].counter< 2)    
            {
                display_data.mispredictions++;
            }          
            if (gshare_table[gshare_set_index.index].counter < 3) 
            {
                gshare_table[gshare_set_index.index].counter++;
            }
        }
        else if(outcome == 'n')
        {
            if (gshare_table[gshare_set_index.index].counter >= 2)
            {
                display_data.mispredictions++;
            }
            if (gshare_table[gshare_set_index.index].counter > 0) 
            {
                gshare_table[gshare_set_index.index].counter--;
            } 
        }      

        if(params.N !=0)
        {
            // Update the global branch history register
            gbh_register = (gbh_register >> 1) | ((outcome == 't' ? 1 : 0) << (params.N - 1));
        }
    }  
}

void allocatingDynamicMemoryFunc(char* bp_name)
{
    unsigned long int i;
    if(strcmp(bp_name, "bimodal") == 0 ) // Bimodal
    {
        bimodal_table = calloc (bimodal_set_index.set,sizeof(branch_prediction_data));
		// Counter Default Intialized with 2
		for(i=0;i<bimodal_set_index.set;i++)
		{
			bimodal_table[i].counter = 2;
		}    
    }
    else if(strcmp(bp_name, "gshare") == 0 ) // gshare
    {
      //  printf("gshare\n");
        gshare_table = calloc (gshare_set_index.set,sizeof(branch_prediction_data));
		// Counter Default Intialized with 2
		for(i=0;i<gshare_set_index.set;i++)
		{
			gshare_table[i].counter = 2;
		}    
    }
    else if(strcmp(bp_name, "hybrid") == 0 ) // hybrid
    { 
        hybrid_table = calloc (hybrid_set_index.set,sizeof(branch_prediction_data));
		// Counter Default Intialized with 1
		for(i=0;i<hybrid_set_index.set;i++)
		{
			hybrid_table[i].counter = 1;
		}    
    }
}

void display(bp_params params)
{
   int i=0;

   printf("OUTPUT\n");
   printf(" number of predictions:    %ld\n", display_data.predictions);
   printf(" number of mispredictions: %ld\n", display_data.mispredictions);
   printf(" misprediction rate:       %.2f%c\n", ((float)display_data.mispredictions/display_data.predictions)*100, '%');

 
        if(strcmp(params.bp_name, "bimodal") == 0 ) // Bimodal
        {
            printf("FINAL BIMODAL CONTENTS\n");
             
            for(i=0;i<bimodal_set_index.set;i++)
            {
                if (i <= 9)
                    printf(" %d      %ld\n", i, bimodal_table[i].counter);
                else if (i >= 10 && i <= 99)
                    printf(" %d     %ld\n", i, bimodal_table[i].counter);
                else if (i >= 100 && i<=999)
                    printf(" %d    %ld\n", i, bimodal_table[i].counter);
                else if (i >= 1000)
                    printf(" %d   %ld\n", i, bimodal_table[i].counter);
            }
        }
        else if(strcmp(params.bp_name, "gshare") == 0 ) // gshare
        {
            printf("FINAL GSHARE CONTENTS\n");

            for(i=0;i<gshare_set_index.set;i++)
            {
                if (i <= 9)
                    printf(" %d      %ld\n", i, gshare_table[i].counter);
                else if (i >= 10 && i <= 99)
                    printf(" %d     %ld\n", i, gshare_table[i].counter);
                else if (i >= 100 && i<=999)
                    printf(" %d    %ld\n", i, gshare_table[i].counter);
                else if (i >= 1000)
                    printf(" %d   %ld\n", i, gshare_table[i].counter);
            }
        }
        else if(strcmp(params.bp_name, "hybrid") == 0 ) // hybrid
        {
            printf("FINAL CHOOSER CONTENTS\n");
            for (i = 0; i < hybrid_set_index.set; i++)
            {
                if (i <= 9)
                    printf(" %d      %ld\n", i, hybrid_table[i].counter);
                else if (i >= 10 && i <= 99)
                    printf(" %d     %ld\n", i, hybrid_table[i].counter);
                else if (i >= 100 && i<=999)
                    printf(" %d    %ld\n", i, hybrid_table[i].counter);
                else if (i >= 1000)
                    printf(" %d   %ld\n", i, hybrid_table[i].counter);
            }

            printf("FINAL GSHARE CONTENTS\n");

            for(i=0;i<gshare_set_index.set;i++)
            {
                if (i <= 9)
                    printf(" %d      %ld\n", i, gshare_table[i].counter);
                else if (i >= 10 && i <= 99)
                    printf(" %d     %ld\n", i, gshare_table[i].counter);
                else if (i >= 100 && i<=999)
                    printf(" %d    %ld\n", i, gshare_table[i].counter);
                else if (i >= 1000)
                    printf(" %d   %ld\n", i, gshare_table[i].counter);
            }

            printf("FINAL BIMODAL CONTENTS\n");
             
            for(i=0;i<bimodal_set_index.set;i++)
            {
                if (i <= 9)
                    printf(" %d      %ld\n", i, bimodal_table[i].counter);
                else if (i >= 10 && i <= 99)
                    printf(" %d     %ld\n", i, bimodal_table[i].counter);
                else if (i >= 100 && i<=999)
                    printf(" %d    %ld\n", i, bimodal_table[i].counter);
                else if (i >= 1000)
                    printf(" %d   %ld\n", i, bimodal_table[i].counter);
            }
        }
}
