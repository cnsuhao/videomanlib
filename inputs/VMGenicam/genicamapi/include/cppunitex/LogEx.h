//-----------------------------------------------------------------------------
//  (c) 2005 by Basler Vision Technologies
//  Section:  Vision Components
//  Project:  CppUnitEx
//  Author:  Fritz Dierks
//  $Header$
//-----------------------------------------------------------------------------
/**
\file
\brief    includes all include files from Log module.
*/

#ifndef DEF_LOG_H_
#define DEF_LOG_H_

#include <Log/CLog.h>

//! Operator required for using hex opeator etc
log4cpp::CategoryStream& operator <<(log4cpp::CategoryStream &ostr, const GenICam::gcstring &str);

//! Operator required for using hex opeator etc
log4cpp::CategoryStream& operator <<(log4cpp::CategoryStream &ostr, const std::string &str);

#endif // DEF_LOG_H_
