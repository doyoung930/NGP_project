#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <unordered_map>
#include <array>
#include <random>
#include "protocol.h"
using namespace std;

#pragma comment (lib,"WS2_32.LIB")

struct glmvec3 {
	float x, y, z;
};