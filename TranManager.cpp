#include "StdAfx.h"
#include "TranManager.h"

#include <boost/variant.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/foreach.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>

using namespace std;
using namespace boost::xpressive;

TranManager::TranManager()
{

}

TranManager::TranManager(fileList files)
{
  vector<vector<string>>::iterator fileIter;
	vector<string>::iterator lineIter;
	vector<string> CurrFile;
	string CurrLine;
	unsigned int CurrLinePos = 0;

	//variables used to hold parsed strings
	int fileIndex=0;
	int fileNum = files.size();
	//vector<int> FileCurrMode, FileCurrTranId;
	int FileCurrMode[10], FileCurrTranId[10], FileCurrReadLine[10], FileLineNum[10];
	int TranIdGlobal = 0;
	
	string  CurrFileName, CurrName;
	Operation CurrOp;

	if(fileNum > 10)
	{
		cerr << "two many files! " << "\n";
	}

	mark_tag op(1),opbegin(2), opabord(3), opcommit(4),
		mode(5), filename(6), id(7), name(8), phone(9),delim(10);

	cregex begin = (opbegin = 'B')
		       >> (delim= *_s)
		       >> (mode = (boost::xpressive::set= '0','1'));

	cregex commit = (opcommit = 'C')
		       >> (delim= *_s)
			   >> (_n |eos);

    cregex abord = (opabord = 'A')
		       >> (delim= *_s)
			   >> (_n |eos);

	//cregex phone = +_d >> '-'>> +_d>> '-'>> +_d;
	
	cregex fileop1 = (op = (boost::xpressive::set= 'R','W','D'))
		       >> (delim= *_s)
		       >> (filename= +(_w | '.'))
			   >> (delim= *_s) >> '('>> (delim= *_s)
			   >> (id= +_d) >>(delim= *_s) >> ','
			   >> (name= +_w) >>(delim= *_s) >> ','
			   >> (phone= +(_d | '-'))
			   >> (delim= *_s) >> ')';
    
	cregex fileop2 = (op = (boost::xpressive::set= 'R','W','D'))
		       >> (delim= +_s)
		       >> (filename= +(_w | '.'))
			   >> (delim= +_s) 
			   >> (id= +_d); 

	cregex fileop3 = (op = 'D')
		       >> (delim= +_s)
		       >> (filename= +(_w | '.'));

			  
			   
	//cregex fileop2 = op; 
		 //      >> delim
		//       >> filename
		//	   >> delim 
		//	   >> id;
			   
    cmatch what;

	//initialize
	for(fileIndex=0; fileIndex<fileNum; fileIndex++)
	{
		
	   FileCurrMode[fileIndex] = -1;
	   FileCurrTranId[fileIndex] = 0;
	   FileCurrReadLine[fileIndex] = 0;
	   FileLineNum[fileIndex] = files[fileIndex].size();

	   cout << "number of transactions in file" << fileIndex << ": " << FileLineNum[fileIndex] <<"\n";
	}

	int fileProcessed, linesProcessed;
	linesProcessed = 0;

	while(1) 
	{
		fileProcessed = 0;

		for(fileIndex=0; fileIndex<fileNum; fileIndex++)
		{
			CurrFile = files[fileIndex];
			
			if(FileCurrReadLine[fileIndex] < FileLineNum[fileIndex])
			{
				CurrLine = CurrFile.at(FileCurrReadLine[fileIndex]);
			    // mark_tag day(1), month(2), year(3), delim(4);

                // this regex finds a date
				
				if(regex_search(CurrLine.c_str(), what, begin))
			    {
			        std::cout << what[0] << '\n'; // whole match
					FileCurrMode[fileIndex] = atoi(what[mode].str().c_str());
				
					TranIdGlobal++;
					FileCurrTranId[fileIndex] = TranIdGlobal;

					CurrOp.m0 = FileCurrTranId[fileIndex];
					CurrOp.m1 = BEGIN;
					CurrOp.m2 = FileCurrMode[fileIndex];
					Transaction.push_back(CurrOp);			
					FileCurrReadLine[fileIndex]++;
					linesProcessed++;
			    }
				
				else if(regex_search(CurrLine.c_str(), what, commit))
				{
					std::cout << what[0] << '\n'; // whole match

					CurrOp.m0 = FileCurrTranId[fileIndex];
					CurrOp.m1 = COMMIT;
					CurrOp.m2 = FileCurrMode[fileIndex];

					Transaction.push_back(CurrOp);
					FileCurrReadLine[fileIndex]++;
					linesProcessed++;
				}
				
				else if(regex_search(CurrLine.c_str(), what, abord))
				{
				    std::cout << what[0] << '\n'; // whole match

					CurrOp.m0 = FileCurrTranId[fileIndex];
					CurrOp.m1 = ABORT;
					CurrOp.m2 = FileCurrMode[fileIndex];

					Transaction.push_back(CurrOp);
					FileCurrReadLine[fileIndex]++;
					linesProcessed++;
				}
				
				else if(regex_search(CurrLine.c_str(), what, fileop1))
				{
				    std::cout << what[0] << '\n'; // whole match

					//operation:  TRAN_ID, OP_TYPE, MODE, FILE_NAME, RECORD_ID, NAME, PHONE

					CurrOp.m0 = FileCurrTranId[fileIndex];
					
					if(what[op].compare("R") == 0)
					   CurrOp.m1 = READ;
					else if(what[op].compare("W") == 0)
					   CurrOp.m1 = WRITE;
					else if(what[op].compare("D") == 0)
					   CurrOp.m1 = DEL;
					  
                    CurrOp.m2 = FileCurrMode[fileIndex];

					CurrOp.m3 = what[filename].str();

					CurrOp.m4 = atoi(what[id].str().c_str());

					CurrOp.m5 = what[name].str();

					CurrOp.m6 = what[phone].str();
              
					Transaction.push_back(CurrOp);

					FileCurrReadLine[fileIndex]++;
					linesProcessed++;
				}
				
				else if(regex_search(CurrLine.c_str(), what, fileop2))
				{
				    std::cout << what[0] << '\n';  // whole match

					//operation:  TRAN_ID, OP_TYPE, MODE, FILE_NAME, RECORD_ID, NAME, PHONE

					CurrOp.m0 = FileCurrTranId[fileIndex];
				
					if(what[op].compare("R") == 0)
					   CurrOp.m1 = READ;
					else if(what[op].compare("W") == 0)
					   CurrOp.m1 = WRITE;
					else if(what[op].compare("D") == 0)
					   CurrOp.m1 = DEL;
				
                    CurrOp.m2 = FileCurrMode[fileIndex];

					CurrOp.m3 = what[filename].str();

					CurrOp.m4 = atoi(what[id].str().c_str());
              
					Transaction.push_back(CurrOp);

					FileCurrReadLine[fileIndex]++;
					linesProcessed++;

				}

				else if(regex_search(CurrLine.c_str(), what, fileop3))
				{
				    std::cout << what[0] << '\n';  // whole match

					//operation:  TRAN_ID, OP_TYPE, MODE, FILE_NAME, RECORD_ID, NAME, PHONE

					CurrOp.m0 = FileCurrTranId[fileIndex];
				
					if(what[op].compare("R") == 0)
					   CurrOp.m1 = READ;
					else if(what[op].compare("W") == 0)
					   CurrOp.m1 = WRITE;
					else if(what[op].compare("D") == 0)
					   CurrOp.m1 = DEL;
				
                    CurrOp.m2 = FileCurrMode[fileIndex];

					CurrOp.m3 = what[filename].str();
              
					Transaction.push_back(CurrOp);

					FileCurrReadLine[fileIndex]++;
					linesProcessed++;

				}
				else
				    FileCurrReadLine[fileIndex]++;

			}

		}
	//	CurrLinePos++;

		for(fileIndex=0; fileIndex<fileNum; fileIndex++)
	    {
		   if(FileCurrReadLine[fileIndex] == FileLineNum[fileIndex] || FileCurrReadLine[fileIndex] == 0)
		     fileProcessed++;
	    }

		if(fileProcessed == fileNum)
			break;
	}

	cout <<"total number of input operations: " << linesProcessed << "\n";
	cout <<"total number of transactions: " << TranIdGlobal << "\n";
}

TranManager::TranManager(fileList files, int lines)
{
	vector<vector<string>>::iterator fileIter;
	vector<string>::iterator lineIter;
	vector<string> CurrFile;
	string CurrLine;
	unsigned int CurrLinePos = 0;

	//variables used to hold parsed strings
	int fileIndex=0;
	int lineIndex;
	int fileNum = files.size();
	//vector<int> FileCurrMode, FileCurrTranId;
    int FileCurrMode[10], FileCurrTranId[10];
	int TranIdGlobal = 0;

	//vector<int> FileLineNum, FileLineReadNum;
	int FileLineNum[10], FileLineReadNum[10];
	//initialize the file properties(line numbers, current red line number, etc.)
	for(fileIndex=0; fileIndex<fileNum; fileIndex++)
	{
		CurrFile = files[fileIndex];
		FileLineNum[fileIndex] = CurrFile.size();
		FileLineReadNum[fileIndex] = 0;
	}
		

//	OpType CurrOpType;
	string  CurrFileName, CurrName;
	Operation CurrOp;


	mark_tag op(1),opbegin(2), opabord(3), opcommit(4),
		mode(5), filename(6), id(7), name(8), phone(9),delim(10);

	cregex begin = (opbegin = 'B')
		       >> (delim= *_s)
		       >> (mode = (boost::xpressive::set= '0','1'));

	cregex commit = (opcommit = 'C')
		       >> (delim= *_s)
			   >> (_n | eos);

    cregex abord = (opabord = 'A')
		       >> (delim= *_s)
			   >> (_n | eos);

	//cregex phone = +_d >> '-'>> +_d>> '-'>> +_d;
	
	cregex fileop1 = (op = (boost::xpressive::set= 'R','W','D'))
		       >> (delim= *_s)
		       >> (filename= +(_w | '.'))
			   >> (delim= *_s) >> '('>> (delim= *_s)
			   >> (id= +_d) >>(delim= *_s) >> ','
			   >> (name= +_w) >>(delim= *_s) >> ','
			   >> (phone= +(_d | '-'))
			   >> (delim= *_s) >> ')';
    
	cregex fileop2 = (op = (boost::xpressive::set= 'R','W','D'))
		       >> (delim= +_s)
		       >> (filename= +(_w | '.'))
			   >> (delim= +_s) 
			   >> (id= +_d); 

	cregex fileop3 = (op = 'D')
		       >> (delim= +_s)
		       >> (filename= +(_w | '.'));
			   
    /*
	cregex date = (month = repeat<1>(_d))           // find the month ...
               >> (delim= (set= '/','-'))            // followed by a delimiter ...
               >> (day=   repeat<1,2>(_d)) >> delim  // and a day followed by the same delimiter ...
               >> (year=  repeat<1,2>(_d >> _d));    // and the year.
    */
    cmatch what;
	int fileProcessed;
	int oldLine;

	while(1) 
	{
		fileProcessed = 0;
		for(int i=0; i<fileNum; i++)
		{
			if(FileLineReadNum[i] == FileLineNum[i])
			   fileProcessed++;
		}

		if(fileProcessed == fileNum)
			break;
        
		while(1)
		{
		   fileIndex = rand() % fileNum;
		   cout << "reading file: " << fileIndex <<"\n";
		   if(FileLineReadNum[fileIndex] < FileLineNum[fileIndex])
			   break;
		}

		CurrFile = files[fileIndex];
		oldLine = FileLineReadNum[fileIndex]; 
		FileLineReadNum[fileIndex] += lines;

		if(FileLineReadNum[fileIndex] >= FileLineNum[fileIndex])
		  FileLineReadNum[fileIndex] = FileLineNum[fileIndex];

		for(lineIndex=oldLine; lineIndex < FileLineReadNum[fileIndex]; lineIndex++)
		{
			CurrLine = CurrFile.at(lineIndex);
		    // mark_tag day(1), month(2), year(3), delim(4);

            // this regex finds a date
			
			if(regex_search(CurrLine.c_str(), what, begin))
		    {
		        std::cout << what[0] << '\n'; // whole match
				FileCurrMode[fileIndex] = atoi(what[mode].str().c_str());
			
				TranIdGlobal++;
				FileCurrTranId[fileIndex] = TranIdGlobal;

				CurrOp.m0 = FileCurrTranId[fileIndex];
				CurrOp.m1 = BEGIN;
				CurrOp.m2 = FileCurrMode[fileIndex];
				Transaction.push_back(CurrOp);			
		    }
			
			else if(regex_search(CurrLine.c_str(), what, commit))
			{
				std::cout << what[0] << '\n'; // whole match

				CurrOp.m0 = FileCurrTranId[fileIndex];
				CurrOp.m1 = COMMIT;
				CurrOp.m2 = FileCurrMode[fileIndex];

				Transaction.push_back(CurrOp);
			}
			
			else if(regex_search(CurrLine.c_str(), what, abord))
			{
			    std::cout << what[0] << '\n'; // whole match

				CurrOp.m0 = FileCurrTranId[fileIndex];
				CurrOp.m1 = ABORT;
				CurrOp.m2 = FileCurrMode[fileIndex];

				Transaction.push_back(CurrOp);
			}
			
			else if(regex_search(CurrLine.c_str(), what, fileop1))
			{
			    std::cout << what[0] << '\n'; // whole match

				//operation:  TRAN_ID, OP_TYPE, MODE, FILE_NAME, RECORD_ID, NAME, PHONE

				CurrOp.m0 = FileCurrTranId[fileIndex];
				
				if(what[op].compare("R") == 0)
				   CurrOp.m1 = READ;
				else if(what[op].compare("W") == 0)
				   CurrOp.m1 = WRITE;
				else if(what[op].compare("D") == 0)
				   CurrOp.m1 = DEL;
				  
                CurrOp.m2 = FileCurrMode[fileIndex];

				CurrOp.m3 = what[filename].str();

				CurrOp.m4 = atoi(what[id].str().c_str());

				CurrOp.m5 = what[name].str();

				CurrOp.m6 = what[phone].str();
          
				Transaction.push_back(CurrOp);
			}
			
			else if(regex_search(CurrLine.c_str(), what, fileop2))
			{
			    std::cout << what[0] << '\n';  // whole match

				//operation:  TRAN_ID, OP_TYPE, MODE, FILE_NAME, RECORD_ID, NAME, PHONE

				CurrOp.m0 = FileCurrTranId[fileIndex];
			
				if(what[op].compare("R") == 0)
				   CurrOp.m1 = READ;
				else if(what[op].compare("W") == 0)
				   CurrOp.m1 = WRITE;
				else if(what[op].compare("D") == 0)
				   CurrOp.m1 = DEL;
			
                CurrOp.m2 = FileCurrMode[fileIndex];

				CurrOp.m3 = what[filename].str();

				CurrOp.m4 = atoi(what[id].str().c_str());
          
				Transaction.push_back(CurrOp);

			}

			else if(regex_search(CurrLine.c_str(), what, fileop3))
				{
				    std::cout << what[0] << '\n';  // whole match

					//operation:  TRAN_ID, OP_TYPE, MODE, FILE_NAME, RECORD_ID, NAME, PHONE

					CurrOp.m0 = FileCurrTranId[fileIndex];
				
					if(what[op].compare("R") == 0)
					   CurrOp.m1 = READ;
					else if(what[op].compare("W") == 0)
					   CurrOp.m1 = WRITE;
					else if(what[op].compare("D") == 0)
					   CurrOp.m1 = DEL;
				
                    CurrOp.m2 = FileCurrMode[fileIndex];

					CurrOp.m3 = what[filename].str();
              
					Transaction.push_back(CurrOp);
				}
		}

	}   
}


std::vector<TranManager::Operation> TranManager::getTrans(void)
{
	return Transaction;
}

TranManager::~TranManager(void)
{
}
