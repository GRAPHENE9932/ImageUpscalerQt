#pragma once

#include "Task.h"

struct TaskSRCNN : public Task {
	unsigned char ker[3];
	unsigned char pad[3];
	unsigned char ch_n[2];
};
