#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <time.h>
#include <stdio.h>

// Error indicating that the file was not opened
#define OPENERR -0xF4

// Indicates successful operation
#define OK 0x0

// Enumerator responsible for logging levels
typedef enum {
    DEBUG,
    INFO,
    WARN,
    WARNING = WARN,
    ERROR,
    CRITICAL,
    FATAL = CRITICAL
} LogLevel;

// Data structure responsible for text in logs
typedef struct {
    // Represents log text
    wchar_t* text;

    // Represents the length of text in logs
    long long length;
} LogText;

// Represents the timestamp of an event in logs
typedef struct {
    // Formatted time
    struct tm timeInfo;

    // Numeric time (Unix-like)
    time_t time;
} LogTimestamp;

// Data structure responsible for log messages
typedef struct {
    // Log level
    LogLevel level;

    // Text of the message
    LogText text;

    // Time of inserting the message
    LogTimestamp timestamp;
} LogMessage;

// Class for working with text tokens
typedef struct {
    // Change the token to the specified value
    void (*replace)(wchar_t* input, const wchar_t* token, const wchar_t* replacement);

    // Check if the token exists in the text
    int (*exist)(wchar_t* input, const wchar_t* token);
} Tokens;

// Main logger class from which all actions are performed
typedef struct {
    // The stream responsible for outputting events to a file
    FILE* stream;

    // The file name or path to the file used in logging
    wchar_t* filename;

    // Basic output format for events
    wchar_t* formatter;

    // Helper class for the formatter designed to work with tokens
    Tokens tokens;
} Logger;

// Logger class constructor (takes the name of the file to log to)
Logger* createLogger(const wchar_t* filename) {
    Logger* logger = (Logger*)malloc(sizeof(Logger));
    if (logger == NULL) {
        return NULL;
    }

    logger->filename = wcsdup(filename);
    logger->stream = _wfopen(logger->filename, L"ab+");
    logger->formatter = wcsdup(L"[%asctime%] [%level%]: %message%");

    if (logger->stream == NULL || logger->filename == NULL || logger->formatter == NULL) {
        fclose(logger->stream);
        free(logger->filename);
        free(logger->formatter);
        free(logger);
        return NULL;
    }

    return logger;
}

// Logger class destructor
void destroyLogger(Logger* logger) {
    if (logger != NULL) {
        fclose(logger->stream);
        free(logger->filename);
        free(logger->formatter);
        free(logger);
    }
}

// Check if the file is open
int checkFile(Logger* logger) {
    if (logger == NULL || logger->stream == NULL) {
        // Return open error
        return OPENERR;
    }

    // File was opened successfully
    return OK;
}

// Get the formatted string
void parseTokens(Logger* logger, wchar_t* logTimestamp, wchar_t* logLevel, wchar_t* logText, wchar_t** result) {
    // Changing tokens to real values
    size_t pos;
    pos = wcsstr(*result, L"%asctime%");
    while (pos != NULL) {
        wcsncpy(*result + pos, logTimestamp, wcslen(logTimestamp));
        pos = wcsstr(*result, L"%asctime%");
    }

    pos = wcsstr(*result, L"%level%");
    while (pos != NULL) {
        wcsncpy(*result + pos, logLevel, wcslen(logLevel));
        pos = wcsstr(*result, L"%level%");
    }

    pos = wcsstr(*result, L"%message%");
    while (pos != NULL) {
        wcsncpy(*result + pos, logText, wcslen(logText));
        pos = wcsstr(*result, L"%message%");
    }
}

// Get extended string from log level enumerator
wchar_t* levelToWideString(LogLevel logLevel) {
    switch (logLevel) {
    case DEBUG:
        return L"DEBUG";
    case INFO:
        return L"INFO";
    case WARN:
        return L"WARN";
    case ERROR:
        return L"ERROR";
    case CRITICAL:
        return L"CRITICAL";
    default:
        return L"UNKNOWN";
    }
}

// Function for converting from an extended string to a structure
LogText* getLogText(const wchar_t* wideString) {
    LogText* logText = (LogText*)malloc(sizeof(LogText));
    if (logText == NULL) {
        return NULL;
    }

    logText->text = wcsdup(wideString);
    logText->length = wcslen(wideString);

    if (logText->text == NULL) {
        free(logText);
        return NULL;
    }

    return logText;
}

// Function for converting from an extended string to a structure
LogTimestamp* getLogTimestamp() {
    LogTimestamp* logTimestamp = (LogTimestamp*)malloc(sizeof(LogTimestamp));
    if (logTimestamp == NULL) {
        return NULL;
    }

    logTimestamp->time = time(NULL);

    // Get formatted time
    setlocale(LC_TIME, "");
    struct tm* timeInfo = localtime(&logTimestamp->time);
    logTimestamp->timeInfo = *timeInfo;

    return logTimestamp;
}

// Function for converting from logger options to a complete event
LogMessage* toLogMessage(LogLevel logLevel, LogText* logText) {
    LogMessage* logMessage = (LogMessage*)malloc(sizeof(LogMessage));
    if (logMessage == NULL) {
        return NULL;
    }

    logMessage->level = logLevel;
    logMessage->text = *logText;
    logMessage->timestamp = *getLogTimestamp();

    return logMessage;
}

// Logging event from structure
int logMessageFromStruct(Logger* logger, LogMessage* logMessage, int consoleStream) {
    if (logger == NULL || logMessage == NULL) {
        return OPENERR;
    }

    LogLevel level = logMessage->level;
    wchar_t* wideLevelString = levelToWideString(level);

    LogText text = logMessage->text;
    LogTimestamp timestamp = logMessage->timestamp;

    wchar_t formattedTime[50];
    wcsftime(formattedTime, sizeof(formattedTime), L"%Y-%m-%d %H:%M:%S", &timestamp.timeInfo);

    if (checkFile(logger) == OPENERR) {
        fwprintf(stderr, L"Open file error: %s\n", logger->filename);
        return OPENERR;
    }

    wchar_t* msg = wcsdup(logger->formatter);
    if (msg == NULL) {
        return OPENERR;
    }

    parseTokens(logger, formattedTime, wideLevelString, text.text, &msg);

    fwprintf(logger->stream, L"%s\n", msg);

    if (consoleStream) {
        wprintf(L"%s\n", msg);
    }

    free(msg);
    return OK;
}

// Logging event from passed values
int logMessage(Logger* logger, LogLevel logLevel, LogText* logText, int consoleStream) {
    if (logger == NULL || logText == NULL) {
        return OPENERR;
    }

    wchar_t* wideLevelString = levelToWideString(logLevel);

    LogTimestamp timestamp = *getLogTimestamp();

    wchar_t formattedTime[50];
    wcsftime(formattedTime, sizeof(formattedTime), L"%Y-%m-%d %H:%M:%S", &timestamp.timeInfo);

    if (checkFile(logger) == OPENERR) {
        fwprintf(stderr, L"Open file error: %s\n", logger->filename);
        return OPENERR;
    }

    wchar_t* msg = wcsdup(logger->formatter);
    if (msg == NULL) {
        return OPENERR;
    }

    parseTokens(logger, formattedTime, wideLevelString, logText->text, &msg);

    fwprintf(logger->stream, L"%s\n", msg);

    if (consoleStream) {
        wprintf(L"%s\n", msg);
    }

    free(msg);
    return OK;
}
