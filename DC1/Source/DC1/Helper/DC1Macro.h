#pragma once


// UButton Cliked Add Dynamic Helper Macro
#define BIND_BUTTON_CLICKED(Button, Func)						\
	do													\
	{													\
		if (true == ::IsValid(Button))					\
		{												\
			Button->OnClicked.AddDynamic(this, Func);	\
		}												\
	} while (0)					