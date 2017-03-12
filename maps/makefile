compile: mapDrawer.cpp
	g++ mapDrawer.cpp `libpng-config --ldflags` -o convert

makeMap: convert map.png layer1 layer2 layer3
	./convert map.png layer1 layer2 layer3

run: compile makeMap