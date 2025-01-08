cmake_minimum_required(VERSION 3.30)
project(Projekt_SO)

set(CMAKE_CXX_STANDARD 20)

add_executable(Projekt_SO main.c pacjent.c rejestracja.c lekarz.c semafor.c)
target_link_libraries(Projekt_SO pthread)