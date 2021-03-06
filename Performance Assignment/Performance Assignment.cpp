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
#include <map>


#include <unordered_map>
#include <iterator>
#include <algorithm>
#include <ctime>

//Any files I have created:
#include "ParserOptions.h"
#include "Trace.h"



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

//Custom Timer to allow multiple timed blocks whenever you want (useful for finding bottlenecks)
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

template<typename A, typename B>
std::pair<B, A> flip_pair(const std::pair<A, B> &p)
{
	return std::pair<B, A>(p.second, p.first);
}

template<typename A, typename B>
std::multimap<B, A> flip_map(const std::map<A, B> &src)
{
	std::multimap<B, A> dst;
	std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()), &flip_pair<A, B>);
	return dst;
}

struct Analytic
{

	void RecordTrace(Trace& t)
	{
		int time = RecordSessionTime(t.GetSessionID(), t.GetVisitedTimes());
		RecordAverageTimeOnSite(time);
		RecordIPAccessed(t.GetIP());
		
		RecordPageVisits(t.GetVisitedSites());
	}

private:

	//Value to store total time on site.
	int total_time_on_site = 0;
	int total_entries = 0;

	//Map that contains each session and its duration.
	std::map<std::string, int> m_map_session_times;

	//Map that contains each IP that accessed the site, and how many times
	std::map<std::string, int> m_map_ip_times;

	//Map that contains each page visited, and how many times
	std::map<std::string, int> m_map_page_visits;

	time_t GetTimeFromFormattedString(std::string& str)
	{
		//Read in 
		int day, month, year, hour, min, sec;
		const char* ch = str.c_str();
		sscanf_s(ch, "%d/%d/%d %d:%d:%d", &day, &month, &year, &hour, &min, &sec);
		struct tm tm;
		tm.tm_isdst = -1; //No info on DST
		tm.tm_hour = hour;
		tm.tm_mday = day;
		tm.tm_mon = month - 1; //Month starts at 0
		tm.tm_year = year - 1900; //Year starts from 1900
		tm.tm_min = min;
		tm.tm_sec = sec;
		return mktime(&tm);
	}

	int RecordSessionTime(std::string session_id, std::vector<std::string>* times)
	{
		//Aka, get the first and last entry in times, find time difference between them, store in map for output (since each session ID will be unique)

		//We now have references to each 
		time_t first_time = GetTimeFromFormattedString(times->at(0));
		time_t last_time = GetTimeFromFormattedString(times->at(times->size() - 1));

		int diff_in_secs = (int)floor(difftime(last_time, first_time));
		m_map_session_times[session_id] = diff_in_secs;

		//return actual time of this session to be used to calculate average
		return diff_in_secs;
	}
	void OutputSessionTime(std::ostream& output)
	{
		output << "\"Sessions\": [";
		auto last_entry = m_map_session_times.end();
		last_entry--;
		//const char* FORMAT = "\n\t{\n\t\t\"Session_ID\":\"%s\",\n\t\t\"Time\": \"%d\"\n\t}";
		for (auto it = m_map_session_times.begin(); it != m_map_session_times.end(); it++)
		{
			output << "\n{\n\t\t\"SessionID\":\"" << it->first << "\",\n\t\t\"SessionTime\": \"" << it->second << "\"\n\t}";
			if (it != last_entry) output << ",";
		}

		output << "\n],";
	}

	void RecordAverageTimeOnSite(const int time)
	{
		total_time_on_site += time;
		total_entries++;
	}
	void OutputAverageTimeOnSite(std::ostream& output)
	{
		float avg = total_time_on_site / total_entries;
		output << "\n\t\"AverageTimeOnSite\" : \"" << std::setprecision(6) << avg << "\",\n";
	}

	void RecordIPAccessed(const char* ip)
	{
		m_map_ip_times[ip]++;
	}
	void OutputIPAccessed(std::ostream& output)
	{
		output << "\"SessionTimes\": [";
		auto last_entry = m_map_ip_times.end();
		last_entry--;
		for (auto it = m_map_ip_times.begin(); it != m_map_ip_times.end(); it++)
		{
			//This would be outputting to a file NOT VEC
			output << "\n\t{\n\t\t\"IP\":\"" << it->first << "\",\n\t\t\"Visited\": \"" << it->second << "\"\n\t}";
			if (it != last_entry) output << ",";
		}
		output << "\n],";
	}

	void RecordPageVisits(std::vector<std::string>* sites)
	{
		for (auto site : *sites)
		{
			static const char* ignore = "/default.html";
			if (site != ignore)
			{
				m_map_page_visits[site]++;
			}
		}
	}
	void OutputTopPageVisits(std::ostream& output, const int max)
	{
		std::multimap<int, std::string> flipped_map = flip_map(m_map_page_visits);
		int counter = 0;

		auto last_entry = flipped_map.rend();
		output << "\n\"20 Most Popular Pages\": [";
		for (auto it = flipped_map.rbegin(); it != flipped_map.rend() && counter < max; it++)
		{
			//This would be outputting to a file NOT VEC
			output << "\n\t\"" << it->second << "\"";
			++counter;
			if (counter != max) output << ",";
		}
		output << "\n]\n";
	}

public:
	void Output(std::ostream& output)
	{
		output << "{\n";
		OutputSessionTime(output);
		OutputAverageTimeOnSite(output);
		OutputIPAccessed(output);
		OutputTopPageVisits(output, 20);
		output << "\n}";
	}
};

bool finished_producing = false;
bool finished_outputting = false;

template <class StorageType>
class Worker
{
	std::mutex worker_storage_mutex;
	std::condition_variable worker_ready_condition;
	std::queue<StorageType> worker_queue;
};

Worker<Trace> trace_worker;
std::mutex trace_storage_mutex, trace_queue_mutex;
std::condition_variable trace_ready_condition;
std::queue<Trace> trace_queue;

Worker<std::queue<std::string>> string_chunk_worker;
std::queue<std::queue<std::string>> string_chunk_store;
std::mutex string_chunk_storage_mutex;	// Lock the mutex to protect the data store, as it will be accessed from other threads for short periods...
std::condition_variable string_chunk_ready_condition; // Signal that we have prepared the data (it is ready for processing on the other thread).

Worker<std::queue<Trace>> trace_chunk_worker;
std::mutex trace_chunk_storage_mutex;	// Lock the mutex to protect the data store, as it will be accessed from other threads for short periods...
std::condition_variable trace_chunk_ready_condition; // Signal that we have prepared the data (it is ready for processing on the other thread).
std::queue<std::queue<Trace>> trace_chunk_store;

std::ifstream input_file;
std::mutex input_file_mutex;

std::ofstream file2, file3;
std::mutex output_file_mutex;



Analytic analytics;


bool ProduceTraceFromFile(std::ifstream& file)
{
	//String producer, ends when no more lines can be read from the file.
	bool remaining_data = true;
	while (remaining_data)
	{
		{
			//Prepare chunk of lines read from file
			std::string line;
			std::queue<Trace> chunk;

			//Could unroll this, kept since want it to be generic (to test chunk size speed)
			{
				std::lock_guard<std::mutex> queue_lock2(input_file_mutex);
				for (int i = 0; i < CHUNK_SIZE; ++i)
					if (std::getline(file, line)) chunk.push(Trace(line));
			}

			if (!chunk.empty())
			{
#ifdef DEBUG
				//std::cout << "[String Chunk Producer]\t " << chunk.size() << " lines pulled from file.\t" << "\t|\tThread [" << std::this_thread::get_id() << "]\n";
#endif
				std::lock_guard<std::mutex> queue_lock(trace_queue_mutex);
				trace_chunk_store.push(chunk);
				//Process the trace here instead!
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
			std::lock_guard<std::mutex> queue_lock(trace_chunk_storage_mutex);
			trace_chunk_ready_condition.notify_one();
		}
	}

	//Finished, set flags - useful for other consumers
	return true;
}

bool ProcessTrace(std::queue<std::queue<Trace>> &trace_storage_queue)
{
	bool more_data = true;
	while (more_data)	// Do this forever (obviously, in reality, there should be some mechanism here to end this loop...)
	{
		std::queue<Trace> trace_chunk;
		bool now_empty=false;
		int queue_size=0;
		{
			std::unique_lock<std::mutex> data_ready_lock(trace_chunk_storage_mutex);	// Mutex for the condition variable.
			trace_chunk_ready_condition.wait(data_ready_lock, [] {return true;});	// Wait then check the queue for data - does data exist?
			{
				std::lock_guard<std::mutex> queue_lock(trace_queue_mutex);
				{
					if (!trace_storage_queue.empty())
					{
						trace_chunk = trace_storage_queue.front();	// Take the data off the queue and make a copy, then unlock the mutex, so we can process the data locally and the other threads can continue.
						trace_storage_queue.pop();
						now_empty = trace_storage_queue.size();
						queue_size = trace_chunk.size();
					}
					else
					{
						more_data = !finished_producing;
					}
				}
			}
		}

		//IE do something to our chunk
		for (int n = queue_size; n > 0; --n)
		{
#ifdef DEBUG
			//std::cout << "[Data Chunk Consumer]\tThread [" << std::this_thread::get_id() << "]\n";
#endif
			{
				std::lock_guard<std::mutex> queue_lock3(output_file_mutex);
				trace_chunk.front().ExportToJSON(file2, (finished_producing && now_empty && n == 1));
				analytics.RecordTrace(trace_chunk.front());
			}
			//trace_chunk.push(t);
			trace_chunk.pop();
			//Did we push any data up?
			
		}
	}
	return true;
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
	
	input_file.open(XML_INPUT_FILE_PATH, std::ios_base::in);
	if (!input_file.is_open()) C_ASSERT("ERROR - INCORRECT FILE PATH");

	file2.open(JSON_OUTPUT_FILE_PATH, std::ios_base::out);
	file2 << "{\n";

	

	//Produces strings from a given file
	const size_t NUM_PRODUCERS = 5;
	std::vector<std::future<bool>> ftr_trace_producers;
	for (auto i = 0; i < PRODUCER_COUNT; ++i)
	{
		ftr_trace_producers.push_back(std::move(std::async(std::launch::async, &ProduceTraceFromFile, std::ref(input_file))));
	}
	//https://stackoverflow.com/questions/36879979/c-how-do-i-get-stdfuture-out-of-a-vector
	//Link to useful post on why we can't initialize a future vector using one line and must move (No copy constructor) .

	std::vector<std::future<bool>> ftr_trace_consumers;
	for (auto i = 0; i < CONSUMER_COUNT; ++i)
	{
		ftr_trace_consumers.push_back(std::move(std::async(std::launch::async, &ProcessTrace, std::ref(trace_chunk_store))));
	}
	
	//Wait for all future threads to finish

	std::for_each(ftr_trace_producers.begin(), ftr_trace_producers.end(), [=](std::future<bool>& future) {	finished_producing = future.get();	});
	//File no longer needed once all lines are read in
	input_file.close();

	std::for_each(ftr_trace_consumers.begin(), ftr_trace_consumers.end(), [=](std::future<bool>& future) {	finished_outputting = future.get(); });
	//File no longer needeed once all traces are outputted
	file2.close();

	//all analysis has been done, so cleanup and output to file

	file3.open(ANALYSIS_OUTPUT_FILE_PATH, std::ios_base::out);
	analytics.Output(file3);
	file3.close();

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

