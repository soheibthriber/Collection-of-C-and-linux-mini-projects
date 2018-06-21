#include <stdio.h>
#include <stdlib.h>

int main () {
   FILE * fp1,*fp2;
    fp1 = fopen ("vec1.txt", "w+");
    fp2 = fopen ("vec2.txt", "w+");
int i=0;
   for(i=0;i<200;i++)
         {              
     fprintf(fp1, "1,2,");  // the values will be 1 than 2 
     fprintf(fp2, "2,1,");  // the values will be 2 than 1       
// this patern will allow us to observe the result when comparing between the two table in order to detect errors 
// the result should be 800.      
    }          
       
   
   
   
   fclose(fp1);
   fclose(fp2);

   return(0);
}

