#version 410

uniform int attEloignement;
const float debAttenuation = 30.0;
const float finAttenuation = 50.0;

in Attribs {
   vec4 couleur;
} AttribsIn;

out vec4 FragColor;

void main( void )
{
   // la couleur du fragment est la couleur interpolée
   FragColor = AttribsIn.couleur;

   // atténuer selon la profondeur
   if ( attEloignement == 1 )
   {
      // Mettre un énoncé bidon afin que l'optimisation du compilateur n'élimine l'attribut "attEloignement".
      // Vous ENLEVEREZ ce test inutile!
      if ( FragColor.x < -10000.0 ) discard;
      
      // Obtenir la distance à la caméra du sommet dans le repère de la caméra
      //float dist = ...;
      // Obtenir un facteur d'interpolation entre 0 et 1
      //float factDist = smoothstep( ... );
      // Modifier la couleur du fragment en utilisant ce facteur
      // ...

      // pour déboguer et « voir » le facteur ou la distance, on peut utiliser:
      //FragColor = vec4( vec3(factDist), 1.0 );
      //FragColor = vec4( vec3(dist-floor(dist)), 1.0 );
   }

   // pour déboguer et « voir » le comportement de z ou w, on peut utiliser:
   //FragColor = vec4( vec3(gl_FragCoord.z), 1.0 );
   //FragColor = vec4( vec3(gl_FragCoord.w), 1.0 );
}
