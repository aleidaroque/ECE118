/*
 * File: TemplateSubHSM.c
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 *
 * Template file to set up a Heirarchical State Machine to work with the Events and
 * Services Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that
 * this file will need to be modified to fit your exact needs, and most of the names
 * will have to be changed to match your code.
 *
 * There is for a substate machine. Make sure it has a unique name
 *
 * This is provided as an example and a good place to start.
 *
 * History
 * When           Who     What/Why
 * -------------- ---     --------
 * 09/13/13 15:17 ghe      added tattletail functionality and recursive calls
 * 01/15/12 11:12 jec      revisions for Gen2 framework
 * 11/07/11 11:26 jec      made the queue static
 * 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 * 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
 */


/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "HSM.h"
#include "SubHSMRunning.h"
#include "roach.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
typedef enum {
    InitPSubState,
    RunningStraight,
    SearchingLeft,
    SearchingRight,
    Reversing,
    Turning,
} SubHSMState_t;

static const char *StateNames[] = {
	"InitPSubState",
	"RunningStraight",
	"SearchingLeft",
	"SearchingRight",
	"Reversing",
	"Turning",
};



/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

static SubHSMState_t CurrentState = InitPSubState; // <- change name to match ENUM
static uint8_t MyPriority;


#define ROACH_SPEED 100
#define ROACH_TIMERS_TICKS 1000
#define ROACH_DANCING_TICKS 4000
#define RUN_TIMER_TICKS 7000
#define ROACH_AVOID_TICKS 300
#define ROACH_TURN_TICKS 200


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitTemplateSubHSM(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateFSM function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitSubHSMRunning(void) {
    ES_Event returnEvent;

    CurrentState = InitPSubState;
    returnEvent = RunSubHSMRunning(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

/**
 * @Function RunTemplateSubHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the heirarchical state
 *        machine, as this is called any time a new event is passed to the event
 *        queue. This function will be called recursively to implement the correct
 *        order for a state transition to be: exit current state -> enter next state
 *        using the ES_EXIT and ES_ENTRY events.
 * @note Remember to rename to something appropriate.
 *       The lower level state machines are run first, to see if the event is dealt
 *       with there rather than at the current level. ES_EXIT and ES_ENTRY events are
 *       not consumed as these need to pass pack to the higher level state machine.
 * @author J. Edward Carryer, 2011.10.23 19:25
 * @author Gabriel H Elkaim, 2011.10.23 19:25 */
ES_Event RunSubHSMRunning(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    SubHSMState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitPSubState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                // this is where you would put any actions associated with the
                // transition from the initial pseudo-state into the actual
                // initial state

                // now put the machine into the actual initial state
                nextState = RunningStraight;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;

        case RunningStraight: // in the first state, replace this with correct names
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Roach_LeftMtrSpeed(ROACH_SPEED);
                    Roach_RightMtrSpeed(ROACH_SPEED);
                    ES_Timer_InitTimer(ROACH_TIMER, RUN_TIMER_TICKS);
                    break;
                    
                case ES_EXIT:
                    ES_Timer_StopTimer(ROACH_TIMER);
                    break;
                    
                case ES_TIMEOUT:
                    nextState = SearchingLeft;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    
                case BUMPER_CHANGED:
                    if (ThisEvent.EventParam == 0b0001 || 
                            ThisEvent.EventParam == 0b0010 ||
                            ThisEvent.EventParam == 0b0011) { // one of the front bumpers was pressed
                        nextState = Reversing;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else {
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    
                    break;
                case ES_NO_EVENT:
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;

        case SearchingLeft: // in the first state, replace this with correct names
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Roach_LeftMtrSpeed(ROACH_SPEED/2);
                    Roach_RightMtrSpeed(ROACH_SPEED);
                    ES_Timer_InitTimer(ROACH_TIMER, ROACH_DANCING_TICKS);
                    break;
                    
                case ES_EXIT:
                    ES_Timer_StopTimer(ROACH_TIMER);
                    break;
                    
                case BUMPER_CHANGED:
                    if (ThisEvent.EventParam == 0b0001 || 
                            ThisEvent.EventParam == 0b0010 ||
                            ThisEvent.EventParam == 0b0011) { // one of the front bumpers was pressed
                        nextState = Reversing;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else {
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                    
                case ES_TIMEOUT:
                    nextState = SearchingRight;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    
                    
                case ES_NO_EVENT:
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        case SearchingRight: // in the first state, replace this with correct names
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Roach_LeftMtrSpeed(ROACH_SPEED);
                    Roach_RightMtrSpeed(ROACH_SPEED/2);
                    ES_Timer_InitTimer(ROACH_TIMER, ROACH_DANCING_TICKS);
                    break;
                    
                case ES_EXIT:
                    ES_Timer_StopTimer(ROACH_TIMER);
                    break;
                    
                case BUMPER_CHANGED:
                    if (ThisEvent.EventParam == 0b0001 || 
                            ThisEvent.EventParam == 0b0010 ||
                            ThisEvent.EventParam == 0b0011) { // one of the front bumpers was pressed
                        nextState = Reversing;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else {
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                    
                case ES_TIMEOUT:
                    nextState = RunningStraight;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    
                    
                case ES_NO_EVENT:
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        case Reversing: // in the first state, replace this with correct names
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Roach_LeftMtrSpeed(-ROACH_SPEED);
                    Roach_RightMtrSpeed(-ROACH_SPEED);
                    ES_Timer_InitTimer(ROACH_TIMER, ROACH_AVOID_TICKS);
                    break;
                    
                case ES_EXIT:
                    ES_Timer_StopTimer(ROACH_TIMER);
                    break;
                    
                case ES_TIMEOUT:
                    nextState = Turning;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    
                case ES_NO_EVENT:
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        case Turning: // in the first state, replace this with correct names
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Roach_LeftMtrSpeed(-ROACH_SPEED);
                    Roach_RightMtrSpeed(ROACH_SPEED);
                    ES_Timer_InitTimer(ROACH_TIMER, ROACH_TURN_TICKS);
                    break;
                    
                case ES_EXIT:
                    ES_Timer_StopTimer(ROACH_TIMER);
                    break;
                    
                case ES_TIMEOUT:
                    nextState = RunningStraight;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    
                case ES_NO_EVENT:
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;

        default: // all unhandled states fall into here
            break;
    } // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunSubHSMRunning(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunSubHSMRunning(ENTRY_EVENT); // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/

