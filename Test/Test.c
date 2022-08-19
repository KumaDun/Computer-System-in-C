#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main () {
   char* queries = "GET /HTTP/1.0\r\nHost:www.baidu.com\r\n\r\n";
   fputs(queries,stdout);
   fflush(stdout);

   /*int x = 18853;
   char str[20];
   sprintf(str,"%d\n",x);
   printf("length is %d", strlen(str));
   printf("%s",str);*/
   
   /*int a;
   char str[50];
   void increment(char* list[],int len);

   char* ptrArrayArray[2][2];
   ptrArrayArray[0][0] = malloc(3*sizeof(char));
   ptrArrayArray[0][1] = malloc(3*sizeof(char));
   ptrArrayArray[1][0] = malloc(3*sizeof(char));
   ptrArrayArray[1][1] = malloc(3*sizeof(char));

   increment(ptrArrayArray[0],2);
   for(int i=0;i<2;i++) {
      printf("ptrArray 0 at %d is %s \n",i, ptrArrayArray[0][i]);
   }*/
   
   /*
   strcpy(ptrArrayArray[0][0],"Im");
   printf("string is %s \n", ptrArrayArray[0][0]);
   free(ptrArrayArray[0][0]);
   */
	 
   /*
   printf("Enter an integer value: ");
   scanf("%d", &a);
   assert(a >= 10);
   printf("Integer entered is %d\n", a);
    
   printf("Enter string: ");
   scanf("%s", str);
   assert(str != NULL);
   printf("String entered is: %s\n", str);

   int length = strlen("1234\n");
   printf("%d\n",length); 
   */


   return(0);
}

void increment (char* list [], int len) {
   for (int i =0;i<len;i++) {
      list[i] = malloc(3*sizeof(char));
      strcpy(list[i],"ab");
   }
}
