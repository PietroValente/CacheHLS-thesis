# CacheHLS - Broadband

<p align="center">
    <img src="https://i.imgur.com/mPb3Qbd.gif" width="180" alt="Politecnico di Milano"/>
</p>

## Overview
The project involves creating a cache for HLS live streaming files transmitted over broadband, introducing a customizable delay in seconds. This can be useful in various testing scenarios or even in unstable broadband streams, as the cache helps to smooth out the streaming and prevent buffering.

Since some servers already have an internal cache for storing chunks of several hours, there are two versions of the program:

1. **CacheHLS**: This version saves both the chunks and the manifest file in the local cache.
2. **CacheHLS - Manifest Only**: This version, on the other hand, saves only the manifest files in the local cache, assuming there is a server cache from which it can fetch the chunks, as mentioned earlier.

The program is essentially a proxy, developed using multiple threads to make it more efficient. The cache employs a dynamic data structure that retains files for the necessary time and then removes them, ensuring it doesn't consume excessive memory space.

<p align="center">
	<a href="https://github.com/PietroValente/CacheHLS-thesis/blob/main/Images/CacheHLS.png"><img src="https://github.com/PietroValente/CacheHLS-thesis/blob/main/Images/CacheHLS.png" alt="" width="49%"></a>
	<a href="https://github.com/PietroValente/CacheHLS-thesis/blob/main/Images/CacheHLS%20-%20Manifest%20Only.png"><img src="https://github.com/PietroValente/CacheHLS-thesis/blob/main/Images/CacheHLS%20-%20Manifest%20Only.png" alt="" width="49%"></a>
</p>

## Usability
1. Open a terminal in the Code folder of the chosen CacheHLS version

2. Compile with the following command:

	g++ main.cpp Client.cpp Global.cpp Cache.cpp Server.cpp -o main -lssl  
	
3. Enter the source information in the config.txt file

4. Run on terminal:

	./main delay_time  
	or  
	./main delay_time -l (for the debugger version)
	
6. Wait for terminal to finish the countdown and print the output address, ctrl+click to open it on the browser (sometimes you need to click twice because the first opening the browser does not open with the HLS extension)

If the delay_time in seconds is not a multiple of the chunk duration it will be rounded down. For example in the case of rai with chunk duration of 3 seconds, if a delay of 11 is requested, the actual delay will be only 9 seconds.
