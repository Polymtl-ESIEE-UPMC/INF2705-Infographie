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

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;
uniform mat3 matrNormale;

/////////////////////////////////////////////////////////////////

layout(location=0) in vec4 Vertex;
layout(location=2) in vec3 Normal;
layout(location=3) in vec4 Color;
layout(location=8) in vec4 TexCoord;

out Attribs {
	vec4 couleur;
	vec3 lumiDir[2];
	vec3 normale;
	vec3 obsVec;
	vec3 spotDir[2];
} AttribsOut;

float calculerSpot( in vec3 D, in vec3 L )
{
	float spotFacteur = 1.0;

	float c = LightSource.spotExponent;
	float cosG = dot(L, D);
	float cosD = cos(radians(LightSource.spotAngleOuverture));
	float cosI = cosD;
	float cosO = pow(cosI, 1.01 + c / 2);
	
	return utiliseDirect
				? smoothstep(cosO, cosI, cosG)
				: cosG > cosD
					? pow(cosG, c)
					: 0;
}

vec4 calculerReflexion( in vec3 L, in vec3 N, in vec3 O )
{
	vec4 grisUniforme = vec4(0); //vec4(0.7,0.7,0.7,1.0);

	// calcul de la composante ambiante de la source de lumière
	grisUniforme += FrontMaterial.ambient * LightSource.ambient;

	// produit scalaire pour le calcul de la réflexion diffuse
	float NdotL = max(0.0, dot( N, L ) );

	// calcul de la composante diffuse de la source de lumière
	grisUniforme += FrontMaterial.diffuse * LightSource.diffuse * NdotL;

	// calcul de la composante spéculaire (selon Phong ou Blinn)
	float NdotHV = utiliseBlinn
						? max ( 0.0, dot( normalize( L + O ), N ) )  // Blinn
						: max ( 0.0, dot( reflect( -L, N ),   O ) ); // Phong

	// calcul de la composante spéculaire de la source de lumière
	grisUniforme += FrontMaterial.specular * LightSource.specular * pow( NdotHV, FrontMaterial.shininess );
	return( grisUniforme );
}

void main( void )
{
	// transformation standard du sommet
	gl_Position = matrProj * matrVisu * matrModel * Vertex;

	// calculer la normale
	vec3 N = matrNormale * Normal;
	AttribsOut.normale = N;

	// calculer la position du sommet dans le repère de la caméra
	vec3 pos = ( matrVisu * matrModel * Vertex).xyz;
	
	// calculer la direction vers l'observateur
	vec3 O = LightModel.localViewer
							? normalize(-pos)
							: vec3(0.0, 0.0, 1.0);
	AttribsOut.obsVec = O;

	
	// ajout de l’émission et du terme ambiant du modèle d’illumination si Gouraud
	vec4 coul = typeIllumination == 0 
					? FrontMaterial.emission + FrontMaterial.ambient * LightModel.ambient
					: vec4(0);

	// calculer la direction de la lumière
	for (int i = 0; i < LightSource.position.length; ++i)
	{
		vec3 L = LightSource.position[i].w == 0
								? (matrVisu * LightSource.position[i]).xyz
								: (matrVisu * LightSource.position[i] / LightSource.position[i].w).xyz - pos;
		
		vec3 D = transpose(inverse(mat3(matrVisu))) * -LightSource.spotDirection[i];

		coul += typeIllumination == 0
					? calculerSpot(normalize(D), normalize(L)) * calculerReflexion( normalize(L), normalize(N), normalize(O) ) // Gouraud
					: coul; // Phong (coul = 0)
		
		AttribsOut.lumiDir[i] = L;
		AttribsOut.spotDir[i] = D;
	}


	AttribsOut.couleur = clamp(coul, 0.0, 1.0);
}
