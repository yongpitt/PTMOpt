#pragma once

#include "TranManager.h"

class DataManager
{
public:

  DataManager(void);

	int UpdateDatabase(Operations Ops);

	~DataManager(void);
};

