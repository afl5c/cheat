# cheat
Command-line tool to cheat in any game

## Usage

Run the executable. (On Mac, you need to run as root.)
Then use any of the following commands:

```
find [part] => find process with [part] in name
fc [value] => find char value
fs [value] => find short value
fi [value] => find int value
reset => reset search
print => print addresses found
wc [value] => write char value
ws [value] => write short value
wi [value] => write int value
```

## Compiling

On Mac: ```g++ main.cpp -lreadline```.
On Windows: ```cl.exe main.cpp```.


