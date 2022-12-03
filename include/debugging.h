#ifndef SUDOKU_DEBUGGING_H
#define SUDOKU_DEBUGGING_H

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdarg>

#ifndef DEBUG_ENABLED
 #define DEBUG_ENABLED false
#endif
#ifndef DEBUG_ENABLED_VERBOSE
 #define DEBUG_ENABLED_VERBOSE false
#endif

#ifndef DEBUG_INDENT_STYLE
 #define DEBUG_INDENT_STYLE std::string("    ")
#endif

class DebugStatus {
    private:
        std::string currentIndent;
    public:
        static DebugStatus& GetInstance() {
            static DebugStatus ds;
            return ds;
        }
        std::string getCurrentIndent() {
            return currentIndent;
        }
        void indent() {
            currentIndent += DEBUG_INDENT_STYLE;
        }
        void outdent() {
            currentIndent = currentIndent.substr(0, currentIndent.length() - DEBUG_INDENT_STYLE.length());
        }
};

#define DEBUG_HEADER_WIDTH 100 - DEBUG_CURRENT_INDENT.length()
#define DEBUG_CURRENT_INDENT DebugStatus::GetInstance().getCurrentIndent()

#define DEBUG_ENABLED_VERBOSE true
#if DEBUG_ENABLED_VERBOSE
    #define DEBUG_INDENT() DebugStatus::GetInstance().indent();
    #define DEBUG_OUTDENT() DebugStatus::GetInstance().outdent();
    #define DEBUG_OUTPUT(fmt, ...) \
        std::cout << DEBUG_CURRENT_INDENT; \
        printf(fmt __VA_OPT__(,) __VA_ARGS__); \
        std::cout << std::endl;
    #define DEBUG_OUTPUT_IF(cond, fmt, ...) if(cond) {DEBUG_OUTPUT(fmt __VA_OPT__(,) __VA_ARGS__)}
    #define DEBUG_FUNC_HEADER(header, ...) \
        DEBUG_OUTPUT(header __VA_OPT__(,) __VA_ARGS__) \
        std::cout << DEBUG_CURRENT_INDENT << std::setw(DEBUG_HEADER_WIDTH) << std::setfill('=') << "" << std::endl; \
        DEBUG_INDENT() 
    #define DEBUG_FUNC_RETURN(retval) \
        DEBUG_OUTPUT("Returning " << retval) \
        DEBUG_OUTDENT()
    #define DEBUG_FUNC_END() \
        DEBUG_OUTDENT() 
    #define DEBUG_STATEMENT(statement) statement;
    #define DEBUG_IF_THEN(cond, statement) if(cond) statement;
#elif DEBUG_ENABLED
    #define DEBUG_FUNC_HEADER(header, ...) DEBUG_OUTPUT(header __VA_OPT__(,) __VA_ARGS__)
    #define DEBUG_OUTPUT(fmt, ...) 
    #define DEBUG_OUTPUT_IF(cond, fmt, ...) 
    #define DEBUG_INDENT()
    #define DEBUG_OUTDENT()
    #define DEBUG_FUNC_RETURN
    #define DEBUG_FUNC_END()
    #define DEBUG_STATEMENT(statement) 
    #define DEBUG_IF_THEN(cond, statement) 
#else
    #define DEBUG_OUTPUT(fmt, ...) 
    #define DEBUG_OUTPUT_IF(cond, fmt, ...) 
    #define DEBUG_FUNC_HEADER(header, ...) 
    #define DEBUG_INDENT()
    #define DEBUG_OUTDENT()
    #define DEBUG_FUNC_RETURN
    #define DEBUG_FUNC_END()
    #define DEBUG_STATEMENT(statement) 
    #define DEBUG_IF_THEN(cond, statement) 
#endif

#endif // SUDOKU_DEBUGGING_H