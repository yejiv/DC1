#pragma once	

// Log
DECLARE_LOG_CATEGORY_EXTERN(LogDC1, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogDC1Warning, Warning, All);
DECLARE_LOG_CATEGORY_EXTERN(LogDC1Error, Error, All);
DECLARE_LOG_CATEGORY_EXTERN(LogDC1Fatal, Fatal, All);

#define LOG(Format, ...) UE_LOG(LogDC1, Log, Format, ##__VA_ARGS__)
#define LOG_WARNING(Format, ...) UE_LOG(LogDC1Warning, Warning, Format, ##__VA_ARGS__)
#define LOG_ERROR(Format, ...) UE_LOG(LogDC1Error, Error, Format, ##__VA_ARGS__)
#define LOG_FATAL(Format, ...) UE_LOG(LogDC1Fatal, Fatal, Format, ##__VA_ARGS__)

// UButton Cliked Add Dynamic Helper Macro
#define BIND_BUTTON_CLICKED(Button, Func)						\
	do													\
	{													\
		if (true == ::IsValid(Button))					\
		{												\
			Button->OnClicked.AddDynamic(this, Func);	\
		}												\
	} while (0)		

