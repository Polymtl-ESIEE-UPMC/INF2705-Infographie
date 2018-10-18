#version 410

// Définition des paramètres des sources de lumière
layout (std140) uniform LightSourceParameters
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position[2];      // dans le repère du monde
	vec3 spotDirection[2]; // dans le repère du monde
	float spotExponent;
	float spotAngleOuverture; // ([0.0,90.0] ou 180.0)
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
} LightSource;

// Définition des paramètres des matériaux
layout (std140) uniform MaterialParameters
{
	vec4 emission;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
} FrontMaterial;

// Définition des paramètres globaux du modèle de lumière
layout (std140) uniform LightModelParameters
{
	vec4 ambient;       // couleur ambiante
	bool localViewer;   // observateur local ou à l'infini?
	bool twoSide;       // éclairage sur les deux côtés ou un seul?
} LightModel;

layout (std140) uniform varsUnif
{
	// partie 1: illumination
	int typeIllumination;     // 0:Gouraud, 1:Phong
	bool utiliseBlinn;        // indique si on veut utiliser modèle spéculaire de Blinn ou Phong
	bool utiliseDirect;       // indique si on utilise un spot style Direct3D ou OpenGL
	bool afficheNormales;     // indique si on utilise les normales comme couleurs (utile pour le débogage)
	// partie 3: texture
	int texnumero;            // numéro de la texture appliquée
	bool utiliseCouleur;      // doit-on utiliser la couleur de base de l'objet en plus de celle de la texture?
	int afficheTexelFonce;    // un texel noir doit-il être affiché 0:noir, 1:mi-coloré, 2:transparent?
};

uniform sampler2D laTexture;

/////////////////////////////////////////////////////////////////

in Attribs {
	vec4 couleur;
	vec3 lumiDir[2];
	vec3 normale;
	vec3 obsVec;
} AttribsIn;

out vec4 FragColor;

float calculerSpot( in vec3 D, in vec3 L )
{
	float spotFacteur = 1.0;
	return( spotFacteur );
}

vec4 calculerReflexion( in vec3 L, in vec3 N, in vec3 O )
{
	vec4 grisUniforme = vec4(0.7,0.7,0.7,1.0);

	// ajout de l’émission et du terme ambiant du modèle d’illumination
	grisUniforme = FrontMaterial.emission + FrontMaterial.ambient * LightModel.ambient;

	// calcul de la composante ambiante de la 1er source de lumière
	grisUniforme += FrontMaterial.ambient * LightSource.ambient;

	// produit scalaire pour le calcul de la réflexion diffuse
	float NdotL = max( 0.0, dot( N, L ) );

	// calcul de la composante diffuse de la 1er source de lumière
	grisUniforme += FrontMaterial.diffuse * LightSource.diffuse * NdotL;

	// calcul de la composante spéculaire (selon Phong ou Blinn)
	float NdotHV = max ( 0.0, dot( normalize( L + O ), N) ); // avec B et N

	// calcul de la composante spéculaire de la 1er source de lumière
	grisUniforme += FrontMaterial.specular * LightSource.specular * pow( NdotHV, FrontMaterial.shininess );

	return( grisUniforme );
}

void main( void )
{
	vec3 L[2];
	for (int i = 0; i < AttribsIn.lumiDir.length; ++i)
	{
		L[i] = normalize( AttribsIn.lumiDir[i] ); // vecteur vers la lumière
	} 
	vec3 N = normalize( AttribsIn.normale ); // vecteur normale
	vec3 O = normalize( AttribsIn.obsVec );  // position de l'observateur

	// assigner la couleur finale
	FragColor = AttribsIn.couleur;
	//FragColor = vec4( 0.5, 0.5, 0.5, 1.0 ); // gris moche!

	
	vec4 coul = calculerReflexion( L[0], N, O );
	coul += calculerReflexion( L[1], N, O );


	// couleur finale du fragment
	FragColor = clamp( coul, 0.0, 1.0 );

	if ( afficheNormales ) FragColor = vec4(N,1.0);
}
