# serial
Cross-platform Serial port (Com port) programming in C++ with wxWidgets for Windows and Linux

* Serial.h : the serial class library to include
* serialcon.cpp : a simple console example program that uses Serial.h
* wxserial.cpp : a wxWidgets GUI example program

The explanation and examples can be found at

http://cool-emerald.blogspot.sg/2017/05/serial-port-programming-in-c-with.html



To build and run console example, serialcon.cpp, on Linux

 g++ serialcon.cpp Serial.h -o serialcon
 
 sudo ./serialcon



To build and run console example, serialcon.cpp, on Windows

 g++ serialcon.cpp Serial.h -o serialcon.exe -std=c++11
 
 .\serialcon.exe



To build and run wxWidgets example, wxserial.cpp, on Linux

 g++ wxserial.cpp Serial.h  `wx-config --cxxflags --libs` -o wxserial -DNDEBUG
 
 sudo ./wxserial



