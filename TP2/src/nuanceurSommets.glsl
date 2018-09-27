#version 410

const float M_PI = 3.14159265358979323846;	// pi
const float M_PI_2 = 1.57079632679489661923;	// pi/2

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

uniform vec4 planDragage; // équation du plan de dragage
uniform vec4 planRayonsX; // équation du plan de rayonX

layout(location=0) in vec4 Vertex;
layout(location=3) in vec4 Color;

out Attribs {
   vec4 couleur;
   float clipDistanceDragage;
   float clipDistanceRayonsX;
} AttribsOut;

void main( void )
{
   // transformation standard du sommet
   gl_Position = matrProj * matrVisu * matrModel * Vertex;

   // couleur du sommet
   AttribsOut.couleur = Color;

   // Mettre un test bidon afin que l'optimisation du compilateur n'élimine l'attribut "planDragage".
   // Mettre un test bidon afin que l'optimisation du compilateur n'élimine l'attribut "planRayonsX".
   // Vous ENLEVEREZ ce test inutile!
   //if ( planDragage.x + planRayonsX.x < -10000.0 ) AttribsOut.couleur.r += 0.001;

   // position dans le monde
   vec4 pos = matrModel * Vertex;

   AttribsOut.clipDistanceDragage = dot(planDragage, pos);
   AttribsOut.clipDistanceRayonsX = dot(planRayonsX, pos);

}
