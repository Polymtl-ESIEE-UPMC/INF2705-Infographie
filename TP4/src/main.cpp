// Prénoms, noms et matricule des membres de l'équipe:
// - Prénom1 NOM1 (matricule1)
// - Prénom2 NOM2 (matricule2)
#warning "Écrire les prénoms, noms et matricule des membres de l'équipe dans le fichier et commenter cette ligne"

#include <stdlib.h>
#include <iostream>
#include "inf2705-matrice.h"
#include "inf2705-nuanceur.h"
#include "inf2705-fenetre.h"
#include "inf2705-texture.h"
#include "inf2705-forme.h"
#include "Etat.h"
#include "VueStereo.h"

// variables pour l'utilisation des nuanceurs
GLuint prog;      // votre programme de nuanceurs
GLint locVertex = -1;
GLint locColor = -1;
GLint locvitesse = -1;
GLint loctempsRestant = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;
GLint locpointsize = -1;
GLint loclaTexture = -1;
GLint loctexnumero = -1;
GLuint progRetroaction;  // votre programme de nuanceurs pour la rétroaction
GLint locpositionRetroaction = -1;
GLint loccouleurRetroaction = -1;
GLint locvitesseRetroaction = -1;
GLint loctempsRestantRetroaction = -1;
GLint loctempsRetroaction = -1;
GLint locdtRetroaction = -1;
GLint locgraviteRetroaction = -1;
GLint loctempsMaxRetroaction = -1;
GLint locposPuitsRetroaction = -1;
GLint locbDimRetroaction = -1;
GLuint progBase;  // le programme de nuanceurs de base
GLint locColorBase = -1;
GLint locmatrModelBase = -1;
GLint locmatrVisuBase = -1;
GLint locmatrProjBase = -1;

GLuint vao[2];
GLuint vbo[2];
GLuint tfo[1];
GLuint requete;

// matrices du pipeline graphique
MatricePipeline matrModel, matrVisu, matrProj;

// les formes
FormeSphere *demisphere = NULL;
FormeDisque *disque = NULL;

//
// les particules
//
class Particule
{
public:
    GLfloat position[3];      // en unités
    GLfloat couleur[4];       // couleur actuelle de la particule
    GLfloat vitesse[3];       // en unités/seconde
    GLfloat tempsRestant;     // temps de vie restant en secondes
    // (vous pouvez ajouter d'autres éléments, mais il faudra les prévoir dans les varyings)
};
const unsigned int MAXNPARTICULES = 1000000;
Particule part[MAXNPARTICULES];   // le tableau de particules

class Parametre
{
public:
    unsigned int nparticules; // nombre de particules utilisées (actuellement affichées)
    float tempsMax;           // temps de vie maximal (en secondes)
    float temps;              // le temps courant dans la simulation (en secondes)
    float dt;                 // intervalle entre chaque affichage (en secondes)
    float gravite;            // gravité utilisée dans le calcul de la position de la particule
} parametre = { 400, 10.0, 0.0, 1.0/60.0, 0.3 };

//
// variables pour définir le point de vue
//
class Camera
{
public:
    void definir()
    {
        glm::vec3 ptVise = glm::vec3( 0.0, 0.0, 0.5*Etat::bDim.z ); // un point au milieu du modèle
        matrVisu.LookAt( ptVise.x + dist*cos(glm::radians(theta))*sin(glm::radians(phi)),
                         ptVise.y + dist*sin(glm::radians(theta))*sin(glm::radians(phi)),
                         ptVise.z + dist*cos(glm::radians(phi)),
                         ptVise.x, ptVise.y, ptVise.z,
                         0.0, 0.0, 1.0 );
    }
    void verifierAngles() // vérifier que les angles ne débordent pas les valeurs permises
    {
        if ( theta > 360.0 ) theta -= 360.0; else if ( theta < 0.0 ) theta += 360.0;
        const GLdouble MINPHI = 0.01, MAXPHI = 180.0 - 0.01;
        phi = glm::clamp( phi, MINPHI, MAXPHI );
    }
    double theta;         // angle de rotation de la caméra (coord. sphériques)
    double phi;           // angle de rotation de la caméra (coord. sphériques)
    double dist;          // distance (coord. sphériques)
} camera = { 270.0, 80.0, 6.0 };


void calculerPhysique( )
{
    if ( Etat::enmouvement )
    {
        // À MODIFIER (partie 1)
        // déplacer les particules en utilisant le nuanceur de rétroaction
        glUseProgram( progRetroaction );
        // ... (MODIFIER)
        // glUniform...
        // glBindBufferBase(...);


        // débuter la requête (si impression)
        if ( Etat::impression )
            glBeginQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, requete );

        // « dessiner »
        // ... (MODIFIER)
        // glDrawArrays( GL_POINTS, ... );

        // terminer la requête (si impression)
        if ( Etat::impression )
            glEndQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN );

        glBindVertexArray( 0 );              // désélectionner le VAO

        if ( Etat::impression )
        {
            glFlush(); // attendre que la carte graphique ait terminé le traitement
            // obtenir et imprimer les résultats
            GLuint nresul;
            glGetQueryObjectuiv( requete, GL_QUERY_RESULT, &nresul );
            std::cout << " Nombre total de particules (=nresul)=" << nresul << std::endl;

            if ( nresul )
            {
                const int NRETOUR = 10;
                Particule Retour[NRETOUR];
                if ( nresul > NRETOUR ) nresul = NRETOUR; // on veut seulement les NRETOUR premières particules
                glGetBufferSubData( GL_TRANSFORM_FEEDBACK_BUFFER, 0, nresul*sizeof(Particule), Retour );

                for ( unsigned int i = 0; i < nresul; ++i )
                    std::cout << "   part["<<i<<"]"
                              << " .position[]="
                              << " " << Retour[i].position[0]
                              << " " << Retour[i].position[1]
                              << " " << Retour[i].position[2]
                              << " .couleur[]="
                              << " " << Retour[i].couleur[0]
                              << " " << Retour[i].couleur[1]
                              << " " << Retour[i].couleur[2]
                              << " " << Retour[i].couleur[3]
                              << " .vitesse[]="
                              << " " << Retour[i].vitesse[0]
                              << " " << Retour[i].vitesse[1]
                              << " " << Retour[i].vitesse[2]
                              << " .tempsRestant[]="
                              << " " << Retour[i].tempsRestant
                              << std::endl;
            }

            Etat::impression = false;
        }

        // échanger les numéros des deux VBO
        std::swap( vbo[0], vbo[1] );

        // avancer le temps
        parametre.temps += parametre.dt;

        // faire varier la texture utilisée
        if ( Etat::texVarie )
            if ( fmod( parametre.temps, 5 ) <= parametre.dt )
                if ( ++Etat::texnumero > 3 ) Etat::texnumero = 1;

#if 0
        // faire varier la taille de la boite
        // static int sensX = 1;
        // Etat::bDim.x += sensX * 0.001;
        // if ( Etat::bDim.x < 1.7 ) sensX = +1;
        // else if ( Etat::bDim.x > 2.3 ) sensX = -1;

        // static int sensY = 1;
        // Etat::bDim.y += sensY * 0.0005;
        // if ( Etat::bDim.y < 1.7 ) sensY = +1;
        // else if ( Etat::bDim.y > 2.3 ) sensY = -1;

        static int sensZ = 1;
        Etat::bDim.z += sensZ * 0.001;
        if ( Etat::bDim.z < 1.0 ) sensZ = +1;
        else if ( Etat::bDim.z > 3.0 ) sensZ = -1;

        Etat::verifierPositionPuits();
#endif

        FenetreTP::VerifierErreurGL("calculerPhysique");
    }
}

void chargerTextures()
{
    unsigned char *pixels;
    GLsizei largeur, hauteur;
    //if ( ( pixels = ChargerImage( "textures/echiquier.bmp", largeur, hauteur ) ) != NULL )
    if ( ( pixels = ChargerImage( "textures/etincelle.bmp", largeur, hauteur ) ) != NULL )
    {
        glGenTextures( 1, &Etat::textureETINCELLE );
        glBindTexture( GL_TEXTURE_2D, Etat::textureETINCELLE );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        //glGenerateMipmap( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, 0 );
        delete[] pixels;
    }
    if ( ( pixels = ChargerImage( "textures/oiseau.bmp", largeur, hauteur ) ) != NULL )
    {
        glGenTextures( 1, &Etat::textureOISEAU );
        glBindTexture( GL_TEXTURE_2D, Etat::textureOISEAU );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        //glGenerateMipmap( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, 0 );
        delete[] pixels;
    }
    if ( ( pixels = ChargerImage( "textures/bonhomme.bmp", largeur, hauteur ) ) != NULL )
    {
        glGenTextures( 1, &Etat::textureBONHOMME );
        glBindTexture( GL_TEXTURE_2D, Etat::textureBONHOMME );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        //glGenerateMipmap( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, 0 );
        delete[] pixels;
    }
}

void chargerNuanceurs()
{
    // charger le nuanceur de base
    std::cout << "Charger le nuanceur de base" << std::endl;
    {
        // créer le programme
        progBase = glCreateProgram();

        // attacher le nuanceur de sommets
        {
            GLuint nuanceurObj = glCreateShader( GL_VERTEX_SHADER );
            glShaderSource( nuanceurObj, 1, &ProgNuanceur::chainesSommetsMinimal, NULL );
            glCompileShader( nuanceurObj );
            glAttachShader( progBase, nuanceurObj );
            ProgNuanceur::afficherLogCompile( nuanceurObj );
        }
        // attacher le nuanceur de fragments
        {
            GLuint nuanceurObj = glCreateShader( GL_FRAGMENT_SHADER );
            glShaderSource( nuanceurObj, 1, &ProgNuanceur::chainesFragmentsMinimal, NULL );
            glCompileShader( nuanceurObj );
            glAttachShader( progBase, nuanceurObj );
            ProgNuanceur::afficherLogCompile( nuanceurObj );
        }

        // faire l'édition des liens du programme
        glLinkProgram( progBase );
        ProgNuanceur::afficherLogLink( progBase );

        // demander la "Location" des variables
        //if ( ( locVertexBase = glGetAttribLocation( progBase, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
        if ( ( locColorBase = glGetAttribLocation( progBase, "Color" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
        if ( ( locmatrModelBase = glGetUniformLocation( progBase, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
        if ( ( locmatrVisuBase = glGetUniformLocation( progBase, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
        if ( ( locmatrProjBase = glGetUniformLocation( progBase, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
    }

    // charger le nuanceur de ce TP
    std::cout << "Charger le nuanceur de ce TP" << std::endl;
    {
        // créer le programme
        prog = glCreateProgram();

        // attacher le nuanceur de sommets
        const GLchar *chainesSommets = ProgNuanceur::lireNuanceur( "nuanceurSommets.glsl" );
        if ( chainesSommets != NULL )
        {
            GLuint nuanceurObj = glCreateShader( GL_VERTEX_SHADER );
            glShaderSource( nuanceurObj, 1, &chainesSommets, NULL );
            glCompileShader( nuanceurObj );
            glAttachShader( prog, nuanceurObj );
            ProgNuanceur::afficherLogCompile( nuanceurObj );
            delete [] chainesSommets;
        }
        const GLchar *chainesGeometrie = ProgNuanceur::lireNuanceur( "nuanceurGeometrie.glsl" );
        if ( chainesGeometrie != NULL )
        {
            GLuint nuanceurObj = glCreateShader( GL_GEOMETRY_SHADER );
            glShaderSource( nuanceurObj, 1, &chainesGeometrie, NULL );
            glCompileShader( nuanceurObj );
            glAttachShader( prog, nuanceurObj );
            ProgNuanceur::afficherLogCompile( nuanceurObj );
            delete [] chainesGeometrie;
        }
        // attacher le nuanceur de fragments
        const GLchar *chainesFragments = ProgNuanceur::lireNuanceur( "nuanceurFragments.glsl" );
        if ( chainesFragments != NULL )
        {
            GLuint nuanceurObj = glCreateShader( GL_FRAGMENT_SHADER );
            glShaderSource( nuanceurObj, 1, &chainesFragments, NULL );
            glCompileShader( nuanceurObj );
            glAttachShader( prog, nuanceurObj );
            ProgNuanceur::afficherLogCompile( nuanceurObj );
            delete [] chainesFragments;
        }

        // faire l'édition des liens du programme
        glLinkProgram( prog );
        ProgNuanceur::afficherLogLink( prog );

        // demander la "Location" des variables
        if ( ( locVertex = glGetAttribLocation( prog, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
        if ( ( locColor = glGetAttribLocation( prog, "Color" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
        if ( ( locvitesse = glGetAttribLocation( prog, "vitesse" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de vitesse" << std::endl;
        if ( ( loctempsRestant = glGetAttribLocation( prog, "tempsRestant" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de tempsRestant" << std::endl;
        if ( ( locmatrModel = glGetUniformLocation( prog, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
        if ( ( locmatrVisu = glGetUniformLocation( prog, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
        if ( ( locmatrProj = glGetUniformLocation( prog, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
        if ( ( locpointsize = glGetUniformLocation( prog, "pointsize" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de pointsize" << std::endl;
        if ( ( loclaTexture = glGetUniformLocation( prog, "laTexture" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de laTexture" << std::endl;
        if ( ( loctexnumero = glGetUniformLocation( prog, "texnumero" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de texnumero" << std::endl;
        // std::cout << " locVertex=" << locVertex << " locColor=" << locColor << " locvitesse=" << locvitesse << " loctempsRestant=" << loctempsRestant << std::endl;
    }

    // charger le nuanceur de rétroaction
    std::cout << "Charger le nuanceur de rétroaction" << std::endl;
    {
        // créer le programme
        progRetroaction = glCreateProgram();

        // attacher le nuanceur de sommets
        const GLchar *chainesSommets = ProgNuanceur::lireNuanceur( "nuanceurRetroaction.glsl" );
        if ( chainesSommets != NULL )
        {
            GLuint nuanceurObj = glCreateShader( GL_VERTEX_SHADER );
            glShaderSource( nuanceurObj, 1, &chainesSommets, NULL );
            glCompileShader( nuanceurObj );
            glAttachShader( progRetroaction, nuanceurObj );
            ProgNuanceur::afficherLogCompile( nuanceurObj );
            delete [] chainesSommets;
        }

        // À MODIFIER (partie 1)
        //const GLchar* vars[] = { ... };
        //glTransformFeedbackVaryings( progRetroaction, sizeof(vars)/sizeof(vars[0]), vars, GL_INTERLEAVED_ATTRIBS );

        // faire l'édition des liens du programme
        glLinkProgram( progRetroaction );
        ProgNuanceur::afficherLogLink( progRetroaction );

        // demander la "Location" des variables
        if ( ( locpositionRetroaction = glGetAttribLocation( progRetroaction, "position" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de position" << std::endl;
        if ( ( loccouleurRetroaction = glGetAttribLocation( progRetroaction, "couleur" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de couleur" << std::endl;
        if ( ( locvitesseRetroaction = glGetAttribLocation( progRetroaction, "vitesse" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de vitesse" << std::endl;
        if ( ( loctempsRestantRetroaction = glGetAttribLocation( progRetroaction, "tempsRestant" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de tempsRestant" << std::endl;
        if ( ( loctempsRetroaction = glGetUniformLocation( progRetroaction, "temps" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de temps" << std::endl;
        if ( ( locdtRetroaction = glGetUniformLocation( progRetroaction, "dt" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de dt" << std::endl;
        if ( ( locgraviteRetroaction = glGetUniformLocation( progRetroaction, "gravite" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de gravite" << std::endl;
        if ( ( loctempsMaxRetroaction = glGetUniformLocation( progRetroaction, "tempsMax" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de tempsMax" << std::endl;
        if ( ( locposPuitsRetroaction = glGetUniformLocation( progRetroaction, "posPuits" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de posPuits" << std::endl;
        if ( ( locbDimRetroaction = glGetUniformLocation( progRetroaction, "bDim" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de bDim" << std::endl;
        // std::cout << " locpositionRetroaction=" << locpositionRetroaction << " loccouleurRetroaction=" << loccouleurRetroaction << " locvitesseRetroaction=" << locvitesseRetroaction << " loctempsRestantRetroaction=" << loctempsRestantRetroaction << std::endl;
    }

}

void FenetreTP::initialiser()
{
    // donner la couleur de fond
    glClearColor( 0.3, 0.3, 0.3, 1.0 );

    // activer les états openGL
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_PROGRAM_POINT_SIZE );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // Initialisation des particules
    for ( unsigned int i = 0 ; i < MAXNPARTICULES ; i++ )
        part[i].tempsRestant = 0.0; // la particule sera initialisée par le nuanceur de rétroaction

    // charger les textures
    chargerTextures();

    // charger les nuanceurs
    chargerNuanceurs();
    FenetreTP::VerifierErreurGL("après le chargement des nuanceurs");

    // Initialiser les formes pour les parois
    glUseProgram( progBase );
    demisphere = new FormeSphere( 1.0, 64, 64, true, false );
    disque = new FormeDisque( 0.0, 1.0, 64, 64 );

    // Initialiser les objets OpenGL
    glGenVertexArrays( 2, vao ); // générer deux VAOs
    glGenBuffers( 2, vbo );      // générer les VBOs
    glGenTransformFeedbacks( 1, tfo );

    // Initialiser le vao pour les particules
    // charger le VBO pour les valeurs modifiés
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, tfo[0] );
    glBindBuffer( GL_ARRAY_BUFFER, vbo[1] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(part), NULL, GL_STREAM_DRAW ); // on ne donne rien sinon la taille
    FenetreTP::VerifierErreurGL("après le chargement de tfo[0]");

    glUseProgram( prog );
    // remplir les VBO et faire le lien avec les attributs du nuanceur de sommets
    glBindVertexArray( vao[0] );
    glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(part), part, GL_STREAM_DRAW );
    glVertexAttribPointer( locVertex, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void*>( offsetof(Particule,position) ) );
    glEnableVertexAttribArray(locVertex);
    glVertexAttribPointer( locColor, 4, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void*>( offsetof(Particule,couleur) ) );
    glEnableVertexAttribArray(locColor);
    glVertexAttribPointer( locvitesse, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void*>( offsetof(Particule,vitesse) ) );
    glEnableVertexAttribArray(locvitesse);
    glVertexAttribPointer( loctempsRestant, 1, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void*>( offsetof(Particule,tempsRestant) ) );
    glEnableVertexAttribArray(loctempsRestant);
    glBindVertexArray( 0 );
    FenetreTP::VerifierErreurGL("après les glVertexAttribPointer de vao[0]");

    // remplir les VBO pour les valeurs modifiées
    glBindVertexArray( vao[1] );
    glBindBuffer( GL_ARRAY_BUFFER, vbo[1] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(part), part, GL_STREAM_DRAW ); // déjà fait ci-dessus
    glVertexAttribPointer( locpositionRetroaction, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void*>( offsetof(Particule,position) ) );
    glEnableVertexAttribArray(locpositionRetroaction);
    glVertexAttribPointer( loccouleurRetroaction, 4, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void*>( offsetof(Particule,couleur) ) );
    glEnableVertexAttribArray(loccouleurRetroaction);
    glVertexAttribPointer( locvitesseRetroaction, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void*>( offsetof(Particule,vitesse) ) );
    glEnableVertexAttribArray(locvitesseRetroaction);
    glVertexAttribPointer( loctempsRestantRetroaction, 1, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void*>( offsetof(Particule,tempsRestant) ) );
    glEnableVertexAttribArray(loctempsRestantRetroaction);
    glBindVertexArray( 0 );
    FenetreTP::VerifierErreurGL("après les glVertexAttribPointer de vao[1]");

    // Défaire tous les liens
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    // créer la requête afin d'obtenir un retour d'information lorsque souhaité
    glGenQueries( 1, &requete );

    FenetreTP::VerifierErreurGL("fin de initialiser");
}

void FenetreTP::conclure()
{
    glUseProgram( 0 );
    glDeleteVertexArrays( 2, vao );
    glDeleteBuffers( 2, vbo );
    glDeleteQueries( 1, &requete );
    delete demisphere;
    delete disque;
}

void definirProjection( int OeilMult, int w, int h ) // 0: mono, -1: oeil gauche, +1: oeil droit
{
    // partie 3: utiliser plutôt Frustum() pour le stéréo
    matrProj.Perspective( 55.0, (GLdouble) w / (GLdouble) h, VueStereo::zavant, VueStereo::zarriere );
}

void afficherDecoration()
{
    // utiliser le programme de base pour les décorations
    glUseProgram( progBase );
    glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj ); // donner la projection courante

    // afficher la boîte (demi-sphère)
    matrModel.PushMatrix();{
        matrModel.Scale( Etat::bDim.x, Etat::bDim.y, Etat::bDim.z );
        glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
        // la base de la boîte
        glVertexAttrib3f( locColorBase, 0.2, 0.2, 0.2 );
        disque->afficher();
        // les faces arrières de la demi-sphère qui sert de boîte
        glEnable( GL_CULL_FACE );
        glCullFace( GL_FRONT ); // on enlève les faces avant pour ne garder que les faces arrières
        glVertexAttrib3f( locColorBase, 0.4, 0.4, 0.5 );
        demisphere->afficher();
        glDisable( GL_CULL_FACE );
    }matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
}

void afficherModele()
{
    // afficher d'abord les décorations (en utilisant progBase)
    afficherDecoration();

    // afficher les particules (en utilisant prog)
    glUseProgram( prog );

    glBindVertexArray( vao[0] );
    // refaire le lien avec les attributs du nuanceur de sommets pour le vbo actuellement utilisé
    glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
    // glVertexAttribPointer( locVertex, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void*>( offsetof(Particule,position) ) );
    // glVertexAttribPointer( locColor, 4, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void*>( offsetof(Particule,couleur) ) );
    // glVertexAttribPointer( locvitesse, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void*>( offsetof(Particule,vitesse) ) );
    // glVertexAttribPointer( loctempsRestant, 1, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void*>( offsetof(Particule,tempsRestant) ) );

    switch ( Etat::texnumero ) // 0-aucune, 1-étincelle, 2-oiseau, 3-bonhomme
    {
    default: glBindTexture( GL_TEXTURE_2D, 0 ); break;
    case 1: glBindTexture( GL_TEXTURE_2D, Etat::textureETINCELLE ); break;
    case 2: glBindTexture( GL_TEXTURE_2D, Etat::textureOISEAU ); break;
    case 3: glBindTexture( GL_TEXTURE_2D, Etat::textureBONHOMME ); break;
    }

    // tracer le résultat de la rétroaction
    //glDrawTransformFeedback( GL_POINTS, tfo[0] );
    glDrawArrays( GL_POINTS, 0, parametre.nparticules );

    glBindTexture( GL_TEXTURE_2D, 0 );
    glBindVertexArray( 0 );
}

void FenetreTP::afficherScene()
{
    // effacer l'écran et le tampon de profondeur
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( progBase );

    // définir le pipeline graphique
    definirProjection( 0, largeur_, hauteur_ );
    glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj );

    camera.definir();
    glUniformMatrix4fv( locmatrVisuBase, 1, GL_FALSE, matrVisu );

    matrModel.LoadIdentity();
    glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );

    // afficher les axes
    if ( Etat::afficheAxes ) FenetreTP::afficherAxes( 0.2 );

    // afficher les particules
    //glActiveTexture( GL_TEXTURE0 ); // activer la texture '0' (valeur de défaut)
    glUseProgram( prog );
    glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
    glUniformMatrix4fv( locmatrVisu, 1, GL_FALSE, matrVisu );
    glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
    glUniform1f( locpointsize, Etat::pointsize );
    glUniform1i( loclaTexture, 0 ); // '0' => utilisation de GL_TEXTURE0
    glUniform1i( loctexnumero, Etat::texnumero );

    // partie 3: afficher la surface en mono ou en stéréo
    switch ( Etat::affichageStereo )
    {
    case 0: // mono
        definirProjection( 0, largeur_, hauteur_ );
        glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
        afficherModele();
        break;

    case 1: // stéréo anaglyphe
        // partie 3: à modifier pour afficher en anaglyphe
        definirProjection( 0, largeur_, hauteur_ );
        glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
        afficherModele();
        break;

    case 2: // stéréo double
        // partie 3: à modifier pour afficher en stéréo double
        definirProjection( 0, largeur_, hauteur_ );
        glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
        afficherModele();
        break;
    }

    VerifierErreurGL("fin de afficherScene");
}

void FenetreTP::redimensionner( GLsizei w, GLsizei h )
{
    glViewport( 0, 0, w, h );
}

void FenetreTP::clavier( TP_touche touche )
{
    // quelques variables pour n'imprimer qu'une seule fois la liste des touches lorsqu'une touche est invalide
    bool toucheValide = true; // on suppose que la touche est connue
    static bool listerTouchesAFAIRE = true; // si la touche est invalide, on imprimera la liste des touches

    switch ( touche )
    {
    case TP_ECHAP:
    case TP_q: // Quitter l'application
        quit();
        break;

    case TP_x: // Activer/désactiver l'affichage des axes
        Etat::afficheAxes = !Etat::afficheAxes;
        std::cout << "// Affichage des axes ? " << ( Etat::afficheAxes ? "OUI" : "NON" ) << std::endl;
        break;

    case TP_v: // Recharger les fichiers des nuanceurs et recréer le programme
        chargerNuanceurs();
        std::cout << "// Recharger nuanceurs" << std::endl;
        break;

    case TP_j: // Incrémenter le nombre de particules
    case TP_CROCHETDROIT:
        {
            unsigned int nparticulesPrec = parametre.nparticules;
            parametre.nparticules *= 1.2;
            if ( parametre.nparticules > MAXNPARTICULES ) parametre.nparticules = MAXNPARTICULES;
            std::cout << " nparticules=" << parametre.nparticules << std::endl;
            // on met les nouvelles particules au puits
            // (glBindBuffer n'est pas très efficace, mais on ne fait pas ça souvent)
            glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
            Particule *ptr = (Particule*) glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
            for ( unsigned int i = nparticulesPrec ; i < parametre.nparticules ; ++i )
                ptr[i].tempsRestant = 0.0; // la particule sera initialisée par le nuanceur de rétroaction
            glUnmapBuffer( GL_ARRAY_BUFFER );
        }
        break;
    case TP_u: // Décrémenter le nombre de particules
    case TP_CROCHETGAUCHE:
        parametre.nparticules /= 1.2;
        if ( parametre.nparticules < 5 ) parametre.nparticules = 5;
        std::cout << " nparticules=" << parametre.nparticules << std::endl;
        break;

    case TP_DROITE: // Augmenter la dimension de la boîte en X
        Etat::bDim.x += 0.1;
        std::cout << " Etat::bDim= " << Etat::bDim.x << " x " << Etat::bDim.y << " x " << Etat::bDim.z << std::endl;
        break;
    case TP_GAUCHE: // Diminuer la dimension de la boîte en X
        if ( Etat::bDim.x > 0.25 ) Etat::bDim.x -= 0.1;
        Etat::verifierPositionPuits();
        std::cout << " Etat::bDim= " << Etat::bDim.x << " x " << Etat::bDim.y << " x " << Etat::bDim.z << std::endl;
        break;
    case TP_BAS: // Augmenter la dimension de la boîte en Y
        Etat::bDim.y += 0.1;
        std::cout << " Etat::bDim= " << Etat::bDim.x << " x " << Etat::bDim.y << " x " << Etat::bDim.z << std::endl;
        break;
    case TP_HAUT: // Diminuer la dimension de la boîte en Y
        if ( Etat::bDim.y > 0.25 ) Etat::bDim.y -= 0.1;
        Etat::verifierPositionPuits();
        std::cout << " Etat::bDim= " << Etat::bDim.x << " x " << Etat::bDim.y << " x " << Etat::bDim.z << std::endl;
        break;
    case TP_PAGEPREC: // Augmenter la dimension de la boîte en Z
        Etat::bDim.z += 0.1;
        std::cout << " Etat::bDim= " << Etat::bDim.x << " x " << Etat::bDim.y << " x " << Etat::bDim.z << std::endl;
        break;
    case TP_PAGESUIV: // Diminuer la dimension de la boîte en Z
        if ( Etat::bDim.z > 0.25 ) Etat::bDim.z -= 0.1;
        Etat::verifierPositionPuits();
        std::cout << " Etat::bDim= " << Etat::bDim.x << " x " << Etat::bDim.y << " x " << Etat::bDim.z << std::endl;
        break;

    case TP_0: // Remettre le puits à la position (0,0,0)
        Etat::posPuits = glm::vec3( 0.0, 0.0, 0.0 );
        break;

    case TP_PLUS: // Avancer la caméra
    case TP_EGAL:
        camera.dist -= 0.2;
        if ( camera.dist < 0.4 ) camera.dist = 0.4;
        std::cout << " camera.dist=" << camera.dist << std::endl;
        break;

    case TP_SOULIGNE:
    case TP_MOINS: // Reculer la caméra
        camera.dist += 0.2;
        if ( camera.dist > 20.0 - Etat::bDim.y ) camera.dist = 20.0 - Etat::bDim.y;
        std::cout << " camera.dist=" << camera.dist << std::endl;
        break;

    case TP_b: // Incrémenter la gravité
        parametre.gravite += 0.05;
        std::cout << " parametre.gravite=" << parametre.gravite << std::endl;
        break;
    case TP_h: // Décrémenter la gravité
        parametre.gravite -= 0.05;
        if ( parametre.gravite < 0.0 ) parametre.gravite = 0.0;
        std::cout << " parametre.gravite=" << parametre.gravite << std::endl;
        break;

    case TP_l: // Incrémenter la durée de vie maximale
        parametre.tempsMax += 0.2;
        std::cout << " parametre.tempsMax=" << parametre.tempsMax << std::endl;
        break;
    case TP_k: // Décrémenter la durée de vie maximale
        parametre.tempsMax -= 0.2;
        if ( parametre.tempsMax < 1.0 ) parametre.tempsMax = 1.0;
        std::cout << " parametre.tempsMax=" << parametre.tempsMax << std::endl;
        break;

    case TP_f: // Incrémenter la taille des particules
        Etat::pointsize += 0.2;
        std::cout << " Etat::pointsize=" << Etat::pointsize << std::endl;
        break;
    case TP_d: // Décrémenter la taille des particules
        Etat::pointsize -= 0.2;
        if ( Etat::pointsize < 1.0 ) Etat::pointsize = 1.0;
        std::cout << " Etat::pointsize=" << Etat::pointsize << std::endl;
        break;

    case TP_t: // Changer la texture utilisée: 0-aucune, 1-étincelle, 2-oiseau, 3-bonhomme
        if ( ++Etat::texnumero > 3 ) Etat::texnumero = 0;
        std::cout << " Etat::texnumero=" << Etat::texnumero << std::endl;
        break;

    case TP_i: // on veut faire une impression
        Etat::impression = true;
        break;

    case TP_a: // Boucler sur les textures automatiquement
        Etat::texVarie = !Etat::texVarie;
        std::cout << " Etat::texVarie=" << Etat::texVarie << std::endl;
        break;

    case TP_s: // Varier le type d'affichage stéréo: mono, stéréo anaglyphe, stéréo double
        if ( ++Etat::affichageStereo > 2 ) Etat::affichageStereo = 0;
        std::cout << " affichageStereo=" << Etat::affichageStereo << std::endl;
        break;

    case TP_g: // Permuter l'affichage en fil de fer ou plein
        Etat::modePolygone = ( Etat::modePolygone == GL_FILL ) ? GL_LINE : GL_FILL;
        glPolygonMode( GL_FRONT_AND_BACK, Etat::modePolygone );
        break;

    case TP_ESPACE: // Mettre en pause ou reprendre l'animation
        Etat::enmouvement = !Etat::enmouvement;
        break;

    default:
        std::cout << " touche inconnue : " << (char) touche << std::endl;
        toucheValide = false;
        break;
    }

    // n'imprimer qu'une seule fois la liste des touches lorsqu'une touche est invalide
    if ( toucheValide ) // si la touche est valide, ...
    {
        listerTouchesAFAIRE = true; // ... on imprimera la liste des touches à la prochaine touche invalide
    }
    else if ( listerTouchesAFAIRE ) // s'il faut imprimer la liste des touches ...
    {
        listerTouchesAFAIRE = false; // ... se souvenir que ça a été fait
        imprimerTouches();
    }

}

// ce qui est déplacé par la souris
static enum { deplaceCam, deplacePuits } deplace = deplaceCam;
static bool pressed = false;
void FenetreTP::sourisClic( int button, int state, int x, int y )
{
    pressed = ( state == TP_PRESSE );
    if ( pressed )
    {
        // on vient de presser la souris
        Etat::sourisPosPrec.x = x;
        Etat::sourisPosPrec.y = y;
        switch ( button )
        {
        case TP_BOUTON_GAUCHE: // Manipuler la caméra
        case TP_BOUTON_MILIEU:
            deplace = deplaceCam;
            break;
        case TP_BOUTON_DROIT: // Déplacer le puits
            deplace = deplacePuits;
            break;
        }
    }
    else
    {
        // on vient de relâcher la souris
    }
}

void FenetreTP::sourisMolette( int x, int y ) // Changer la distance de la caméra
{
    const int sens = +1;
    camera.dist -= 0.2 * sens*y;
    if ( camera.dist < 0.4 ) camera.dist = 0.4;
    else if ( camera.dist > 20.0 - Etat::bDim.y ) camera.dist = 20.0 - Etat::bDim.y;
}

void FenetreTP::sourisMouvement( int x, int y )
{
    if ( pressed )
    {
        int dx = x - Etat::sourisPosPrec.x;
        int dy = y - Etat::sourisPosPrec.y;
        switch ( deplace )
        {
        case deplaceCam:
            camera.theta -= dx / 3.0;
            camera.phi   -= dy / 3.0;
            break;
        case deplacePuits:
            {
                glm::mat4 VM = matrVisu*matrModel;
                glm::mat4 P = matrProj;
                glm::vec4 cloture( 0, 0, largeur_, hauteur_ );
                glm::vec3 ecranPosPrec = glm::project( glm::vec3(Etat::posPuits), VM, P, cloture );
                glm::vec3 ecranPos( x, hauteur_-y, ecranPosPrec.z );
                Etat::posPuits = glm::vec4( glm::unProject( ecranPos, VM, P, cloture ), 1.0 );
                Etat::verifierPositionPuits();
                std::cout << " Etat::posPuits=" << glm::to_string(Etat::posPuits) << std::endl;
            }
            break;
        }

        Etat::sourisPosPrec.x = x;
        Etat::sourisPosPrec.y = y;

        camera.verifierAngles();
    }
}

int main( int argc, char *argv[] )
{
    // créer une fenêtre
    FenetreTP fenetre( "INF2705 TP" );

    // allouer des ressources et définir le contexte OpenGL
    fenetre.initialiser();

    bool boucler = true;
    while ( boucler )
    {
        // mettre à jour la physique
        calculerPhysique( );

        // affichage
        fenetre.afficherScene();
        fenetre.swap();

        // récupérer les événements et appeler la fonction de rappel
        boucler = fenetre.gererEvenement();
    }

    // détruire les ressources OpenGL allouées
    fenetre.conclure();

    return 0;
}
