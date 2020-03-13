
#include "mcu/debug.h"
#include "cortexm/cortexm.h"
#include "cortexm/task.h"

void __assert_func(
		const char *file,
		int line,
		const char *func,
		const char *failedexpr
		){


	if( cortexm_is_root_mode() ||
			(task_get_pid( task_get_current() ) == 0)
			){
		mcu_debug_printf(
					"assertion \"%s\" failed: file \"%s\", line %d%s%s\n",
					failedexpr,
					file,
					line,
					func ? ", function: " : "",
					func ? func : ""
								 );
		mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_SYSTEM_ASSERT, NULL);

	} else {
		fiprintf(
					stderr,
					"assertion \"%s\" failed: file \"%s\", line %d%s%s\n",
					failedexpr,
					file,
					line,
					func ? ", function: " : "",
					func ? func : ""
								 );
		abort();

	}

}
