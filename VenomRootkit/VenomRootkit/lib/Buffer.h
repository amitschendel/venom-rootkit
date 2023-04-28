#pragma once

#include "../Config.h"
#include "Vector.h"

using PagedBuffer = Vector<const char, PagedPool, POOL_TAG>;
using NonPagedBuffer = Vector<unsigned char, NonPagedPool, POOL_TAG>;
