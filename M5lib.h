#include <M5Unified.h>
#include <M5GFX.h>
#include <WiFi.h>
#include "MFRC522_I2C.h"
#include <UNIT_UHF_RFID.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>

extern MFRC522 mfrc522; // Création d'un objet pour gérer le capteur RFID
extern Unit_UHF_RFID uhf;   // Pareil pour le capteur RFID UHF
extern HTTPClient http; // Création d'un objet pour gérer les requêtes d'API
extern JsonDocument doc;    // Création d'un objet pour gérer les fichiers JSON retournés par l'API

class UI{   // Classe d'interface utilisateur (commune à tous les M5)
public:
    static String showchoice(std::vector<String> choices);    // UI de sélection de choix multiples
    static void titre(String txt, int y);   // Affichage de texte en gros centré (taille 4, 13 char max)
    static void soustitre(String txt, int y);   // Affichage de texte en moyen centré (taille 3, 17 char max)
    static void corps(String txt, int y);   // Affichage de texte en  petit centré (taille 2, 26 char max)
    static void feedback(); // Vibration pour retour utilisateur, à appeler à chaque fois qu'un bouton est pressé
};

class M5lib {   // Classe de base (commune à tous les M5)
public:
    String urfid[200];  // Tableau pour récupérer les EPC scannés par le capteur RFID UHF
    String user[4]; // Tableau pour récupérer les données d'un utilisateur
    String sesskey = "sesskey";

    void setupstd();    // Setup commun à tous les M5
    JsonObject reducejson(String json); // Réduire la taille des objets JSON de réponse de l'API TimeTonic
    String scancard(); // Scanner l'UID d'un tag RFID
    void scanuhf(String *urfid);    // Scanner l'EPC de tags RFID UHF
    virtual int uploadlog(String card, String action, String other) = 0; // Upload des logs dans TimeTonic (dépendant du type)
    virtual int changestatus(int etat) = 0; // Update des status d'emprunt/de présence des personnes/objets (obsolète)
    void getuser(String* user); // Récupértion du nom/rowid d'un utilisateur avec un scancard()
    void getprojects(std::vector<String>& projets);
    void borrow();  // Emprunt/utilisation standardisée de servante/machine
};

class accueil : public M5lib {  // Classe des M5 de type accueil
public:
    std::vector<String> motifs = {"Projet personnel", "Stage", "Projet d'UE", "Atelier", "Association",   // Tableau contenant tous les motifs de visite (utilisé dans showchoice() pour la fonction etree())
                        "Projet Pépite", "Projet start-up", "Projet Sorbonne", "Visite/renseignements"};
    std::vector<String> projets;
    String name;    // Nom du M5 déployé (entrée, sortie ou 1ère inscription)

    int uploadlog(String card, String action, String other) override;   // Uploadlog spécifique aux M5 de type accueil
    int changestatus(int etat) override;    // Obsolète
    void entree();  // Cycle de gestion de l'arrivée des utilisateurs au FabLab
    void sortie();  // Cycle de gestion du départ des utilisateurs au FabLab
    void regcard(); // Enregistrement initial de la carte après inscription
};

class servante : public M5lib { // Classe des M5 de type servante
public:
    String name;    // Nom de la servante (Servante x)
    String rowid;   // Id de la rangée de la servante correspondante dans TimeTonic (voir table d'équivalence)

    int uploadlog(String card, String action, String other) override;   // Uploadlog spécifique aux M5 de type servante
    int changestatus(int etat) override;    // Obsolète
};

class machine : public M5lib {  // Classe des M5 de type machine
public:
    String name;    // Nom de la machine (voir noms utilisés dans la table TimeTonic)
    String rowid;   // Id de la rangée de la machine correspondante dans TimeTonic (voir table d'équivalence)

    int uploadlog(String card, String action, String other) override;  // Uploadlog spécifique aux M5 de type machine 
    int changestatus(int etat) override;    // Obsolète
};

class ordinateur : public M5lib {   // Classe des M5 de type ordinateur (un seul pour le moment)
public:
    String ordinateur;  // WIP
    String name;    // Nom des M5 (un seul = Ordinateurs pour le moment)

    int uploadlog(String card, String action, String other) override; // WIP
};

class materiel : public M5lib { // Classe des M5 de type matériel
public:
    String outil;   // WIP
    String name;    // Nom des M5 (Matériel x)

    int uploadlog(String card, String action, String other) override;   // WIP
};