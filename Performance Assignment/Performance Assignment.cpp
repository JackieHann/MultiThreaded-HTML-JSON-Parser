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

std::queue<std::queue<std::string>> string_chunk_store;

#define DEBUG
#define XML_FILE_PATH "testdata_short.txt"

bool string_ready = false;
bool trace_ready = false;


std::mutex string_chunk_storage_mutex;	// Lock the mutex to protect the data store, as it will be accessed from other threads for short periods...
std::condition_variable string_chunk_ready_condition; // Signal that we have prepared the data (it is ready for processing on the other thread).

std::mutex trace_chunk_storage_mutex;	// Lock the mutex to protect the data store, as it will be accessed from other threads for short periods...
std::condition_variable trace_chunk_ready_condition; // Signal that we have prepared the data (it is ready for processing on the other thread).
std::queue<std::queue<Trace>> trace_chunk_store;

#define CHUNK_SIZE 4
bool ProduceStringsFromFile(const char* file_path)
{
	//String producer, ends when no more lines can be read from the file.
	std::ifstream file(file_path);
	if (!file.is_open()) C_ASSERT("ERROR - INCORRECT FILE PATH");
		
	bool remaining_data = true;
	while (remaining_data)
	{
		{
			//Prepare chunk of lines read from file
			std::string line;
			std::queue<std::string> chunk;

			//Could unroll this, kept since want it to be generic (to test chunk size speed)
			for (int i = 0; i < CHUNK_SIZE; ++i)
				if (std::getline(file, line)) chunk.push(line);

			if (!chunk.empty())
			{
#ifdef DEBUG
				std::cout << "[String Chunk Producer]\t " << chunk.size() << " lines pulled from file.\t" << "\t|\tThread [" << std::this_thread::get_id() << "]\n";
#endif
				std::lock_guard<std::mutex> queue_lock(string_queue_mutex);
				string_chunk_store.push(chunk);
			}
			else
			{
				//End of file, so flag exit condition
				remaining_data = false;
			}
		}

		//Did we push any data up?
		if (remaining_data)
		{
			//Notify open waiting thread
			std::lock_guard<std::mutex> queue_lock(string_chunk_storage_mutex);
			string_chunk_ready_condition.notify_one();
		}
	}

	//Finished, set flags - useful for other consumers
	return true;
}

Trace ConvertToTrace(std::string str)
{
	//std::this_thread::sleep_for(std::chrono::milliseconds(rand()%1000));	// Wait for a second.
	Trace t;
	t.example = str;
	return t;
}

void ProcessData(std::queue<std::queue<std::string>> &data_storage_queue)
{
	bool more_data = true;
	while (more_data)	// Do this forever (obviously, in reality, there should be some mechanism here to end this loop...)
	{
		std::queue<std::string> q;
		int queue_size=0;
		{
			std::unique_lock<std::mutex> data_ready_lock(string_chunk_storage_mutex);	// Mutex for the condition variable.
			//std::lock_guard<std::mutex> queue_lock(string_queue_mutex);	// Mutex for the queue data (for the 'empty' call).
			string_chunk_ready_condition.wait(data_ready_lock, [=] {return true;});	// Wait then check the queue for data - does data exist?
			{
				std::lock_guard<std::mutex> queue_lock(string_queue_mutex);
				if (!data_storage_queue.empty())
				{
					q = data_storage_queue.front();	// Take the data off the queue and make a copy, then unlock the mutex, so we can process the data locally and the other threads can continue.
					//q.pop();
					data_storage_queue.pop();
					queue_size = q.size();
				}
			}
		}	// Release the locks at this point.
		for (int n = 0; n < queue_size; ++n)
		{
#ifdef DEBUG
			std::cout << "[Data Chunk Consumer]\tThread [" << std::this_thread::get_id() << "]\n";
#endif
		}
		
	}
}


void ProcessStringsIntoTrace(std::queue<std::string> &string_storage_queue, std::queue<Trace> &trace_storage_queue)
{

	bool remaining_data = true;
	while ((remaining_data || !finished_producing))
	{

		//Trace new_trace;
		//bool got_trace = false;
		{
			std::unique_lock<std::mutex> data_ready_lock(string_storage_mutex);	// Lock the mutex for the 'data_ready' flag
			std::lock_guard<std::mutex> lock(string_queue_mutex);	// Now process the queue...
			string_ready_condition.wait(data_ready_lock, [=] { return !string_storage_queue.empty(); });	// Wait until the flag is set BAD BAD BAD! - Will continue waiting even 

#ifdef DEBUG
			std::cout << "[Consumer]\tTrace Attempted:\t" << "\t|\tThread [" << std::this_thread::get_id() << "]\n";
#endif
			//std::string str = string_storage_queue.front();
			//new_trace = ConvertToTrace(str);
			//got_trace = true;
			//string_storage_queue.pop();
		}

		


			//If strings have finished producing the vector, then we need to be able to exit this loop

			/*if (!string_storage_queue.empty())
			{
				//process trace here!
				std::string str = string_storage_queue.front();
				Trace new_trace = ConvertToTrace(str);
				got_trace = true;
#ifdef DEBUG
				std::cout << "[Consumer]\tTrace Extracted:\t" << string_queue.size() << "\t|\tThread [" << std::this_thread::get_id() << "]\n";
#endif
				string_storage_queue.pop();
			}

			remaining_data = !string_storage_queue.empty();*/
		//}
		/*if(got_trace)
		{
			trace_ready = false;
			std::lock_guard<std::mutex> lock(trace_queue_mutex); // Lock the mutex for the queue for a short period of time while we write to it.
			trace_storage_queue.push(new_trace);
			trace_ready = true;
			trace_ready_condition.notify_one();
		}*/
	}
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
	std::future<bool> ftr_string_producer(std::async(std::launch::async, ProduceStringsFromFile, XML_FILE_PATH));
	std::this_thread::sleep_for(std::chrono::milliseconds(10));	// Simulate a lengthy process - wait for a half a second.
	//Multithreaded extract data from these strings into another queue
	std::future<void> future2(std::async(std::launch::async, ProcessData, std::ref(string_chunk_store)));
	std::future<void> future3(std::async(std::launch::async, ProcessData, std::ref(string_chunk_store)));
	std::future<void> future4(std::async(std::launch::async, ProcessData, std::ref(string_chunk_store)));
	//std::future<void> ftr_trace_producer_1(std::async(ProcessStringsIntoTrace, std::ref(string_queue), std::ref(trace_queue)));
	//std::future<void> ftr_trace_producer_2(std::async(ProcessStringsIntoTrace, std::ref(string_queue), std::ref(trace_queue)));
	//s//td::future<void> ftr_trace_producer_3(std::async(ProcessStringsIntoTrace, std::ref(string_queue), std::ref(trace_queue)));

	//Process each line that was produced into a queue of 'Traces' multithreaded


	//Process each Trace to analyse data

	//Process each Trace again to get the json :)


	//Wait for all future threads to finish
	finished_producing = ftr_string_producer.get();

	future2.get();
	future3.get();
	future4.get();
	//ftr_trace_producer_1.get();
	//ftr_trace_producer_2.get();
	//ftr_trace_producer_3.get();



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

