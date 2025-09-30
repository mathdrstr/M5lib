#include <M5lib.h>

machine lib;

void setup(){
    lib.setupstd();
    lib.name = "name";
}

void loop(){
    lib.user[0] = "";
    lib.user[1] = "";
    lib.user[2] = "";
    lib.user[3] = "";
    lib.borrow();
}