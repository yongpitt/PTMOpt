#pragma once

#include "TranManager.h"
#include "Transaction.h"

class Scheduler
{
public:
  Scheduler(void);
	~Scheduler(void);

	void ScheduleTransactions(Operations Transactions);

	Operations GetCommitedOutput();

private:
	Operations commitedOps;
};

