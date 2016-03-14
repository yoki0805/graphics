// mosaic

uniform vec2 center;
uniform float radius;
uniform sampler2D tex;

uniform float gran; // granularity!

void main()
{
	vec4 color;
//	float radius = 80.0;
	float dis = distance(gl_FragCoord.st,center);
	if (dis < radius) {
		vec2 texcoord = gl_TexCoord[0].st;
		texcoord = floor(texcoord * gran)/gran;

		color = texture2D (tex, texcoord);
	} else if (dis < radius+2.0) {
		color = vec4(1.,0.,0.,1.);
	} else {
		color = texture2D (tex, gl_TexCoord[0].st);
	}

	gl_FragColor = color;
}
