// Prénoms, noms et matricule des membres de l'équipe:
// Mikael MARSOLAIS (1844166)
// Quoc-Hao TRAN (1972967)
//#warning "Écrire les prénoms, noms et matricule des membres de l'équipe dans le fichier et commenter cette ligne"

#include <stdlib.h>
#include <iostream>
#include "inf2705-matrice.h"
#include "inf2705-nuanceur.h"
#include "inf2705-fenetre.h"
#include "inf2705-forme.h"
#include <glm/gtx/io.hpp>
#include <vector>
#include <iterator>

// variables pour l'utilisation des nuanceurs
GLuint prog;      // votre programme de nuanceurs
GLint locVertex = -1;
GLint locColor = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;
GLint locplanDragage = -1;
GLint locplanRayonsX = -1;
GLint locattEloignement = -1;
GLuint progBase;  // le programme de nuanceurs de base
GLint locVertexBase = -1;
GLint locColorBase = -1;
GLint locmatrModelBase = -1;
GLint locmatrVisuBase = -1;
GLint locmatrProjBase = -1;

// matrices du pipeline graphique
MatricePipeline matrModel, matrVisu, matrProj;

// les formes
FormeCube *cubeFil = NULL;
FormeCube *cube = NULL;
FormeCylindre *cylindre = NULL; // un cylindre centré dans l'axe des Z, de rayon 1, entre (0,0,0) et (0,0,1)
GLuint vao = 0;
GLuint vbo[2] = {0,0};

//
// variables d'état
//
struct Etat
{
   bool modeSelection;    // on est en mode sélection?
   bool enmouvement;      // le modèle est en mouvement/rotation automatique ou non
   bool afficheAxes;      // indique si on affiche les axes
   bool attEloignement;   // indique si on veut atténuer selone l'éloignement
   glm::vec4 bDim;        // les dimensions de l'aquarium: une boite [-x,+x][-y,+y][-z,+z]
   glm::ivec2 sourisPosPrec;
   // partie 1: utiliser des plans de coupe:  Ax + By + Cz + D = 0  <=>  Ax + By + Cz = -D
   glm::vec4 planRayonsX; // équation du plan de rayonX (partie 1)
   glm::vec4 planDragage; // équation du plan de dragage (partie 1)
   GLfloat angleDragage;  // angle (degrés) du plan de dragage autour de y (partie 1)
} etat = { false, true, true, false, glm::vec4( 16.0, 10.0, 8.0, 1.0 ), glm::ivec2(0), glm::vec4( 1, 0, 0, 4.0 ), glm::vec4( 0, 0, 1, 7.9 ), 0.0 };

//
// variables pour définir le point de vue
//
class Camera
{
public:
   void definir()
   {
      matrVisu.LookAt( dist*cos(glm::radians(theta))*sin(glm::radians(phi)),
                       dist*sin(glm::radians(theta))*sin(glm::radians(phi)),
                       dist*cos(glm::radians(phi)),
                       0, 0, 0,
                       0, 0, 1 );
   }
   void verifierAngles() // vérifier que les angles ne débordent pas les valeurs permises
   {
      const GLdouble MINPHI = 0.01, MAXPHI = 180.0 - 0.01;
      phi = glm::clamp( phi, MINPHI, MAXPHI );
   }
   double theta;         // angle de rotation de la caméra (coord. sphériques)
   double phi;           // angle de rotation de la caméra (coord. sphériques)
   double dist;          // distance (coord. sphériques)
} camera = { 90.0, 75.0, 35.0 };


//
// les poissons
//
class Poisson
{
public:
   Poisson( glm::vec3 pos = glm::vec3(3.0,1.0,0.0), glm::vec3 vit = glm::vec3(1.0,0.0,0.0), float tai = 0.5 )
      : position(pos), vitesse(vit), taille(tai)
   {}

   void afficher()
   {
      matrModel.PushMatrix();{ // sauvegarder la tranformation courante

         // amener le repère à la position courante
         matrModel.Translate( position.x, position.y, position.z );

         // partie 2: modifs ici ...
         // donner la couleur de sélection

         // afficher le corps
         // (en utilisant le cylindre centré dans l'axe des Z, de rayon 1, entre (0,0,0) et (0,0,1))
         glm::vec3 coulCorps( 0.0, 1.0, 0.0 ); // vert
         glVertexAttrib3fv( locColor, glm::value_ptr(coulCorps) );
         matrModel.PushMatrix();{
            matrModel.Scale( 5.0*taille, taille, taille );
            matrModel.Rotate( 90.0, 0.0, 1.0, 0.0 );
            if ( vitesse.x > 0.0 ) matrModel.Rotate( 180.0, 0.0, 1.0, 0.0 );
            matrModel.Translate( 0.0, 0.0, -0.5 );
            glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel ); // ==> Avant de tracer, on doit informer la carte graphique des changements faits à la matrice de modélisation

            cylindre->afficher();
         }matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );

         // afficher les yeux
         // (en utilisant le cylindre centré dans l'axe des Z, de rayon 1, entre (0,0,0) et (0,0,1))
         glm::vec3 coulYeux( 1.0, 1.0, 0.0 ); // jaune
         glVertexAttrib3fv( locColor, glm::value_ptr(coulYeux) );
         matrModel.PushMatrix();{
            matrModel.Rotate( 90.0, 1.0, 0.0, 0.0 );
            matrModel.Scale( 0.5*taille, 0.5*taille, 2*taille );
            matrModel.Translate( vitesse.x > 0.0 ? 4.0 : -4.0, 0.2, -0.5 );
            glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            cylindre->afficher();
            matrModel.Translate( 0.0, 0.0, 2.0 );
         }matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );

      }matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
      glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel ); // informer ...
   }

   void avancerPhysique()
   {
      const float dt = 0.5; // intervalle entre chaque affichage (en secondes)
      position += dt * vitesse;
      // test rapide pour empêcher que les poissons sortent de l'aquarium
      if ( abs(position.x) > 0.9*etat.bDim.x ) vitesse = -vitesse;
   }

   // les variables du poisson
   glm::vec3 position;   // en unités
   glm::vec3 vitesse;    // en unités/seconde
   float taille;         // en unités
};

//
// l'aquarium
//
class Aquarium
{
public:
   Aquarium( )
   {
      // les positions des 25 poissons
      glm::vec3 pos[] =
      {
         glm::vec3( 0.0, -6.5, -5.3 ),
         glm::vec3( 0.0,  0.0,  0.0 ),
         glm::vec3( 0.0, -6.6, -2.5 ),
         glm::vec3( 0.0, -6.5,  2.7 ),
         glm::vec3( 0.0, -3.3, -5.7 ),
         glm::vec3( 0.0, -3.3,  5.1 ),
         glm::vec3( 0.0,  6.5,  5.1 ),
         glm::vec3( 0.0,  6.1, -5.3 ),
         glm::vec3( 0.0,  0.0, -5.3 ),
         glm::vec3( 0.0,  0.1, -2.5 ),
         glm::vec3( 0.0, -3.4,  2.2 ),
         glm::vec3( 0.0,  6.8,  0.0 ),
         glm::vec3( 0.0,  0.3,  2.1 ),
         glm::vec3( 0.0, -3.3,  0.0 ),
         glm::vec3( 0.0,  0.2,  5.3 ),
         glm::vec3( 0.0,  3.3, -5.2 ),
         glm::vec3( 0.0,  6.2, -2.5 ),
         glm::vec3( 0.0,  3.2, -2.2 ),
         glm::vec3( 0.0, -6.7,  0.0 ),
         glm::vec3( 0.0, -3.5, -2.5 ),
         glm::vec3( 0.0,  3.7,  0.0 ),
         glm::vec3( 0.0,  3.8,  5.3 ),
         glm::vec3( 0.0, -6.1,  5.3 ),
         glm::vec3( 0.0,  3.3,  2.4 ),
         glm::vec3( 0.0,  6.4,  2.8 ),
      };

      // initialiser la génération de valeurs aléatoires pour la création de poissons
      srand( time(NULL) );

      // remplir l'aquarium
      for ( unsigned int i = 0 ; i < sizeof(pos)/sizeof(pos[0]) ; ++i )
      {
         // donner position aléatoire en x
         pos[i].x = glm::mix( -0.9*etat.bDim.x, 0.9*etat.bDim.x, rand()/((double)RAND_MAX) );
         // donner vitesse aléatoire en x
         glm::vec3 vit = glm::vec3( glm::mix( -0.2, 0.2, rand()/((double)RAND_MAX) ), 0.0, 0.0 );
         vit.x += 0.1 * glm::sign(vit.x); // ajouter ou soustraire 0.1 selon le signe de vx afin d'avoir : 0.1 <= abs(vx) <= 0.3
         // donner taille aléatoire
         float taille = glm::mix( 0.5 , 0.9, rand()/((double)RAND_MAX) );

         // créer un nouveau poisson
         Poisson *p = new Poisson( pos[i], vit, taille );

         // assigner une couleur de sélection
         // partie 2: modifs ici ...

         // ajouter ce poisson dans la liste
         poissons.push_back( p );
      }
   }

   void afficherQuad( GLfloat alpha ) // le plan qui ferme les solides
   {
      glVertexAttrib4f( locColor, 1.0, 1.0, 1.0, alpha );
      // afficher le plan mis à l'échelle, tourné selon l'angle courant et à la position courante
      // partie 1: modifs ici ...

   }

   void afficherParois()
   {
      // tracer les parois de verre de l'aquarium
      glEnable( GL_CULL_FACE ); glCullFace( GL_FRONT ); // ne pas afficher les faces avant
      glVertexAttrib3f( locColorBase, 0.1, 0.1, 0.3 ); // bleuté
      matrModel.PushMatrix();{
         matrModel.Scale( etat.bDim.x, etat.bDim.y, etat.bDim.z );
         glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
         cube->afficher();
      }matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
      glDisable( GL_CULL_FACE );

      // tracer les coins de l'aquarium
      glVertexAttrib3f( locColorBase, 1.0, 1.0, 1.0 ); // blanc
      matrModel.PushMatrix();{
         matrModel.Scale( 0.999*etat.bDim.x, 0.999*etat.bDim.y, 0.999*etat.bDim.z ); // légèrement à l'intérieur
         glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
         cubeFil->afficher();
      }matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
   }

   void afficherTousLesPoissons()
   {
      glVertexAttrib4f( locColor, 1.0, 1.0, 1.0, 1.0 );

      std::vector<Poisson*>::iterator it;
      for ( it = poissons.begin() ; it != poissons.end() ; it++ )
      {
         (*it)->afficher();
      }
   }

   void afficherContenu()
   {
      // partie 1: modifs ici ...
      // afficher les poissons en plein et en fil de fer en tenant compte du plan de rayonsX,
      // puis tenir compte aussi du plan de dragage

      // activer les plans de coupe et afficher la scène normalement
      // partie 1: modifs ici ...
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      afficherTousLesPoissons();

      glEnable(GL_CLIP_PLANE0);
      glEnable(GL_CLIP_PLANE1);
      
      // afficher les poissons en plein
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      afficherTousLesPoissons();
      // afficher les poissons en fil de fer (squelette)
      // ...
      glDisable(GL_CLIP_PLANE1);
      glDisable(GL_CLIP_PLANE0);
      // « fermer » les poissons
      // partie 1: modifs ici ...
      // ...
   }

   void calculerPhysique( )
   {
      if ( etat.enmouvement )
      {
         std::vector<Poisson*>::iterator it;
         for ( it = poissons.begin() ; it != poissons.end() ; it++ )
         {
            (*it)->avancerPhysique();
         }
      }
   }

   // la liste des poissons
   std::vector<Poisson*> poissons;
};
Aquarium aquarium;


void chargerNuanceurs()
{
   // charger le nuanceur de base
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
      if ( ( locVertexBase = glGetAttribLocation( progBase, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
      if ( ( locColorBase = glGetAttribLocation( progBase, "Color" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
      if ( ( locmatrModelBase = glGetUniformLocation( progBase, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
      if ( ( locmatrVisuBase = glGetUniformLocation( progBase, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
      if ( ( locmatrProjBase = glGetUniformLocation( progBase, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
   }

   {
      // charger le nuanceur de ce TP

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
#if 0
      // partie 2: enlever le "#if 0" pour utiliser le nuanceur de géométrie
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
#endif
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
      if ( ( locmatrModel = glGetUniformLocation( prog, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
      if ( ( locmatrVisu = glGetUniformLocation( prog, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
      if ( ( locmatrProj = glGetUniformLocation( prog, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
      if ( ( locplanDragage = glGetUniformLocation( prog, "planDragage" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de planDragage" << std::endl;
      if ( ( locplanRayonsX = glGetUniformLocation( prog, "planRayonsX" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de planRayonsX" << std::endl;
      if ( ( locattEloignement = glGetUniformLocation( prog, "attEloignement" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de attEloignement" << std::endl;
   }
}

void FenetreTP::initialiser()
{
   // donner la couleur de fond
   glClearColor( 0.1, 0.1, 0.1, 1.0 );

   // activer les états openGL
   glEnable( GL_DEPTH_TEST );

   // activer le mélange de couleur pour la transparence
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

   // charger les nuanceurs
   chargerNuanceurs();
   glUseProgram( prog );

   // les valeurs à utiliser pour tracer le quad
   GLfloat coo[] = { -1,  1, 0,
                      1,  1, 0,
                      1, -1, 0,
                     -1, -1, 0 };
   const GLuint connec[] = { 0, 1, 2, 2, 3, 0 };

   // *** l'initialisation des objets graphiques doit être faite seulement après l'initialisation de la fenêtre graphique

   // partie 1: initialiser le VAO (pour le quad de l'aquarium)
   // ...
   // partie 1: créer les deux VBO pour les sommets et la connectivité
   // ...

   // ...

   // créer quelques autres formes
   cubeFil = new FormeCube( 2.0, false );
   cube = new FormeCube( 2.0 );
   cylindre = new FormeCylindre( 1.0, 1.0, 1.0, 12, 1, true );
}

void FenetreTP::conclure()
{
   delete cubeFil;
   delete cube;
   delete cylindre;
   glDeleteBuffers( 2, vbo );
   glDeleteVertexArrays( 1, &vao );
}

void FenetreTP::afficherScene( )
{
   // effacer les tampons de couleur, de profondeur et de stencil
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

   glUseProgram( progBase );

   // définir le pipeline graphique
   matrProj.Perspective( 50.0, (GLdouble) largeur_ / (GLdouble) hauteur_, 0.1, 100.0 );
   glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj );

   camera.definir();
   glUniformMatrix4fv( locmatrVisuBase, 1, GL_FALSE, matrVisu );

   matrModel.LoadIdentity();
   glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );

   // afficher les axes
   if ( etat.afficheAxes ) FenetreTP::afficherAxes(4);

   // dessiner la scène
   glUseProgram( prog );
   glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
   glUniformMatrix4fv( locmatrVisu, 1, GL_FALSE, matrVisu );
   glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
   glUniform4fv( locplanDragage, 1, glm::value_ptr(etat.planDragage) );
   glUniform4fv( locplanRayonsX, 1, glm::value_ptr(etat.planRayonsX) );
   glUniform1i( locattEloignement, etat.attEloignement );

   // afficher le contenu de l'aquarium
   aquarium.afficherContenu();

   // en plus, dessiner le plan de dragage en transparence pour bien voir son étendue
   aquarium.afficherQuad( 0.25 );

   // revenir au programme de base pour tracer l'aquarium
   glUseProgram( progBase );

   // tracer les parois de l'aquarium
   aquarium.afficherParois( );

   // sélectionner ?
   // partie 2: modifs ici ...

}

void FenetreTP::redimensionner( GLsizei w, GLsizei h )
{
   glViewport( 0, 0, w, h );
}

void FenetreTP::clavier( TP_touche touche )
{
   switch ( touche )
   {
   case TP_ECHAP:
   case TP_q: // Quitter l'application
      quit();
      break;

   case TP_x: // Activer/désactiver l'affichage des axes
      etat.afficheAxes = !etat.afficheAxes;
      std::cout << "// Affichage des axes ? " << ( etat.afficheAxes ? "OUI" : "NON" ) << std::endl;
      break;

   case TP_v: // Recharger les fichiers des nuanceurs et recréer le programme
      chargerNuanceurs();
      std::cout << "// Recharger nuanceurs" << std::endl;
      break;

   case TP_ESPACE: // Mettre en pause ou reprendre l'animation
      etat.enmouvement = !etat.enmouvement;
      break;

   case TP_HAUT: // Déplacer le plan de dragage vers le haut
      etat.planDragage.w += 0.1;
      std::cout << " etat.planDragage.w=" << etat.planDragage.w << std::endl;
      break;

   case TP_BAS: // Déplacer le plan de dragage vers le bas
      etat.planDragage.w -= 0.1;
      std::cout << " etat.planDragage.w=" << etat.planDragage.w << std::endl;
      break;

   case TP_CROCHETDROIT:
   case TP_DROITE: // Augmenter l'angle du plan de dragage
      etat.angleDragage += 1.0; if ( etat.angleDragage > 60 ) etat.angleDragage = 60;
      etat.planDragage.x = sin(glm::radians(etat.angleDragage));
      etat.planDragage.z = cos(glm::radians(etat.angleDragage));
      std::cout << " etat.angleDragage=" << etat.angleDragage << std::endl;
      break;
   case TP_CROCHETGAUCHE:
   case TP_GAUCHE: // Diminuer l'angle du plan de dragage
      etat.angleDragage -= 1.0; if ( etat.angleDragage < -60 ) etat.angleDragage = -60;
      etat.planDragage.x = sin(glm::radians(etat.angleDragage));
      etat.planDragage.z = cos(glm::radians(etat.angleDragage));
      std::cout << " etat.angleDragage=" << etat.angleDragage << std::endl;
      break;

   case TP_PLUS: // Incrémenter la distance de la caméra
   case TP_EGAL:
      camera.dist--;
      std::cout << " camera.dist=" << camera.dist << std::endl;
      break;

   case TP_SOULIGNE:
   case TP_MOINS: // Décrémenter la distance de la caméra
      camera.dist++;
      std::cout << " camera.dist=" << camera.dist << std::endl;
      break;

   case TP_a: // Atténuer ou non la couleur selon l'éloignement
      etat.attEloignement = !etat.attEloignement;
      std::cout << " etat.attEloignement=" << etat.attEloignement << std::endl;
      break;

   default:
      std::cout << " touche inconnue : " << (char) touche << std::endl;
      imprimerTouches();
      break;
   }
}

static bool pressed = false;
void FenetreTP::sourisClic( int button, int state, int x, int y )
{
   pressed = ( state == TP_PRESSE );
   if ( pressed )
   {
      switch ( button )
      {
      default:
      case TP_BOUTON_GAUCHE: // Modifier le point de vue
         etat.modeSelection = false;
         break;
      case TP_BOUTON_DROIT: // Sélectionner des objets
         etat.modeSelection = true;
         break;
      }
      etat.sourisPosPrec.x = x;
      etat.sourisPosPrec.y = y;
   }
   else
   {
      etat.modeSelection = false;
   }
}

void FenetreTP::sourisMolette( int x, int y ) // Déplacer le plan de dragage
{
   const int sens = +1;
   etat.planDragage.w += 0.05 * sens * y;
   std::cout << " etat.planDragage.w=" << etat.planDragage.w << std::endl;
}

void FenetreTP::sourisMouvement( int x, int y )
{
   if ( pressed )
   {
      if ( !etat.modeSelection )
      {
         int dx = x - etat.sourisPosPrec.x;
         int dy = y - etat.sourisPosPrec.y;
         camera.theta -= dx / 3.0;
         camera.phi   -= dy / 3.0;
      }

      etat.sourisPosPrec.x = x;
      etat.sourisPosPrec.y = y;

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
      aquarium.calculerPhysique( );

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
