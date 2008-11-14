/* Copyright 2008, Mariano Cerdeiro
 *
 * This file is part of OpenSEK.
 *
 * OpenSEK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSEK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSEK. If not, see <http://www.gnu.org/licenses/>.
 *
 */

/** \brief OpenSEK ActivateTask Implementation File
 **
 ** This file implements the ActiveTask API
 **
 ** \file ActivateTask.c
 **
 **/

/** \addtogroup OpenSEK
 ** @{ */
/** \addtogroup OpenSEK_Global
 ** @{ */

/*
 * Initials     Name
 * ---------------------------
 * MaCe         Mariano Cerdeiro
 * KLi          Kang Li
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20081113 v0.1.1 KLi        - Added memory layout attribute macros
 * 20080713 v0.1.0 MaCe       - initial version
 */

/*==================[inclusions]=============================================*/
#include "Osek_Internal.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
#pragma MEM_ATTRIB_OSEK_CODE_BEGIN

StatusType ActivateTask
(
	TaskType TaskID
)
{
	/* \req OSEK_SYS_3.1 The system service StatusType
	 * ActivateTask ( TaskType TaskID ) shall be defined. */

	/* \req OSEK_SYS_3.1.3 The service may be called from interrupt category 2
    * level and from task level. */
	/* nothing to do for this req. */

	/* \req OSEK_SYS_3.1.7-1/3 Possible return values in Standard mode are E_OK or E_OS_LIMIT */
	StatusType ret = E_OK;

#if (ERROR_CHECKING_TYPE == ERROR_CHECKING_EXTENDED)
	/* check if the task id is valid */
	if ( TaskID >= TASKS_COUNT )
	{
		/* if an invalid task id return E_OS_ID */
		/* \req OSEK_SYS_3.1.5-1/3 If other than E_OK is returned the activation
		 * is ignored */
		/* \req OSEK_SYS_3.1.8 Added possible return values in Extended mode is
		 * E_OS_ID */
		ret = E_OS_ID;
	}
	else
#endif
	{
		IntSecure_Start();

		/* check if the task is susspended */
		/* \req OSEK_SYS_3.1.1-1/2 The task TaskID shall be transferred from the
		 * suspended state into the ready state. */
		if ( TasksVar[TaskID].Flags.State == TASK_ST_SUSPENDED )
		{
			/* increment activation counter */
			TasksVar[TaskID].Activations++;
			/* if the task was suspended set it to ready */
			/* OSEK_SYS_3.1.1-2/2 The task TaskID shall be transferred from the
			 * suspended state into the ready state.*/
			TasksVar[TaskID].Flags.State = TASK_ST_READY;
			/* clear all events */
			/* \req OSEK_SYS_3.1.6 When an extended task is transferred from
			 * suspended state into ready state all its events are cleared. */
			TasksVar[TaskID].Events = 0;
			/* add the task to the ready list */
			AddReady(TaskID);
		}
		else
		{
			/* task is not suspended */

			/* check if the task is a extended task */
			if ( TasksConst[TaskID].ConstFlags.Extended ) 
			{
				/* return E_OS_LIMIT */
				/* \req OSEK_SYS_3.1.5-2/3 If other than E_OK is returned the activation
				 * is ignored */
				/* \req OSEK_SYS_3.1.7-2/3 Possible return values in Standard mode are
				 * E_OK or E_OS_LIMIT */
				ret = E_OS_LIMIT;
			}
			else
			{
				/* check if more activations are allowed */
				if ( TasksVar[TaskID].Activations < TasksConst[TaskID].MaxActivations )
				{
					/* increment activation counter */
					TasksVar[TaskID].Activations++;
					/* add the task to the ready list */
					AddReady(TaskID);
				}
				else
				{
					/* maximal activation reached, return E_OS_LIMIT */
					/* \req OSEK_SYS_3.1.5-3/3 If other than E_OK is returned the
					 * activation is ignored */
					/* \req OSEK_SYS_3.1.7-3/3 Possible return values in Standard mode are
					 * E_OK or E_OS_LIMIT */
					ret = E_OS_LIMIT;
				}
			}
		}		

		IntSecure_End();


		if (	( GetCallingContext() ==  CONTEXT_TASK ) &&
			( TasksConst[GetRunningTask()].ConstFlags.Preemtive ) &&
			( ret == E_OK )	)
		{
			/* \req OSEK_SYS_3.1.4 Rescheduling shall take place only if called from a
			 * preemptable task. */
			(void)Schedule();
		}
	}


#if (HOOK_ERRORHOOK == ENABLE)
	/* \req OSEK_ERR_1.3-1/xx The ErrorHook hook routine shall be called if a
 	 * system service returns a StatusType value not equal to E_OK.*/
	/* \req OSEK_ERR_1.3.1-1/xx The hook routine ErrorHook is not called if a
 	 * system service is called from the ErrorHook itself. */
	if ( ( ret != E_OK ) && (ErrorHookRunning != 1U))
 	{
		SetError_Api(OSServiceId_ActivateTask);
		SetError_Param1(TaskID);
		SetError_Ret(ret);
		SetError_Msg("ActivateTask returns != than E_OK");
		SetError_ErrorHook();
	}
#endif

	return ret;
}

#pragma MEM_ATTRIB_OSEK_CODE_END

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/

