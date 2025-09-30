#include "M5lib.h"
#include "logo_fablab.h"

MFRC522 mfrc522(0x28);
Unit_UHF_RFID uhf;
HTTPClient http;
JsonDocument doc;
WiFiClientSecure client;
M5GFX gfx;
LGFX_Button button;


// Classe M5lib
void M5lib::setupstd(){
    M5.begin();
    M5.Display.setBrightness(85);
    M5.Display.fillRect(0, 0, 320, 240, WHITE);
    M5.Lcd.setSwapBytes(true);
    M5.Lcd.pushImage(0, 50, 320, 121, bitmap_logo_fablab);
    Wire.begin();
    Serial.begin(115200);
    mfrc522.PCD_Init();
    M5.Display.setCursor(0, 0);
    M5.Display.setTextSize(2);
    M5.Display.setFont(&fonts::efontCN_12);
    M5.Display.drawString(String(M5.Power.getBatteryLevel()), 280, 0);
    WiFi.begin("ssid", "mdp");
    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        M5.Display.println(".");
    }
    M5.Display.print("Connecté !");
    M5.Display.print(WiFi.localIP());
    getadmin(adminkeys);
    M5.Power.setLed(255);
    M5.delay(1000);
    M5.Display.clear();
}

JsonObject M5lib::reducejson(String json) {
    doc.clear();
    JsonObject result = doc.to<JsonObject>();
    DeserializationError error = deserializeJson(doc, json);
    JsonArray fields = doc["tableValues"]["fields"].as<JsonArray>();
    JsonObject rowInfos = doc["tableValues"]["rowInfos"].as<JsonObject>();
    result["fields"] = fields;
    result["rowInfos"] = rowInfos;
    return result;
}

String M5lib::scancard() {
    String uid = "";
    M5.Display.clear();
    screen::titre("Présentez", 100);
    screen::titre("votre carte", 140);
    while (1) {
        if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
            for (byte i = 0; i < mfrc522.uid.size; i++) {
                if (mfrc522.uid.uidByte[i] < 0x10)
                    uid += "0";
                uid += String(mfrc522.uid.uidByte[i], HEX);
                if (i < mfrc522.uid.size - 1)
                    uid += ":";
            }
            M5.Display.clear();
            screen::titre("Carte scannée", 120);
            M5.delay(1000);
            return uid;
        }
    }
}

void M5lib::getadmin(std::vector<const char*>& adminkeys){
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=getTableValues"
                             "&catId= 590054"
                             "&fieldIds= [7835100]"
                             "&b_o=username"
                             "&o_u=username"
                             "&u_c=username"
                             "&sesskey=" + sesskey);
    if (httpcode == 200){
        String response = http.getString();
        http.end();
        JsonArray values = reducejson(response)["fields"][0]["values"].as<JsonArray>();
        for (JsonVariant val : values){
            if (val["value"].is<const char*>()) {
                adminkeys.push_back(val["value"].as<const char*>());
            }
        }
    }
    else{
        http.end();
        M5.Display.clear();
        screen::soustitre("didn't get", 100);
        screen::soustitre("admin : " + String(httpcode), 140);
        M5.delay(1500);
    }
}

bool M5lib::isadmin(String card){
    for (int i=0; i <  adminkeys.size(); i++){
        if (card == adminkeys[i]){
            return true;
        }
    }
    return false;
}

void M5lib::scanuhf(String *urfid) {
    uhf.begin(&Serial2, 115200, 33, 32, false);
    uhf.setTxPower(2600);
    uint8_t result = uhf.pollingOnce();
    Serial.printf("scan result: %d\r\n", result);
    if (result > 0) {
        for (uint8_t i = 0; i < result; i++) {
            urfid[i] = uhf.cards[i].epc_str;
        }
    } else {
        M5.Display.print("no uhf tag");
    }
}

bool  M5lib::getuser(String* user) {
    user[2] = scancard();
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=getTableValues"
                             "&catId=583482"
                             "&fieldIds=[7721788,7721789]"
                             "&filterRowIds={\"applyViewFilters\": {\"filterGroup\": {\"operator\": \"and\",\"id\": \"tmpId\",\"filters\": [{\"field_id\": 7721790,\"json\": {\"predicate\": \"is\",\"operand\": \"" + user[2] + "\"}}]}}}"
                             "&b_o=username"
                             "&o_u=username"
                             "&u_c=username"
                             "&sesskey=" + sesskey);
    if (httpcode == 200) {
        String response = http.getString();
        http.end();
        JsonArray fields = reducejson(response)["fields"];
        for (JsonObject field : fields) {
            String champ = field["name"];
            if (champ == "Nom")
                user[0] = field["values"][0]["value"].as<String>();
            else if (champ == "Prénom")
                user[1] = field["values"][0]["value"].as<String>();
        }
        for (JsonPair rowId : doc["rowInfos"].as<JsonObject>()) {
            user[3] = rowId.key().c_str();
        }
        if (user[0] == ""){
            M5.Display.clear();
            screen::corps("Utilisateur inconnu", 100);
            screen::titre("Réessayez", 140);
            M5.delay(1500);
            return false;
        }
        else{
            return true;
        }
    }
    else{
        http.end();
        M5.Display.clear();
        screen::titre("error : " + String(httpcode), 120);
        M5.delay(1500);
        return false;
    }
}

void M5lib::getprojects(std::vector<const char*>& projets){
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=getTableValues"
                             "&catId=590165"
                             "&fieldIds=[7836338]"
                             "&filterRowIds={\"applyViewFilters\": {\"filterGroup\": {\"operator\": \"and\",\"id\": \"tmpId\",\"filters\": [{\"field_id\": 7840870,\"json\": {\"predicate\": \"is\",\"operand\": \"" + user[2] + "\"}}]}}}"
                             "&b_o=username"
                             "&o_u=username"
                             "&u_c=username"
                             "&sesskey=" + sesskey);
    if (httpcode == 200) {
        String response = http.getString();
        http.end();
        JsonArray values = reducejson(response)["fields"][0]["values"].as<JsonArray>();
        for (JsonVariant val : values){
            if (val["value"].is<const char*>()) {
                projets.push_back(val["value"].as<const char*>());
            }
        }
    }
    else{
        http.end();
        M5.Display.clear();
        screen::titre("error : " + String(httpcode), 120);
        M5.delay(1500);
    }
}

void M5lib::borrow(){
    String card;
    M5.Display.clear();
    screen::titre(name, 110);
    screen::corps("Emprunter    Incident", 220);
    while(1){
        M5.update();
        if (M5.BtnA.isPressed()){
            break;
        }
        else if (M5.BtnC.isPressed()){
            incident();
            M5.Lcd.clear();
            screen::titre("Machine", 90);
            screen::titre("Hors-service", 130);
            screen::corps("Fin de l'incident", 230);
            while(1){
                M5.update();
                if (M5.BtnB.isPressed()){
                    card = scancard();
                    if (isadmin(card)){
                        M5.Lcd.clear();
                        screen::titre("Fin de", 100);
                        screen::titre("l'incident", 140);
                        M5.delay(2000);
                        return;
                    }
                    else{
                        M5.Lcd.clear();
                        screen::titre("Accès non", 100);
                        screen::titre("autorisé", 140);
                        M5.delay(2000);
                        M5.Lcd.clear();
                        screen::titre("Machine", 90);
                        screen::titre("Hors-service", 130);
                        screen::corps("Fin de l'incident", 230);
                    }
                }
                M5.delay(10);
            }
        }
        M5.delay(10);
    }

    bool isuser = getuser(user);
    if (isuser == true){
        int code = uploadlog(user[2], "Emprunt", "");
        if (code !=200){
            M5.Display.clear();
            screen::titre("error : " + code, 120);
            M5.delay(1500);
            return;
        }
    }
    else{
        return;
    }

    M5.Display.clear();
    screen::soustitre("En cours d'usage", 100);
    screen::soustitre("par " + user[1], 130);
    screen::soustitre("Fin", 220);
    while (true){
        M5.update();
        if (M5.BtnB.isPressed()){
            button::feedback();
            card = scancard();
            if (card == user[2]){
                M5.Display.clear();
                screen::titre("Fin de", 100);
                screen::titre("l'emprunt", 140);
                uploadlog(user[2], "Retour", "");
                M5.delay(1500);
                M5.Display.clear();
                break;
            }
            else if (isadmin(card) == true){
                M5.Display.clear();
                screen::titre("Emprunt", 100);
                screen::titre("arrêté", 140);
                uploadlog(card, "Fin forcée", "");
                M5.delay(2000);
                break;
            }
            else{
                M5.Display.clear();
                screen::corps("Mauvais utilisateur", 100);
                screen::titre("Réessayez", 140);
                M5.delay(1500);
                M5.Display.clear();
                screen::soustitre("En cours d'usage", 100);
                screen::soustitre("par " + user[1], 130);
                screen::soustitre("Fin", 220);
            }
        }
        M5.delay(10);
    }
    return;
}

void M5lib::incident(){ // Existe juste pour héritage, voir classes dérivées pour comportement spécifique
    return;
}

// Classe accueil
int accueil::uploadlog(String card, String action, String other) {
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=createOrUpdateTableRow"
                                "&rowId=tmp"
                                "&catId=560046"
                                "&fieldValues={\"7427748\": \"" + card + "\",\"7837269\": \"" + card +"\",\"7427753\": \"" + name + "\",\"7450020\": \"" + other + "\"}"
                                "&b_o=username"
                                "&o_u=username"
                                "&u_c=username"
                                "&sesskey=" + sesskey);
    http.end();
    return httpcode;
}

int accueil::changestatus(int etat) {
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = 200; // méthode à compléter ?
    http.end();
    return httpcode;
}

void accueil::regcard() {
    user[2] = scancard();
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=getTableValues"
                             "&catId=583482"
                             "&fieldIds=[7721788,7721789]"
                             "&filterRowIds={\"applyViewFilters\": 1217760}"
                             "&maxRows=1"
                             "&b_o=username"
                             "&o_u=username"
                             "&u_c=username"
                             "&sesskey=" + sesskey);
    if (httpcode == 200){
        String response = http.getString();
        JsonArray fields = reducejson(response)["fields"];
        for (JsonObject field : fields){
            String champ = field["name"];
            if (champ == "Nom")
                user[0] = field["values"][0]["value"].as<String>();
            else if (champ == "Prénom")
                user[1] = field["values"][0]["value"].as<String>();
        }
        for (JsonPair rowId : doc["rowInfos"].as<JsonObject>()){
            user[3] = rowId.key().c_str();
        }
    }
    else{
        http.end();
        M5.Display.clear();
        screen::titre("error : " + String(httpcode), 120);
        M5.delay(1500);
        return;
    }
    if (user[0] != ""){
        M5.Display.clear();
        screen::soustitre("Etes-vous bien", 60);
        screen::soustitre(user[1], 100);
        screen::soustitre(user[0], 140);
        screen::soustitre("?", 180);
        M5.Display.setTextSize(2);
        M5.Display.drawString("Non", 55, 220);
        M5.Display.drawString("Oui", 265, 220);
        while (true){
            M5.update();
            if (M5.BtnA.isPressed()){
                button::feedback();
                M5.Display.clear();
                screen::soustitre("Enregistrement de", 100);
                screen::soustitre("la carte annulé", 140);
                M5.delay(1500);
                break;
            }
            else if (M5.BtnC.isPressed()){
                button::feedback();
                int httpcode = http.POST("req=createOrUpdateTableRow"
                                     "&catId=583482"
                                     "&rowId=" + user[3] +
                                     "&fieldValues={\"7721790\": \"" + user[2] + "\"}"
                                     "&b_o=username"
                                     "&o_u=username"
                                     "&u_c=username"
                                     "&sesskey=" + sesskey);
                if (httpcode == 200){
                    http.end();
                    uploadlog(user[2], "Inscription", "Enregistrement carte");
                    M5.Display.clear();
                    screen::titre("Carte", 100);
                    screen::titre("enregistrée", 140);
                }
                else{
                    http.end();
                    M5.Display.clear();
                    screen::soustitre("Erreur lors de", 80);
                    screen::soustitre("l'upload de la carte", 110);
                    screen::titre("Réessayez", 150);
                }
                M5.delay(3000);
                break;
            }
            M5.delay(20);
        }
    }
    else{
        M5.Display.clear();
        screen::titre("Impossible de", 100);
        screen::titre("trouver l'utilisateur", 140);
        M5.delay(2000);
    }
}

void accueil::M5accueil(){
    LGFX_Button depart, arrivee;
    arrivee.initButton(&M5.Lcd, 160, 60, 320, 120, WHITE, BLACK, WHITE, "Arrivée", 5);
    depart.initButton(&M5.Lcd, 160, 180, 320, 120, WHITE, BLACK, WHITE, "Départ", 5);
    M5.Display.clear();
    arrivee.drawButton();
    depart.drawButton();
    while (true){
        M5.update();
        m5::Touch_Class::touch_detail_t touch = M5.Touch.getDetail();
        if (touch.isPressed()){
            if (arrivee.contains(touch.x, touch.y)){
                button::feedback();
                name = "Entrée Prototypage";
                entree();
                break;
            }
            else if (depart.contains(touch.x, touch.y)){
                button::feedback();
                name = "Sortie Prototypage";
                sortie();
                break;
            }
        }
        M5.delay(10);
    }
    return;
}

void accueil::entree(){
    bool isuser = getuser(user);
    if (isuser == false){
        return;
    }
    M5.Display.clear();
    screen::titre("Bonjour", 100);
    screen::titre(user[1], 140);
    M5.delay(1500);
    M5.Display.clear();
    screen::soustitre("Pourquoi venez-", 100);
    screen::soustitre("vous aujourd'hui ?", 130);
    M5.delay(2000);
    String motif = screen::showchoice(motifs);
    String projet = "";
    if (motif == "Projet personnel" || motif == "Projet d'UE"){
        getprojects(projets);
        projet = " : " + screen::showchoice(projets);
    }
    uploadlog(user[2], "Arrivée", motif + projet);
    M5.Display.clear();
    screen::titre("Bienvenu !", 120);
    M5.delay(3000);
}

void accueil::sortie(){
    getuser(user);
    M5.Display.clear();
    screen::titre("Au revoir", 100);
    screen::titre(user[1], 140);
    uploadlog(user[2], "Départ", "");
    M5.delay(2000);
}

// Classe servante
int servante::uploadlog(String card, String action, String other) {
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=createOrUpdateTableRow"
                                "&rowId=tmp"
                                "&catId=562677"
                                "&fieldValues={\"7459877\": \"" + card + "\",\"7459884\": \"" + card + "\",\"7837272\": \"" + name + "\",\"7460301\": \"" + name +"\",\"7459883\": \"" + action + "\"}"
                                "&b_o=username"
                                "&o_u=username"
                                "&u_c=username"
                                "&sesskey=" + sesskey);
    http.end();
    return httpcode;
}

int servante::changestatus(int etat) {
    return 200;
}

// Classe machine
int machine::uploadlog(String card, String action, String other) {
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=createOrUpdateTableRow"
                                "&rowId=tmp"
                                "&catId=562678"
                                "&fieldValues={\"7459890\": \"" + card + "\",\"7459897\": \"" + card + "\",\"7837275\": \"" + name + "\",\"7460895\": \"" + name + "\",\"7459896\": \"" + action + "\"}"
                                "&b_o=username"
                                "&o_u=username"
                                "&u_c=username"
                                "&sesskey=" + sesskey);
    http.end();
    return httpcode;
}

int machine::changestatus(int etat){
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=createOrUpdateTableRow"
                                "&rowId=" + rowid +
                                "&catId=562753"
                                "&fieldValues={\"7460892\": \"" + String(etat) + "\"}"
                                "&b_o=username"
                                "&o_u=username"
                                "&u_c=username"
                                "&sesskey=" + sesskey);
    http.end();
    return httpcode;
}

void machine::incident(){
    user[2] = scancard();
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=createOrUpdateTableRow"
                                "&rowId=tmp"
                                "&catId=567091"
                                "&fieldValues={\"7705235\": \"" + name + "\", \"7705234\": \"" + user[2] + "\"}"
                                "&b_o=username"
                                "&o_u=username"
                                "&u_c=username"
                                "&sesskey=" + sesskey);
    if (httpcode != 200){
        http.end();
        M5.Lcd.clear();
        screen::soustitre("Erreur lors de", 80);
        screen::soustitre("l'upload de l'incident", 110);
        screen::titre("Réessayez", 150);
        M5.delay(3000);
        return;
    }
    httpcode = http.POST("req=getTableValues"
                             "&catId= 567091"
                             "&fieldIds= [7705256]"
                             "&filterRowIds= {\"applyViewFilters\": 1215482}"
                             "&maxRows= 1"
                             "&b_o=username"
                             "&o_u=username"
                             "&u_c=username"
                             "&sesskey=" + sesskey);
    if (httpcode == 200){
        String response = http.getString();
        http.end();
        JsonObject field = reducejson(response)["fields"][0];
        String url = field["values"][0]["value"].as<String>();
        M5.Lcd.clear();
        screen::titre("Scannez le", 60);
        screen::titre("QR code", 100);
        screen::soustitre("Puis appuyez", 175);
        screen::soustitre("sur OK", 210);
        M5.delay(3000);
        M5.Lcd.qrcode(url, 40, 0, 240, 6);
        screen::corps("OK", 225);
        while (!M5.BtnB.isPressed()){
            M5.update();
            M5.delay(10);
        }
        M5.Lcd.clear();
        screen::soustitre("Merci pour votre", 100);
        screen::soustitre("signalement", 140);
        uploadlog(user[2], "Incident", "");
        M5.delay(3000);
        M5.Lcd.clear();
    }
    else{
        http.end();
        M5.Lcd.clear();
        screen::soustitre("Erreur lors de", 80);
        screen::soustitre("la récupération du QR code", 110);
        screen::titre("Réessayez", 150);
        M5.delay(3000);
        return;
    }
}

// Classe ordinateur
int ordinateur::uploadlog(String card, String action, String other) {
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=createOrUpdateTableRow"
                                "&rowId=tmp"
                                "&catId=562679"
                                "&fieldValues={\"7459903\": \"" + card + "\",\"7459905\": \"" + other + "\",\"7459909\": \"" + action + "\"}"
                                "&b_o=username"
                                "&o_u=username"
                                "&u_c=username"
                                "&sesskey=" + sesskey);
    http.end();
    return httpcode;
}

// Classe matériel
int materiel::uploadlog(String card, String action, String other) {
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=createOrUpdateTableRow"
                                "&rowId=tmp"
                                "&catId=562680"
                                "&fieldValues={\"7459916\": \"" + card + "\",\"7460245\": \"" + other + "\",\"7459922\": \"" + action + "\"}"
                                "&b_o=username"
                                "&o_u=username"
                                "&u_c=username"
                                "&sesskey=" + sesskey);
    http.end();
    return httpcode;
}
