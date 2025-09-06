#pragma once
#include "pch.h"

namespace Util {
	ImVec2 CalcSize(short count = 1, bool spacing = true);
	ImVec2 CalcFrameSize(const char* text);
	bool ColorCombo(const char* label, int* pOutId);
	void GenerateCarcol();
};