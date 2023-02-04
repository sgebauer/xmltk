/*********************************************
*                                            *
*   XAGG - XML AGGREGATION TOOL - xds.hpp    *
*    essential xagg data structures          *
*                                            *
*  Written by Demian Raven 3/2002            *
*   under support of Dan Suciu               *
*  demiart@hotmail.com                       *
*                                            *
*********************************************/

//for output
ITSAXContentHandler * pchOut;

//global depth variable
int g_idepth = -1;
char * g_outname = "xagg";

enum ATYPE {SUM,MIN,MAX,FIRST,LAST,CHOICE,COUNT,AVERAGE};
enum STYPE {TEXT,INT,FLOAT,DEPTH,LENGTH};


class AGGDS
{ 
public:
    	AGGDS(){AGGDS(NULL,SUM,INT,"","/");}
	AGGDS(Variable * pvContext, ATYPE aType,STYPE sType, char * xpExpr, char * xpConExpr):pvMyAgg(pvContext),iAggType(aType),iScalarType(sType),xpathExpr(xpExpr), xpathConExpr(xpConExpr)
	{
		switch(sType)
		{case INT:{iIntData = 0;break;}
		case FLOAT:{fFloatData = 0.0;break;}
		case TEXT:{break;}
		case DEPTH:{iDepthData = 0; break;}
		default: break;}
		counter = 0;
		pcStrData = NULL;
		iStrDataTotal = 0;
		vecStrData.resize(0);
		aggname = "aggregate";
		conname = "context";
	}
	~AGGDS(){;}
	void Clear()
	{
		pvMyAgg = NULL;
		vecStrData.empty();
	}
	void PrintDS_Cerr(void);
	bool ModifyData(char * pcData, int iDataLen);
	void SetChoiceID(int iData){iChoiceID=iData;return;}
	bool CompareAgg(Variable * pvInput)
	{bool result = (pvInput->getlParam() == pvMyAgg->getlParam()) ? true : false; return result;}
	char * GetResult();
	char * AggString();
	char * GetXPathExpr(); 
	char * GetXPathContextExpr();
	char * GetAggName();
	char * GetConName();
	void SetConName(char * sin);
	void SetAggName(char * sin);

private:
	bool _ModifyTEXT(char * pcData, int iDataLen);
	bool _ModifyINT(char * pcData);
	bool _ModifyFLOAT(char * pcData);
	bool _ModifyDEPTH(void);
	bool _ModifyLENGTH(char * pcData, int iDataLen);
	void _PutInVec(char * pcData, int iDataLen, int flag);
	void ClearData(void);

    	Variable *      pvMyAgg;		//Pointer to the Aggregate Variable
    	ATYPE           iAggType;		//Which type of aggregate
    	STYPE           iScalarType; 		//Which type of scalar
    	unsigned int	iIntData;		//Scalar INT data
    	float           fFloatData;		//Scalar FLOAT data
	unsigned int  	iDepthData;		//Scalar DEPTH data
	vector<char*>	vecStrData;		//all TEXT data
	int		iStrDataTotal;		//counter for TEXT data
	char *		pcStrData;		//The return string
	int 		iChoiceID;		//In case of AGG type CHOICE or ID
	int 		counter;		//Modification counter
	char * 		xpathExpr;		//The aggregate's context
	char * 		xpathConExpr;		//The overall context
	char * 		aggname;		//User option of renaming aggregate
	char * 		conname;		//User option of renaming context
};

// Epties data contents when output
void AGGDS::ClearData(void)
{
	iIntData = 0;
	fFloatData = 0;
	vecStrData.empty();
	iStrDataTotal = 0;
	counter = 0;
}

// Returns the user-defined aggregate name
char * AGGDS::GetAggName(void)
{
	return aggname;
}

// Returns the user-defined context name
char * AGGDS::GetConName(void)
{
	return conname;
}

// Sets the user-defined context name
void AGGDS::SetConName(char * sin)
{
	conname = sin;
}

// Sets the user-defined aggregate name
void AGGDS::SetAggName(char * sin)
{
	aggname = sin;
}

// Adds new data to the data structure (general)
bool AGGDS::ModifyData(char * pcData, int iDataLen)
{
	bool bresult = false;
	counter++;

	switch (iScalarType)
	{
		case TEXT:	
			{bresult = _ModifyTEXT(pcData, iDataLen); break;}
		case INT:	
			{bresult = _ModifyINT(pcData); break;}
		case FLOAT:	
			{bresult = _ModifyFLOAT(pcData); break;}
		case DEPTH:
			{bresult = _ModifyDEPTH(); break;}
		case LENGTH:
			{bresult = _ModifyLENGTH(pcData, iDataLen); break;}
		default:	
		{
			break;
			return false;
		}//this is an error condition
	}
	return bresult;
}

// Adds character data to the data structure's vector
void AGGDS::_PutInVec(char * pcData, int iDataLen, int flag)
{
	char * ptemp = new char[iDataLen + 1];
	strncpy(ptemp, pcData, iDataLen);
	ptemp[iDataLen] = '\0';
	if (flag == 0) 
	{
		vecStrData.push_back(ptemp);
		iStrDataTotal += iDataLen;
	}
	else
	{
		vecStrData.clear();
		vecStrData.push_back(ptemp);
		iStrDataTotal = iDataLen;
	}
}

// Adds character data
bool AGGDS::_ModifyTEXT(char * pcData, int iDataLen)
{
	switch (iAggType)
    	{
        	//MIN, MAX, AVERAGE not defined
		case SUM:	
		{	
			_PutInVec(pcData, iDataLen, 0);
			break;
		}
		case FIRST:	
		{	
			if (counter == 1)
			{	
				_PutInVec(pcData, iDataLen, 0);
			}
			break;
		}
		case LAST:
		{	
			_PutInVec(pcData, iDataLen, 1);
			break;
		}
		case CHOICE:
		{	
			if (counter == iChoiceID)
			{	
				_PutInVec(pcData, iDataLen, 0);
			}
			break;
		}
		default: 
		{
			return false;
			break;
		}
	}
	return true;
}


// Adds integral data
bool AGGDS::_ModifyINT(char * pcData)
{
	char * pc;
	int iData = strtoul(pcData,&pc,0);
	switch (iAggType)
	{
		case SUM:
		case AVERAGE:	
			{iIntData += iData; break;}
		case MIN:		
			{if (counter <= 1) iIntData = iData;
			else  iIntData = (iData < iIntData) ? iData : iIntData; 
			break;}
		case MAX:		
			{iIntData = (iData > iIntData) ? iData : iIntData; break;}
		case FIRST:		
			{if (counter == 1) iIntData = iData; break;}
		case LAST:		
			{iIntData = iData; break;}
		case CHOICE:	
			{if (counter == iChoiceID) iIntData = iData; break;}
		default:
			{return false;
			break;}
	}  
	return true;
}

//adds length data
bool AGGDS::_ModifyLENGTH(char * pcdata, int iDataLen)
{
	int i = strlen(pcdata);
	switch (iAggType)
	{
		case SUM:
		case AVERAGE:
			{iIntData += i; break;}
		case MIN:
			{if (counter <= 1) iIntData = i;
			else iIntData = (i < iIntData) ? i : iIntData;
			break;}
		case MAX:
			{iIntData = (iIntData > i) ? iIntData : i;
			break;}
		case FIRST:
			{if (counter <= 1) iIntData = i; break;}
		case LAST:
			{iIntData = i; break;}
		case CHOICE:
			{if (counter == iChoiceID) iIntData = i; break;}
		default:
			{return false; break;}
	}
	return true;
}
// Adds depth data
bool AGGDS::_ModifyDEPTH(void)
{
        switch (iAggType)
        {
                case SUM:
                case AVERAGE:
                        {iDepthData += g_idepth; break;}
                case MIN:
                        {if (counter <= 1) iDepthData = g_idepth;
                        else  iDepthData = (g_idepth < iDepthData) ? g_idepth : iDepthData;
                        break;}
                case MAX:
                        {iDepthData = (g_idepth > iDepthData) ? g_idepth : iDepthData; break;}
                case FIRST:
                        {if (counter == 1) iDepthData = g_idepth; break;}
                case LAST:
                        {iDepthData = g_idepth; break;}
                case CHOICE:
                        {if (counter == iChoiceID) iDepthData = g_idepth; break;}
                default:
			{return false;
			break;}
	}
	return true;
}

// Adds float data
bool AGGDS::_ModifyFLOAT(char * pcData)
{
	float fData = (float)atof(pcData);
	switch (iAggType)
	{
		case SUM:
		case AVERAGE:		
			{fFloatData += fData;	break;}
		case MIN:		
			{if (counter <= 1) fFloatData = fData;
			else fFloatData = (fData < fFloatData) ? fData : fFloatData; break;}
		case MAX:		
			{fFloatData = (fData > fFloatData) ? fData : fFloatData; break;}
		case FIRST:		
			{if (counter == 1) fFloatData = fData; break;}
		case LAST:		
			{fFloatData = fData; break;}
		case CHOICE:	
			{if (counter == iChoiceID) fFloatData = fData; break;}
		default:
			{return false;
			break;}
	}
	return true;
}

// Gets a string corresponding to the aggregate type
char * AGGDS::AggString(void)
{
	char * retstr = "";
	switch (iAggType)
	{
		case SUM: retstr = "sum"; break;
		case AVERAGE: retstr = "average"; break;
		case MIN: retstr = "min"; break;
		case MAX: retstr = "max"; break;
		case FIRST: retstr = "first"; break;
		case LAST: retstr = "last"; break;
		case COUNT: retstr = "count"; break;
		case CHOICE:
		{
			retstr = new char[20];
			char * temploc = retstr;
			strcpy(temploc, "id#");
			temploc += 3;
			sprintf(temploc,"%d",iChoiceID);
			break;
		}
		default: break;
	}
	return retstr;
}

// Gets the xpath expression corresponding to the aggregate
char * AGGDS::GetXPathExpr(void)
{
	return xpathExpr;
}

// Gets the xpath expression corresponding to the context
char * AGGDS::GetXPathContextExpr(void)
{
	return xpathConExpr;
}

// Returns the final data as a character string
char * AGGDS::GetResult(void)
{
	switch (iScalarType)
	{
		case TEXT:	
		{
			delete pcStrData;
			if (iAggType == COUNT)
			{
				pcStrData = new char[100];
				sprintf(pcStrData, "%d", counter);
			}
			else
			{
#ifdef SPACE_TEXT
				if (iAggType != CHOICE)
				{
					pcStrData = new char[iStrDataTotal + counter + 1];
				}
				else pcStrData = new char[iStrDataTotal + 1];
#else
				pcStrData = new char[iStrDataTotal + 1];
#endif
				char * pcInsert = pcStrData;
				int length;
				for(vector<char *>::iterator iter = vecStrData.begin();iter < vecStrData.end(); iter++)
				{
					length = strlen(*iter);
					strncpy(pcInsert,(*iter),length);
					pcInsert += length;
#ifdef SPACE_TEXT
					if (iAggType != CHOICE)
					{
						strncpy(pcInsert,"     ",1);
						pcInsert += 1;
					}
#endif
				}
#ifdef SPACE_TEXT
				if (iAggType != CHOICE)
				{
					pcStrData[iStrDataTotal + counter] = '\0';
				}
				else pcStrData[iStrDataTotal] = '\0';
#else
				pcStrData[iStrDataTotal] = '\0';
#endif
			
			}
			ClearData();
			return pcStrData;
			break;
		}
		case INT:
		{
			delete pcStrData;
			pcStrData = new char[100];
			if (iAggType == COUNT) iIntData = counter;
			if (iAggType == AVERAGE) iIntData /= counter;
			sprintf(pcStrData,"%u",iIntData);
			ClearData();
			return pcStrData;
			break;
		}
		case FLOAT:
		{
			delete pcStrData;
			pcStrData = new char[100];
			if (iAggType == COUNT)
			{
				sprintf(pcStrData,"%d",counter);
			}
			else if (iAggType == AVERAGE)
			{
				fFloatData /= (float)counter;
				sprintf(pcStrData,"%f",fFloatData);
			}
			else sprintf(pcStrData,"%f",fFloatData);
			ClearData();
			return pcStrData;
			break;
		}
		case DEPTH:
		{
			delete pcStrData;
			pcStrData = new char[100];
			if (iAggType == COUNT) iDepthData = counter; 
                        if (iAggType == AVERAGE) iDepthData /= counter;
                        sprintf(pcStrData,"%u",iDepthData);
                        ClearData();
                        return pcStrData;
                        break;
		}
		case LENGTH:
		{
			delete pcStrData;
			pcStrData = new char[100];
			if (iAggType == COUNT) iIntData = counter;
			if (iAggType == AVERAGE) iIntData /= counter;
			sprintf(pcStrData, "%u", iIntData);
			ClearData();
			return pcStrData;
			break;
		}
		default: break;
	}
	return NULL;
}
		


class CONVEC
{
public:
	CONVEC(){};
	~CONVEC()
	{
		Clear();
	};
	void Clear(void)
	{
		vector< pair<Variable *, AGGDS *> >::iterator iter;
		iter = vecPrData.end() - 1;
		//NOTE: all variables must be cleaned up AFTER this operation to avoid memory leak
		while (iter >= vecPrData.begin())
		{
			(*iter).first = NULL;
			(*iter).second->Clear();
			iter->second = NULL;
			vecPrData.erase(iter);
			iter = vecPrData.end() - 1;
		}
	}
	bool InsertAgg(Variable * pvContext, Variable * pvAgg, ATYPE aType, STYPE sType, int ID, char * xpExpr, char * xpConExpr);
	bool ModifyData(Variable * pvInput, char * pcData, int iDatalen);
	bool IsContext(Variable * pvInput);
	char * GetResult(Variable * pvInput);
	char * GetXPathExpr(Variable * pvInput);
	char * StartDocument(void);
	char * EndDocument(void);
	char * StartContext(Variable * pv);
	char * EndContext(void);
	char * Aggregates(Variable * pv);
	void SetConName(char * sin);
	void SetAggName(char * sin);
	
private:
	void FocusContext(Variable * pvContext);

	vector< pair<Variable *,AGGDS *> > vecPrData;		//vector of contexts and corresponding data
	pair<Variable *, AGGDS *> * pprCurrent; 		// pointer to the current pair

	char * outStr;						// final output string
};

// Sets the user-defined name of the context
void CONVEC::SetConName(char * sin)
{
	pprCurrent->second->SetConName(sin);
}

//Sets the user-defined name of the aggregate
void CONVEC::SetAggName(char * sin)
{
	pprCurrent->second->SetAggName(sin);
}

// Inserts a new context-aggregate grouping into the data structure
bool CONVEC::InsertAgg(Variable * pvContext, Variable * pvAgg, ATYPE aType, STYPE sType, int ID, char * xpExpr, char * xpConExpr)
{
	pair<Variable *, AGGDS *> * pprtemp = new pair<Variable *, AGGDS *>;
	pprtemp->first = pvContext;
	pprtemp->second = new AGGDS(pvAgg,aType,sType,xpExpr,xpConExpr);
	vecPrData.push_back(*pprtemp);
	pprCurrent = vecPrData.end() - 1;
	pprCurrent->second->SetChoiceID(ID);
	return true;
}

// Is this variable a context or aggregate?
bool CONVEC::IsContext(Variable * pvInput)
{
	bool result = false;
	for (vector< pair<Variable *, AGGDS *> >::iterator iter = vecPrData.begin();iter < vecPrData.end();iter++)
	{
		if (pvInput->getlParam() == iter->first->getlParam()) result = true;
	}
	return result;
}

// Find the correct context pair
void CONVEC::FocusContext(Variable * pvContext)
{
	pprCurrent = vecPrData.begin();
	for (vector< pair<Variable *, AGGDS *> >::iterator iter = vecPrData.begin();iter < vecPrData.end();iter++)
	{
		if (iter->first->getlParam() == pvContext->getlParam())
		{
			pprCurrent = iter;
			goto END;
		}
	}
	END:
	return;
}

// Add data
bool CONVEC::ModifyData(Variable * pvInput, char * pcData, int iDataLen)
{
	for (vector< pair<Variable *, AGGDS *> >::iterator iter = vecPrData.begin();iter < vecPrData.end();iter++)
	{
		pprCurrent = iter;
		if (pprCurrent->second->CompareAgg(pvInput)) break;
	}		
	if (pprCurrent == vecPrData.end()) return false;
	else return pprCurrent->second->ModifyData(pcData, iDataLen);
}

// Get the result as a character string
char * CONVEC::GetResult(Variable * pvInput)
{
	if (!(pprCurrent->first->getlParam() == pvInput->getlParam()))
	{
		FocusContext(pvInput);
	}
	return pprCurrent->second->GetResult();
}


// Get the xpath expression for the proper agg or context
char * CONVEC::GetXPathExpr(Variable * pvInput)
{
	for (vector< pair< Variable *,AGGDS*> >::iterator iter = vecPrData.begin(); iter < vecPrData.end(); iter++)
	{
		if (iter->first->getlParam() == pvInput->getlParam())
		{
			return (iter->second->GetXPathContextExpr());
		}
		else if (iter->second->CompareAgg(pvInput))
		{
			return (iter->second->GetXPathExpr());
		}
	}
	return "";
}


char * CONVEC::StartDocument(void)
{
	char * retstr = new char[2 + strlen(g_outname) + 2];
	char * pretstr = retstr;
	strncpy(pretstr, "\n<", 2);
	pretstr += 2;
	strncpy(pretstr, g_outname, strlen(g_outname));
	pretstr += strlen(g_outname);
	strncpy(pretstr, ">\0", 2);
	return retstr;
}

char * CONVEC::EndDocument(void)
{
	char * retstr = new char[3 + strlen(g_outname) + 4];
	char * pretstr = retstr;
	strncpy(pretstr, "\n</", 3);
	pretstr += 3;
	strncpy(pretstr, g_outname, strlen(g_outname));
	pretstr += strlen(g_outname);
	strncpy(pretstr, ">\n\n\0", 4);
	return retstr;
}

char * CONVEC::StartContext(Variable * pv)
{
	int count = 0;
	char * retstr = "";
	FocusContext(pv);
	char * constr = pprCurrent->second->GetXPathContextExpr();
	vector<char *> vpch;
	vpch.push_back("\n\t<");
	vpch.push_back(pprCurrent->second->GetConName());
	vpch.push_back(" path=\"");
	vpch.push_back(constr);
	vpch.push_back("\">");
	for (vector<char *>::iterator iter = vpch.begin(); iter < vpch.end(); iter++)
	{
		count += strlen(*iter);
	}
	retstr = new char[count + 1];
	char * retstrptr = retstr;
	for (vector<char *>::iterator iter = vpch.begin(); iter < vpch.end(); iter++)
	{
		count = strlen(*iter);
		strncpy(retstrptr, *iter, count);
		retstrptr += count;
	}
	retstrptr[0] = '\0';
	return retstr;
}

char * CONVEC::EndContext(void)
{
	char * temp = "\n\t</";
	char * temp2 = ">";
	int count = (strlen(temp) + strlen(temp2) + strlen(pprCurrent->second->GetConName()) + 1);
	char * retstr = new char[count];
	char * retstrcpy = retstr;
	count = strlen(temp);
	strncpy(retstrcpy, temp, count);
	retstrcpy += count;
	char * conname = pprCurrent->second->GetConName();
	count = strlen(conname);
	strncpy(retstrcpy, conname, count);
	retstrcpy += count;
	count = strlen(temp2);
	strncpy(retstrcpy, temp2, count);
	retstrcpy += count;
	retstrcpy[0] = '\0';
	return retstr;
}

char * CONVEC::Aggregates(Variable * pv)
{
 	int count = 0;
	char * retstr = "";
	char * temp = "";
	vector<char *> vstrs;
	vstrs.empty();
	for (vector< pair<Variable *, AGGDS *> >::iterator iter = vecPrData.begin(); iter < vecPrData.end(); iter++)
	{
		if (iter->first->getlParam() == pv->getlParam())
		{
			vstrs.push_back("\n\t\t<");
			vstrs.push_back(iter->second->GetAggName());
			vstrs.push_back(" aggtype=\"");
			vstrs.push_back(iter->second->AggString());
			vstrs.push_back("\" path=\"");
			vstrs.push_back(iter->second->GetXPathExpr());
			vstrs.push_back("\">");
			vstrs.push_back(iter->second->GetResult());
			vstrs.push_back("\n\t\t</");
			vstrs.push_back(iter->second->GetAggName());
			vstrs.push_back(">");
		}
	}
	for (vector<char *>::iterator iter3 = vstrs.begin(); iter3 < vstrs.end(); iter3++)
	{
		count += strlen(*iter3);
	}
	retstr = new char[count + 1];
	char * retstrptr = retstr;
	for (vector<char *>::iterator iter2 = vstrs.begin(); iter2 < vstrs.end(); iter2++)
	{
		count = strlen(*iter2);
		strncpy(retstrptr, (*iter2), count);
		retstrptr += count;
	}
	retstrptr[0] = '\0';
	return retstr;
}

