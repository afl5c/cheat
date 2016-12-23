# cheat
Command-line tool to cheat in any game

## Download

Precompiled binaries:

 - [Mac](./cheat)
 - [Windows](./cheat.exe)

(Use "raw" link to download.)

## Usage

Run the executable. (On Mac, you need to run as root.)
Then use any of the following commands:

```
find [part] => find process with [part] in name
fc [value] => find char value
fs [value] => find short value
fi [value] => find int value
ff [value] => find float value
fd [value] => find double value
reset => reset search
print => print addresses found
wc [value] => write char value
ws [value] => write short value
wi [value] => write int value
wf [value] => write float value
wd [value] => write double value
```

## Compiling

On Mac: ```g++ main.cpp -lreadline```.

On Windows: ```cl.exe main.cpp```.


