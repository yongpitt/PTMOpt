#pragma once

#include <map>

#include "Transaction.h"

#define READ_TYPE 0
#define WRITE_TYPE 1
#define DELETE_TYPE 2

typedef std::vector<TIDS> LockCycles;

class LockCondition
{
public:
  LockCondition():get(false) {}

	bool get; //get it?
	TIDS owners; //Current owners
	LockCycles deadlock_ids; //the current deadlock ids
};

class LockInfo
{
public:
	class Job{
	public:
		Job():itemId_(-1), type_(-1){}
		Job (int itemid, int type, const std::string& fileName):itemId_(itemid), type_(type), fileName_(fileName){}

		int itemId_;//for R/W
		int type_;
		std::string	 fileName_;//For R/W
	};
	typedef std::map<int, Job> WaitingQueue;
public:
	LockInfo():itemId(-1), type(-1){}

	TIDS owners; //Current owners
	int itemId;
	int type; // 0-read 1-write
	std::string	fileName;

public:
	/* Insert one transaction to the waiting queue
	* tid: transaction id
	* needWrite: want a write lock or not
	*/
	void setWaitingQueue(int tid, const Job& job);
	//free the transaction from the waiting queue
	void unsetWaitingQueue(int tid);
	//get current waiting transactions
	TIDS getWaitingTransactions();

private:
	//tid, need upgrade to WL?
	WaitingQueue waitingQueue;
};

class LockManager
{
public:
	typedef std::map<int, LockInfo> ItemLocks;
	typedef std::map<std::string, LockInfo> FileLocks;

public:
	LockManager(void);
	~LockManager(void);

	//lock the item and return the trans ids in the cycle if detect a dead lock
	// type: 0-read 1-write -1-error
	LockCondition Lock (int tid, int itemid, int type, const std::string&	fileName);

	//lock the whole file  and return the trans ids in the cycle if detect a dead lock
	LockCondition Lock (int tid, const std::string&	fileName);

	//Reinit the class
	void Clear();

	//Free all the locks that owned by tid
	void FreeLock(int tid);

private:
	//lockId = tid + itemid << 10 + type << 20, LockInfo
	ItemLocks	itemLocks;
	//file lock : fileName, LockInfo
	FileLocks fileLocks;
};

class DeadLockDetector
{
public:

public:
	DeadLockDetector(void);
	~DeadLockDetector(void);

	LockCycles Detect(const LockManager::ItemLocks& ils, const LockManager::FileLocks& fls);
};
