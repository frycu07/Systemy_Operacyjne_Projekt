#include <stdio.h>
#include "czas.c"

int test_porownaj_czas(){
    Czas czas3 = {22, 15};
    Czas czas4 = {22, 30};
    int wynik = porownaj_czas(czas3, czas4);
    if(wynik != -1){
        return 1;
    }
    return 0;
  }



  int main(){
    printf("Test porownaj_czas: %s\n", test_porownaj_czas() ? "FAILED" : "PASSED");
    }