#include "StdAfx.h"

#include "LockManager.h"

using namespace std;

LockManager::LockManager(void){}

LockManager::~LockManager(void){}

LockCondition LockManager::Lock( int tid, int itemid, int type, const std::string& fileName )
{
  int lockId = tid + (itemid << 10) + (type << 20);

	LockCondition lc; //the return lock 
	LockInfo& ili = itemLocks[lockId]; //item lock info
	LockInfo& fli = fileLocks[fileName]; //file lock info

	DeadLockDetector detector;//for dead lock
	LockInfo::Job job(itemid, type, fileName);

	//for file locks only with WL
	if (!fli.owners.empty() && *fli.owners.begin() == tid /*&& type == READ_TYPE*/){//WL | self-owned | Want to read or write
		lc.get = true;
		lc.owners = fli.owners;
		return lc;
	}
	else if (!fli.owners.empty() && *fli.owners.begin() != tid && type == READ_TYPE){//WL | other-owned | Want to read
		fli.setWaitingQueue(tid, job);

		lc.get = false;
		lc.owners = fli.owners;
		lc.deadlock_ids = detector.Detect(itemLocks, fileLocks);
		return lc;
	}
	else if (!fli.owners.empty() && *fli.owners.begin() != tid && type == WRITE_TYPE){//WL | other-owned | Want to write
		fli.setWaitingQueue(tid, job);

		lc.get = false;
		lc.owners = fli.owners;
		lc.deadlock_ids = detector.Detect(itemLocks, fileLocks);
		return lc;
	}

	//for item locks
	if (itemLocks.find(lockId) == itemLocks.end()){//no one lock it, get the lock
		ili.owners.insert(tid);
		ili.fileName = fileName;
		ili.itemId = itemid;
		ili.type = type;

		lc.get = true;
		lc.owners.insert(tid);
		return lc;
	}
	else{//lock on this item exist, 4 types: R/W SELF/OTHERS
		if (ili.type == READ_TYPE && ili.owners.find(tid) != ili.owners.end() && type == READ_TYPE){// RL | self-owned | Want to read
			lc.get = true;
			lc.owners = ili.owners;
			return lc;
		}
		else if (ili.type == READ_TYPE && ili.owners.find(tid) != ili.owners.end() && type == WRITE_TYPE){// RL | self-owned | Want to write
			if (ili.owners.size() == 1){//only me owns this read lock
				ili.type = WRITE_TYPE;//Upgrade RL 2 WL

				lc.get = true;
				lc.owners = ili.owners;
				return lc;
			}
			else{//Others also sharing this lock
				ili.setWaitingQueue(tid, job);// insert current requesting tid to queue with the req to up RL 2 WL

				lc.get = false;
				lc.owners = ili.owners;
				lc.deadlock_ids = detector.Detect(itemLocks, fileLocks);
				return lc;
			}
		}
		else if (ili.type == WRITE_TYPE && ili.owners.find(tid) != ili.owners.end() && type == READ_TYPE){// WL | self-owned | Want to read
			lc.get = true;
			lc.owners = ili.owners;
			return lc;
		}
		else if (ili.type == WRITE_TYPE && ili.owners.find(tid) != ili.owners.end() && type == WRITE_TYPE){// WL | self-owned | Want to write
			lc.get = true;
			lc.owners = ili.owners;
			return lc;
		}
		else if (ili.type == READ_TYPE && ili.owners.find(tid) == ili.owners.end() && type == READ_TYPE){// RL | other-owned | Want to read
			ili.owners.insert(tid);

			lc.get = true;
			lc.owners = ili.owners;
			return lc;
		}
		else if (ili.type == READ_TYPE && ili.owners.find(tid) == ili.owners.end() && type == WRITE_TYPE){// RL | other-owned | Want to write
			ili.setWaitingQueue(tid, job);// insert current requesting tid to queue with the req to up RL 2 WL

			lc.get = false;
			lc.owners = ili.owners;
			lc.deadlock_ids = detector.Detect(itemLocks, fileLocks);
			return lc;
		}
		else if (ili.type == WRITE_TYPE && ili.owners.find(tid) == ili.owners.end() && type == READ_TYPE){// WL | other-owned | Want to read
			ili.setWaitingQueue(tid, job);

			lc.get = false;
			lc.owners = ili.owners;
			lc.deadlock_ids = detector.Detect(itemLocks, fileLocks);
			return lc;
		}
		else if (ili.type == WRITE_TYPE && ili.owners.find(tid) == ili.owners.end() && type == WRITE_TYPE){// WL | other-owned | Want to write
			ili.setWaitingQueue(tid, job);

			lc.get = false;
			lc.owners = ili.owners;
			lc.deadlock_ids = detector.Detect(itemLocks, fileLocks);
			return lc;
		}
	}
	assert (0);

	return lc;
}

LockCondition LockManager::Lock( int tid, const std::string& fileName )
{//The whole file lock will always be the WL
	LockCondition lc;
	LockInfo& li = fileLocks[fileName];
	DeadLockDetector detector;

	assert (li.owners.size() <= 1);

	if(li.owners.size() == 0){
		//no one owns it ???
	}
	else if (*li.owners.begin() == tid){//already get it
		lc.get = true;
		lc.owners = li.owners;
		return lc;
	}
	else{//others owning it
		LockInfo::Job job;
		job.type_ = DELETE_TYPE;

		li.setWaitingQueue(tid, job);

		lc.get = false;
		lc.owners = li.owners;
		lc.deadlock_ids = detector.Detect(itemLocks, fileLocks);
		return lc;
	}

	return lc;
}

void LockManager::FreeLock( int tid )
{
	//free file lock
	//for each(FileLocks::value_type& pair in fileLocks){
		//if (tid == *pair.second.owners.begin()){// I am currently owning one lock
			//get one out from the waiting queue with delete_type
			//if has one, let it get the DL and pop it out from the queue
			//if none, make owners null ????
		//}
	//}
	//free item lock
	//if 
}

void LockManager::Clear()
{
	itemLocks.clear();
	fileLocks.clear();
}

//////////////////////////////////////////////////////////////////////////

DeadLockDetector::DeadLockDetector( void ){}

DeadLockDetector::~DeadLockDetector( void ){}

LockCycles DeadLockDetector::Detect(const LockManager::ItemLocks& ils, const LockManager::FileLocks& fls){
	return LockCycles();
}

void LockInfo::setWaitingQueue( int tid, const LockInfo::Job& job )
{
	if (waitingQueue.find(tid) != waitingQueue.end()){// this tid has a pending job
		const auto& pendingJob = waitingQueue[tid];
		if (pendingJob.type_ == DELETE_TYPE)
			return;//Need whole access already, no need to insert to the queue again
		else if (pendingJob.type_ == WRITE_TYPE){
			if (job.type_ == DELETE_TYPE)
				waitingQueue[tid] = job;//WL -> DL upgrade
			else
				return;
		}
		else if (pendingJob.type_ == READ_TYPE){
			if (job.type_ == DELETE_TYPE || job.type_ == WRITE_TYPE)
				waitingQueue[tid] = job;//RL -> DL/WL upgrade
			else
				return;
		}
	}
	else
		waitingQueue[tid] = job;
}

void LockInfo::unsetWaitingQueue( int tid )
{
	waitingQueue.erase(tid);
}

TIDS LockInfo::getWaitingTransactions()
{
	TIDS ret;
	for_each(waitingQueue.begin(), waitingQueue.end(), [&ret](const WaitingQueue::value_type& val){
		ret.insert(val.first);
	});

	return ret;
}
