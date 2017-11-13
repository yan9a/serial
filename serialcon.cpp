//File: serialcon.cpp
//Description: Serial communication console program for Windows and Linux
//WebSite: http://cool-emerald.blogspot.sg/2017/05/serial-port-programming-in-c-with.html
//MIT License (https://opensource.org/licenses/MIT)
//Copyright (c) 2017 Yan Naing Aye

#include<stdio.h>
#include "Serial.h"
using namespace std;
int main()
{

#if defined (__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)
 Serial com("\\\\.\\COM1",9600,8,'N',1); //Windows
#else
 Serial com("/dev/ttyS0",9600,8,'N',1); //Linux
#endif

 printf("Opening port %s.\n",com.GetPort().c_str());
 if (com.Open() == 0) {
	 printf("OK.\n");
 }
 else {
	 printf("Error.\n");
	 return 1;
 }

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

 printf("Closing port %s.\n",com.GetPort().c_str());
 com.Close();
 return 0;
}
