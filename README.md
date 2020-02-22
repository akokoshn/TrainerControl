# Read data and control a bike trainer

This is a server application allowing to read data from ANT+ devices and
sending them over to a TCP connection.  It can currently read heart rate from
an ANT+ HRM and read power, speed and cadence from an ANT+ FE-C trainer (most
recent trainers support this).  It can also control the resistance of the
trainer by setting the slope.

## Dependencies

lubusb - https://github.com/libusb/libusb

## Building the application

The application is built on a Windows platform using Visual Studio 2017 (the
Community Edition will work fine).  It could be easily ported to Linux as
well, but this hasn't been done yet.

1. Get TrainerControl and libusb repo and format the workspace folder as below
```
<workspace>
    |- libusb
    |- TrainerControl
```
2. Build libusb as static library for required configuration (Release/Debug + Win32/x64): open <workspace>\libusdb\msvc\libusb_2017.sln
3. Build TrainerControl via <workspace>\TrainerControl\vs2017\TrainerControl.sln

## SetUp environment

To connet with Ant device need to install libusbK driver.
You can use Zadig (https://zadig.akeo.ie/) application: select your Ant device and libusbK driver.

## Running the application

To run the application, open a command window and type:

    ./TrainerControl.exe
    
The application will try to find the ANT+ USB stick and connect to the heart
rate monitor and bike trainer.  It will also accept TCP connections on port
7500.
