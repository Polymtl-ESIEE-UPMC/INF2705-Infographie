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

in Attribs {
	vec4 vertex;
	vec4 couleur;
	vec3 normale;
	vec2 texCoord;
} AttribsIn[];

out Attribs {
	vec4 couleur;
	vec3 lumiDir[2];
	vec3 normale;
	vec3 obsVec;
	vec3 spotDir[2];
	vec2 texCoord;
} AttribsOut;

layout(quads) in;

float interpole( float v0, float v1, float v2, float v3 )
{
	// mix( x, y, f ) = x * (1-f) + y * f.
	float v01 = mix( v0, v1, gl_TessCoord.x );
	float v32 = mix( v3, v2, gl_TessCoord.x );
	return mix( v01, v32, gl_TessCoord.y );
}

vec2 interpole( vec2 v0, vec2 v1, vec2 v2, vec2 v3 )
{
	// mix( x, y, f ) = x * (1-f) + y * f.
	vec2 v01 = mix( v0, v1, gl_TessCoord.x );
	vec2 v32 = mix( v3, v2, gl_TessCoord.x );
	return mix( v01, v32, gl_TessCoord.y );
}

vec3 interpole( vec3 v0, vec3 v1, vec3 v2, vec3 v3 )
{
	// mix( x, y, f ) = x * (1-f) + y * f.
	vec3 v01 = mix( v0, v1, gl_TessCoord.x );
	vec3 v32 = mix( v3, v2, gl_TessCoord.x );
	return mix( v01, v32, gl_TessCoord.y );
}

vec4 interpole( vec4 v0, vec4 v1, vec4 v2, vec4 v3 )
{
	// mix( x, y, f ) = x * (1-f) + y * f.
	vec4 v01 = mix( v0, v1, gl_TessCoord.x );
	vec4 v32 = mix( v3, v2, gl_TessCoord.x );
	return mix( v01, v32, gl_TessCoord.y );
}

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
	// calcul de la composante ambiante de la source de lumière
	vec4 coul = FrontMaterial.ambient * LightSource.ambient;

	// produit scalaire pour le calcul de la réflexion diffuse
	float NdotL = max(0.0, dot( N, L ) );

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
	// transformation standard du sommet
	gl_Position = interpole( gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[3].gl_Position, gl_in[2].gl_Position );
	// gl_Position.z = 1.0 - length( gl_Position.xy );
	// gl_Position.xyz = normalize( gl_Position.xyz );
	
	// calculer la normale
	vec3 N = interpole( AttribsIn[0].normale, AttribsIn[1].normale, AttribsIn[3].normale, AttribsIn[2].normale );
	AttribsOut.normale = N;

	// calculer la position du sommet dans le repère de la caméra
	vec3 pos = ( matrVisu * matrModel * interpole( AttribsIn[0].vertex, AttribsIn[1].vertex, AttribsIn[3].vertex, AttribsIn[2].vertex )).xyz;
	
	// calculer la direction vers l'observateur
	vec3 O = LightModel.localViewer
							? normalize(-pos)
							: vec3(0.0, 0.0, 1.0);
	AttribsOut.obsVec = O; 
	//AttribsOut.couleur = interpole( AttribsIn[0].couleur, AttribsIn[1].couleur, AttribsIn[3].couleur, AttribsIn[2].couleur );

	
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

	AttribsOut.texCoord = interpole( AttribsIn[0].texCoord, AttribsIn[1].texCoord, AttribsIn[3].texCoord, AttribsIn[2].texCoord );
	AttribsOut.couleur = clamp(coul, 0.0, 1.0);
}