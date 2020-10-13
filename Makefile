all:
	g++ -o main *.cpp -I /usr/local/include/SDL2 -I /usr/include/SDL2 -L /usr/local/lib/ -lSDL2

clean:
	rm -f main
