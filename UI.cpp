#include "UI.h"
#define w 320
#define h 240

// Classe screen
String screen::showchoice(std::vector<String> choices) {
    int current = 0;
    bool selected = false;
    M5Canvas menuCanvas(&M5.Display);
    menuCanvas.createSprite(320, 240);
    menuCanvas.setTextDatum(TC_DATUM);
    menuCanvas.setFont(&fonts::efontCN_12);
    menuCanvas.setTextSize(2);
    while (!selected) {
        M5.update();
        menuCanvas.fillSprite(TFT_BLACK);
        menuCanvas.fillTriangle(45, 229, 65, 238, 65, 220, TFT_WHITE);
        menuCanvas.fillTriangle(275, 229, 255, 238, 255, 220, TFT_WHITE);
        menuCanvas.drawLine(w/2 , 238, 170, 220, TFT_WHITE);
        menuCanvas.drawLine(w/2 , 238, 155, 230, TFT_WHITE);
        for (int i = 0; i < choices.size(); i++) {
            int y = i * 24;
            if (i == current) {
                menuCanvas.setTextColor(TFT_BLACK, TFT_WHITE);
            } 
            else {
                menuCanvas.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            menuCanvas.drawString(choices[i], w/2 , y);
        }
        menuCanvas.pushSprite(0, 0);

        if (M5.BtnA.wasPressed()){
            button::feedback();
            current = (current - 1 + choices.size()) % choices.size();
        }
        if (M5.BtnC.wasPressed()){
            button::feedback();
            current = (current + 1) % choices.size();
        }
        if (M5.BtnB.wasPressed()){
            button::feedback();
            selected = true;
        }
        M5.delay(10);
    }
    menuCanvas.deleteSprite();
    M5.Display.clear();
    screen::soustitre(choices[current], 100);
    screen::soustitre("Validé !", 140);
    M5.delay(1500);
    return choices[current];
}

void screen::titre(String txt, int y){
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.setTextSize(4);
    M5.Display.drawString(txt, w/2 , y);
}

void screen::soustitre(String txt, int y){
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.setTextSize(3);
    M5.Display.drawString(txt, w/2 , y);
}

void screen::corps(String txt, int y){
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.setTextSize(2.5);
    M5.Display.drawString(txt, w/2 , y);
}

void screen::statusbar(){
    M5.Display.setTextDatum(TR_DATUM);
    M5.Display.setTextSize(1);
    M5.Display.drawString(String(M5.Power.getBatteryLevel()), 320, 240);
}

void screen::clear(){

}

// Classe button
void button::feedback(){
    M5.Power.setVibration(255);
    M5.delay(100);
    M5.Power.setVibration(0);
}