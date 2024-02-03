# Neologger ![Version](https://img.shields.io/badge/Version-0.1.0-blue.svg) ![Language](https://img.shields.io/badge/Language-C%2FC%2B%2B-purple.svg)

# **Dependencies**
- [**Git**](https://git-scm.com/downloads)

# Using
- **Installation from repository**
  ```bash
  git clone https://github.com/reslaid/neologger.git
  ```

- **Inclusion in the project**
    ```cpp
    #include "neologger/neologger.hpp"
    ```

- **Example**
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
