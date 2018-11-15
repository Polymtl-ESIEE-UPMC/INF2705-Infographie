#include "Etat.h"

SINGLETON_DECLARATION_CPP(Etat);

bool        Etat::enmouvement = true;
bool        Etat::impression = false;
bool        Etat::afficheAxes = true;
GLenum      Etat::modePolygone = GL_FILL;
int         Etat::texnumero = 1;
GLuint      Etat::textureETINCELLE = 0;
GLuint      Etat::textureOISEAU = 0;
GLuint      Etat::textureBONHOMME = 0;
bool        Etat::texVarie = false;
glm::ivec2  Etat::sourisPosPrec = glm::ivec2(0);
int         Etat::modele = 1;
glm::vec3   Etat::posPuits = glm::vec3( 0.0, 0.0, 0.0 );
glm::vec3   Etat::bDim = glm::vec3( 2.0, 1.5, 2.2 );
int         Etat::affichageStereo = 0;
float       Etat::pointsize = 5.0;
