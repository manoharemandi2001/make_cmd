//mainfile
#include "myheader.h"

int main()
{
    int arr[SIZE];

    for(int i=0; i<SIZE; i++)
    {
        arr[i]=i+1;
    }

    print(arr);
    sort(arr);
    
    printf("\nAfter sort:\n");
    print(arr);

return 0;
}
