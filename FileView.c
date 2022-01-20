#include <stdio.h>
#define STRSIZE 255


int main () {
   FILE *fp;
   int c;
   int n = 0;
   
   char source_file[STRSIZE];
      
   printf("Enter name of file to View\n");
   scanf("%s",source_file);
   
   /* 
   Para Trabalhar com valores FIXOS use a forma abaixo
   fgets(source_file,STRSIZE,stdin);
   */
   
   /*
   printf(" \n");
   printf("Reading File...");
   printf( "%s",source_file);
   printf("!!!\n");
   */
   
   fp = fopen(source_file,"r");
   if(fp == NULL) {
      perror("Error in opening file");
      return(-1);
   } do {
      c = fgetc(fp);
      if( feof(fp) ) {
         break ;
      }
      printf("%c", c);
   } while(1);

   fclose(fp);
   return(0);
}


