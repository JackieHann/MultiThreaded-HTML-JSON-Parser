// Performance Assignment.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <stdio.h>

#include <thread>
#include <iostream>
#include <future>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>

class TIMER
{
	LARGE_INTEGER t_;

	__int64 current_time_;

public:
	TIMER()	// Default constructor. Initialises this timer with the current value of the hi-res CPU timer.
	{
		QueryPerformanceCounter(&t_);
		current_time_ = t_.QuadPart;
	}

	TIMER(const TIMER &ct)	// Copy constructor.
	{
		current_time_ = ct.current_time_;
	}

	TIMER& operator=(const TIMER &ct)	// Copy assignment.
	{
		current_time_ = ct.current_time_;
		return *this;
	}

	TIMER& operator=(const __int64 &n)	// Overloaded copy assignment.
	{
		current_time_ = n;
		return *this;
	}

	~TIMER() {}		// Destructor.

	static __int64 get_frequency()
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return frequency.QuadPart;
	}

	__int64 get_time() const
	{
		return current_time_;
	}

	void get_current_time()
	{
		QueryPerformanceCounter(&t_);
		current_time_ = t_.QuadPart;
	}

	inline bool operator==(const TIMER &ct) const
	{
		return current_time_ == ct.current_time_;
	}

	inline bool operator!=(const TIMER &ct) const
	{
		return current_time_ != ct.current_time_;
	}

	__int64 operator-(const TIMER &ct) const		// Subtract a TIMER from this one - return the result.
	{
		return current_time_ - ct.current_time_;
	}

	inline bool operator>(const TIMER &ct) const
	{
		return current_time_ > ct.current_time_;
	}

	inline bool operator<(const TIMER &ct) const
	{
		return current_time_ < ct.current_time_;
	}

	inline bool operator<=(const TIMER &ct) const
	{
		return current_time_ <= ct.current_time_;
	}

	inline bool operator>=(const TIMER &ct) const
	{
		return current_time_ >= ct.current_time_;
	}
};

//Container to allow multiple timed blocks whenever you want (useful for finding bottlenecks)
class TimedBlock
{
public:
	TimedBlock():
		m_block_name("*none*")
	{
		Start();
	}

	TimedBlock(std::string block_name):
		m_block_name(block_name)
	{
		Start();
	}
	void Stop()
	{
		TIMER end;

		TIMER elapsed;

		elapsed = end - m_start;

		__int64 ticks_per_second = m_start.get_frequency();

		// Display the resulting time...

		double elapsed_seconds = (double)elapsed.get_time() / (double)ticks_per_second;

		std::cout.precision(17);
		std::cout << "Function: [" << m_block_name << "] finished, Elapsed time (sec): " << std::fixed << elapsed_seconds << std::endl;
	}
	private:

	void Start()
	{
		TIMER newTimer;
		m_start = newTimer;
	}

	


private:
	std::string m_block_name;
	TIMER m_start;
};


struct Trace
{
	std::string example;
};

std::mutex string_storage_mutex, string_queue_mutex, string_ready_mutex;
std::condition_variable string_ready_condition;
std::queue<std::string> string_queue;

bool finished_producing = false;


std::mutex trace_storage_mutex, trace_queue_mutex;
std::condition_variable trace_ready_condition;
std::queue<Trace> trace_queue;

#define DEBUG
#define XML_FILE_PATH "testdata_short.txt"

bool string_ready = false;
bool trace_ready = false;

bool ProduceStringsFromFile(const char* file_path)
{
	//String producer, stores each line within a given file into the string stor
	bool remaining_data = true;
	std::ifstream file(file_path);
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			{
				string_ready = false;
				std::lock_guard<std::mutex> lock(string_queue_mutex); // Lock the mutex for the queue for a short period of time while we write to it.
				string_queue.push(line);
				string_ready = true;
			}
#ifdef DEBUG
					std::cout << "[Producer]\tString Extracted:\t" << string_queue.size() << "\t|\tThread [" << std::this_thread::get_id() << "]\n";
#endif
			{
				string_ready_condition.notify_one();
			}

		}
		file.close();
	}

	return true;
}

Trace ConvertToTrace(const std::string str)
{
	//std::this_thread::sleep_for(std::chrono::milliseconds(rand()%1000));	// Wait for a second.
	Trace t;
	t.example = str;
	return t;
}

void ProcessStringsIntoTrace(std::queue<std::string> &string_storage_queue, std::queue<Trace> &trace_storage_queue)
{

	bool remaining_data = false;

	do 
	{

		Trace new_trace;
		bool got_trace = false;

		{
			std::unique_lock<std::mutex> data_ready_lock(string_ready_mutex);	// Lock the mutex for the 'data_ready' flag
			string_ready_condition.wait(data_ready_lock, [] {return string_ready; });	// Wait until the flag is set.
			std::lock_guard<std::mutex> lock(string_queue_mutex);	// Now process the queue...

			if (!string_storage_queue.empty())
			{
				//process trace here!
				Trace new_trace = ConvertToTrace(string_storage_queue.front());
				got_trace = true;
#ifdef DEBUG
				std::cout << "[Consumer]\tTrace Extracted:\t" << string_queue.size() << "\t|\tThread [" << std::this_thread::get_id() << "]\n";
#endif
				string_storage_queue.pop();
			}

			remaining_data = !string_storage_queue.empty();
		}
		if(got_trace)
		{
			trace_ready = false;
			std::lock_guard<std::mutex> lock(trace_queue_mutex); // Lock the mutex for the queue for a short period of time while we write to it.
			trace_storage_queue.push(new_trace);
			trace_ready = true;
		}

		trace_ready_condition.notify_one();
		//if (!(remaining_data || !finished_producing)) return;
	} while ((remaining_data || !finished_producing));
}




int main()
{
	// Application starts here...

	// Time the application's execution time.
	TIMER start;	// DO NOT CHANGE THIS LINE. Timing will start here.

					//--------------------------------------------------------------------------------------
					// Insert your code from here...

#ifdef DEBUG
	//TimedBlock timer_split_text("Split Text Load");
#endif
	
	//reserve enough space per line??? (reserve based on filesize?)

	//Produces strings from a given file
	int thread = 1;
	std::future<bool> ftr_string_producer(std::async(std::launch::async, ProduceStringsFromFile, XML_FILE_PATH));
	//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	//Multithreaded extract data from these strings into another queue
	std::future<void> ftr_trace_producer_1(std::async(std::launch::async, ProcessStringsIntoTrace, std::ref(string_queue), std::ref(trace_queue)));
	std::future<void> ftr_trace_producer_2(std::async(std::launch::async, ProcessStringsIntoTrace, std::ref(string_queue), std::ref(trace_queue)));
	//std::future<void> ftr_trace_producer_3(std::async(std::launch::async, ProcessStringsIntoTrace, std::ref(string_queue), std::ref(trace_queue)));
	//std::future<void> ftr_trace_producer_4(std::async(std::launch::async, ProcessStringsIntoTrace, std::ref(string_queue), std::ref(trace_queue)));
	//std::future<void> ftr_trace_producer_2(std::async(ProcessStringsIntoTrace, std::ref(string_queue), thread++));
	//std::future<void> ftr_trace_producer_3(std::async(ProcessStringsIntoTrace, std::ref(string_queue), thread++));

	//Process each line that was produced into a queue of 'Traces' multithreaded


	//Process each Trace to analyse data

	//Process each Trace again to get the json :)


	//Wait for all future threads to finish
	finished_producing = ftr_string_producer.get();

	ftr_trace_producer_1.get();
	ftr_trace_producer_2.get();
	//ftr_trace_producer_3.get();
	//ftr_trace_producer_4.get();



#ifdef DEBUG
	//timer_split_text.Stop();
#endif
					//-------------------------------------------------------------------------------------------------------
					// How long did it take?...   DO NOT CHANGE FROM HERE...

	TIMER end;

	TIMER elapsed;

	elapsed = end - start;

	__int64 ticks_per_second = start.get_frequency();

	// Display the resulting time...

	double elapsed_seconds = (double)elapsed.get_time() / (double)ticks_per_second;

	std::cout.precision(17);
	std::cout << "Elapsed time (seconds): " << std::fixed << elapsed_seconds;
	std::cout << std::endl;
	std::cout << "Press a key to continue" << std::endl;

	char c;
	std::cin >> c;

    return 0;
}

