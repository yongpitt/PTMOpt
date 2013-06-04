#include "StdAfx.h"

#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/lexical_cast.hpp>
#include <map>

#include "DataManager.h"
#include "TranManager.h"

using namespace std;

extern string ReadAll(const char* fileName);
extern vector<string> ReadLines1(const char* fileName);

DataManager::DataManager(void)
{
     
}

int DataManager::UpdateDatabase(Operations Ops)
{
  TranManager::Operation Op;
	unsigned int opIndex;
	string fileName, ClientName, Phone;
	int RecordId;
	int lineNum, lineIndex;
	int numOps = Ops.size();
	//vector<string> &theFile;
	map<string, vector<string> > outBuff;
	map<string, vector<string> >::iterator buffIter;
	string::size_type rPos;
	string RecordIdStr;
	int rId;

	for(opIndex=0; opIndex<numOps; opIndex++)
	{
	  Op = Ops[opIndex];
	  if(Op.m1 == TranManager::WRITE)
	  {
		 fileName = Op.m3;
		 RecordId = Op.m4;
		 ClientName = Op.m5;
		 Phone = Op.m6;
		 
		 if(outBuff.find(fileName) == outBuff.end())
		 {
			 outBuff[fileName] = ReadLines1(fileName.c_str()); 
		 }

		 RecordIdStr = boost::lexical_cast<string>(RecordId);
		 vector<string> &theFile = outBuff[fileName];
		 lineNum = theFile.size();

		 for(lineIndex = 0; lineIndex<lineNum; lineIndex++)
		 {
			 rPos = theFile[lineIndex].find_first_of(",",0);
			 rId = atoi(theFile[lineIndex].substr(0, rPos).c_str());
			 if(rId == RecordId)
			 {
			     theFile[lineIndex] = RecordIdStr + "," + ClientName + "," + Phone;
				 break;
			 }
		 }

		 if(lineIndex == lineNum)
		 {
			 theFile.push_back(RecordIdStr + "," + ClientName + "," + Phone);
		 }
	  }
	  else if(Op.m1 == TranManager::DEL)
	  {
		 fileName = Op.m3;
		 RecordId = Op.m4;	     

		 if(outBuff.find(fileName) == outBuff.end())
		 {
			 outBuff[fileName] = ReadLines1(fileName.c_str()); 
		 }

		 RecordIdStr = boost::lexical_cast<string>(RecordId);
		 vector<string> &theFile = outBuff[fileName];
		 lineNum = theFile.size();

 		 //if( 0>RecordId || 10000<RecordId)
		 //{
            theFile.erase(theFile.begin(),theFile.end());	    	 
		 //}

		 /*
	     for(lineIndex = 0; lineIndex<lineNum; lineIndex++)
		 {
			 rPos = theFile[lineIndex].find_first_of(",",0);
			 rId = atoi(theFile[lineIndex].substr(0, rPos).c_str());
			 if(rId == RecordId)
			 {
			     theFile.erase(theFile.begin()+lineIndex); 
				 break;
			 }
		 }
		 */

	  }
	  
     
	}

	//write to files


	for(buffIter = outBuff.begin(); buffIter!=outBuff.end(); buffIter++)
	{
		ofstream ofs(buffIter->first.c_str(), ios_base::out);
		
		lineNum = buffIter->second.size();
		for(lineIndex=0; lineIndex<lineNum; lineIndex++)
		{
			ofs << buffIter->second[lineIndex] << "\n" ;
		}
		ofs.close();
	}
	

	return 0;
}

DataManager::~DataManager(void)
{

}
