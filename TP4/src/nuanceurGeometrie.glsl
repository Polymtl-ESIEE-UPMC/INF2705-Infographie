#version 410

layout(points) in;
layout(points, max_vertices = 1) out;

in Attribs {
    vec4 couleur;
    float tempsRestant;
    //float sens; // du vol
} AttribsIn[];

out Attribs {
    vec4 couleur;
} AttribsOut;

void main()
{
    // assigner la position du point
    gl_Position = gl_in[0].gl_Position;

    // assigner la taille des points (en pixels)
    gl_PointSize = gl_in[0].gl_PointSize;

    // assigner la couleur courante
    AttribsOut.couleur = AttribsIn[0].couleur;

    EmitVertex();
}
