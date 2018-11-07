# Using Serial Port with C++

Cross-platform Serial port (Com port) programming in C++ with wxWidgets for Windows and Linux


* ceSerial.h : the header of the serial class library to include
* ceSerial.cpp : the implementation of the serial class library
* conserial.cpp : a simple console example program that uses 'ceSerial' class library
* wxserial.cpp : a wxWidgets GUI example program using serial port


The explanation and examples can be found at


[http://cool-emerald.blogspot.sg/2017/05/serial-port-programming-in-c-with.html](http://cool-emerald.blogspot.sg/2017/05/serial-port-programming-in-c-with.html)



To build and run console example, conserial.cpp, on Linux

```
 $ g++ conserial.cpp ceSerial.cpp -o conserial -std=c++11
 
 $ sudo ./conserial
```


To build and run console example, conserial.cpp, on Windows

```
 g++ conserial.cpp ceSerial.cpp -o conserial.exe -std=c++11
 
 .\conserial.exe

```


To build and run wxWidgets example, wxserial.cpp, on Linux

```
 $ g++ wxserial.cpp ceSerial.cpp  `wx-config --cxxflags --libs` -o wxserial -std=c++11 -DNDEBUG
 
 $ sudo ./wxserial

```



