build:
	g++ -std=c++11 -c dmstring.cpp -o dmlib.o

test:
	g++ -std=c++11 -g -DCATCH_CONFIG_MAIN em406.cpp -o em406test.out
	./em406test.out
