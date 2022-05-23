/********************************************************************************************************/
/*				New Global Electronics		BL 2012 Copyright Boost License					*/
/********************************************************************************************************/



#include <string.h>   // include file for memcpy
#include "ngeos.h"

static unsigned int uiNumberOfEvents = 0;

tTask tskTaskArray[MAX_SCH_TASKS]; // create the array containing all the tasks
u16 uiIndexTaskArray;
tTask EmergencyTskArray[];
u08 uSchTimer=0;
u08 uSchTic=0;
/*______________________________________________________________________ 
|                        Elements locaux                                |
|_______________________________________________________________________|*/




/*______________________________________________________________________________________________________// 
//	Procedure	:	GetFromEVENTArray
//______________________________________________________________________________________________________//
//	Role		:	Add an tEvent to  EVENTArray 
//					
//	Inputs		:	A pointer to a given task and  the evnt to add
//					
//	Outputs		:	Status of adding
//
//	Added By	:  	NGE / RBB / LB 
//____________________________________________________________________________________*/
tEvent* AddEventToEventArray(tTask* tTaskToAddEvt, tEvent* evtEventToAdd)
{
	unsigned int uiEventPosition = 0;
	tEvent *pCopyEVENTTo = NULL;
	/* BL 055/05/2015 if the task is SUSPENDED we do nothing: new events are added only if the task is a state different from SUSPENDED*/
	if (tTaskToAddEvt->tskStatus != SUSPENDED)
	{
		while ((_GET_ucEVT(tTaskToAddEvt, tTaskToAddEvt->uTaskWriteIndex) != NO_EVENT) && (uiEventPosition <= tTaskToAddEvt->uEventArrayLength))
		{
			uiEventPosition++;
			tTaskToAddEvt->uTaskWriteIndex = ((++tTaskToAddEvt->uTaskWriteIndex) % tTaskToAddEvt->uEventArrayLength);	  // increment the write index
		}
		if (uiEventPosition < tTaskToAddEvt->uEventArrayLength)
		{
			pCopyEVENTTo = _GET_pEVT(tTaskToAddEvt, tTaskToAddEvt->uTaskWriteIndex);
			//memmove(pCopyEVENTTo,evtEventToAdd,tTaskToAddEvt->uDataTypeLength);
			memcpy(pCopyEVENTTo, evtEventToAdd, tTaskToAddEvt->uDataTypeLength); // insert a new tEvent.
			// We increase the number of Events to be processed by the Scheduler main loop, but only for non periodic events (for which 1ms UpdateTasks() will do the work)
			if (pCopyEVENTTo->lTimer)
			{
				//uiNumberOfEvents++;
				tTaskToAddEvt->uTaskNbPendingEvents++;
			}
		}
	}
    return pCopyEVENTTo;
}


/*______________________________________________________________________________________________________// 
//	Procedure	:	DeleteEVENTFromEVENTArray
//______________________________________________________________________________________________________//
//	Role			:	Delete an tEvent ( abort a timeout ) 
//					
//	Inputs		:	A pointer to the index of the tEvent to be deleted
//					
//	Outputs		:	None
//
//	Added By		:  	NGE / RBB / LB 
//____________________________________________________________________________________*/

void DeleteEVENTFromEVENTArray(tEvent* eEVENTToDelete)
{   	
	eEVENTToDelete->ucEvt = NO_EVENT;
}

/*______________________________________________________________________________________________________// 
//	Procedure	:	InitEVENTArray
//______________________________________________________________________________________________________//
//	Role			:	Initialize the tEvent array of a given task: set the ucEvt of every tEvent to NO_EVENT
//					
//	Inputs		:	The  task to initialize
//					
//	Outputs		:	None
//
//	Added By		:  	NGE / RBB / LB 
//____________________________________________________________________________________*/

void InitEVENTArray(tTask* tTaskToInitEvtArray)
{   
	unsigned int EVENTArrayCounter = 0 ;
	for(EVENTArrayCounter = 0 ; EVENTArrayCounter < tTaskToInitEvtArray->uEventArrayLength; EVENTArrayCounter++)
	{
		_GET_ucEVT(tTaskToInitEvtArray,EVENTArrayCounter) = NO_EVENT;	// Initialize all ucEvt to No tEvent
	}
}


//______________________________________________________________________
//	Procedure	:	SchEventManager
//______________________________________________________________________
//	Role			:	Manage EVENTs 
//					
//	Inputs		:	The array of existing tasks , the current index of task array
//					
//	Outputs 	:	non
//
//	Added By		:	NGE / LB
//____________________________________________________________________________________


void SchEventManager(tTask *SchTaskArray)
{
	tEvent* evtCurrentEvent;
	tTask* tskCurrentTask;
    
	u16 uiCounterTaskArray = 0;
    static u08 uDelayTic=0;
        
	u16 uCurrenEvent;
	u08* ucEvt;
	u32* lTO;
	
	while (1)
	{
		for (uiCounterTaskArray = 0; (uiCounterTaskArray < uiIndexTaskArray) && (!uSchTic); uiCounterTaskArray++)
		{
			tskCurrentTask = &SchTaskArray[uiCounterTaskArray];
			for(uCurrenEvent=0;uCurrenEvent<tskCurrentTask->uEventArrayLength ;uCurrenEvent++) {
				evtCurrentEvent = _GET_pEVT(tskCurrentTask, uCurrenEvent);
				ucEvt=&evtCurrentEvent->ucEvt;
				if(*ucEvt != NO_EVENT)
				{
					lTO=&evtCurrentEvent->lTO;
					if (!*lTO)
					{												
						if ((tskCurrentTask->Fnct(evtCurrentEvent) != IN_PROGRESS)) 
							if (!(*lTO+=evtCurrentEvent->lTimer)) *ucEvt = NO_EVENT;
					}
					else if(uDelayTic) *lTO=*lTO-1;
				}
			}
		}
        
        if(uDelayTic) uDelayTic--;
        uDelayTic+=uSchTic;
        uSchTic=0;  
	}
}


