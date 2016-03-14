// bumpmap
uniform sampler2D NormalMap;
uniform sampler2D ColorMap;
uniform sampler2D AlphaMap;
uniform bool UseBump;
uniform bool DigHole;

varying vec3 eyetan, eyebitan, eyenorm, eyepos;


void main()
{
	vec3 n = normalize(eyenorm);
	vec3 t = normalize(eyetan - dot(n,eyetan)*n);
	vec3 b = normalize(eyebitan - dot(n,eyebitan)*n
											- dot(t,eyebitan)*t);
											
	vec3 l = normalize(gl_LightSource[0].position.xyz - eyepos);
	
	vec3 lightTS = vec3( dot(l,t), dot(l,b), dot(l,n));
	
	vec3 normalTS = 2.0*texture2D(NormalMap, gl_TexCoord[0].st).rgb - vec3(1.0);
	
	float ndotl = dot(normalTS,lightTS);

	if (!UseBump) {
		ndotl = dot(eyenorm,l);
	}
	
	vec3 mycolor ;
	
	if( DigHole && texture2D(AlphaMap,gl_TexCoord[0].st).a < 1.0 ) {
		discard;
	} else {
		ndotl = (ndotl+0.2)*1.25;
		mycolor += ndotl * texture2D(ColorMap,gl_TexCoord[0].st).rgb;
	}
	
	gl_FragColor = vec4(mycolor,1.0);
}