#version 450
#extension GL_ARB_separate_shader_objects: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_multiview : enable

layout(set = 0, binding = 2) uniform sampler2D texSamplers[];

layout(push_constant) uniform PushConstant {
	uint modelIndex;
};

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(texSamplers[modelIndex], fragTexCoord);
}
