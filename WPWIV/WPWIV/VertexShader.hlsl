#include "GlobalInclude.hlsli"

//VS_OUTPUT main(VS_INPUT input)
//{
//    VS_OUTPUT output;
//    output.pos = mul(mul(viewProj, model), float4(input.pos, 1));
//    output.texCoord = input.texCoord;
//    return output;
//}

//VS_OUTPUT main(VS_INPUT input)
//{
//    VS_OUTPUT output;
//    output.pos = mul(mul(viewProj, model), float4(input.pos, 1));
//    output.texCoord = input.texCoord;
//    return output;
//}

//VS_OUTPUT main(uint vid : SV_VertexID)
//{
//	VS_OUTPUT output;
//
//}


// outputs a full screen triangle with screen-space coordinates
// input: three empty vertices
VSQuadOut main(uint vertexID : SV_VertexID) {
	VSQuadOut result;
	result.uv = float2((vertexID << 1) & 2, vertexID & 2);
	result.position = float4(result.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return result;
}