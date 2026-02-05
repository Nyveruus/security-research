# How it works:

# Architecture

This program takes advantage of the fact that everything is a file in Linux. To elaborate, it takes advantage of the keyboard event file which is a character device node located in /dev/input. /dev/input hosts a myriad of different event files, thus /dev/input/by-id can be used to find the symlink to the correct event file, once it's found with root priveleges, access to the stream of all keyboard events is gained.

Because the event file works directly with the kernel, system functions need to be used to be able to properly parse the stream.

The program begins with opening the file descriptor for the event file, which later will be used in an infinite for loop for reading into an input_event struct defined in <linux/input.h>. The input_event struct keeps track of the fields: type, value and code which are defined in <linux/input-event-codes.h>. Documentation can be found in man pages, https://www.kernel.org/doc/Documentation/input/event-codes.rst and https://www.kernel.org/doc/html/v4.14/input/event-codes.html

Afterwards, the main keyboard keylogger boils down to filtering to EV_KEY types - "used to describe state changes of keyboards, buttons, or other key-like devices...." - using conditional logic against the "code" field in the struct and printing the char that it relates to. Originally this consisted of using many if-else statements and a very large switch, but so as to avoid repeating code, the keys - both lower and upper - are directly related to their codes in a struct array called "keymap" which is used for indexing. The program keeps track of capslock and shift using boolean values torwards the beginning of the for loop; the boolean values are later used in a XOR operation to decide whether a char should be printed as uppercase or lowercase.

The second logical half of the program consists opening a socket file descriptor and connecting it to an IP address and port over TCP to send key presses to. To be able to connect it to the IP address and port, a sockaddr struct needs to be passed to connect() which is done via casting a sockaddr_in struct that holds the family, port and address. Before assigning to the fields in the sockaddr_in struct, it needs to be zeroed out to avoid undefined behavior from the padding when casted as sockaddr.

If connection succeeds, the keylogger sends all key events that are recorded thereafter over the socket. If it doesn't recognize a key, it will output the key code.

# Prevention:
