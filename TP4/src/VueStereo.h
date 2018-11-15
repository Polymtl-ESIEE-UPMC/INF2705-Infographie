#ifndef __VUESTEREO_H__
#define __VUESTEREO_H__

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Singleton.h"

//
// variables pour la vue stéréo
//
class VueStereo : public Singleton<VueStereo>
{
    SINGLETON_DECLARATION_CLASSE(VueStereo);
public:
    static GLdouble dip;       // la distance interpupillaire
    static GLdouble factzoom;  // le facteur de zoom
    static GLdouble zavant;    // la position du plan avant du volume de visualisation
    static GLdouble zarriere;  // la position du plan arrière du volume de visualisation
    static GLdouble zecran;    // la position du plan de l'écran: les objets affichés en avant de ce plan «sortiront» de l'écran
};

#endif
