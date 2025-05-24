

struct MatrixPair
{
    float4x4 Matrix1;
    float4x4 Matrix2;
};

StructuredBuffer<MatrixPair> Input : register(t0);

// �o�́F�|���Z����1�̍s��
RWStructuredBuffer<float4x4> Output : register(u0);

[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{ 
    uint i = DTid.x;
    Output[i] = mul(Input[i].Matrix1, Input[i].Matrix2);
  
}