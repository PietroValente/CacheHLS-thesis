# CacheHLS - Broadband

<p align="center">
    <img src="https://i.imgur.com/mPb3Qbd.gif" width="180" alt="Politecnico di Milano"/>
</p>

## Overview

## Usability
1. Open a terminal in the Code folder of the chosen CacheHLS version

2. Compile with the following command:

	g++ main.cpp Client.cpp Global.cpp Cache.cpp Server.cpp -o main -lssl  
	
3. Enter the source information in the config.txt file

4. Run on terminal:

	./main delay_time  	
 or   

	./main delay_time -l (for the debugger version)
	
5. Wait for terminal to finish the countdown and print the output address, ctrl+click to open it on the browser (sometimes you need to click twice because the first opening the browser does not open with the HLS extension)

If the delay_time in seconds is not a multiple of the chunk duration it will be rounded down. For example in the case of rai with chunk duration of 3 seconds, if a delay of 11 is requested, the actual delay will be only 9 seconds.
