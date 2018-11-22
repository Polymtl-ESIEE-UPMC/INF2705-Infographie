#version 410

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in Attribs {
	vec4 couleur;
	float tempsRestant;
	float sens; // du vol
} AttribsIn[];

out Attribs {
	vec4 couleur;
	vec2 texCoord;
} AttribsOut;

uniform mat4 matrProj;
uniform int texnumero;

void main()
{	
	// assigner la taille des points (en pixels)
	gl_PointSize = gl_in[0].gl_PointSize;

	const int N_SPRITE = 16;
	vec2 coins[4];
	for ( int i = 0; i < coins.length; ++i)
	{
		coins[i] = vec2(floor (i / 2) - 0.5, i % 2 -0.5 );
	}
	for ( int i = 0 ; i < coins.length ; ++i )
	{
		// assigner la position du point
		float fact = gl_in[0].gl_PointSize / 50;
		
		// rotation des sommets n'est pas appliquée
		if (texnumero == 1)
		{
			float theta = 6*AttribsIn[0].tempsRestant;//int(mod(6*AttribsIn[0].tempsRestant, 2*M_PI));

			mat3 rot = mat3(cos(theta), -sin(theta), 0,
							sin(theta), cos(theta),  0,
							0,			0,			 1);

			vec2 decalage = (rot * vec3(coins[i].xy, 1)).xy;
		}
		// on positionne successivement aux quatre coins
		vec2 decalage = coins[i]; 
		
		vec4 pos = vec4( gl_in[0].gl_Position.xy + fact * decalage, gl_in[0].gl_Position.zw );
		gl_Position = matrProj * pos;	// on termine la transformation débutée dans le nuanceur de sommets
		
		// assigner la couleur courante
		AttribsOut.couleur = AttribsIn[0].couleur;

		AttribsOut.texCoord = coins[i] + vec2(0.5, 0.5); // on utilise coins[] pour définir des coordonnées de texture
		if (texnumero > 1)
		{
			if (texnumero == 2)
			{
				coins[i].x = AttribsIn[0].sens * coins[i].x;
			}
			int frame = int(mod(18*AttribsIn[0].tempsRestant, N_SPRITE));
			AttribsOut.texCoord.x = (coins[i].x + 0.5 +  frame)/ N_SPRITE;
		}
		else if (texnumero == 1)
		{
			// rotation de la texture
			float theta = 6 * AttribsIn[0].tempsRestant;
			mat3 rot = mat3(cos(theta), -sin(theta), 0,
							sin(theta), cos(theta),  0,
							0,			0,			 1);

			mat3 trans = mat3(1, 0, -0.5,
							  0, 1, -0.5,
							  0, 0, 1);

			mat3 transInv = mat3(1, 0, 0.5,
								 0, 1, 0.5,
								 0, 0, 1);

			vec3 point = vec3(coins[i].xy + vec2(0.5, 0.5), 1);

			AttribsOut.texCoord = (point * trans * rot * transInv).xy;
		}
		EmitVertex();
	}

	EmitVertex();
}