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
	vec3 spotDir[2];
	vec2 texCoord;
} AttribsIn;

out vec4 FragColor;

float calculerSpot( in vec3 D, in vec3 L )
{
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
	// calcul de la composante ambiante de la source de lumière
	vec4 coul = FrontMaterial.ambient * LightSource.ambient;

	// produit scalaire pour le calcul de la réflexion diffuse
	float NdotL = max( 0.0, dot( N, L ) );

	// calcul de la composante diffuse de la source de lumière
	coul += (utiliseCouleur ? FrontMaterial.diffuse : vec4(0.7,0.7,0.7,1.0)) * LightSource.diffuse * NdotL;

	// calcul de la composante spéculaire (selon Phong ou Blinn)
	float NdotHV = utiliseBlinn
						? max ( 0.0, dot( normalize( L + O ), N ) )  // Blinn
						: max ( 0.0, dot( reflect( -L, N ),   O ) ); // Phong 

	// calcul de la composante spéculaire de la source de lumière
	coul += FrontMaterial.specular * LightSource.specular * pow( NdotHV, FrontMaterial.shininess );

	return( coul );
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


	// couleur finale du fragment
	if (typeIllumination == 1)
	{
		// ajout de l’émission et du terme ambiant du modèle d’illumination
		vec4 coul = FrontMaterial.emission + FrontMaterial.ambient * LightModel.ambient;
		
		for (int i = 0; i < AttribsIn.lumiDir.length; ++i)
		{
			vec3 D = normalize(AttribsIn.spotDir[i]);
				coul += calculerSpot(D, L[i]) * calculerReflexion( L[i], N, O );
		}
		FragColor = clamp( coul, 0.0, 1.0 );
	}
	vec4 coul = clamp(texture(laTexture, AttribsIn.texCoord), 0.0, 1.0);
	if(coul.r < 0.5 && coul.g < 0.5 && coul.b < 0.5) {
	
		switch (afficheTexelFonce) {
			case 0:
				FragColor *= coul;
				break;
			case 1:
				FragColor *= (FragColor + coul) * 0.5;
				break;
			case 2:
				discard;
			default:
				break;
		}
	}	
	FragColor = clamp(FragColor, 0.0, 1.0);

	if ( afficheNormales ) FragColor = vec4(N,1.0);
}
