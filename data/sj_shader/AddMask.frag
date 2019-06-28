#version 120
#extension GL_ARB_texture_rectangle : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2DRect MaskAdd;
uniform sampler2DRect MaskAll;

void main(){
	/********************
	********************/
	vec2 tex_pos = gl_TexCoord[0].xy;
	
	vec4 color_All = texture2DRect(MaskAll, tex_pos);
	vec4 color_Add = texture2DRect(MaskAdd, tex_pos);
	
	color_Add.rgb = vec3(1.0, 1.0, 1.0) - color_Add.rgb;
	
	vec4 color = min(color_All, color_Add);
	color.a = 1.0;
	
	/********************
	********************/
	gl_FragColor = color;
}
