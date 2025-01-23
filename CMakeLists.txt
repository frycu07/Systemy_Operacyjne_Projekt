cmake_minimum_required(VERSION 3.30)

project(Projekt_SO)

#set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

# Plik wykonywalny dla "main"
add_executable(main
        main.c
        kolejka.c
)

# Plik wykonywalny dla "rejestracja"
add_executable(rejestracja
        rejestracja.c
        kolejka.c
        czas.c
)

add_executable(pacjent
        pacjent.c
        kolejka.c
        czas.c
)
# Dodanie biblioteki pthread do plików
target_link_libraries(main pthread)
target_link_libraries(rejestracja pthread)
target_link_libraries(pacjent pthread)