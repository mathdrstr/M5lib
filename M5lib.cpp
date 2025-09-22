#include "M5lib.h"

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
    Wire.begin();
    Serial.begin(115200);
    mfrc522.PCD_Init();
    M5.Display.setCursor(0, 0);
    M5.Display.setTextSize(2);
    M5.Display.setFont(&fonts::efontCN_12);
    WiFi.begin("SSID", "password");
    client.setInsecure();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Display.println(".");
    }
    M5.Display.print("Connecté !");
    M5.Display.print(WiFi.localIP());
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

void M5lib::getadmin(String *adminkeys){
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=getTableValues"
                             "&catId= à modifier"
                             "&fieldIds= à modifier"
                             "&filterRowIds= à modifier"
                             "&b_o=username"
                             "&o_u=username"
                             "&u_c=username"
                             "&sesskey=" + sesskey);
    if (httpcode == 200) {
        String response = http.getString();
        http.end();
        JsonArray fields = reducejson(response)["fields"];
        // à complêter
    }
    else{
        http.end();
        M5.Display.clear();
        screen::titre("error : " + String(httpcode), 120);
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

void M5lib::getuser(String* user) {
    user[2] = scancard();
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=getTableValues"
                             "&catId=562424"
                             "&fieldIds=[7457220,7457221]"
                             "&filterRowIds={\"applyViewFilters\": {\"filterGroup\": {\"operator\": \"and\",\"id\": \"tmpId\",\"filters\": [{\"field_id\": 7457222,\"json\": {\"predicate\": \"is\",\"operand\": \"" + user[2] + "\"}}]}}}"
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
    }
    else{
        http.end();
        M5.Display.clear();
        screen::titre("error : " + String(httpcode), 120);
        M5.delay(1500);
    }
}

void M5lib::getprojects(std::vector<String>& projets){ // WIP
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=getTableValues"
                             "&catId=562424"
                             "&fieldIds=[7457220,7457221]"
                             "&filterRowIds={\"applyViewFilters\": {\"filterGroup\": {\"operator\": \"and\",\"id\": \"tmpId\",\"filters\": [{\"field_id\": 7457222,\"json\": {\"predicate\": \"is\",\"operand\": \"" + user[2] + "\"}}]}}}"
                             "&b_o=username"
                             "&o_u=username"
                             "&u_c=username"
                             "&sesskey=" + sesskey);
    if (httpcode == 200) {
        String response = http.getString();
        http.end();
        JsonArray fields = reducejson(response)["fields"];
        for (JsonObject field : fields) {
            // Parsage projets
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
    bool emprunt = false;
    bool admin = false;
    String card;
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
    getuser(user);
    if (user[0] != ""){
        int code = uploadlog(user[2], "Emprunt", "");
        if (code !=200){
            M5.Display.clear();
            screen::titre("error : " + code, 120);
            M5.delay(1500);
            return;
        }
        changestatus(1);
        emprunt = true;
    }
    else{
        M5.Display.clear();
        screen::corps("Utilisateur inconnu", 100);
        screen::titre("Réessayez", 140);
        M5.delay(1500);
        return;
    }
    M5.Display.clear();
    screen::soustitre("En cours d'usage", 100);
    screen::soustitre("par " + user[1], 130);
    screen::soustitre("Fin", 220);
    while (emprunt = true){
        M5.update();
        if (M5.BtnB.isPressed()){
            button::feedback();
            card = scancard();
            admin = isadmin(card);
            if (card == user[2]){
                M5.Display.clear();
                screen::titre("Fin de", 100);
                screen::titre("l'emprunt", 140);
                uploadlog(user[2], "Retour", "");
                changestatus(0);
                M5.delay(1500);
                emprunt = false;
                M5.Display.clear();
                break;
            }
            else if (admin == true){
                M5.Display.clear();
                screen::titre("Emprunt", 100);
                screen::titre("arrêté", 140);
                uploadlog(card, "Fin forcée", "");
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
                break;
            }
        }
        M5.delay(20);
    }
    return;
}

void M5lib::incident(){
    return;
}

// Classe accueil
int accueil::uploadlog(String card, String action, String other) {
    http.begin("https://timetonic.com/live/api.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpcode = http.POST("req=createOrUpdateTableRow"
                                "&rowId=tmp"
                                "&catId=560046"
                                "&fieldValues={\"7427748\": \"" + card + "\",\"7457231\": \"" + card +"\",\"7427753\": \"" + name + "\",\"7450020\": \"" + other + "\"}"
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
    getuser(user);
    if (user[0] != ""){
        M5.Display.clear();
        screen::soustitre("Etes-vous bien", 60);
        screen::soustitre(user[1], 100);
        screen::soustitre(user[0], 140);
        screen::soustitre("?", 180);
        M5.Display.setTextSize(2);
        M5.Display.drawString("Non", 55, 220);
        M5.Display.drawString("Oui", 265, 220);

        while (1) {
            M5.update();
            if (M5.BtnA.isPressed()) {
                button::feedback();
                M5.Display.clear();
                screen::soustitre("Enregistrement de", 100);
                screen::soustitre("la carte annulé", 140);
                M5.delay(1500);
                break;
            }
            else if (M5.BtnC.isPressed()) {
                button::feedback();
                int httpcode = http.POST("req=createOrUpdateTableRow"
                                     "&rowId=" + user[3] +
                                     "&catId=562424"
                                     "&fieldValues={\"7457222\": \"" + user[2] + "\"}"
                                     "&b_o=username"
                                     "&o_u=username"
                                     "&u_c=username"
                                     "&sesskey=" + sesskey);
                if (httpcode == 200) {
                    uploadlog(user[2], "Inscription", "");
                    M5.Display.clear();
                    screen::titre("Carte", 100);
                    screen::titre("enregistrée", 140);
                } 
                else {
                    M5.Display.clear();
                    screen::soustitre("Erreur lors de", 80);
                    screen::soustitre("l'upload de la carte", 110);
                    screen::titre("Réessayez", 150);
                }
                M5.delay(1500);
                break;
            }
            M5.delay(20);
        }
    } 
    else {
        M5.Display.clear();
        screen::titre("Impossible de", 100);
        screen::titre("trouver l'utilisateur", 130);
        M5.delay(1500);
    }
}

void accueil::entree(){
    getuser(user);
    M5.Display.clear();
    screen::titre("Bonjour", 100);
    screen::titre(user[1], 140);
    M5.delay(1500);
    M5.Display.clear();
    screen::soustitre("Pourquoi venez-", 100);
    screen::soustitre("vous aujourd'hui ?", 130);
    M5.delay(2000);
    String motif = screen::showchoice(motifs);
    if (motif == "Projet"){
        getprojects(projets);
        motif = screen::showchoice(projets);
    }
    uploadlog(user[2], "Arrivée", motif);
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
                                "&fieldValues={\"7459877\": \"" + card + "\",\"7459884\": \"" + card + "\",\"7459879\": \"" + name + "\",\"7460301\": \"" + name +"\",\"7459883\": \"" + action + "\"}"
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
                                "&fieldValues={\"7459890\": \"" + card + "\",\"7459897\": \"" + card + "\",\"7459892\": \"" + name + "\",\"7460895\": \"" + name + "\",\"7459896\": \"" + action + "\"}"
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

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
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
        screen::titre("Scannez le", 100);
        screen::titre("QR code", 140);
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
        M5.delay(3000);
        M5.Lcd.clear();
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
