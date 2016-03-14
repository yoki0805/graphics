// bump map
attribute vec3 Tangent;
attribute vec3 Bitangent;

varying vec3 eyetan, eyebitan, eyenorm, eyepos;


void main()
{
	gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;

	eyepos = vec3(gl_ModelViewMatrix * gl_Vertex);
	
	//TBN in eyespace
	eyetan = normalize( gl_NormalMatrix * Tangent );
	eyebitan = normalize( gl_NormalMatrix * Bitangent );
	eyenorm = normalize( gl_NormalMatrix * gl_Normal );
}
