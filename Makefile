VPATH = lodepng
CC = g++
CFLAG = --std=c++11 -I lodepng

all : diamondsquare

diamondsquare : diamondsquare.o lodepng.o
	g++ -o diamondsquare diamondsquare.o lodepng.o

diamondsquare.o : diamondsquare.cpp
	g++ -c diamondsquare.cpp $(CFLAG)

.PHONY : clean tempRender render
clean : 
	rm diamondsquare diamondsquare.o lodepng.o

tempRender : diamondsquare
	./diamondsquare temp.png
	eog temp.png
	rm temp.png

render : diamondsquare
	./diamondsquare
	eog heightmap.png 

renderTex: diamondsquare
	./diamondsquare heightmap.png t
	eog atexture.png heightmap.png
