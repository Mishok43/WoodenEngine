/*
	Существует несколько техник реализации самозатенения.
	Воксельная - минус главный - требуемый 3д ресурс, много жрет много памяти и долго обрабатывать (дискретизрованный рейтрейсинг)
	Карта фурье - тоже долго обрабатывается
	Используемый метод - серединный вектор. Требуется малое количество ресурсов по памяти (только на сортировку посути) и все вычисления также приходятся только на сортировку. 
	При небольшом количество частиц если все запускается в одном Thread Group благодаря этому можно достичь высокой производительости, из-за использования L1 кеша. 
	Нужны лишь 2д ресурсы небольшого объема

	Для повышения производительности нужно процессором передать вектор равный серединному вектору между вектором света и вектором камеры
	halfDir = (litDir+eyeDir)/2;
	Алгоритм разделен на несколько этапов:
	1. Находим проекцию координат каждой частицы на нормированный вектор halfDir
	2. Сортируем мы относительно данной координаты с помощью стандартного RADIX SIMD алгоритма (велосипедный - https://www.semanticscholar.org/paper/Fast-Data-Parallel-Radix-Sort-Implementation-in-11-Alvizo-Olivares/3683baede76b6180e19f1442e82ddcc5e07148bf)  
	3. Далее разделяем нашу массу частиц на слои (NUM_SLICES) и для каждого слоя находим среднюю прозрачность путем прохождением циклом
	по частицам данного слоя перемножая opacity*radius и деля на общую сумму радиусов. И на основе этого состовляем карту затененности
*/

#define THREAD_X 32 //- от 1 до 32
#define THREAD_Y 32 //- от 1 до 32
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

// Серединный вектор = (litDir+eyeDir)/2
float3 halfDir;

// Прозрачности слоев
groupshared uint smSliceOpacity[SLICE_SIZE];
// Объекты сортировки Проекции координат векторов частиц
groupshared Pair smObjects[GROUP_THREADS];
// Текущий обрабатываемый бит RADIX сортировки
groupshared uint smCurBit[GROUP_THREADS];
// Счетчик
groupshared uint smCounters[GROUP_THREADS];
groupshared uint smDestIndices[GROUP_THREADS];

[numthreads(THREAD_X, THREAD_Y, 1)]
void csComputeSelfShadowing( 
	int3 dispatchThreadID: SV_DispatchThreadID,
	int3 threadID: SV_GroupIndex)
{
	// Вычисляем проекцию координат частицы на вектор halfDir. 
	// Преобразовываем в uint так как только с ней RADIX сортировка работает корректно.
	// Одно ограничение при сортировки - отсутствие отрицательные координаты, но их можно костылем починить (как минимум - прибавление координат)
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
		Вычисляем здесь префикс ненормированную взвешенную сумму от iSlice*SLICE_SIZE до (iSlice+1)*SLICE_SIZE-1 по формуле: радиус*прозрачность
		Вычисляем здесь  сумму радиусов от iSlice*SLICE_SIZE до (iSlice+1)*SLICE_SIZE-1 по формуле: радиус*прозрачность
		и нормируем
		
	*/

	float PrefixSumWeightedOpacity = ...;
	float PrefixSumRadius = ...;
	float CurOpacity = PrefixSumWeightedOpacit / PrefixSumRadius;
	sbShadows[particleIndex] = CurOpacity;
}
