varying vec4 LSCoord;
varying vec4 diffuse;

void main ()
{
    vec4 ecposition = gl_ModelViewMatrix * gl_Vertex;

    // shadowmap coord
    LSCoord = gl_TextureMatrix[3] * ecposition;

    vec3 N = normalize (gl_NormalMatrix * gl_Normal);
    vec3 L = normalize (gl_LightSource[0].position.xyz - vec3(ecposition));
    
    float NdotL = dot(N,L);
    diffuse = NdotL * gl_FrontLightProduct[0].diffuse;
        
    gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
