# build for local:
g++ -std=c++11 olamapper.cpp -o olamapper.out $(pkg-config --cflags --libs libola)
