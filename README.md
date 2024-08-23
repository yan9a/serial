# Simple to use serial port library in C++

Cross-platform Serial port (Com port) C++ library for Windows, Mac and Linux.
Just need to include a single header file 'ceSerial.h' to use it.


* ceSerial.h : the implementation of the serial class library in a single header file
* test_console.cpp : a simple console example program 
* test_gui.cpp : a wxWidgets GUI example program using serial port


The explanation and examples can be found at


[http://cool-emerald.blogspot.sg/2017/05/serial-port-programming-in-c-with.html](http://cool-emerald.blogspot.sg/2017/05/serial-port-programming-in-c-with.html)



To build and run console example, test_console.cpp, on Linux

```
 $ g++ test_console.cpp -o test_console -std=c++11

 $ sudo ./test_console
```


To build and run console example, test_console.cpp, on Windows

```
 g++ test_console.cpp -o test_console.exe -std=c++11
 
 .\test_console.exe

```


To build and run wxWidgets example, test_gui.cpp, on Linux

```
 $ cd serialgui
 
 $ g++ test_gui.cpp test_frame.cpp `wx-config --cxxflags --libs` -o test_gui -std=c++11 -I..
 
 $ sudo ./test_gui

```



