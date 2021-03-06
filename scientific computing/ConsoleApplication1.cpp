// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
  
#include "stdafx.h"
#include <array>
#include <queue>
#include <mutex>
#include <thread>
#include <iostream>
#include <optional>
#include <chrono>

using elementType = std::array<int, 20000>;

class Queue {
	int limit;
	std::queue<elementType> data_queue;
	std::mutex mut;

public:
	Queue(int limit = 100) {
		this->limit = limit;
	}

	bool push(elementType element) {
		std::lock_guard<std::mutex> lokaloka(mut);
		
		if (data_queue.size() < limit)
		{
			data_queue.push(element);
			return true;
		}
		return false;
	}

	std::optional<elementType> pop() {
		std::lock_guard<std::mutex> lokaloka(mut);
		if (data_queue.size() > 0)
		{
			elementType lastElement = data_queue.front();
			data_queue.pop();
			return std::make_optional(lastElement);
		}

		return std::nullopt;
	}

	size_t size() {
		return data_queue.size();
	}

};

class Producer {
	Queue &queue;
	int addedTablesCounter = 0;
	int tablesAmount;

public:
	Producer(Queue &queue, int tablesAmount) : queue(queue) {
		this->tablesAmount = tablesAmount;
	}

	void produceTable() {

		while (addedTablesCounter < tablesAmount)
		{
			elementType elementToPush;
			while (!queue.push(elementToPush)){}
			addedTablesCounter++;
		}
	}

	bool isWorking() {
		return addedTablesCounter < tablesAmount;
	}
};

class Consumer {
	Queue &queue;
	Producer &producer;

public:
	Consumer(Queue &queue, Producer &producer) : queue(queue), producer(producer) {}
	int sortedTablesCounter = 0;

	void takeFromTable() {

		//std::cout << "takeFromTable";


		while (queue.size() > 0 || producer.isWorking())
		{
			std::optional<elementType> poppedEffect = queue.pop();
			if (poppedEffect.has_value())
			{
				elementType poppedValue = poppedEffect.value();
				std::sort(poppedValue.begin(),
					poppedValue.end());
				sortedTablesCounter++;
			}
		}

		//std::cout << "\ncounter: " << sortedTablesCounter;
	}
};

int main()
{
	Queue queue;
	Producer producer(queue, 1000);


	std::thread producerThread = std::thread(&Producer::produceTable, &producer);
	std::vector<std::thread> vectorOfThreads;
	std::vector<Consumer> vectorOfConsumers;

	int threadsNumber = 2;

	vectorOfThreads.reserve(threadsNumber);
	vectorOfConsumers.reserve(threadsNumber);

	for (int i = 0; i < threadsNumber; i++) {
		vectorOfConsumers.push_back(Consumer(queue, producer));
	}

	auto start = std::chrono::steady_clock::now();

	for (int i = 0; i < threadsNumber; i++) {
		vectorOfThreads.push_back(std::thread(&Consumer::takeFromTable, 
			&vectorOfConsumers[i]));
	}


	producerThread.join();

	for (std::thread & th : vectorOfThreads) {
		if (th.joinable())
			th.join();
	}

	auto end = std::chrono::steady_clock::now();

	std::cout << "\n\nElapsed time in seconds : "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
		<< " ms\n\n";

    return 0;
}

