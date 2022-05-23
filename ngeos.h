/********************************************************************************************************/
/*				New Global Electronics		BL 2012 Copyright Boost License														*/
/********************************************************************************************************/	
/*
*******************************************************************************
**  Structures
*******************************************************************************
*/
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include <string.h>


/*
*******************************************************************************
**  Macro define / Functions Prototypes
*******************************************************************************
*/


typedef unsigned char u08;
typedef unsigned short u16;
typedef unsigned long u32;


typedef unsigned int (*pCBFct)(void*);

	

typedef void (*pBaseFct)(void);

/*
Task possible status (tTask.tskStatus)
WAIT: Task has no event in the task event list to process, wait state until a new event has been elected by the scheduler to be processed by the task.
IN_PROGRESS: If an event is being processed without timeout (tEvent.ucEvt is DELAY, PERIODIC, NORMAL, ...) the scheduler will keep running the same event until a WAIT return status. If the event is being processed with timout (tEvent.ucEvt=TIMEOUT), the scheduler will stop running the same event if lTO=0.
PAUSED:  The task is not removed from the task list managed by the scheduler. The event list task is not evaluated to elect an event to process by the task, but the event list can still receive events until it's full (a error is then returned to the sender).
SUSPENDED: The task is removed from the task list processed by the scheduler. If any event is sent to the task a error value is returned to the sender. A task can be re-activated by adding it to the scheduler tasks list.
*/
typedef enum {WAIT,IN_PROGRESS, PAUSED, SUSPENDED}eTskStatus;


/* Event types (tEvent.ucEvt):
NO_EVENT: The element in the event list has no event, should be ignored.
INIT: This is an initialization event to be processed only once at the startup of the task.
TIMEOUT: The lTO counter is decremented until it reaches 0, whatever is the possible task state (should be only WAIT or IN_PROGRESS_WITH_TIMEOUT). The event has not to be processed anymore if tT0=0 (tTask.tskStatus changes from IN_PROGRESS_WITH_TIMEOUT to WAIT and tEvent.ucEvt changes to NO_EVENT)
DELAY: The lTO is decremented until it reached 0. Until then, the event is not processed. When lTO is 0, the event can be processed (with no timeout)
PERIODIC: Same behaviour than DELAY, the difference is that after reaching lTO=0, the time counter is set to the period value to retrig a next processing of the same event.
NORMAL: No special behaviour to process the event. The event is processed when the scheduler has elected the event.
*/
/*Caution: MUTEX should always be the last enumeration value in EVENT_TYPE list */
typedef enum {NO_EVENT, INIT, TIMEOUT, DELAY, PERIODIC, NORMAL}EVENT_TYPE;


/* ________________________________________________________//
An Event   is a  structure stored in the RAM memory composed of :
-The Event Type stored in ucEvt.
-A Free field : pMsg to store any kind of message
-A Time Value tTo 
//_________________________________________________________*/


typedef struct
{
	u08 ucEvt;
	u08 uMsg;
	u32 lTimer;
	u32 lTO;
	void* pMsg;
} tEvent;



/* Application tEvent handler function pointer */
//typedef eTskStatus (*tSchTask)(tEvent *);

/* An action function should start with a switch-case statement to select the action to be performed.
switch (DATA.action)
{
	
}
To be noted that "INIT" action is often needed.
*/
typedef u08 (*tActionFnct)(void *);


/* ________________________________________________________//
A Task   is a  structure stored in the RAM memory composed of :
-An Array of Event containing the events to be processed by the event manager
-A Task ( function to be executed ) that return In progress or wait 
-A Read index to trace which task is executed
-A Write index to trace the position to add a new task and the number of existing task
-The event array length
-The Status of the task
//_________________________________________________________*/
typedef struct 
{
	u08 uEventArrayLength;	
	u08 uTaskReadIndex;
	u08 uTaskWriteIndex;
	u08 uTaskNbPendingEvents;
	u08 tskStatus;
	u08 uDataTypeLength;
	tActionFnct Fnct;
	tEvent* pEventArray;
}tTask;



/*
*******************************************************************************
**  Global Variables
*******************************************************************************
*/

#define MAX_SCH_TASKS	15
extern tTask tskTaskArray[MAX_SCH_TASKS]; // create the array containing all the tasks
extern u16 uiIndexTaskArray;
extern tTask EmergencyTskArray[];
extern u08 uSchTimer;//BOOLEAN_T



#define OBJECT_CREATE(NAME,OBJECT, DATA)                                           \
	u08 TOKENPASTE(OBJECT,FCT)(DATA* CurrentEvent)                             \
	{                                                                          \
		static const (OBJECT*) pParent = NAME;

/*______________________________________________________________________//
-//	Macro		:	SchlTaskCreate
-//______________________________________________________________________//
-//	Role			:	Create a task 
-//					
-//	Inputs		:	The array of existing tasks ,  ,  
-//					the index where to store the task and the length of tEvent table
-//					
-//	Outputs 		:	Pointer to the tEvent list of the task
-//
-//	Added By		:  	NGE / RBB / LB
-//______________________________________________________________________*/


#define ADD_EVENT(TaskName,Event)	AddEventToEventArray((TaskName),(tEvent*)(Event))


#define RESUME_TASK(TaskName)	    (TaskName)->tskStatus = WAIT

#define SCH_TSK_CREATE(TaskName,NbEventMax)      	SCH_TSK_CREATE2(TaskName,NbEventMax, tEvent)   



/*______________________________________________________________________//
-//	Macro		:	DELETE_TASK
-//______________________________________________________________________//
-//	Role			:	Delete a task  from 
-//					
-//	Inputs		:	Pointer to the task to be deleted
-//					
-//	Outputs 		:	None
-//
-//	Added By		:  	NGE / RBB / LB
-//______________________________________________________________________*/
#define DELETE_TASK(pTaskToDelete)		{                                       \
		memcpy(pTaskToDelete,pTaskToDelete + 1 ,sizeof(tTask));                 \
		uiIndexTaskArray--;                                                     \
		}

/*____________________________________________________________________________________//
//	Macros    	:	_GET_pEVT, _GET_ucEVT, _GET_lTO
//____________________________________________________________________________________//
//	Role			:	Get tEvent information from EVENTArray of different size (tTaskEVENT->uDataTypeLength) knowing the index
//					
//	Inputs		:	A pointer to a given task and an index
//					
//	Outputs		:	the current tEvent information
//
//	Added By		:  	NGE / BL
//____________________________________________________________________________________*/

#define _GET_pEVT(Task,IndexInEvtTable)		((tEvent*)((char*)Task->pEventArray+IndexInEvtTable*Task->uDataTypeLength))
#define _GET_ucEVT(Task,IndexInEvtTable)	_GET_pEVT(Task,IndexInEvtTable)->ucEvt
#define _GET_lTO(Task,IndexInEvtTable)		_GET_pEVT(Task,IndexInEvtTable)->lTO


/*______________________________________________________________________//
-//	Macro		:	ENDTASK
-//______________________________________________________________________//
-//	Role			:	End a normal task
-//______________________________________________________________________*/
#define ENDTASK()								\
		return WAIT;							\
	}											

/*______________________________________________________________________//
-//	Macro		:	SchlTaskCreate
-//______________________________________________________________________//
-//	Role			:	Create a task 
-//					
-//	Inputs		:	The array of existing tasks , the function associeted to the task ,  
-//					the index where to store the task and the length of tEvent table
-//					
-//	Outputs 		:	Pointer to the tEvent list of the task
-//
-//	Added By		:  	NGE / RBB / LB
-//______________________________________________________________________*/

/*____________________________________________________________________________________________//
-//	Macro		:	MUTEX_TASK
-//___________________________________________________________________________________________//
-//	Role			:	Create a mutex task  and a mutex function : if there is no Task in progress driver function
-//					are instantly executed , else the task add an event to the task managing the Mutex
-//___________________________________________________________________________________________//
-//  NB !              :	This implementation is only to handle concurencial accecc between task .
-//					To handle the execution of the same task in many parts a switch case is to be added 
-//					into the Mutex Create to switch the status of the task between Wait and IN_PROGRESS
-//___________________________________________________________________________________________*/

typedef void*    (*tMutexFct) (char, void*);
typedef enum {INITIALIZED,EXECUTED,PENDING,CANCELLED,ERROR}eMutexStatus;


//////////////////////////////////////////////////////////////////////////////////////
/* Specific types */


#define TOKENPASTE(x,y)		x##y
#define TOKENPASTE2(x,y,z)   	TOKENPASTE(x,y)##z


#define DevPtrStruct1(var,p1)         		    (var)->p1;
#define DevPtrStruct2(var,p1,p2)                  (var)->p2;	                        \
												DevPtrStruct1(var,p1)
#define DevPtrStruct3(var,p1,p2,p3)              (var)->p3;	                            \
												DevPtrStruct2(var,p1,p2)
#define DevPtrStruct4(var,p1,p2,p3,p4)           (var)->p4;	                            \
												DevPtrStruct3(var,p1,p2,p3)
#define DevPtrStruct5(var,p1,p2,p3,p4,p5)        (var)->p5;	                            \
												DevPtrStruct4(var,p1,p2,p3,p4)
#define DevPtrStruct6(var,p1,p2,p3,p4,p5,p6)     (var)->p6;	                            \
												DevPtrStruct5(var,p1,p2,p3,p4,p5)
#define DevPtrStruct7(var,p1,p2,p3,p4,p5,p6,p7) 	(var)->p7;                          \
												DevPtrStruct6(var,p1,p2,p3,p4,p5,p6)
#define DevPtrStruct8(var,p1,p2,p3,p4,p5,p6,p7,p8) (var)->p8;                           \
												DevPtrStruct7(var,p1,p2,p3,p4,p5,p6,p7)



/* End of Specific types */
/////////////////////////////////////////////////////////////////////////////////////////////


/* A Mutex action is a task action that can be called directly. If the action is finished, a value is returned directly to the caller, no event is added to the event list of the action associated with the Mutex.
If the action is not finished (still IN_PROGRESS), an event is added to the event list of the task associated with the Mutex. The present event (which is not a real event from the event list the 1st time) is set to NO_EVENT.
*/
	


#define _GET_MUTEX_STATUS(MutexName)		MutexName->tskStatus
#define _GET_MUTEX_ACTION(MutexName)				
#define _GET_MUTEX_FUNCTION(MutexName)
#define _GET_MUTEX_PMSG(MutexName)


#define MUTEX(MutexName,NbEventMax,EvtType)							SCH_TSK_CREATE2(MutexName,NbEventMax, EvtType)

#define SCH_TSK_CREATE2(TaskName,NbEventMax, EvtType)                                                    \
	EvtType TOKENPASTE(EvtArray, TaskName)[NbEventMax];                                          \
	tTask* TaskName;                                                                           \
	eTskStatus TOKENPASTE(TaskName, FCT)(tEvent*);          							               \
	eTskStatus TOKENPASTE(init, TaskName)(void)                      \
{                                                                                          \
	tskTaskArray[uiIndexTaskArray].Fnct = &TOKENPASTE(TaskName, FCT);                       \
	tskTaskArray[uiIndexTaskArray].pEventArray = (tEvent*)TOKENPASTE(EvtArray, TaskName);            \
	tskTaskArray[uiIndexTaskArray].uEventArrayLength = NbEventMax;                          \
	tskTaskArray[uiIndexTaskArray].uDataTypeLength = sizeof(EvtType);					   \
	tskTaskArray[uiIndexTaskArray].uTaskReadIndex = 0;                                     \
	tskTaskArray[uiIndexTaskArray].uTaskWriteIndex = 0;                                    \
	tskTaskArray[uiIndexTaskArray].tskStatus = WAIT;                                       \
	InitEVENTArray(&tskTaskArray[uiIndexTaskArray]);                                       \
	TaskName = &tskTaskArray[uiIndexTaskArray];                                              \
	uiIndexTaskArray++; 															   \
	return WAIT;                                                                           \
}                                                                                          \
	eTskStatus TOKENPASTE(TaskName, FCT)(tEvent* hEvent)							               \
{																		                   \
	tTask* pParent = TaskName;									\
	EvtType* CurrentEvent =(EvtType*) hEvent;					                                   



/*
*******************************************************************************
**  Functions
*******************************************************************************
*/
void SchEventManager(tTask *SchTaskArray);
tEvent* GetEventFromEventArray(tTask* tTaskEVENT);
tEvent* AddEventToEventArray(tTask* tTaskToAddEvt,tEvent* evtEventToAdd);
void DeleteEVENTFromEVENTArray(tEvent* eEVENTToDelete);
void InitEVENTArray(tTask* tTaskToInitEvtArray);
void AddEmergencyEVENT(tTask* tEmergencyTask,tEvent* evtEventToAdd);
void fProgramReset(void);
void UpdateTasks(void);


#endif

