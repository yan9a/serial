#include<stdio.h>
#include "serial.h"
using namespace std;
int main()
{
 Serial com(3,9600);


 printf("Opening port %ld.\n",com.GetPortNumber());
 long r = com.Open();//return 0 if success

 bool successFlag;
 printf("Writing.\n");
 char s[]="Hello";
 successFlag=com.Write(s);//write string
 successFlag=com.WriteChar('!');//write a character

 printf("Waiting 3 seconds.\n");
 delay(3000);//delay 5 sec to wait for a character

 printf("Reading.\n");
 char c=com.ReadChar(successFlag);//read a char
 if(successFlag) printf("Rx: %c\n",c);

printf("Closing port %ld.\n",com.GetPortNumber());
 com.Close();
return 0;
}
