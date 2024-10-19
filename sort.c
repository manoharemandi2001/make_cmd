//sort array function
#include "myheader.h"
//hllo
void sort(int* arr)
{
    int temp;
    for(int i=0; i<(SIZE-1); i++)
    {
        for(int j=0; j<(SIZE-i-1); j++)
        {
            temp=arr[j];
            arr[j]=arr[j+1];
            arr[j+1]=temp;
        }
    }
}
