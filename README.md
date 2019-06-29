# SM Logger
The API for SourceMod plugin developers.

## About
The main goal of SM Logger is to make debugging plugins easier and don't choke with unnecessary information. SM Logger split debug messages to channels, each channel has its own name (tag e.g. [MY_TAG_NAME]) and bit. Any number of channels can be output at the same time (turn it on/off) by adding or removing bits via convar. 

## Features
 - Output data where you need (File, server console, chat).
 - Output data you need (Switch debugging channels).
 - Output data at the speed of light! (Faster than LogMessage() approximately in 5 times)

## Note
A lot I/O operation (e.g. log spamming) can lead to server lags and bad gaming experience. SM Logger keeps a log file always open, so it reduce I/O operation and lead to more speed and may solve the lag problem.

## Naming
The log file has the name equivalent to plugin name but lower case letters (spaces ` `  will be replaced by underscores `_`). Let's call that name as **SMLOGGER_NAME**. The log file will be created in **sourcemod/logs** folder. 

The API provide two convars:  
**SMLOGGER_NAME**_log_channel - Outputs bit channel flags (add numbers together): *defined by developer*  
**SMLOGGER_NAME**_log_output - Outputs bit flags (add numbers together): 0=Disable, 1=Logs to file, 2=Prints to server, 4=Prints to chat, 7=all

## Setup

    #include <sm_logger>
    
    char LOG_TAGS[][] =	 {"CORE", "WARNING"}; // <- adds new tag here (channel names)
    
    // Bitwise values definitions
    enum (<<= 1)
    {
    	SML_CORE = 1,
    	SML_WARN,
    	// <- adds new bit here
    }
    
    public void OnPluginStart()
    {
    	SMLoggerInit(LOG_TAGS, sizeof(LOG_TAGS), SML_CORE|SML_EXAMPLE, SML_FILE); // setup logger
    }

Now you can use any of SMLog functions.

## Examples
Code:

    public void OnMapStart()
    {
    	SMLogTag(SML_CORE, "OnMapStart");
    }
    
    public void OnLibraryRemoved(const char[] name)
    {
    	SMLogTag(SML_WARN, "%s lib is removed!", name);
    }
Log file:

    06/29/2019 - 21:23:30 [CORE] OnMapStart
    06/29/2019 - 21:23:30 [WARNING] l4d2 lib is removed!
For more examples see [logger_test](https://github.com/raziEiL/SM-Logger/blob/master/scripting/logger_test.sp "logger_test") plugin.

## Donation
My cat wants a new toy! I try to make quality and beautiful toys for my beloved cat. I create toys in my spare time but sometimes I can step into a tangle and get confused! Oops! It takes time to get out! When the toy is ready I give it to the cat, the GitHub cat and the community can also play with it. So if you enjoy my toys and want to thank me for my work, you can send any amount. All money will be spent on milk! [Donate :feet:](https://www.paypal.me/razicat)
