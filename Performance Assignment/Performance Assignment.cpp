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
#define FILE_FLAG_NO_BUFFERING

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

struct Analytic
{


};

// Container for each 'trace' entry, including parsing data from string.
struct Trace
{

	//When constructing a trace, we 
	Trace(std::string& str)
	{
		//Going for speed, so can ignore a completely abstract approach

		int t;
		m_session_id = ExtractTagValue(str, "sessionid", t);
		m_ip_address = ExtractTagValue(str, "ipaddress",t);
		m_browser = ExtractTagValue(str, "browser", t);

		ExtractMultipleTagValues(m_visited_sites, str, "path");
		ExtractMultipleTagValues(m_visited_time, str, "time");

	}

	void ExportToJSON(std::ofstream& oss)
	{
		//const int size = 300;
		//char buffer[size];
		//int cx;

		//Opening
		/*cx = snprintf(buffer, sizeof(buffer), "\t\"entry\": {\n", m_session_id.c_str());
		oss << buffer;
		
		//test
		cx = snprintf(buffer, sizeof(buffer), "\t\t\"sessionid\": \"%s\",\n\t\t\"ipaddress\": \"%s\",\n\t\t\"browser\": \"%s\",\n\t\t\"path\": [\n", m_session_id.c_str(), m_ip_address.c_str(), m_browser.c_str());
		oss << buffer;

		//COntent
		/*cx = snprintf(buffer, sizeof(buffer), "\t\t\"sessionid\": \"%s\",\n", m_session_id.c_str());
		oss << buffer;
		cx = snprintf(buffer, sizeof(buffer), "\t\t\"ipaddress\": \"%s\",\n", m_ip_address.c_str());
		oss << buffer;
		cx = snprintf(buffer, sizeof(buffer), "\t\t\"browser\": \"%s\",\n", m_browser.c_str());
		oss << buffer;*/

		/*std::string strtest;
		std::string strtest2;
		for (int i = 0; i < m_visited_sites.size(); ++i)
		{
			const int size = 100;
			char buffer[size];
			int c = snprintf(buffer, sizeof(buffer), "\t\t\t\"%s\",\n", m_visited_sites[i].c_str());

			char buffer2[size];
			int s = snprintf(buffer2, sizeof(buffer2), "\t\t\t\"%s\",\n", m_visited_time[i].c_str());

			strtest += buffer;
			strtest2 += buffer2;
		}
		
		oss << strtest.c_str();

		oss << "\t\t],\n\t\t\"time\": [\n" << strtest2.c_str() << "\t\t]\n";

		//Closing bracket
		oss << "\t},\n";*/


		oss << "\t\"entry\": {\n\t\t\"sessionid\": \"" << m_session_id.c_str() << "\",\n\t\t\"ipaddress\": \"" << m_ip_address.c_str() << "\",\n\t\t\"browser\": \"" << m_browser.c_str() << "\",\n\t\t\"path\": [\n";
		
		std::string timeStr = "\t\t\"time\": [\n";
		const int site_size = m_visited_sites.size();
		for (int i = 0; i < site_size; ++i)
		{
			bool do_end_comma = (i != (m_visited_sites.size() - 1));


			oss << "\t\t\t\"" << m_visited_sites[i].c_str() << "\"";
			std::string temp = "\t\t\t\"" + m_visited_time[i]+ "\"";
			if (do_end_comma)
			{
				oss << ",\n";
				temp += ",\n";
			}
			else
			{
				oss << "\n\t\t]\n";
				temp += "\n\t\t]\n";
			}
			timeStr += temp;
		}

		oss << timeStr.c_str();
		oss << "\t},\n";


	};

	void ExtractMultipleTagValues(std::vector<std::string>& vec, std::string str, std::string tag)
	{
		bool eof = false;
		while (!eof)
		{
			int end_pos = -1;
			std::string val = ExtractTagValue(str, tag, end_pos);
			if (val != "")
			{
				vec.push_back(val);
				str = str.substr(end_pos);
			}
			else
			{
				eof = true;
			}
		}
	}

	std::string ExtractTagValue(std::string& str, std::string tag, int& end_pos)
	{
		//Simple function that returns
		size_t first_occurrence = str.find("<" + tag + ">");
		size_t last_occurrence = str.find("</" + tag + ">");
		
		const int first_tag_size = tag.length() + 2;

		//Ie did we find tags within this?
		if (first_occurrence != std::string::npos && last_occurrence != std::string::npos)
		{
			const int start_idx = first_occurrence + first_tag_size;
			end_pos = last_occurrence + first_tag_size + 1;
			return str.substr(first_occurrence + first_tag_size, last_occurrence - start_idx);
		}

		return "";
	}

	std::string GetSomething()
	{
		return m_ip_address;
	}

private:
	//For now all strings
	std::string m_session_id;

	//could just be a number?! probably not...
	std::string m_ip_address;

	//Lookup table?
	std::string m_browser;

	//These are actually a pair
	std::vector<std::string> m_visited_sites;
	std::vector<std::string> m_visited_time;
};

std::mutex string_storage_mutex, string_queue_mutex, string_ready_mutex;
std::condition_variable string_ready_condition;
std::queue<std::string> string_queue;

bool finished_producing = false;
bool finished_outputting = false;


std::mutex trace_storage_mutex, trace_queue_mutex;
std::condition_variable trace_ready_condition;
std::queue<Trace> trace_queue;

std::queue<std::queue<std::string>> string_chunk_store;

//#define DEBUG
#define XML_FILE_PATH "testdata_large.json"

bool string_ready = false;
bool trace_ready = false;


std::mutex string_chunk_storage_mutex;	// Lock the mutex to protect the data store, as it will be accessed from other threads for short periods...
std::condition_variable string_chunk_ready_condition; // Signal that we have prepared the data (it is ready for processing on the other thread).

std::mutex trace_chunk_storage_mutex;	// Lock the mutex to protect the data store, as it will be accessed from other threads for short periods...
std::condition_variable trace_chunk_ready_condition; // Signal that we have prepared the data (it is ready for processing on the other thread).
std::queue<std::queue<Trace>> trace_chunk_store;

#define CHUNK_SIZE 1

std::ifstream file;
std::mutex string_queue_mutex2;

std::ofstream file2, file3;
std::mutex string_queue_mutex3;


bool ProduceStringsFromFile(std::ifstream& file)
{
	//String producer, ends when no more lines can be read from the file.
	
		
	bool remaining_data = true;
	while (remaining_data)
	{
		{
			//Prepare chunk of lines read from file
			std::string line;
			std::queue<std::string> chunk;

			//Could unroll this, kept since want it to be generic (to test chunk size speed)
			std::lock_guard<std::mutex> queue_lock2(string_queue_mutex2);
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

bool ProcessData(std::queue<std::queue<std::string>> &data_storage_queue, std::queue<std::queue<Trace>> &trace_storage_queue)
{
	bool more_data = true;
	while (more_data)	// Do this forever (obviously, in reality, there should be some mechanism here to end this loop...)
	{
		std::queue<std::string> string_chunk;
		int queue_size=0;
		{
			std::unique_lock<std::mutex> data_ready_lock(string_chunk_storage_mutex);	// Mutex for the condition variable.
			string_chunk_ready_condition.wait(data_ready_lock, [] {return true;});	// Wait then check the queue for data - does data exist?
			{
				std::lock_guard<std::mutex> queue_lock(string_queue_mutex);
				{
					if (!data_storage_queue.empty())
					{
						string_chunk = data_storage_queue.front();	// Take the data off the queue and make a copy, then unlock the mutex, so we can process the data locally and the other threads can continue.
						data_storage_queue.pop();
						queue_size = string_chunk.size();
					}
					else
					{
						more_data = !finished_producing;
					}
				}
			}
		}

		//IE do something to our chunk
		std::queue<Trace> trace_chunk;
		for (int n = queue_size; n > 0; --n)
		{
#ifdef DEBUG
			std::cout << "[Data Chunk Consumer]\tThread [" << std::this_thread::get_id() << "]\n";
#endif
			Trace t(string_chunk.front());
			{
				std::lock_guard<std::mutex> queue_lock3(string_queue_mutex3);
				t.ExportToJSON(file2);
			}
			trace_chunk.push(t);
			string_chunk.pop();
			//Did we push any data up?
			
		}

		if (!trace_chunk.empty())
		{
#ifdef DEBUG
			std::cout << "[String Chunk Consumer]\t " << trace_chunk.size() << " traces pulled from strings.\t" << "\t|\tThread [" << std::this_thread::get_id() << "]\n";
#endif
			std::lock_guard<std::mutex> queue_lock(trace_queue_mutex);
			trace_chunk_store.push(trace_chunk);
			{
				std::lock_guard<std::mutex> queue_lock(trace_chunk_storage_mutex);
				trace_chunk_ready_condition.notify_one();
			}
		}
	}

	return true;
}

void ProcessData2(std::queue<std::queue<Trace>> &trace_storage_queue)
{
	bool more_data = true;
	while (more_data)	// Do this forever (obviously, in reality, there should be some mechanism here to end this loop...)
	{
		//more_data = false;
		int i = 0;
		std::queue<Trace> trace_chunk;
		int queue_size = 0;
		{
			std::unique_lock<std::mutex> data_ready_lock(trace_chunk_storage_mutex);	// Mutex for the condition variable.
			trace_chunk_ready_condition.wait(data_ready_lock, [&] { return true; });	// Wait then check the queue for data - does data exist?
			{
				std::lock_guard<std::mutex> queue_lock(trace_queue_mutex);
				{
					if (!trace_storage_queue.empty())
					{
						trace_chunk = trace_storage_queue.front();	// Take the data off the queue and make a copy, then unlock the mutex, so we can process the data locally and the other threads can continue.
						trace_storage_queue.pop();
						queue_size = trace_chunk.size();
					}
					else
					{
						more_data = !finished_outputting;
					}
				}
			}
			//process the chunk here, remove f
		}

		for (int n = queue_size; n > 0; --n)
		{
			Trace t = trace_chunk.front();
			trace_chunk.pop();
			file3 << t.GetSomething();
		}
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
	file.open("testdata_large.json", std::ios_base::in);
	if (!file.is_open()) C_ASSERT("ERROR - INCORRECT FILE PATH");

	file2.open("AsJson.json", std::ios_base::out);
	file2 << "{\n";

	file3.open("AsJsonAnalysis.json", std::ios_base::out);

	//Produces strings from a given file
	std::future<bool> ftr_string_producer(std::async(std::launch::async, ProduceStringsFromFile, std::ref(file)));
	std::future<bool> ftr_string_producer1(std::async(std::launch::async, ProduceStringsFromFile, std::ref(file)));

	//Multithreaded extract data from these strings into another queue (Less is faster...?)
	std::future<bool> ftr_trace_producer_1(std::async(std::launch::async, ProcessData, std::ref(string_chunk_store), std::ref(trace_chunk_store)));
	std::future<bool> ftr_trace_producer_2(std::async(std::launch::async, ProcessData, std::ref(string_chunk_store), std::ref(trace_chunk_store)));
	std::future<bool> ftr_trace_producer_3(std::async(std::launch::async, ProcessData, std::ref(string_chunk_store), std::ref(trace_chunk_store)));

	//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	//std::future<void> ftr_trace_producer_5(std::async(std::launch::async, ProcessData2, std::ref(trace_chunk_store)));
	//std::future<void> ftr_trace_producer_6(std::async(std::launch::async, ProcessData2, std::ref(trace_chunk_store)));
	//std::future<void> ftr_trace_producer_7(std::async(std::launch::async, ProcessData2, std::ref(trace_chunk_store)));/*
	//std::future<void> ftr_trace_producer_8(std::async(ProcessData2, std::ref(trace_chunk_store)));
	//std::future<void> ftr_trace_producer_9(std::async(ProcessData2, std::ref(trace_chunk_store)));
	//std::future<void> ftr_trace_producer_0(std::async(ProcessData2, std::ref(trace_chunk_store)));*/

	//Process each Trace to analyse data

	//Process each Trace again to get the json :)

	//Wait for all future threads to finish
	finished_producing = ftr_string_producer.get();
	finished_producing = ftr_string_producer1.get();

	file.close();

	finished_outputting = ftr_trace_producer_1.get();
	finished_outputting = ftr_trace_producer_2.get();
	finished_outputting = ftr_trace_producer_3.get();

	file2 << "}";
	file2.close();

	//ftr_trace_producer_5.get();
	//ftr_trace_producer_6.get();
	//ftr_trace_producer_7.get();
	/*ftr_trace_producer_8.get();
	ftr_trace_producer_9.get();
	ftr_trace_producer_0.get();*/

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

