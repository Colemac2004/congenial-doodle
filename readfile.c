#include <stdio.h>
#include <stdlib.h>

int main(){
     FILE *file;
     int* buff;
     char filename[]="filename.txt";
     char buffer[256];
     int character;

     file=fopen(filename,"r");
     if (file==NULL){
        perror("Error Opening File");
        return 1;
     }
     //print by string
     while (fgets(buffer,sizeof(buffer),file)!=NULL){
        printf("%s",buffer);
     }
     //rewind to beginning
     rewind(file);
     //print by char
     while ((character=fgetc(file))!=EOF){
        printf("%c",character);
     }
}