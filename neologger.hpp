#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <chrono>
#include <string>
#include <cstring>
#include <cstdarg>
#include <cwchar>
#include <tuple>

#ifdef _WIN32
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")
#else
#include <unistd.h>
#include <iconv.h>
#include <sys/types.h>
#include <pwd.h>
#endif

namespace NeoLogger
{

#pragma region constants

	// Error indicating that the file was not opened
	constexpr int OPENERR = -0xF1;

	// Error indicator getting device name
	constexpr int DEVICENAMERR = -0xF2;

	// Error indicator getting login
	constexpr int LOGINERR = -0xF3;

	// Indicates successful operation
	constexpr int OK = 0x0;

	// Error indicator
	constexpr int FAILURE = OPENERR | DEVICENAMERR | LOGINERR;


#pragma endregion

	// Namespace for storing converters
	namespace Converter
	{
		// Converts a tuple type to a string type
		template <typename... Args>
		std::string tupleToString(const std::tuple<Args...>& myTuple)
		{
			// Creating a Stream Object for a String
			std::stringstream ss;

			ss << "(";

			// Recursive function to process each element of a tuple
			std::apply([&ss](const auto&... args) {
				((ss << args << ", "), ...);
				}, myTuple);

			// Removing extra commas and spaces
			std::string result = ss.str();
			result = result.substr(0, result.size() - 2);

			result += ")";

			// Return the finished line
			return result;
		}

		// Converting string to an wide string
		std::wstring toWideString(const std::string& str)
		{

#ifdef _WIN32
			int bufferSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
			wchar_t* buffer = new wchar_t[bufferSize];
			MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer, bufferSize);

			std::wstring result(buffer);
			delete[] buffer;
			return result;

#else

			iconv_t conversion = iconv_open("WCHAR_T", "UTF-8");
			if (conversion == (iconv_t)-1) {
				// Handling a converter opening error
				return L"";
			}

			size_t inSize = str.size();
			size_t outSize = inSize * sizeof(wchar_t);
			char* inBuffer = const_cast<char*>(str.c_str());
			wchar_t* outBuffer = new wchar_t[inSize];

			if (iconv(conversion, &inBuffer, &inSize, reinterpret_cast<char*>(&outBuffer), &outSize) == (size_t)-1) {
				// Handling conversion error
				delete[] outBuffer;
				iconv_close(conversion);
				return L"";
			}

			iconv_close(conversion);

			std::wstring result(outBuffer);
			delete[] outBuffer;
			return result;

#endif
		}
	} // namespace Converter

	// Namespace for storing information about the logger (version, publisher, etc.)
	namespace Metadata
	{
		const std::tuple<int, int, int> __Version__(0, 1, 0);

		// Get logger version
		std::wstring getVersion()
		{
			return NeoLogger::Converter::toWideString(
				NeoLogger::Converter::tupleToString(
					__Version__
				)
			);
		}
	} // namespace Metadata

	// Namespace for storing structures
	namespace Core
	{
		// Enumerator responsible for logging levels
		enum class LogLevel
		{
			// Debug logging level
			DEBUG,

			// Logging level for informational messages
			INFO,

			// Logging level for warnings
			WARN,

			// Logging level for warnings
			WARNING = WARN,

			// Logging level for errors
			ERR,

			// Logging level for critical errors
			CRITICAL,

			// Logging level for critical errors
			FATAL = CRITICAL
		};

		// Data structure responsible for text in logs
		struct LogText
		{
			// Represents log text
			std::wstring text;

			// Represents the length of text in logs
			std::int64_t length;
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
	} // namespace Core

	// Class for working with text tokens
	class Tokens
	{
	public:

		// Change the token to the specified value
		void replace(std::wstring& input,
			const std::wstring& token,
			const std::wstring& replacement)
		{

			// Getting a position
			size_t pos = input.find(token);

			// Changing characters
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
	}; // class Tokens

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
			if (!this->is_open())
			{
				// Return open error
				return NeoLogger::OPENERR;
			}

			// File was opened successfully
			return NeoLogger::OK;
		}

		// Get Login (Cross-Platform)
		int getLogin(int bufferSize, wchar_t* buffer)
		{
#ifdef _WIN32
			// On Windows
			DWORD size = static_cast<DWORD>(bufferSize);
			if (!GetUserNameW(buffer, &size))
			{
				// Failed to get the user name
				return NeoLogger::LOGINERR;
			}
#else
			// On other platforms
			char* loginName;

			uid_t euid = geteuid();
			struct passwd* pw = getpwuid(euid);

			if (pw != nullptr) {
				loginName = pw->name;
			}
		
			if (loginName != nullptr)
			{
				// Convert the login name to a wide string
				mbstowcs(buffer, loginName, bufferSize - 1);
				buffer[bufferSize - 1] = L'\0';  // Null-terminate the wide string
			}
			else
			{
				// Failed to get the user name
				return NeoLogger::LOGINERR;
			}
#endif

			// Successfully retrieved the login name
			return NeoLogger::OK;
		}

		// Get Device Name (Cross-Platform)
		int getDeviceName(int bufferSize, wchar_t* buffer)
		{
#ifdef _WIN32
			// On Windows
			DWORD size = static_cast<DWORD>(bufferSize);
			if (!GetComputerNameW(buffer, &size))
			{
				// Failed to get the device name
				return NeoLogger::DEVICENAMERR;
			}
#else
			// On other platforms
			if (gethostname(buffer, bufferSize) != 0)
			{
				return NeoLogger::DEVICENAMERR;
			}
#endif

			// Successfully retrieved the device name
			return NeoLogger::OK;

		}

		// Get the formatted string
		void parseTokens(
			std::wstring logTimestamp,
			std::wstring logLevel,
			std::wstring logText,
			std::wstring* result)
		{
			// Total Buffer Size
			constexpr int bufferlen = 0x100;

			// Changing tokens to real values
			this->tokens_.replace(*result, L"%asctime%", logTimestamp);
			this->tokens_.replace(*result, L"%level%", logLevel);
			this->tokens_.replace(*result, L"%message%", logText);

			if (this->tokens_.exist(*result, L"%login%"))
			{
				// Declaring object
				wchar_t* loginBuffer = new wchar_t[bufferlen];

				// Initializing the object of a variable
				this->getLogin(bufferlen, loginBuffer);

				// Using the object
				this->tokens_.replace(*result, L"%login%", std::wstring(loginBuffer));

				// Removing object from stack
				delete[] loginBuffer;
			}

			if (this->tokens_.exist(*result, L"%device%"))
			{
				// Declaring object
				wchar_t* deviceNameBuffer = new wchar_t[bufferlen];

				// Initializing the object of a variable
				this->getDeviceName(bufferlen, deviceNameBuffer);

				// Using the object
				this->tokens_.replace(*result, L"%device%", std::wstring(deviceNameBuffer));

				// Removing object from stack
				delete[] deviceNameBuffer;
			}
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
			case NeoLogger::Core::LogLevel::ERR:
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

		// Function allowing to change the output format
		void setFormatter(std::wstring& _Formatter)
		{
			if (this->tokens_.exist(_Formatter, L"%message%"))
			{
				this->formatter_ = _Formatter;
			}
		}

		// Function for converting from an extended string to a structure
		NeoLogger::Core::LogText getLogText(const std::wstring& wideString)
		{
			// Initialize our structure
			NeoLogger::Core::LogText logText;

			// Assign our values
			logText.text = wideString;
			logText.length = wideString.length();

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

#ifdef _WIN32
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
			const NeoLogger::Core::LogText& logText)
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
		int logMessage(NeoLogger::Core::LogMessage logMessage, bool consoleStream, bool fileStream)
		{
			// Getting the message level
			NeoLogger::Core::LogLevel level = logMessage.level;
			std::wstring wideLevelString = levelToWideString(level);

			// Getting the message text
			NeoLogger::Core::LogText text = logMessage.text;
			NeoLogger::Core::LogTimestamp timestamp = logMessage.timestamp;

			// Getting the message timestamp
			std::wstringstream formattedTime;
			formattedTime << std::put_time(&timestamp.timeInfo, L"%Y-%m-%d %H:%M:%S");

			// Checking if the file is open
			if (this->checkFile() == NeoLogger::OPENERR)
			{
				std::wcerr << L"Open file error: " << this->filename_ << std::endl;

				// We drop the same error
				return NeoLogger::OPENERR;
			}

			// Copying the formatter
			std::wstring msg = formatter_;

			// Substituting values ​​into our variable
			this->parseTokens(formattedTime.str(), wideLevelString, text.text, &msg);

			// Writing data into the file stream
			if (fileStream) {

				this->stream_ << msg << std::endl;

			}

			// Writing data into the cli stream
			if (consoleStream) {
				std::wcout << msg << std::endl;
			}

			// Returning the result (Everything went fine)
			return NeoLogger::OK;
		}

		// Logging event from passed values
		int logMessage(NeoLogger::Core::LogLevel logLevel, NeoLogger::Core::LogText logText, bool consoleStream, bool fileStream)
		{
			// Getting the message level
			std::wstring wideLevelString = levelToWideString(logLevel);

			// Getting the message timestamp
			NeoLogger::Core::LogTimestamp timestamp = getLogTimestamp();

			std::wstringstream formattedTime;
			formattedTime << std::put_time(&timestamp.timeInfo, L"%Y-%m-%d %H:%M:%S");

			// Checking if the file is open
			if (this->checkFile() == NeoLogger::OPENERR)
			{
				std::wcerr << L"Open file error: " << this->filename_ << std::endl;

				// We drop the same error
				return NeoLogger::OPENERR;
			}

			// Copying the formatter
			std::wstring msg = formatter_;

			// Substituting values ​​into our variable
			this->parseTokens(formattedTime.str(), wideLevelString, logText.text, &msg);

			// Writing data into the file stream
			if (fileStream) {

				this->stream_ << msg << std::endl;

			}

			// Writing data into the cli stream
			if (consoleStream) {
				std::wcout << msg << std::endl;
			}

			// Returning the result (Everything went fine)
			return NeoLogger::OK;
		}

		// Destroy the logger object in memory
		void destroy()
		{
			// Destroying an instance (calling deconstructor then clearing memory)
			delete this;
		}

		// Check if the file is open
		bool is_open()
		{
			// Getting the result of whether a file is open in boolean form
			return this->stream_.is_open();
		}
	}; // class Logger
} // namespace NeoLogger
