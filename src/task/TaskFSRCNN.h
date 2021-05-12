#pragma once

#include "Task.h"

struct TaskFSRCNN : public Task {
	unsigned char ker[4];
	unsigned char pad[4];
	unsigned char ch_n[3];
};
