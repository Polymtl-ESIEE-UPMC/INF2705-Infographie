// Prénoms, noms et matricule des membres de l'équipe:
// Mikael MARSOLAIS (1844166)
// Quoc-Hao TRAN (1972967)
// #warning "Écrire les prénoms, noms et matricule des membres de l'équipe dans le fichier et commenter cette ligne"

#include <stdlib.h>
#include <iostream>
#include "inf2705-matrice.h"
#include "inf2705-nuanceur.h"
#include "inf2705-fenetre.h"
#include "inf2705-texture.h"
#include "inf2705-forme.h"
#include "Etat.h"

// variables pour l'utilisation des nuanceurs
GLuint prog;      // votre programme de nuanceurs
GLint locVertex = -1;
GLint locNormal = -1;
GLint locTexCoord = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;
GLint locmatrNormale = -1;
GLint loclaTexture = -1;
GLint locfacteurDeform = -1;
GLint locTessLevelInner = -1;
GLint locTessLevelOuter = -1;
GLuint indLightSource;
GLuint indFrontMaterial;
GLuint indLightModel;
GLuint indvarsUnif;
GLuint progBase;  // le programme de nuanceurs de base
GLint locVertexBase = -1;
GLint locColorBase = -1;
GLint locmatrModelBase = -1;
GLint locmatrVisuBase = -1;
GLint locmatrProjBase = -1;

GLuint vao[2];
GLuint vbo[5];
GLuint vboLumi;
GLuint ubo[4];

// matrices du pipeline graphique
MatricePipeline matrModel, matrVisu, matrProj;

// les formes
FormeSphere *sphere = NULL, *sphereLumi = NULL;
FormeTheiere *theiere = NULL;
FormeTore *tore = NULL;
FormeCylindre *cylindre = NULL;
FormeCylindre *cone = NULL;

// partie 3: texture
GLuint textures[4];              // les textures chargées

//
// variables pour définir le point de vue
//
class Camera
{
public:
	void definir()
	{
		matrVisu.LookAt( 0.0, 0.0, dist,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0 );
		matrVisu.Rotate( phi, -1.0, 0.0, 0.0 );
		matrVisu.Rotate( theta, 0.0, -1.0, 0.0 );
	}
	void verifierAngles() // vérifier que les angles ne débordent pas les valeurs permises
	{
		const GLdouble MINPHI = -89.9, MAXPHI = 89.9;
		phi = glm::clamp( phi, MINPHI, MAXPHI );
	}
	double theta;         // angle de rotation de la caméra (coord. sphériques)
	double phi;           // angle de rotation de la caméra (coord. sphériques)
	double dist;          // distance (coord. sphériques)
} camera = { -7.0, -5.0, 30.0 };


////////////////////////////////////////
// déclaration des variables globales //
////////////////////////////////////////

// définition des lumières
glm::vec4 posLumiInit[2] = { glm::vec4( -6.5, -2.0, 14.0, 1.0 ),
							glm::vec4( 5.5, 5.5, 14.0, 1.0 ) };
glm::vec4 spotDirInit[2] = { 4.0f*glm::normalize( glm::vec4( 5.0, -2.0, -10.0, 1.0 ) ),
							4.0f*glm::normalize( glm::vec4( -5.0, -2.0, -10.0, 1.0 ) ) };
struct LightSourceParameters
{
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	glm::vec4 position[2];       // dans le repère du monde (il faudra convertir vers le repère de la caméra pour les calculs)
	glm::vec4 spotDirection[2];  // dans le repère du monde (il faudra convertir vers le repère de la caméra pour les calculs)
	float spotExposant;
	float spotAngleOuverture;    // angle d'ouverture delta du spot ([0.0,90.0] ou 180.0)
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
} LightSource = { glm::vec4( 0.1, 0.1, 0.1, 1.0 ),
				glm::vec4( 0.55, 0.55, 0.55, 1.0 ),
				glm::vec4( 0.6, 0.6, 0.6, 1.0 ),
				{ posLumiInit[0], posLumiInit[1] },
				{ spotDirInit[0], spotDirInit[1] },
				1.0,       // l'exposant du cône
				25.0,      // l'angle du cône du spot
				1., 0., 0. };

// définition du matériau
struct MaterialParameters
{
	glm::vec4 emission;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float shininess;
} FrontMaterial = { glm::vec4( 0.0, 0.0, 0.0, 1.0 ),
					glm::vec4( 0.1, 0.1, 0.1, 1.0 ),
					glm::vec4( 1.0, 0.1, 1.0, 1.0 ),
					glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
					90.0 };

struct LightModelParameters
{
	glm::vec4 ambient; // couleur ambiante
	int localViewer;   // doit-on prendre en compte la position de l'observateur? (local ou à l'infini)
	int twoSide;       // éclairage sur les deux côtés ou un seul?
} LightModel = { glm::vec4( 0.1, 0.1, 0.1, 1.0 ), false, false };

struct
{
	// partie 1: illumination
	int typeIllumination;     // 0:Gouraud, 1:Phong
	int utiliseBlinn;         // indique si on veut utiliser modèle spéculaire de Blinn ou Phong
	int utiliseDirect;        // indique si on utilise un spot style Direct3D ou OpenGL
	int afficheNormales;      // indique si on utilise les normales comme couleurs (utile pour le débogage)
	// partie 3: texture
	int texnumero;            // numéro de la texture appliquée
	int utiliseCouleur;       // doit-on utiliser la couleur de base de l'objet en plus de celle de la texture?
	int afficheTexelFonce;    // un texel foncé doit-il être affiché 0:normalement, 1:mi-coloré, 2:transparent?
} varsUnif = { 1, false, false, false,
			0, true, 0 };
// ( En GLSL, les types 'bool' et 'int' sont de la même taille, ce qui n'est pas le cas en C++.
// Ci-dessus, on triche donc un peu en déclarant les 'bool' comme des 'int', mais ça facilite la
// copie directe vers le nuanceur où les variables seront bien de type 'bool'. )


void calculerPhysique( )
{
	if ( Etat::enmouvement )
	{
		static int sensTheta = 1;
		static int sensPhi = 1;
		camera.theta += 0.3 * sensTheta;
		camera.phi += 0.5 * sensPhi;
		if ( camera.theta <= -40. || camera.theta >= 40.0 ) sensTheta = -sensTheta;
		if ( camera.phi < -40. || camera.phi > 40. ) sensPhi = -sensPhi;

		static int sensAngle = 1;
		LightSource.spotAngleOuverture += sensAngle * 0.3;
		if ( LightSource.spotAngleOuverture < 5.0 ) sensAngle = -sensAngle;
		if ( LightSource.spotAngleOuverture > 60.0 ) sensAngle = -sensAngle;

	#if 0
		static int sensExposant = 1;
		LightSource.spotExposant += sensExposant * 0.3;
		if ( LightSource.spotExposant < 1.0 ) sensExposant = -sensExposant;
		if ( LightSource.spotExposant > 10.0 ) sensExposant = -sensExposant;
	#endif

	#if 1
		// faire varier la déformation
		static int sensZ = +1;
		Etat::facteurDeform += 0.01 * sensZ;
		if ( Etat::facteurDeform < 0.1 ) sensZ = +1.0;
		else if ( Etat::facteurDeform > 1.0 ) sensZ = -1.0;
	#endif

		// De temps à autre, alterner entre le modèle d'illumination: Gouraud, Phong
		static float type = 0;
		type += 0.005;
		varsUnif.typeIllumination = fmod(type,2);

		// De temps à autre, alterner entre colorer selon couleur ou non
		varsUnif.utiliseCouleur = fmod(type+0.017,2);
	}

	camera.verifierAngles();
}

void charger1Texture( std::string fichier, GLuint &texture )
{
	unsigned char *pixels;
	GLsizei largeur, hauteur;
	if ( ( pixels = ChargerImage( fichier, largeur, hauteur ) ) != NULL )
	{
		glGenTextures( 1, &texture );
		glBindTexture( GL_TEXTURE_2D, texture );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glGenerateMipmap( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, 0 );
		delete[] pixels;
	}
}
void chargerTextures()
{
	charger1Texture( "textures/de.bmp", textures[0] );
	charger1Texture( "textures/echiquier.bmp", textures[1] );
	charger1Texture( "textures/metal.bmp", textures[2] );
	charger1Texture( "textures/mosaique.bmp", textures[3] );
}

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

	// charger le nuanceur de ce TP
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
		if ( Etat::utiliseTess )
		{
			// partie 4: À ACTIVER (touche '9')
			// attacher le nuanceur de controle de la tessellation
			const GLchar *chainesTessCtrl = ProgNuanceur::lireNuanceur( "nuanceurTessCtrl.glsl" );
			if ( chainesTessCtrl != NULL )
			{
				GLuint nuanceurObj = glCreateShader( GL_TESS_CONTROL_SHADER );
				glShaderSource( nuanceurObj, 1, &chainesTessCtrl, NULL );
				glCompileShader( nuanceurObj );
				glAttachShader( prog, nuanceurObj );
				ProgNuanceur::afficherLogCompile( nuanceurObj );
				delete [] chainesTessCtrl;
			}
			// attacher le nuanceur d'évaluation de la tessellation
			const GLchar *chainesTessEval = ProgNuanceur::lireNuanceur( "nuanceurTessEval.glsl" );
			if ( chainesTessEval != NULL )
			{
				GLuint nuanceurObj = glCreateShader( GL_TESS_EVALUATION_SHADER );
				glShaderSource( nuanceurObj, 1, &chainesTessEval, NULL );
				glCompileShader( nuanceurObj );
				glAttachShader( prog, nuanceurObj );
				ProgNuanceur::afficherLogCompile( nuanceurObj );
				delete [] chainesTessEval;
			}
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
		if ( !ProgNuanceur::afficherLogLink( prog ) ) exit(1);

		// demander la "Location" des variables
		if ( ( locVertex = glGetAttribLocation( prog, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
		if ( ( locNormal = glGetAttribLocation( prog, "Normal" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Normal (partie 1)" << std::endl;
		if ( ( locTexCoord = glGetAttribLocation( prog, "TexCoord" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de TexCoord (partie 3)" << std::endl;
		if ( ( locmatrModel = glGetUniformLocation( prog, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
		if ( ( locmatrVisu = glGetUniformLocation( prog, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
		if ( ( locmatrProj = glGetUniformLocation( prog, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
		if ( ( locmatrNormale = glGetUniformLocation( prog, "matrNormale" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrNormale (partie 1)" << std::endl;
		if ( ( loclaTexture = glGetUniformLocation( prog, "laTexture" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de laTexture (partie 3)" << std::endl;
		// partie 4:
		if ( Etat::utiliseTess )
		{
			if ( ( locfacteurDeform = glGetUniformLocation( prog, "facteurDeform" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de facteurDeform" << std::endl;
			if ( ( locTessLevelInner = glGetUniformLocation( prog, "TessLevelInner" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de TessLevelInner (partie 4)" << std::endl;
			if ( ( locTessLevelOuter = glGetUniformLocation( prog, "TessLevelOuter" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de TessLevelOuter (partie 4)" << std::endl;
		}
		if ( ( indLightSource = glGetUniformBlockIndex( prog, "LightSourceParameters" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de LightSource" << std::endl;
		if ( ( indFrontMaterial = glGetUniformBlockIndex( prog, "MaterialParameters" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de FrontMaterial" << std::endl;
		if ( ( indLightModel = glGetUniformBlockIndex( prog, "LightModelParameters" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de LightModel" << std::endl;
		if ( ( indvarsUnif = glGetUniformBlockIndex( prog, "varsUnif" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de varsUnif" << std::endl;

		// charger les ubo
		{
			glBindBuffer( GL_UNIFORM_BUFFER, ubo[0] );
			glBufferData( GL_UNIFORM_BUFFER, sizeof(LightSource), &LightSource, GL_DYNAMIC_COPY );
			glBindBuffer( GL_UNIFORM_BUFFER, 0 );
			const GLuint bindingIndex = 0;
			glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[0] );
			glUniformBlockBinding( prog, indLightSource, bindingIndex );
		}
		{
			glBindBuffer( GL_UNIFORM_BUFFER, ubo[1] );
			glBufferData( GL_UNIFORM_BUFFER, sizeof(FrontMaterial), &FrontMaterial, GL_DYNAMIC_COPY );
			glBindBuffer( GL_UNIFORM_BUFFER, 0 );
			const GLuint bindingIndex = 1;
			glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[1] );
			glUniformBlockBinding( prog, indFrontMaterial, bindingIndex );
		}
		{
			glBindBuffer( GL_UNIFORM_BUFFER, ubo[2] );
			glBufferData( GL_UNIFORM_BUFFER, sizeof(LightModel), &LightModel, GL_DYNAMIC_COPY );
			glBindBuffer( GL_UNIFORM_BUFFER, 0 );
			const GLuint bindingIndex = 2;
			glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[2] );
			glUniformBlockBinding( prog, indLightModel, bindingIndex );
		}
		{
			glBindBuffer( GL_UNIFORM_BUFFER, ubo[3] );
			glBufferData( GL_UNIFORM_BUFFER, sizeof(varsUnif), &varsUnif, GL_DYNAMIC_COPY );
			glBindBuffer( GL_UNIFORM_BUFFER, 0 );
			const GLuint bindingIndex = 3;
			glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[3] );
			glUniformBlockBinding( prog, indvarsUnif, bindingIndex );
		}
	}
}

// initialisation d'openGL
void FenetreTP::initialiser()
{
	// couleur de l'arrière-plan
	glClearColor( 0.4, 0.2, 0.0, 1.0 );

	// activer les etats openGL
	glEnable( GL_DEPTH_TEST );

	// charger les textures
	chargerTextures();

	// allouer les UBO pour les variables uniformes
	glGenBuffers( 4, ubo );

	// charger les nuanceurs
	chargerNuanceurs();
	glUseProgram( prog );

	// partie 1: créer le cube
	/*         +Y                    */
	/*   3+-----------+2             */
	/*    |\          |\             */
	/*    | \         | \            */
	/*    |  \        |  \           */
	/*    |  7+-----------+6         */
	/*    |   |       |   |          */
	/*    |   |       |   |          */
	/*   0+---|-------+1  |          */
	/*     \  |        \  |     +X   */
	/*      \ |         \ |          */
	/*       \|          \|          */
	/*       4+-----------+5         */
	/*             +Z                */

	GLfloat sommets[3*4*6] =
	{
		-1.0,  1.0, -1.0,    1.0,  1.0, -1.0,  -1.0, -1.0, -1.0,    1.0, -1.0, -1.0,   // P3,P2,P0,P1
		 1.0, -1.0,  1.0,   -1.0, -1.0,  1.0,   1.0, -1.0, -1.0,   -1.0, -1.0, -1.0,   // P5,P4,P1,P0
		 1.0,  1.0,  1.0,    1.0, -1.0,  1.0,   1.0,  1.0, -1.0,    1.0, -1.0, -1.0,   // P6,P5,P2,P1
		-1.0,  1.0,  1.0,    1.0,  1.0,  1.0,  -1.0,  1.0, -1.0,    1.0,  1.0, -1.0,   // P7,P6,P3,P2
		-1.0, -1.0,  1.0,   -1.0,  1.0,  1.0,  -1.0, -1.0, -1.0,   -1.0,  1.0, -1.0,   // P4,P7,P0,P3
		-1.0, -1.0,  1.0,    1.0, -1.0,  1.0,  -1.0,  1.0,  1.0,    1.0,  1.0,  1.0    // P4,P5,P7,P6
	};

	GLfloat normales[3*4*6] =
	{
		 0.0,  0.0, -1.0,        0.0,  0.0, -1.0,        0.0,  0.0, -1.0,        0.0,  0.0, -1.0,
		 0.0, -1.0,  0.0,        0.0, -1.0,  0.0,        0.0, -1.0,  0.0,        0.0, -1.0,  0.0,
		 1.0,  0.0,  0.0,        1.0,  0.0,  0.0,        1.0,  0.0,  0.0,        1.0,  0.0,  0.0,
		 0.0,  1.0,  0.0,        0.0,  1.0,  0.0,        0.0,  1.0,  0.0,        0.0,  1.0,  0.0,
		-1.0,  0.0,  0.0,       -1.0,  0.0,  0.0,       -1.0,  0.0,  0.0,       -1.0,  0.0,  0.0,
		 0.0,  0.0,  1.0,        0.0,  0.0,  1.0,        0.0,  0.0,  1.0,        0.0,  0.0,  1.0
	};
	
	
	GLfloat cootex1[2*4*6] =
	{
		2.0 / 3.0,	0.0,			2.0 / 3.0,	1.0 / 3.0,		1.0, 		0.0,			1.0,		1.0 / 3.0,
		0.0,		1.0 / 3.0,		0.0,		2.0 / 3.0,		1.0 / 3.0,	1.0 / 3.0,		1.0 / 3.0,	2.0 / 3.0,
		1.0 / 3.0,	2.0 / 3.0,		1.0 / 3.0,	1.0,			2.0 / 3.0,	2.0 / 3.0,		2.0 / 3.0,	1.0,
		2.0 / 3.0,	1.0 / 3.0,		2.0 / 3.0,	2.0 / 3.0,		1.0,		1.0 / 3.0,		1.0,		2.0 / 3.0,
		1.0 / 3.0,	0.0,			1.0 / 3.0,	1.0 / 3.0,		2.0 / 3.0,	0.0,			2.0 / 3.0,	1.0 / 3.0,
		1.0 / 3.0,	1.0 / 3.0,		1.0 / 3.0,	2.0 / 3.0,		2.0 / 3.0,	1.0 / 3.0,		2.0 / 3.0,	2.0 / 3.0,
	};
	
	GLfloat cootex2[2*4*6] =
	{
		0.0,	0.0,		0.0,	3.0,		3.0,	0.0,		3.0,	3.0,
		0.0,	0.0,		0.0,	3.0,		3.0,	0.0,		3.0,	3.0,
		0.0,	0.0,		0.0,	3.0,		3.0,	0.0,		3.0,	3.0,
		0.0,	0.0,		0.0,	3.0,		3.0,	0.0,		3.0,	3.0,
		0.0,	0.0,		0.0,	3.0,		3.0,	0.0,		3.0,	3.0,
		0.0,	0.0,		0.0,	3.0,		3.0,	0.0,		3.0,	3.0
	};
	
	GLfloat cootex3[2*4*6] =
	{
		0.0,	0.0,		0.0,	1.0,		1.0,	0.0,		1.0,	1.0,
		0.0,	0.0,		0.0,	1.0,		1.0,	0.0,		1.0,	1.0,
		0.0,	0.0,		0.0,	1.0,		1.0,	0.0,		1.0,	1.0,
		0.0,	0.0,		0.0,	1.0,		1.0,	0.0,		1.0,	1.0,
		0.0,	0.0,		0.0,	1.0,		1.0,	0.0,		1.0,	1.0,
		0.0,	0.0,		0.0,	1.0,		1.0,	0.0,		1.0,	1.0
	};

	// allouer les objets OpenGL
	glGenVertexArrays( 2, vao );
	glGenBuffers( 5, vbo );
	// initialiser le VAO
	glBindVertexArray( vao[0] );

	// charger le VBO pour les sommets
	glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(sommets), sommets, GL_STATIC_DRAW );
	glVertexAttribPointer( locVertex, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	glEnableVertexAttribArray(locVertex);

	// partie 1: charger le VBO pour les normales
	// ...
	glBindBuffer( GL_ARRAY_BUFFER, vbo[1] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(normales), normales, GL_STATIC_DRAW );
	glVertexAttribPointer( locNormal, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	glEnableVertexAttribArray(locNormal);

	// partie 3: charger le VBO pour les coordonnées de texture
	// ...
	glBindBuffer( GL_ARRAY_BUFFER, vbo[2] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(cootex1), cootex1, GL_STATIC_DRAW );
	glVertexAttribPointer( locTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0 );
	glEnableVertexAttribArray(locTexCoord);

	glBindBuffer( GL_ARRAY_BUFFER, vbo[3] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(cootex2), cootex2, GL_STATIC_DRAW );
	glVertexAttribPointer( locTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0 );
	glEnableVertexAttribArray(locTexCoord);

	glBindBuffer( GL_ARRAY_BUFFER, vbo[4] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(cootex3), cootex3, GL_STATIC_DRAW );
	glVertexAttribPointer( locTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0 );
	glEnableVertexAttribArray(locTexCoord);

	glBindVertexArray(0);

	// initialiser le VAO pour une ligne (montrant la direction du spot)
	glGenBuffers( 1, &vboLumi );
	glBindVertexArray( vao[1] );
	GLfloat coords[] = { 0., 0., 0., 0., 0., 1. };
	glBindBuffer( GL_ARRAY_BUFFER, vboLumi );
	glBufferData( GL_ARRAY_BUFFER, sizeof(coords), coords, GL_STATIC_DRAW );
	glVertexAttribPointer( locVertexBase, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	glEnableVertexAttribArray(locVertexBase);
	glBindVertexArray(0);

	// créer quelques autres formes
	sphere = new FormeSphere( 1.0, 32, 32 );
	sphereLumi = new FormeSphere( 0.5, 10, 10 );
	theiere = new FormeTheiere( );
	tore = new FormeTore( 0.4, 0.8, 32, 32 );
	cylindre = new FormeCylindre( 0.3, 0.3, 3.0, 32, 32 );
	cone = new FormeCylindre( 0.0, 0.5, 3.0, 32, 32 );
}

void FenetreTP::conclure()
{
	glUseProgram( 0 );
	glDeleteVertexArrays( 2, vao );
	glDeleteBuffers( 5, vbo );
	glDeleteBuffers( 1, &vboLumi );
	glDeleteBuffers( 4, ubo );
	delete sphere;
	delete sphereLumi;
	delete theiere;
	delete tore;
	delete cylindre;
	delete cone;
}

void afficherModele()
{
	glBindVertexArray(vao[0]);
	// partie 3: paramètres de texture
	switch ( varsUnif.texnumero )
	{
	default:
		glBindTexture( GL_TEXTURE_2D, 0);
		//std::cout << "Sans texture" << std::endl;
		break;
	case 1:
		glBindBuffer( GL_ARRAY_BUFFER, vbo[2]);
		glVertexAttribPointer( locTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0 );
		glBindTexture( GL_TEXTURE_2D, textures[0]);
		//std::cout << "Texture 1 DE" << std::endl;
		break;
	case 2:
		glBindBuffer( GL_ARRAY_BUFFER, vbo[3]);
		glVertexAttribPointer( locTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0 );
		glBindTexture( GL_TEXTURE_2D, textures[1]);
		//std::cout << "Texture 2 ECHIQUIER" << std::endl;
		break;
	case 3:
		glBindBuffer( GL_ARRAY_BUFFER, vbo[4]);
		glVertexAttribPointer( locTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0 );
		glBindTexture( GL_TEXTURE_2D, textures[2]);
		//std::cout << "Texture 3 METAL" << std::endl;
		break;
	case 4:
		glBindBuffer( GL_ARRAY_BUFFER, vbo[4]);
		glVertexAttribPointer( locTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0 );
		glBindTexture( GL_TEXTURE_2D, textures[3]);
		//std::cout << "Texture 4 MOSAIQUE" << std::endl;
		break;
	}
	



	// Dessiner le modèle
	matrModel.PushMatrix(); {

		// mise à l'échelle
		matrModel.Scale( 5.0, 5.0, 5.0 );

		glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
		// (partie 1: ne pas oublier de calculer et donner une matrice pour les transformations des normales)
		glUniformMatrix3fv( locmatrNormale, 1, GL_TRUE, glm::value_ptr( glm::inverse( glm::mat3( matrVisu.getMatr() * matrModel.getMatr() ) ) ) );

		glPatchParameteri( GL_PATCH_VERTICES, 4 );
		switch ( Etat::modele )
		{
		default:
		case 1:
			// afficher le cube
			glBindVertexArray( vao[0] );
			if ( Etat::utiliseTess )
			{
				// partie 4: afficher le cube avec des GL_PATCHES
			}
			else
			{
				glDrawArrays( GL_TRIANGLE_STRIP,  0, 4 );
				glDrawArrays( GL_TRIANGLE_STRIP,  4, 4 );
				glDrawArrays( GL_TRIANGLE_STRIP,  8, 4 );
				glDrawArrays( GL_TRIANGLE_STRIP, 12, 4 );
				glDrawArrays( GL_TRIANGLE_STRIP, 16, 4 );
				glDrawArrays( GL_TRIANGLE_STRIP, 20, 4 );
			}
			glBindVertexArray( 0 );
			break;
		case 2:
			tore->afficher();
			break;
		case 3:
			sphere->afficher();
			break;
		case 4:
			matrModel.Rotate( -90.0, 1.0, 0.0, 0.0 );
			matrModel.Translate( 0.0, 0.0, -0.5 );
			matrModel.Scale( 0.5, 0.5, 0.5 );
			glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
			glUniformMatrix3fv( locmatrNormale, 1, GL_TRUE, glm::value_ptr( glm::inverse( glm::mat3( matrVisu.getMatr() * matrModel.getMatr() ) ) ) );
			theiere->afficher( );
			break;
		case 5:
			matrModel.PushMatrix(); {
				matrModel.Translate( 0.0, 0.0, -1.5 );
				glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
				glUniformMatrix3fv( locmatrNormale, 1, GL_TRUE, glm::value_ptr( glm::inverse( glm::mat3( matrVisu.getMatr() * matrModel.getMatr() ) ) ) );
				cylindre->afficher();
			} matrModel.PopMatrix();
			break;
		case 6:
			matrModel.PushMatrix(); {
				matrModel.Translate( 0.0, 0.0, -1.5 );
				glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
				glUniformMatrix3fv( locmatrNormale, 1, GL_TRUE, glm::value_ptr( glm::inverse( glm::mat3( matrVisu.getMatr() * matrModel.getMatr() ) ) ) );
				cone->afficher();
			} matrModel.PopMatrix();
			break;
		}
	} matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
}

void afficherLumiere()
{
	// Dessiner les lumières
	for ( int i = 0 ; i < 2 ; ++i )
	{
		// dessiner une ligne vers la source lumineuse
		GLfloat coords[] =
		{
			LightSource.position[i].x                               , LightSource.position[i].y                               , LightSource.position[i].z,
			LightSource.position[i].x+LightSource.spotDirection[i].x, LightSource.position[i].y+LightSource.spotDirection[i].y, LightSource.position[i].z+LightSource.spotDirection[i].z
		};
		glLineWidth( 3.0 );
		glVertexAttrib3f( locColorBase, 1.0, 1.0, 0.5 ); // jaune
		glBindVertexArray( vao[1] );
		matrModel.PushMatrix(); {
			glBindBuffer( GL_ARRAY_BUFFER, vboLumi );
			glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(coords), coords );
			glDrawArrays( GL_LINES, 0, 2 );
		} matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
		glBindVertexArray( 0 );
		glLineWidth( 1.0 );

		// dessiner une sphère à la position de la lumière
		matrModel.PushMatrix(); {
			matrModel.Translate( LightSource.position[i].x, LightSource.position[i].y, LightSource.position[i].z );
			glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
			sphereLumi->afficher();
		} matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
	}
}

// fonction d'affichage
void FenetreTP::afficherScene()
{
	// effacer l'ecran et le tampon de profondeur
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glUseProgram( progBase );

	// définir le pipeline graphique
	if ( Etat::enPerspective )
	{
		matrProj.Perspective( 35.0, (GLdouble)largeur_ / (GLdouble)hauteur_,
								0.1, 60.0 );
	}
	else
	{
		const GLfloat d = 8.0;
		if ( largeur_ <= hauteur_ )
		{
			matrProj.Ortho( -d, d,
							-d*(GLdouble)hauteur_ / (GLdouble)largeur_,
							d*(GLdouble)hauteur_ / (GLdouble)largeur_,
							0.1, 60.0 );
		}
		else
		{
			matrProj.Ortho( -d*(GLdouble)largeur_ / (GLdouble)hauteur_,
							d*(GLdouble)largeur_ / (GLdouble)hauteur_,
							-d, d,
							0.1, 60.0 );
		}
	}
	glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj );

	camera.definir();
	glUniformMatrix4fv( locmatrVisuBase, 1, GL_FALSE, matrVisu );

	matrModel.LoadIdentity();
	glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
		glUniformMatrix3fv( locmatrNormale, 1, GL_TRUE, glm::value_ptr( glm::inverse( glm::mat3( matrVisu.getMatr() * matrModel.getMatr() ) ) ) );

	// afficher les axes
	if ( Etat::afficheAxes ) FenetreTP::afficherAxes( 8.0 );

	// dessiner la scène
	afficherLumiere();

	glUseProgram( prog );

	// mettre à jour les blocs de variables uniformes
	{
		glBindBuffer( GL_UNIFORM_BUFFER, ubo[0] );
		GLvoid *p = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
		memcpy( p, &LightSource, sizeof(LightSource) );
		glUnmapBuffer( GL_UNIFORM_BUFFER );
	}
	{
		glBindBuffer( GL_UNIFORM_BUFFER, ubo[1] );
		GLvoid *p = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
		memcpy( p, &FrontMaterial, sizeof(FrontMaterial) );
		glUnmapBuffer( GL_UNIFORM_BUFFER );
	}
	{
		glBindBuffer( GL_UNIFORM_BUFFER, ubo[2] );
		GLvoid *p = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
		memcpy( p, &LightModel, sizeof(LightModel) );
		glUnmapBuffer( GL_UNIFORM_BUFFER );
	}
	{
		glBindBuffer( GL_UNIFORM_BUFFER, ubo[3] );
		GLvoid *p = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
		memcpy( p, &varsUnif, sizeof(varsUnif) );
		glUnmapBuffer( GL_UNIFORM_BUFFER );
	}

	// mettre à jour les matrices et autres uniformes
	glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
	glUniformMatrix4fv( locmatrVisu, 1, GL_FALSE, matrVisu );
	glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
	//glActiveTexture( GL_TEXTURE0 ); // activer la texture '0' (valeur de défaut)
	glUniform1i( loclaTexture, 0 ); // '0' => utilisation de GL_TEXTURE0
	glUniform1f( locfacteurDeform, Etat::facteurDeform );
	glUniform1f( locTessLevelInner, Etat::TessLevelInner );
	glUniform1f( locTessLevelOuter, Etat::TessLevelOuter );

	afficherModele();
}

// fonction de redimensionnement de la fenêtre graphique
void FenetreTP::redimensionner( GLsizei w, GLsizei h )
{
	glViewport( 0, 0, w, h );
}

static void echoEtats( )
{
	static std::string illuminationStr[] = { "0:Gouraud", "1:Phong" };
	static std::string reflexionStr[] = { "0:Phong", "1:Blinn" };
	static std::string spotStr[] = { "0:OpenGL", "1:Direct3D" };
	std::cout << " modèle d'illumination= " << illuminationStr[varsUnif.typeIllumination]
				<< ", refléxion spéculaire= " << reflexionStr[varsUnif.utiliseBlinn]
				<< ", spot= " << spotStr[varsUnif.utiliseDirect]
				<< std::endl;
}

// fonction de gestion du clavier
void FenetreTP::clavier( TP_touche touche )
{
	// traitement des touches q et echap
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

	case TP_9: // Permuter l'utilisation des nuanceurs de tessellation
		Etat::utiliseTess = !Etat::utiliseTess;
		std::cout << " Etat::utiliseTess=" << Etat::utiliseTess << std::endl;
		// recréer les nuanceurs
		chargerNuanceurs();
		break;

	case TP_v: // Recharger les fichiers des nuanceurs et recréer le programme
		chargerNuanceurs();
		std::cout << "// Recharger nuanceurs" << std::endl;
		break;

	case TP_i: // Augmenter le niveau de tessellation interne
		++Etat::TessLevelInner;
		std::cout << " TessLevelInner=" << Etat::TessLevelInner << " TessLevelOuter=" << Etat::TessLevelOuter << std::endl;
		glPatchParameteri( GL_PATCH_DEFAULT_INNER_LEVEL, Etat::TessLevelInner );
		break;
	case TP_k: // Diminuer le niveau de tessellation interne
		if ( --Etat::TessLevelInner < 1 ) Etat::TessLevelInner = 1;
		std::cout << " TessLevelInner=" << Etat::TessLevelInner << " TessLevelOuter=" << Etat::TessLevelOuter << std::endl;
		glPatchParameteri( GL_PATCH_DEFAULT_INNER_LEVEL, Etat::TessLevelInner );
		break;

	case TP_o: // Augmenter le niveau de tessellation externe
		++Etat::TessLevelOuter;
		std::cout << " TessLevelInner=" << Etat::TessLevelInner << " TessLevelOuter=" << Etat::TessLevelOuter << std::endl;
		glPatchParameteri( GL_PATCH_DEFAULT_OUTER_LEVEL, Etat::TessLevelOuter );
		break;
	case TP_l: // Diminuer le niveau de tessellation externe
		if ( --Etat::TessLevelOuter < 1 ) Etat::TessLevelOuter = 1;
		std::cout << " TessLevelInner=" << Etat::TessLevelInner << " TessLevelOuter=" << Etat::TessLevelOuter << std::endl;
		glPatchParameteri( GL_PATCH_DEFAULT_OUTER_LEVEL, Etat::TessLevelOuter );
		break;

	case TP_u: // Augmenter les deux niveaux de tessellation
		++Etat::TessLevelOuter;
		Etat::TessLevelInner = Etat::TessLevelOuter;
		std::cout << " TessLevelInner=" << Etat::TessLevelInner << " TessLevelOuter=" << Etat::TessLevelOuter << std::endl;
		glPatchParameteri( GL_PATCH_DEFAULT_OUTER_LEVEL, Etat::TessLevelOuter );
		glPatchParameteri( GL_PATCH_DEFAULT_INNER_LEVEL, Etat::TessLevelInner );
		break;
	case TP_j: // Diminuer les deux niveaux de tessellation
		if ( --Etat::TessLevelOuter < 1 ) Etat::TessLevelOuter = 1;
		Etat::TessLevelInner = Etat::TessLevelOuter;
		std::cout << " TessLevelInner=" << Etat::TessLevelInner << " TessLevelOuter=" << Etat::TessLevelOuter << std::endl;
		glPatchParameteri( GL_PATCH_DEFAULT_OUTER_LEVEL, Etat::TessLevelOuter );
		glPatchParameteri( GL_PATCH_DEFAULT_INNER_LEVEL, Etat::TessLevelInner );
		break;

	case TP_p: // Permuter la projection: perspective ou orthogonale
		Etat::enPerspective = !Etat::enPerspective;
		break;

	case TP_w: // Alterner entre le modèle d'illumination: Gouraud, Phong
		if ( ++varsUnif.typeIllumination > 1 ) varsUnif.typeIllumination = 0;
		echoEtats( );
		break;

	case TP_r: // Alterner entre le modèle de réflexion spéculaire: Phong, Blinn
		varsUnif.utiliseBlinn = !varsUnif.utiliseBlinn;
		echoEtats( );
		break;

	case TP_s: // Alterner entre le modèle de spot: OpenGL, Direct3D
		varsUnif.utiliseDirect = !varsUnif.utiliseDirect;
		echoEtats( );
		break;

	//case TP_b: // Alterner entre une caméra locale à la scène ou distante (localViewer)
	//   LightModel.localViewer = !LightModel.localViewer;
	//   std::cout << " localViewer=" << LightModel.localViewer << std::endl;
	//   break;

	case TP_a: // Incrémenter l'angle d'ouverture du cône du spot
	case TP_EGAL:
	case TP_PLUS:
		LightSource.spotAngleOuverture += 2.0;
		if ( LightSource.spotAngleOuverture > 90.0 ) LightSource.spotAngleOuverture = 90.0;
		std::cout <<  " spotAngleOuverture=" << LightSource.spotAngleOuverture << std::endl;
		break;
	case TP_z: // Décrémenter l'angle d'ouverture du cône du spot
	case TP_MOINS:
	case TP_SOULIGNE:
		LightSource.spotAngleOuverture -= 2.0;
		if ( LightSource.spotAngleOuverture < 0.0 ) LightSource.spotAngleOuverture = 0.0;
		std::cout <<  " spotAngleOuverture=" << LightSource.spotAngleOuverture << std::endl;
		break;

	case TP_d: // Incrémenter l'exposant du spot
		LightSource.spotExposant += 0.3;
		if ( LightSource.spotExposant > 89.0 ) LightSource.spotExposant = 89.0;
		std::cout <<  " spotExposant=" << LightSource.spotExposant << std::endl;
		break;
	case TP_e: // Décrémenter l'exposant du spot
		LightSource.spotExposant -= 0.3;
		if ( LightSource.spotExposant < 0.0 ) LightSource.spotExposant = 0.0;
		std::cout <<  " spotExposant=" << LightSource.spotExposant << std::endl;
		break;

	case TP_y: // Incrémenter le coefficient de brillance
	case TP_CROCHETDROIT:
		FrontMaterial.shininess *= 1.1;
		std::cout << " FrontMaterial.shininess=" << FrontMaterial.shininess << std::endl;
		break;
	case TP_h: // Décrémenter le coefficient de brillance
	case TP_CROCHETGAUCHE:
		FrontMaterial.shininess /= 1.1; if ( FrontMaterial.shininess < 0.0 ) FrontMaterial.shininess = 0.0;
		std::cout << " FrontMaterial.shininess=" << FrontMaterial.shininess << std::endl;
		break;

	case TP_m: // Choisir le modèle affiché: cube, tore, sphère, théière, cylindre, cône
		if ( ++Etat::modele > 6 ) Etat::modele = 1;
		std::cout << " Etat::modele=" << Etat::modele << std::endl;
		break;

	case TP_0: // Replacer Caméra et Lumière afin d'avoir une belle vue
		camera.theta = -7.0; camera.phi = -5.0; camera.dist = 30.0;
		LightSource.position[0] = posLumiInit[0];
		LightSource.position[1] = posLumiInit[1];
		LightSource.spotDirection[0] = spotDirInit[0];
		LightSource.spotDirection[1] = spotDirInit[1];
		break;

	case TP_t: // Choisir la texture utilisée: aucune, dé, échiquier, métal, mosaique
		varsUnif.texnumero++;
		if ( varsUnif.texnumero > 4 ) varsUnif.texnumero = 0;
		std::cout << " varsUnif.texnumero=" << varsUnif.texnumero << std::endl;
		break;

	case TP_c: // Changer l'affichage de l'objet texturé avec couleur ou sans couleur
		varsUnif.utiliseCouleur = !varsUnif.utiliseCouleur;
		std::cout << " utiliseCouleur=" << varsUnif.utiliseCouleur << std::endl;
		break;

	case TP_f: // Changer l'affichage des texels foncés (normal, mi-coloré, transparent)
		varsUnif.afficheTexelFonce++;
		if ( varsUnif.afficheTexelFonce > 2 ) varsUnif.afficheTexelFonce = 0;
		std::cout << " afficheTexelFonce=" << varsUnif.afficheTexelFonce << std::endl;
		break;

	case TP_POINT: // Augmenter l'effet du déplacement
		Etat::facteurDeform += 0.01;
		std::cout << " facteurDeform=" << Etat::facteurDeform << std::endl;
		break;
	case TP_BARREOBLIQUE: // Diminuer l'effet du déplacement
		Etat::facteurDeform -= 0.01;
		std::cout << " facteurDeform=" << Etat::facteurDeform << std::endl;
		break;

	case TP_g: // Permuter l'affichage en fil de fer ou plein
		Etat::modePolygone = ( Etat::modePolygone == GL_FILL ) ? GL_LINE : GL_FILL;
		glPolygonMode( GL_FRONT_AND_BACK, Etat::modePolygone );
		break;

	case TP_n: // Utiliser ou non les normales calculées comme couleur (pour le débogage)
		varsUnif.afficheNormales = !varsUnif.afficheNormales;
		break;

	case TP_ESPACE: // Permuter la rotation automatique du modèle
		Etat::enmouvement = !Etat::enmouvement;
		break;

	default:
		std::cout << " touche inconnue : " << (char) touche << std::endl;
		imprimerTouches();
		break;
	}

}

// fonction callback pour un clic de souris
// la dernière position de la souris
static enum { deplaceCam, deplaceSpotDirection, deplaceSpotPosition } deplace = deplaceCam;
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
		case TP_BOUTON_GAUCHE: // Tourner l'objet
			deplace = deplaceCam;
			break;
		case TP_BOUTON_MILIEU: // Modifier l'orientation du spot
			deplace = deplaceSpotDirection;
			break;
		case TP_BOUTON_DROIT: // Déplacer la lumière
			deplace = deplaceSpotPosition;
			break;
		}
		if ( deplace != deplaceCam )
		{
			glm::mat4 M = matrModel;
			glm::mat4 V = matrVisu;
			glm::mat4 P = matrProj;
			glm::vec4 cloture( 0, 0, largeur_, hauteur_ );
			glm::vec2 ecranLumi0 = glm::vec2( glm::project( glm::vec3(LightSource.position[0]), V*M, P, cloture ) );
			glm::vec2 ecranLumi1 = glm::vec2( glm::project( glm::vec3(LightSource.position[1]), V*M, P, cloture ) );
			glm::vec2 ecranXY( x, hauteur_-y );
			Etat::curLumi = ( glm::distance( ecranLumi0, ecranXY ) <
							glm::distance( ecranLumi1, ecranXY ) ) ? 0 : 1;
		}
	}
	else
	{
		// on vient de relâcher la souris
	}
}

void FenetreTP::sourisMolette( int x, int y ) // Changer la taille du spot
{
	const int sens = +1;
	LightSource.spotAngleOuverture += sens*y;
	if ( LightSource.spotAngleOuverture > 90.0 ) LightSource.spotAngleOuverture = 90.0;
	if ( LightSource.spotAngleOuverture < 0.0 ) LightSource.spotAngleOuverture = 0.0;
	std::cout <<  " spotAngleOuverture=" << LightSource.spotAngleOuverture << std::endl;
}

// fonction de mouvement de la souris
void FenetreTP::sourisMouvement( int x, int y )
{
	if ( pressed )
	{
		int dx = x - Etat::sourisPosPrec.x;
		int dy = y - Etat::sourisPosPrec.y;
		glm::mat4 M = matrModel;
		glm::mat4 V = matrVisu;
		glm::mat4 P = matrProj;
		glm::vec4 cloture( 0, 0, largeur_, hauteur_ );
		switch ( deplace )
		{
		case deplaceCam:
			camera.theta -= dx / 3.0;
			camera.phi   -= dy / 3.0;
			// std::cout << " camera.theta=" << camera.theta << " camera.phi=" << camera.phi << std::endl;
			break;
		case deplaceSpotDirection:
			{
				//LightSource.spotDirection[Etat::curLumi].x += 0.06 * dx;
				//LightSource.spotDirection[Etat::curLumi].y -= 0.06 * dy;
				// obtenir les coordonnées d'écran correspondant à la pointe du vecteur
				glm::vec3 ecranLumi = glm::project( glm::vec3(LightSource.position[Etat::curLumi]+LightSource.spotDirection[Etat::curLumi]), V*M, P, cloture );
				// définir la nouvelle position (en utilisant la profondeur actuelle)
				glm::vec3 ecranPos( x, hauteur_-y, ecranLumi[2] );
				// placer la lumière à cette nouvelle position
				LightSource.spotDirection[Etat::curLumi] = glm::vec4( glm::unProject( ecranPos, V*M, P, cloture ), 1.0 ) - LightSource.position[Etat::curLumi];
				// normaliser sa longueur
				LightSource.spotDirection[Etat::curLumi] = 4.0f*glm::normalize( LightSource.spotDirection[Etat::curLumi] );
				// std::cout << " LightSource.spotDirection[Etat::curLumi]=" << glm::to_string(LightSource.spotDirection[Etat::curLumi]) << std::endl;
			}
			break;
		case deplaceSpotPosition:
			{
				// obtenir les coordonnées d'écran correspondant à la position de la lumière
				glm::vec3 ecranLumi = glm::project( glm::vec3(LightSource.position[Etat::curLumi]), V*M, P, cloture );
				// définir la nouvelle position (en utilisant la profondeur actuelle)
				glm::vec3 ecranPos( x, hauteur_-y, ecranLumi[2] );
				// placer la lumière à cette nouvelle position
				LightSource.position[Etat::curLumi] = glm::vec4( glm::unProject( ecranPos, V*M, P, cloture ), 1.0 );
				// std::cout << " LightSource.position[Etat::curLumi]=" << glm::to_string(LightSource.position[Etat::curLumi]) << std::endl;
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
