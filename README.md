# Neologger ![Version](https://img.shields.io/badge/Version-0.1.0-blue.svg) ![Language](https://img.shields.io/badge/Language-C%2FC%2B%2B-purple.svg) ![Standart](https://img.shields.io/badge/C++_STD-17-404.svg)


# **Dependencies**
- [**Git**](https://git-scm.com/downloads)

# Installing
- **Installation from repository**
  ```bash
  git clone https://github.com/reslaid/neologger.git
  ```

# Docs
- **Including a header file**
    ```cpp
    #include "neologger/neologger.hpp"
    ```

- **Creating an Instance**
    ```cpp
    NeoLogger::Logger logger(L"application.log");
    ```
- **Destroying an instance**
    ```cpp
    logger.destroy();
    ```
- **Formatting Output**
    - **Formatter tokens**
        - `%asctime%`: **Event time**
        - `%level%`: **Event level**
        - `%message%`: **Event message text**
        - `%login%`: **Login of the user on which the recording was made (does not work well with Cyrillic)**
        - `%device%`: **Name of the device on which the recording was made**

    - **Customization formatters**
        ```cpp
        logger.setFormatter(L"{%login%}:{%device%} [%asctime%] [%level%]: %message%");
        ```

- **LogText Structure**
    ```cpp
    NeoLogger::Core::LogText nlText;
    ```
    - **Manual creation**
        ```cpp
        nlText.text = L"Message Text";
        nlText.length = nlText.text.length();
        ```
    - **Creating a method from a class**
        ```cpp
        nlText = logger.getLogText(L"Message Text");
        ```

- **LogMessage Structure**
    ```cpp
    NeoLogger::Core::LogMessage nlMessage;
    ```
    - **Manual creation**
        ```cpp
        nlMessage.level = NeoLogger::Core::LogLevel::INFO;
        nlMessage.text = nlText;
        nlMessage.timestamp = logger.getLogTimestamp();
        ```

    - **Creating a method from a class**
        ```cpp
        nlMessage = logger.toLogMessage(
            NeoLogger::Core::LogLevel::INFO,
            nlText
        );
        ```

- **Checking the file for successfulopening**
    ```cpp
    bool nlFileIsOpen = logger.is_open();
    ```

- **Event logging**
    - **Overload No. 1**
        ```cpp
        logger.logMessage(
            nlMessage,
            true // Logging to the cli (console)
        );
        ```

        - **Parameters**
            - `logMessage`: ***NeoLogger::Core::LogMessage***
            - `consoleStream`: ***bool***
    
    - **Overload No. 2**
        ```cpp
        logger.logMessage(
            NeoLogger::Core::LogLevel::INFO,
            nlText,
            true // Logging to the cli (console)
        );
        ```

        - **Parameters**
            - `logLevel`: ***NeoLogger::Core::LogLevel***
            - `logText`: ***NeoLogger::Core::LogText logText***
            - `consoleStream`: ***bool***

# **Example**
- **Basic logging**
    
    ```cpp
    #include "neologger/neologger.hpp"

    int main()
    {
        NeoLogger::Logger logger(L"log.txt");

        logger.logMessage(
            NeoLogger::Core::LogLevel::DEBUG,
            logger.getLogText(L"Debug message"),
            false
        );

        logger.logMessage(
            logger.toLogMessage(
                NeoLogger::Core::LogLevel::INFO,
                logger.getLogText(L"Informational message")
            ),
            false
        );

        logger.logMessage(
            logger.toLogMessage(
                NeoLogger::Core::LogLevel::ERROR,
                logger.getLogText(L"Error message")
            ),
            false
        );

        return 0x0;
    }
    ```

- **Logging after destruction**
    ```cpp
    #include "neologger.hpp"

    int main() {
        NeoLogger::Logger logger(L"destroy-example.txt");

        // Creating a lambda to save time and improve code readability
        auto log = [&](const wchar_t* message) {
            auto nlText = logger.getLogText(message);
            auto nlMessage = logger.toLogMessage(NeoLogger::Core::LogLevel::INFO, nlText);
            logger.logMessage(nlMessage, true);
        };

        // OK
        log(L"Message: 1");

        // Logger destruction (removal from stack)
        logger.destroy();

        // INVALID_HEAP_POINTER
        log(L"Message: 2");

        return 0x0;
    }
    ```
