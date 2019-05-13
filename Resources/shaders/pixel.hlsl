struct PixelShaderInput {
    float3 Normal : NORMAL;
    float3 WorldPosition : POSITION;
    float2 TextureCoord : TEXCOORD2;
    float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_Target {
    // const
    const float3 lightColor = float3(0.3, 0.3, 0.3);
    const float3 lightPosition = float3(0, 50, 450);
    const float ambientStrength = 0.3;
    const float specularStrength = 30.0;

    // ambient
    float3 ambient = ambientStrength * lightColor;

    // diffuse
    float3 normal = normalize(IN.Normal);
    float3 lightDir = normalize(lightPosition - IN.WorldPosition);
    float3 diffuseFactor = max(dot(normal, lightDir), 0.0);
    float3 diffuse = diffuseFactor * lightColor;

    // specular
    float3 viewDir = normalize(float3(0, 0, 0) - IN.WorldPosition);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float3 specular = specularStrength * spec * lightColor;

    float3 result = (ambient + diffuse + specular) * float3(1, 1, 1);
    return float4(result, 1.0);
}
