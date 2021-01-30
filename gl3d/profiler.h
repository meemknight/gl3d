#pragma once
#include <intrin.h>
#include <Windows.h>

///////////////////////////////////////////
//https://github.com/meemknight/profilerLib
///////////////////////////////////////////

namespace PL
{
	struct PerfFreqvency
	{
		PerfFreqvency()
		{
			QueryPerformanceFrequency(&perfFreq);
		}

		LARGE_INTEGER perfFreq;
	};

	const static PerfFreqvency freq;



	struct ProfileRezults
	{
		float timeSeconds;
		unsigned int cpuClocks;
	};

	struct Profiler
	{

		LARGE_INTEGER startTime = {};
		__int64 cycleCount = {};

		void start()
		{
			QueryPerformanceCounter(&startTime);
			cycleCount = __rdtsc();
		}

		ProfileRezults end()
		{
			__int64 endCycleCount = __rdtsc();
			LARGE_INTEGER endTime;
			QueryPerformanceCounter(&endTime);

			cycleCount = endCycleCount - cycleCount;
			startTime.QuadPart = endTime.QuadPart - startTime.QuadPart;


			ProfileRezults r = {};

			r.timeSeconds = (float)startTime.QuadPart / (float)freq.perfFreq.QuadPart;
			r.cpuClocks = cycleCount;

			return r;
		}

	};

	const int AverageProfilerMaxTests = 200;

	struct AverageProfiler
	{
		ProfileRezults rezults[AverageProfilerMaxTests];
		int index = 0;

		Profiler profiler;

		void start()
		{
			profiler.start();
		}

		ProfileRezults end()
		{
			auto r = profiler.end();

			if (index < AverageProfilerMaxTests)
			{
				rezults[index] = r;
				index++;
			}

			return r;
		}

		ProfileRezults getAverageNoResetData()
		{
			if (index == 0)
			{
				return { 0,0 };
			}

			long double time = 0;
			unsigned long cpuTime = 0;

			for (int i = 0; i < index; i++)
			{
				time += rezults[i].timeSeconds;
				cpuTime += rezults[i].cpuClocks;
			}


			return { (float)(time / index), cpuTime / index };
		}

		void resetData()
		{
			index = 0;
		}

		ProfileRezults getAverageAndResetData()
		{
			auto r = getAverageNoResetData();
			resetData();
			return r;
		}


	};
};