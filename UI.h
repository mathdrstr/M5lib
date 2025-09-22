#include <SPIFFS.h>
#include <M5Unified.h>
#include <M5GFX.h>

class screen{   // Classe d'interface utilisateur (commune à tous les M5)
public:
    //static int w;
    //static int h;
    
    static String showchoice(std::vector<String> choices);    // UI de sélection de choix multiples
    static void titre(String txt, int y);   // Affichage de texte en gros centré (taille 4, 13 char max)
    static void soustitre(String txt, int y);   // Affichage de texte en moyen centré (taille 3, 17 char max)
    static void corps(String txt, int y);   // Affichage de texte en  petit centré (taille 2, 26 char max)

    static void statusbar();
    static void clear();
};

class button{
public:
    static void feedback(); // Vibration pour retour utilisateur, à appeler à chaque fois qu'un bouton est pressé
};