#include "stdafx.h"
#include "Transaction.h"

Transaction::Transaction(void)
{
}


Transaction::~Transaction(void)
{
}

bool Transaction::doOne( const TranManager::Operation& oper )
{
  return false;
}

Operations Transaction::getCommitedResults()
{
	return Operations();
}
