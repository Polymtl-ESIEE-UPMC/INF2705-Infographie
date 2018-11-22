#version 410

uniform sampler2D laTexture;
uniform int texnumero;

in Attribs {
	vec4 couleur;
	vec2 texCoord;
} AttribsIn;

out vec4 FragColor;

void main( void )
{
	// Mettre un test bidon afin que l'optimisation du compilateur n'élimine l'attribut "couleur".
	// Vous ENLEVEREZ cet énoncé inutile!
	if ( AttribsIn.couleur.r + texnumero + texture(laTexture,vec2(0.0)).r < 0.0 ) discard;

	vec4 couleur = AttribsIn.couleur;
	if ( texnumero != 0 )
	{
		couleur = texture( laTexture, AttribsIn.texCoord );
	}
	if (couleur.a < 0.1)
	{
		discard;
	}
	couleur.rgb = mix(AttribsIn.couleur.rgb, couleur.rgb, 0.7 );
	FragColor = couleur;
}
