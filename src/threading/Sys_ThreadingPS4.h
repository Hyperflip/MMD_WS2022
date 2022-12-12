#pragma once

#include <cstdint>
#include <map>

#include <kernel.h>

typedef void (*thread_t)(void*);

typedef uintptr_t threadHandle_t;
constexpr uintptr_t INVALID_THREAD_HANDLE = ~static_cast<uintptr_t>(0u);

std::map<threadHandle_t, ScePthread*> threads;

struct threadCreateParam_t {
	threadCreateParam_t()
		: function(nullptr)
		, params(NULL)
		, name("")
	{}

	thread_t function;
	void* params;
	const char* name;
};

/*
 * Creates a thread on the platform based on the provided parameters
 */
threadHandle_t Sys_CreateThread(threadCreateParam_t& params) {
	ScePthread* thread = new ScePthread();
	threadHandle_t id = scePthreadCreate(thread, NULL, reinterpret_cast<void* (*)(void*)>(params.function), params.params, params.name);
	threads[id] = thread;
	return id;
}

/*
 * Get the ID / handle of the thread this function was called from
 */
threadHandle_t Sys_GetCurrentThreadID() {
	return scePthreadGetthreadid();
}

/*
 * Returns true when the thread this function was called on, has the same ID as the provided one
 */
bool Sys_IsCallingThread(threadHandle_t threadHandle) {
	return threadHandle == scePthreadGetthreadid();
}

/*
 * Sets the name of the provided thread
 */
void Sys_SetThreadName(threadHandle_t threadHandle, const char* name) {
	scePthreadRename(*threads[threadHandle], name);
}

/*
 * Waits for the thread in question to finish execution
 */
void Sys_WaitForThread(threadHandle_t threadHandle) {
	scePthreadJoin(*threads[threadHandle], nullptr);
}

/*
 * Destroys the thread in question
 */
void Sys_DestroyThread(threadHandle_t threadHandle) {
	int result = scePthreadCancel(*threads[threadHandle]);
	threads[threadHandle]->~ScePthread();
	delete threads[threadHandle];
}