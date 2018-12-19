#include <iostream>
#include <chrono>
#include <vector>
#include <queue>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>



using namespace std;

mutex print_mutex; 
mutex accum_mutex;

mutex value_mutex;
condition_variable cond_var;

/* producer consumer variables */
condition_variable empty_cond;
condition_variable full_cond;
mutex pc_mutex; 
bool is_empty = true, bool is_full = false;
int max_size = 500, size = 0;
vector<int> goods;


int square (int x) {
	print_mutex.lock();
	cout << "square(): thread id = " << this_thread::get_id() << endl;
	cout << "square(): x = " << x << endl;
	print_mutex.unlock();
	return x * x;
	this_thread::sleep_for(chrono::milliseconds(100));
}

void add_square_atomic (atomic<int>& accum, int x) {
	accum += x*x;
} 

void add_square (int& accum, int x) {
	accum += x*x;
} 

void print_id (int id) {
	print_mutex.lock();
	cout << "thread id  " << id << endl;
	print_mutex.unlock();
}

void run_with_threads () {
	cout << "running with threads ... " << endl;
	atomic<int> accum(0);
	vector<std::thread> threads;
	int num_threads = 20; 
	for (int i = 0; i < num_threads; i++)
		threads.push_back(thread(add_square_atomic, ref(accum), i));
	
	for (auto& x : threads)
		x.join();

	cout << endl << "accum: " << accum << endl;

	// run print_id
	for (int i = 0; i < num_threads; i++)
		threads[i] = thread(print_id, i);
	for (auto& x : threads)
		x.join();
}

void run_with_future () {
	cout << "running with future objects ... " << endl;
	int accum = 0;
	vector<future<int>> futures;
	int num_futures = 20;
	for (int i = 0; i < num_futures; i++) {
		future<int> a = async(launch::async, square, i);
		futures.push_back(move(a));
	}

	for (auto& a  : futures)
		accum += a.get();

	cout << "accum: " << accum << endl;
}

void producer_consumer () {
	bool done = false;
	thread producer([&] () {
		while (true) {
			unique_lock<mutex> lock(pc_mutex); /* acquire lock */
			while (is_full) /* wait until goods is not full */
				cond_full.wait(lock);
			goods.push_back(i); 
			size++;
			/* buffer no longer empty */
			is_empty = false; 
			cond_empty.notify_one();
		}
				});

	thread consumer( [&] () {
			while (true) {
				unique_lock<mutex> lock(pc_mutex); /* acquire lock */
				while (is_empty) /* wait until goods is not empty */
					cond_empty.wait(lock);
				goods.pop_back(); 
				size--;
				/* buffer no longer full */
				is_full = false; 
				cond_full.notify_one();
			}
				});
	producer.join();
	consumer.join();
	cout << "net: " << size << endl;
}

int main () {
	// run the accumulator	
	run_with_threads();
	cout << "main: " << this_thread::get_id() << endl;
	run_with_future();

	// condition variables
	int value = 100; 
	bool notified = false;
	
	thread get_value ([&] () {
		unique_lock<mutex> lock(value_mutex); // lock value_mutex
		while (!notified) // unlock value_mutex until notified
			cond_var.wait(lock);
		// value_mutex is once again locked
		
		cout << "get_value thread:  value is " << value << endl; 
		// value_mutex is unlocked at the end due to scope (RAII)
	});

	thread change_value ([&] () {
		value = 20;
		notified = true;
		cond_var.notify_one();
	});

	get_value.join();
	change_value.join();

	return 0;
}
