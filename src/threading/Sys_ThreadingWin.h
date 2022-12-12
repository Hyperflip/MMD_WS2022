#pragma once

#include <cstdint>
#include <thread>
#include <string>
#include <map>

typedef void (*thread_t)(void*);

typedef uintptr_t threadHandle_t;
constexpr uintptr_t INVALID_THREAD_HANDLE = ~static_cast<uintptr_t>(0u);

std::map<threadHandle_t, std::string> names;
std::map<threadHandle_t, std::thread*> threads;

struct threadCreateParam_t {
	threadCreateParam_t()
		: function(nullptr)
		, params(NULL)
		, name("") {}

	thread_t function;
	void* params;
	const char* name;
};

/*
 * Creates a thread on the platform based on the provided parameters
 */
 uintptr_t Sys_CreateThread(threadCreateParam_t& params) {
	std::thread* thread = new std::thread(params.function, params.params);
	threadHandle_t id = *static_cast<unsigned int*>(static_cast<void*>(&thread->get_id()));
	names[id] = params.name;
	threads[id] = thread;
	return id;
 }

 /*
 * Get the ID / handle of the thread this function was called from
 */
 threadHandle_t Sys_GetCurrentThreadID() {
	 threadHandle_t id = *static_cast<unsigned int*>(static_cast<void*>(&std::this_thread::get_id()));
	 return id;
 }

 /*
 * Returns true when the thread this function was called on, has the same ID as the provided one
 */
 bool Sys_IsCallingThread(threadHandle_t threadHandle) {
	 return threads[threadHandle]->get_id() == std::this_thread::get_id();
 }

 /*
 * Sets the name of the provided thread
 */
 void Sys_SetThreadName(threadHandle_t threadHandle, const char* name) {
	 names[threadHandle] = name;
 }

 /*
 * Waits for the thread in question to finish execution
 */
 void Sys_WaitForThread(threadHandle_t threadHandle) {
	 threads[threadHandle]->join();
 }

 /*
 * Destroys the thread in question
 */
 void Sys_DestroyThread(threadHandle_t threadHandle) {
	 threads[threadHandle]->~thread();
	 delete threads[threadHandle];
 }


