(c) Manowar / Rolling Pixels 1998

Files contained:

	FF.exe		The example program
	FF.cpp		The source of above program
	fract.cpp	The library source
	fract.h		The library header
	t1.bat		|
	t2.bat		|-> Some example on using FF. The values are taken from the book
	t3.bat		|   "The Beauty of Fractals"
	wc11ff		Bath file needed to compile FF and FRACT


Switch in FF:

Type FF ? for a little help.
This is the default command line:

	FF tM c0 b4 w1024 h768 i1024 m2 l -2.5 1.5 1.5 -1.5

tM: Set the Mandelbrot set. Only this fractal now available.

c0: Set colour mode. Valid number are 0,1,2

b4: The block size. 4 is almost perfect. However con be any postive number.
    Rember that higher value can produce low precision fractals. ('cause the
    programm try to guess to much)

w1024 h768: Screen size

m2: Pass Mode. 2 is the best and fastest. 1 is keep 'cause...well..I don't know
    why i keep it :) Probably will be omitted in other verision. It's less accurate
    and quite often slower.

l -2.5 1.5 1.5 -1.5 : Limit of the gaussian plane (Well: the area to be rendered)
		      Remember the after "l" there must be a space! (I didn't have 
		      time to write a better parser routine...)

