#include "UI.h"
#include <algorithm>
#define w 320
#define h 240

// Classe screen
String screen::showchoice(std::vector<const char*> choices){
    const int line_height = h/10;
    const int visible_count = (h/line_height)-1;
    const int menu_height = visible_count * line_height;
    int current = 0;
    int top = 0;

    M5.Display.clear();
    M5.Display.fillTriangle(45, 238, 65, 238, 55, 220, TFT_WHITE);
    M5.Display.fillTriangle(255, 220, 275, 220, 265, 238, TFT_WHITE);
    M5.Display.drawLine(w/2 , 238, 170, 220, TFT_WHITE);
    M5.Display.drawLine(w/2 , 238, 155, 230, TFT_WHITE);

    M5Canvas menuCanvas(&M5.Display);
    menuCanvas.createSprite(w, menu_height);
    menuCanvas.setTextDatum(TC_DATUM);
    menuCanvas.setFont(&fonts::efontCN_12);
    menuCanvas.setTextSize(2);

    int deldef = 0;
    unsigned long lastNavTime = 0;
    const unsigned long navDelay = 250;

    while (true){
        M5.update();
        menuCanvas.fillSprite(TFT_BLACK);
        for (int i = top; i < top + visible_count; i++){
            if (i >= choices.size()) break;
            menuCanvas.setTextColor(i == current ? TFT_BLACK : TFT_WHITE, i == current ? TFT_WHITE : TFT_BLACK);
            menuCanvas.drawString(choices[i], 160, (i - top) * line_height);
        }
        menuCanvas.pushSprite(0, 0);

        if (M5.BtnB.wasPressed()){
            button::feedback();
            break;
        }
        else if ((M5.BtnA.isHolding() && deldef > 150) || M5.BtnA.wasPressed()){
            button::feedback();
            deldef = 0;
            if (current > 0){
                current--;
            }
            else{
                current = choices.size() - 1;
                top = std::max(0, (int)choices.size() - visible_count);
            }

            if (current < top + 1 && current != 0){
                top = current - 1;
            } 
            else if (current == 0){
                top = 0;
            }
        }
        else if ((M5.BtnC.isHolding() && deldef > 150) || M5.BtnC.wasPressed()){
            button::feedback();
            deldef = 0;
            if (current < choices.size() -1){
                current++;
            }
            else{ 
                current = 0;
                top = 0;
            }
            if (current > top + visible_count - 2 && current != choices.size() - 1){
                top = current - visible_count + 2;
            } 
            else if (current == choices.size() - 1){
                top = std::max(0, (int)choices.size() - visible_count);
            }
        }
        M5.delay(10);
        deldef = deldef+20;
    }
    menuCanvas.deleteSprite();
    M5.Display.clear();
    screen::corps(choices[current], 100);
    screen::soustitre("Validé !", 140);
    M5.delay(2000);
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


// Classe button
void button::feedback(){
    M5.Power.setVibration(255);
    M5.delay(100);
    M5.Power.setVibration(0);
}