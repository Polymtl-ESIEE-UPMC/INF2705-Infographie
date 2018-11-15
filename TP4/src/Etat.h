#ifndef __ETAT_H__
#define __ETAT_H__

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Singleton.h"

//
// variables d'état
//
class Etat : public Singleton<Etat>
{
    SINGLETON_DECLARATION_CLASSE(Etat);
public:
    // s'assurer que le puits n'a pas été déplacé en dehors des limites de la demi-sphère
    static void verifierPositionPuits()
    {
        // on ne veut pas aller sous le plancher
        if ( posPuits.z < 0.0 ) posPuits.z = 0.0;

        const float deplLimite = 0.9; // on ne veut pas aller trop près de la paroi
        float dist = glm::length( glm::vec3( posPuits.x/bDim.x, posPuits.y/bDim.y, posPuits.z/bDim.z ) );
        if ( dist >= deplLimite ) // on réassigne une nouvelle position
            posPuits = deplLimite * glm::vec3( posPuits.x/dist, posPuits.y/dist, posPuits.z/dist );
    }
    static bool enmouvement;         // le modèle est en mouvement/rotation automatique ou non
    static bool impression;          // on veut une impression des propriétés des premières particules
    static bool afficheAxes;         // indique si on affiche les axes
    static GLenum modePolygone;      // comment afficher les polygones
    static int texnumero;            // numéro de la texture utilisée: 0-aucune, 1-étincelle, 2-oiseau, 3-bonhomme
    static GLuint textureETINCELLE;  // la texture étincelle
    static GLuint textureOISEAU;     // la texture oiseau
    static GLuint textureBONHOMME;   // la texture bonhomme
    static bool texVarie;            // boucler sur les textures automatiquement
    static glm::ivec2 sourisPosPrec; // la position précédente de la souris
    static int modele;               // le modèle à afficher
    static glm::vec3 posPuits;       // position du puits de particules
    static glm::vec3 bDim;           // les dimensions de la bulle en x,y,z
    static int affichageStereo;      // type d'affichage: 0-mono, 1-stéréo anaglyphe, 2-stéréo double
    static float pointsize;          // taille des points (en pixels)
};

#endif
