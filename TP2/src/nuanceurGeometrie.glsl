#version 410

layout(triangles) in;
layout(triangle_strip, max_vertices = 8) out;


in Attribs {
   vec4 couleur;
   float clipDistanceDragage;
   float clipDistanceRayonsX;
} AttribsIn[];

out Attribs {
   vec4 couleur;
} AttribsOut;

void main()
{
   // émettre les sommets
   for ( int i = 0 ; i < gl_in.length() ; ++i )
   {
      gl_ViewportIndex = 0;
      gl_Position = gl_in[i].gl_Position;
      AttribsOut.couleur = AttribsIn[i].couleur;
      gl_ClipDistance[0] = AttribsIn[i].clipDistanceDragage;
      gl_ClipDistance[1] = AttribsIn[i].clipDistanceRayonsX;
      EmitVertex();
   }
   EndPrimitive();

    for ( int i = 0 ; i < gl_in.length() ; ++i )
    {
      gl_ViewportIndex = 1;
      gl_Position = gl_in[i].gl_Position;
      AttribsOut.couleur = AttribsIn[i].couleur;
      gl_ClipDistance[0] = -AttribsIn[i].clipDistanceDragage;
      gl_ClipDistance[1] = AttribsIn[i].clipDistanceRayonsX;
      EmitVertex();
   }
   EndPrimitive();
}