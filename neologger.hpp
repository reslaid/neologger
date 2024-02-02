#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ostream>
#include <sstream>
#include <istream>
#include <chrono>

namespace NeoLogger
{
    // Error indicating that the file was not opened
    constexpr int OPENERR = -0xF4;

    // Indicates successful operation
    constexpr int OK = 0x0;

    namespace Core
    {
        // Enumerator responsible for logging levels
        enum class LogLevel
        {
            DEBUG,
            INFO,
            WARN,
            WARNING = WARN,
            ERROR,
            CRITICAL,
            FATAL = CRITICAL
        };

        // Data structure responsible for text in logs
        struct LogText
        {
            // Represents log text
            std::wstring text;

            // Represents the length of text in logs
            std::int64_t length;

            // Operator: (LogText) += (LogText)
            LogText& operator+=(const LogText& logText)
            {
                this->text += logText.text;
                return *this;
            }
        };

        // Represents the timestamp of an event in logs
        struct LogTimestamp
        {
            // Formatted time
            std::tm timeInfo;

            // Numeric time (Unix-like)
            std::time_t time;
        };

        // Data structure responsible for log messages
        struct LogMessage
        {
            // Log level
            LogLevel level;

            // Text of the message
            LogText text;

            // Time of inserting the message
            LogTimestamp timestamp;
        };
    }

    // Class for working with text tokens
    class Tokens
    {
    public:

        // Change the token to the specified value
        void replace(std::wstring& input,
            const std::wstring& token,
            const std::wstring& replacement)
        {

            size_t pos = input.find(token);
            while (pos != std::wstring::npos) {
                input.replace(pos, token.length(), replacement);
                pos = input.find(token);
            }

        }

        // Check if the token exists in the text
        bool exist(std::wstring& input,
            const std::wstring& token)

        {

            return input.find(token) != std::wstring::npos;

        }
    };

    // Main logger class from which all actions are performed
    class Logger {
    private:
        // The stream responsible for outputting events to a file
        std::wofstream stream_;

        // The file name or path to the file used in logging
        std::wstring filename_;

        // Basic output format for events
        std::wstring formatter_ = L"[%asctime%] [%level%]: %message%";

        // Helper class for the formatter designed to work with tokens
        NeoLogger::Tokens tokens_;

    public:
        // Logger class constructor (takes the name of the file to log to)
        Logger(const std::wstring& filename)
        {
            this->filename_ = filename;
            this->stream_.open(this->filename_, std::ios::out | std::ios::app);

            if (this->checkFile() == NeoLogger::OPENERR)
            {
                std::wcerr << L"Open file error: " << filename << std::endl;
            }
        }

        // Logger class destructor
        ~Logger()
        {
            this->stream_.close();
        }

    private:
        // Check if the file is open
        int checkFile()
        {
            if (!this->stream_.is_open())
            {
                // Return open error
                return NeoLogger::OPENERR;
            }

            // File was opened successfully
            return NeoLogger::OK;
        }

        // Get the formatted string
        void parseTokens(
            std::wstring logTimestamp,
            std::wstring logLevel,
            std::wstring logText,
            std::wstring* result)
        {
            // Changing tokens to real values
            
            this->tokens_.replace(*result, L"%asctime%", logTimestamp);
            this->tokens_.replace(*result, L"%level%", logLevel);
            this->tokens_.replace(*result, L"%message%", logText);
        }

        // Get extended string from log level enumerator
        std::wstring levelToWideString(NeoLogger::Core::LogLevel logLevel)
        {
            std::wstring wideResult;

            switch (logLevel) {
            case NeoLogger::Core::LogLevel::DEBUG:
                return L"DEBUG";
            case NeoLogger::Core::LogLevel::INFO:
                return L"INFO";
            case NeoLogger::Core::LogLevel::WARN:
                return L"WARN";
            case NeoLogger::Core::LogLevel::ERROR:
                return L"ERROR";
            case NeoLogger::Core::LogLevel::CRITICAL:
                return L"CRITICAL";
            default:
                return L"UNKNOWN";
            }
        }

    public:

        // Function allowing to change the output format
        void setFormatter(std::wstring _Formatter)
        {
            if (this->tokens_.exist(_Formatter, L"%message%"))
            {
                this->formatter_ = _Formatter;
            }
        }

        // Function for converting from an extended string to a structure
        NeoLogger::Core::LogText getLogText(std::wstring wideString)
        {
            // Initialize our structure
            NeoLogger::Core::LogText logText;

            // Assign our values
            logText.text = wideString;
            logText.length = wideString.length();

            // Return the prepared structure
            return logText;
        }

        // Function for converting from an extended char pointer to a structure
        NeoLogger::Core::LogText getLogText(wchar_t* wideString)
        {
            // Initialize our structure
            NeoLogger::Core::LogText logText;

            // Assign our values
            logText.text = (std::wstring)wideString;
            logText.length = wcslen(wideString);

            // Return the prepared structure
            return logText;
        }

        // Function for converting from an extended char to a structure
        NeoLogger::Core::LogText getLogText(wchar_t wideChar)
        {
            // Initialize our structure
            NeoLogger::Core::LogText logText;

            // Assign our values
            logText.length = 0x1;
            logText.text = std::wstring(logText.length, wideChar);

            // Return the prepared structure
            return logText;
        }

        // Function for converting from an extended string to a structure
        NeoLogger::Core::LogTimestamp getLogTimestamp()
        {
            // Initialize our structure
            NeoLogger::Core::LogTimestamp logTimestamp;

            // Get Unix-like time
            std::time_t time = std::time(nullptr);

            // Get formatted time
            auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm timeInfo;

            // Safe time retrieval

#ifdef _WIN32 | _WIN64
            localtime_s(&timeInfo, &currentTime);
#else
            localtime_r(&currentTime, &timeInfo);
#endif
            // Assign our values
            logTimestamp.timeInfo = timeInfo;
            logTimestamp.time = time;

            // Return the prepared structure
            return logTimestamp;
        }

        // Function for converting from logger options to a complete event
        NeoLogger::Core::LogMessage toLogMessage(
            NeoLogger::Core::LogLevel logLevel,
            NeoLogger::Core::LogText logText)
        {
            // Initialize the structure
            NeoLogger::Core::LogMessage logMessage;

            // Assign values
            logMessage.level = logLevel;
            logMessage.text = logText;
            logMessage.timestamp = getLogTimestamp();

            // Return the prepared structure
            return logMessage;
        }

        // Logging event from structure
        int logMessage(NeoLogger::Core::LogMessage logMessage, bool consoleStream)
        {
            NeoLogger::Core::LogLevel level = logMessage.level;
            std::wstring wideLevelString = levelToWideString(level);

            NeoLogger::Core::LogText text = logMessage.text;
            NeoLogger::Core::LogTimestamp timestamp = logMessage.timestamp;

            std::wstringstream formattedTime;
            formattedTime << std::put_time(&timestamp.timeInfo, L"%Y-%m-%d %H:%M:%S");

            if (this->checkFile() == NeoLogger::OPENERR)
            {
                std::wcerr << L"Open file error: " << this->filename_ << std::endl;
                return NeoLogger::OPENERR;
            }

            std::wstring msg = formatter_;
            this->parseTokens(formattedTime.str(), wideLevelString, text.text, &msg);

            this->stream_ << msg << std::endl;

            if (consoleStream) {
                std::wcout << msg << std::endl;
            }

            return NeoLogger::OK;
        }

        // Logging event from passed values
        int logMessage(NeoLogger::Core::LogLevel logLevel, NeoLogger::Core::LogText logText, bool consoleStream)
        {
            std::wstring wideLevelString = levelToWideString(logLevel);

            NeoLogger::Core::LogTimestamp timestamp = getLogTimestamp();

            std::wstringstream formattedTime;
            formattedTime << std::put_time(&timestamp.timeInfo, L"%Y-%m-%d %H:%M:%S");

            if (this->checkFile() == NeoLogger::OPENERR)
            {
                std::wcerr << L"Open file error: " << this->filename_ << std::endl;
                return NeoLogger::OPENERR;
            }

            std::wstring msg = formatter_;
            this->parseTokens(formattedTime.str(), wideLevelString, logText.text, &msg);

            this->stream_ << msg << std::endl;
            
            if (consoleStream) {
                std::wcout << msg << std::endl;
            }

            return NeoLogger::OK;
        }
    };
}
