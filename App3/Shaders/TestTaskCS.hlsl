/*
	���������� ��������� ������ ���������� �������������.
	���������� - ����� ������� - ��������� 3� ������, ����� ���� ����� ������ � ����� ������������ (����������������� �����������)
	����� ����� - ���� ����� ��������������
	������������ ����� - ���������� ������. ��������� ����� ���������� �������� �� ������ (������ �� ���������� ������) � ��� ���������� ����� ���������� ������ �� ����������. 
	��� ��������� ���������� ������ ���� ��� ����������� � ����� Thread Group ��������� ����� ����� ������� ������� �����������������, ��-�� ������������� L1 ����. 
	����� ���� 2� ������� ���������� ������

	��� ��������� ������������������ ����� ����������� �������� ������ ������ ����������� ������� ����� �������� ����� � �������� ������
	halfDir = (litDir+eyeDir)/2;
	�������� �������� �� ��������� ������:
	1. ������� �������� ��������� ������ ������� �� ������������� ������ halfDir
	2. ��������� �� ������������ ������ ���������� � ������� ������������ RADIX SIMD ��������� (������������ - https://www.semanticscholar.org/paper/Fast-Data-Parallel-Radix-Sort-Implementation-in-11-Alvizo-Olivares/3683baede76b6180e19f1442e82ddcc5e07148bf)  
	3. ����� ��������� ���� ����� ������ �� ���� (NUM_SLICES) � ��� ������� ���� ������� ������� ������������ ����� ������������ ������
	�� �������� ������� ���� ���������� opacity*radius � ���� �� ����� ����� ��������. � �� ������ ����� ���������� ����� ������������
*/

#define THREAD_X 32 //- �� 1 �� 32
#define THREAD_Y 32 //- �� 1 �� 32
#define NUM_SLICES 32
#define PARTICLES 1024
#define SLICE_SIZE = PARTICLES/NUM_SLICES
#define GROUP_THREADS THREAD_X*THREAD_Y

struct Particle
{
	float3 pos;
	float radius;
	float opacity;
};

struct Pair
{
	uint value;
	uint index;
};

StructuredBuffer<Particle> sbParticles;

RWBuffer<float> sbShadows;

// ���������� ������ = (litDir+eyeDir)/2
float3 halfDir;

// ������������ �����
groupshared uint smSliceOpacity[SLICE_SIZE];
// ������� ���������� �������� ��������� �������� ������
groupshared Pair smObjects[GROUP_THREADS];
// ������� �������������� ��� RADIX ����������
groupshared uint smCurBit[GROUP_THREADS];
// �������
groupshared uint smCounters[GROUP_THREADS];
groupshared uint smDestIndices[GROUP_THREADS];

[numthreads(THREAD_X, THREAD_Y, 1)]
void csComputeSelfShadowing( 
	int3 dispatchThreadID: SV_DispatchThreadID,
	int3 threadID: SV_GroupIndex)
{
	// ��������� �������� ��������� ������� �� ������ halfDir. 
	// ��������������� � uint ��� ��� ������ � ��� RADIX ���������� �������� ���������.
	// ���� ����������� ��� ���������� - ���������� ������������� ����������, �� �� ����� �������� �������� (��� ������� - ����������� ���������)
	uint projectedPosition = uint(dot(sbParticles[dispatchThreadID.xy].pos, halfDir));
	smObjects[threadID].value = projectedPosition; 
	smObjects[threadID].index = threadID;

	// Radix sort
	[unroll(32)]
	for (int n = 0; n < 32; n++)
	{
		// Gets current bit 
		smCurBit[threadID] = ((smObjects[threadID].value >> n) & 1) != 1;

		GroupMemoryBarrierWithGroupSync();

		// Computes prefix sum 
		if (threadID != 0)
		{
			smCounters[threadID] = smCurBit[threadID - 1];
		}
		else
		{
			smCounters[threadID] = 0;
		}

		GroupMemoryBarrierWithGroupSync(); 

		
		[unroll(int(log2(GROUP_THREADS)))]
		for (uint i = 1; i < GROUP_THREADS; i <<= 1)
		{
			uint temp;
			if (threadID > i)
			{
				temp = smCounters[threadID] + smCounters[threadID - i];
			}
			else
			{
				temp = smCounters[threadID];
			}
			GroupMemoryBarrierWithGroupSync();
			smCounters[threadID] = temp;
			GroupMemoryBarrierWithGroupSync();

		}

		// Computes total fales
		if (ThreadID == 0)
		{
			totalFalses = smCurBit[GROUP_THREADS - 1] + smCounters[GROUP_THREADS - 1];
		}

		GroupMemoryBarrierWithGroupSync();

		smDestIndices[ThreadID] = smCurBit[ThreadID] ? smCounters[ThreadID] : ThreadID - smCounters[ThreadID] + totalFalses;

		Pair temp = smObjects[ThreadID];

		GroupMemoryBarrierWithGroupSync(); 

		// Updates object
		smObjects[smDestIndices[ThreadID]] = temp;

		GroupMemoryBarrierWithGroupSync();
	}

	uint particleIndex = smObjects[ThreadID].index;

	uint iSlice = particleIndex / SLICE_SIZE;
	
	/*
		��������� ����� ������� ��������������� ���������� ����� �� iSlice*SLICE_SIZE �� (iSlice+1)*SLICE_SIZE-1 �� �������: ������*������������
		��������� �����  ����� �������� �� iSlice*SLICE_SIZE �� (iSlice+1)*SLICE_SIZE-1 �� �������: ������*������������
		� ���������
		
	*/

	float PrefixSumWeightedOpacity = ...;
	float PrefixSumRadius = ...;
	float CurOpacity = PrefixSumWeightedOpacit / PrefixSumRadius;
	sbShadows[particleIndex] = CurOpacity;
}
