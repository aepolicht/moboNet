moboNet
=======

  This repo is a collection of elements contributing to one home automation project. It consists of analysis, schematics,
and software.

Notes
=====

Version 0.0.1
	Added Features:
		-Recall previous state from memory using EEPROM

	Bug Fixes:
		-Remove Flashes when receiving message

Version 0.0.2
	Added Features:
		-All communication now shall be over UDP packets

Version 0.0.3
	Changes:
		-Added notifications features to all devices
		-Added basic music detection and visualization code
			-Changes were also made to the hardware side of the music detection.
			 The low pass filter desired seems to have a cutoff right at 220 Hz.
		 	 Ideally this simple filter will consist of a 1 uF capacitor and a
		 	 7.5 kOhm resistor.
		-Implemented serial connection to bulb array to communicate with display 
		 projection from PC. Currently it checks serial, then homenet for any input
		 but if this proves to slow down the projection too much i may need a blocking
		 version where when serial is inbound all other code is ignored.
		-Changed to "FastSPI" library for communication with WS2801 LED strings
			-Also tried out an Adafruit library for this purpose but i found that
			 that library was blocking and took significantly more time to run
			 than the FastSPI library took.
			-FastSPI is non-blocking and takes around 300 uS to finish its write
		     otherwise overwriting while in progress produces undesirable effects.
	    -Implemented notifications for door knock however the whole system needs an
	     overhaul to simplify the process of [alarm device]->[base station]->[bulb clients]
	    -Implemented code for the door knock detector notifier.
	    
Version 0.0.4
	Note:
		I have begun to implement the music detection system. The basis for the whole system
		is contingent upon the frequency response of the low pass filter. I have chosen a simple
		filter with passband edge around 300 Hz. In this design a large gap between the passband 
		and the stop-band is desired because of how i plan to implement the software side of the
		music detection. Here is a rough graph of the frequency response:

			                    |H(w)| for LPF (1st order fc = 300Hz)
					  (dB)
					 0	|x x x x x x x x x x x 
					 	|                      x 
					-5	|                        x 
						|                          x
					-10	|                            x
						|                              x              
					-15	|                                x               
						|                                  x         
					-20	|                                    x           
						|                                      x         
					-25	|                                        x            
						|                                          x              
					-30	|                                            x                  
						|                                              x      
					-35	|                                                x      
						-------|----------|-----------|------------|----- Hz (log)
						0     10         100        1000         10000   

		The basis for the software side is having an understanding of the filters response
		and using this to determine whether or not the input signal is a Low, Mid or High.
		I am doing it this way to avoid any additional circuitry because i want a small
		package. It is easiest to detect the beat and very low frequencies because these are
		the largest in magnitude. I want a large gap between the passband and stop-band edges
		because i want to be able to detect other frequencies at attenuated magnitudes.
		To overcome the problem of detecting loud mids as a bass and other situations, i take
		an average of the last 64 samples as a reference. Given that reference the respective
		frequencies will then be function of the average unique to those frequency bands. To
		obtain a reference i sampled the desired frequency at different volumes IE different 
		values of the variable "ave". I then applied best fit lines and use them as cutoffs
		to determine which band a sample should be classified to. The best fits i have found
		are:
 
			Lows  (0-230Hz)   Threshold = 67*log(ave-1)-10
			Mids  (230-800Hz) Threshold = 25*log(ave-1)-5
			Highs (800+Hz)    Threshold = 12*log(ave-1)-5

		It is much more difficult to detect a high with this filter because signals higher than 
		800 Hz are attenuated more severely beginning with 800 Hz at -9 dB. The other method for
		doing this type of thing would be with a ~6000 Hz lpf and performing an FFT which i will
		do to see if its viable given the constraints.

		My preliminary DFT implementation with a 128 sample input and no special windowing takes around 2.5 seconds per DFT which is much too long. When N is 32, it takes about 155 mS.
		This would mean a sampling rate of T > 155 mS which a sampling frequency of less than
		6 Hz which is pretty much useless. The to attain a Nyquist frequency of a signal of 2000 Hz
		i need to have a sampling frequency of 4000 Hz which means the DFT finish in <250 uS. I 
		have found an FFT library for Arduino but it takes more than 600 uS to complete the FFT.
		This means i will continue with my previous method of implementing the lump spectral
		analysis.

TBD:
x-Add days support to android
x-Redesign notifications so that they are handled by base station
o-Setup music mode on all devices
x-Setup Door knock codes for base station and knocker
o-Setup Ambiance mode
o-Fix Android save and return functionality
o-Finish C# desktop application
o-Add notification options to base station and android options menu
o-Implement Groups on all devices