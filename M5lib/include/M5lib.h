#include <M5Unified.h>
#include <M5GFX.h>
#include <WiFi.h>
#include "MFRC522_I2C.h"
#include <UNIT_UHF_RFID.h>
#include <HTTPClient.h>
#define ARDUINOJSON_DEFAULT_NESTING_LIMIT 20
#include <ArduinoJson.h>
#include <Wire.h>
#include "UI.h"

extern MFRC522 mfrc522; // Création d'un objet pour gérer le capteur RFID
extern Unit_UHF_RFID uhf;   // Pareil pour le capteur RFID UHF
extern HTTPClient http; // Création d'un objet pour gérer les requêtes d'API
extern JsonDocument doc;    // Création d'un objet pour gérer les fichiers JSON retournés par l'API


class M5lib {   // Classe de base (commune à tous les M5)
public:
    String urfid[200];  // Tableau pour récupérer les EPC scannés par le capteur RFID UHF
    String user[4]; // Tableau pour récupérer les données d'un utilisateur
    String name;  // Nom de l'appareil
    std::vector<const char*> adminkeys = {"************"};
    String sesskey = "*************************";

    void setupstd();    // Setup commun à tous les M5
    JsonObject reducejson(String json); // Réduire la taille des objets JSON de réponse de l'API TimeTonic
    String scancard(); // Scanner l'UID d'un tag RFID
    void getadmin(std::vector<const char*>& adminkeys);  // Récupère la liste des cartes administrateur
    bool isadmin(String card);  // Vérifie si la carte scannée est une carte d'administrateur
    void scanuhf(String *urfid);    // Scanner l'EPC de tags RFID UHF
    virtual int uploadlog(String card, String action, String other) = 0; // Upload des logs dans TimeTonic (dépendant du type)
    virtual int changestatus(int etat) = 0; // Update des status d'emprunt/de présence des personnes/objets (obsolète)
    bool getuser(String* user); // Récupértion du nom/rowid d'un utilisateur avec un scancard()
    void getprojects(std::vector<const char*>& projets);
    void borrow();  // Emprunt/utilisation standardisée de servante/machine
    virtual void incident();  // Gestion différenciée des incidents machine/servante/ordi/matériel
};

class accueil : public M5lib{  // Classe des M5 de type accueil
public:
    std::vector<const char*> motifs = {"Projet personnel", "Stage", "Projet d'UE", "Atelier", "Association",   // Tableau contenant tous les motifs de visite (utilisé dans showchoice() pour la fonction etree())
                        "Projet Pépite", "Projet start-up", "Projet Sorbonne", "Renseignements"};
    std::vector<const char*> projets = {};

    int uploadlog(String card, String action, String other) override;   // Uploadlog spécifique aux M5 de type accueil
    int changestatus(int etat) override;    // Obsolète
    void entree();  // Cycle de gestion de l'arrivée des utilisateurs au FabLab
    void sortie();  // Cycle de gestion du départ des utilisateurs au FabLab
    void M5accueil();  // Fonction unifiée pour l'arrivée/départ des utilisateurs
    void regcard(); // Enregistrement initial de la carte après inscription
};

class servante : public M5lib { // Classe des M5 de type servante
public:
    String rowid;   // Id de la rangée de la servante correspondante dans TimeTonic (voir table d'équivalence)

    int uploadlog(String card, String action, String other) override;   // Uploadlog spécifique aux M5 de type servante
    int changestatus(int etat) override;    // Obsolète
};

class machine : public M5lib {  // Classe des M5 de type machine
public:
    String rowid;   // Id de la rangée de la machine correspondante dans TimeTonic (voir table d'équivalence)

    int uploadlog(String card, String action, String other) override;  // Uploadlog spécifique aux M5 de type machine 
    int changestatus(int etat) override;    // Obsolète
    void incident() override;
};

class ordinateur : public M5lib {   // Classe des M5 de type ordinateur (un seul pour le moment) WIP
public:
    String ordinateur;  // WIP
    String name;    // Nom des M5 (un seul = Ordinateurs pour le moment)

    int uploadlog(String card, String action, String other) override; // WIP
};

class materiel : public M5lib { // Classe des M5 de type matériel WIP
public:
    String outil;   // WIP
    String name;    // Nom des M5 (Matériel x)

    int uploadlog(String card, String action, String other) override;   // WIP
};