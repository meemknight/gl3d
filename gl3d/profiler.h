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

	const int AverageProfilerMaxTests = 300;

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

	struct ImguiProfiler
	{

		ImguiProfiler() {};
		ImguiProfiler(std::string n, float min, float max): plotName(n), 
			min(min), max(max){};

		PL::AverageProfiler profiler;

		void start() { profiler.start(); }
		void end() { profiler.end(); }

		static const int ARR_SIZE = 60;
		int valuePos = 0;
		float profileAverageValue[ARR_SIZE] = { };

		std::string plotName;
		float min = 0;
		float max = 60;

		void updateValue(float value = 1)
		{
			value *= profiler.getAverageAndResetData().timeSeconds;

			if (valuePos < ARR_SIZE)
			{

				profileAverageValue[valuePos] = value;
				valuePos++;
			}
			else
			{
				for (int i = 0; i < ARR_SIZE - 1; i++)
				{
					profileAverageValue[i] = profileAverageValue[i + 1];
				}
				profileAverageValue[ARR_SIZE - 1] = value;
			}
		}

		void imguiPlotValues()
		{
			ImGui::NewLine();
			ImGui::PlotHistogram(("##" + plotName + " graph").c_str(), profileAverageValue, ARR_SIZE, 0, 0,
			min, max);
			ImGui::Text((plotName + ", %f").c_str(), profileAverageValue[valuePos-1]);
		};

	};


};