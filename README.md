# Neologger ![Version](https://img.shields.io/badge/Version-0.1.0-blue.svg) ![Language](https://img.shields.io/badge/Language-C%2FC%2B%2B-blue.svg)
***Neo Logger*** - **Is a project written in pure C/C++ for custom recording of events to a file**

# **Dependencies**
- [**Git**](https://git-scm.com/downloads)

# Using
- **Installation from repository**
  ```bash
  git clone https://github.com/reslaid/neologger.git
  ```

- **Inclusion in the project**
  - *C*
    ```c
    #include "neologger/neologger.h"
    ```
  - *C++*
    ```cpp
    #include "neologger/neologger.hpp"
    ```

- **Example**
  - *C*
    ```c
    #include "neologger/neologger.h"
    
    int main() {
        Logger* logger = createLogger(L"log.txt");
        if (logger == NULL) {
            wprintf(L"Logger creation failed.\n");
            return 1;
        }
    
        LogText* logText = getLogText(L"Sample log message");
        if (logText == NULL) {
            wprintf(L"LogText creation failed.\n");
            destroyLogger(logger);
            return 1;
        }
    
        LogMessage* logMessage = toLogMessage(INFO, logText);
        if (logMessage == NULL) {
            wprintf(L"LogMessage creation failed.\n");
            free(logText->text);
            free(logText);
            destroyLogger(logger);
            return 1;
        }
    
        logMessage(logger, logMessage, 0);
    
        free(logText->text);
        free(logText);
        free(logMessage);
    
        destroyLogger(logger);
    
        return 0;
    }
    ```

  - *C++*
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
