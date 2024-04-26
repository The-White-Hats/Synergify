#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header.h"
#define null 0

struct processData
{
    int arrival_time;
    int priority;
    int running_time;
    int id;
};

int main(int argc, char * argv[])
{
    FILE * pFile;
    char file_path[PATH_SIZE];
    getAbsolutePath(file_path, "processes.txt");
    pFile = fopen(file_path, "w");
    int no;
    struct processData pData;
    printf("Please enter the number of processes you want to generate: ");
    scanf("%d", &no);
    srand(time(null));
    //fprintf(pFile,"%d\n",no);
    fprintf(pFile, "#id arrival runtime priority\n");
    pData.arrival_time = 1;
    for (int i = 1 ; i <= no ; i++)
    {
        //generate Data Randomly
        //[min-max] = rand() % (max_number + 1 - minimum_number) + minimum_number
        pData.id = i;
        pData.arrival_time += rand() % (11); //processes arrives in order
        pData.running_time = rand() % (30);
        pData.priority = rand() % (11);
        fprintf(pFile, "%d\t%d\t%d\t%d\n", pData.id, pData.arrival_time, pData.running_time, pData.priority);
    }
    fclose(pFile);
}
