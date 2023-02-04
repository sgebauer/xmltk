
/************************************************
*                                               *
*  XAGG - XML AGGREGATION TOOL : xagg.cpp       *
*                                               *
*  Written by Demian Raven 3/2002               *
*   under support of Dan Suciu                  *
*  demiart@hotmail.com                          *
*                                               *
************************************************/

#define SPACE_TEXT
//#define AGG_OUTPUT

#include "xmltk.h" 
#include "xds.hpp"
 
#ifndef AGG_OUTPUT
#define REG_OUTPUT
#endif

Variable * g_pvRoot = NULL;	//pointer to the root context variable
CONVEC * g_pconvec = NULL;	//pointer to a context and aggregate data-holding vector
Variable * pvCurCon = NULL;
bool g_bBinary = false;
bool bModifyFlag = false;
bool bstartDoc = true;

class CAggHandler : public IFilteredTSAXHandler
{
public:
	// *** IUnknownCL Methods ***
	CL_STDMETHOD(QueryInterface) (RCLIID riid, void ** ppvObj);
	CL_STDMETHOD_(ULONG,AddRef) ();
	CL_STDMETHOD_(ULONG,Release) ();

	// *** ITSAXContentHandler methods ***
	CL_STDMETHOD(startDocument) ();
	CL_STDMETHOD(endDocument) ();
	CL_STDMETHOD(startElement) (XTOKEN xtName);
	CL_STDMETHOD(endElement) (XTOKEN xtName);
	CL_STDMETHOD(attribute) (XTOKEN xtName, char * pszChars, int cchChars);
	CL_STDMETHOD(characters) (char * pszChars, int cchChars);
	CL_STDMETHOD(cdata) (char * pszChars, int cchChars);
	CL_STDMETHOD(extendedint) (XTOKEN xtName, int iInt);
	
	// *** IFilteredTSAXHandler methods ***
	CL_STDMETHOD(startContext) (Variable * pv);
	CL_STDMETHOD(endContext) (Variable * pv);

	CAggHandler(ITSAXContentHandler * pchOut):m_pvContext(NULL),m_pvElement(NULL),m_cRef(1)
	{
		m_pchCur = pchOut;
		m_pchOut = pchOut;
		m_pchCur->AddRef();
		m_pchOut->AddRef();
	}
	virtual ~CAggHandler()
	{
		ATOMICRELEASE(m_pchOut);
	}

private:
	Variable * m_pvContext;
	Variable * m_pvElement;
	ITSAXContentHandler * m_pchOut;
	ITSAXContentHandler * m_pchCur;
	ULONG m_cRef;
};

bool CAggHandler::QueryInterface(RCLIID riid, void ** ppvObj)
{
	if (	IsEqualCLIID(riid, &IID_IUnknownCL) ||
		IsEqualCLIID(riid, &IID_ITSAXContentHandler) ||
		IsEqualCLIID(riid, &IID_IFilteredTSAXHandler))
	{
		*ppvObj = static_cast<IFilteredTSAXHandler *>(this);
	}
	else
	{
		*ppvObj = NULL;
		return false;
	}

	AddRef();
	return true;
}

ULONG CAggHandler::AddRef()
{
	return ++m_cRef;
}

ULONG CAggHandler::Release()
{
	if (--m_cRef > 0)
	{
		return m_cRef;
	}
	delete this;
	return 0;
}

// *** ITSAXContentHandler methods ***
bool CAggHandler::startDocument()
{
	return true;
}

bool CAggHandler::endDocument()
{
	char * temp = g_pconvec->EndDocument();
	m_pchCur->characters(temp, strlen(temp));
	return true;
}

bool CAggHandler::startElement(XTOKEN xtName)
{
	g_idepth++;
	return true;
}

bool CAggHandler::endElement(XTOKEN xtName)
{
	g_idepth--;
	return true;
}

bool CAggHandler::attribute(XTOKEN xtName, char * pszChars, int cchChars)
{
	return true;
}

bool CAggHandler::characters(char * pszChars, int cchChars)
{
	if (bModifyFlag == true)
	{
		g_pconvec->ModifyData(m_pvElement, pszChars, cchChars);
	}
	return true;
}

bool CAggHandler::cdata(char * pszChars, int cchChars)
{
	return true;
}

bool CAggHandler::extendedint(XTOKEN xt, int iInt)
{
	return true;
}

// *** IFilteredTSAXHandler methods ***
bool CAggHandler::startContext(Variable * pv)
{	
	switch(GetVariableDepth(pv))
	{
		case 1:
		{
			if (pv != g_pvRoot)
			{
				if (bstartDoc)
				{
					bstartDoc = false;
					char * temp = g_pconvec->StartDocument();
					m_pchCur->characters(temp, strlen(temp));
				}	
				//ALL CONTEXT STUFF HERE!
				char * temp = g_pconvec->StartContext(pv);
				m_pchCur->characters(temp, strlen(temp));
			}
			else 
			{
				//cerr << "Found Context Root Path: " << g_pconvec->GetXPathExpr(pv) << endl;
			}
			m_pvContext = pv;
			break;
		}
		case 2:
		{
			if (g_pconvec->IsContext(pv))
			{
				//cerr << "Found Other Context Path: " << g_pconvec->GetXPathExpr(pv) << endl;
				m_pvContext = pv;
			}
			else
			{
				//ALL AGG STUFF HERE!
				bModifyFlag = true;
				m_pvElement = pv;
			}
			break;
		}
		default: break;
	}

	return true;
}

bool CAggHandler::endContext(Variable * pv)
{
	switch (GetVariableDepth(pv))
	{
		case 1:
		{
			//ALL CONTEXT STUFF HERE!
			char * temp = g_pconvec->Aggregates(pv);
			m_pchCur->characters(temp, strlen(temp));

			temp = g_pconvec->EndContext();
			m_pchCur->characters(temp, strlen(temp));
			break;
		}
		case 2:
		{
			//ALL AGG STUFF HERE!
			bModifyFlag = false;
			break;
		}
		default: break;
	}

	return true;
}


char * g_pszProgName;
char * g_pszCurCon;
char ** g_targv;
char * g_szConName;
IDfaFilter *g_pfilter = NULL;
char * g_pszFileName = NULL;
int g_iContextNextID = 0;

void myparse(char ** parglist);
void usage(const char *);

void _useFile(char * psfile)
{
	g_pszFileName = strdup(psfile);
}

void _useContext(char * pscontext, char * tagname)
{
	// Register the context and get the context variable
	pvCurCon = g_pfilter->RegisterQuery(g_pvRoot, pscontext, false);
	pvCurCon->setlParam(g_iContextNextID++);
	g_pszCurCon = pscontext;
	if (tagname != NULL) g_szConName = tagname;
}

void _useAggregate(char * psaggfunct, char * psaggscalar, char * pscontext, char * tagname)
{
	Variable * pvCurAgg = NULL;	
	ATYPE ifunct;
	STYPE iscalartype;
	int iid = 0;
	char * psid;

	// get the function value - error check
	if (strcmp(psaggfunct, "SUM") == 0 || strcmp(psaggfunct, "sum") == 0) 
		ifunct = SUM;
	else if (strcmp(psaggfunct, "MIN") == 0 || strcmp(psaggfunct, "min") == 0) 
		ifunct = MIN;
	else if (strcmp(psaggfunct, "MAX") == 0 || strcmp(psaggfunct, "max") == 0) 
		ifunct = MAX;
	else if (strcmp(psaggfunct, "AVERAGE") == 0 || strcmp(psaggfunct, "average") == 0 ||
		strcmp(psaggfunct, "AVG") == 0 || strcmp(psaggfunct, "avg") == 0)
		 ifunct = AVERAGE;
	else if (strcmp(psaggfunct, "FIRST") == 0 || strcmp(psaggfunct, "first") == 0) 
		ifunct = FIRST;
	else if (strcmp(psaggfunct, "LAST") == 0 || strcmp(psaggfunct, "last") == 0) 
		ifunct = LAST;
	else if (strcmp(psaggfunct, "COUNT") == 0 || strcmp(psaggfunct, "count") == 0) 
		ifunct = COUNT;
	else if (strncmp(psaggfunct, "ID#", 3) == 0 || strncmp(psaggfunct, "id#", 3) == 0)
	{
		ifunct = CHOICE;
		psid = psaggfunct + 3;
		iid = atoi(psid);
	}
	else if (strncmp(psaggfunct, "CHOICE#", 7) == 0 || strncmp(psaggfunct, "choice#", 7) == 0)
	{
		ifunct = CHOICE;
		psid = psaggfunct + 7;
		iid = atoi(psid);
	}
	else
	{
		cerr << "Illegal Function: " << psaggfunct << endl;
		exit(-1);
	}

	if (strcmp(psaggscalar, "INT") == 0 || strcmp(psaggscalar, "int") == 0 ||
		strcmp(psaggscalar, "INTEGER") == 0 || strcmp(psaggscalar, "integer") == 0)
		iscalartype = INT;
	else if (strcmp(psaggscalar, "FLOAT") == 0 || strcmp(psaggscalar, "float") == 0)
		iscalartype = FLOAT;
	else if (strcmp(psaggscalar, "TEXT") == 0 || strcmp(psaggscalar, "text") == 0 ||
		strcmp(psaggscalar, "CDATA") == 0 || strcmp(psaggscalar, "cdata") == 0)
		iscalartype = TEXT;
	else if (strcmp(psaggscalar, "DEPTH") == 0 || strcmp(psaggscalar, "depth") == 0)
		iscalartype = DEPTH;
	else if (strcmp(psaggscalar, "LENGTH") == 0 || strcmp(psaggscalar, "length") == 0)
		iscalartype = LENGTH;
	else
	{
		cerr << "Illegal Scalar Type: " << psaggscalar << endl;
		exit(-1);
	}
	
	if (g_iContextNextID == 0) pvCurCon = g_pvRoot;
	pvCurAgg = g_pfilter->RegisterQuery(pvCurCon, pscontext, true);
	pvCurAgg->setlParam(g_iContextNextID++);

	// Insert the aggregate context variable and function value into the vector

	g_pconvec->InsertAgg(pvCurCon, pvCurAgg, ifunct, iscalartype, iid, pscontext, g_pszCurCon); 
	if (g_szConName != NULL) g_pconvec->SetConName(g_szConName);
	if (tagname != NULL) g_pconvec->SetAggName(tagname);
} 

void _Aggregate()
{
	// Create a file stream for the file
	IFileStream *pstm = _CreateFileStream(g_pszFileName);

	if (pstm)
	{
		
		//get a content handler - output to stream
		// (currently binary false)
		if (g_bBinary == false)
			pchOut = CreateStdoutStream(false);
		else pchOut = CreateStdoutStream(true);		

		if (pchOut)
		{
			//HERE: Create an aggregator object
			CAggHandler *pagg = new CAggHandler(pchOut);
			if (pagg)
			{
				//parse it and output!
				//set the handler with the DFAFilter
				IUnknownCL_SetHandler(g_pfilter, pagg);

				//Whoopie! Now parse it!
				ParseUnknownStream(pstm, g_pfilter);

			}
			//release the sax content handler/ output stream
			pchOut->Release();
			
			//release the handler
			if (pagg) pagg->Release();

		}
		//release the file stream handle
		pstm->Release();
	}
	//That's it!
}


int main(int argc, char * argv[])
{
	g_pconvec = new CONVEC();
	InitGlobalTokenTable();
	bModifyFlag = false;

	// attempt to create a dfa filter
	if (CreateDfaFilter(&IID_IDfaFilter, (void**)&g_pfilter))
	{
		// get the name of the program
		g_pszProgName = *argv;	
		// get the remainder of the arg list
		g_targv = argv + 1;
		
		try
		{
			// register the root and get a pointer to its variable.
			g_pvRoot = g_pfilter->RegisterQuery(NULL, "/", true);

			// myparse drives everything via callbacks to the "_use..." functions
			if (*g_targv == NULL)
				usage(NULL);
			else
				myparse(g_targv);

			// now we run the aggregation
			_Aggregate();

			//DEBUG
//			g_pconvec->PrintDS_Cerr();

			//release the filter
			ATOMICRELEASE(g_pfilter);
		}
		catch (_Error &err)
		{
			// handle errors
			err.perror();
		}

		// free pointers
		if (g_pszFileName) free(g_pszFileName);
	}
	
	CleanupGlobalTokenTable();
	delete g_pconvec;
}

void usage(const char * pserr)
{
	if (pserr) cerr << "\nError: " << pserr << endl;
	cerr << "\nUsage: " << g_pszProgName 
		<< " [-b]?"
		<< " [FILE] " 
		<< "[ ( -f | -file) filename ] [ [ ( -c | -context ) xpath-expr ] "
		<< "[ ( -a | -aggregate ) AGG SCALAR xpath-expr ] ]* " 
		<< "\n\n\tWhere AGG = \n\t\t( ( sum | SUM ) \n\t\t| ( max | MAX ) \n\t\t| ( min | MIN ) \n\t\t| ( average | AVERAGE ) "
		<< "\n\t\t| ( first | FIRST ) \n\t\t| ( last | LAST ) \n\t\t| ( count | COUNT ) \n\t\t| ( depth | DEPTH ) "
		<< "\n\t\t| ( length | LENGTH )"
		<< "\n\t\t| ( choice#[INTEGER] | CHOICE#[INTEGER] | id#[INTEGER] | ID#[INTEGER] ) )"
		<< "\n\n\tWhere SCALAR = \n\t\t( ( int | integer | INT | INTEGER ) \n\t\t| ( float | FLOAT ) "
		<< "\n\t\t| ( text | cdata | TEXT | CDATA ) \n\t\t| ( depth | DEPTH ) )\n" << endl;  
	exit(-1);
}

char ** file2argc(char * filename);

void myparse(char ** parglist)
{
	char ** myargs = parglist;
	bool firsttime = true;
	char * conpath = NULL;
	char * conname = NULL;
	char * aggpath = NULL;
	char * aggname = NULL;
	char * aggscalar = NULL;
	char * aggtype = NULL;

	if (myargs[0][0] == '-' && myargs[0][1] == 'h')
	{
		usage(NULL);
	}		
	
	if (myargs[0][0] != '-')
	{
		_useFile(*myargs);
		myargs = myargs + 1;
	}
	
	if (myargs[0][0] == '-' && myargs[0][1] == 'b')
	{
		g_bBinary = true;
		myargs = myargs + 1;
		cerr << "Binary Output Detected\n";
	}

	// if there are no contexts then give usage
	if (*myargs == NULL) 
		usage("You must include an aggregate or context.");
	else if (strcmp(*myargs, "-f") == 0 || strcmp(*myargs, "-file") == 0)
	{
		myargs = myargs + 1;
		if (*myargs == NULL)
			usage("You must supply a filename containing an argument list");
		else 
			myargs = file2argc(*myargs);
	}

	if (strcmp(*myargs, "-t") == 0 || strcmp(*myargs, "-tag") == 0)
	{
		myargs = myargs + 1;
		if (myargs[0][0] == '-' || *myargs == NULL)
			usage("You must include an output tag name");
		else
		{
			g_outname = *myargs;
			myargs = myargs + 1;
		}
	}	

	// while there is still an option to manage
	while (*myargs != NULL)
	{
		// check for context option
		if (strcmp(*myargs, "-c") == 0 || strcmp(*myargs, "-context") == 0)
		{
			//then context!
			myargs = myargs + 1;
			if (*myargs == NULL || **myargs == '-') 
				usage("You must include a contextual xpath expression.");
			conpath = *myargs;
			myargs = myargs + 1;
			conname = "context";
			if (*myargs != NULL)
			{
				if (strcmp(*myargs, "-t") == 0 || strcmp(*myargs, "-tag") == 0)
				{
					myargs = myargs + 1;
					//check for a tag name
					if (*myargs == NULL || **myargs =='-')
						usage("The tagname option requires an element tag name");
					conname = *myargs;
					myargs = myargs + 1;
				}
			}
			_useContext(conpath, conname);
			conname = NULL;
			if (*myargs == NULL) 
				usage("You must include an aggregate for the context.");
			if (strcmp(*myargs, "-a") == 0 || strcmp(*myargs, "-aggregate") == 0)
	                {
        	                myargs = myargs + 1;
                	        // check for a function, a scalar type, and a aggregate context
                	        if (*myargs == NULL || *(myargs + 1) == NULL || *(myargs + 2) == NULL ||
					**myargs == '-' || myargs[1][0] == '-' || myargs[2][0] == '-') 
					usage("The aggregate option requires a function and an xpath expression");
                		else
				{
					aggpath = *myargs;
					aggtype = *(myargs + 1);
					aggscalar = *(myargs + 2);
					myargs = myargs + 3;
					aggname = "aggregate";
				}
	 	                if (*myargs != NULL)
				{
					if (strcmp(*myargs, "-t") == 0 || strcmp(*myargs, "-tag") == 0)
                        		{
                                		myargs = myargs + 1;
                                		//check for a tag name
                                		if (*myargs == NULL || **myargs =='-')
                                		        usage("The tagname option requires an element tag name");
                                		else
						{
							aggname = *myargs;
                                			myargs = myargs + 1;
						}
                        		}
				}
			        _useAggregate(aggpath, aggtype, aggscalar, aggname);
				aggname = NULL;
                	}
			else usage("You must include an aggregate for the context.");
			firsttime = false;
		}
		// check for aggregate option
		else if (strcmp(*myargs, "-a") == 0 || strcmp(*myargs, "-aggregate") == 0)
		{
			if (firsttime)
			{
				_useContext("/",NULL);
				firsttime = false;
			}
                        myargs = myargs + 1;
                        // check for a function, a scalar type, and a aggregate context
                        if (*myargs == NULL || *(myargs + 1) == NULL || *(myargs + 2) == NULL ||
                    		**myargs == '-' || myargs[1][0] == '-' || myargs[2][0] == '-')
                         	usage("The aggregate option requires a function and an xpath expression");
                 	else
                   	{
                     		aggpath = *myargs;
                    		aggtype = *(myargs + 1);
                    		aggscalar = *(myargs + 2);
                    		myargs = myargs + 3;
				aggname = "aggregate";
                 	}
                 	if (*myargs != NULL)
			{
				if (strcmp(*myargs, "-t") == 0 || strcmp(*myargs, "-tag") == 0)
                    		{
                     			myargs = myargs + 1;
                    			//check for a tag name
                      			if (*myargs == NULL || **myargs =='-')
                                     		usage("The tagname option requires an element tag name");
                           		else
					{
						aggname = *myargs;
                      				myargs = myargs + 1;
					}
        			}
	             	}
                      	_useAggregate(aggpath, aggtype, aggscalar, aggname);
                      	aggname = NULL;
		}
		else if (!(strcmp(*myargs, "-h") && strcmp(*myargs, "-help"))) usage(NULL);
		else usage("Improper syntax.");
	}
}

char ** file2argc(char * filename)
{
        int count = 0;
        int total = 0;
        char * buffer = new char[100];
        char ** retarr;
        char bchar;
        vector<char *> vchstr;
        ifstream MYFILE(filename);
        if(! (MYFILE.is_open()))
        {
                cerr << "\nUnable to open file." << endl;
                exit(1);
        }
        while(!MYFILE.eof())
        {
		MYFILE.getline(buffer,99,' ');
//		cerr << buffer << endl;
		vchstr.push_back(strdup(buffer));
		total++;
        }
        retarr = (char **) new char*[total + 1];
        count = 0;
        for (vector<char *>::iterator iter = vchstr.begin();iter < vchstr.end(); iter++)
        {
		retarr[count] = (*iter);
//		cerr << *iter << endl;
                count++;
        }
	retarr[count - 1][strlen(retarr[count - 1]) - 1] = '\0';
	retarr[count] = NULL;
        MYFILE.close();
        return retarr;
}
