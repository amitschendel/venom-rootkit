#pragma once

#include "../Config.h"
#include "Vector.h"

using PagedBuffer = Vector<char, PagedPool, POOL_TAG>;
using NonPagedBuffer = Vector<char, NonPagedPool, POOL_TAG>;
