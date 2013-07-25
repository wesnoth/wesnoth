// =================================================================================
// ORACLE, ODBC and DB2/CLI Template Library, Version 4.0.298,
// Copyright (C) 1996-2013, Sergei Kuchin (skuchin@gmail.com)
// 
// This library is free software. Permission to use, copy, modify,
// and/or distribute this software for any purpose with or without fee
// is hereby granted, provided that the above copyright notice and
// this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// a.k.a. as Open BSD license
// (http://www.openbsd.org/cgi-bin/cvsweb/~checkout~/src/share/misc/license.template
// =================================================================================

#ifndef __OTL_H__
#define __OTL_H__

#if defined(OTL_INCLUDE_0)
#include "otl_include_0.h"
#endif

#define OTL_VERSION_NUMBER (0x040130L)

#if defined(_MSC_VER) && (_MSC_VER >= 1600) || \
  (defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__*100+__GNUC_MINOR__>=407)) && \
   defined(OTL_CPP_11_ON)

// VC++ 10 or higher, g++ 4.7 or higher

#if !defined(OTL_ANSI_CPP_11_RVAL_REF_SUPPORT)
#define OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
#endif

#if !defined(OTL_ANSI_CPP_11_NULLPTR_SUPPORT)
#define OTL_ANSI_CPP_11_NULLPTR_SUPPORT
#endif

// VC++ doesn't support = delete feature yet
#if !defined(_MSC_VER)
#if !defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
#define OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT
#endif
#endif

#if defined(_MSC_VER) && (_MSC_VER==1600)
// VC++ 10 
#if !defined(OTL_ANSI_CPP_11_NOEXCEPT)
#define OTL_ANSI_CPP_11_NOEXCEPT
#endif
#elif defined(_MSC_VER) && (_MSC_VER>=1700)
// VC++ 11 and higher
#if !defined(OTL_ANSI_CPP_11_NOEXCEPT)
#define OTL_ANSI_CPP_11_NOEXCEPT _NOEXCEPT
#endif
#elif defined(_MSC_VER)
#define OTL_ANSI_CPP_11_NOEXCEPT 
#else
#if !defined(OTL_ANSI_CPP_11_NOEXCEPT)
#define OTL_ANSI_CPP_11_NOEXCEPT_SUPPORT
#define OTL_ANSI_CPP_11_NOEXCEPT noexcept
#define OTL_ANSI_CPP_11_NOEXCEPT_FALSE noexcept(false)
#endif
#endif

#else

#define OTL_ANSI_CPP_11_NOEXCEPT
#if !defined(OTL_ANSI_CPP_11_NULLPTR_SUPPORT)
#if !defined(__cplusplus_cli)
// define nullptr as 0 only if C++/CLI is not used
#define nullptr 0
#endif
#endif
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning (disable:4351)
#pragma warning (disable:4290)
#define OTL_STRCAT_S(dest,dest_sz,src) strcat_s(dest,dest_sz,src)
#define OTL_STRCPY_S(dest,dest_sz,src) strcpy_s(dest,dest_sz,src)
#define OTL_STRNCPY_S(dest,dest_sz,src,count) strncpy_s(dest,dest_sz,src,count)
#else
#define OTL_STRCAT_S(dest,dest_sz,src) strcat(dest,src)
#define OTL_STRCPY_S(dest,dest_sz,src) strcpy(dest,src)
#define OTL_STRNCPY_S(dest,dest_sz,src,count) strncpy(dest,src,count)
#endif

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

//======================= CONFIGURATION #DEFINEs ===========================

// Uncomment the following line in order to include the OTL for ODBC:
//#define OTL_ODBC

// Uncomment the following line in order to include the OTL for
// MySQL/MyODBC for MyODBC 2.5 (pretty old). Otherwise, use OTL_ODBC
//#define OTL_ODBC_MYSQL

// Uncomment the following line in order to include the OTL for DB2 CLI:
//#define OTL_DB2_CLI

// Uncomment the following line in order to include the OTL for
// Oracle 7: 
//#define OTL_ORA7

// Uncomment the following line in order to include the OTL for
// Oracle 8:
//#define OTL_ORA8

// Uncomment the following line in order to include the OTL for
// Oracle 8i:
//#define OTL_ORA8I

// Uncomment the following line in order to include the OTL for
// Oracle 9i:
//#define OTL_ORA9I

// Uncomment the following line in order to include the OTL for
// Oracle 10g Release 1:
//#define OTL_ORA10G

// Uncomment the following line in order to include the OTL for
// Oracle 10g Release 2:
//#define OTL_ORA10G_R2

// Uncomment the following line in order to include the OTL for
// Oracle 11g Release 1
//#define OTL_ORA11G


// The macro definitions may be also turned on via C++ compiler command line
// option, e.g.: -DOTL_ODBC, -DOTL_ORA7, -DOTL_ORA8, -DOTL_ORA8I, -DOTL_ODBC_UNIX
// -DOTL_ODBC_MYSQL, -DOTL_DB2_CLI

// this becomes the default from version 4.0.162 and on.
// the #define is not enabled for vc++ 6.0 in version 4.0.167 and higher.
#if !defined(OTL_UNCAUGHT_EXCEPTION_ON) && !(defined(_MSC_VER)&&(_MSC_VER==1200))
#define OTL_UNCAUGHT_EXCEPTION_ON
#endif

#if !defined(OTL_TRACE_LEVEL)

#define OTL_TRACE_FORMAT_TZ_DATETIME(s)
#define OTL_TRACE_FORMAT_DATETIME(s)

#else

#if !defined(OTL_TRACE_FORMAT_DATETIME)

#define OTL_TRACE_FORMAT_TZ_DATETIME(s)                         \
s.month<<"/"<<s.day<<"/"<<s.year                                \
<<" "<<s.hour<<":"<<s.minute<<":"<<s.second<<"."<<s.fraction    \
<<" "<<s.tz_hour<<":"<<s.tz_minute

#define OTL_TRACE_FORMAT_DATETIME(s)                            \
s.month<<"/"<<s.day<<"/"<<s.year                                \
<<" "<<s.hour<<":"<<s.minute<<":"<<s.second<<"."<<s.fraction

#endif
#endif

#if defined(OTL_ORA11G)
#define OTL_ORA10G_R2
#if defined(OTL_UNICODE)
#define OTL_ORA_UNICODE_LONG_LENGTH_IN_BYTES
#endif
#endif

#if defined(OTL_ORA12C)
#define OTL_ORA11G_R2
#if defined(OTL_UNICODE)
#define OTL_ORA_UNICODE_LONG_LENGTH_IN_BYTES
#endif
#endif

#if defined(OTL_ORA11G_R2)
#define OTL_ORA10G_R2
#if defined(OTL_UNICODE) && !defined(OTL_ORA_UNICODE_LONG_LENGTH_IN_BYTES)
#define OTL_ORA_UNICODE_LONG_LENGTH_IN_BYTES
#endif
#endif

#if defined(OTL_STREAM_LEGACY_BUFFER_SIZE_TYPE)
typedef short int otl_stream_buffer_size_type;
#else
typedef int otl_stream_buffer_size_type;
#endif


#if defined(OTL_ODBC_MULTI_MODE)
#define OTL_ODBC
#define OTL_ODBC_SQL_EXTENDED_FETCH_ON
#endif

#if defined(OTL_ODBC_MSSQL_2005)
#define OTL_ODBC
#endif

#if defined(OTL_ODBC_MSSQL_2008)
#define OTL_ODBC
#define OTL_ODBC_MSSQL_2005
#endif

#if defined(OTL_IODBC_BSD)
#define OTL_ODBC
#define OTL_ODBC_UNIX
#endif

#if defined(OTL_ODBC_TIMESTEN_WIN)
#define OTL_ODBC_TIMESTEN
#define OTL_ODBC
#define OTL_ODBC_SQL_EXTENDED_FETCH_ON
#define ODBCVER 0x0250
#include <timesten.h>
#endif

#if defined(OTL_ODBC_TIMESTEN_UNIX)
#define OTL_ODBC_TIMESTEN
#define OTL_ODBC
#define OTL_ODBC_UNIX
#define OTL_ODBC_SQL_EXTENDED_FETCH_ON
#include <timesten.h>
#endif

#if defined(OTL_ODBC_ENTERPRISEDB)
#define OTL_ODBC_POSTGRESQL
#endif

#if defined(OTL_ODBC_POSTGRESQL)
#define OTL_ODBC
#endif

// Comment out this #define when using pre-ANSI C++ compiler
#if !defined(OTL_ODBC_zOS) && !defined (OTL_ANSI_CPP)
#define OTL_ANSI_CPP
#endif

#if defined(OTL_ODBC_zOS)
#define OTL_ODBC_UNIX
#define OTL_ODBC_SQL_EXTENDED_FETCH_ON
#endif

#if defined(OTL_ORA8I)
#define OTL_ORA8
#define OTL_ORA8_8I_REFCUR
#define OTL_ORA8_8I_DESC_COLUMN_SCALE
#endif

#if defined(OTL_ORA10G)||defined(OTL_ORA10G_R2)
#define  OTL_ORA9I
#define OTL_ORA_NATIVE_TYPES
#if defined(OTL_UNICODE)
#define OTL_ORA_UNICODE_LONG_LENGTH_IN_BYTES
#endif
#endif

#if defined(OTL_ORA9I)
#define OTL_ORA8
#define OTL_ORA8_8I_REFCUR
#define OTL_ORA8_8I_DESC_COLUMN_SCALE
#endif

#if defined(OTL_ODBC_MYSQL)
#define OTL_ODBC
#endif

#if defined(OTL_ODBC_XTG_IBASE6)
#define OTL_ODBC
#endif

#define OTL_VALUE_TEMPLATE
//#define OTL_ODBC_SQL_EXTENDED_FETCH_ON

#if defined(OTL_ODBC_UNIX) && !defined(OTL_ODBC)
#define OTL_ODBC
#endif

#if defined(OTL_ORA7) && defined(OTL_UBIGINT)
#error OTL_UBIGINT is not supported when OTL_ORA7 is defined
#endif

#if !defined(OTL_ORA11G_R2) && defined(OTL_ORA8) && defined(OTL_UBIGINT)
#error OTL_UBIGINT is only supported for OTL_ORA11G_R2 or higher
#endif

#if defined(OTL_UBIGINT) && defined(OTL_BIGINT_TO_STR) && \
    defined(OTL_STR_TO_BIGINT)
#error OTL_UBIGINT is not supported when OTL_BIGINT_TO_STR / \
OTL_STR_TO_BIGINT are defined
#endif

#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID) && \
    !defined(OTL_NUMERIC_TYPE_1) && !defined(OTL_STR_TO_NUMERIC_TYPE_1) &&  \
    !defined(OTL_NUMERIC_TYPE_1_TO_STR) && !defined(OTL_NUMERIC_TYPE_1_ID)
#error OTL_NUMERIC_TYPE_2 macros should be used after OTL_NUMERIC_TYPE_1 macros \
are already defined
#endif

#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID) && \
  ((!defined(OTL_NUMERIC_TYPE_1) && !defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    !defined(OTL_NUMERIC_TYPE_1_TO_STR) && !defined(OTL_NUMERIC_TYPE_1_ID)) || \
   (!defined(OTL_NUMERIC_TYPE_2) && !defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    !defined(OTL_NUMERIC_TYPE_2_TO_STR) && !defined(OTL_NUMERIC_TYPE_2_ID)))
#error OTL_NUMERIC_TYPE_3 macros should be used after OTL_NUMERIC_TYPE_1 and \
OTL_NUMERIC_TYPE_2 macros are already defined
#endif
 
#if defined(OTL_BIND_VAR_STRICT_TYPE_CHECKING_ON)
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
#define OTL_CHECK_BIND_VARS                     \
  if(strcmp(type_arr,"INT")==0||                \
     strcmp(type_arr,"UNSIGNED")==0||           \
     strcmp(type_arr,"SHORT")==0||              \
     strcmp(type_arr,"LONG")==0||               \
     strcmp(type_arr,"FLOAT")==0||              \
     strcmp(type_arr,"DOUBLE")==0||             \
     strcmp(type_arr,"BFLOAT")==0||             \
     strcmp(type_arr,"BDOUBLE")==0||            \
     strcmp(type_arr,"TIMESTAMP")==0||          \
     strcmp(type_arr,"TZ_TIMESTAMP")==0||       \
     strcmp(type_arr,"LTZ_TIMESTAMP")==0||      \
     strcmp(type_arr,"BIGINT")==0||             \
     strcmp(type_arr,"UBIGINT")==0||            \
     strcmp(type_arr,"CHAR")==0||               \
     strcmp(type_arr,"CHARZ")==0||              \
     strcmp(type_arr,"DB2DATE")==0||            \
     strcmp(type_arr,"DB2TIME")==0||            \
     strcmp(type_arr,"VARCHAR_LONG")==0||       \
     strcmp(type_arr,"RAW_LONG")==0||           \
     strcmp(type_arr,"RAW")==0||                \
     strcmp(type_arr,"CLOB")==0||               \
     strcmp(type_arr,"BLOB")==0||               \
     strcmp(type_arr,"NCHAR")==0||              \
     strcmp(type_arr,"NCLOB")==0||              \
     strcmp(type_arr,"REFCUR")==0||             \
     strcmp(type_arr,OTL_NUMERIC_TYPE_1_ID)==0||\
     strcmp(type_arr,OTL_NUMERIC_TYPE_2_ID)==0||\
     strcmp(type_arr,OTL_NUMERIC_TYPE_3_ID)==0) \
    ;                                           \
  else                                          \
    return 0;
#else
#define OTL_CHECK_BIND_VARS                     \
  if(strcmp(type_arr,"INT")==0||                \
     strcmp(type_arr,"UNSIGNED")==0||           \
     strcmp(type_arr,"SHORT")==0||              \
     strcmp(type_arr,"LONG")==0||               \
     strcmp(type_arr,"FLOAT")==0||              \
     strcmp(type_arr,"DOUBLE")==0||             \
     strcmp(type_arr,"BFLOAT")==0||             \
     strcmp(type_arr,"BDOUBLE")==0||            \
     strcmp(type_arr,"TIMESTAMP")==0||          \
     strcmp(type_arr,"TZ_TIMESTAMP")==0||       \
     strcmp(type_arr,"LTZ_TIMESTAMP")==0||      \
     strcmp(type_arr,"BIGINT")==0||             \
     strcmp(type_arr,"UBIGINT")==0||            \
     strcmp(type_arr,"CHAR")==0||               \
     strcmp(type_arr,"CHARZ")==0||              \
     strcmp(type_arr,"DB2DATE")==0||            \
     strcmp(type_arr,"DB2TIME")==0||            \
     strcmp(type_arr,"VARCHAR_LONG")==0||       \
     strcmp(type_arr,"RAW_LONG")==0||           \
     strcmp(type_arr,"RAW")==0||                \
     strcmp(type_arr,"CLOB")==0||               \
     strcmp(type_arr,"BLOB")==0||               \
     strcmp(type_arr,"NCHAR")==0||              \
     strcmp(type_arr,"NCLOB")==0||              \
     strcmp(type_arr,"REFCUR")==0||             \
     strcmp(type_arr,OTL_NUMERIC_TYPE_1_ID)==0||\
     strcmp(type_arr,OTL_NUMERIC_TYPE_2_ID)==0) \
    ;                                           \
  else                                          \
    return 0;
#endif
#else
#define OTL_CHECK_BIND_VARS                     \
  if(strcmp(type_arr,"INT")==0||                \
     strcmp(type_arr,"UNSIGNED")==0||           \
     strcmp(type_arr,"SHORT")==0||              \
     strcmp(type_arr,"LONG")==0||               \
     strcmp(type_arr,"FLOAT")==0||              \
     strcmp(type_arr,"DOUBLE")==0||             \
     strcmp(type_arr,"BFLOAT")==0||             \
     strcmp(type_arr,"BDOUBLE")==0||            \
     strcmp(type_arr,"TIMESTAMP")==0||          \
     strcmp(type_arr,"TZ_TIMESTAMP")==0||       \
     strcmp(type_arr,"LTZ_TIMESTAMP")==0||      \
     strcmp(type_arr,"BIGINT")==0||             \
     strcmp(type_arr,"UBIGINT")==0||             \
     strcmp(type_arr,"CHAR")==0||               \
     strcmp(type_arr,"CHARZ")==0||              \
     strcmp(type_arr,"DB2DATE")==0||            \
     strcmp(type_arr,"DB2TIME")==0||            \
     strcmp(type_arr,"VARCHAR_LONG")==0||       \
     strcmp(type_arr,"RAW_LONG")==0||           \
     strcmp(type_arr,"RAW")==0||                \
     strcmp(type_arr,"CLOB")==0||               \
     strcmp(type_arr,"BLOB")==0||               \
     strcmp(type_arr,"NCHAR")==0||              \
     strcmp(type_arr,"NCLOB")==0||              \
     strcmp(type_arr,"REFCUR")==0||             \
     strcmp(type_arr,OTL_NUMERIC_TYPE_1_ID)==0) \
    ;                                           \
  else                                          \
    return 0;
#endif
#else
#define OTL_CHECK_BIND_VARS                     \
  if(strcmp(type_arr,"INT")==0||                \
     strcmp(type_arr,"UNSIGNED")==0||           \
     strcmp(type_arr,"SHORT")==0||              \
     strcmp(type_arr,"LONG")==0||               \
     strcmp(type_arr,"FLOAT")==0||              \
     strcmp(type_arr,"DOUBLE")==0||             \
     strcmp(type_arr,"BFLOAT")==0||             \
     strcmp(type_arr,"BDOUBLE")==0||            \
     strcmp(type_arr,"TIMESTAMP")==0||          \
     strcmp(type_arr,"TZ_TIMESTAMP")==0||       \
     strcmp(type_arr,"LTZ_TIMESTAMP")==0||      \
     strcmp(type_arr,"BIGINT")==0||             \
     strcmp(type_arr,"UBIGINT")==0||            \
     strcmp(type_arr,"CHAR")==0||               \
     strcmp(type_arr,"CHARZ")==0||              \
     strcmp(type_arr,"DB2DATE")==0||            \
     strcmp(type_arr,"DB2TIME")==0||            \
     strcmp(type_arr,"VARCHAR_LONG")==0||       \
     strcmp(type_arr,"RAW_LONG")==0||           \
     strcmp(type_arr,"RAW")==0||                \
     strcmp(type_arr,"CLOB")==0||               \
     strcmp(type_arr,"BLOB")==0||               \
     strcmp(type_arr,"NCHAR")==0||              \
     strcmp(type_arr,"NCLOB")==0||              \
     strcmp(type_arr,"REFCUR")==0)              \
    ;                                           \
  else                                          \
    return 0;
#endif
#else
#define OTL_CHECK_BIND_VARS
#endif

// ------------------- Namespace generation ------------------------
#if defined(OTL_EXPLICIT_NAMESPACES)

#if defined(OTL_DB2_CLI)
#define OTL_ODBC_NAMESPACE_BEGIN namespace db2 {
#define OTL_ODBC_NAMESPACE_PREFIX db2::
#define OTL_ODBC_NAMESPACE_END }
#else
#define OTL_ODBC_NAMESPACE_BEGIN namespace odbc {
#define OTL_ODBC_NAMESPACE_PREFIX odbc::
#define OTL_ODBC_NAMESPACE_END }
#endif

#define OTL_ORA7_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA7_NAMESPACE_PREFIX oracle::
#define OTL_ORA7_NAMESPACE_END }

#define OTL_ORA8_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA8_NAMESPACE_PREFIX oracle::
#define OTL_ORA8_NAMESPACE_END }

#else

// Only one OTL is being intantiated
#if defined(OTL_ODBC)&&!defined(OTL_ORA8)&& \
    !defined(OTL_ORA7)&&!defined(OTL_DB2_CLI) \
 || !defined(OTL_ODBC)&&defined(OTL_ORA8)&& \
    !defined(OTL_ORA7)&&!defined(OTL_DB2_CLI) \
 || !defined(OTL_ODBC)&&!defined(OTL_ORA8)&& \
    defined(OTL_ORA7)&&!defined(OTL_DB2_CLI) \
 || !defined(OTL_ODBC)&&!defined(OTL_ORA8)&& \
    !defined(OTL_ORA7)&&defined(OTL_DB2_CLI)

#define OTL_ODBC_NAMESPACE_BEGIN
#define OTL_ODBC_NAMESPACE_PREFIX
#define OTL_ODBC_NAMESPACE_END

#define OTL_ORA7_NAMESPACE_BEGIN
#define OTL_ORA7_NAMESPACE_PREFIX
#define OTL_ORA7_NAMESPACE_END

#define OTL_ORA8_NAMESPACE_BEGIN
#define OTL_ORA8_NAMESPACE_PREFIX
#define OTL_ORA8_NAMESPACE_END

#endif

// ================ Combinations of two OTLs =========================
#if defined(OTL_ODBC) && defined(OTL_ORA7) && \
    !defined(OTL_ORA8) && !defined(OTL_DB2_CLI)

#define OTL_ODBC_NAMESPACE_BEGIN namespace odbc{
#define OTL_ODBC_NAMESPACE_PREFIX odbc::
#define OTL_ODBC_NAMESPACE_END }

#define OTL_ORA7_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA7_NAMESPACE_PREFIX oracle::
#define OTL_ORA7_NAMESPACE_END }

#define OTL_ORA8_NAMESPACE_BEGIN
#define OTL_ORA8_NAMESPACE_PREFIX
#define OTL_ORA8_NAMESPACE_END

#endif

#if defined(OTL_ODBC) && !defined(OTL_ORA7) && \
    defined(OTL_ORA8) && !defined(OTL_DB2_CLI)

#define OTL_ODBC_NAMESPACE_BEGIN namespace odbc{
#define OTL_ODBC_NAMESPACE_PREFIX odbc::
#define OTL_ODBC_NAMESPACE_END }

#define OTL_ORA8_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA8_NAMESPACE_PREFIX oracle::
#define OTL_ORA8_NAMESPACE_END }

#define OTL_ORA7_NAMESPACE_BEGIN
#define OTL_ORA7_NAMESPACE_PREFIX
#define OTL_ORA7_NAMESPACE_END


#endif

#if !defined(OTL_ODBC) && defined(OTL_ORA7) && \
    !defined(OTL_ORA8) && defined(OTL_DB2_CLI)

#define OTL_ORA7_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA7_NAMESPACE_PREFIX oracle::
#define OTL_ORA7_NAMESPACE_END }

#define OTL_ORA8_NAMESPACE_BEGIN
#define OTL_ORA8_NAMESPACE_PREFIX
#define OTL_ORA8_NAMESPACE_END

#define OTL_ODBC_NAMESPACE_BEGIN namespace db2 {
#define OTL_ODBC_NAMESPACE_PREFIX db2::
#define OTL_ODBC_NAMESPACE_END }


#endif

#if !defined(OTL_ODBC) && !defined(OTL_ORA7) && \
    defined(OTL_ORA8) && defined(OTL_DB2_CLI)

#define OTL_ORA8_NAMESPACE_BEGIN namespace oracle {
#define OTL_ORA8_NAMESPACE_PREFIX oracle::
#define OTL_ORA8_NAMESPACE_END }

#define OTL_ORA7_NAMESPACE_BEGIN
#define OTL_ORA7_NAMESPACE_PREFIX
#define OTL_ORA7_NAMESPACE_END

#define OTL_ODBC_NAMESPACE_BEGIN namespace db2 {
#define OTL_ODBC_NAMESPACE_PREFIX db2::
#define OTL_ODBC_NAMESPACE_END }


#endif

#endif

// -------------------- End of namespace generation -------------------

// --------------------- Invalid combinations --------------------------

#if (defined(OTL_ORA7_TIMESTAMP_TO_STRING)||defined(OTL_ORA7_STRING_TO_TIMESTAMP)) \
  && defined(OTL_ORA_TIMESTAMP)
#error Invalid combination: OTL_ORA_TIMESTAMP and \
OTL_ORA7_TIMESTAMP_TO_STRING/OTL_ORA7_STRING_TO_TIMESTAMP
#endif

#if defined(OTL_ORA_MAP_BIGINT_TO_LONG) && \
    defined(OTL_BIGINT_TO_STR) && \
    defined(OTL_STR_TO_BIGINT)
#error OTL_ORA_MAP_BIGINT_TO_LONG cannot be used when \
OTL_BIGINT_TO_STR and OTL_STR_TO_BIGINT are defined
#endif

#if defined(OTL_STL) && defined(OTL_UNICODE_STRING_TYPE)
#error Invalid combination: OTL_STL and OTL_UNICODE_STRING_TYPE
#endif

#if defined(OTL_ORA_UTF8) && !defined(OTL_ORA10G) && \
    !defined(OTL_ORA_10G_R2) && !defined(OTL_ORA9I)
#error Invalid combination: OTL_ORA_UTF8 can only be used with OTL_ORA9I or higher
#endif

#if defined(OTL_ORA_UTF8) && defined(OTL_UNICODE)
#error Invalid combination: OTL_ORA_UTF8 and OTL_UNICODE are mutually exclusive
#endif

#if defined(OTL_ODBC) && defined(OTL_DB2_CLI)
#error Invalid combination: OTL_ODBC && OTL_DB2_CLI together
#endif

#if defined (OTL_ORA7) && defined(OTL_ORA8)
#error Invalid combination: OTL_ORA7 && OTL_ORA8(I) together
#endif

#if defined(OTL_ORA_OCI_ENV_CREATE) && \
    (!defined(OTL_ORA8I) && !defined(OTL_ORA9I) && \
     !defined(OTL_ORA10G) && !defined(OTL_ORA10G_R2))
#error OTL_ORA_OCI_ENV_CREATE can be only defined when OTL_ORA8I, OTL_ORA9I, OTL_ORA10G, OTL_ORA10G_R2, or OTL_ORA11G is defined
#endif
// --------------------------------------------------------------------

#if defined(OTL_TRACE_LEVEL)

#if !defined(OTL_TRACE_LINE_PREFIX)
#define OTL_TRACE_LINE_PREFIX "OTL TRACE ==> "
#endif

#if defined(OTL_UNICODE_CHAR_TYPE) && !defined(OTL_UNICODE)
#error OTL_UNICODE needs to be defined if OTL_UNICODE_CHAR_TYPE is defined
#endif

#if defined(OTL_UNICODE_STRING_TYPE) && !defined(OTL_UNICODE_CHAR_TYPE)
#error OTL_UNICODE_CHAR_TYPE needs to be defined if OTL_UNICODE_STRING_TYPE is defined
#endif

#if defined(OTL_UNICODE_STRING_TYPE) && !defined(OTL_UNICODE_CHAR_TYPE)
#error OTL_UNICODE_CHAR_TYPE needs to be defined if OTL_UNICODE_STRING_TYPE is defined
#endif

#if defined(OTL_UNICODE_STRING_TYPE) && !defined(OTL_UNICODE)
#error OTL_UNICODE needs to be defined if OTL_UNICODE_STRING_TYPE is defined
#endif

#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON) && !defined(OTL_UNICODE_CHAR_TYPE)
#error OTL_UNICODE_CHAR_TYPE needs to be defined if OTL_UNICODE_EXCEPTION_AND_RLOGON is defined
#endif


#if !defined(OTL_TRACE_LINE_SUFFIX)
#if defined(OTL_UNICODE)
#define OTL_TRACE_LINE_SUFFIX L"\n"
#else
#define OTL_TRACE_LINE_SUFFIX "\n"
#endif
#endif

#if !defined(OTL_TRACE_STREAM_OPEN)
#define OTL_TRACE_STREAM_OPEN                   \
  if(OTL_TRACE_LEVEL & 0x4){                    \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;    \
    OTL_TRACE_STREAM<<"otl_stream(this=";       \
    OTL_TRACE_STREAM<<OTL_RCAST(void*,this);    \
    OTL_TRACE_STREAM<<")::open(buffer_size=";   \
    OTL_TRACE_STREAM<<arr_size;                 \
    OTL_TRACE_STREAM<<", sqlstm=";              \
    OTL_TRACE_STREAM<<sqlstm;                   \
    OTL_TRACE_STREAM<<", connect=";             \
    OTL_TRACE_STREAM<<OTL_RCAST(void*,&db);     \
    OTL_TRACE_STREAM<<", implicit_select=";     \
    OTL_TRACE_STREAM<<implicit_select;          \
    if(sqlstm_label){                           \
      OTL_TRACE_STREAM<<", label=";             \
      OTL_TRACE_STREAM<<sqlstm_label;           \
    }                                           \
    OTL_TRACE_STREAM<<");";                     \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;    \
  }
#endif

#if !defined(OTL_TRACE_STREAM_OPEN2)
#define OTL_TRACE_STREAM_OPEN2                          \
  if(OTL_TRACE_LEVEL & 0x4){                            \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;            \
    OTL_TRACE_STREAM<<"otl_stream(this=";               \
    OTL_TRACE_STREAM<<OTL_RCAST(void*,this);            \
    OTL_TRACE_STREAM<<")::open(buffer_size=";           \
    OTL_TRACE_STREAM<<arr_size;                         \
    OTL_TRACE_STREAM<<", sqlstm=";                      \
    OTL_TRACE_STREAM<<sqlstm;                           \
    OTL_TRACE_STREAM<<", connect=";                     \
    OTL_TRACE_STREAM<<OTL_RCAST(void*,&db);             \
    if(ref_cur_placeholder){                            \
      OTL_TRACE_STREAM<<", ref_cur_placeholder=";       \
      OTL_TRACE_STREAM<<ref_cur_placeholder;            \
    }                                                   \
    if(sqlstm_label){                                   \
      OTL_TRACE_STREAM<<", label=";                     \
      OTL_TRACE_STREAM<<sqlstm_label;                   \
    }                                                   \
    OTL_TRACE_STREAM<<");";                             \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;            \
  }
#endif

#if !defined(OTL_TRACE_DIRECT_EXEC)
#define OTL_TRACE_DIRECT_EXEC                             \
  if(OTL_TRACE_LEVEL & 0x2){                              \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;              \
    OTL_TRACE_STREAM<<"otl_cursor::direct_exec(connect="; \
    OTL_TRACE_STREAM<<OTL_RCAST(void*,&connect);          \
    OTL_TRACE_STREAM<<",sqlstm=\"";                       \
    OTL_TRACE_STREAM<<sqlstm;                             \
    OTL_TRACE_STREAM<<"\",exception_enabled=";            \
    OTL_TRACE_STREAM<<exception_enabled;                  \
    OTL_TRACE_STREAM<<");";                               \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;              \
  }
#endif

#if !defined(OTL_TRACE_SYNTAX_CHECK)
#define OTL_TRACE_SYNTAX_CHECK                             \
  if(OTL_TRACE_LEVEL & 0x2){                               \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;               \
    OTL_TRACE_STREAM<<"otl_cursor::syntax_check(connect="; \
    OTL_TRACE_STREAM<<OTL_RCAST(void*,&connect);           \
    OTL_TRACE_STREAM<<",sqlstm=\"";                        \
    OTL_TRACE_STREAM<<sqlstm;                              \
    OTL_TRACE_STREAM<<"\"";                                \
    OTL_TRACE_STREAM<<");";                                \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;               \
  }
#endif

#if !defined(OTL_TRACE_FUNC)
#define OTL_TRACE_FUNC(level,class_name,func_name,args) \
  if(OTL_TRACE_LEVEL & level){                          \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;            \
    OTL_TRACE_STREAM<<class_name;                       \
    OTL_TRACE_STREAM<<"(this=";                         \
    OTL_TRACE_STREAM<<OTL_RCAST(void*,this);            \
    OTL_TRACE_STREAM<<")::" func_name "(";              \
    OTL_TRACE_STREAM<<args;                             \
    OTL_TRACE_STREAM<<");";                             \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;            \
  }
#endif

#if !defined(OTL_TRACE_EXCEPTION)
#define OTL_TRACE_EXCEPTION(code,msg,stm_text,var_info) \
  if(OTL_TRACE_LEVEL & 0x20){                           \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;            \
    OTL_TRACE_STREAM<<"otl_exception, code=";           \
    OTL_TRACE_STREAM<<code;                             \
    OTL_TRACE_STREAM<<", msg=";                         \
    char* c=OTL_RCAST(char*,msg);                       \
    while(*c && *c!='\n'){                              \
      OTL_TRACE_STREAM<<*c;                             \
      ++c;                                              \
    }                                                   \
    OTL_TRACE_STREAM<<", stm_text=";                    \
    OTL_TRACE_STREAM<<stm_text;                         \
    OTL_TRACE_STREAM<<", var_info=";                    \
    OTL_TRACE_STREAM<<var_info;                         \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;            \
  }
#endif

#if !defined(OTL_TRACE_EXCEPTION2)
#define OTL_TRACE_EXCEPTION2(code,msg,stm_text,var_info,\
                             input_str,input_str_size)  \
  if(OTL_TRACE_LEVEL & 0x20){                           \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;            \
    OTL_TRACE_STREAM<<"otl_exception, code=";           \
    OTL_TRACE_STREAM<<code;                             \
    OTL_TRACE_STREAM<<", msg=";                         \
    char* c=OTL_RCAST(char*,msg);                       \
    while(*c && *c!='\n'){                              \
      OTL_TRACE_STREAM<<*c;                             \
      ++c;                                              \
    }                                                   \
    OTL_TRACE_STREAM<<", stm_text=";                    \
    OTL_TRACE_STREAM<<stm_text;                         \
    OTL_TRACE_STREAM<<", var_info=";                    \
    OTL_TRACE_STREAM<<var_info;                         \
    OTL_TRACE_STREAM<<", input string=\"";              \
    const char* str=OTL_RCAST(const char*,input_str);   \
    int i=0;                                            \
    while(i<input_str_size)                             \
      OTL_TRACE_STREAM<<str[i++];                       \
    OTL_TRACE_STREAM<<"\""<<OTL_TRACE_LINE_SUFFIX;      \
  }
#endif

#if !defined(OTL_TRACE_RLOGON_ORA7) && defined(OTL_ORA7)
#define OTL_TRACE_RLOGON_ORA7(level,class_name,func_name,       \
                              connect_str,auto_commit)          \
  if(OTL_TRACE_LEVEL & level){                                  \
    char temp_connect_str[2048];                                \
    const char* c1=OTL_RCAST(const char*,connect_str);          \
    char* c2=temp_connect_str;                                  \
    while(*c1 && *c1!='/'){                                     \
      *c2=*c1;                                                  \
      ++c1; ++c2;                                               \
    }                                                           \
    if(*c1=='/'){                                               \
      *c2=*c1;                                                  \
      ++c1; ++c2;                                               \
      while(*c1 && *c1!='@'){                                   \
        *c2='*';                                                \
        ++c1; ++c2;                                             \
      }                                                         \
      if(*c1=='@'){                                             \
        while(*c1){                                             \
          *c2=*c1;                                              \
          ++c1; ++c2;                                           \
        }                                                       \
      }                                                         \
    }                                                           \
    *c2=0;                                                      \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                    \
    OTL_TRACE_STREAM<<class_name;                               \
    OTL_TRACE_STREAM<<"(this=";                                 \
    OTL_TRACE_STREAM<<OTL_RCAST(void*,this);                    \
    OTL_TRACE_STREAM<<")::" func_name "(";                      \
    OTL_TRACE_STREAM<<"connect_str=\"";                         \
    OTL_TRACE_STREAM<<temp_connect_str;                         \
    OTL_TRACE_STREAM<<"\", auto_commit=";                       \
    OTL_TRACE_STREAM<<auto_commit;                              \
    OTL_TRACE_STREAM<<");";                                     \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                    \
  }
#endif

#if !defined(OTL_TRACE_RLOGON_ORA8) && defined(OTL_ORA8)
#define OTL_TRACE_RLOGON_ORA8(level,class_name,func_name,               \
                              tnsname,userid,passwd,                    \
                              auto_commit)                              \
  if(OTL_TRACE_LEVEL & level){                                          \
    char temp_connect_str[2048];                                        \
    OTL_STRCPY_S(temp_connect_str,sizeof(temp_connect_str),userid);     \
    OTL_STRCAT_S(temp_connect_str,sizeof(temp_connect_str),"/");        \
    size_t sz=strlen(passwd);                                           \
    for(size_t i=0;i<sz;++i)                                            \
      OTL_STRCAT_S(temp_connect_str,sizeof(temp_connect_str),"*");      \
    size_t tns_sz=strlen(tnsname);                                      \
    if(tns_sz>0){                                                       \
      OTL_STRCAT_S(temp_connect_str,sizeof(temp_connect_str),"@");      \
      OTL_STRCAT_S(temp_connect_str,sizeof(temp_connect_str),tnsname);  \
    }                                                                   \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                            \
    OTL_TRACE_STREAM<<class_name<<"(this=";                             \
    OTL_TRACE_STREAM<<OTL_RCAST(void*,this);                            \
    OTL_TRACE_STREAM<<")::" func_name "(";                              \
    OTL_TRACE_STREAM<<"connect_str=\"";                                 \
    OTL_TRACE_STREAM<<temp_connect_str;                                 \
    OTL_TRACE_STREAM<<"\", auto_commit=";                               \
    OTL_TRACE_STREAM<<auto_commit <<");";                               \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                            \
  }
#endif

#if !defined(OTL_TRACE_RLOGON_ODBC)&&(defined(OTL_ODBC)||defined(OTL_DB2_CLI))
#define OTL_TRACE_RLOGON_ODBC(level,class_name,func_name,                       \
                              tnsname,userid,passwd,                            \
                              auto_commit)                                      \
  if(OTL_TRACE_LEVEL & level){                                                  \
    char temp_connect_str2[2048];                                               \
    OTL_STRCPY_S(temp_connect_str2,sizeof(temp_connect_str2),userid);           \
    OTL_STRCAT_S(temp_connect_str2,sizeof(temp_connect_str2),"/");              \
    size_t sz=strlen(passwd);                                                   \
    for(size_t i=0;i<sz;++i)                                                    \
      OTL_STRCAT_S(temp_connect_str2,sizeof(temp_connect_str2),"*");            \
    size_t tns_sz=strlen(tnsname);                                              \
    if(tns_sz>0){                                                               \
      OTL_STRCAT_S(temp_connect_str2,sizeof(temp_connect_str2),"@");            \
      OTL_STRCAT_S(temp_connect_str2,sizeof(temp_connect_str2),tnsname);        \
    }                                                                           \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                                    \
    OTL_TRACE_STREAM<<class_name;                                               \
    OTL_TRACE_STREAM<<"(this=";                                                 \
    OTL_TRACE_STREAM<<OTL_RCAST(void*,this);                                    \
    OTL_TRACE_STREAM<<")::" func_name "(";                                      \
    OTL_TRACE_STREAM<<"connect_str=\"";                                         \
    OTL_TRACE_STREAM<<temp_connect_str2;                                        \
    OTL_TRACE_STREAM<<"\", auto_commit=";                                       \
    OTL_TRACE_STREAM<<auto_commit;                                              \
    OTL_TRACE_STREAM<<");";                                                     \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                                    \
  }
#endif

#if !defined(OTL_TRACE_RLOGON_ODBC_W)&& \
    (defined(OTL_ODBC)||defined(OTL_DB2_CLI))
#define OTL_TRACE_RLOGON_ODBC_W(level,class_name,func_name,     \
                              tnsname,userid,passwd,            \
                              auto_commit)                      \
  if(OTL_TRACE_LEVEL & level){                                  \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                    \
    OTL_TRACE_STREAM<<class_name;                               \
    OTL_TRACE_STREAM<<L"(this=";                                \
    OTL_TRACE_STREAM<<OTL_RCAST(void*,this);                    \
    OTL_TRACE_STREAM<<L")::" func_name L"(";                    \
    OTL_TRACE_STREAM<<L"connect_str=\"";                        \
    OTL_TRACE_STREAM<<userid<<L"/***@"<<tnsname;                \
    OTL_TRACE_STREAM<<L"\", auto_commit=";                      \
    OTL_TRACE_STREAM<<auto_commit;                              \
    OTL_TRACE_STREAM<<L");";                                    \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                    \
  }
#endif

#if !defined(OTL_TRACE_FIRST_FETCH)
#if defined(OTL_TRACE_ENABLE_STREAM_LABELS)
#define OTL_TRACE_FIRST_FETCH                                                   \
  if(OTL_TRACE_LEVEL & 0x8){                                                    \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                                    \
    OTL_TRACE_STREAM<<"otl_stream(this=";                                       \
    OTL_TRACE_STREAM<<this->master_stream_ptr_;                                 \
    OTL_TRACE_STREAM<<"), ";                                                    \
    OTL_TRACE_STREAM<<"fetched the first batch of rows, SQL Stm=";              \
    if(this->stm_label)                                                         \
      OTL_TRACE_STREAM<<this->stm_label;                                        \
    else                                                                        \
      OTL_TRACE_STREAM<<this->stm_text;                                         \
    OTL_TRACE_STREAM<<", RPC=";                                                 \
    OTL_TRACE_STREAM<<row_count;                                                \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                                    \
  }
#else
#define OTL_TRACE_FIRST_FETCH                                                   \
  if(OTL_TRACE_LEVEL & 0x8){                                                    \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                                    \
    OTL_TRACE_STREAM<<"otl_stream(this=";                                       \
    OTL_TRACE_STREAM<<this->master_stream_ptr_;                                 \
    OTL_TRACE_STREAM<<"), ";                                                    \
    OTL_TRACE_STREAM<<"fetched the first batch of rows, SQL Stm=";              \
    OTL_TRACE_STREAM<<this->stm_text;                                           \
    OTL_TRACE_STREAM<<", RPC=";                                                 \
    OTL_TRACE_STREAM<<row_count;                                                \
    OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                                    \
  }
#endif
#endif

#if !defined(OTL_TRACE_NEXT_FETCH)
#if defined(OTL_TRACE_ENABLE_STREAM_LABELS)
#define OTL_TRACE_NEXT_FETCH                                                    \
   if(OTL_TRACE_LEVEL & 0x8 && cur_row==0){                                     \
     OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                                   \
     OTL_TRACE_STREAM<<"otl_stream(this=";                                      \
     OTL_TRACE_STREAM<<this->master_stream_ptr_;                                \
     OTL_TRACE_STREAM<<"), ";                                                   \
     OTL_TRACE_STREAM<<"fetched the next batch of rows, SQL Stm=" ;             \
     if(this->stm_label)                                                        \
       OTL_TRACE_STREAM<<this->stm_label;                                       \
     else                                                                       \
       OTL_TRACE_STREAM<<this->stm_text;                                        \
     OTL_TRACE_STREAM<<", RPC=";                                                \
     OTL_TRACE_STREAM<<row_count;                                               \
     OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                                   \
   }
#define OTL_TRACE_NEXT_FETCH2                                                   \
   if(OTL_TRACE_LEVEL & 0x8 && cur_row==1){                                     \
     OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                                   \
     OTL_TRACE_STREAM<<"otl_stream(this=";                                      \
     OTL_TRACE_STREAM<<this->master_stream_ptr_;                                \
     OTL_TRACE_STREAM<<"), ";                                                   \
     OTL_TRACE_STREAM<<"fetched the next batch of rows, SQL Stm=" ;             \
     if(this->stm_label)                                                        \
       OTL_TRACE_STREAM<<this->stm_label;                                       \
     else                                                                       \
       OTL_TRACE_STREAM<<this->stm_text;                                        \
     OTL_TRACE_STREAM<<", RPC=";                                                \
     OTL_TRACE_STREAM<<row_count;                                               \
     OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                                   \
   }
#else
#define OTL_TRACE_NEXT_FETCH                                                    \
   if(OTL_TRACE_LEVEL & 0x8 && cur_row==0){                                     \
     OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                                   \
     OTL_TRACE_STREAM<<"otl_stream(this=";                                      \
     OTL_TRACE_STREAM<<this->master_stream_ptr_;                                \
     OTL_TRACE_STREAM<<"), ";                                                   \
     OTL_TRACE_STREAM<<"fetched the next batch of rows, SQL Stm=" ;             \
     OTL_TRACE_STREAM<<this->stm_text;                                          \
     OTL_TRACE_STREAM<<", RPC=";                                                \
     OTL_TRACE_STREAM<<row_count;                                               \
     OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                                   \
   }
#define OTL_TRACE_NEXT_FETCH2                                                   \
   if(OTL_TRACE_LEVEL & 0x8 && cur_row==1){                                     \
     OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                                   \
     OTL_TRACE_STREAM<<"otl_stream(this=";                                      \
     OTL_TRACE_STREAM<<this->master_stream_ptr_;                                \
     OTL_TRACE_STREAM<<"), ";                                                   \
     OTL_TRACE_STREAM<<"fetched the next batch of rows, SQL Stm=" ;             \
     OTL_TRACE_STREAM<<this->stm_text;                                          \
     OTL_TRACE_STREAM<<", RPC=";                                                \
     OTL_TRACE_STREAM<<row_count;                                               \
     OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                                   \
   }
#endif
#endif

#if !defined(OTL_TRACE_STREAM_EXECUTION)
#if defined(OTL_TRACE_ENABLE_STREAM_LABELS)
#define OTL_TRACE_STREAM_EXECUTION                              \
   if(OTL_TRACE_LEVEL & 0x8){                                   \
     OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                   \
     OTL_TRACE_STREAM<<"otl_stream(this=";                      \
     OTL_TRACE_STREAM<<override_->get_master_stream_ptr();      \
     OTL_TRACE_STREAM<<"), executing SQL Stm=";                 \
     if(this->stm_label)                                        \
       OTL_TRACE_STREAM<<this->stm_label;                       \
     else                                                       \
       OTL_TRACE_STREAM<<this->stm_text;                        \
     OTL_TRACE_STREAM<<", buffer size=";                        \
     OTL_TRACE_STREAM<<this->array_size;                        \
     OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                   \
   }
#else
#define OTL_TRACE_STREAM_EXECUTION                              \
   if(OTL_TRACE_LEVEL & 0x8){                                   \
     OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                   \
     OTL_TRACE_STREAM<<"otl_stream(this=";                      \
     OTL_TRACE_STREAM<<override_->get_master_stream_ptr();       \
     OTL_TRACE_STREAM<<"), executing SQL Stm=";                 \
     OTL_TRACE_STREAM<<this->stm_text;                          \
     OTL_TRACE_STREAM<<", buffer size=";                        \
     OTL_TRACE_STREAM<<this->array_size;                        \
     OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                   \
   }
#endif
#endif

#if !defined(OTL_TRACE_STREAM_EXECUTION2)
#if defined(OTL_TRACE_ENABLE_STREAM_LABELS)
#define OTL_TRACE_STREAM_EXECUTION2                             \
    if(OTL_TRACE_LEVEL & 0x8){                                  \
      OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                  \
      OTL_TRACE_STREAM<<"otl_stream(this=";                     \
      OTL_TRACE_STREAM<<this->master_stream_ptr_;               \
      OTL_TRACE_STREAM<<"), executing SQL Stm=";                \
     if(this->stm_label)                                        \
       OTL_TRACE_STREAM<<this->stm_label;                       \
     else                                                       \
       OTL_TRACE_STREAM<<this->stm_text;                        \
      OTL_TRACE_STREAM<<", current batch size=";                \
      OTL_TRACE_STREAM<<(cur_y+1);                              \
      OTL_TRACE_STREAM<<", row offset=";                        \
      OTL_TRACE_STREAM<<rowoff;                                 \
      OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                  \
    }
#else
#define OTL_TRACE_STREAM_EXECUTION2                             \
    if(OTL_TRACE_LEVEL & 0x8){                                  \
      OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                  \
      OTL_TRACE_STREAM<<"otl_stream(this=";                     \
      OTL_TRACE_STREAM<<this->master_stream_ptr_;               \
      OTL_TRACE_STREAM<<"), executing SQL Stm=";                \
      OTL_TRACE_STREAM<<this->stm_text;                         \
      OTL_TRACE_STREAM<<", current batch size=";                \
      OTL_TRACE_STREAM<<(cur_y+1);                              \
      OTL_TRACE_STREAM<<", row offset=";                        \
      OTL_TRACE_STREAM<<rowoff;                                 \
      OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                  \
    }
#endif
#endif

#if !defined(OTL_TRACE_READ)

#define OTL_TRACE_READ(val,function,type)                       \
  if(OTL_TRACE_LEVEL & 0x10){                                   \
    otl_var_desc* temp_vdesc=describe_next_in_var();            \
    if(temp_vdesc){                                             \
      OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                  \
      OTL_TRACE_STREAM<<"otl_stream(this=";                     \
      OTL_TRACE_STREAM<<OTL_RCAST(void*,this);                  \
      OTL_TRACE_STREAM<<")::" function "(" type ": ";           \
      OTL_TRACE_STREAM<<"ftype=";                               \
      OTL_TRACE_STREAM<<temp_vdesc->ftype;                      \
      OTL_TRACE_STREAM<<", placeholder=";                       \
      OTL_TRACE_STREAM<<temp_vdesc->name;                       \
      OTL_TRACE_STREAM<<", value=";                             \
      OTL_TRACE_STREAM<<val;                                    \
      OTL_TRACE_STREAM <<");";                                  \
      OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                  \
    }else{                                                      \
      OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                  \
      OTL_TRACE_STREAM<<"loading superfluous input variable";   \
      OTL_TRACE_STREAM<<"::" function "(" type ": ";            \
      OTL_TRACE_STREAM<<", value=";                             \
      OTL_TRACE_STREAM<<val;                                    \
      OTL_TRACE_STREAM<<");";                                   \
      OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                  \
    }                                                           \
  }
#endif

#if !defined(OTL_TRACE_WRITE)
#define OTL_TRACE_WRITE(val,function,type)                      \
   if(OTL_TRACE_LEVEL & 0x10){                                  \
     otl_var_desc* temp_vdesc=describe_next_out_var();          \
     if (temp_vdesc){                                           \
       OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                 \
       OTL_TRACE_STREAM<<"otl_stream(this=";                    \
       OTL_TRACE_STREAM<<OTL_RCAST(void*,this);                 \
       OTL_TRACE_STREAM<<")::" function "(" type " : ";         \
       OTL_TRACE_STREAM<<"ftype=";                              \
       OTL_TRACE_STREAM<<temp_vdesc->ftype;                     \
       OTL_TRACE_STREAM<<", placeholder=";                      \
       OTL_TRACE_STREAM<<temp_vdesc->name;                      \
       OTL_TRACE_STREAM<<", value=";                            \
       if(this->is_null())                                      \
         OTL_TRACE_STREAM<<"NULL";                              \
       else                                                     \
         OTL_TRACE_STREAM<<val;                                 \
       OTL_TRACE_STREAM<<");";                                  \
       OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                 \
     }else{                                                     \
       OTL_TRACE_STREAM<<OTL_TRACE_LINE_PREFIX;                 \
       OTL_TRACE_STREAM<<"writing superfluous output variable"; \
       OTL_TRACE_STREAM<<"::" function "(" type " : ";          \
       OTL_TRACE_STREAM<<", value=";                            \
       if(this->is_null())                                      \
         OTL_TRACE_STREAM<<"NULL";                              \
       else                                                     \
         OTL_TRACE_STREAM<<val;                                 \
       OTL_TRACE_STREAM<<");";                                  \
       OTL_TRACE_STREAM<<OTL_TRACE_LINE_SUFFIX;                 \
     }                                                          \
   }
#endif

#else

#define OTL_TRACE_LINE_PREFIX
#define OTL_TRACE_LINE_SUFFIX
#define OTL_TRACE_DIRECT_EXEC
#define OTL_TRACE_SYNTAX_CHECK
#define OTL_TRACE_FUNC(level,class_name,func_name,args)
#define OTL_TRACE_EXCEPTION(code,msg,stm_text,var_info)
#define OTL_TRACE_EXCEPTION2(code,msg,stm_text,var_info,input_str,inputs_str_size)
#define OTL_TRACE_RLOGON_ORA7(level,class_name,func_name, \
                              connect_str,auto_commit)
#define OTL_TRACE_RLOGON_ORA8(level,class_name,func_name,       \
                              tnsname,userid,passwd,            \
                              auto_commit)
#define OTL_TRACE_RLOGON_ODBC(level,class_name,func_name,       \
                              tnsname,userid,passwd,            \
                              auto_commit)
#define OTL_TRACE_STREAM_OPEN
#define OTL_TRACE_STREAM_OPEN2
#define OTL_TRACE_FIRST_FETCH
#define OTL_TRACE_NEXT_FETCH
#define OTL_TRACE_NEXT_FETCH2
#define OTL_TRACE_STREAM_EXECUTION
#define OTL_TRACE_STREAM_EXECUTION2
#define OTL_TRACE_WRITE(val,function,type)
#define OTL_TRACE_READ(val,function,type)
#define OTL_TRACE_RLOGON_ODBC_W(level,class_name,func_name, \
                              tnsname,userid,passwd,        \
                              auto_commit)
#endif

#if defined(OTL_DB2_CLI)
#define OTL_ODBC
#endif

#if defined(OTL_UNICODE)

#if !defined(OTL_UNICODE_CHAR_TYPE)
#define OTL_UNICODE_CHAR_TYPE wchar_t
#endif

#if !defined(OTL_UNICODE_CHAR_TYPE_TRACE_NAME)
#define OTL_UNICODE_CHAR_TYPE_TRACE_NAME "wchar_t"
#endif

#endif

#if defined(OTL_UNICODE)&& \
    (defined(OTL_ORA8I)|| \
     defined(OTL_ORA9I)|| \
     defined(OTL_ORA10G)||defined(OTL_ORA10G_R2))
#define OTL_ORA_UNICODE
#endif

#if defined(OTL_UNICODE)

#if (defined(OTL_ORA8I)||defined(OTL_ORA9I)||defined(OTL_ORA10G)||defined(OTL_ORA10G_R2)) && defined(OTL_ODBC)
#error OTL_UNICODE is not supported when both OTL_ORAxx and OTL_ODBC/OTL_DB2_CLI are defined
#endif

#if defined(OTL_ORA8I)||defined(OTL_ORA9I)||defined(OTL_ORA10G)||defined(OTL_ORA10G_R2)

#define OTL_CHAR unsigned short
#define OTL_UNICODE_CHAR OTL_CHAR
#define OTL_WCHAR unsigned long

#if defined(OTL_ORA8I) && !defined(OTL_ORA9I) && !defined(OTL_ORA10G) \
  && !defined(OTL_ORA10G_R2) && !defined(OTL_ORA11G) && defined(OTL_ORA11G_R2)
#define OTL_UNICODE_ID OCI_UCS2ID
#endif

#if defined(OTL_ORA9I)
#define OTL_UNICODE_ID OCI_UTF16ID
#endif

#elif defined(OTL_ODBC)

#define OTL_CHAR unsigned short
#define OTL_UNICODE_CHAR OTL_CHAR
#define OTL_WCHAR OTL_CHAR

#endif

#else

#define OTL_CHAR unsigned char

#endif


#if defined(OTL_ORA7) || defined(OTL_ORA8)
#define OTL_PL_TAB
#endif

const int otl_short_int_max=32760;

const int otl_odbc_adapter=1;
const int otl_ora7_adapter=2;
const int otl_ora8_adapter=3;

const int otl_inout_binding=1;
const int otl_select_binding=2;

const int otl_unsupported_type=-10000;

#if defined(OTL_ANSI_CPP)

#define OTL_SCAST(_t,_e) static_cast<_t >(_e)
#define OTL_RCAST(_t,_e) reinterpret_cast<_t >(_e)
#define OTL_DCAST(_t,_e) dynamic_cast<_t >(_e)
#define OTL_CCAST(_t,_e) const_cast<_t >(_e)

#define OTL_CONST_EXCEPTION const

#if defined(OTL_FUNC_THROW_SPEC_ON)
#define OTL_THROWS_OTL_EXCEPTION throw(otl_exception)
#if defined(OTL_ANSI_CPP_11_NOEXCEPT_SUPPORT)
#define OTL_NO_THROW OTL_ANSI_CPP_11_NOEXCEPT
#else
#define OTL_NO_THROW throw()
#endif
#else

#if defined(OTL_ANSI_CPP_11_NOEXCEPT_FALSE)
#define OTL_THROWS_OTL_EXCEPTION OTL_ANSI_CPP_11_NOEXCEPT_FALSE
#else
#define OTL_THROWS_OTL_EXCEPTION
#endif

#if defined(OTL_ANSI_CPP_11_NOEXCEPT_SUPPORT)
#define OTL_NO_THROW OTL_ANSI_CPP_11_NOEXCEPT
#else
#if defined(OTL_NO_THROW_IS_EMPTY_THROW)
#define OTL_NO_THROW throw()
#else
#define OTL_NO_THROW
#endif
#endif
#endif

#define OTL_TYPE_NAME typename

#include <new>

#else

#define OTL_SCAST(_t,_e) ((_t)(_e))
#define OTL_RCAST(_t,_e) ((_t)(_e))
#define OTL_DCAST(_t,_e) ((_t)(_e))
#define OTL_CCAST(_t,_e) ((_t)(_e))
#define OTL_CONST_EXCEPTION
#define OTL_THROWS_OTL_EXCEPTION
#if defined(OTL_NO_THROW_IS_EMPTY_THROW)
#define OTL_NO_THROW throw()
#else
#define OTL_NO_THROW
#endif
#define OTL_TYPE_NAME class

#endif

#define OTL_PCONV(_to,_from,_val) \
OTL_SCAST(_to,*OTL_RCAST(_from*,OTL_CCAST(void*,_val)))

#if defined(OTL_ACE)

#include <ace/SString.h>
#include <ace/Containers_T.h>
#include <ace/Null_Mutex.h>
#include <ace/Functor.h>
#include <ace/RB_Tree.h>

#define OTL_USER_DEFINED_STRING_CLASS_ON
#define USER_DEFINED_STRING_CLASS ACE_TString
#define OTL_VALUE_TEMPLATE_ON

const int otl_tmpl_vector_default_size=16;

template<OTL_TYPE_NAME T>
class otl_tmpl_vector: public ACE_Array<T>{
public:

 otl_tmpl_vector(const int init_size=otl_tmpl_vector_default_size)
  : ACE_Array<T>(init_size==0?otl_tmpl_vector_default_size:init_size)
 {
   length_=0;
 }

 ~otl_tmpl_vector(){}

 int capacity(void) const
 {
  return this->max_size();
 }

 int size(void) const
 {
  return length_;
 }

 void clear(void)
 {
  length_=0;
 }

 void resize(const int new_size, const T& t=T())
 {
  ACE_Array<T>::size(new_size);
  if(new_size>length_){
   for(int i=length_;i<new_size;++i)
    (*this)[i]=t;
  }
  length_=new_size;
 }

 void push_back(const T& elem)
 {
   int curr_max_size=OTL_SCAST(int,this->max_size());
   if(length_==curr_max_size)
     ACE_Array<T>::size(curr_max_size*2);
   ++length_;
   (*this)[length_-1]=elem;
 }

 void pop_back(void)
 {
  if(length_>0)
   --length_;
 }

private:

 int length_;
 
};

#endif

#if defined(OTL_STLPORT)
#if defined(__STLPORT_STD)
#define OTL_STLPORT_NAMESPACE __STLPORT_STD
#else
#if defined(_STLP_USE_OWN_NAMESPACE)
#define OTL_STLPORT_NAMESPACE _STL
#else
#define OTL_STLPORT_NAMESPACE std
#endif
#endif

#define OTL_STL

#endif

#if defined(OTL_UNCAUGHT_EXCEPTION_ON)
#include <exception>
#if !defined(OTL_STLPORT)
inline bool otl_uncaught_exception()           
{                                               
  return std::uncaught_exception();             
}
#else
inline bool otl_uncaught_exception()
{                                             
#if defined(OTL_STLPORT_USES_STD_ALIAS_NAMESPACE)
  return __std_alias::uncaught_exception();             
#else
  return OTL_STLPORT_NAMESPACE::uncaught_exception();             
#endif
}
#endif
#else
inline bool otl_uncaught_exception()
{                                             
  return false; 
}
#endif


#if defined(OTL_VALUE_TEMPLATE_ON) && !defined(OTL_STL) && !defined(OTL_ACE)
#define STD_NAMESPACE_PREFIX
#if (defined(_MSC_VER)&&(_MSC_VER>=1300))||defined(OTL_ANSI_CPP)
#include <iostream>
#else
#include <iostream.h>
#endif
#endif

#if defined(OTL_USER_DEFINED_STRING_CLASS_ON)

#if defined(OTL_STL)
#error OTL_STL cannot be used in combination with OTL_USER_DEFINED_STRING_CLASS_ON
#endif

#if defined(USER_DEFINED_STRING_CLASS)
#define OTL_STRING_CONTAINER USER_DEFINED_STRING_CLASS
#define STD_NAMESPACE_PREFIX
#else
#error USER_DEFINED_STRING_CLASS macro needs to be defined before including otlv4.h
#endif

#endif


#if defined(OTL_STL)
#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning (disable:4786) 
#pragma warning (disable:4290)
#pragma warning (disable:4996)
#endif
#endif

#if defined(OTL_STL_NOSTD_NAMESPACE)
#ifndef OTL_STRING_CONTAINER
#define OTL_STRING_CONTAINER string
#endif
#define STD_NAMESPACE_PREFIX
#else
#ifndef OTL_STRING_CONTAINER

#if defined(OTL_STLPORT)
#define OTL_STRING_CONTAINER OTL_STLPORT_NAMESPACE ::string
#else
#define OTL_STRING_CONTAINER std::string
#endif

#endif

#if defined(OTL_STLPORT)
#define STD_NAMESPACE_PREFIX OTL_STLPORT_NAMESPACE ::
#else
#define STD_NAMESPACE_PREFIX std::
#endif


#endif

#include <string>
#include <iterator>
#include <vector>

#ifndef OTL_STL_NOSTD_NAMESPACE
#include <iostream>
#else
#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#include <iostream>
#else
#include <iostream.h>
#endif
#endif

#endif

//======================= END OF CONFIGURATION ==============================


// ====== COMMON NON-TEMPLATE OBJECTS: CONSTANTS, CLASSES, ETC. ===========

#if defined(OTL_INITIAL_VAR_LIST_SIZE)

const int otl_var_list_size=OTL_INITIAL_VAR_LIST_SIZE;

#else

#if defined(OTL_ORA8)
const int otl_var_list_size=1024;
#else
const int otl_var_list_size=512;

#endif

#endif

const int otl_error_code_0=32000;
#define otl_error_msg_0 "Incompatible data types in stream operation"

const int otl_error_code_1=32004;
#define otl_error_msg_1 "No input variables have been defined in SQL statement"

const int otl_error_code_2=32003;
#define otl_error_msg_2 "Not all input variables have been initialized"

const int otl_error_code_3=32001;
#define otl_error_msg_3 "Row must be full for flushing output stream"

const int otl_error_code_4=32005;
#define otl_error_msg_4 "Input string value is too large to fit into the buffer"

const int otl_error_code_5=32006;
#define otl_error_msg_5 "Input otl_long_string is too large to fit into the buffer"

const int otl_error_code_6=32007;
#define otl_error_msg_6 "PL/SQL table size is too large (>32767)"

const int otl_error_code_7=32008;
#define otl_error_msg_7 "Writing CLOB/BLOB in stream mode: actual size is greater than specified"

const int otl_error_code_8=32009;
#define otl_error_msg_8 "Closing CLOB/BLOB in stream mode: actual size is not equal to specified size"

const int otl_error_code_9=32010;
#define otl_error_msg_9 "CLOB/BLOB stream is not open for writing"

const int otl_error_code_10=32011;
#define otl_error_msg_10 "CLOB/BLOB stream is not open for reading"

const int otl_error_code_11=32012;
#define otl_error_msg_11 "First session must be started with session_begin()"

const int otl_error_code_12=32013;
#define otl_error_msg_12 "Invalid bind variable declaration"

const int otl_error_code_13=32014;
#define otl_error_msg_13 "No stored procedure was found"

const int otl_error_code_14=32015;
#define otl_error_msg_14 "Unsupported data type: "

const int otl_error_code_15=32016;
#define otl_error_msg_15 "Unsupported procedure type"

const int otl_error_code_16=32017;
#define otl_error_msg_16 "Stream buffer size can't be > 1 in this case"

const int otl_error_code_17=32018;
#define otl_error_msg_17                                        \
"ODBC / DB2 CLI function name is not recognized. "              \
"It should be one of the following: SQLTables, SQLColumns, "    \
"SQLProcedures, SQLColumnPrivileges, SQLTablePrivileges, "      \
"SQLPrimaryKeys, SQLProcedureColumns, SQLForeignKeys, SQLStatistics"


const int otl_error_code_18=32019;
#define otl_error_msg_18                                \
"otl_stream::operator>>() should have been called "     \
"before otl_stream::operator int()"

const int otl_error_code_19=32020;
#define otl_error_msg_19                                \
"otl_stream_read_iterator: otl_stream is not open"

const int otl_error_code_20=32021;
#define otl_error_msg_20                                \
"otl_stream_read_iterator: PL/SQL table and 'refcur' "  \
"parameters are not supported"

const int otl_error_code_21=32022;
#define otl_error_msg_21                                        \
"otl_stream_read_iterator: otl_stream cannot be described" 

const int otl_error_code_22=32023;
#define otl_error_msg_22                                \
"otl_stream_read_iterator: position is out of range" 

const int otl_error_code_23=32024;
#define otl_error_msg_23                        \
"otl_stream_read_iterator: incompatible types in get()" 

const int otl_error_code_24=32025;
#define otl_error_msg_24 \
"otl_stream::operator int() is not supported in the LOB stream mode"

const int otl_error_code_25=32026;
#define otl_error_msg_25                                                     \
"otl_stream_read_iterator: get(otl_lob_stream*&) function "                  \
"can only be used if otl_stream::set_lob_stream_mode(true) had been called " \
"before the iterator was attached to the stream"

const int otl_error_code_26=32027;
#define otl_error_msg_26                                \
"otl_stream_read_iterator: variable name is not recognized " 

const int otl_error_code_27=32028;
#define otl_error_msg_27 "Unsupported column data type" 

const int otl_error_code_28=32029;
#define otl_error_msg_28 \
"RAW value cannot be read with otl_lob_stream, use otl_long_string instead" 

const int otl_error_code_29=32030;
#define otl_error_msg_29 \
"otl_stream is already open" 

const int otl_error_code_30=32031;
#define otl_error_msg_30 \
"otl_connect is already connected" 

const int otl_error_code_31=32032;
#define otl_error_msg_31 \
"SELECT otl_stream buffer size for TimesTen should be in [0..128] range" 

const int otl_error_code_32=32033;
#define otl_error_msg_32 \
"otl_connect object needs to be connected to DB before using otl_subscriber" 

const int otl_error_code_33=32034;
#define otl_error_msg_33 \
"otl_stream buffer size should be 1 when refcur or plsql table is used" 

const int otl_error_code_34=32035;
#define otl_error_msg_34 "END-OF-ROW check failed"

const int otl_error_code_35=32036;
#define otl_error_msg_35 "otl_connect is not connected to the database"

const int otl_error_code_36=32037;
#define otl_error_msg_36 \
"SQL Statement has a white space in bind variable declaration" 

const int otl_error_code_37=32038;
#define otl_error_msg_37 \
"otl_long_unicode_string should be used with strings when OTL_UNICODE is enabled, " \
" and otl_long_string should be use with strings when OTL_UNICODE is not enabled" 

const int otl_error_code_38=32039;
#define otl_error_msg_38 \
"otl_long_string should be used with nonstrings when OTL_UNICODE is enabled" 

const int otl_error_code_39=32040;
#define otl_error_msg_39 \
"This type of otl_stream can only have input variables"

const int otl_error_code_40=32041;
#define otl_error_msg_40 "Invalid stream buffer size (<=0)"

const int otl_error_code_41=32042;
#define otl_error_msg_41 "otl_lob_stream can't be used as an input parameter " \
"with a SELECT statement or a stored procedure that returns an implicit result set"

const int otl_oracle_date_size=7;

const int otl_explicit_select=0;
const int otl_implicit_select=1;

const int otl_input_param=0;
const int otl_output_param=1;
const int otl_inout_param=2;

const unsigned int otl_all_num2str=1;
const unsigned int otl_all_date2str=2;

const int otl_num_str_size=60;
const int otl_date_str_size=60;

template <OTL_TYPE_NAME T>
class otl_auto_array_ptr{
public:
  
  otl_auto_array_ptr():
    ptr(0),
    arr_size_(0)
  {
  }
  
  otl_auto_array_ptr(const int arr_size):
    ptr(new T[arr_size]),
    arr_size_(arr_size)
  {
  }
  
  void double_size(void)
  {
    int old_arr_size=arr_size_;
    arr_size_*=2;
    T* temp_ptr=new T[arr_size_];
    for(int i=0;i<old_arr_size;++i)
      temp_ptr[i]=ptr[i];
    delete[] ptr;
    ptr=temp_ptr;
  }
  
  virtual ~otl_auto_array_ptr()
  {
    delete[] ptr;
  }

  T* get_ptr()
  {
    return ptr;
  }

  int get_arr_size() const
  {
    return arr_size_;
  }

private:

  T* ptr;
  int arr_size_;

  otl_auto_array_ptr(const otl_auto_array_ptr<T>&):
    ptr(nullptr),
    arr_size_(0)
  {
  }

  otl_auto_array_ptr<T>& operator=(const otl_auto_array_ptr<T>&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_auto_array_ptr(otl_auto_array_ptr<T>&&):
    ptr(nullptr),
    arr_size_(0)
  {
  }

  otl_auto_array_ptr<T>& operator=(otl_auto_array_ptr<T>&&)
  {
    return *this;
  }
#endif

};

template <OTL_TYPE_NAME T>
class otl_ptr{
public:

  otl_ptr():
    ptr(nullptr),
    arr_flag(0)
 {
 }

 void assign(T** var)
 {
  ptr=var;
  arr_flag=0;
 }

 void assign_array(T** var)
 {
  ptr=var;
  arr_flag=1;
 }


 void disconnect(void)
 {
  if(ptr!=nullptr)
   *ptr=nullptr;
  ptr=nullptr;
 }

 void destroy(void)
 {
  if(ptr==nullptr)return;
  if(*ptr!=nullptr){
   if(arr_flag)
    delete[] *ptr;
   else
    delete *ptr;
   *ptr=nullptr;
  }
 }

 ~otl_ptr()
 {
  destroy();
 }

protected:

 T** ptr;
 int arr_flag;

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_ptr(const otl_ptr&) = delete;
  otl_ptr& operator=(const otl_ptr&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_ptr(otl_ptr&&) = delete;
  otl_ptr& operator=(otl_ptr&&) = delete;
#endif
private:
#else
  otl_ptr(const otl_ptr&):
    ptr(nullptr),
    arr_flag(0)
  {
  }

  otl_ptr& operator=(const otl_ptr&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_ptr(otl_ptr&&):
    ptr(nullptr),
    arr_flag(0)
  {
  }

  otl_ptr& operator=(otl_ptr&&)
  {
    return *this;
  }
#endif
#endif

};

template <OTL_TYPE_NAME T>
class otl_Tptr{
public:

 otl_Tptr():
    ptr(nullptr),
    do_not_destroy(false)
  {
  }
  
  void assign(T* var)
  {
    ptr=var;
 }
  
  void disconnect(void)
  {
    ptr=nullptr;
  }
  
  void destroy(void)
  {
    if(do_not_destroy)
      return;
    delete ptr;
    ptr=nullptr;
 }
  
  ~otl_Tptr()
  {
    destroy();
  }

  otl_Tptr& operator=(const otl_Tptr& src)
  {
    ptr=src.ptr;
    do_not_destroy=src.do_not_destroy;
    return *this;
  }

  void set_do_not_destroy(const bool ado_not_destroy)
  {
    do_not_destroy=ado_not_destroy;
  }

  T* get_ptr()
  {
    return ptr;
  }

protected:

  T* ptr;
  bool do_not_destroy;

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_Tptr(const otl_Tptr&) = delete;
  otl_Tptr(otl_Tptr&&) = delete;
private:
#else
  otl_Tptr(const otl_Tptr&):
    ptr(nullptr),
    do_not_destroy(false)
  {
  }
#if defined(OTL_ANSI_CPP_11_RVAL_REF_SUPPORT)
  otl_Tptr(otl_Tptr&&):
    ptr(nullptr),
    do_not_destroy(false)
  {
  }
#endif
#endif

};

class otl_select_struct_override{
public:

  otl_select_struct_override():
    col_ndx(new short int[otl_var_list_size]),
    col_type(new short int[otl_var_list_size]),
    col_size(new int[otl_var_list_size]),
    len(0),
    all_mask(0),
    lob_stream_mode(false),
    container_size_(otl_var_list_size),
    master_stream_ptr_(nullptr)
  {
  }

  ~otl_select_struct_override()
  {
    delete[] col_ndx;
    delete[] col_type;
    delete[] col_size;
  }

  void set_master_stream_ptr(void* stream_ptr)
  {
    master_stream_ptr_=stream_ptr;
  }

  void* get_master_stream_ptr(){return master_stream_ptr_;}

  void reset(void)
  {
    len=0;
    all_mask=0;
    lob_stream_mode=false;
  }
  
  void add_override(const int andx, const int atype, const int asize=0)
  {
    if(len==container_size_){
      int temp_container_size=container_size_;
      container_size_*=2;
      short int* temp_col_ndx=nullptr;
      short int* temp_col_type=nullptr;
      int* temp_col_size=nullptr;
      try{
        temp_col_ndx=new short int[container_size_];
        temp_col_type=new short int[container_size_];
        temp_col_size=new int[container_size_];
      }catch(const std::bad_alloc&){
        delete[] temp_col_ndx;
        delete[] temp_col_type;
        delete[] temp_col_size;
        throw;
      }
      memcpy(temp_col_ndx,col_ndx,sizeof(short int)*temp_container_size);
      memcpy(temp_col_type,col_type,sizeof(short int)*temp_container_size);
      memcpy(temp_col_size,col_size,sizeof(int)*temp_container_size);
      delete[] col_ndx;
      delete[] col_type;
      delete[] col_size;
      col_ndx=temp_col_ndx;
      col_type=temp_col_type;
      col_size=temp_col_size;
    }
    ++len;
    col_ndx[len-1]=OTL_SCAST(short,andx);
    col_type[len-1]=OTL_SCAST(short,atype);
    col_size[len-1]=asize;
  }
  
  int find(const int ndx) const
  {
    int i;
    for(i=0;i<len;++i)
      if(ndx==col_ndx[i])
        return i;
    return -1;
  }
  
  void set_all_column_types(const unsigned int amask=0)
  {
    all_mask=amask;
  }
  
  int getLen(void) const
  {
    return len;
  }

  unsigned int get_all_mask() const
  {
    return all_mask;
  }

  short int get_col_type(int ndx) const
  {
    return col_type[ndx];
  }

  int get_col_size(int ndx) const
  {
    return col_size[ndx];
  }

  void setLen(const int alen)
  {
    len=alen;
  }

  bool get_lob_stream_mode() const
  {
    return lob_stream_mode;
  }

  void set_lob_stream_mode(const bool alobs_tream_mode)
  {
    lob_stream_mode=alobs_tream_mode;
  }

private:

  short int* col_ndx;
  short int* col_type;
  int* col_size;
  int len;
  
  unsigned int all_mask;
  bool lob_stream_mode;

  int container_size_;
  void* master_stream_ptr_;

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_select_struct_override(const otl_select_struct_override&) = delete;
  otl_select_struct_override& operator=(const otl_select_struct_override&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_select_struct_override(otl_select_struct_override&&) = delete;
  otl_select_struct_override& operator=(otl_select_struct_override&&) = delete;
#endif
private:
#else
  otl_select_struct_override(const otl_select_struct_override&):
    col_ndx(nullptr),
    col_type(nullptr),
    col_size(nullptr),
    len(0),
    all_mask(0),
    lob_stream_mode(false),
    container_size_(0),
    master_stream_ptr_(nullptr)
  {
  }

  otl_select_struct_override& operator=(const otl_select_struct_override&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_select_struct_override(otl_select_struct_override&&):
    col_ndx(nullptr),
    col_type(nullptr),
    col_size(nullptr),
    len(0),
    all_mask(0),
    lob_stream_mode(false),
    container_size_(0),
    master_stream_ptr_(nullptr)
  {
  }

  otl_select_struct_override& operator=(otl_select_struct_override&&)
  {
    return *this;
  }
#endif
#endif
};

inline int otl_decimal_degree(unsigned int num)
{
  int n=0;
  while(num!=0){
    ++n;
    num/=10;
  }
  return n;
}

inline bool otl_isspace(char c)
{
  return c==' '||c=='\t'||c=='\n'||
         c=='\r'||c=='\f'||c=='\v';
}

inline char otl_to_upper(char c)
{
 return OTL_SCAST(char,toupper(c));
}

inline bool otl_str_case_insensitive_equal(const char* s1, const char* s2)
{
  while(*s1 && *s2){
    if(otl_to_upper(*s1)!=otl_to_upper(*s2))
      return false;
    ++s1;
    ++s2;
  }
  if(*s1==0 && *s2==0)
    return true;
  else
    return false;
}

inline unsigned int otl_to_fraction
(unsigned int fraction,int frac_prec)
{
  if(fraction==0||frac_prec==0)return fraction;
  int fraction_degree=otl_decimal_degree(fraction);
  if(fraction_degree>frac_prec){
    // in case if the actual fraction value is beyond the "fraction
    // precision" range, truncate the value
    int excess_decimal_digits=fraction_degree-frac_prec;
    for(int iter=1;iter<=excess_decimal_digits;++iter)
      fraction/=10;
  }
  int degree_diff=9-frac_prec;
  for(int i=0;i<degree_diff;++i)
    fraction*=10;
  return fraction;
}

inline unsigned int otl_from_fraction
(unsigned int fraction,int frac_prec)
{
  if(fraction==0||frac_prec==0)return fraction;
  int degree_diff=9-frac_prec;
  for(int i=0;i<degree_diff;++i)
    fraction/=10;
  return fraction;
}

#define OTL_NO_STM_TEXT "#No Stm Text available#"

#if (defined(OTL_ODBC)||defined(OTL_DB2_CLI)) && defined(OTL_ORA_UTF8)
#if !defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
#define OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS
#endif
#endif

class otl_datetime{
public:

  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  unsigned long fraction;
  int frac_precision;

#if defined(OTL_ORA_TIMESTAMP) || defined(OTL_ODBC_TIME_ZONE) || \
    defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
  short int tz_hour;
  short int tz_minute;
#endif

  otl_datetime():
    year(1900),
    month(1),
    day(1),
    hour(0),
    minute(0),
    second(0),
    fraction(0),
    frac_precision(0)
#if defined(OTL_ORA_TIMESTAMP) || defined(OTL_ODBC_TIME_ZONE) || \
    defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
    ,tz_hour(0),
    tz_minute(0)
#endif
 {
 }

 otl_datetime
 (const int ayear,
  const int amonth,
  const int aday,
  const int ahour,
  const int aminute,
  const int asecond,
  const unsigned long afraction=0,
  const int afrac_precision=0
#if defined(OTL_ORA_TIMESTAMP)||defined(OTL_ODBC_TIME_ZONE) || \
    defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
  , 
  const short int atz_hour=0,
  const short int atz_minute=0
#endif
   ):
   year(ayear),
   month(amonth),
   day(aday),
   hour(ahour),
   minute(aminute),
   second(asecond),
   fraction(afraction),
   frac_precision(afrac_precision)
#if defined(OTL_ORA_TIMESTAMP)||defined(OTL_ODBC_TIME_ZONE) || \
    defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
   ,tz_hour(atz_hour),
   tz_minute(atz_minute)
#endif
 {
 }

  otl_datetime(const otl_datetime& dt):
    year(dt.year),
    month(dt.month),
    day(dt.day),
    hour(dt.hour),
    minute(dt.minute),
    second(dt.second),
    fraction(dt.fraction),
    frac_precision(dt.frac_precision)
#if defined(OTL_ORA_TIMESTAMP)||defined(OTL_ODBC_TIME_ZONE) || \
    defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
    ,tz_hour(dt.tz_hour),
    tz_minute(dt.tz_minute)
#endif
 {
 }

 ~otl_datetime(){}

 otl_datetime& operator=(const otl_datetime& dt)
 {
   copy(dt);
   return *this;
 }

private:

  void copy(const otl_datetime& dt)
  {
    year=dt.year;
    month=dt.month;
    day=dt.day;
    hour=dt.hour;
    minute=dt.minute;
    second=dt.second;
    fraction=dt.fraction;
    frac_precision=dt.frac_precision;
#if defined(OTL_ORA_TIMESTAMP)||defined(OTL_ODBC_TIME_ZONE) || \
    defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
    tz_hour=dt.tz_hour;
    tz_minute=dt.tz_minute;
#endif
  }

};

struct otl_oracle_date{

 unsigned char century;
 unsigned char year;
 unsigned char month;
 unsigned char day;
 unsigned char hour;
 unsigned char minute;
 unsigned char second;

  otl_oracle_date():
    century(0),
    year(0),
    month(0),
    day(0),
    hour(0),
    minute(0),
    second(0)
  {
  }

 ~otl_oracle_date(){}

};

inline void convert_date(otl_datetime& t,const otl_oracle_date& s)
{
 t.year=(OTL_SCAST(int, s.century-100)*100+(OTL_SCAST(int, s.year-100)));
 t.month=s.month;
 t.day=s.day;
 t.hour=s.hour-1;
 t.minute=s.minute-1;
 t.second=s.second-1;
}

inline void convert_date(otl_oracle_date& t,const otl_datetime& s)
{
 t.year=OTL_SCAST(unsigned char, ((s.year%100)+100));
 t.century=OTL_SCAST(unsigned char, ((s.year/100)+100));
 t.month=OTL_SCAST(unsigned char, s.month);
 t.day=OTL_SCAST(unsigned char, s.day);
 t.hour=OTL_SCAST(unsigned char, (s.hour+1));
 t.minute=OTL_SCAST(unsigned char, (s.minute+1));
 t.second=OTL_SCAST(unsigned char, (s.second+1));
}

class otl_null{
public:

 otl_null(){}
 ~otl_null(){}

private:

#if (defined(_MSC_VER)&&(_MSC_VER==1200))
 int dummy; // this is to fix a compiler bug in VC++ 6.0
#endif

};

class otl_column_desc{
public:

  char* name;
  int  dbtype;
  int  otl_var_dbtype;
#if defined(_WIN64)
  __int64 dbsize;
#else
  int  dbsize;
#endif
  int  scale;
#if defined(_WIN64)
  __int64 prec;
#else
  int  prec;
#endif
  int  nullok;
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)||\
    defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
  int charset_form;
  int char_size;
#endif

  otl_column_desc():
    name(nullptr),
    dbtype(0),
    otl_var_dbtype(0),
    dbsize(0),
    scale(0),
    prec(0),
    nullok(0),
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)||\
    defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
    charset_form(0),
    char_size(0),
#endif
    name_len_(0)
  {
  }

  ~otl_column_desc()
  {
    delete[] name;
  }

  otl_column_desc& operator=(const otl_column_desc& desc)
  {
    if(name_len_>=desc.name_len_)
      OTL_STRCPY_S(name,name_len_,desc.name);
    else if(name==nullptr && desc.name!=nullptr){
      name=new char[desc.name_len_];
      name_len_=desc.name_len_;
      OTL_STRCPY_S(name,name_len_,desc.name);
    }else if(name_len_<desc.name_len_ && desc.name!=nullptr){
      delete[] name;
      name=new char[desc.name_len_];
      name_len_=desc.name_len_;
      OTL_STRCPY_S(name,name_len_,desc.name);
    }
    dbtype=desc.dbtype;
    otl_var_dbtype=desc.otl_var_dbtype;
    dbsize=desc.dbsize;
    scale=desc.scale;
    prec=desc.prec;
    nullok=desc.nullok;
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)||\
    defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
    charset_form=desc.charset_form;
    char_size=desc.char_size;
#endif

    return *this;
  }

  otl_column_desc(const otl_column_desc& desc):
    name(nullptr),
    dbtype(desc.dbtype),
    otl_var_dbtype(0),
    dbsize(desc.dbsize),
    scale(desc.scale),
    prec(desc.prec),
    nullok(desc.nullok),
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)||\
    defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
    charset_form(desc.charset_form),
    char_size(desc.char_size),
#endif
    name_len_(desc.name_len_)
  {
    if(desc.name!=nullptr){
      name=new char[desc.name_len_];
      OTL_STRCPY_S(name,name_len_,desc.name);
    }
  }


#if defined(OTL_ANSI_CPP_11_RVAL_REF_SUPPORT)

  otl_column_desc& operator=(otl_column_desc&& desc) OTL_ANSI_CPP_11_NOEXCEPT
  {
    if(name!=nullptr)delete[] name;
    name=desc.name;
    name_len_=desc.name_len_;
    desc.name=nullptr;
    desc.name_len_=0;
    dbtype=desc.dbtype;
    otl_var_dbtype=desc.otl_var_dbtype;
    dbsize=desc.dbsize;
    scale=desc.scale;
    prec=desc.prec;
    nullok=desc.nullok;
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)||\
    defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
    charset_form=desc.charset_form;
    char_size=desc.char_size;
#endif
    return *this;
  }

  otl_column_desc(otl_column_desc&& desc) OTL_ANSI_CPP_11_NOEXCEPT
   :name(desc.name),
    dbtype(desc.dbtype),
    otl_var_dbtype(0),
    dbsize(desc.dbsize),
    scale(desc.scale),
    prec(desc.prec),
    nullok(desc.nullok),
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)||\
    defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
    charset_form(desc.charset_form),
    char_size(desc.char_size),
#endif
    name_len_(desc.name_len_)
  {
    desc.name=nullptr;
    desc.name_len_=0;
  }

#endif

  void set_name(const char* aname,const int aname_len=0)
  {
    int len;
    if(aname_len==0)
      len=OTL_SCAST(int,strlen(aname))+1;
    else
      len=aname_len+1;
    if(name_len_<len){
      if(name)delete[] name;
      name=new char[len];
      name_len_=len;
      for(int i=0;i<len-1;++i)
        name[i]=aname[i];
      name[len-1]=0;
    }
  }

private:

  int name_len_;

};

class otl_var_desc{
public:

  int  param_type;
  int  ftype;
  int  elem_size;
  int  array_size;
  int  pos;
  int  name_pos;
  char name[128];
  int  pl_tab_flag;

  otl_var_desc():
    param_type(0),
    ftype(0),
    elem_size(0),
    array_size(0),
    pos(0),
    name_pos(0),
    name(),
    pl_tab_flag(0)
 {
   name[0]=0;
 }

 ~otl_var_desc(){}

 void copy_name(const char* nm)
 {
  if(!nm)
   name[0]=0;
  else{
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
   OTL_STRNCPY_S(name,sizeof(name),nm,sizeof(name)-1);
   name[sizeof(name)-1]=0;
#else
   strncpy(name,nm,sizeof(name));
   name[sizeof(name)-1]=0;
#endif
#else
   strncpy(name,nm,sizeof(name));
   name[sizeof(name)-1]=0;
#endif
  }
 }

};

const int otl_var_none=0;
const int otl_var_char=1;
const int otl_var_double=2;
const int otl_var_float=3;
const int otl_var_int=4;
const int otl_var_unsigned_int=5;
const int otl_var_short=6;
const int otl_var_long_int=7;
const int otl_var_timestamp=8;
const int otl_var_varchar_long=9;
const int otl_var_raw_long=10;
const int otl_var_clob=11;
const int otl_var_blob=12;
const int otl_var_refcur=13;
const int otl_var_long_string=15;
const int otl_var_db2time=16;
const int otl_var_db2date=17;
const int otl_var_tz_timestamp=18;
const int otl_var_ltz_timestamp=19;
const int otl_var_bigint=20;
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
const int otl_var_nchar=21;
const int otl_var_nclob=22;
#else
#endif
const int otl_var_raw=23;
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
const int otl_var_numeric_type_1=24;
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
const int otl_var_numeric_type_2=25;
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
const int otl_var_numeric_type_3=26;
#endif
const int otl_var_ubigint=27;
const int otl_var_bfloat=28;
const int otl_var_bdouble=29;

const int otl_var_lob_stream=100;

const int otl_bigint_str_size=40;
const int otl_ubigint_str_size=40;
#if defined(OTL_NUMERIC_TYPE_1_STR_SIZE)
const int otl_numeric_type_1_str_size=OTL_NUMERIC_TYPE_1_STR_SIZE;
#else
const int otl_numeric_type_1_str_size=60;
#endif
#if defined(OTL_NUMERIC_TYPE_2_STR_SIZE)
const int otl_numeric_type_2_str_size=OTL_NUMERIC_TYPE_2_STR_SIZE;
#else
const int otl_numeric_type_2_str_size=60;
#endif
#if defined(OTL_NUMERIC_TYPE_3_STR_SIZE)
const int otl_numeric_type_3_str_size=OTL_NUMERIC_TYPE_3_STR_SIZE;
#else
const int otl_numeric_type_3_str_size=60;
#endif

const int otl_sql_exec_from_cursor_class=0;
const int otl_sql_exec_from_select_cursor_class=1;

class otl_long_string{
public:

  unsigned char* v;

  otl_long_string(const int buffer_size=otl_short_int_max,const int input_length=0):
    v(nullptr),
    length(0),
    extern_buffer_flag(0),
    buf_size(0),
    this_is_last_piece_(false),
    unicode_flag_(false)
 {
   this_is_last_piece_=false;
   if(buffer_size==0){
     v=nullptr;
     length=0;
     extern_buffer_flag=0;
   }else{
     extern_buffer_flag=0;
     length=input_length;
     buf_size=buffer_size;
     v=new unsigned char[buffer_size+1];
     memset(v,0,buffer_size);
   }
 }

 otl_long_string
 (const void* external_buffer,
  const int buffer_size,
  const int input_length=0):
    v(OTL_RCAST(unsigned char*, OTL_CCAST(void*, external_buffer))),
    length(input_length),
    extern_buffer_flag(1),
    buf_size(buffer_size),
    this_is_last_piece_(false),
    unicode_flag_(false)
 {
 }

  otl_long_string& operator=(const otl_long_string& s)
  {
    this_is_last_piece_=s.this_is_last_piece_;
    if(s.extern_buffer_flag){
      if(!extern_buffer_flag)
        delete[] v;
      v=s.v;
      length=s.length;
      extern_buffer_flag=s.extern_buffer_flag;
      buf_size=s.buf_size;
    }else{
      if(extern_buffer_flag){
        v=new unsigned char[s.buf_size+1];
        buf_size=s.buf_size;
      }else if(buf_size<s.buf_size){
        delete[] v;
        v=new unsigned char[s.buf_size+1];
        buf_size=s.buf_size;
      }
      length=s.length;
      extern_buffer_flag=s.extern_buffer_flag;
      memcpy(v,s.v,length);
      if(length<buf_size&&s.v[length]==0)
        v[length]=0;
    }
    return *this;
  }

  otl_long_string(const otl_long_string& s):
    v(nullptr),
    length(s.length),
    extern_buffer_flag(s.extern_buffer_flag),
    buf_size(s.buf_size),
    this_is_last_piece_(s.this_is_last_piece_),
    unicode_flag_(false)
  {
    if(s.extern_buffer_flag)
      v=s.v;
    else{
      v=new unsigned char[buf_size+1];
      memcpy(v,s.v,length);
      if(length<buf_size&&s.v[length]==0)
        v[length]=0;
    }
  }

#if defined(OTL_ANSI_CPP_11_RVAL_REF_SUPPORT)
  otl_long_string& operator=(otl_long_string&& s) OTL_ANSI_CPP_11_NOEXCEPT
  {
    if(!extern_buffer_flag)delete[] v;
    this_is_last_piece_=s.this_is_last_piece_;
    length=s.length;
    extern_buffer_flag=s.extern_buffer_flag;
    v=s.v;
    s.v=nullptr;
    s.length=0;
    s.buf_size=0;
    return *this;
  }

  otl_long_string(otl_long_string&& s) OTL_ANSI_CPP_11_NOEXCEPT 
  : v(s.v),
    length(s.length),
    extern_buffer_flag(s.extern_buffer_flag),
    buf_size(s.buf_size),
    this_is_last_piece_(s.this_is_last_piece_),
    unicode_flag_(false)
  {
    s.v=nullptr;
    s.length=0;
    s.buf_size=0;
  }
#endif
  
  virtual ~otl_long_string()
  {
   if(!extern_buffer_flag)delete[] v;
  }

  void set_len(const int alen=0){length=alen;}
  int len(void)const {return length;}

  void set_last_piece(const bool this_is_last_piece=false)
  {
    this_is_last_piece_=this_is_last_piece;
  }
  
  bool is_last_piece() const
  {
    return this_is_last_piece_;
  }

  unsigned char& operator[](int ndx) 
  {
    return v[ndx];
  }

  virtual void null_terminate_string(const int alen) 
  {
    (*this)[alen]=0;
  }

  int get_buf_size() const
  {
    return buf_size;
  }

  int get_extern_buffer_flag() const
  {
    return extern_buffer_flag;
  }

  bool get_unicode_flag() const
  {
    return unicode_flag_;
  }

protected:

  int length;
  int extern_buffer_flag;
  int buf_size;
  bool this_is_last_piece_;
  bool unicode_flag_;

};

#if defined(OTL_UNICODE)
class otl_long_unicode_string: public otl_long_string{
public:

  otl_long_unicode_string(const int buffer_size=otl_short_int_max,const int input_length=0)
    : otl_long_string(0,0)
  {
    unicode_flag_=true;
    extern_buffer_flag=0;
    length=input_length;
    buf_size=buffer_size;
    v=new unsigned char[(buffer_size+1)*sizeof(OTL_WCHAR)];
    memset(v,0,buffer_size*sizeof(OTL_WCHAR));
  }
  
  otl_long_unicode_string
  (const void* external_buffer, 
   const int buffer_size,
   const int input_length=0)
    : otl_long_string(external_buffer,buffer_size,input_length)
  {
    unicode_flag_=true;
    extern_buffer_flag=1;
    length=input_length;
    buf_size=buffer_size;
    v=OTL_RCAST(unsigned char*, OTL_CCAST(void*, external_buffer));
  }

  otl_long_unicode_string(const otl_long_unicode_string& s) : otl_long_string(0,0)
  {
    this->unicode_flag_=true;
    this->buf_size=s.buf_size;
    this->this_is_last_piece_=s.this_is_last_piece_;
    this->extern_buffer_flag=s.extern_buffer_flag;
    this->v=nullptr;
    this->length=s.length;
    if(s.extern_buffer_flag)
      v=s.v;
    else{
      v=new unsigned char[(buf_size+1)*sizeof(OTL_WCHAR)];
      memcpy(v,s.v,length*sizeof(OTL_WCHAR));
      if(length<buf_size&&s.v[length]==0)
        (*this)[length]=0;
    }
  }

  otl_long_unicode_string& operator=(const otl_long_unicode_string& s)
  {
    this_is_last_piece_=s.this_is_last_piece_;
    if(s.extern_buffer_flag){
      if(!extern_buffer_flag)
        delete[] v;
      v=s.v;
      length=s.length;
      extern_buffer_flag=s.extern_buffer_flag;
      buf_size=s.buf_size;
    }else{
      if(extern_buffer_flag){
        v=new unsigned char[(s.buf_size+1)*sizeof(OTL_WCHAR)];
        buf_size=s.buf_size;
      }else if(buf_size<s.buf_size){
        delete[] v;
        v=new unsigned char[(s.buf_size+1)*sizeof(OTL_WCHAR)];
        buf_size=s.buf_size;
      }
      length=s.length;
      extern_buffer_flag=s.extern_buffer_flag;
      memcpy(v,s.v,length*sizeof(OTL_WCHAR));
      if(length<buf_size&&s.v[length]==0)
        (*this)[length]=0;
    }
    return *this;
  }

#if defined(OTL_ANSI_CPP_11_RVAL_REF_SUPPORT)
  otl_long_unicode_string(otl_long_unicode_string&& s) OTL_ANSI_CPP_11_NOEXCEPT
    : otl_long_string(0,0)
  {
    unicode_flag_=true;
    buf_size=s.buf_size;
    this_is_last_piece_=s.this_is_last_piece_;
    extern_buffer_flag=s.extern_buffer_flag;
    length=s.length;
    v=s.v;
    s.v=nullptr;
    s.buf_size=0;
    s.length=0;
  }

  otl_long_unicode_string& operator=(otl_long_unicode_string&& s) OTL_ANSI_CPP_11_NOEXCEPT
  {
    this_is_last_piece_=s.this_is_last_piece_;
    if(!extern_buffer_flag)delete[] v;
    v=s.v;
    length=s.length;
    extern_buffer_flag=s.extern_buffer_flag;
    buf_size=s.buf_size;
    s.v=nullptr;
    s.buf_size=0;
    s.length=0;
    return *this;
  }
#endif


  virtual ~otl_long_unicode_string(){}
  
  OTL_CHAR& operator[](int ndx) 
  {
    return OTL_RCAST(OTL_CHAR*,v)[ndx];
  }

  virtual void null_terminate_string(const int alen)
  {
    (*this)[alen]=0;
  }

};

#endif

inline const char* otl_var_type_name(const int ftype)
{
  const char* const_CHAR="CHAR";
  const char* const_DOUBLE="DOUBLE";
  const char* const_FLOAT="FLOAT";
  const char* const_BDOUBLE="BINARY_DOUBLE";
  const char* const_BFLOAT="BINARY_FLOAT";
  const char* const_INT="INT";
  const char* const_UNSIGNED_INT="UNSIGNED INT";
  const char* const_SHORT_INT="SHORT INT";
  const char* const_LONG_INT="LONG INT";
  const char* const_TIMESTAMP="TIMESTAMP";
  const char* const_DB2DATE="DB2DATE";
  const char* const_DB2TIME="DB2TIME";
  const char* const_TZ_TIMESTAMP="TIMESTAMP WITH TIME ZONE";
  const char* const_LTZ_TIMESTAMP="TIMESTAMP WITH LOCAL TIME ZONE";
  const char* const_BIGINT="BIGINT";
  const char* const_UBIGINT="UBIGINT";
  const char* const_VARCHAR_LONG="VARCHAR LONG";
  const char* const_RAW_LONG="RAW LONG";
  const char* const_CLOB="CLOB";
  const char* const_BLOB="BLOB";
  const char* const_RAW="RAW";
  const char* const_UNKNOWN="UNKNOWN";
  const char* const_LONG_STRING="otl_long_string()";
  const char* const_LOB_STREAM="otl_lob_stream*&";
  const char* const_USER_DEFINED="User-defined type (object type, VARRAY, Nested Table)";
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
  const char* const_NCHAR="NCHAR"; 
  const char* const_NCLOB="NCLOB";
#endif
  
  switch(ftype){
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
  case otl_var_nchar:
    return const_NCHAR; 
  case otl_var_nclob:
    return const_NCLOB; 
#endif
  case otl_var_char:
    return const_CHAR;
  case otl_var_double:
    return const_DOUBLE;
  case otl_var_float:
    return const_FLOAT;
  case otl_var_bfloat:
    return const_BFLOAT;
  case otl_var_bdouble:
    return const_BDOUBLE;
  case otl_var_int:
    return const_INT;
  case otl_var_unsigned_int:
    return const_UNSIGNED_INT;
  case otl_var_short:
    return const_SHORT_INT;
  case otl_var_long_int:
    return const_LONG_INT;
  case otl_var_timestamp:
    return const_TIMESTAMP;
  case otl_var_db2date:
    return const_DB2DATE;
  case otl_var_db2time:
    return const_DB2TIME;
  case otl_var_tz_timestamp:
    return const_TZ_TIMESTAMP;
  case otl_var_ltz_timestamp:
    return const_LTZ_TIMESTAMP;
  case otl_var_bigint:
    return const_BIGINT;
  case otl_var_ubigint:
    return const_UBIGINT;
  case otl_var_varchar_long:
    return const_VARCHAR_LONG;
  case otl_var_raw_long:
    return const_RAW_LONG;
  case otl_var_clob:
    return const_CLOB;
  case otl_var_blob:
    return const_BLOB;
  case otl_var_raw:
    return const_RAW;
  case otl_var_long_string:
    return const_LONG_STRING;
  case otl_var_lob_stream:
    return const_LOB_STREAM;
  case 108:
    return const_USER_DEFINED;
 default:
  return const_UNKNOWN;
 }
}

inline void otl_var_info_var
(const char* name,
 const int ftype,
 const int type_code,
 char* var_info,
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400) // VC++ 8.0 or higher
 const size_t var_info_sz
#else
 const size_t /*var_info_sz*/
#endif
#else
 const size_t /*var_info_sz*/
#endif
)
{char buf1[128];
 char buf2[128];
 OTL_STRCPY_S(buf1,sizeof(buf1),otl_var_type_name(ftype));
 OTL_STRCPY_S(buf2,sizeof(buf2),otl_var_type_name(type_code));
 OTL_STRCPY_S(var_info,var_info_sz,"Variable: ");
 OTL_STRCAT_S(var_info,var_info_sz,name);
 OTL_STRCAT_S(var_info,var_info_sz,"<");
 OTL_STRCAT_S(var_info,var_info_sz,buf1);
 OTL_STRCAT_S(var_info,var_info_sz,">, datatype in operator <</>>: ");
 OTL_STRCAT_S(var_info,var_info_sz,buf2);
}

inline void otl_var_info_var2
(const char* name,
 const int ftype,
 char* var_info,
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400) // VC++ 8.0 or higher
 const size_t var_info_sz
#else
 const size_t /*var_info_sz*/
#endif
#else
 const size_t /*var_info_sz*/
#endif
)
{char buf1[128];
 OTL_STRCPY_S(buf1,sizeof(buf1),otl_var_type_name(ftype));
 OTL_STRCPY_S(var_info,var_info_sz,"Variable: ");
 OTL_STRCPY_S(var_info,var_info_sz,name);
 OTL_STRCAT_S(var_info,var_info_sz,"<");
 OTL_STRCAT_S(var_info,var_info_sz,buf1);
 OTL_STRCAT_S(var_info,var_info_sz,">");
}

inline void otl_var_info_var3
(const char* name,
 const int ftype,
 const int type_code,
 char* var_info,
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400) // VC++ 8.0 or higher
 const size_t var_info_sz
#else
 const size_t /*var_info_sz*/
#endif
#else
 const size_t /*var_info_sz*/
#endif
)
{char buf1[128];
 char buf2[128];
 OTL_STRCPY_S(buf1,sizeof(buf1),otl_var_type_name(ftype));
 OTL_STRCPY_S(buf2,sizeof(buf2),otl_var_type_name(type_code));
 OTL_STRCPY_S(var_info,var_info_sz,"Variable: ");
 OTL_STRCAT_S(var_info,var_info_sz,name);
 OTL_STRCAT_S(var_info,var_info_sz,"<");
 OTL_STRCAT_S(var_info,var_info_sz,buf1);
 OTL_STRCAT_S(var_info,
              var_info_sz,
              ">, datatype in otl_stream_read_iterator::get(): ");
 OTL_STRCAT_S(var_info,var_info_sz,buf2);
}

inline void otl_var_info_var4
(const char* name,
 const int ftype,
 const int type_code,
 char* var_info,
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400) // VC++ 8.0 or higher
 const size_t var_info_sz
#else
 const size_t /*var_info_sz*/
#endif
#else
 const size_t /*var_info_sz*/
#endif
)
{char buf1[128];
 char buf2[128];
 OTL_STRCPY_S(buf1,sizeof(buf1),otl_var_type_name(ftype));
 OTL_STRCPY_S(buf2,sizeof(buf2),otl_var_type_name(type_code));
 OTL_STRCPY_S(var_info,var_info_sz,"Variable: ");
 OTL_STRCAT_S(var_info,var_info_sz,name);
 OTL_STRCAT_S(var_info,var_info_sz,"<");
 OTL_STRCAT_S(var_info,var_info_sz,buf1);
 OTL_STRCAT_S(var_info,
              var_info_sz,
              ">, datatype in otl_stream_read_iterator::get(): ");
 OTL_STRCAT_S(var_info,var_info_sz,buf2);
}

inline void otl_strcpy(
  unsigned char* trg,
  unsigned char* src,
  int& overflow,
  const int inp_size=0,
  const int actual_inp_size=-1
)
{
  OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
  const OTL_CHAR* c2=OTL_RCAST(const OTL_CHAR*,src);
  int out_size=0;
  overflow=0;
  if(actual_inp_size!=-1){
    while(out_size<inp_size-1 && out_size<actual_inp_size){
      *c1++=*c2++;
      ++out_size;
    }
    *c1=0;
    if(out_size==inp_size-1 && out_size<actual_inp_size)
      overflow=1;
  }else{
    while(*c2 && out_size<inp_size-1){
      *c1++=*c2++;
      ++out_size;
    }
    *c1=0;
    if(*c2 && out_size==inp_size-1)
      overflow=1;
  }
}

#if defined(OTL_UNICODE) || (defined(_MSC_VER) && (_MSC_VER >= 1400))
inline void otl_strcpy
 (unsigned char* trg,
  const unsigned char* src)
{
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 const OTL_CHAR* c2=OTL_RCAST(const OTL_CHAR*,src);
 while(*c2){
  *c1++=*c2++;
 }
 *c1=0;
}
#else
inline void otl_strcpy(unsigned char* trg,const unsigned char* src)
{
  strcpy(OTL_RCAST(char*,trg),OTL_RCAST(const char*,src));
}
#endif

inline void otl_strcat(char* trg,const char* src)
{
  while(*trg)++trg;
  while(*src){
    *trg=*src;
    ++trg;
    ++src;
  }
  *trg=0;
}

#if defined(OTL_UNICODE) && !defined(OTL_ODBC)
inline void otl_strcpy2(
  unsigned char* trg,
  const unsigned char* src,
  const int max_src_len
)
{
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 const OTL_CHAR* c2=OTL_RCAST(const OTL_CHAR*,src);
 int src_len=OTL_SCAST(int,*OTL_SCAST(const unsigned short*,c2));
 int len=0;
 ++c2;
 while(*c2&&len<max_src_len&&len<src_len){
  *c1++=*c2++;
  ++len;
 }
 *c1=0;
#else
inline void otl_strcpy2(
  unsigned char* trg,
  const unsigned char* src,
  const int /* max_src_len */
)
{
 otl_strcpy(trg,src);
#endif
}

#if defined(OTL_UNICODE)
inline void otl_memcpy(
  unsigned char* trg,
  unsigned char* src,
  const int src_len,
  const int ftype
)
{
  if(ftype==otl_var_raw_long||ftype==otl_var_raw){
    memcpy(trg,src,src_len);
    return;
  }

  OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
  OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
  int len=0;
  while(len<src_len){
    *c1++=*c2++;
    ++len;
  }

#else
inline void otl_memcpy(
  unsigned char* trg,
  unsigned char* src,
  const int src_len,
  const int /* ftype */
)
{
 memcpy(trg,src,src_len);
#endif
}

#if defined(OTL_UNICODE) && !defined(OTL_ODBC)
inline void otl_strcpy3(
  unsigned char* trg,
  unsigned char* src,
  const int max_src_len,
  int& overflow,
  const int inp_size=0
)
{
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
 int len=0;
 int src_len=OTL_SCAST(int,*OTL_RCAST(unsigned short*,c2));
 ++c2;
 int out_size=0;
 overflow=0;
 while(len<src_len&&len<max_src_len&&out_size<inp_size-1){
  *c1++=*c2++;
  ++out_size;
  ++len;
 }
 *c1=0;
 if(len<src_len&&out_size==inp_size-1)
  overflow=1;
#else
inline void otl_strcpy3(
  unsigned char* trg,
  unsigned char* src,
  const int /* max_src_len */,
  int& overflow,
  const int inp_size=0
)
{
 OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
 OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
 int out_size=0;
 overflow=0;
 while(*c2&&out_size<inp_size-1){
  *c1++=*c2++;
  ++out_size;
 }
 *c1=0;
 if(*c2&&out_size==inp_size-1)
  overflow=1;
#endif
}

inline void otl_strcpy4(
  unsigned char* trg,
  unsigned char* src,
  int& overflow,
  const int inp_size=0,
  const int actual_inp_size=-1
)
{
#if  defined(OTL_UNICODE) && !defined(OTL_ODBC)
  OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
  OTL_CHAR* bc1=c1;
  ++c1;
  OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
  int out_size=0;
  overflow=0;
  if(actual_inp_size!=-1){
    while(out_size<inp_size-1 && out_size<actual_inp_size){
      *c1++=*c2++;
      ++out_size;
    }
    *OTL_RCAST(unsigned short*,bc1)=OTL_SCAST(unsigned short,out_size);
    if(out_size==inp_size-1 && out_size<actual_inp_size)
      overflow=1;
  }else{
    while(*c2&&out_size<inp_size-1){
     *c1++=*c2++;
     ++out_size;
    }
    *OTL_RCAST(unsigned short*,bc1)=OTL_SCAST(unsigned short,out_size);
    if(*c2&&out_size==inp_size-1)
      overflow=1;
  }
#else
  OTL_CHAR* c1=OTL_RCAST(OTL_CHAR*,trg);
  OTL_CHAR* c2=OTL_RCAST(OTL_CHAR*,src);
  int out_size=0;
  overflow=0;
  if(actual_inp_size!=-1){
    while(out_size<inp_size-1 && out_size<actual_inp_size){
      *c1++=*c2++;
      ++out_size;
    }
    *c1=0;
    if(out_size==inp_size-1 && out_size<actual_inp_size)
      overflow=1;
  }else{
    while(*c2&&out_size<inp_size-1){
      *c1++=*c2++;
      ++out_size;
    }
    *c1=0;
    if(*c2&&out_size==inp_size-1)
      overflow=1;
  }
#endif
}

inline char* otl_itoa(int i,char* a)
{
  const char* digits="0123456789";
  int n=i;
  int k;
  char buf[64];
  char* c=buf;
  char *c1=a;
  int klen=0;
  char digit=' ';
  bool negative=false;
  if(n<0){
    n=-n;
    negative=true;
  }
  do{
    if(n>=10)
      k=n%10;
    else
      k=n;
    digit=digits[k];
    *c=digit;
    ++c;
    ++klen;
    n=n/10;
  }while(n!=0);
  *c=0;
  if(negative){
    *c1='-';
    ++c1;
  }
  for(int j=klen-1;j>=0;--j){
    *c1=buf[j];
    ++c1;
  }
  *c1=0;
  return c1;
}

inline void otl_var_info_col
(const int pos,
 const int ftype,
 const int type_code,
 char* var_info,
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400) // VC++ 8.0 or higher
 const size_t var_info_sz
#else
 const size_t /*var_info_sz*/
#endif
#else
 const size_t /*var_info_sz*/
#endif
)
{
 char buf1[128];
 char buf2[128];
 char name[128];

 otl_itoa(pos,name);
 OTL_STRCPY_S(buf1,sizeof(buf1),otl_var_type_name(ftype));
 OTL_STRCPY_S(buf2,sizeof(buf2),otl_var_type_name(type_code));
 OTL_STRCPY_S(var_info,var_info_sz,"Column: ");
 OTL_STRCAT_S(var_info,var_info_sz,name);
 OTL_STRCAT_S(var_info,var_info_sz,"<");
 OTL_STRCAT_S(var_info,var_info_sz,buf1);
 OTL_STRCAT_S(var_info,
              var_info_sz,
              ">, datatype in operator <</>>: ");
 OTL_STRCAT_S(var_info,var_info_sz,buf2);
}

inline void otl_var_info_col2
(const int pos,
 const int ftype,
 char* var_info,
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400) // VC++ 8.0 or higher
 const size_t var_info_sz
#else
 const size_t /*var_info_sz*/
#endif
#else
 const size_t /*var_info_sz*/
#endif
)
{
  char buf1[128];
  char name[128];
  
  otl_itoa(pos,name);
  OTL_STRCPY_S(buf1,sizeof(buf1),otl_var_type_name(ftype));
  OTL_STRCPY_S(var_info,var_info_sz,"Column: ");
  OTL_STRCAT_S(var_info,var_info_sz,name);
  OTL_STRCAT_S(var_info,var_info_sz,"<");
  OTL_STRCAT_S(var_info,var_info_sz,buf1);
  OTL_STRCAT_S(var_info,var_info_sz,">");
}

inline void otl_var_info_col3
(const int pos,
 const int ftype,
 const char* col_name,
 char* var_info,
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400) // VC++ 8.0 or higher
 const size_t var_info_sz
#else
 const size_t /*var_info_sz*/
#endif
#else
 const size_t /*var_info_sz*/
#endif
)
{
  char buf1[128];
  char name[128];
  
  otl_itoa(pos,name);
  OTL_STRCPY_S(buf1,sizeof(buf1),otl_var_type_name(ftype));
  OTL_STRCPY_S(var_info,var_info_sz,"Column: ");
  OTL_STRCAT_S(var_info,var_info_sz,name);
  OTL_STRCAT_S(var_info,var_info_sz," / ");
  OTL_STRCAT_S(var_info,var_info_sz,col_name);
  OTL_STRCAT_S(var_info,var_info_sz," <");
  OTL_STRCAT_S(var_info,var_info_sz,buf1);
  OTL_STRCAT_S(var_info,var_info_sz,">");
}

class otl_pl_tab_generic{
public:

  otl_pl_tab_generic():
    p_v(nullptr),
    p_null(nullptr),
    elem_size(0),
    tab_size(0),
    tab_len(0),
    vtype(0)
 {
 }

 virtual ~otl_pl_tab_generic(){}

 unsigned char* val(int ndx=0)
 {
  return p_v+(ndx*elem_size);
 }

 int is_null(int ndx=0)
 {
  return p_null[ndx]!=0;
 }

 void set_null(int ndx=0)
 {
  p_null[ndx]=1;
 }

 void set_non_null(int ndx=0)
 {
  p_null[ndx]=0;
 }

 void init_generic(void)
 {int i;
  memset(p_v,0,elem_size*tab_len);
  for(i=0;i<tab_len;++i)
   p_null[i]=0;
 }

 int len()
 {
  return tab_len;
 }

 void set_len(int new_len=0)
 {
  tab_len=new_len;
 }

  int get_vtype() const
  {
    return vtype;
  }

  int get_elem_size() const
  {
    return elem_size;
  }

  int get_tab_size() const
  {
    return tab_size;
  }

  unsigned char* get_p_v()
  {
    return p_v;
  }

protected:

 unsigned char* p_v;
 short* p_null;
 int elem_size;
 int tab_size;
 int tab_len;
 int vtype;

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_pl_tab_generic(const otl_pl_tab_generic&) = delete;
  otl_pl_tab_generic& operator=(const otl_pl_tab_generic&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_pl_tab_generic(otl_pl_tab_generic&&) = delete;
  otl_pl_tab_generic& operator=(otl_pl_tab_generic&&) = delete;
#endif

private:
#else

  otl_pl_tab_generic(const otl_pl_tab_generic&):
    p_v(nullptr),
    p_null(nullptr),
    elem_size(0),
    tab_size(0),
    tab_len(0),
    vtype(0)
 {
 }

  otl_pl_tab_generic& operator=(const otl_pl_tab_generic&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_pl_tab_generic(otl_pl_tab_generic&&):
    p_v(nullptr),
    p_null(nullptr),
    elem_size(0),
    tab_size(0),
    tab_len(0),
    vtype(0)
 {
 }

  otl_pl_tab_generic& operator=(otl_pl_tab_generic&&)
  {
    return *this;
  }
#endif

#endif
};

#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
template<OTL_TYPE_NAME T,const int T_type>
inline int otl_numeric_convert_T
(const int ftype,const void* val,T& n)
{
  if(ftype==T_type){
    n=*OTL_RCAST(T*,OTL_CCAST(void*,val));
    return 1;
  }else
    return 0;
}

template<OTL_TYPE_NAME T>
inline int otl_numeric_convert_T2(const int ftype,const void* val,T& n)
{
  int rc=1;
  switch(ftype){
  case otl_var_double:
    n=OTL_PCONV(T,double,val);
    break;
  case otl_var_short:
    n=OTL_PCONV(T,short,val);
    break;
  case otl_var_int:
    n=OTL_PCONV(T,int,val);
    break;
  case otl_var_unsigned_int:
    n=OTL_PCONV(T,unsigned int,val);
    break;
  case otl_var_long_int:
    n=OTL_PCONV(T,long int,val);
    break;
  case otl_var_float:
    n=OTL_PCONV(T,float,val);
    break;
  case otl_var_bfloat:
    n=OTL_PCONV(T,float,val);
    break;
  case otl_var_bdouble:
    n=OTL_PCONV(T,double,val);
    break;
#if defined(OTL_BIGINT)
  case otl_var_bigint:
    n=OTL_PCONV(T,OTL_BIGINT,val);
    break;
#endif
#if defined(OTL_UBIGINT)
  case otl_var_ubigint:
    n=OTL_PCONV(T,OTL_UBIGINT,val);
    break;
#endif
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
  case otl_var_numeric_type_1:
    n=OTL_PCONV(T,OTL_NUMERIC_TYPE_1,val);
    break;
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
  case otl_var_numeric_type_2:
    n=OTL_PCONV(T,OTL_NUMERIC_TYPE_2,val);
    break;
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
  case otl_var_numeric_type_3:
    n=OTL_PCONV(T,OTL_NUMERIC_TYPE_3,val);
    break;
#endif
 default:
  rc=0;
  break;
 }
 return rc;  
}

#else
template<OTL_TYPE_NAME T>
inline int otl_numeric_convert_T(const int ftype,const void* val,T& n)
{
  int rc=1;
  switch(ftype){
  case otl_var_double:
    n=OTL_PCONV(T,double,val);
    break;
  case otl_var_short:
    n=OTL_PCONV(T,short,val);
    break;
  case otl_var_int:
    n=OTL_PCONV(T,int,val);
    break;
  case otl_var_unsigned_int:
    n=OTL_PCONV(T,unsigned int,val);
    break;
  case otl_var_long_int:
    n=OTL_PCONV(T,long int,val);
    break;
  case otl_var_float:
    n=OTL_PCONV(T,float,val);
    break;
  case otl_var_bfloat:
    n=OTL_PCONV(T,float,val);
    break;
  case otl_var_bdouble:
    n=OTL_PCONV(T,double,val);
    break;
#if defined(OTL_BIGINT)
  case otl_var_bigint:
    n=OTL_PCONV(T,OTL_BIGINT,val);
    break;
#endif
#if defined(OTL_UBIGINT)
  case otl_var_ubigint:
    n=OTL_PCONV(T,OTL_UBIGINT,val);
    break;
#endif
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
  case otl_var_numeric_type_1:
    n=OTL_PCONV(T,OTL_NUMERIC_TYPE_1,val);
    break;
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
  case otl_var_numeric_type_2:
    n=OTL_PCONV(T,OTL_NUMERIC_TYPE_2,val);
    break;
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
  case otl_var_numeric_type_3:
    n=OTL_PCONV(T,OTL_NUMERIC_TYPE_3,val);
    break;
#endif
 default:
  rc=0;
  break;
 }
 return rc;  
}
#endif

#if defined(OTL_STL) && defined(OTL_STREAM_POOLING_ON)

class otl_ltstr{
public:
 
 bool operator()(const OTL_STRING_CONTAINER& s1, const OTL_STRING_CONTAINER& s2) const
 {
  return strcmp(s1.c_str(), s2.c_str()) < 0;
 }
 
};

const int otl_max_default_pool_size=32;

#endif

#if defined(OTL_UNICODE_STRING_TYPE) && defined(OTL_STREAM_POOLING_ON)
#include <string>
class otl_ltstr{
public:
 
 bool operator()(const std::string& s1, const std::string& s2) const
 {
  return strcmp(s1.c_str(), s2.c_str()) < 0;
 }
 
};

const int otl_max_default_pool_size=32;
#endif

#if defined(OTL_ACE)
const int otl_max_default_pool_size=32;
#endif


class otl_stream_shell_generic{
public:

  otl_stream_shell_generic():
    should_delete(0)
 {
 }

 virtual ~otl_stream_shell_generic(){}

  int get_should_delete() const
  {
    return should_delete;
  }

  void set_should_delete(const int ashould_delete)
  {
    should_delete=ashould_delete;
  }

protected:

 int should_delete;

};

#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)

#if defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)
#include <map>
#include <vector>
#endif

class otl_stream_pool;

class otl_stream_pool_entry{
public:

  friend class otl_stream_pool;

#if defined(OTL_ACE)
 otl_tmpl_vector<otl_stream_shell_generic*> s;
#elif defined(OTL_UNICODE_STRING_TYPE)
 std::vector<otl_stream_shell_generic*> s;
#else
 STD_NAMESPACE_PREFIX vector<otl_stream_shell_generic*> s;
#endif

  otl_stream_pool_entry():
    s(),
    cnt(0)
 {
 }
 
  otl_stream_pool_entry(const otl_stream_pool_entry& sc):
    s(),
    cnt(0)
 {
   copy(sc);
 }
 
 otl_stream_pool_entry& operator=(const otl_stream_pool_entry& sc)
 {
   copy(sc);
   return *this;
 }
 
 virtual ~otl_stream_pool_entry(){}

  int get_cnt() const
  {
    return cnt;
  }

  void set_cnt(const int acnt)
  {
    cnt=acnt;
  }

private:

 int cnt;

  void copy(const otl_stream_pool_entry& sc)
  {
    s.clear();
#if defined(OTL_ACE)
    for(int i=0;i<sc.s.size();++i)
#else
    for(size_t i=0;i<sc.s.size();++i)
#endif
      s.push_back(sc.s[i]);
    cnt=sc.cnt;
  }

};

class otl_stream_pool{
public:
 
 typedef otl_stream_pool_entry cache_entry_type;
#if defined(OTL_ACE)
 typedef
 ACE_RB_Tree
   <OTL_STRING_CONTAINER,cache_entry_type,
    ACE_Less_Than<OTL_STRING_CONTAINER>,
    ACE_Null_Mutex> sc_type;
 typedef otl_tmpl_vector<otl_stream_shell_generic*> vec_type;
 typedef ACE_RB_Tree_Node<OTL_STRING_CONTAINER,cache_entry_type> ace_map_entry;
#elif defined(OTL_UNICODE_STRING_TYPE)
  typedef std::map<std::string,cache_entry_type,otl_ltstr> sc_type;
  typedef std::vector<otl_stream_shell_generic*> vec_type;
#else
  typedef STD_NAMESPACE_PREFIX
  map<OTL_STRING_CONTAINER,cache_entry_type,otl_ltstr> sc_type;
  typedef STD_NAMESPACE_PREFIX vector<otl_stream_shell_generic*> vec_type;
#endif

protected:

  sc_type sc;
  bool pool_enabled_;
  int max_size;
  int size;

public:

  otl_stream_pool():
    sc(),
    pool_enabled_(true),
    max_size(otl_max_default_pool_size),
    size(0)
 {
 }

 void init(int amax_size=otl_max_default_pool_size)
 {
  if(size==0&&max_size==0)return;
  if(amax_size<2)
    amax_size=2;
#if defined(OTL_ACE)
  sc_type::iterator elem0=sc.begin();
  sc_type::iterator elemN=sc.end();
  for(sc_type::iterator i=elem0; i!=elemN; ++i){
   cache_entry_type& ce=(*i).item();
   int sz=ce.s.size();
   for(int j=0;j<sz;++j){
     ce.s[j]->set_should_delete(1);
    delete ce.s[j];
    ce.s[j]=nullptr;
   }
   ce.s.clear();
   ce.cnt=0;
  }
  sc.clear();
#else
  sc_type::iterator elem0=sc.begin();
  sc_type::iterator elemN=sc.end();
  for(sc_type::iterator i=elem0; i!=elemN; ++i){
   cache_entry_type& ce=(*i).second;
   size_t sz=ce.s.size();
   for(size_t j=0;j<sz;++j){
     ce.s[j]->set_should_delete(1);
     delete ce.s[j];
     ce.s[j]=nullptr;
   }
   ce.s.clear();
   ce.set_cnt(0);
  }
  sc.clear();
#endif

  size=0;
  max_size=amax_size;

 }
#if defined(OTL_UNICODE_STRING_TYPE)
 otl_stream_shell_generic* find(const std::string& stmtxt)
#else
 otl_stream_shell_generic* find(const OTL_STRING_CONTAINER& stmtxt)
#endif
 {
  otl_stream_shell_generic* s;
  
#if defined(OTL_ACE)
  ace_map_entry* ce=0;
  int found=sc.find(stmtxt,ce);
  if(found==-1)return 0; // entry not found
  s=ce->item().s[ce->item().s.size()-1];
  ce->item().s.pop_back();
  if(ce->item().s.size()==0){
   sc.unbind(ce);
   --size;
  }
#else
  sc_type::iterator cur=sc.find(stmtxt);
  if(cur==sc.end())
    return nullptr; // entry not found
  cache_entry_type& ce=(*cur).second;
  s=ce.s[ce.s.size()-1];
  ce.s.pop_back();
  if(ce.s.size()==0){
   sc.erase(cur);
   --size;
  }
#endif

  return s;
 }

#if defined(OTL_UNICODE_STRING_TYPE)
 void remove(const otl_stream_shell_generic* s,const std::string& stmtxt)
#else
 void remove(const otl_stream_shell_generic* s,const OTL_STRING_CONTAINER& stmtxt)
#endif

 {
#if defined(OTL_ACE)
  ace_map_entry* cur=0;
  int found=sc.find(stmtxt,cur);
  if(found==-1)
   return;
  cache_entry_type& ce=(*cur).item();
  for(int i=0;i<ce.s.size();++i)
   if(ce.s[i]==s){
    if(ce.s.size()>1 && i!=ce.s.size()-1){
     otl_stream_shell_generic* temp_s=ce.s[i];
     ce.s[i]=ce.s[ce.s.size()-1];
     ce.s[ce.s.size()-1]=temp_s;
    }
    ce.s.pop_back();
    --size;
    return;
   }
#else
  sc_type::iterator cur=sc.find(stmtxt);
  if(cur==sc.end())
   return;
  cache_entry_type& ce=(*cur).second;
  vec_type::iterator bgn=ce.s.begin();
  vec_type::iterator end=ce.s.end();
  for(vec_type::iterator i=bgn;i!=end;++i)
   if((*i)==s){
    ce.s.erase(i);
    --size;
    return;
   }
#endif
 }

  int get_max_size() const
  {
    return max_size;
  }

 void add(otl_stream_shell_generic* s,const char* stm_text)
 {
#if defined(OTL_UNICODE_STRING_TYPE)
  std::string stmtxt(stm_text);
#else
  OTL_STRING_CONTAINER stmtxt(stm_text);
#endif

#if defined(OTL_ACE)

  ace_map_entry* cur=0;
  int found_in_map=sc.find(stmtxt,cur);
  if(found_in_map==0){ // entry found
   bool found=false;
   cache_entry_type& ce=(*cur).item();
   int sz=ce.s.size();
   for(int i=0;i<sz;++i){
    if(s==ce.s[i]){
     found=true;
     break;
    }
   }
   if(!found)
     ce.s.push_back(s);
   ++ce.cnt;
  }else{ // entry not found
   if(size<max_size-1){ // add new entry
    cache_entry_type ce;
    ce.s.push_back(s);
    ce.cnt=1;
    sc.bind(stmtxt,ce);
    ++size;
   }else{ // erase the least used entry and add new one

    sc_type::iterator elem0=sc.begin();
    sc_type::iterator elemN=sc.end();
    int min_cnt=0;
    ace_map_entry* min_entry=0;
    
    for(sc_type::iterator i=elem0;i!=elemN;++i){
     if(i==elem0){ // first element
      min_entry=&(*i);
      min_cnt=(*i).item().cnt;
     }
     if(min_cnt>(*i).item().cnt){ // found less used entry
      min_entry=&(*i);
      min_cnt=(*i).item().cnt;
     }
    }
    cache_entry_type& me=(*min_entry).item();
    int sz=me.s.size();
    for(int n=0;n<sz;++n){
      me.s[n]->set_should_delete(1);
      otl_stream_shell_generic* tmp=me.s[n];
      delete tmp;
    }
    me.s.clear();
    sc.unbind(min_entry);
    cache_entry_type ce;
    ce.cnt=1;
    ce.s.push_back(s);
    sc.bind(stmtxt,ce);
   }
  }

#else

  sc_type::iterator cur=sc.find(stmtxt);

  if(cur!=sc.end()){ // entry found
   bool found=false;
   cache_entry_type& ce=(*cur).second;
   size_t sz=ce.s.size();
   for(size_t i=0;i<sz;++i){
    if(s==ce.s[i]){
     found=true;
     break;
    }
   }
   if(!found)ce.s.push_back(s);
   ce.set_cnt(ce.get_cnt()+1);
  }else{ // entry not found
   if(size<max_size-1){ // add new entry
    cache_entry_type ce;
    ce.s.push_back(s);
    ce.set_cnt(1);
    sc[stmtxt]=ce;
    ++size;
   }else{ // erase the least used entry and add new one

    sc_type::iterator elem0=sc.begin();
    sc_type::iterator elemN=sc.end();
    int min_cnt=0;
    sc_type::iterator min_entry;
    
    for(sc_type::iterator i=elem0;i!=elemN;++i){
     if(i==elem0){ // first element
      min_entry=i;
      min_cnt=(*i).second.get_cnt();
     }
     if(min_cnt>(*i).second.get_cnt()){ // found less used entry
      min_entry=i;
      min_cnt=(*i).second.get_cnt();
     }
    }
    cache_entry_type& me=(*min_entry).second;
    size_t sz=me.s.size();
    for(size_t n=0;n<sz;++n){
      me.s[n]->set_should_delete(1);
     otl_stream_shell_generic* tmp=me.s[n];
     delete tmp;
    }
    me.s.clear();
    sc.erase(min_entry);
    cache_entry_type ce;
    ce.set_cnt(1);
    ce.s.push_back(s);
    sc[stmtxt]=ce;
   }
  }
#endif
 }
 
 virtual ~otl_stream_pool()
 {
  init();
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_stream_pool(const otl_stream_pool&) = delete;
  otl_stream_pool& operator=(const otl_stream_pool&) = delete;
#if defined(OTL_ANSI_CPP_11_RVAL_REF_SUPPORT)
  otl_stream_pool(otl_stream_pool&&) = delete;
  otl_stream_pool& operator=(otl_stream_pool&&) = delete;
#endif
private:
#else
  otl_stream_pool(const otl_stream_pool&):
    sc(),
    pool_enabled_(true),
    max_size(0),
    size(0)
 {
 }

 otl_stream_pool& operator=(const otl_stream_pool&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_stream_pool(otl_stream_pool&&):
    sc(),
    pool_enabled_(true),
    max_size(0),
    size(0)
 {
 }

 otl_stream_pool& operator=(otl_stream_pool&&)
 {
   return *this;
 }
#endif
#endif
 
};

#endif


// =========================== COMMON TEMPLATES  ============================


#if (defined(OTL_STL)||defined(OTL_VALUE_TEMPLATE_ON)) && defined(OTL_VALUE_TEMPLATE)

template <OTL_TYPE_NAME TData>
class otl_value{
public:

 TData v;
 bool ind;

  otl_value():
    v(),
   ind(true)
 {
 }

 virtual ~otl_value(){}

 otl_value(const otl_value<TData>& var):
   v(var.v),
   ind(var.ind)
 {
 }

  otl_value(const TData& var):
   v(var),
   ind(false)
 {
 }

  otl_value(const otl_null&):
   v(),
   ind(true)
 {
 }

 otl_value<TData>& operator=(const otl_value<TData>& var)
 {
  v=var.v;
  ind=var.ind;
  return *this;
 }

 otl_value<TData>& operator=(const TData& var)
 {
  v=var;
  ind=false;
  return *this;
 }

 otl_value<TData>& operator=(const otl_null&)
 {
  ind=true;
  return *this;
 }

 bool is_null(void)const {return ind;}
 void set_null(void){ind=true;}
 void set_non_null(void){ind=false;}

};

template <OTL_TYPE_NAME TData>
STD_NAMESPACE_PREFIX ostream& operator<<
  (STD_NAMESPACE_PREFIX ostream& s, 
   const otl_value<TData>& var)
{
 if(var.ind)
  s<<"NULL";
 else
  s<<var.v;
 return s;
}

#if defined(OTL_DISABLE_OPERATOR_GT_GT_FOR_OTL_VALUE_OTL_DATETIME)
#else
inline STD_NAMESPACE_PREFIX ostream& operator<<(
 STD_NAMESPACE_PREFIX ostream& s, 
 const otl_value<otl_datetime>& var)
{
 if(var.ind)
   s<<"NULL";
 else{
#if !defined(OTL_TRACE_LEVEL)
   s<<var.v.month<<"/"<<var.v.day<<"/"<<var.v.year 
    <<" "<<var.v.hour<<":"<<var.v.minute
    <<":"<<var.v.second<<"."<<var.v.fraction;
#else
   s<<OTL_TRACE_FORMAT_DATETIME(var.v);
#endif
 }
 return s;
}
#endif

#endif

template <OTL_TYPE_NAME OTLStream,
          OTL_TYPE_NAME OTLConnect,
          OTL_TYPE_NAME otl_exception>
class otl_tmpl_nocommit_stream: public OTLStream{
public:

 otl_tmpl_nocommit_stream() OTL_NO_THROW
   : OTLStream()
 {
  OTLStream::set_commit(0);
 }

 otl_tmpl_nocommit_stream
 (const otl_stream_buffer_size_type arr_size, 
  const char* sqlstm,
  OTLConnect& pdb,
  const char* ref_cur_placeholder=nullptr)
   OTL_THROWS_OTL_EXCEPTION
  : OTLStream(arr_size,sqlstm,pdb,ref_cur_placeholder)
 {
  OTLStream::set_commit(0);
 }

 void open
 (otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  OTLConnect& db,
  const char* ref_cur_placeholder=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  OTLStream::open(arr_size,sqlstm,db,ref_cur_placeholder);
  OTLStream::set_commit(0);
 }

};

#if defined(OTL_STL)

class otl_pl_vec_generic{
public:

 typedef STD_NAMESPACE_PREFIX vector<bool> null_flag_type;


 otl_pl_vec_generic():
   p_v(nullptr),
   null_flag(),
   vtype(0),
   elem_size(0)
 {
 }

 virtual int len(void) const
 {
  return 0;
 }

  virtual void set_len(const int /*new_len*/=0,
                       const bool /*set_all_to_null*/=true)

 {
 }

 bool is_null(const int ndx=0) const
 {
  return null_flag[ndx];
 }

 void set_null(const int ndx=0)
 {
  null_flag[ndx]=true;
 }

 void set_non_null(const int ndx=0)
 {
  null_flag[ndx]=false;
 }

 virtual ~otl_pl_vec_generic(){}

  int get_vtype() const
  {
    return vtype;
  }

  int get_elem_size() const
  {
    return elem_size;
  }

  void* get_p_v()
  {
    return p_v;
  }

protected:

 void* p_v;
 null_flag_type null_flag;
 int vtype;
 int elem_size;

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_pl_vec_generic(const otl_pl_vec_generic&) = delete;
 otl_pl_vec_generic& operator=(const otl_pl_vec_generic&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_pl_vec_generic(otl_pl_vec_generic&&) = delete;
 otl_pl_vec_generic& operator=(otl_pl_vec_generic&&) = delete;
#endif
private:
#else
 otl_pl_vec_generic(const otl_pl_vec_generic&):
   p_v(nullptr),
   null_flag(),
   vtype(0),
   elem_size(0)
 {
 }

 otl_pl_vec_generic& operator=(const otl_pl_vec_generic&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_pl_vec_generic(otl_pl_vec_generic&&):
   p_v(nullptr),
   null_flag(),
   vtype(0),
   elem_size(0)
 {
 }

 otl_pl_vec_generic& operator=(otl_pl_vec_generic&&)
 {
   return *this;
 }
#endif
#endif

};

template<OTL_TYPE_NAME T,const int type_code,const int T_sz>
class otl_T_vec: public otl_pl_vec_generic{
public:

  STD_NAMESPACE_PREFIX vector<T> v;
  typedef T value_type;

  otl_T_vec():
    v()
  {
    this->p_v=OTL_RCAST(void*,&v);
    this->vtype=type_code;
    this->elem_size=T_sz;
  }

  virtual ~otl_T_vec(){}

 virtual void set_len
   (const int new_len=0,
    const bool set_all_to_null=true)
 {int i,vsize;

  v.resize(new_len);
  this->null_flag.resize(new_len);
  vsize=OTL_SCAST(int,v.size());
  if(set_all_to_null)
    for(i=0;i<vsize;++i)
      this->null_flag[i]=true;
 }

 virtual int len(void) const
 {
  return OTL_SCAST(int,v.size());
 }

 T& operator[](int ndx)
 {
  return v[ndx];
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_T_vec(const otl_T_vec&) = delete;
 otl_T_vec& operator=(const otl_T_vec&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_T_vec(otl_T_vec&&) = delete;
 otl_T_vec& operator=(otl_T_vec&&) = delete;
#endif
private:
#else
 otl_T_vec(const otl_T_vec&):
    v()
 {
 }

 otl_T_vec& operator=(const otl_T_vec&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_T_vec(otl_T_vec&&):
    v()
 {
 }

 otl_T_vec& operator=(otl_T_vec&&)
 {
   return *this;
 }
#endif
#endif

};

typedef otl_T_vec<double,otl_var_double,sizeof(double)> otl_double_vec;
typedef otl_T_vec<float,otl_var_float,sizeof(float)> otl_float_vec;
typedef otl_T_vec<int,otl_var_int,sizeof(int)> otl_int_vec;
typedef otl_T_vec<short,otl_var_short,sizeof(short)> otl_short_vec;
typedef otl_T_vec<long,otl_var_long_int,sizeof(long)> otl_long_int_vec;
typedef otl_T_vec<otl_datetime,otl_var_timestamp,
                  sizeof(otl_oracle_date)> otl_datetime_vec;
typedef otl_T_vec<OTL_STRING_CONTAINER,otl_var_char,1> otl_string_vec;

#endif

template <OTL_TYPE_NAME T,const int atab_size,const int avtype>
class otl_tmpl_pl_tab: public otl_pl_tab_generic{
public:

 T v[atab_size];

 void init(void)
 {int i;
  tab_len=0;
  vtype=avtype;
  tab_size=atab_size;
  p_null=null_flag;
  p_v=OTL_RCAST(unsigned char*,v);
  elem_size=sizeof(T);
  for(i=0;i<atab_size;++i)
   null_flag[i]=0;
  memset(v,0,sizeof(v));
 }

 otl_tmpl_pl_tab():
   v(),
   null_flag()
 {
  init();
 }

 virtual ~otl_tmpl_pl_tab(){}

private:

 short null_flag[atab_size];

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_tmpl_pl_tab& operator=(const otl_tmpl_pl_tab&) = delete;
  otl_tmpl_pl_tab(const otl_tmpl_pl_tab&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_tmpl_pl_tab& operator=(otl_tmpl_pl_tab&&) = delete;
  otl_tmpl_pl_tab(otl_tmpl_pl_tab&&) = delete;
#endif
private:
#else
 otl_tmpl_pl_tab& operator=(const otl_tmpl_pl_tab&)
 {
   return *this;
 }

  otl_tmpl_pl_tab(const otl_tmpl_pl_tab&):
   v(),
   null_flag()
 {
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_tmpl_pl_tab& operator=(otl_tmpl_pl_tab&&)
 {
   return *this;
 }

  otl_tmpl_pl_tab(otl_tmpl_pl_tab&&):
   v(),
   null_flag()
 {
 }
#endif
#endif

};

template <const int atab_size>
class otl_int_tab: public otl_tmpl_pl_tab<int,atab_size,otl_var_int>{
public:
 otl_int_tab():otl_tmpl_pl_tab<int,atab_size,otl_var_int>(){}
};

template <const int atab_size>
class otl_double_tab: public otl_tmpl_pl_tab<double,atab_size,otl_var_double>{
public:
 otl_double_tab():otl_tmpl_pl_tab<double,atab_size,otl_var_double>(){}
};

template <const int atab_size>
class otl_float_tab: public otl_tmpl_pl_tab<float,atab_size,otl_var_float>{
public:
 otl_float_tab():otl_tmpl_pl_tab<float,atab_size,otl_var_float>(){}
};

template <const int atab_size>
class otl_unsigned_tab: public otl_tmpl_pl_tab<unsigned,atab_size,otl_var_unsigned_int>{
public:
 otl_unsigned_tab():otl_tmpl_pl_tab<unsigned,atab_size,otl_var_unsigned_int>(){}
};

template <const int atab_size>
class otl_short_tab: public otl_tmpl_pl_tab<short,atab_size,otl_var_short>{
public:
 otl_short_tab():otl_tmpl_pl_tab<short,atab_size,otl_var_short>(){}
};

template <const int atab_size>
class otl_long_int_tab: public otl_tmpl_pl_tab<long,atab_size,otl_var_long_int>{
public:
 otl_long_int_tab():otl_tmpl_pl_tab<long,atab_size,otl_var_long_int>(){}
};

template <const int atab_size,const int str_size>
class otl_cstr_tab: public otl_pl_tab_generic{
public:
 typedef unsigned char T[str_size];
 T v[atab_size];

 void init(void)
 {int i;
  tab_len=0;
  vtype=otl_var_char;
  tab_size=atab_size;
  p_null=null_flag;
  p_v=OTL_RCAST(unsigned char*,v);
  elem_size=sizeof(T);
  for(i=0;i<atab_size;++i)
   null_flag[i]=0;
  memset(v,0,sizeof(v));
 }

  otl_cstr_tab():
    v(),
    null_flag()
 {
  init();
 }

 virtual ~otl_cstr_tab(){}

private:

 short null_flag[atab_size];

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_cstr_tab(const otl_cstr_tab&) = delete;
  otl_cstr_tab& operator=(const otl_cstr_tab&) = delete;

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_cstr_tab(otl_cstr_tab&&) = delete;
  otl_cstr_tab& operator=(otl_cstr_tab&&) = delete;
#endif
private:
#else
  otl_cstr_tab(const otl_cstr_tab&):
    v(),
    null_flag()
 {
 }

 otl_cstr_tab& operator=(const otl_cstr_tab&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_cstr_tab(otl_cstr_tab&&):
    v(),
    null_flag()
 {
 }

 otl_cstr_tab& operator=(otl_cstr_tab&&)
 {
   return *this;
 }
#endif
#endif

};

template <const int atab_size>
class otl_datetime_tab: public otl_pl_tab_generic{
public:

 typedef otl_datetime T;

 T v[atab_size];

 void init(void)
 {int i;
  tab_len=0;
  vtype=otl_var_timestamp;
  tab_size=atab_size;
  p_null=null_flag;
  p_v=OTL_RCAST(unsigned char*,v);
  elem_size=sizeof(otl_oracle_date);
  for(i=0;i<atab_size;++i)
   null_flag[i]=0;
 }

  otl_datetime_tab():
    v(),
    null_flag()
 {
  init();
 }

 virtual ~otl_datetime_tab(){}

private:

 short null_flag[atab_size];

};

template <OTL_TYPE_NAME T,const int avtype>
class otl_tmpl_dyn_pl_tab: public otl_pl_tab_generic{
public:
 T* v;

 void init(const int atab_size=1)
 {int i;
  tab_len=0;
  vtype=avtype;
  tab_size=atab_size;
  v=new T[tab_size];
  null_flag=new short[tab_size];
  p_null=null_flag;
  p_v=OTL_RCAST(unsigned char*,v);
  elem_size=sizeof(T);
  for(i=0;i<atab_size;++i)
   null_flag[i]=0;
  memset(v,0,elem_size*tab_size);
 }

  otl_tmpl_dyn_pl_tab(const int atab_size=1):
    v(nullptr),
    null_flag(nullptr)
 {
  init(atab_size);
 }

 virtual ~otl_tmpl_dyn_pl_tab()
 {
  delete[] v;
  delete[] null_flag;
 }

private:

  short* null_flag;

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_tmpl_dyn_pl_tab(const otl_tmpl_dyn_pl_tab<T,avtype>&) = delete;
  otl_tmpl_dyn_pl_tab<T,avtype>& operator=(const otl_tmpl_dyn_pl_tab<T,avtype>&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_tmpl_dyn_pl_tab(otl_tmpl_dyn_pl_tab<T,avtype>&&) = delete;
  otl_tmpl_dyn_pl_tab<T,avtype>& operator=(otl_tmpl_dyn_pl_tab<T,avtype>&&) = delete;
#endif
private:
#else
  otl_tmpl_dyn_pl_tab(const otl_tmpl_dyn_pl_tab<T,avtype>&):
    v(nullptr),
    null_flag(nullptr)
  {
  }

  otl_tmpl_dyn_pl_tab<T,avtype>& operator=(const otl_tmpl_dyn_pl_tab<T,avtype>&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_tmpl_dyn_pl_tab(otl_tmpl_dyn_pl_tab<T,avtype>&&):
    v(nullptr),
    null_flag(nullptr)
  {
  }

  otl_tmpl_dyn_pl_tab<T,avtype>& operator=(otl_tmpl_dyn_pl_tab<T,avtype>&&)
  {
    return *this;
  }
#endif
#endif

};

class otl_dynamic_int_tab: public otl_tmpl_dyn_pl_tab<int,otl_var_int>{
public:
 otl_dynamic_int_tab(const int atab_size=1)
  :otl_tmpl_dyn_pl_tab<int,otl_var_int>(atab_size){}
};

class otl_dynamic_double_tab: public otl_tmpl_dyn_pl_tab<double,otl_var_double>{
public:
 otl_dynamic_double_tab(const int atab_size=1)
  :otl_tmpl_dyn_pl_tab<double,otl_var_double>(atab_size){}
};

class otl_dynamic_float_tab: public otl_tmpl_dyn_pl_tab<float,otl_var_float>{
public:
 otl_dynamic_float_tab(const int atab_size=1)
  :otl_tmpl_dyn_pl_tab<float,otl_var_float>(atab_size){}
};

class otl_dynamic_unsigned_tab: public
otl_tmpl_dyn_pl_tab<unsigned,otl_var_unsigned_int>{
public:
 otl_dynamic_unsigned_tab(const int atab_size=1)
  :otl_tmpl_dyn_pl_tab<unsigned,otl_var_unsigned_int>(atab_size){}
};

class otl_dynamic_short_tab: public otl_tmpl_dyn_pl_tab<short,otl_var_short>{
public:
 otl_dynamic_short_tab(const int atab_size=1)
  :otl_tmpl_dyn_pl_tab<short,otl_var_short>(atab_size){}
};

class otl_dynamic_long_int_tab: public otl_tmpl_dyn_pl_tab<long,otl_var_long_int>{
public:
 otl_dynamic_long_int_tab(const int atab_size=1)
  :otl_tmpl_dyn_pl_tab<long,otl_var_long_int>(atab_size){}
};

template <const int str_size>
class otl_dynamic_cstr_tab: public otl_pl_tab_generic{
public:
 typedef unsigned char T[str_size];
 T* v;

 void init(const int atab_size=1)
 {int i;
  tab_len=0;
  vtype=otl_var_char;
  tab_size=atab_size;
  v=new T[tab_size];
  null_flag=new short[tab_size];
  p_null=null_flag;
  p_v=OTL_RCAST(unsigned char*,v);
  elem_size=sizeof(T);
  for(i=0;i<atab_size;++i)
   null_flag[i]=0;
  memset(v,0,elem_size*tab_size);
 }

  otl_dynamic_cstr_tab(const int atab_size=1):
    v(nullptr),
    null_flag(nullptr)
 {
  init(atab_size);
 }

 virtual ~otl_dynamic_cstr_tab()
 {
  delete[] v;
  delete[] null_flag;
 }

private:

  short* null_flag;

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_dynamic_cstr_tab(const otl_dynamic_cstr_tab&) = delete;
  otl_dynamic_cstr_tab& operator=(const otl_dynamic_cstr_tab&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_dynamic_cstr_tab(otl_dynamic_cstr_tab&&) = delete;
  otl_dynamic_cstr_tab& operator=(otl_dynamic_cstr_tab&&) = delete;
#endif

private:
#else
  otl_dynamic_cstr_tab(const otl_dynamic_cstr_tab&):
    v(nullptr),
    null_flag(nullptr)
 {
 }

  otl_dynamic_cstr_tab& operator=(const otl_dynamic_cstr_tab&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_dynamic_cstr_tab(otl_dynamic_cstr_tab&&):
    v(nullptr),
    null_flag(nullptr)
 {
 }

  otl_dynamic_cstr_tab& operator=(otl_dynamic_cstr_tab&&)
  {
    return *this;
  }
#endif
#endif

};

class otl_dynamic_datetime_tab: public otl_pl_tab_generic{
public:

 typedef otl_datetime T;

 T* v;

 void init(const int atab_size=1)
 {int i;
  tab_len=0;
  vtype=otl_var_timestamp;
  tab_size=atab_size;
  v=new T[tab_size];
  null_flag=new short[tab_size];
  p_null=null_flag;
  p_v=OTL_RCAST(unsigned char*,v);
  elem_size=sizeof(otl_oracle_date);
  for(i=0;i<atab_size;++i)
   null_flag[i]=0;
 }

 otl_dynamic_datetime_tab(const int atab_size=1):
   otl_pl_tab_generic(),
   v(nullptr),
   null_flag(nullptr)
 {
  init(atab_size);
 }

 virtual ~otl_dynamic_datetime_tab()
 {
  delete[] v;
  delete[] null_flag;
 }

private:

 short* null_flag;

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_dynamic_datetime_tab(const otl_dynamic_datetime_tab&) = delete;
 otl_dynamic_datetime_tab& operator=(const otl_dynamic_datetime_tab&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_dynamic_datetime_tab(otl_dynamic_datetime_tab&&) = delete;
 otl_dynamic_datetime_tab& operator=(otl_dynamic_datetime_tab&&) = delete;
#endif
private:
#else
 otl_dynamic_datetime_tab(const otl_dynamic_datetime_tab&):
   otl_pl_tab_generic(),
   v(nullptr),
   null_flag(nullptr)
 {
 }

 otl_dynamic_datetime_tab& operator=(const otl_dynamic_datetime_tab&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_dynamic_datetime_tab(otl_dynamic_datetime_tab&&):
   otl_pl_tab_generic(),
   v(nullptr),
   null_flag(nullptr)
 {
 }

 otl_dynamic_datetime_tab& operator=(otl_dynamic_datetime_tab&&)
 {
   return *this;
 }
#endif
#endif

};

#define OTL_TMPL_EXCEPTION   \
  otl_tmpl_exception         \
    <TExceptionStruct,       \
     TConnectStruct,         \
     TCursorStruct>

#define OTL_TMPL_CONNECT  \
  otl_tmpl_connect        \
   <TExceptionStruct,     \
    TConnectStruct,       \
    TCursorStruct>

#define OTL_TMPL_CURSOR                   \
    otl_tmpl_cursor                       \
   <TExceptionStruct,TConnectStruct,      \
    TCursorStruct,TVariableStruct>      

#define OTL_TMPL_OUT_STREAM            \
  otl_tmpl_out_stream                  \
   <TExceptionStruct,TConnectStruct,   \
    TCursorStruct,TVariableStruct,     \
    TTimestampStruct>

#define OTL_TMPL_SELECT_CURSOR                          \
 otl_tmpl_select_cursor                                 \
  <TExceptionStruct,TConnectStruct,                     \
   TCursorStruct,TVariableStruct,TSelectCursorStruct>

#define OTL_TMPL_INOUT_STREAM             \
  otl_tmpl_inout_stream                   \
   <TExceptionStruct,TConnectStruct,      \
    TCursorStruct,TVariableStruct,        \
    TTimestampStruct>

#define OTL_TMPL_SELECT_STREAM                      \
 otl_tmpl_select_stream                             \
   <TExceptionStruct,TConnectStruct,TCursorStruct,  \
    TVariableStruct,TSelectCursorStruct,            \
    TTimestampStruct>

#if defined(OTL_EXCEPTION_IS_DERIVED_FROM_STD_EXCEPTION)
#if defined(OTL_EXCEPTION_DERIVED_FROM)
#error OTL_EXCEPTION_DERIVED_FROM is already defined. \
OTL_EXCEPTION_IS_DERIVED_FROM_STD_EXCEPTION cannot be used
#endif
#define OTL_EXCEPTION_DERIVED_FROM std::exception
#if defined(__GNUC__) && (__GNUC__>=3)
#if defined(OTL_ANSI_CPP_11_NOEXCEPT_SUPPORT)
#define OTL_EXCEPTION_HAS_MEMBERS                       \
  virtual const char* what() const noexcept             \
  {                                                     \
    return reinterpret_cast<const char*>(this->msg);    \
  } 
#else
#define OTL_EXCEPTION_HAS_MEMBERS                       \
  virtual const char* what() const throw()              \
  {                                                     \
    return reinterpret_cast<const char*>(this->msg);    \
  } 
#endif
#else
#define OTL_EXCEPTION_HAS_MEMBERS                       \
  virtual const char* what() const throw()              \
  {                                                     \
    return reinterpret_cast<const char*>(this->msg);    \
  } 
#endif
#endif

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct>
#if defined(OTL_EXCEPTION_DERIVED_FROM)
class otl_tmpl_exception: 
  public OTL_EXCEPTION_DERIVED_FROM,
  public TExceptionStruct{
#else
class otl_tmpl_exception: public TExceptionStruct{
#endif
public:

#if defined(OTL_EXCEPTION_HAS_MEMBERS)
  OTL_EXCEPTION_HAS_MEMBERS
#endif

#if defined(OTL_EXCEPTION_STM_TEXT_SIZE)
  char stm_text[OTL_EXCEPTION_STM_TEXT_SIZE];
#else
  char stm_text[2048];
#endif
  char var_info[256];

  otl_tmpl_exception() 
#if defined(__GNUC__) && (__GNUC__>=3)
#if defined(OTL_ANSI_CPP_11_NOEXCEPT_SUPPORT)
   OTL_ANSI_CPP_11_NOEXCEPT
#else
    throw()
#endif
#else
    OTL_NO_THROW
#endif
 {
  stm_text[0]=0;
  var_info[0]=0;
 }

 otl_tmpl_exception(TConnectStruct& conn_struct, const char* sqlstm=nullptr)
#if defined(__GNUC__) && (__GNUC__>=3)
#if defined(OTL_ANSI_CPP_11_NOEXCEPT_SUPPORT)
   OTL_ANSI_CPP_11_NOEXCEPT
#else
    throw()
#endif
#else
    OTL_NO_THROW
#endif
 {
  stm_text[0]=0;
  var_info[0]=0;
  if(sqlstm){
   OTL_STRNCPY_S(OTL_RCAST(char*,stm_text),
                 sizeof(stm_text),
                 sqlstm,
                 sizeof(stm_text)-1);
   stm_text[sizeof(stm_text)-1]=0;
  }
  conn_struct.error(OTL_SCAST(TExceptionStruct&,*this));
  OTL_TRACE_EXCEPTION(this->code,this->msg,this->stm_text,this->var_info)
 }

 otl_tmpl_exception(TCursorStruct& cursor_struct, const char* sqlstm=nullptr)
#if defined(__GNUC__) && (__GNUC__>=3)
#if defined(OTL_ANSI_CPP_11_NOEXCEPT_SUPPORT)
   OTL_ANSI_CPP_11_NOEXCEPT
#else
    throw()
#endif
#else
    OTL_NO_THROW
#endif
 {
  stm_text[0]=0;
  var_info[0]=0;
  if(sqlstm){
   OTL_STRNCPY_S(OTL_RCAST(char*,stm_text),
                 sizeof(stm_text),
                 sqlstm,
                 sizeof(stm_text)-1);
   stm_text[sizeof(stm_text)-1]=0;
  }
  cursor_struct.error(OTL_SCAST(TExceptionStruct&,*this));
  OTL_TRACE_EXCEPTION(this->code,this->msg,this->stm_text,this->var_info)
 }

 otl_tmpl_exception
 (const char* amsg,
  const int acode,
  const char* sqlstm=nullptr,
  const char* varinfo=nullptr)
#if defined(__GNUC__) && (__GNUC__>=3)
#if defined(OTL_ANSI_CPP_11_NOEXCEPT_SUPPORT)
   OTL_ANSI_CPP_11_NOEXCEPT
#else
    throw()
#endif
#else
    OTL_NO_THROW
#endif
 {
  stm_text[0]=0;
  var_info[0]=0;
  if(sqlstm){
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
    OTL_STRNCPY_S(OTL_RCAST(char*,stm_text),
                  sizeof(stm_text),
                  sqlstm,
                  sizeof(stm_text)-1);
#else
    strncpy(OTL_RCAST(char*,stm_text),sqlstm,sizeof(stm_text));
    stm_text[sizeof(stm_text)-1]=0;
#endif
#else
    strncpy(OTL_RCAST(char*,stm_text),sqlstm,sizeof(stm_text));
    stm_text[sizeof(stm_text)-1]=0;
#endif
  }
  if(varinfo)
    OTL_STRCPY_S(OTL_RCAST(char*,var_info),sizeof(var_info),varinfo);
  TExceptionStruct::init(amsg,acode);
  OTL_TRACE_EXCEPTION(this->code,this->msg,this->stm_text,this->var_info)
 }
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW) && \
  !defined(OTL_EXCEPTION_DERIVED_FROM)
#error OTL_EXCEPTION_DERIVED_FROM needs to be defined when \
OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW is defined
#endif
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
 otl_tmpl_exception
 (const char* amsg,
  const int acode,
  const char* sqlstm,
  const char* varinfo,
  const void* input_string,
  int input_string_size)
#if defined(__GNUC__) && (__GNUC__>=3)
#if defined(OTL_ANSI_CPP_11_NOEXCEPT_SUPPORT)
   OTL_ANSI_CPP_11_NOEXCEPT
#else
    throw()
#endif
#else
    OTL_NO_THROW
#endif
 {
  stm_text[0]=0;
  var_info[0]=0;
  if(sqlstm){
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
    OTL_STRNCPY_S(OTL_RCAST(char*,stm_text),
                  sizeof(stm_text),
                  sqlstm,
                  sizeof(stm_text)-1);
#else
    strncpy(OTL_RCAST(char*,stm_text),sqlstm,sizeof(stm_text));
    stm_text[sizeof(stm_text)-1]=0;
#endif
#else
    strncpy(OTL_RCAST(char*,stm_text),sqlstm,sizeof(stm_text));
    stm_text[sizeof(stm_text)-1]=0;
#endif
  }
  if(varinfo)
    OTL_STRCPY_S(OTL_RCAST(char*,var_info),sizeof(var_info),varinfo);
  TExceptionStruct::init(amsg,acode);
  OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW(input_string,input_string_size)
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
  OTL_TRACE_EXCEPTION2(this->code,
                       this->msg,
                       this->stm_text,
                       this->var_info,
                       input_string,
                       input_string_size)
#else
  OTL_TRACE_EXCEPTION(this->code,this->msg,this->stm_text,this->var_info)
#endif
 }
#endif

 virtual ~otl_tmpl_exception() 
#if defined(__GNUC__) && (__GNUC__>=3)
#if defined(OTL_ANSI_CPP_11_NOEXCEPT_SUPPORT)
   OTL_ANSI_CPP_11_NOEXCEPT
#else
    throw()
#endif
#else
    OTL_NO_THROW
#endif
 {
 }

};

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct>
class otl_tmpl_connect{
protected:

#if defined(OTL_ODBC_USES_SQL_FETCH_SCROLL_WHEN_SPECIFIED_IN_OTL_CONNECT)
  bool use_fetch_scroll_;
#endif

 TConnectStruct connect_struct;
 int long_max_size;
 int retcode;
 int throw_count;

public:

#if defined(OTL_ODBC_USES_SQL_FETCH_SCROLL_WHEN_SPECIFIED_IN_OTL_CONNECT)

  void set_fetch_scroll_mode(const bool use_fetch_scroll_mode=true)
  {
    use_fetch_scroll_=use_fetch_scroll_mode;
  }

  bool get_fetch_scroll_mode() const 
  {
    return use_fetch_scroll_;
  }

#endif

  void set_retcode(const int aretcode)
  {
    retcode=aretcode;
  }

  int get_retcode() const
  {
    return retcode;
  }

  void reset_throw_count(void)
  {
    throw_count=0;
  }

  void increment_throw_count()
  {
    throw_count++;
  }

  int get_throw_count() const
  {
    return throw_count;
  }

  TConnectStruct& get_connect_struct()
  {
    return connect_struct;
  }

 int connected;

 void set_max_long_size(const int amax_size)
 {
  reset_throw_count();
#if defined(OTL_UNICODE)
#if defined(OTL_ORA8I)||defined(OTL_ORA9I)||defined(OTL_ORA10G)||defined(OTL_ORA10G_R2)
  long_max_size=amax_size*sizeof(OTL_WCHAR);
#else
  long_max_size=amax_size;
#endif
#else
  long_max_size=amax_size;
#endif
 }

 int get_max_long_size(void)
 {
  reset_throw_count();
  return long_max_size;
 }

 void set_timeout(const int atimeout=0)
 {
  reset_throw_count();
  connect_struct.set_timeout(atimeout);
 }

 void set_cursor_type(const int acursor_type=0)
 {
  reset_throw_count();
  connect_struct.set_cursor_type(acursor_type);
 }

  otl_tmpl_connect():
#if defined(OTL_ODBC_USES_SQL_FETCH_SCROLL_WHEN_SPECIFIED_IN_OTL_CONNECT)
   use_fetch_scroll_(false),
#endif
    connect_struct(),
    long_max_size(otl_short_int_max),
    retcode(1),
    throw_count(0),
    connected(0)
 {
 }

  otl_tmpl_connect(const char* connect_str,const int auto_commit=0):
#if defined(OTL_ODBC_USES_SQL_FETCH_SCROLL_WHEN_SPECIFIED_IN_OTL_CONNECT)
   use_fetch_scroll_(false),
#endif
    connect_struct(),
    long_max_size(otl_short_int_max),
    retcode(1),
    throw_count(0),
    connected(0)
 {
  rlogon(connect_str,auto_commit);
 }

 virtual ~otl_tmpl_connect()
 {
  logoff();
 }

 static int otl_initialize(const int threaded_mode=0)
 {
  return TConnectStruct::initialize(threaded_mode);
 }

 void rlogon(const char* connect_str,const int auto_commit=0)
 {
  throw_count=0;
  retcode=connect_struct.rlogon(connect_str,auto_commit);
  if(retcode)
   connected=1;
  else{
   connected=0;
   increment_throw_count();
  if(get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   OTL_TMPL_EXCEPTION ex(connect_struct);
   connect_struct.cleanup();
   throw ex;
  }
 }

 void logoff(void)
 {
  if(!connected)return;
  OTL_TRACE_FUNC(0x1,"otl_connect","logoff","")
  retcode=connect_struct.logoff();
  connected=0;
  if(retcode)return;
  if(get_throw_count()>0)
   return;
  increment_throw_count();
  if(otl_uncaught_exception()) return; 
  throw OTL_TMPL_EXCEPTION(connect_struct);
 }

 void commit(void)
 {
  if(!connected)return;
  OTL_TRACE_FUNC(0x1,"otl_connect","commit","")
  reset_throw_count();
  retcode=connect_struct.commit();
  if(retcode)return;
  increment_throw_count();
  if(get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
  throw OTL_TMPL_EXCEPTION(connect_struct);
 }

 void auto_commit_on(void)
 {
  if(!connected)return;
  OTL_TRACE_FUNC(0x1,"otl_connect","auto_commit_on","")
  reset_throw_count();
  retcode=connect_struct.auto_commit_on();
  if(retcode)return;
  increment_throw_count();
  if(get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
  throw OTL_TMPL_EXCEPTION(connect_struct);
 }

 void auto_commit_off(void)
 {
  if(!connected)return;
  OTL_TRACE_FUNC(0x1,"otl_connect","auto_commit_off","")
  reset_throw_count();
  retcode=connect_struct.auto_commit_off();
  if(retcode)return;
  increment_throw_count();
  if(get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
  throw OTL_TMPL_EXCEPTION(connect_struct);
 }

 void rollback(void)
 {
  if(!connected)return;
  OTL_TRACE_FUNC(0x1,"otl_connect","rollback","")
  reset_throw_count();
  retcode=connect_struct.rollback();
  if(retcode)return;
  increment_throw_count();
  if(get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
  throw OTL_TMPL_EXCEPTION(connect_struct);
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_tmpl_connect(const otl_tmpl_connect&) = delete;
  otl_tmpl_connect& operator=(const otl_tmpl_connect&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_tmpl_connect(otl_tmpl_connect&&) = delete;
  otl_tmpl_connect& operator=(otl_tmpl_connect&&) = delete;
#endif
private:
#else
  otl_tmpl_connect(const otl_tmpl_connect&):
    connected(0),
#if defined(OTL_ODBC_USES_SQL_FETCH_SCROLL_WHEN_SPECIFIED_IN_OTL_CONNECT)
    use_fetch_scroll_(false),
#endif
    connect_struct(),
    long_max_size(otl_short_int_max),
    retcode(1),
    throw_count(0)
 {
 }

 otl_tmpl_connect& operator=(const otl_tmpl_connect&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_tmpl_connect(otl_tmpl_connect&&):
    connected(0),
    connect_struct(),
    long_max_size(otl_short_int_max),
    retcode(1),
    throw_count(0)
 {
 }

 otl_tmpl_connect& operator=(otl_tmpl_connect&&)
 {
   return *this;
 }
#endif
#endif

};

template <OTL_TYPE_NAME TVariableStruct>
class otl_tmpl_variable{
protected:

  int param_type;
  int ftype;
  int elem_size;
  int array_size;
  char* name;
  int pos;
  int name_pos;
  int bound;

  int pl_tab_flag;

  TVariableStruct var_struct;

public:

  TVariableStruct& get_var_struct(){return var_struct;}  
  const TVariableStruct& get_const_var_struct() const {return var_struct;}  
  int get_bound() const {return bound;}
  int get_param_type() const {return param_type;}
  int get_ftype() const {return ftype;}
  int get_name_pos() const {return name_pos;}
  int get_elem_size() const {return elem_size;}
  int get_pl_tab_flag() const {return pl_tab_flag;}
  int get_pos() const {return pos;}
  int get_array_size() const {return array_size;}
  const char* get_name() const {return name;}

  void set_pos(const int apos)
  {
    this->pos=apos;
  }

  void set_bound(const int abound)
  {
    this->bound=abound;
  }

  void set_name_pos(const int aname_pos)
  {
    this->name_pos=aname_pos;
  }

  void set_ftype(const int aftype)
  {
    this->ftype=aftype;
  }


  int actual_elem_size(void)
  {
    return var_struct.actual_elem_size();
  }

  void copy_var_desc(otl_var_desc& v)
  {
    v.param_type=param_type;
    v.ftype=ftype;
    v.elem_size=elem_size;
    v.array_size=array_size;
    v.pos=pos;
    v.name_pos=name_pos;
    if(name){
      OTL_STRNCPY_S(v.name,sizeof(v.name),name,sizeof(v.name)-1);
      v.name[sizeof(v.name)-1]=0;
    }else
      v.name[0]=0;
    v.pl_tab_flag=pl_tab_flag;
  }

  otl_tmpl_variable():
    param_type(0),
    ftype(0),
    elem_size(0),
    array_size(0),
    name(nullptr),
    pos(0),
    name_pos(0),
    bound(0),
    pl_tab_flag(0),
    var_struct()
  {
  }

  virtual ~otl_tmpl_variable()
  {
    delete[] name;
  }

  otl_tmpl_variable
  (const int column_num,
   const int aftype,
   const int aelem_size,
   const short aarray_size):
    param_type(0),
    ftype(0),
    elem_size(0),
    array_size(0),
    name(nullptr),
    pos(0),
    name_pos(0),
    bound(0),
    pl_tab_flag(0),
    var_struct()
  {
    copy_pos(column_num);
    init(aftype,aelem_size,aarray_size);
  }

  otl_tmpl_variable
  (const char* aname,
   const int aftype,
   const int aelem_size,
   const short aarray_size,
   const int apl_tab_flag=0)
  {
    copy_name(aname);
    init
      (aftype,
       aelem_size,
       aarray_size,
       0,
       apl_tab_flag);
  }

  void init
  (const bool select_stm_flag,
   const int aftype,
   const int aelem_size,
   const otl_stream_buffer_size_type aarray_size,
   const void* connect_struct=0,
   const int apl_tab_flag=0)
  {
    ftype=aftype;
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
    if(ftype==otl_var_nchar)
      ftype=otl_var_char;
    else if(ftype==otl_var_nclob)
      ftype=otl_var_clob;
#endif
    elem_size=aelem_size;
    array_size=aarray_size;
    pl_tab_flag=apl_tab_flag;
    bound=0;
    var_struct.init(select_stm_flag,
                    aftype,
                    elem_size,
                    aarray_size,
                    connect_struct,
                    pl_tab_flag);
  }

  void set_param_type(const int aparam_type=otl_input_param)
  {
    param_type=aparam_type;
  }

  int get_param_type(void)
  {
    return param_type;
  }

  void copy_name(const char* aname)
  {
    pos=0;
    if(name==aname)return;
    if(name)delete[] name;
    size_t len=strlen(aname)+1;
    name=new char[len];
    OTL_STRCPY_S(name,len,aname);
  }

  void copy_pos(const int apos)
  {
    if(name){
      delete[] name;
      name=nullptr;
      name_pos=0;
    }
    pos=apos;
  }

  
  void set_null(int ndx)
  {
    var_struct.set_null(ndx);
  }

  void set_not_null(int ndx)
  {
    var_struct.set_not_null(ndx,elem_size);
  }

  void set_len(int len, int ndx=0)
  {
    var_struct.set_len(len,ndx);
  }
  
  int get_len(int ndx=0)
  {
    return var_struct.get_len(ndx);
  }

  int get_pl_tab_len(void)
  {
    return this->var_struct.get_pl_tab_len();
  }

  int get_max_pl_tab_len(void)
  {
    return this->var_struct.get_max_pl_tab_len();
  }

  void set_pl_tab_len(const int pl_tab_len)
  {
    this->var_struct.set_pl_tab_len(pl_tab_len);
  }

  int is_null(int ndx)
  {
    return var_struct.is_null(ndx);
  }

  void* val(int ndx=0)
  {
    return var_struct.val(ndx,elem_size);
  }

  static void map_ftype
  (otl_column_desc& desc,
   const int max_long_size,
   int& aftype,
   int& aelem_size,
   otl_select_struct_override& a_override,
   const int column_ndx,
   const int connection_type)
  {
    TVariableStruct::map_ftype
      (desc,
       max_long_size,
       aftype,
       aelem_size,
       a_override,
       column_ndx,
       connection_type);
  }

  static int int2ext(int int_type)
  {
    return TVariableStruct::int2ext(int_type);
  }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_tmpl_variable(const otl_tmpl_variable&) = delete;
  otl_tmpl_variable& operator=(const otl_tmpl_variable&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_tmpl_variable(otl_tmpl_variable&&) = delete;
  otl_tmpl_variable& operator=(otl_tmpl_variable&&) = delete;
#endif
private:
#else
  otl_tmpl_variable(const otl_tmpl_variable&):
    param_type(0),
    ftype(0),
    elem_size(0),
    array_size(0),
    name(nullptr),
    pos(0),
    name_pos(0),
    bound(0),
    pl_tab_flag(0),
    var_struct()
  {
  }

  otl_tmpl_variable& operator=(const otl_tmpl_variable&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_tmpl_variable(otl_tmpl_variable&&):
    param_type(0),
    ftype(0),
    elem_size(0),
    array_size(0),
    name(nullptr),
    pos(0),
    name_pos(0),
    bound(0),
    pl_tab_flag(0),
    var_struct()
  {
  }

  otl_tmpl_variable& operator=(otl_tmpl_variable&&)
  {
    return *this;
  }
#endif
#endif

};

  template <OTL_TYPE_NAME TExceptionStruct,
            OTL_TYPE_NAME TConnectStruct,
            OTL_TYPE_NAME TCursorStruct,
            OTL_TYPE_NAME TVariableStruct>
  class otl_tmpl_cursor{
  protected:

    int connected;
    char* stm_text;
    char* stm_label;
  
    TCursorStruct cursor_struct;
    int vl_len;
    otl_tmpl_variable<TVariableStruct>** vl;
    OTL_TMPL_CONNECT* adb;
    int eof_data;
    int eof_desc;
    int retcode;
    long _rpc;
    int in_destructor;

  public:

    OTL_TMPL_CONNECT* get_adb(){return adb;}
    void set_adb(OTL_TMPL_CONNECT* aadb)
    {
      adb=aadb;
    }

    TCursorStruct& get_cursor_struct_ref(){return cursor_struct;}

    otl_tmpl_variable<TVariableStruct>** get_vl(){return vl;}
    int get_vl_len() const {return vl_len;}

    const char* get_stm_label() const
    {
      return stm_label;
    }  

    const char* get_stm_text() const
    {
      return stm_text;
    }  

    void set_connected(const int aconnected)
    {
      connected=aconnected;
    }

    int& get_eof_data_ref()
    {
      return eof_data;
    }

    TCursorStruct& get_cursor_struct()
    {
      return cursor_struct;
    }

    int get_connected() const
    {
      return connected;
    }

    otl_tmpl_cursor():
      connected(0),
      stm_text(nullptr),
      stm_label(nullptr),
      cursor_struct(),
      vl_len(0),
      vl(nullptr),
      adb(nullptr),
      eof_data(),
      eof_desc(),
      retcode(1),
      _rpc(0),
      in_destructor(0)
    {
    }

    otl_tmpl_cursor(OTL_TMPL_CONNECT& connect):
      connected(0),
      stm_text(nullptr),
      stm_label(nullptr),
      cursor_struct(),
      vl_len(0),
      vl(nullptr),
      adb(&connect),
      eof_data(),
      eof_desc(),
      retcode(1),
      _rpc(0),
      in_destructor(0)
    {
      open(connect);
    }

    otl_tmpl_cursor
    (OTL_TMPL_CONNECT& connect,
     TVariableStruct* var):
      connected(0),
      stm_text(nullptr),
      stm_label(nullptr),
      cursor_struct(),
      vl_len(0),
      vl(nullptr),
      adb(&connect),
      eof_data(),
      eof_desc(),
      retcode(1),
      _rpc(0),
      in_destructor(0)
    {
      open(connect,var);
    }

    virtual ~otl_tmpl_cursor()
    {
      in_destructor=1;
      close();
      delete[] stm_label;
      stm_label=nullptr;
      delete[] stm_text;
      stm_text=nullptr;
    }

    void open
    (OTL_TMPL_CONNECT& connect,
     TVariableStruct* var=nullptr)
    {
      in_destructor=0;
      eof_data=0;
      eof_desc=0;
      retcode=1;
      adb=&connect;
      _rpc=0;
      if(var==nullptr)
        retcode=cursor_struct.open(connect.get_connect_struct());
      else
        retcode=cursor_struct.open(connect.get_connect_struct(),var);
      if(retcode){
        connected=1;
        return;
      }
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return;
      if(otl_uncaught_exception()) return; 
      throw OTL_TMPL_EXCEPTION(cursor_struct);
    }

    virtual void close(void)
    {_rpc=0;
      if(!connected)return;
      if(!this->adb)return;
      if(!adb->connected){
        connected=0;
        adb=nullptr;
        retcode=1;
        return;
      }
      connected=0;
      retcode=cursor_struct.close();
      if(retcode){
        adb=nullptr;
        return;
      }
      if(this->adb->get_throw_count()>0){
        adb=nullptr;
        return;
      }
      this->adb->increment_throw_count();
      adb=nullptr;
      if(otl_uncaught_exception()) return; 
      throw OTL_TMPL_EXCEPTION(cursor_struct);
    }

    void parse(void)
    {_rpc=0;
      if(!connected)return;
      retcode=cursor_struct.parse(stm_text);
      switch(retcode){
      case 0:
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return;
        if(otl_uncaught_exception()) return; 
        throw OTL_TMPL_EXCEPTION(cursor_struct,stm_label?stm_label:stm_text);
      case 2:
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return;
        if(otl_uncaught_exception()) return; 
        char var_info[1];
        var_info[0]=0;
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_17,
           otl_error_code_17,
           this->stm_label?this->stm_label:this->stm_text,
           var_info);
      }
    }

    void parse(const char* sqlstm)
    {
      if(!connected)return;
      if(stm_text){
        delete[] stm_text;
        stm_text=nullptr;
      }
      size_t len=strlen(sqlstm)+1;
      stm_text=new char[len];
      OTL_STRCPY_S(stm_text,len,sqlstm);
      parse();
    }

    long get_rpc() OTL_NO_THROW
    {
      return _rpc;
    }

    void exec(const int iters/*=1*/,
              const int rowoff/*=0*/,
              const int otl_sql_exec_from_class/*=otl_sql_exec_from_cursor_class*/)
    {
      if(!connected)return; 
      retcode=cursor_struct.exec(iters,rowoff,otl_sql_exec_from_class);
      _rpc=cursor_struct.get_rpc();
      if(retcode)return;
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return;
      if(otl_uncaught_exception()) return; 
      throw OTL_TMPL_EXCEPTION(cursor_struct,stm_label?stm_label:stm_text);
    }

    virtual bool valid_binding
    (const otl_tmpl_variable<TVariableStruct>& v,
     const int binding_type)
    {
      bool rc=true;
      if(((v.get_ftype()==otl_var_varchar_long||v.get_ftype()==otl_var_raw_long) &&
          (v.get_const_var_struct().get_otl_adapter()==otl_ora7_adapter||
           v.get_const_var_struct().get_otl_adapter()==otl_ora8_adapter) &&
          v.get_array_size()>1) ||
         ((v.get_ftype()==otl_var_blob||v.get_ftype()==otl_var_clob)&&
          v.get_const_var_struct().get_otl_adapter()==otl_ora8_adapter&&
          v.get_array_size()>1 && 
          binding_type==otl_inout_binding)) 
        rc=false;
      return rc;
    }

    virtual void bind
    (const char* name,
     otl_tmpl_variable<TVariableStruct>& v)
    {
      if(!connected)return;
      if(v.get_bound())return;
      v.copy_name(name);
      if(!valid_binding(v,otl_inout_binding)){
        char var_info[256];
        otl_var_info_var2
          (v.get_name(),
           v.get_ftype(),
           var_info,
           sizeof(var_info));
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return;
        if(otl_uncaught_exception()) return; 
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_16,
           otl_error_code_16,
           stm_label?stm_label:stm_text,
           var_info);
      }
      retcode=cursor_struct.bind
        (name,
         v.get_var_struct(),
         v.get_elem_size(),
         v.get_ftype(),
         v.get_param_type(),
         v.get_name_pos(),
         this->adb->get_connect_struct().get_connection_type(),
         v.get_pl_tab_flag());
      if(retcode){
        v.set_bound(1);
        return;
      }
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return;
      if(otl_uncaught_exception()) return; 
      throw OTL_TMPL_EXCEPTION(cursor_struct,stm_label?stm_label:stm_text);
    }

    virtual void bind
    (const int column_num,
     otl_tmpl_variable<TVariableStruct>& v)
    {
      if(!connected)return;
      v.copy_pos(column_num);
      if(!valid_binding(v,otl_select_binding)){
        char var_info[256];
        otl_var_info_col2
          (v.get_pos(),
           v.get_ftype(),
           var_info,
           sizeof(var_info));
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return;
        if(otl_uncaught_exception()) return; 
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_16,
           otl_error_code_16,
           stm_label?stm_label:stm_text,
           var_info);
      }
      retcode=cursor_struct.bind
        (column_num,
         v.get_var_struct(),
         v.get_elem_size(),
         v.get_ftype(),
         v.get_param_type());
      if(retcode)return;
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return;
      if(otl_uncaught_exception()) return; 
      throw OTL_TMPL_EXCEPTION(cursor_struct,stm_label?stm_label:stm_text);
    }

    virtual void bind(otl_tmpl_variable<TVariableStruct>& v)
    {
      if(!connected)return;
      if(v.get_name()) bind(v.get_name(),v);
      if(v.get_pos()) bind(v.get_pos(),v);
    }

  
    static long direct_exec
    (OTL_TMPL_CONNECT& connect,
     const char* sqlstm,
     const int exception_enabled=1)
#if defined(OTL_ANSI_CPP) && defined(OTL_FUNC_THROW_SPEC_ON)
      throw(OTL_TMPL_EXCEPTION)
#endif
    {
      connect.reset_throw_count();
      OTL_TRACE_DIRECT_EXEC
        try{
          OTL_TMPL_CURSOR cur(connect);
          cur.cursor_struct.set_direct_exec(1);
          cur.parse(sqlstm);
          cur.exec(1,0,otl_sql_exec_from_cursor_class);
          return cur.cursor_struct.get_rpc();
        }catch(OTL_CONST_EXCEPTION OTL_TMPL_EXCEPTION&){
          if(exception_enabled){
            connect.increment_throw_count();
            throw;
          }
        }
      return -1;
    }

    static void syntax_check
    (OTL_TMPL_CONNECT& connect,
     const char* sqlstm)
#if defined(OTL_ANSI_CPP) && defined(OTL_FUNC_THROW_SPEC_ON)
      throw(OTL_TMPL_EXCEPTION)
#endif
    {
      connect.reset_throw_count();
      OTL_TRACE_SYNTAX_CHECK
        OTL_TMPL_CURSOR cur(connect);
      cur.cursor_struct.set_direct_exec(1);
      cur.cursor_struct.set_parse_only(1);
      cur.parse(sqlstm);
    }
  
    int eof(void){return eof_data;}

    int describe_column
    (otl_column_desc& col,
     const int column_num)
    {
      if(!connected)return 0;
      retcode=cursor_struct.describe_column
        (col,column_num,eof_desc);
      if(eof_desc)return 0;
      if(retcode)return 1;
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return 0;
      if(otl_uncaught_exception()) return 0; 
      throw OTL_TMPL_EXCEPTION(cursor_struct,stm_label?stm_label:stm_text);
    }

  private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
  public:
    otl_tmpl_cursor(const otl_tmpl_cursor&) = delete;
    otl_tmpl_cursor& operator=(const otl_tmpl_cursor&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
    otl_tmpl_cursor(otl_tmpl_cursor&&) = delete;
    otl_tmpl_cursor& operator=(otl_tmpl_cursor&&) = delete;
#endif
  private:
#else
    otl_tmpl_cursor(const otl_tmpl_cursor&):
      connected(0),
      stm_text(nullptr),
      stm_label(nullptr),
      cursor_struct(),
      vl_len(0),
      vl(nullptr),
      adb(nullptr),
      eof_data(),
      eof_desc(),
      retcode(1),
      _rpc(0),
      in_destructor(0)
    {
    }

    otl_tmpl_cursor& operator=(const otl_tmpl_cursor&)
    {
      return *this;
    }


#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
    otl_tmpl_cursor(otl_tmpl_cursor&&):
      connected(0),
      stm_text(nullptr),
      stm_label(nullptr),
      cursor_struct(),
      vl_len(0),
      vl(nullptr),
      adb(nullptr),
      eof_data(),
      eof_desc(),
      retcode(1),
      _rpc(0),
      in_destructor(0)
    {
    }

    otl_tmpl_cursor& operator=(otl_tmpl_cursor&&)
    {
      return *this;
    }
#endif
#endif

  };

  inline int is_num(char c)
  {
    return c>='0' && c<='9';
  }

  template <OTL_TYPE_NAME TVariableStruct,
            OTL_TYPE_NAME TTimestampStruct,
            OTL_TYPE_NAME TExceptionStruct,
            OTL_TYPE_NAME TConnectStruct,
            OTL_TYPE_NAME TCursorStruct>
  class otl_tmpl_ext_hv_decl{
  private:

    char** hv;
    short int* inout;
    int* pl_tab_size;
    int array_size;
    int prev_array_size;
    short int vst[4];
    int len;
    char* stm_text_;
    char* stm_label_;
    int container_size_;
    bool has_plsql_tabs_or_refcur_;
    bool has_space_in_bind_variable_;

  public:

    bool has_plsql_tabs_or_refcur() const {return has_plsql_tabs_or_refcur_;}
    bool has_space_in_bind_variable() const {return has_space_in_bind_variable_;}
    short int get_vst(const int ndx) const {return vst[ndx];}
    int get_len() const {return len;}
    short int get_inout(const int ndx) const {return inout[ndx];}
    int get_pl_tab_size(const int ndx) const {return pl_tab_size[ndx];}
    const char* stm_label() const {return stm_label_;}
    const char* stm_text() const {return stm_text_;}

    enum var_status{
      in=0,
      out=1,
      io=2,
      def=3
    };

    otl_tmpl_ext_hv_decl(char* stm,
                         int arr_size=1,
                         char* label=nullptr,
                         otl_select_struct_override** select_override=nullptr,
                         OTL_TMPL_CONNECT* adb=nullptr):
      hv(nullptr),
      inout(nullptr),
      pl_tab_size(nullptr),
      array_size(0),
      prev_array_size(0),
      vst(),
      len(0),
      stm_text_(nullptr),
      stm_label_(nullptr),
      container_size_(0),
      has_plsql_tabs_or_refcur_(false),
      has_space_in_bind_variable_(false)
    {
      container_size_=otl_var_list_size;
      hv=new char*[container_size_];
      inout=new short[container_size_];
      pl_tab_size=new int[container_size_];

      int j;
      array_size=arr_size;
      prev_array_size=arr_size;
      stm_text_=stm;
      stm_label_=label;
      int i=0;
      short in_str=0;
      bool in_comment=false;
      bool in_one_line_comment=false;
      char *c=stm;
      bool in_comment_column_override=false;
      hv[i]=nullptr;
      while(*c){
        switch(*c){
        case '\'':
          if(!in_comment&&!in_one_line_comment){
            if(!in_str)
              in_str=1;
            else{
              if(c[1]=='\'')
                ++c;
              else
                in_str=0;
            }
          }
          break;
        case '/':
          if(c[1]=='*' && !in_str && c[2]==':' && c[3]=='#'){
            in_comment_column_override=true;
            *c=' ';
            ++c;
            *c=' ';
            ++c;
          }else if(c[1]=='*'&&!in_str){
            in_comment=true;
            ++c;
          }
          break;
        case '-':
          if(c[1]=='-'&&!in_str){
            in_one_line_comment=true;
            ++c;
          }
          break;
        case '*':
          if(c[1]=='/' && in_comment){
            in_comment=false;
            ++c;
          }else if(c[1]=='/' && in_comment_column_override){
            *c=' ';
            ++c;
            *c=' ';
          }
          break;
        case '\n':
          if(in_one_line_comment)
            in_one_line_comment=false;
          break;
        }
        if(*c==':' && !in_str && !in_comment && !in_one_line_comment &&
           ((c>stm && *(c-1)!='\\') || c==stm)){
          char* bind_var_ptr=c;
          short in_out=def;
          int apl_tab_size=0;
          char var[64];
          char* v=var;
          *v++=*c++;
          while(is_id(*c))
            *v++=*c++;
          while(otl_isspace(*c)&&*c)
            ++c;
          if(*c=='<' || (*c=='/' && c[1]=='*')){
            if(*c=='<')
              *c=' ';
            else if(*c=='/'&&c[1]=='*'){
              *c=' ';
              ++c;
              *c=' ';
            }
            while(*c!='>' && *c!=',' && *c!='*' && *c){
              *v++=*c;
              *c++=' ';
            }
            if(*c==',' && otl_isspace(c[1]))
              has_space_in_bind_variable_=true;
            if(*c==','){
              *c++=' ';
              if(otl_to_upper(*c)=='I'){
                if(otl_to_upper(c[2])=='O')
                  in_out=io;
                else
                  in_out=in;
              }else if(otl_to_upper(*c)=='O')
                in_out=out;
              while(*c!='>' && *c && *c!='*' && (*c!='[' && *c!='(') )
                *c++=' ';
              if(*c=='*'){
                *c=' ';
                ++c;
                *c=' ';
              }
              if(*c=='[' || *c=='('){
                char tmp[32];
                char *t=tmp;
                *c++=' ';
                while((*c!=']' && *c!=')') && *c!='>' && *c!='*' && *c){
                  *t++=*c;
                  *c++=' ';
                }
                if(*c=='*'){
                  *c=' ';
                  ++c;
                  *c=' ';
                }
                *t=0;
                apl_tab_size=atoi(tmp);
                while(*c!='>' && *c!='*' && *c)
                  *c++=' ';
                if(*c=='*'){
                  *c=' ';
                  ++c;
                  *c=' ';
                }
              }
            }else if(*c=='*' && c[1]=='/'){
              *c=' ';
              ++c;
              *c=' ';
            }
            if(*c)*c=' ';
            *v=0;
            if(select_override!=nullptr && bind_var_ptr[1]=='#'){
              char* c4=bind_var_ptr+2;
              char col_num[64];
              char* col_num_ptr=col_num;
              while(is_num(*c4) && *c4){
                *col_num_ptr=*c4;
                ++col_num_ptr;
                ++c4;
              }
              *col_num_ptr=0;
              int col_ndx=atoi(col_num);
              if(col_ndx>0){
                if(*select_override==nullptr){
                  *select_override=new otl_select_struct_override();
                }
                int data_type=otl_var_none;
                int data_len=0;
                char name[128];
                parse_var
                  (adb,
                   var,
                   data_type,
                   data_len,
                   name);
                (*select_override)->add_override
                  (col_ndx,
                   data_type,
                   data_len);
              }
              c4=bind_var_ptr;
              while(*c4 && *c4!=' '){
                *c4=' ';
                ++c4;
              }
            }else
              add_var(i,var,in_out,apl_tab_size);
          }
        }
        if(*c)++c;
      }
      for(j=0;j<4;++j)vst[j]=0;
      i=0;
      while(hv[i]){
        switch(inout[i]){
        case in:
          ++vst[0];
          break;
        case out:
          ++vst[1];
          break;
        case io:
          ++vst[2];
          break;
        case def:
          ++vst[3];
          break;
        }
        ++i;
      }
      len=i;
    }
  
    virtual ~otl_tmpl_ext_hv_decl()
    {int i;
      for(i=0;hv[i]!=nullptr;++i)
        delete[] hv[i];
      delete[] hv;
      delete[] inout;
      delete[] pl_tab_size;
    }


    char* operator[](int ndx){return hv[ndx];}
    short v_status(int ndx){return inout[ndx];}
    int is_id(char c){return isalnum(OTL_SCAST(unsigned char,c))||c=='_'||c=='#';}

    int name_comp(char* n1,char* n2)
    {
      while(*n1!=' '&&*n1!=0&&*n2!=' '&&*n2!=0){
        if(otl_to_upper(*n1)!=otl_to_upper(*n2))return 0;
        ++n1;
        ++n2;
      }
      if((*n1==' '&&*n2!=' ')||(*n2==' '&&*n1!=' '))
        return 0;
      return 1;
    }

    void add_var(int &n,char* v,short in_out,int apl_tab_size=0)
    {int i;
      for(i=0;i<n;++i)
        if(name_comp(hv[i],v))
          return;
      char *c=v;
      bool is_space=false;
      while(*c){
        is_space=otl_isspace(*c);
        if(is_space) break;
        ++c;
      }
      if(is_space && otl_str_case_insensitive_equal((c+1),"REFCUR")){
        has_plsql_tabs_or_refcur_=true;
        if(apl_tab_size==0)
          apl_tab_size=1;
      }
      if(apl_tab_size>0)
        has_plsql_tabs_or_refcur_=true;
      size_t v_len=strlen(v)+1;
      hv[n]=new char[v_len];
      OTL_STRCPY_S(hv[n],v_len,v);
      inout[n]=in_out;
      pl_tab_size[n]=apl_tab_size;
      if(n==container_size_-1){
        int temp_container_size=container_size_;
        container_size_*=2;
        char** temp_hv=nullptr;
        short* temp_inout=nullptr;
        int* temp_pl_tab_size=nullptr;
        try{
          temp_hv=new char*[container_size_];
          temp_inout=new short[container_size_];
          temp_pl_tab_size=new int[container_size_];
        }catch(const std::bad_alloc&){
          delete[] temp_hv;
          delete[] temp_inout;
          delete[] temp_pl_tab_size;
          throw;
        }
        memcpy(temp_hv,hv,sizeof(char*)*temp_container_size);
        memcpy(temp_inout,inout,sizeof(short)*temp_container_size);
        memcpy(temp_pl_tab_size,pl_tab_size,sizeof(int)*temp_container_size);
        delete[] hv;
        delete[] inout;
        delete[] pl_tab_size;
        hv=temp_hv;
        inout=temp_inout;
        pl_tab_size=temp_pl_tab_size;
      }
      hv[++n]=nullptr;
      inout[n]=def;
      pl_tab_size[n]=0;
    }

    int parse_var
    (OTL_TMPL_CONNECT* pdb,
     char* s,
     int& data_type,
     int& data_len,
     char* name)
    {
      data_type=otl_var_none;
      data_len=0;

#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) &&    \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID) || \
    defined(OTL_BIND_VAR_STRICT_TYPE_CHECKING_ON)
      char type_arr[256];
#endif
      char type=' ';
      char t2=' ';
      char t3=' ';
      char t4=' ';
      int size=0;

      char *c=name,*c1=s;
      while(*c1!=' '&&*c1)
        *c++=*c1++;
      *c=0;
      while(*c1==' '&&*c1)
        ++c1;

#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) &&    \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID) || \
    defined(OTL_BIND_VAR_STRICT_TYPE_CHECKING_ON)
      char* ct=c1;
      char* tac=type_arr;
      size_t ta_len=0;
      while(*ct && (*ct!='[' && *ct!='(') && ta_len<sizeof(type_arr)){
        *tac=otl_to_upper(*ct);
        ++ct;
        ++tac;
        ++ta_len;
      }
      *tac=0;
#endif
      size_t clen=strlen(c1);
      if(clen>=3){
        type=otl_to_upper(c1[0]);
        t2=otl_to_upper(c1[1]);
        t3=otl_to_upper(c1[2]);
        t4=otl_to_upper(c1[3]);
      }
      if((type=='C'&&t2=='H')||(type=='R'&&t2=='A'&&t3=='W'&&(t4=='['||t4=='('))){
        char tmp[32];
        char *t=tmp;
        while((*c1!='[' && *c1!='(')&&*c1)
          ++c1;
        ++c1;
        while((*c1!=']' && *c1!=')')&&*c1)
          *t++=*c1++;
        *t=0;
        size=atoi(tmp);
#if defined(OTL_ADD_NULL_TERMINATOR_TO_STRING_SIZE)
        size+=1;
#endif
      }

#if defined(OTL_ORA_UNICODE)
      if(type=='N'&&t2=='C'&&t3=='H'){
        char tmp[32];
        char *t=tmp;
        while((*c1!='[' && *c1!='(')&&*c1)
          ++c1;
        ++c1;
        while((*c1!=']' && *c1!=')')&&*c1)
          *t++=*c1++;
        *t=0;
        size=atoi(tmp);
#if defined(OTL_ADD_NULL_TERMINATOR_TO_STRING_SIZE)
        size+=1;
#endif
      }
#endif
  
      OTL_CHECK_BIND_VARS

      int rc=1;
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
      if(strcmp(type_arr,OTL_NUMERIC_TYPE_1_ID)==0){
        data_type=otl_var_char;
        data_len=otl_numeric_type_1_str_size;
        rc=0;
        return rc;
      }
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
      if(strcmp(type_arr,OTL_NUMERIC_TYPE_2_ID)==0){
        data_type=otl_var_char;
        data_len=otl_numeric_type_2_str_size;
        rc=0;
        return rc;
      }
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
      if(strcmp(type_arr,OTL_NUMERIC_TYPE_3_ID)==0){
        data_type=otl_var_char;
        data_len=otl_numeric_type_3_str_size;
        rc=0;
        return rc;
      }
#endif

      switch(type){
      case 'B':
        if(t2=='L'){
          data_type=otl_var_blob;
          if(pdb)
            data_len=pdb->get_max_long_size();
          else
            data_len=0;
        }else if(t2=='F'){
          data_type=otl_var_bfloat;
          data_len=sizeof(float);
        }else if(t2=='D'){
          data_type=otl_var_bdouble;
          data_len=sizeof(double);
        }
#if defined(OTL_BIGINT)
        else if(t2=='I'){
          data_type=TConnectStruct::var_bigint;
          data_len=TConnectStruct::bigint_size;
        }
#endif
        break;
      case 'C':
        if(t2=='H'){
          data_type=otl_var_char;
          data_len=size;
        }else if(t2=='L'){
          data_type=otl_var_clob;
          if(pdb)
            data_len=pdb->get_max_long_size();
          else
            data_len=0;
        }else
          rc=0;
        break;
      case 'D':
        if(t2=='O'){
          data_type=otl_var_double;
          data_len=sizeof(double);
        }
        else if(t2=='B'&&t3=='2'){
          if(t4=='T'){
            data_type=otl_var_db2time;
            data_len=sizeof(TTimestampStruct);
          }
          else if(t4=='D'){
            data_type=otl_var_db2date;
            data_len=sizeof(TTimestampStruct);
          }else
            rc=0;
        }else
          rc=0;
        break;
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
      case 'N':
        if(t2=='C'){
          if(t3=='L'){
            data_type=otl_var_nclob;
            if(pdb)
              data_len=pdb->get_max_long_size();
            else
              data_len=0;
          }else if(t3=='H'){
            data_type=otl_var_nchar;
            data_len=size;
          }
        }
        break;
#endif
      case 'F':
        data_type=otl_var_float;
        data_len=sizeof(float);
        break;
      case 'I':
        data_type=otl_var_int;
        data_len=sizeof(int);
        break;
      case 'U':
        if(t2=='N'){
          data_type=otl_var_unsigned_int;
          data_len=sizeof(unsigned);
        }
#if defined(OTL_UBIGINT)
        else if(t2=='B'){
          data_type=TConnectStruct::var_ubigint;
          data_len=TConnectStruct::ubigint_size;
        }
#endif
        break;
      case 'R':
        if(t2=='E'&&t3=='F'){
          data_type=otl_var_refcur;
          data_len=1;
        }else if(t2=='A'&&t3=='W'&&t4!='_'){
          data_type=otl_var_raw;
          data_len=size;
        }else if(t2=='A'&&t3=='W'&&t4=='_'){
          data_type=otl_var_raw_long;
          if(pdb)
            data_len=pdb->get_max_long_size();
          else
            data_len=0;
        }
        break;
      case 'S':
        data_type=otl_var_short;
        data_len=sizeof(short);
        break;
      case 'L':
        if(t2=='O'&&t3=='N'){
          data_type=otl_var_long_int;
          data_len=sizeof(long);
        }else if(t2=='T'&&t3=='Z'){
          data_type=otl_var_ltz_timestamp;
          data_len=sizeof(TTimestampStruct);
        }else
          rc=0;
        break;
      case 'T':
        if(t2=='Z'){
          data_type=otl_var_tz_timestamp;
          data_len=sizeof(TTimestampStruct);
        }else if(t2=='I' && t3=='M'){
          data_type=otl_var_timestamp;
          data_len=sizeof(TTimestampStruct);
        }else
          rc=0;
        break;
      case 'V':
        data_type=otl_var_varchar_long;
        if(pdb)
          data_len=pdb->get_max_long_size();
        else
          data_len=0;
        break;
      default:
        return 0;
      }
      return rc;
    }

    otl_tmpl_variable<TVariableStruct>* alloc_var
    (char* s,
     const int vstat,
     const int status,
     OTL_TMPL_CONNECT& adb,
     const int apl_tab_size=0)
    {
      char name[128];
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) &&    \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID) || \
    defined(OTL_BIND_VAR_STRICT_TYPE_CHECKING_ON)
      char type_arr[256];
#endif
      char type=' ';
      char t2=' ';
      char t3=' ';
      char t4=' ';
      char t5=' ';
   
      int size=0;
   
      char *c=name,*c1=s;
      while(*c1!=' '&&*c1)
        *c++=*c1++;
      *c=0;
      while(*c1==' '&&*c1)
        ++c1;
   
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) &&    \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID) || \
    defined(OTL_BIND_VAR_STRICT_TYPE_CHECKING_ON)
      char* ct=c1;
      char* tac=type_arr;
      size_t ta_len=0;
      while(*ct && (*ct!='[' && *ct!='(') && ta_len<sizeof(type_arr)){
        *tac=otl_to_upper(*ct);
        ++ct;
        ++tac;
        ++ta_len;
      }
      *tac=0;
#endif
      size_t clen=strlen(c1);
      if(clen>=3){
        type=otl_to_upper(c1[0]);
        t2=otl_to_upper(c1[1]);
        t3=otl_to_upper(c1[2]);
        t4=otl_to_upper(c1[3]);
      }
      if(clen>4)
        t5=otl_to_upper(c1[4]);
      if((type=='C'&&t2=='H')||(type=='R'&&t2=='A'&&t3=='W'&&(t4=='['||t4=='('))){
        char tmp[32];
        char *t=tmp;
        while((*c1!='[' && *c1!='(')&&*c1)
          ++c1;
        if(*c1)++c1;
        while((*c1!=']' && *c1!=')')&&*c1)
          *t++=*c1++;
        *t=0;
        if(*tmp==0)
          // declaration <char> is invalid
          return nullptr;
        size=atoi(tmp);
#if defined(OTL_ADD_NULL_TERMINATOR_TO_STRING_SIZE)
        if(type=='C')size+=1;
#endif
        if(size<2)
          // minimum size of <char[XXX]> should be at 2
          return nullptr;
      }
   
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
      if(type=='N'&&t2=='C'&&t3=='H'){
        char tmp[32];
        char *t=tmp;
        while((*c1!='[' && *c1!='(')&&*c1)
          ++c1;
        if(*c1)++c1;
        while((*c1!=']' && *c1!=')')&&*c1)
          *t++=*c1++;
        *t=0;
        if(*tmp==0)
          return nullptr;
        size=atoi(tmp);
#if defined(OTL_ADD_NULL_TERMINATOR_TO_STRING_SIZE)
        size+=1;
#endif
      }
#endif
   
   
      if(status==in && (vstat==in||vstat==io))
        ;
      else if(status==out && (vstat==out||vstat==io||vstat==def))
        ;
      else if(status==def)
        ;
      else
        return nullptr;
   
      OTL_CHECK_BIND_VARS
     
      int pl_tab_flag=0;
   
      if(apl_tab_size){
        array_size=apl_tab_size;
        pl_tab_flag=1;
      }else
        array_size=prev_array_size;
   
      otl_tmpl_variable<TVariableStruct>* v=
        new otl_tmpl_variable<TVariableStruct>;
      v->copy_name(name);
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
      if(strcmp(type_arr,OTL_NUMERIC_TYPE_1_ID)==0){
       v->init(false,otl_var_char,
               otl_numeric_type_1_str_size,
               OTL_SCAST(otl_stream_buffer_size_type,array_size),
               &adb.get_connect_struct(),pl_tab_flag);
      }else
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
      if(strcmp(type_arr,OTL_NUMERIC_TYPE_2_ID)==0){
       v->init(false,otl_var_char,
               otl_numeric_type_2_str_size,
               OTL_SCAST(otl_stream_buffer_size_type,array_size),
               &adb.get_connect_struct(),pl_tab_flag);
      }else
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
      if(strcmp(type_arr,OTL_NUMERIC_TYPE_3_ID)==0){
       v->init(false,otl_var_char,
               otl_numeric_type_3_str_size,
               OTL_SCAST(otl_stream_buffer_size_type,array_size),
               &adb.get_connect_struct(),pl_tab_flag);
      }else
#endif
      {
        switch(type){
        case 'B':
          if(t2=='L')
            v->init(false,
                    otl_var_blob,
                    adb.get_max_long_size(),
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct());
#if defined(OTL_BIGINT)
          else if(t2=='I')
            v->init(false,
                    TConnectStruct::var_bigint,
                    TConnectStruct::bigint_size,
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct(),
                    pl_tab_flag);
#endif
          else if(t2=='F')
            v->init(false,otl_var_bfloat,
                    sizeof(float),
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct(),pl_tab_flag);
          else if(t2=='D')
            v->init(false,otl_var_bdouble,
                    sizeof(double),
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct(),pl_tab_flag);
          break;
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
        case 'N':
          if(t2=='C' && (t3=='L'||t3=='H')){
            if(t3=='L'){
              v->init(false,otl_var_nclob,
                      adb.get_max_long_size(),
                      OTL_SCAST(otl_stream_buffer_size_type,array_size),
                      &adb.get_connect_struct());
              v->set_ftype(otl_var_clob);
            }else if(t3=='H'){
              v->init(false,otl_var_nchar,
                      size,
                      OTL_SCAST(otl_stream_buffer_size_type,array_size),
                      &adb.get_connect_struct(),pl_tab_flag);
              v->set_ftype(otl_var_char);
            }
          }else{
            delete v;
            v=nullptr;
          }
          break;
#endif
        case 'C':
          if(t2=='H'){
            v->init(false,otl_var_char,
                    size,
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct(),pl_tab_flag);
            if(t5=='Z')
              v->get_var_struct().set_charz_flag(true);
          }else if(t2=='L')
            v->init(false,otl_var_clob,
                    adb.get_max_long_size(),
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct());
          else{
            delete v;
            v=nullptr;
          }
          break;
        case 'D':
          if(t2=='O')
            v->init(false,otl_var_double,sizeof(double),
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct(),pl_tab_flag);
          else if(t2=='B'&&t3=='2'){
            if(t4=='T')
              v->init(false,otl_var_db2time,sizeof(TTimestampStruct),
                      OTL_SCAST(otl_stream_buffer_size_type,array_size),
                      &adb.get_connect_struct(),pl_tab_flag);
            else if(t4=='D')
              v->init(false,otl_var_db2date,sizeof(TTimestampStruct),
                      OTL_SCAST(otl_stream_buffer_size_type,array_size),
                      &adb.get_connect_struct(),pl_tab_flag);
            else{
              delete v;
              v=nullptr;
            }
          }else{
            delete v;
            v=nullptr;
          }
          break;
        case 'F':
          v->init(false,otl_var_float,
                  sizeof(float),
                  OTL_SCAST(otl_stream_buffer_size_type,array_size),
                  &adb.get_connect_struct(),pl_tab_flag);
          break;
        case 'I':
          v->init(false,otl_var_int,
                  sizeof(int),
                  OTL_SCAST(otl_stream_buffer_size_type,array_size),
                  &adb.get_connect_struct(),pl_tab_flag);
          break;
        case 'U':
          if(t2=='N')
            v->init(false,otl_var_unsigned_int,
                    sizeof(unsigned),
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct(),pl_tab_flag);
#if defined(OTL_UBIGINT)
          else if(t2=='B')
            v->init(false,
                    TConnectStruct::var_ubigint,
                    TConnectStruct::ubigint_size,
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct(),
                    pl_tab_flag);
#endif
          break;
        case 'R':
          if(t2=='E'&&t3=='F')
            v->init(false,otl_var_refcur,
                    1,
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct(),0);
          else if(t2=='A'&&t3=='W'&&(t4=='['||t4=='('))
            v->init(false,otl_var_raw,
                    size,
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct(),pl_tab_flag);
          else if(t2=='A'&&t3=='W')
            v->init(false,otl_var_raw_long,
                    adb.get_max_long_size(),
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct());
          break;
        case 'S':
          v->init(false,otl_var_short,
                  sizeof(short),
                  OTL_SCAST(otl_stream_buffer_size_type,array_size),
                  &adb.get_connect_struct(),pl_tab_flag);
          break;
        case 'L':
          if(t2=='O'&&t3=='N')
            v->init(false,otl_var_long_int,
                    sizeof(long),
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct(),pl_tab_flag);
          else if(t2=='T'&&t3=='Z')
            v->init(false,otl_var_ltz_timestamp,
                    sizeof(TTimestampStruct),
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct(),pl_tab_flag);
          else{
            delete v;
            v=nullptr;
          }
          break;
        case 'T':
          if(t2=='Z')
            v->init(false,otl_var_tz_timestamp,sizeof(TTimestampStruct),
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
                    &adb.get_connect_struct(),pl_tab_flag);
          else if(t2=='I' && t3=='M')
            v->init(false,otl_var_timestamp,sizeof(TTimestampStruct),
                    OTL_SCAST(otl_stream_buffer_size_type,array_size),
               &adb.get_connect_struct(),pl_tab_flag);
          else{
            delete v;
            v=nullptr;
          }
          break;
        case 'V':
          v->init(false,otl_var_varchar_long,adb.get_max_long_size(),
                  OTL_SCAST(otl_stream_buffer_size_type,array_size),
                  &adb.get_connect_struct());
          break;
        default:
          delete v;
          v=nullptr;
          break;
        }
      }
      return v;
    }

 void alloc_host_var_list
 (otl_tmpl_variable<TVariableStruct>** &vl,
  int& vl_len,
  OTL_TMPL_CONNECT& adb,
  const int status=def)
 {
  int j;
  vl_len=0;
  if(!hv[0]){
   vl=nullptr;
   return;
  }
  otl_auto_array_ptr<otl_tmpl_variable<TVariableStruct>*> 
    loc_ptr(container_size_);
  otl_tmpl_variable<TVariableStruct>** tmp_vl=loc_ptr.get_ptr();
  int i=0;
  while(hv[i]){
    otl_tmpl_variable<TVariableStruct>* vp=
      alloc_var(hv[i],inout[i],status,adb,pl_tab_size[i]);
    if(vp==nullptr){
      int j2;
      for(j2=0;j2<vl_len;++j2)
        delete tmp_vl[j2];
      vl_len=0;
      throw OTL_TMPL_EXCEPTION
        (otl_error_msg_12,
         otl_error_code_12,
         stm_label_?stm_label_:stm_text_,
         hv[i]);
    }
    vp->set_name_pos(i+1);
    if(vp){
      ++vl_len;
      tmp_vl[vl_len-1]=vp;
    }
    ++i;
  }
  if(vl_len>0){
   vl=new otl_tmpl_variable<TVariableStruct>*[vl_len];
   for(j=0;j<vl_len;++j)
    vl[j]=tmp_vl[j];
  }
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
  public:
 otl_tmpl_ext_hv_decl
 (const otl_tmpl_ext_hv_decl
  <TVariableStruct,
   TTimestampStruct,
   TExceptionStruct,
   TConnectStruct,
   TCursorStruct>&) = delete;
otl_tmpl_ext_hv_decl
  <TVariableStruct,
   TTimestampStruct,
   TExceptionStruct,
   TConnectStruct,
   TCursorStruct>&
operator=
(const otl_tmpl_ext_hv_decl
  <TVariableStruct,
   TTimestampStruct,
   TExceptionStruct,
   TConnectStruct,
   TCursorStruct>&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_tmpl_ext_hv_decl
 (otl_tmpl_ext_hv_decl
  <TVariableStruct,
   TTimestampStruct,
   TExceptionStruct,
   TConnectStruct,
   TCursorStruct>&&) = delete;
otl_tmpl_ext_hv_decl
  <TVariableStruct,
   TTimestampStruct,
   TExceptionStruct,
   TConnectStruct,
   TCursorStruct>&
operator=
(otl_tmpl_ext_hv_decl
  <TVariableStruct,
   TTimestampStruct,
   TExceptionStruct,
   TConnectStruct,
   TCursorStruct>&&) = delete;
#endif
  private:
#else
 otl_tmpl_ext_hv_decl
 (const otl_tmpl_ext_hv_decl
  <TVariableStruct,
   TTimestampStruct,
   TExceptionStruct,
   TConnectStruct,
   TCursorStruct>&):
   hv(nullptr),
   inout(nullptr),
   pl_tab_size(0),
   array_size(0),
   prev_array_size(0),
   vst(),
   len(0),
   stm_text_(nullptr),
   stm_label_(nullptr),
   container_size_(0),
   has_plsql_tabs_or_refcur_(0)
 {
 }

otl_tmpl_ext_hv_decl
  <TVariableStruct,
   TTimestampStruct,
   TExceptionStruct,
   TConnectStruct,
   TCursorStruct>&
operator=
(const otl_tmpl_ext_hv_decl
  <TVariableStruct,
   TTimestampStruct,
   TExceptionStruct,
   TConnectStruct,
   TCursorStruct>&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_tmpl_ext_hv_decl
 (otl_tmpl_ext_hv_decl
  <TVariableStruct,
   TTimestampStruct,
   TExceptionStruct,
   TConnectStruct,
   TCursorStruct>&&):
   hv(nullptr),
   inout(nullptr),
   pl_tab_size(0),
   array_size(0),
   prev_array_size(0),
   vst(),
   len(0),
   stm_text_(nullptr),
   stm_label_(nullptr),
   container_size_(0),
   has_plsql_tabs_or_refcur_(0)
 {
 }

otl_tmpl_ext_hv_decl
  <TVariableStruct,
   TTimestampStruct,
   TExceptionStruct,
   TConnectStruct,
   TCursorStruct>&
operator=
(otl_tmpl_ext_hv_decl
  <TVariableStruct,
   TTimestampStruct,
   TExceptionStruct,
   TConnectStruct,
   TCursorStruct>&&)
 {
   return *this;
 }
#endif
#endif

};

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct,
          OTL_TYPE_NAME TSelectCursorStruct>
class otl_tmpl_select_cursor: public OTL_TMPL_CURSOR{

protected:

  int cur_row;
  int cur_size;
  int row_count;
  int array_size;
  int prefetch_array_size;
  
  TSelectCursorStruct select_cursor_struct;
  otl_select_struct_override local_override;
  void* master_stream_ptr_;

public:

  TSelectCursorStruct& get_select_cursor_struct()
  {
    return select_cursor_struct;
  }  
 
 otl_tmpl_select_cursor
 (OTL_TMPL_CONNECT& pdb,
  void* master_stream_ptr,
  const otl_stream_buffer_size_type arr_size=1,
  const char* sqlstm_label=nullptr): 
   OTL_TMPL_CURSOR(pdb),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(0),
   prefetch_array_size(0),
   select_cursor_struct(),
   local_override(),
   master_stream_ptr_(master_stream_ptr)
 {
   local_override.reset();
   if(sqlstm_label!=nullptr){
     if(this->stm_label!=nullptr){
       delete[] this->stm_label;
       this->stm_label=nullptr;
     }
     size_t len=strlen(sqlstm_label)+1;
     this->stm_label=new char[len];
     OTL_STRCPY_S(this->stm_label,len,sqlstm_label);
   }
   select_cursor_struct.set_arr_size
     (arr_size,
      array_size,
      prefetch_array_size);
   select_cursor_struct.init(array_size);
 }

 otl_tmpl_select_cursor()
   : OTL_TMPL_CURSOR(),   
     master_stream_ptr_(nullptr)
 {
 }

 void open
 (OTL_TMPL_CONNECT& db,
  otl_stream_buffer_size_type  arr_size=1)
 {
   local_override.reset();
   cur_row=-1;
   row_count=0;
   cur_size=0;
   array_size=arr_size;
   OTL_TMPL_CURSOR::open(db);
 }

 void close(void)
 {
   local_override.reset();
   OTL_TMPL_CURSOR::close();
 }

 int first(void)
 {
  if(!OTL_TMPL_CURSOR::connected)return 0;
  select_cursor_struct.set_prefetch_size(prefetch_array_size);
  int rc=select_cursor_struct.first
   (this->cursor_struct,
    cur_row,cur_size,
    row_count,
    this->eof_data,
    array_size);
  OTL_TRACE_FIRST_FETCH
  if(!rc){
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return 0;
   if(otl_uncaught_exception()) return 0; 
   throw OTL_TMPL_EXCEPTION
     (this->cursor_struct,
      this->stm_label?
      this->stm_label:
      this->stm_text);
  }
  return cur_size!=0;
 }

  int next_throw(void)
  {
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return 0;
    if(otl_uncaught_exception()) return 0; 

    throw OTL_TMPL_EXCEPTION
      (this->cursor_struct,
       this->stm_label?
       this->stm_label:
       this->stm_text);
  }

 int next(void)
 {
  if(!this->connected)return 0;
  if(cur_row==-1)return first();
  int rc=select_cursor_struct.next
   (this->cursor_struct,
    cur_row,cur_size,
    row_count,
    this->eof_data,
    array_size);
  if(!rc){
    return next_throw();
  }
  OTL_TRACE_NEXT_FETCH
  return cur_size!=0;
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_tmpl_select_cursor(const otl_tmpl_select_cursor&) = delete;
 otl_tmpl_select_cursor& operator=(const otl_tmpl_select_cursor&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_tmpl_select_cursor(otl_tmpl_select_cursor&&) = delete;
 otl_tmpl_select_cursor& operator=(otl_tmpl_select_cursor&&) = delete;
#endif
private:
#else
 otl_tmpl_select_cursor
 (const otl_tmpl_select_cursor&): 
   OTL_TMPL_CURSOR(),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(0),
   prefetch_array_size(0),
   select_cursor_struct(),
   local_override()
 {
 }

 otl_tmpl_select_cursor& operator=
 (const otl_tmpl_select_cursor&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_tmpl_select_cursor
 (otl_tmpl_select_cursor&&): 
   OTL_TMPL_CURSOR(),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(0),
   prefetch_array_size(0),
   select_cursor_struct(),
   local_override()
 {
 }

 otl_tmpl_select_cursor& operator=
 (otl_tmpl_select_cursor&&)
 {
   return *this;
 }
#endif
#endif

};

#if defined(OTL_ORA8)||defined(OTL_ODBC)

const int otl_lob_stream_read_mode=1;
const int otl_lob_stream_write_mode=2;
const int otl_lob_stream_zero_mode=3;

const int otl_lob_stream_first_piece=1;
const int otl_lob_stream_next_piece=2;
const int otl_lob_stream_last_piece=3;

class otl_lob_stream_generic{
protected:

 int mode;
 int retcode;
 int ndx;
 int offset;
 int lob_len;
 int in_destructor;
 int eof_flag;
 int lob_is_null;
 bool ora_lob;

public:

  int get_ora_lob() const {return ora_lob;}


  otl_lob_stream_generic(const bool aora_lob=true):
    mode(0),
    retcode(0),
    ndx(0),
    offset(0),
    lob_len(0),
    in_destructor(0),
    eof_flag(0),
    lob_is_null(0),
    ora_lob(aora_lob)
 {
 }

 virtual ~otl_lob_stream_generic(){}

 virtual void init
 (void* avar,void* aconnect,void* acursor,int andx,
  int amode,const int alob_is_null=0) = 0;
 virtual void set_len(const int new_len=0) = 0;
 virtual otl_lob_stream_generic& operator<<(const otl_long_string& s) = 0;
 virtual otl_lob_stream_generic& operator>>(otl_long_string& s) = 0;
#if (defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)) && !defined(OTL_UNICODE)
  virtual otl_lob_stream_generic& operator<<(const OTL_STRING_CONTAINER& s) = 0;
  virtual otl_lob_stream_generic& operator>>(OTL_STRING_CONTAINER& s) = 0;
  virtual  void setStringBuffer(const int chunk_size) = 0;
#endif

 virtual int eof(void) = 0;
 virtual int len(void) = 0;
 virtual bool is_initialized(void) = 0;
 virtual void close(bool dont_throw_size_doesnt_match_exception=false) = 0;

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_lob_stream_generic(const otl_lob_stream_generic&) = delete;
  otl_lob_stream_generic& operator=(const otl_lob_stream_generic&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_lob_stream_generic(otl_lob_stream_generic&&) = delete;
  otl_lob_stream_generic& operator=(otl_lob_stream_generic&&) = delete;
#endif
private:
#else
  otl_lob_stream_generic(const otl_lob_stream_generic&):
    mode(0),
    retcode(0),
    ndx(0),
    offset(0),
    lob_len(0),
    in_destructor(0),
    eof_flag(0),
    lob_is_null(0),
    ora_lob(false)
  {
  }

  otl_lob_stream_generic& operator=(const otl_lob_stream_generic&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_lob_stream_generic(otl_lob_stream_generic&&):
    mode(0),
    retcode(0),
    ndx(0),
    offset(0),
    lob_len(0),
    in_destructor(0),
    eof_flag(0),
    lob_is_null(0),
    ora_lob(false)
  {
  }

  otl_lob_stream_generic& operator=(otl_lob_stream_generic&&)
  {
    return *this;
  }
#endif
#endif

};

#endif

#if defined(_MSC_VER) && (_MSC_VER<=1300)
#if !defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
#define OTL_NO_TMPL_MEMBER_FUNC_SUPPORT
#endif
#endif

#if defined(__SUNPRO_CC) || defined(__HP_aCC) || defined(__BORLANDC__)
#if !defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
#define OTL_NO_TMPL_MEMBER_FUNC_SUPPORT
#endif
#endif

#if defined(__IBMC__) || defined(__IBMCPP__) || defined(__xlC__)
#if !defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
#define OTL_NO_TMPL_MEMBER_FUNC_SUPPORT
#endif
#endif

#if defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__*100+__GNUC_MINOR__==400)
#if !defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
#define OTL_NO_TMPL_MEMBER_FUNC_SUPPORT
#endif
#endif

#if defined(__GNUC__) && (__GNUC__<=3)
#if !defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
#define OTL_NO_TMPL_MEMBER_FUNC_SUPPORT
#endif
#endif

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct,
          OTL_TYPE_NAME TSelectCursorStruct,
          OTL_TYPE_NAME TTimestampStruct>
class otl_tmpl_select_stream: public OTL_TMPL_SELECT_CURSOR{

protected:

 otl_column_desc* sl_desc;
 otl_tmpl_variable<TVariableStruct>* sl;
 int sl_len;
 int null_fetched;
 int cur_col;
 int cur_in;
 int executed;
 int eof_status;
 char var_info[256];
 otl_select_struct_override* override_;
 int delay_next;
 bool lob_stream_mode;
 long _rfc;

public:

  int get_select_row_count() const 
  {
    return this->cur_row==-1?0:this->cur_size-this->cur_row; 
  }

  int get_prefetched_row_count() const {return this->row_count;}
  int get_row_count() const {return this->row_count;}
  int get_sl_len() const {return sl_len;}
  otl_tmpl_variable<TVariableStruct>* get_sl(){return sl;}
  otl_column_desc* get_sl_desc(){return sl_desc;}
  long get_rfc() const {return _rfc;}

 void cleanup(void)
 {int i;
  delete[] sl;
  for(i=0;i<this->vl_len;++i)
   delete this->vl[i];
  delete[] this->vl;
  delete[] sl_desc;
 }

 virtual ~otl_tmpl_select_stream()
 {
  cleanup();
 }

 otl_tmpl_select_stream
 (otl_select_struct_override* aoverride,
  const otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  OTL_TMPL_CONNECT& pdb,
  const int implicit_select=otl_explicit_select,
  const char* sqlstm_label=nullptr)
   : OTL_TMPL_SELECT_CURSOR
     (pdb,
      aoverride->get_master_stream_ptr(),
      arr_size,sqlstm_label),
     sl_desc(nullptr),
     sl(nullptr),
     sl_len(0),
     null_fetched(0),
     cur_col(0),
     cur_in(0),
     executed(0),
     eof_status(0),
     var_info(),
     override_(nullptr),
     delay_next(0),
     lob_stream_mode(false),
     _rfc(0)
 {
   int i;
   this->select_cursor_struct.set_select_type(implicit_select);
   sl=nullptr;
   sl_len=0;
   _rfc=0;
   null_fetched=0;
   lob_stream_mode=aoverride->get_lob_stream_mode();
   this->retcode=0;
   sl_desc=nullptr;
   executed=0;
   cur_in=0;
   this->stm_text=nullptr;
   eof_status=1;
   override_=aoverride;
   
   bool out_or_inout_variable_flag=false;
   {
     size_t len=strlen(sqlstm)+1;
     this->stm_text=new char[len];
     OTL_STRCPY_S(this->stm_text,len,sqlstm);
     otl_select_struct_override* temp_local_override=&this->local_override;
     otl_tmpl_ext_hv_decl
       <TVariableStruct,TTimestampStruct,TExceptionStruct,
       TConnectStruct,TCursorStruct> hvd
       (this->stm_text,
        1,
        this->stm_label,
        &temp_local_override,
        &pdb
         );
     if(hvd.get_vst(1)>0||hvd.get_vst(2)>0)
       out_or_inout_variable_flag=true;
     hvd.alloc_host_var_list(this->vl,this->vl_len,pdb);
     if(temp_local_override!=&this->local_override)
       delete temp_local_override;
   }
   if(out_or_inout_variable_flag){
     throw OTL_TMPL_EXCEPTION
         (otl_error_msg_39,
          otl_error_code_39,
          this->stm_label?this->stm_label:
          this->stm_text);
   }
   try{
     this->parse();
     if(!this->select_cursor_struct.get_implicit_cursor()){
       get_select_list();
       bind_all();
     }else{
       for(i=0;i<this->vl_len;++i)
         this->bind(*this->vl[i]);
     }
     if(this->vl_len==0){
       rewind();
       null_fetched=0;
     }
   }catch(OTL_CONST_EXCEPTION OTL_TMPL_EXCEPTION&){
     cleanup();
     if(this->adb)this->adb->increment_throw_count();
     throw;
   }

 }

 void rewind(void)
 {
  OTL_TRACE_STREAM_EXECUTION
  int i;
  _rfc=0;
  if(!this->select_cursor_struct.close_select(this->cursor_struct)){
   throw OTL_TMPL_EXCEPTION
    (this->cursor_struct,
     this->stm_label?this->stm_label:this->stm_text);
  }
  if(this->select_cursor_struct.get_implicit_cursor()){
    this->exec(1,0,otl_sql_exec_from_select_cursor_class);
   if(sl){
    delete[] sl;
    sl=nullptr;
   }
   get_select_list();
   for(i=0;i<sl_len;++i)this->bind(sl[i]);
  }
  eof_status=this->first();
  null_fetched=0;
  cur_col=-1;
  cur_in=0;
  executed=1;
  delay_next=0;
 }
  
  void clean(void)
  {
    _rfc=0;
    this->cursor_struct.set_canceled(false);
    null_fetched=0;
    cur_col=-1;
    cur_in=0;
    executed=0;
    eof_status=0;
    delay_next=0;
    this->cur_row=-1;
    this->row_count=0;
    this->cur_size=0;
    if(!this->select_cursor_struct.close_select(this->cursor_struct)){
      throw OTL_TMPL_EXCEPTION
        (this->cursor_struct,
         this->stm_label?this->stm_label:this->stm_text);
    }
  }

 int is_null(void)
 {
  return null_fetched;
 }

 int eof(void)
 {
#if defined(OTL_SELECT_STREAM_ALTERNATE_FETCH)
   if(cur_col==sl_len-1){
     get_next();
     cur_col=-1;
   }else{
     if(delay_next){
       look_ahead();
       delay_next=0;
     }
   }
  return !eof_status;
#else
  if(delay_next){
   look_ahead();
   delay_next=0;
  }
  return !eof_status;
#endif
 }

 int eof_intern(void)
 {
  return !eof_status;
 }

 void skip_to_end_of_row()
 {
   check_if_executed();
   if(eof_intern())return;
   while(cur_col<sl_len-1){
     ++cur_col;
     null_fetched=sl[cur_col].is_null(this->cur_row);
   }
   eof_status=this->next();
   cur_col=0;
   if(!eof_intern())
     cur_col=-1;
  ++_rfc;
 }

 void bind_all(void)
 {int i;
  for(i=0;i<this->vl_len;++i)this->bind(*this->vl[i]);
  for(i=0;i<sl_len;++i)this->bind(sl[i]);
 }

 void get_select_list(void)
 {
   int j;
   otl_auto_array_ptr<otl_column_desc> loc_ptr(otl_var_list_size);
   otl_column_desc* sl_desc_tmp=loc_ptr.get_ptr();
   int sld_tmp_len=0;
   int ftype,elem_size,i;
   for(i=1;this->describe_column(sl_desc_tmp[i-1],i);++i){
     int temp_code_type=
       otl_tmpl_variable<TVariableStruct>::int2ext
       (sl_desc_tmp[i-1].dbtype);
     if(temp_code_type==otl_unsupported_type){
       otl_var_info_col3
         (i-1,
          sl_desc_tmp[i-1].dbtype,
          sl_desc_tmp[i-1].name,
          this->var_info,
          sizeof(this->var_info));
       throw OTL_TMPL_EXCEPTION
         (otl_error_msg_27,
          otl_error_code_27,
          this->stm_label?
          this->stm_label:
          this->stm_text,
          this->var_info);
     }
     ++sld_tmp_len;
     if(sld_tmp_len==loc_ptr.get_arr_size()){
       loc_ptr.double_size();
       sl_desc_tmp=loc_ptr.get_ptr();
     }
   }
   sl_len=sld_tmp_len;
   if(sl){
     delete[] sl;
     sl=nullptr;
   }
   sl=new otl_tmpl_variable<TVariableStruct>[sl_len==0?1:sl_len];
   int max_long_size=this->adb->get_max_long_size();
   for(j=0;j<sl_len;++j){
     otl_tmpl_variable<TVariableStruct>::map_ftype
       (sl_desc_tmp[j],
        max_long_size,
        ftype,
        elem_size,
        this->local_override.getLen()>0?this->local_override:*override_,
        j+1,
        this->adb->get_connect_struct().get_connection_type());
     sl[j].copy_pos(j+1);
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
     if(sl_desc_tmp[j].charset_form==2)
       sl[j].get_var_struct().set_nls_flag(true);
#endif
     sl[j].init(true,
                ftype,
                elem_size,
                OTL_SCAST(otl_stream_buffer_size_type,(this->array_size)),
                &this->adb->get_connect_struct()
                );
     sl[j].get_var_struct().set_lob_stream_mode(this->lob_stream_mode);
   }
   if(sl_desc){
     delete[] sl_desc;
     sl_desc=nullptr;
   }
   sl_desc=new otl_column_desc[sl_len==0?1:sl_len];
   for(j=0;j<sl_len;++j)
     sl_desc[j]=sl_desc_tmp[j];
 }
  
  void check_if_executed_throw(void)
  {
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return;
    if(otl_uncaught_exception()) return; 
    throw OTL_TMPL_EXCEPTION
      (otl_error_msg_2,
       otl_error_code_2,
       this->stm_label?
       this->stm_label:
       this->stm_text,
       nullptr);
  }

 void check_if_executed(void)
 {
  if(!executed){
    check_if_executed_throw();
  }
 }

  int check_type_throw(int type_code,int actual_data_type)
  {
   int out_type_code;
   if(actual_data_type!=0)
     out_type_code=actual_data_type;
   else
     out_type_code=type_code;
   otl_var_info_col
     (sl[cur_col].get_pos(),
      sl[cur_col].get_ftype(),
      out_type_code,
      var_info,
      sizeof(var_info));
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return 0;
  if(otl_uncaught_exception()) return 0; 
   throw OTL_TMPL_EXCEPTION
     (otl_error_msg_0,
      otl_error_code_0,
      this->stm_label?
      this->stm_label:
      this->stm_text,
      var_info);
  }

#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
  void strict_check_throw(int type_code)
  {
   otl_var_info_col
     (sl[cur_col].get_pos(),
      sl[cur_col].get_ftype(),
      type_code,
      var_info,
      sizeof(var_info));
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw OTL_TMPL_EXCEPTION
     (otl_error_msg_0,
      otl_error_code_0,
      this->stm_label?
      this->stm_label:
      this->stm_text,
      var_info);
  }
#endif

 int check_type(int type_code,int actual_data_type=0)
 {
   switch(sl[cur_col].get_ftype()){
   case otl_var_timestamp:
   case otl_var_tz_timestamp:
   case otl_var_ltz_timestamp:
     if(type_code==otl_var_timestamp)
       return 1;
     break;
   default:
     if(sl[cur_col].get_ftype()==type_code)
       return 1;
     break;
   }
   return check_type_throw(type_code,actual_data_type);
 }

 void get_next(void)
 {
  if(cur_col<sl_len-1){
   ++cur_col;
   null_fetched=sl[cur_col].is_null(this->cur_row);
  }else{
   eof_status=this->next();
   cur_col=0;
  }
 }

 void look_ahead(void)
 {
  if(cur_col==sl_len-1){
#if defined(OTL_SELECT_STREAM_ALTERNATE_FETCH)
#else
   eof_status=this->next();
   cur_col=-1;
#endif
   ++_rfc;
  }
 }

  OTL_TMPL_SELECT_STREAM& operator>>(char& c)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   c=*OTL_RCAST(char*,sl[cur_col].val(this->cur_row));
   look_ahead();
  }
  return *this;
 }

  OTL_TMPL_SELECT_STREAM& operator>>(unsigned char& c)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   c=*OTL_RCAST(unsigned char*,sl[cur_col].val(this->cur_row));
   look_ahead();
  }
  return *this;
 }


#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
  OTL_TMPL_SELECT_STREAM& operator>>(OTL_STRING_CONTAINER& s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();

  switch(sl[cur_col].get_ftype()){
  case otl_var_raw:
    {
      int len2;
      if(!eof_intern()){
        unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(this->cur_row));
        if(sl[cur_col].get_var_struct().get_otl_adapter()==otl_ora7_adapter||
           sl[cur_col].get_var_struct().get_otl_adapter()==otl_ora8_adapter){
          len2=OTL_SCAST(int,*OTL_RCAST(unsigned short*,c));
          c+=sizeof(short int);
        }else
          len2=sl[cur_col].get_len(this->cur_row);           
#if (defined(OTL_USER_DEFINED_STRING_CLASS_ON) || defined(OTL_STL)) \
     && !defined(OTL_ACE)
        s.assign(OTL_RCAST(char*,c),len2);
#elif defined(OTL_ACE)
        s.set(OTL_RCAST(char*,c),len2,1);
#endif
        look_ahead();
       }
    }
    break;
  case otl_var_char:
    if(!eof_intern()){
#if defined(OTL_ACE)
      s.set(OTL_RCAST(char*,sl[cur_col].val(this->cur_row)),1);
#else
      s=OTL_RCAST(char*,sl[cur_col].val(this->cur_row));
#endif
      look_ahead();
    }
    break;
#if defined(OTL_USER_DEFINED_STRING_CLASS_ON) || \
    defined(OTL_STL) || defined(OTL_ACE)
  case otl_var_varchar_long:
  case otl_var_raw_long:
    if(!eof_intern()){
      unsigned char* c=OTL_RCAST(unsigned char*,
                                 sl[cur_col].val(this->cur_row));
      int len=sl[cur_col].get_len(this->cur_row);
      int buf_sz=sl[cur_col].get_elem_size();
      if(len>buf_sz)len=buf_sz;

#if (defined(OTL_USER_DEFINED_STRING_CLASS_ON) || defined(OTL_STL)) \
     && !defined(OTL_ACE)
      s.assign(OTL_RCAST(char*,c),len);
#elif defined(OTL_ACE)
      s.set(OTL_RCAST(char*,c),len,1);
#endif
      look_ahead();
    }
    break;
  case otl_var_blob:
  case otl_var_clob:
    if(!eof_intern()){
      int len=0;
      int max_long_sz=this->adb->get_max_long_size();
      otl_auto_array_ptr<unsigned char> loc_ptr(max_long_sz);
      unsigned char* temp_buf=loc_ptr.get_ptr();
      int rc=sl[cur_col].get_var_struct().get_blob
        (this->cur_row,
         temp_buf,
         max_long_sz,
         len);
      if(rc==0){
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
        throw OTL_TMPL_EXCEPTION
          (this->adb->get_connect_struct(),
           this->stm_label?this->stm_label:
           this->stm_text);
      }
#if (defined(OTL_USER_DEFINED_STRING_CLASS_ON) || \
     defined(OTL_STL)) && !defined(OTL_ACE)
      s.assign(OTL_RCAST(char*,temp_buf),len);
#elif defined(OTL_ACE)
      s.set(OTL_RCAST(char*,temp_buf),len,1);
#endif
      look_ahead();
    }
    break;
#endif
  default:
    check_type(otl_var_char);
  } // switch
  return *this;
 }
#endif

  OTL_TMPL_SELECT_STREAM& operator>>(char* s)
  {
    check_if_executed();
    if(eof_intern())return *this;
    get_next();
    if(check_type(otl_var_char)&&!eof_intern()){
      otl_strcpy(OTL_RCAST(unsigned char*,s),
                 OTL_RCAST(const unsigned char*,sl[cur_col].val(this->cur_row))
                 );
      look_ahead();
    }
    return *this;
  }

#if defined(OTL_UNICODE_STRING_TYPE)
  OTL_TMPL_SELECT_STREAM& operator<<(const OTL_UNICODE_STRING_TYPE& s)
  {
    check_in_var();
    if(check_in_type(otl_var_char,1)){
      
      int overflow;
#if defined(OTL_C_STR_FOR_UNICODE_STRING_TYPE)
      otl_strcpy4
        (OTL_RCAST(unsigned char*,this->vl[cur_in]->val()),
         OTL_RCAST(unsigned char*,
                   OTL_CCAST(OTL_UNICODE_CHAR_TYPE*,s.OTL_C_STR_FOR_UNICODE_STRING_TYPE())),
#else
      otl_strcpy4
        (OTL_RCAST(unsigned char*,this->vl[cur_in]->val()),
         OTL_RCAST(unsigned char*,
                   OTL_CCAST(OTL_UNICODE_CHAR_TYPE*,s.c_str())),
#endif
         overflow,
         this->vl[cur_in]->get_elem_size(),
         OTL_SCAST(int,s.length())
        );
      if(overflow){
        char temp_var_info[256];
        otl_var_info_var
          (this->vl[cur_in]->get_name(),
           this->vl[cur_in]->get_ftype(),
           otl_var_char,
           temp_var_info,
           sizeof(temp_var_info));
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_4,
           otl_error_code_4,
           this->stm_label?this->stm_label:this->stm_text,
           temp_var_info,
           OTL_RCAST(const void*,s.c_str()),
           OTL_SCAST(int,this->vl[cur_in]->get_elem_size()));
#else
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_4,
           otl_error_code_4,
           this->stm_label?this->stm_label:this->stm_text,
           temp_var_info);
#endif
      }
      this->vl[cur_in]->set_not_null(0);
    }
    get_in_next();
    return *this;
  }

#endif

#if defined(OTL_UNICODE_STRING_TYPE)
  OTL_TMPL_SELECT_STREAM& operator>>(OTL_UNICODE_STRING_TYPE& s)
  {
    check_if_executed();
    if(eof_intern())return *this;
    get_next();
    switch(sl[cur_col].get_ftype()){
    case otl_var_char:
      if(!eof_intern()){
#if defined(OTL_ODBC) || defined(DB2_CLI)
        s=OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,sl[cur_col].val(this->cur_row));
#else

#if defined(OTL_UNICODE_STRING_TYPE_CAST_FROM_CHAR)
        OTL_UNICODE_CHAR_TYPE* temp_s=OTL_RCAST
          (OTL_UNICODE_CHAR_TYPE*,sl[cur_col].val(this->cur_row));
        OTL_UNICODE_STRING_TYPE_CAST_FROM_CHAR(s,temp_s+1,*temp_s);
#else
        OTL_UNICODE_CHAR_TYPE* temp_s=OTL_RCAST
          (OTL_UNICODE_CHAR_TYPE*,sl[cur_col].val(this->cur_row));
        s.assign(temp_s+1,*temp_s);
#endif

#endif
        look_ahead();
      }
      break;
    case otl_var_varchar_long:
      if(!eof_intern()){
#if defined(OTL_ORA_UNICODE_LONG_LENGTH_IN_BYTES)
        int source_len=sl[cur_col].get_var_struct().get_len(this->cur_row);
        OTL_CHAR* source=OTL_RCAST(OTL_CHAR*,sl[cur_col].val(this->cur_row));
        s.assign(OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,source),source_len);
#else
        s=OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,sl[cur_col].val(this->cur_row));
#endif
        look_ahead();
      }
      break;
    case otl_var_clob:
      if(!eof_intern()){
        int len=0;
        int max_long_sz=this->adb->get_max_long_size();
        otl_auto_array_ptr<unsigned short> loc_ptr(max_long_sz);
        unsigned char* temp_buf=OTL_RCAST(unsigned char*,loc_ptr.get_ptr());

        int rc=sl[cur_col].get_var_struct().get_blob
          (this->cur_row,
           temp_buf,
           max_long_sz,
           len);
        if(rc==0){
          if(this->adb)this->adb->increment_throw_count();
          if(this->adb&&this->adb->get_throw_count()>1)return *this;
          if(otl_uncaught_exception()) return *this; 
          throw OTL_TMPL_EXCEPTION
            (this->adb->get_connect_struct(),
             this->stm_label?this->stm_label:
             this->stm_text);
        }
        s=OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,temp_buf);
        look_ahead();
      }
      break;
    default:
      check_type(otl_var_char);
    }
    return *this;
  }
#endif

  OTL_TMPL_SELECT_STREAM& operator>>(unsigned char* s)
  {
    check_if_executed();
    if(eof_intern())return *this;
    get_next();
    if(check_type(otl_var_char)&&!eof_intern()){
      otl_strcpy2(OTL_RCAST(unsigned char*,s),
                  OTL_RCAST(unsigned char*,sl[cur_col].val(this->cur_row)),
                  sl[cur_col].get_len(this->cur_row)
                  );
      look_ahead();
    }
    return *this;
  }

#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
#define OTL_D1(T,T_type)                                        \
  OTL_TMPL_SELECT_STREAM& operator>>(T& n)                      \
  {                                                             \
    check_if_executed();                                        \
    if(eof_intern())return *this;                               \
    get_next();                                                 \
    if(!eof_intern()){                                          \
      int match_found=otl_numeric_convert_T<T,T_type>           \
        (sl[cur_col].get_ftype(),                               \
         sl[cur_col].val(this->cur_row),                        \
         n);                                                    \
      if(!match_found)                                          \
        strict_check_throw(T_type);                             \
        look_ahead();                                           \
    }                                                           \
    return *this;                                               \
  }
#else
#define OTL_D1(T,T_type)                                        \
  OTL_TMPL_SELECT_STREAM& operator>>(T& n)                      \
  {                                                             \
    check_if_executed();                                        \
    if(eof_intern())return *this;                               \
    get_next();                                                 \
    if(!eof_intern()){                                          \
      int match_found=otl_numeric_convert_T                     \
        (sl[cur_col].get_ftype(),                               \
         sl[cur_col].val(this->cur_row),                        \
         n);                                                    \
      if(!match_found){                                         \
        if(check_type(otl_var_double,T_type))                   \
        n=OTL_PCONV(T,double,sl[cur_col].val(this->cur_row));   \
      }                                                         \
      look_ahead();                                             \
   }                                                            \
   return *this;                                                \
  }
#endif


#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  OTL_D1(int,otl_var_int)
#if defined(OTL_BIGINT)
  OTL_D1(OTL_BIGINT,otl_var_bigint)
#endif
#if defined(OTL_UBIGINT)
  OTL_D1(OTL_UBIGINT,otl_var_ubigint)
#endif
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
  OTL_D1(OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1)
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
  OTL_D1(OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2)
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
  OTL_D1(OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3)
#endif
  OTL_D1(unsigned,otl_var_unsigned_int)
  OTL_D1(long,otl_var_long_int)
  OTL_D1(short,otl_var_short)
  OTL_D1(float,otl_var_float)
  OTL_D1(double,otl_var_double)

#else
  template<OTL_TYPE_NAME T,const int T_type> OTL_D1(T,T_type)
#endif

  OTL_TMPL_SELECT_STREAM& operator>>(TTimestampStruct& t)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_timestamp)&&!eof_intern()){
   TTimestampStruct* tm=
    OTL_RCAST(TTimestampStruct*,sl[cur_col].val(this->cur_row));
   int rc=sl[cur_col].get_var_struct().read_dt
     (&t,tm,sizeof(TTimestampStruct));
   if(rc==0){
     if(this->adb)this->adb->increment_throw_count();
     if(this->adb&&this->adb->get_throw_count()>1)return *this;
     if(otl_uncaught_exception()) return *this; 
     throw OTL_TMPL_EXCEPTION
       (this->adb->get_connect_struct(),
        this->stm_label?this->stm_label:
        this->stm_text);
   }
   look_ahead();
  }
  return *this;
 }

  OTL_TMPL_SELECT_STREAM& operator>>(otl_long_string& s)
 {
   check_if_executed();
   if(eof_intern())return *this;
   get_next();
   switch(sl[cur_col].get_ftype()){
   case otl_var_raw_long:
   {
     if(s.get_unicode_flag()){
       throw OTL_TMPL_EXCEPTION
         (otl_error_msg_38,
          otl_error_code_38,
          this->stm_label?this->stm_label:
          this->stm_text);
     }
     if(!eof_intern()){
       unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(this->cur_row));
       int len2=sl[cur_col].get_len(this->cur_row);
       if(len2>s.get_buf_size())
         len2=s.get_buf_size();
       otl_memcpy(s.v,c,len2,sl[cur_col].get_ftype());
       s.set_len(len2);
       look_ahead();
     }
   }
   break;
   case otl_var_varchar_long:
   {
     bool in_unicode_mode=sizeof(OTL_CHAR)>1;
     if(s.get_unicode_flag() != in_unicode_mode){
       throw OTL_TMPL_EXCEPTION
         (otl_error_msg_37,
          otl_error_code_37,
          this->stm_label?this->stm_label:
          this->stm_text);
     }
     if(!eof_intern()){
       if(sl[cur_col].get_var_struct().get_otl_adapter()==otl_ora8_adapter){
#if defined(OTL_UNICODE)
         int len2=0;
         OTL_CHAR* source=OTL_RCAST(OTL_CHAR*,sl[cur_col].val(this->cur_row));
         OTL_CHAR* target=OTL_RCAST(OTL_CHAR*,s.v);
#if defined(OTL_ORA_UNICODE_LONG_LENGTH_IN_BYTES)
         int source_len=sl[cur_col].get_var_struct().get_len(this->cur_row);
         while(*source && len2<s.get_buf_size() && len2<source_len){
           *target++=*source++;
           ++len2;
         }
#else
         while(*source && len2<s.get_buf_size()){
           *target++=*source++;
           ++len2;
         }
#endif
         s.null_terminate_string(len2);
         s.set_len(len2);
         look_ahead();
#else
         unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(this->cur_row));
         int len2=sl[cur_col].get_len(this->cur_row);
         if(len2>s.get_buf_size())
           len2=s.get_buf_size();
         otl_memcpy(s.v,c,len2,sl[cur_col].get_ftype());
         s.null_terminate_string(len2);
         s.set_len(len2);
         look_ahead();
#endif
       }else{
         unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(this->cur_row));
         int len2=sl[cur_col].get_len(this->cur_row);
         if(len2>s.get_buf_size())
           len2=s.get_buf_size();
         otl_memcpy(s.v,c,len2,sl[cur_col].get_ftype());
         s.null_terminate_string(len2);
         s.set_len(len2);
         look_ahead();
       }
     }
   }
   break;
   case otl_var_raw:
     {
       if(s.get_unicode_flag()){
         throw OTL_TMPL_EXCEPTION
           (otl_error_msg_38,
            otl_error_code_38,
            this->stm_label?this->stm_label:
            this->stm_text);
       }
       if(!eof_intern()){
         unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(this->cur_row));
         if(sl[cur_col].get_var_struct().get_otl_adapter()==otl_ora7_adapter||
            sl[cur_col].get_var_struct().get_otl_adapter()==otl_ora8_adapter){
           int len2=OTL_SCAST(int,*OTL_RCAST(unsigned short*,c));
           otl_memcpy(s.v,c+sizeof(short int),len2,sl[cur_col].get_ftype());
           s.set_len(len2);
         }else{
           int len2=sl[cur_col].get_len(this->cur_row);           
           if(len2>s.get_buf_size())
             len2=s.get_buf_size();
           otl_memcpy(s.v,c,len2,sl[cur_col].get_ftype());
           s.set_len(len2);
         }
         look_ahead();
       }
     }
     break;
   case otl_var_blob:
   case otl_var_clob:
   {
     bool in_unicode_mode=sizeof(OTL_CHAR)>1;
     if(!s.get_unicode_flag() && in_unicode_mode &&
        sl[cur_col].get_ftype()==otl_var_clob){
       throw OTL_TMPL_EXCEPTION
         (otl_error_msg_37,
          otl_error_code_37,
          this->stm_label?this->stm_label:
          this->stm_text);
     }else if(s.get_unicode_flag() && sl[cur_col].get_ftype()==otl_var_blob){
       throw OTL_TMPL_EXCEPTION
         (otl_error_msg_38,
          otl_error_code_38,
          this->stm_label?this->stm_label:
          this->stm_text);
     }
     if(!eof_intern()){
       int len=0;
       int rc=sl[cur_col].get_var_struct().get_blob
         (this->cur_row,s.v,s.get_buf_size(),len);
       if(rc==0){
         if(this->adb)this->adb->increment_throw_count();
         if(this->adb&&this->adb->get_throw_count()>1)return *this;
         if(otl_uncaught_exception()) return *this; 
         throw OTL_TMPL_EXCEPTION
           (this->adb->get_connect_struct(),
            this->stm_label?this->stm_label:
            this->stm_text);
       }
       if(len>s.get_buf_size())
         len=s.get_buf_size();
       s.set_len(len);
       if(sl[cur_col].get_ftype()==otl_var_clob)
         s.null_terminate_string(len);
         look_ahead();
     }      
   }
   break;
   default:
     {
       char tmp_var_info[256];
       otl_var_info_col
         (sl[cur_col].get_pos(),
          sl[cur_col].get_ftype(),
          otl_var_long_string,
          tmp_var_info,
          sizeof(tmp_var_info));
       if(this->adb)this->adb->increment_throw_count();
       if(this->adb&&this->adb->get_throw_count()>1)return *this;
       if(otl_uncaught_exception()) return *this; 
       throw OTL_TMPL_EXCEPTION
         (otl_error_msg_0,
          otl_error_code_0,
          this->stm_label?this->stm_label:
          this->stm_text,
          tmp_var_info);
     }
   }
   return *this;
 }

#if defined(OTL_ORA8)||defined(OTL_ODBC)
  OTL_TMPL_SELECT_STREAM& operator>>
  (otl_lob_stream_generic& s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(s.get_ora_lob() &&
     (sl[cur_col].get_ftype()==otl_var_blob||
      sl[cur_col].get_ftype()==otl_var_clob)&&
     !eof_intern()){
   s.init
     (OTL_RCAST(void*,&sl[cur_col]),
      OTL_RCAST(void*,this->adb),
      OTL_RCAST(void*,this),
      this->cur_row,
      otl_lob_stream_read_mode,
      this->is_null());
   delay_next=1;
  }else if((sl[cur_col].get_ftype()==otl_var_varchar_long||
            sl[cur_col].get_ftype()==otl_var_raw_long)&&
           !eof_intern()){
   s.init
    (OTL_RCAST(void*,&sl[cur_col]),
     OTL_RCAST(void*,this->adb),
     OTL_RCAST(void*,this),
     this->cur_row,
     otl_lob_stream_read_mode);
   delay_next=1;
  }else{
   char tmp_var_info[256];
   otl_var_info_col
     (sl[cur_col].get_pos(),
      sl[cur_col].get_ftype(),
      otl_var_long_string,
      tmp_var_info,
      sizeof(tmp_var_info));
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return *this;
   if(otl_uncaught_exception()) return *this; 
   throw OTL_TMPL_EXCEPTION
    (otl_error_msg_0,
     otl_error_code_0,
     this->stm_label?this->stm_label:
     this->stm_text,
     tmp_var_info);
  }
  return *this;
 }
#endif

  int check_in_type_throw(int type_code)
  {
    otl_var_info_var
      (this->vl[cur_in]->get_name(),
       this->vl[cur_in]->get_ftype(),
       type_code,
       var_info,
       sizeof(var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return 0;
    if(otl_uncaught_exception()) return 0; 
    throw OTL_TMPL_EXCEPTION
      (otl_error_msg_0,
       otl_error_code_0,
       this->stm_label?this->stm_label:
       this->stm_text,
       var_info);
  }

  int check_in_type(int type_code,int tsize)
  {
    switch(this->vl[cur_in]->get_ftype()){
    case otl_var_char:
      if(type_code==otl_var_char)
        return 1;
    case otl_var_raw:
      if(type_code==otl_var_raw)
        return 1;
    case otl_var_db2date:
    case otl_var_db2time:
    case otl_var_timestamp:
    case otl_var_tz_timestamp:
    case otl_var_ltz_timestamp:
      if(type_code==otl_var_timestamp)
        return 1;
    default:
      if(this->vl[cur_in]->get_ftype()==type_code &&
         this->vl[cur_in]->get_elem_size()==tsize)
        return 1;
    }
    return check_in_type_throw(type_code);
  }

  void check_in_var_throw(void)
  {
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
    throw OTL_TMPL_EXCEPTION
      (otl_error_msg_1,
       otl_error_code_1,
       this->stm_label?this->stm_label:
       this->stm_text,
       nullptr);
  }

  void check_in_var(void)
  {
    if(this->vl_len==0)
      check_in_var_throw();
  }

 void get_in_next(void)
 {
  if(cur_in==this->vl_len-1)
   rewind();
  else{
   ++cur_in;
   executed=0;
  }
 }

  OTL_TMPL_SELECT_STREAM& operator<<(const otl_null& /* n */)
 {
  check_in_var();
  this->vl[cur_in]->set_null(0);
  get_in_next();
  return *this;
 }

 OTL_TMPL_SELECT_STREAM& operator<<(const char c)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){
   char* tmp=OTL_RCAST(char*,this->vl[cur_in]->val());
   tmp[0]=c;
   tmp[1]=0;
   this->vl[cur_in]->set_not_null(0);
  }
  get_in_next();
  return *this;
 }

  OTL_TMPL_SELECT_STREAM& operator<<(const unsigned char c)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){
   unsigned char* tmp=
    OTL_RCAST(unsigned char*,this->vl[cur_in]->val());
   tmp[0]=c;
   tmp[1]=0;
   this->vl[cur_in]->set_not_null(0);
  }
  get_in_next();
  return *this;
 }

 OTL_TMPL_SELECT_STREAM& operator<<(const char* s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy
    (OTL_RCAST(unsigned char*,this->vl[cur_in]->val()),
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,s)),
     overflow,
     this->vl[cur_in]->get_elem_size()
    );
   if(overflow){
    char tmp_var_info[256];
    otl_var_info_var
      (this->vl[cur_in]->get_name(),
       this->vl[cur_in]->get_ftype(),
       otl_var_char,
       tmp_var_info,
       sizeof(tmp_var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return *this;
    if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_4,
           otl_error_code_4,
           this->stm_label?this->stm_label:this->stm_text,
           tmp_var_info,
           OTL_RCAST(const void*,s),
           OTL_SCAST(int,this->vl[cur_in]->get_elem_size()));
#else
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_4,
           otl_error_code_4,
           this->stm_label?this->stm_label:this->stm_text,
           tmp_var_info);
#endif
   }

   this->vl[cur_in]->set_not_null(0);

  }
  get_in_next();
  return *this;
 }

 OTL_TMPL_SELECT_STREAM& operator<<(const otl_long_string& s)
 {
  check_in_var();
  switch(this->vl[cur_in]->get_ftype()){
  case otl_var_varchar_long:
    {
      bool in_unicode_mode=sizeof(OTL_CHAR)>1;
      if(!s.get_unicode_flag() && in_unicode_mode){
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_37,
           otl_error_code_37,
           this->stm_label?this->stm_label:
           this->stm_text);
      }
      unsigned char* c=OTL_RCAST(unsigned char*,this->vl[cur_in]->val(0));
      int len=OTL_CCAST(otl_long_string*,&s)->len();
      this->vl[cur_in]->set_not_null(0);
      if(len>this->vl[cur_in]->actual_elem_size()){
        otl_var_info_var
          (this->vl[cur_in]->get_name(),
           this->vl[cur_in]->get_ftype(),
           otl_var_long_string,
           var_info,
           sizeof(var_info));
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_5,
           otl_error_code_5,
           this->stm_label?this->stm_label:
           this->stm_text,
           var_info);
      }
      otl_memcpy(c,s.v,len,this->vl[cur_in]->get_ftype());
      this->vl[cur_in]->set_len(len,0);
    }
    break;
  case otl_var_raw_long:
    {
      if(s.get_unicode_flag()){
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_38,
           otl_error_code_38,
           this->stm_label?this->stm_label:
           this->stm_text);
      }
      unsigned char* c=OTL_RCAST(unsigned char*,this->vl[cur_in]->val(0));
      int len=OTL_CCAST(otl_long_string*,&s)->len();
      if(len>this->vl[cur_in]->actual_elem_size()){
        otl_var_info_var
          (this->vl[cur_in]->get_name(),
           this->vl[cur_in]->get_ftype(),
           otl_var_char,
           var_info,
           sizeof(var_info));
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_5,
           otl_error_code_5,
           this->stm_label?this->stm_label:
           this->stm_text,
           var_info);
      }
      this->vl[cur_in]->set_not_null(0);
      otl_memcpy(c,s.v,len,this->vl[cur_in]->get_ftype());
      this->vl[cur_in]->set_len(len,0);
    }
    break;
  case otl_var_raw:
    {
      if(s.get_unicode_flag()){
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_38,
           otl_error_code_38,
           this->stm_label?this->stm_label:
           this->stm_text);
      }
      unsigned char* c=OTL_RCAST(unsigned char*,this->vl[cur_in]->val(0));
      int len=OTL_CCAST(otl_long_string*,&s)->len();
      if(len>this->vl[cur_in]->actual_elem_size()){
        otl_var_info_var
          (this->vl[cur_in]->get_name(),
           this->vl[cur_in]->get_ftype(),
           otl_var_raw,
           var_info,
           sizeof(var_info));
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_5,
           otl_error_code_5,
           this->stm_label?this->stm_label:
           this->stm_text,
           var_info);
      }
      this->vl[cur_in]->set_not_null(0);
      if((this->vl[cur_in]->get_var_struct().get_otl_adapter()==otl_ora7_adapter||
          this->vl[cur_in]->get_var_struct().get_otl_adapter()==otl_ora8_adapter) &&
         this->vl[cur_in]->get_ftype()==otl_var_raw){
        otl_memcpy
          (c+sizeof(unsigned short),
           s.v,
           len,
           this->vl[cur_in]->get_ftype());
        *OTL_RCAST(unsigned short*,
                   this->vl[cur_in]->val(0))=OTL_SCAST(unsigned short,len);
        this->vl[cur_in]->set_len(len,0);
      }else{
        otl_memcpy(c,s.v,len,this->vl[cur_in]->get_ftype());
        this->vl[cur_in]->set_len(len,0);
      }
    }
    break;
  }
  get_in_next();
  return *this;
 }


#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
  OTL_TMPL_SELECT_STREAM& operator<<(const OTL_STRING_CONTAINER& s)
  {
    check_in_var();
    if(this->vl[cur_in]->get_ftype()==otl_var_raw){
      unsigned char* c=OTL_RCAST(unsigned char*,this->vl[cur_in]->val(0));
      int len=OTL_SCAST(int,s.length());
      if(len>this->vl[cur_in]->actual_elem_size()){
        otl_var_info_var
          (this->vl[cur_in]->get_name(),
           this->vl[cur_in]->get_ftype(),
           otl_var_raw,
           var_info,
           sizeof(var_info));
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_5,
           otl_error_code_5,
           this->stm_label?this->stm_label:
           this->stm_text,
           var_info);
      }
      this->vl[cur_in]->set_not_null(0);
      if((this->vl[cur_in]->get_var_struct().get_otl_adapter()==otl_ora7_adapter||
          this->vl[cur_in]->get_var_struct().get_otl_adapter()==otl_ora8_adapter)){
        otl_memcpy
          (c+sizeof(unsigned short),
           OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
           len,
           this->vl[cur_in]->get_ftype());
        *OTL_RCAST(unsigned short*,
                   this->vl[cur_in]->val(0))=OTL_SCAST(unsigned short,len);
        this->vl[cur_in]->set_len(len,0);
      }else{
        otl_memcpy(c,
                   OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
                   len,
                   this->vl[cur_in]->get_ftype());
        this->vl[cur_in]->set_len(len,0);
      }
    }else if(this->vl[cur_in]->get_ftype()==otl_var_char){
      int overflow;
      otl_strcpy
        (OTL_RCAST(unsigned char*,this->vl[cur_in]->val()),
         OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
         overflow,
         this->vl[cur_in]->get_elem_size(),
         OTL_SCAST(int,s.length())
        );
      if(overflow){
        char temp_var_info[256];
        otl_var_info_var
          (this->vl[cur_in]->get_name(),
           this->vl[cur_in]->get_ftype(),
           otl_var_char,
           temp_var_info,
           sizeof(temp_var_info));
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_4,
           otl_error_code_4,
           this->stm_label?this->stm_label:this->stm_text,
           temp_var_info,
           OTL_RCAST(const void*,s.c_str()),
           OTL_SCAST(int,this->vl[cur_in]->get_elem_size()));
#else
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_4,
           otl_error_code_4,
           this->stm_label?this->stm_label:
           this->stm_text,
           temp_var_info);
#endif
      }
      
      this->vl[cur_in]->set_not_null(0);
      
    }else
      check_in_type_throw(otl_var_char);
    get_in_next();
    return *this;
  }

#endif

  OTL_TMPL_SELECT_STREAM& operator<<(const unsigned char* s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy4
    (OTL_RCAST(unsigned char*,this->vl[cur_in]->val()),
     OTL_CCAST(unsigned char*,s),
     overflow,
     this->vl[cur_in]->get_elem_size()
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
      (this->vl[cur_in]->get_name(),
       this->vl[cur_in]->get_ftype(),
       otl_var_char,
       temp_var_info,
       sizeof(temp_var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return *this;
    if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
    throw OTL_TMPL_EXCEPTION
      (otl_error_msg_4,
       otl_error_code_4,
       this->stm_label?this->stm_label:this->stm_text,
       temp_var_info,
       OTL_RCAST(const void*,s),
       OTL_SCAST(int,this->vl[cur_in]->get_elem_size()));
#else
    throw OTL_TMPL_EXCEPTION
      (otl_error_msg_4,
       otl_error_code_4,
       this->stm_label?this->stm_label:this->stm_text,
       temp_var_info);
#endif
   }

   this->vl[cur_in]->set_not_null(0);

  }
  get_in_next();
  return *this;
 }

#define OTL_D2(T,T_type)                        \
  OTL_TMPL_SELECT_STREAM& operator<<(const T n) \
  {                                             \
    check_in_var();                             \
    if(check_in_type(T_type,sizeof(T))){        \
      *OTL_RCAST(T*,this->vl[cur_in]->val())=n; \
    }                                           \
    this->vl[cur_in]->set_not_null(0);          \
    get_in_next();                              \
    return *this;                               \
  }

#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  OTL_D2(int,otl_var_int)
  OTL_D2(unsigned,otl_var_unsigned_int)
#if defined(OTL_BIGINT)
  OTL_D2(OTL_BIGINT,otl_var_bigint)
#endif
#if defined(OTL_UBIGINT)
  OTL_D2(OTL_UBIGINT,otl_var_ubigint)
#endif
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
  OTL_D2(OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1)
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
  OTL_D2(OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2)
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
  OTL_D2(OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3)
#endif
  OTL_D2(long,otl_var_long_int)
  OTL_D2(short,otl_var_short)
  OTL_D2(float,otl_var_float)
  OTL_D2(double,otl_var_double)
#else
  template<OTL_TYPE_NAME T,const int T_type> OTL_D2(T,T_type)
#endif

  OTL_TMPL_SELECT_STREAM& operator<<(const TTimestampStruct& t)
 {
  check_in_var();
  if(check_in_type(otl_var_timestamp,sizeof(TTimestampStruct))){
   TTimestampStruct* tm=
    OTL_RCAST(TTimestampStruct*,this->vl[cur_in]->val());
   int rc=this->vl[cur_in]->get_var_struct().write_dt
     (tm,&t,sizeof(TTimestampStruct));
   if(rc==0){
     if(this->adb)this->adb->increment_throw_count();
     if(this->adb&&this->adb->get_throw_count()>1)return *this;
     if(otl_uncaught_exception()) return *this; 
     throw OTL_TMPL_EXCEPTION
       (this->adb->get_connect_struct(),
        this->stm_label?this->stm_label:
        this->stm_text);
   }
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_tmpl_select_stream(const otl_tmpl_select_stream&) = delete;
 otl_tmpl_select_stream& operator=(const otl_tmpl_select_stream&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_tmpl_select_stream(otl_tmpl_select_stream&&) = delete;
 otl_tmpl_select_stream& operator=(otl_tmpl_select_stream&&) = delete;
#endif
private:
#else
 otl_tmpl_select_stream
 (const otl_tmpl_select_stream&): 
   OTL_TMPL_SELECT_CURSOR(),
   sl_desc(nullptr),
   sl(nullptr),
   sl_len(0),
   null_fetched(0),
   cur_col(0),
   cur_in(0),
   executed(0),
   eof_status(0),
   var_info(),
   override_(nullptr),
   delay_next(0),
   lob_stream_mode(false),
   _rfc(0)
 {
 }

 otl_tmpl_select_stream& operator=(const otl_tmpl_select_stream&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_tmpl_select_stream
 (otl_tmpl_select_stream&&): 
   OTL_TMPL_SELECT_CURSOR(),
   sl_desc(nullptr),
   sl(nullptr),
   sl_len(0),
   null_fetched(0),
   cur_col(0),
   cur_in(0),
   executed(0),
   eof_status(0),
   var_info(),
   override_(nullptr),
   delay_next(0),
   lob_stream_mode(false),
   _rfc(0)
 {
 }

 otl_tmpl_select_stream& operator=(otl_tmpl_select_stream&&)
 {
   return *this;
 }
#endif
#endif

};

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct,
          OTL_TYPE_NAME TTimestampStruct>
class otl_tmpl_out_stream: public OTL_TMPL_CURSOR{

protected:

  int auto_commit_flag;
  int dirty;
  int cur_x;
  int cur_y;
  otl_stream_buffer_size_type array_size;
  int in_exception_flag;
  int in_destruct_flag;
  int should_delete_flag;
  char var_info[256];
  bool flush_flag;
  bool flush_flag2;
  bool lob_stream_mode;
  void* master_stream_ptr_;

public:

  int get_dirty_buf_len() const 
  {
    if(dirty)
      return cur_y+1;
    else
      return 0;
  }

  void set_flush_flag(const bool aflush_flag)
  {
    flush_flag=aflush_flag;
  }

  void set_flush_flag2(const bool aflush_flag2)
  {
    flush_flag2=aflush_flag2;
  }

 void cleanup(void)
 {int i;
  if(should_delete_flag){
   for(i=0;i<this->vl_len;++i)
    delete this->vl[i];
  }
  delete[] this->vl;
 }

 otl_tmpl_out_stream
 (otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  OTL_TMPL_CONNECT& db,
  void* master_stream_ptr,
  const bool alob_stream_mode=false,
  const char* sqlstm_label=0):
   OTL_TMPL_CURSOR(db),
   auto_commit_flag(0),
   dirty(0),
   cur_x(0),
   cur_y(0),
   array_size(0),
   in_exception_flag(0),
   in_destruct_flag(0),
   should_delete_flag(0),
   var_info(),
   flush_flag(0),
   flush_flag2(0),
   lob_stream_mode(0),
   master_stream_ptr_(master_stream_ptr)
 {
   int i;
   if(sqlstm_label!=nullptr){
     if(this->stm_label!=nullptr){
       delete[] this->stm_label;
       this->stm_label=nullptr;
     }
     size_t len=strlen(sqlstm_label)+1;
     this->stm_label=new char[len];
     OTL_STRCPY_S(this->stm_label,len,sqlstm_label);
   }
   dirty=0;
   auto_commit_flag=1;
   flush_flag=true;
   flush_flag2=true;
   lob_stream_mode=alob_stream_mode;
   this->cursor_struct.last_param_data_token=0;
   this->cursor_struct.last_sql_param_data_status=0;
   this->cursor_struct.sql_param_data_count=0;
   cur_x=-1;
   cur_y=0;
   should_delete_flag=1;
   in_exception_flag=0;
   in_destruct_flag=0;
   this->stm_text=0;
   array_size=arr_size;
   {
     int len=strlen(sqlstm)+1;
     this->stm_text=new char[len];
     OTL_STRCPY_S(this->stm_text,len,sqlstm);
     otl_tmpl_ext_hv_decl
       <TVariableStruct,TTimestampStruct,TExceptionStruct,
       TConnectStruct,TCursorStruct> hvd(this->stm_text,arr_size);
     hvd.alloc_host_var_list(this->vl,this->vl_len,db);
   }
   try{
     this->parse();
     for(i=0;i<this->vl_len;++i){
       if(this->vl[i]->get_var_struct().otl_adapter==otl_odbc_adapter){
         this->vl[i]->get_var_struct().lob_stream_mode=lob_stream_mode;
         this->vl[i]->get_var_struct().vparam_type=this->vl[i]->get_param_type();
         if(this->vl[i]->get_ftype()==otl_var_varchar_long||
            this->vl[i]->get_ftype()==otl_var_raw_long){
           this->vl[i]->set_not_null(0);
         }
       }
       bind(*(this->vl[i]));
     }
   }catch(OTL_CONST_EXCEPTION OTL_TMPL_EXCEPTION&){
     cleanup();
     if(this->adb)this->adb->increment_throw_count();
     throw;
   }
 }

 otl_tmpl_out_stream
 (OTL_TMPL_CONNECT& pdb,
  void* master_stream_ptr,
  const bool alob_stream_mode=false,
  const char* sqlstm_label=nullptr):
   OTL_TMPL_CURSOR(pdb),
   auto_commit_flag(0),
   dirty(0),
   cur_x(0),
   cur_y(0),
   array_size(0),
   in_exception_flag(0),
   in_destruct_flag(0),
   should_delete_flag(0),
   var_info(),
   flush_flag(0),
   flush_flag2(0),
   lob_stream_mode(0),
   master_stream_ptr_(master_stream_ptr)
 {
   if(sqlstm_label!=nullptr){
     if(this->stm_label!=nullptr){
       delete[] this->stm_label;
       this->stm_label=nullptr;
     }
     size_t len=strlen(sqlstm_label)+1;
     this->stm_label=new char[len];
     OTL_STRCPY_S(this->stm_label,len,sqlstm_label);
   }
   should_delete_flag=1;
   in_exception_flag=0;
   in_destruct_flag=0;
   dirty=0;
   auto_commit_flag=1;
   flush_flag=true;
   flush_flag2=true;
   lob_stream_mode=alob_stream_mode;
   this->cursor_struct.reset_last_param_data_token();
   this->cursor_struct.reset_last_sql_param_data_status();
   this->cursor_struct.reset_sql_param_data_count();
   cur_x=-1;
   cur_y=0;
   this->stm_text=nullptr;
 }

 virtual ~otl_tmpl_out_stream()
 {in_destruct_flag=1;
  this->in_destructor=1;
  if(dirty&&!in_exception_flag&&
     flush_flag&&flush_flag2)
   flush();
  cleanup();
  in_destruct_flag=0;
 }

  void reset_to_last_valid_row()
  {
    if(cur_y>0){
      --cur_y;
      this->in_exception_flag=0;
      cur_x=this->vl_len-1;
    }
  }

 virtual void flush(const int rowoff=0,const bool force_flush=false)
 {int i,rc;

 this->_rpc=0;

  if(!dirty)return;
  if(!flush_flag2)return;

  if(force_flush){
    if(rowoff>cur_y){
      clean();
      return;
    }
    int temp_rc;
    OTL_TRACE_STREAM_EXECUTION2
    this->exec(OTL_SCAST(otl_stream_buffer_size_type,(cur_y+1)),
               rowoff,
               otl_sql_exec_from_cursor_class);
    for(i=0;i<this->vl_len;++i){
      temp_rc=this->vl[i]->get_var_struct().put_blob();
      if(temp_rc==0){
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return;
        if(otl_uncaught_exception()) return; 
        throw OTL_TMPL_EXCEPTION
          (this->adb->get_connect_struct(),
           this->stm_label?this->stm_label:
           this->stm_text);
      }
    }
    if(auto_commit_flag)
      this->adb->commit();
    clean();
    return;
  }

#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
  if(otl_uncaught_exception()){
   clean();
   return; 
  }
#elif defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(otl_uncaught_exception()){
     clean();
     return; 
  }
#endif

   if(this->retcode==0||this->adb->get_retcode()==0){
    clean();
    return;
  }
  if(cur_x!=this->vl_len-1){
   in_exception_flag=1;
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return;
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(
otl_uncaught_exception()){
    clean();
    return; 
   }
#elif defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(otl_uncaught_exception()){
     clean();
     return; 
   }
#endif
   throw OTL_TMPL_EXCEPTION
    (otl_error_msg_3,
     otl_error_code_3,
     this->stm_label?this->stm_label:
     this->stm_text,
     nullptr);
  }
  if(in_destruct_flag){
    OTL_TRACE_STREAM_EXECUTION2
    this->retcode=this->cursor_struct.exec
      (cur_y+1,
       rowoff,
       otl_sql_exec_from_cursor_class);
    for(i=0;i<this->vl_len;++i){
      rc=this->vl[i]->get_var_struct().put_blob();
      if(rc==0){
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return;
        if(otl_uncaught_exception()) return; 
        throw OTL_TMPL_EXCEPTION
          (this->adb->get_connect_struct(),
           this->stm_label?this->stm_label:
           this->stm_text);
      }
    }
    if(!this->retcode){
    clean();
    in_exception_flag=1;
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return;
    if(otl_uncaught_exception()) return;
    throw OTL_TMPL_EXCEPTION
      (this->cursor_struct,
       this->stm_label?this->stm_label:
       this->stm_text);
   }
   if(auto_commit_flag){
     this->adb->set_retcode(this->adb->get_connect_struct().commit());
     if(!this->adb->get_retcode()){
     clean();
     if(this->adb)this->adb->increment_throw_count();
     if(this->adb&&this->adb->get_throw_count()>1)return;
     if(otl_uncaught_exception()) return; 
     throw OTL_TMPL_EXCEPTION
       (this->adb->get_connect_struct(),
        this->stm_label?this->stm_label:
        this->stm_text);
    }
   }
  }else{
    int temp_rc;
    OTL_TRACE_STREAM_EXECUTION2
    this->exec(OTL_SCAST(otl_stream_buffer_size_type,(cur_y+1)),
               rowoff,
               otl_sql_exec_from_cursor_class);
    long curr_rpc=this->get_rpc();
    for(i=0;i<this->vl_len;++i){
      int otl_adapter_type=this->vl[i]->get_const_var_struct().get_otl_adapter();
      if(!(otl_adapter_type==otl_ora8_adapter&&curr_rpc==0)){
        temp_rc=this->vl[i]->get_var_struct().put_blob();
        if(temp_rc==0){
          if(this->adb)this->adb->increment_throw_count();
          if(this->adb&&this->adb->get_throw_count()>1)return;
          if(otl_uncaught_exception()) return; 
          throw OTL_TMPL_EXCEPTION
            (this->adb->get_connect_struct(),
             this->stm_label?this->stm_label:
             this->stm_text);
        }
      }
    }
   if(auto_commit_flag)
    this->adb->commit();
   if(rowoff>0)
     clean();
   else
     clean(0);
  }

 }

 virtual void clean(const int clean_up_error_flag=0)
 {
   if(clean_up_error_flag){             
     this->retcode=1;
     this->in_exception_flag=0;
   }
   if(!dirty)return;
   cur_x=-1;
   cur_y=0;
   dirty=0;
 }

 bool get_error_state(void) const
 {
   if(this->retcode==0||this->in_exception_flag==1)
     return true;
   else
     return false;
 }

 void set_commit(int auto_commit=0)
 {
   auto_commit_flag=auto_commit;
 }

 void get_next(void)
 {
  if(cur_x<this->vl_len-1)
   ++cur_x;
  else{
   if(cur_y<array_size-1){
    ++cur_y;
    cur_x=0;
   }else{
    flush();
    cur_x=0;
   }
  }
  dirty=1;
 }

  int check_type_throw(int type_code)
  {
   in_exception_flag=1;
   otl_var_info_var
     (this->vl[cur_x]->get_name(),
      this->vl[cur_x]->get_ftype(),
      type_code,
      var_info,
      sizeof(var_info));
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return 0;
   if(otl_uncaught_exception()) return 0; 
   throw OTL_TMPL_EXCEPTION
     (otl_error_msg_0,
      otl_error_code_0,
      this->stm_label?this->stm_label:
      this->stm_text,
      var_info);
  }

 int check_type(int type_code, int tsize)
 {
   switch(this->vl[cur_x]->get_ftype()){
   case otl_var_char:
     if(type_code==otl_var_char)return 1;
     break;
   case otl_var_bfloat:
     if(type_code==otl_var_float)return 1;
     break;
   case otl_var_bdouble:
     if(type_code==otl_var_double)return 1;
     break;
   case otl_var_db2time:
   case otl_var_tz_timestamp:
   case otl_var_ltz_timestamp:
   case otl_var_db2date:
     if(type_code==otl_var_timestamp)return 1;
     break;
   case otl_var_refcur:
     if(type_code==otl_var_refcur)return 1;
     break;
   default:
     if(this->vl[cur_x]->get_ftype()==type_code &&
        this->vl[cur_x]->get_elem_size()==tsize)
       return 1;
     break;
   }
   return check_type_throw(type_code);
 }

 void check_buf(void)
 {
  if(cur_x==this->vl_len-1 && cur_y==array_size-1)
   flush();
 }

 OTL_TMPL_OUT_STREAM& operator<<(const char c)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_char,1)){
    char* tmp=OTL_RCAST(char*,this->vl[cur_x]->val(cur_y));
    tmp[0]=c;
    tmp[1]=0;
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

 OTL_TMPL_OUT_STREAM& operator<<(const unsigned char c)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_char,1)){
    unsigned char* tmp=OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y));
    tmp[0]=c;
    tmp[1]=0;
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 OTL_TMPL_OUT_STREAM& operator<<(const OTL_STRING_CONTAINER& s)
 {
   if(this->vl_len>0){
     get_next();
     
     switch(this->vl[cur_x]->get_ftype()){
#if defined(OTL_USER_DEFINED_STRING_CLASS_ON) || \
     defined(OTL_STL) || defined(OTL_ACE)
     case otl_var_raw:
       {
         unsigned char* c=OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y));
         int len=OTL_SCAST(int,s.length());
         this->vl[cur_x]->set_not_null(cur_y);
         if(len>this->vl[cur_x]->actual_elem_size()){
           otl_var_info_var
           (this->vl[cur_x]->get_name(),
            this->vl[cur_x]->get_ftype(),
            otl_var_long_string,
            var_info,
            sizeof(var_info));
           if(this->adb)this->adb->increment_throw_count();
           if(this->adb&&this->adb->get_throw_count()>1)return *this;
           if(otl_uncaught_exception()) return *this; 
           throw OTL_TMPL_EXCEPTION
             (otl_error_msg_5,
              otl_error_code_5,
              this->stm_label?this->stm_label:
              this->stm_text,
              var_info);
         }
         if((this->vl[cur_x]->get_var_struct().get_otl_adapter()==otl_ora7_adapter||
             this->vl[cur_x]->get_var_struct().get_otl_adapter()==otl_ora8_adapter)){
           otl_memcpy
             (c+sizeof(unsigned short),
              OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
              len,
              this->vl[cur_x]->get_ftype());
           *OTL_RCAST(unsigned short*,
                      this->vl[cur_x]->val(cur_y))=OTL_SCAST(unsigned short,len);
           this->vl[cur_x]->set_len(len,cur_y);
         }else{
           otl_memcpy(c,
                      OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
                      len,
                      this->vl[cur_x]->get_ftype());
           this->vl[cur_x]->set_len(len,cur_y);
         }
       }
       break;
#endif
     case otl_var_char:
       {
         int overflow;
         otl_strcpy
           (OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y)),
            OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
            overflow,
            this->vl[cur_x]->get_elem_size(),
            OTL_SCAST(int,s.length()));
         if(overflow){
           otl_var_info_var
             (this->vl[cur_x]->get_name(),
              this->vl[cur_x]->get_ftype(),
              otl_var_char,
              var_info,
              sizeof(var_info));
           in_exception_flag=1;
           if(this->adb)this->adb->increment_throw_count();
           if(this->adb&&this->adb->get_throw_count()>1)return *this;
           if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_4,
           otl_error_code_4,
           this->stm_label?this->stm_label:this->stm_text,
           var_info,
           OTL_RCAST(const void*,s.c_str()),
           OTL_SCAST(int,this->vl[cur_x]->get_elem_size()));
#else
           throw OTL_TMPL_EXCEPTION
             (otl_error_msg_4,
              otl_error_code_4,
              this->stm_label?this->stm_label:this->stm_text,
              var_info);
#endif
         }
         this->vl[cur_x]->set_not_null(cur_y);
       }
       break;
#if defined(OTL_USER_DEFINED_STRING_CLASS_ON) || \
     defined(OTL_STL) || defined(OTL_ACE)
     case otl_var_varchar_long:
     case otl_var_raw_long:
       {
         unsigned char* c=OTL_RCAST(unsigned char*,
                                    this->vl[cur_x]->val(cur_y));
         int len=OTL_SCAST(int,s.length());
         this->vl[cur_x]->set_not_null(cur_y);
         if(len>this->vl[cur_x]->actual_elem_size()){
           otl_var_info_var
             (this->vl[cur_x]->get_name(),
              this->vl[cur_x]->get_ftype(),
              otl_var_char,
              var_info,
              sizeof(var_info));
           if(this->adb)this->adb->increment_throw_count();
           if(this->adb&&this->adb->get_throw_count()>1)return *this;
           if(otl_uncaught_exception()) return *this; 
           throw OTL_TMPL_EXCEPTION
             (otl_error_msg_5,
              otl_error_code_5,
              this->stm_label?this->stm_label:
              this->stm_text,
              var_info);
         }
         
         otl_memcpy(c,
                    OTL_RCAST(unsigned char*,
                              OTL_CCAST(char*,s.c_str())),
                    len,
                    this->vl[cur_x]->get_ftype());
         this->vl[cur_x]->set_len(len,cur_y);
       }
       break;
     case otl_var_blob:
     case otl_var_clob:
       {
         int len=OTL_SCAST(int,s.length());
         if(len>this->vl[cur_x]->actual_elem_size()){
           otl_var_info_var
             (this->vl[cur_x]->get_name(),
              this->vl[cur_x]->get_ftype(),
              otl_var_char,
              var_info,
              sizeof(var_info));
           if(this->adb)this->adb->increment_throw_count();
           if(this->adb&&this->adb->get_throw_count()>1)return *this;
           if(otl_uncaught_exception()) return *this; 
           throw OTL_TMPL_EXCEPTION
             (otl_error_msg_5,
              otl_error_code_5,
              this->stm_label?this->stm_label:
              this->stm_text,
              var_info);
         }
         this->vl[cur_x]->set_not_null(cur_y);
         this->vl[cur_x]->get_var_struct().save_blob
           (OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),len,0);
       }
       break;
#endif
     default:
       check_type(otl_var_char,1);
     } // switch     
     check_buf();
   }
   return *this;
 }
#endif

#if defined(OTL_UNICODE_STRING_TYPE)
  OTL_TMPL_OUT_STREAM& operator<<(const OTL_UNICODE_STRING_TYPE& s)
  {
    if(this->vl_len>0){
      get_next();
      switch(this->vl[cur_x]->get_ftype()){
      case otl_var_char:
        {
          int overflow;
#if defined(OTL_C_STR_FOR_UNICODE_STRING_TYPE)
          otl_strcpy4
            (OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y)),
             OTL_RCAST(unsigned char*,
                       OTL_CCAST(OTL_UNICODE_CHAR_TYPE*,s.OTL_C_STR_FOR_UNICODE_STRING_TYPE())),
#else
          otl_strcpy4
            (OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y)),
             OTL_RCAST(unsigned char*,
                       OTL_CCAST(OTL_UNICODE_CHAR_TYPE*,s.c_str())),
#endif
             overflow,
             this->vl[cur_x]->get_elem_size(),
             OTL_SCAST(int,s.length())
             );
          if(overflow){
            otl_var_info_var
              (this->vl[cur_x]->get_name(),
               this->vl[cur_x]->get_ftype(),
               otl_var_char,
               var_info,
               sizeof(var_info));
            in_exception_flag=1;
            if(this->adb)this->adb->increment_throw_count();
            if(this->adb&&this->adb->get_throw_count()>1)return *this;
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
            throw OTL_TMPL_EXCEPTION
              (otl_error_msg_4,
               otl_error_code_4,
               this->stm_label?this->stm_label:this->stm_text,
               var_info,
               OTL_RCAST(const void*,s.c_str()),
               OTL_SCAST(int,this->vl[cur_x]->get_elem_size()));
#else
            throw OTL_TMPL_EXCEPTION
              (otl_error_msg_4,
               otl_error_code_4,
               this->stm_label?this->stm_label:this->stm_text,
               var_info);
#endif
          }
          this->vl[cur_x]->set_not_null(cur_y);
          break;
        }
      case otl_var_varchar_long:
        {
          unsigned char* c=OTL_RCAST(unsigned char*,
                                     this->vl[cur_x]->val(cur_y));
          int len=OTL_SCAST(int,s.length());
          this->vl[cur_x]->set_not_null(cur_y);
          if(len>this->vl[cur_x]->actual_elem_size()){
            otl_var_info_var
              (this->vl[cur_x]->get_name(),
               this->vl[cur_x]->get_ftype(),
               otl_var_char,
               var_info,
               sizeof(var_info));
            if(this->adb)this->adb->increment_throw_count();
            if(this->adb&&this->adb->get_throw_count()>1)return *this;
            if(otl_uncaught_exception()) return *this; 
            throw OTL_TMPL_EXCEPTION
              (otl_error_msg_5,
               otl_error_code_5,
               this->stm_label?this->stm_label:
               this->stm_text,
               var_info);
          }
#if defined(OTL_C_STR_FOR_UNICODE_STRING_TYPE)
          otl_memcpy(c,
                     OTL_RCAST(unsigned char*,
                               OTL_CCAST(OTL_UNICODE_CHAR_TYPE*,s.OTL_C_STR_FOR_UNICODE_STRING_TYPE())),
#else
          otl_memcpy(c,
                     OTL_RCAST(unsigned char*,
                               OTL_CCAST(OTL_UNICODE_CHAR_TYPE*,s.c_str())),
#endif
                     len,
                     this->vl[cur_x]->get_ftype());
          this->vl[cur_x]->set_len(len,cur_y);
          break;
        }
      case otl_var_clob:
        {
          int len=OTL_SCAST(int,s.length());
          if(len>this->vl[cur_x]->actual_elem_size()){
            otl_var_info_var
              (this->vl[cur_x]->get_name(),
               this->vl[cur_x]->get_ftype(),
               otl_var_char,
               var_info,
               sizeof(var_info));
            if(this->adb)this->adb->increment_throw_count();
            if(this->adb&&this->adb->get_throw_count()>1)return *this;
            if(otl_uncaught_exception()) return *this; 
            throw OTL_TMPL_EXCEPTION
              (otl_error_msg_5,
               otl_error_code_5,
               this->stm_label?this->stm_label:
               this->stm_text,
               var_info);
          }
          this->vl[cur_x]->set_not_null(cur_y);
          this->vl[cur_x]->get_var_struct().save_blob
#if defined(OTL_C_STR_FOR_UNICODE_STRING_TYPE)
            (OTL_RCAST(const unsigned char*,s.OTL_C_STR_FOR_UNICODE_STRING_TYPE()),
#else
            (OTL_RCAST(const unsigned char*,s.c_str()),
#endif
             len,
             0);
        }
        break;
      default:
        check_type(otl_var_char,1);
      }
      check_buf();
    }
    return *this;
  }
#endif

  OTL_TMPL_OUT_STREAM& operator<<(const char* s)
  {
    if(this->vl_len>0){
      get_next();
      if(check_type(otl_var_char,1)){

        int overflow;
        otl_strcpy
          (OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y)),
           OTL_RCAST(unsigned char*,OTL_CCAST(char*,s)),
           overflow,
           this->vl[cur_x]->get_elem_size()
           );
        if(overflow){
          otl_var_info_var
            (this->vl[cur_x]->get_name(),
             this->vl[cur_x]->get_ftype(),
             otl_var_char,
             var_info,
             sizeof(var_info));
          in_exception_flag=1;
          if(this->adb)this->adb->increment_throw_count();
          if(this->adb&&this->adb->get_throw_count()>1)return *this;
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
          throw OTL_TMPL_EXCEPTION
            (otl_error_msg_4,
             otl_error_code_4,
             this->stm_label?this->stm_label:this->stm_text,
             var_info,
             OTL_RCAST(const void*,s),
             OTL_SCAST(int,this->vl[cur_x]->get_elem_size()));
#else
          throw OTL_TMPL_EXCEPTION
            (otl_error_msg_4,
             otl_error_code_4,
             this->stm_label?this->stm_label:this->stm_text,
             var_info);
#endif
        }
        this->vl[cur_x]->set_not_null(cur_y);
      }
      check_buf();
    }
    return *this;
  }

 OTL_TMPL_OUT_STREAM& operator<<(const unsigned char* s)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_char,1)){

    int overflow;
    otl_strcpy4
     (OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y)),
      OTL_CCAST(unsigned char*,s),
      overflow,
      this->vl[cur_x]->get_elem_size()
     );
    if(overflow){
     otl_var_info_var
       (this->vl[cur_x]->get_name(),
        this->vl[cur_x]->get_ftype(),
        otl_var_char,
        var_info,
        sizeof(var_info));
     in_exception_flag=1;
     if(this->adb)this->adb->increment_throw_count();
     if(this->adb&&this->adb->get_throw_count()>1)return *this;
     if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
     throw OTL_TMPL_EXCEPTION
       (otl_error_msg_4,
        otl_error_code_4,
        this->stm_label?this->stm_label:this->stm_text,
        var_info,
        OTL_RCAST(const void*,s),
        OTL_SCAST(int,this->vl[cur_x]->get_elem_size()));
#else
     throw OTL_TMPL_EXCEPTION
      (otl_error_msg_4,
       otl_error_code_4,
       this->stm_label?this->stm_label:this->stm_text,
       var_info);
#endif
    }
    this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }
  return *this;
 }

#define OTL_D3(T,T_type)                                \
 OTL_TMPL_OUT_STREAM& operator<<(const T n)             \
 {                                                      \
  if(this->vl_len>0){                                   \
   get_next();                                          \
   if(check_type(T_type,sizeof(T))){                    \
    *OTL_RCAST(T*,this->vl[cur_x]->val(cur_y))=n;       \
    this->vl[cur_x]->set_not_null(cur_y);               \
   }                                                    \
   check_buf();                                         \
  }                                                     \
  return *this;                                         \
 }

#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  OTL_D3(int,otl_var_int)
#if defined(OTL_BIGINT)
  OTL_D3(OTL_BIGINT,otl_var_bigint)
#endif
#if defined(OTL_UBIGINT)
  OTL_D3(OTL_UBIGINT,otl_var_ubigint)
#endif
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
  OTL_D3(OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1)
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
  OTL_D3(OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2)
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
  OTL_D3(OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3)
#endif
  OTL_D3(unsigned,otl_var_unsigned_int)
  OTL_D3(long,otl_var_long_int)
  OTL_D3(short,otl_var_short)
  OTL_D3(float,otl_var_float)
  OTL_D3(double,otl_var_double)
#else
    template<OTL_TYPE_NAME T,const int T_type> OTL_D3(T,T_type)
#endif

#if defined(OTL_PL_TAB)
 OTL_TMPL_OUT_STREAM& operator<<(otl_pl_tab_generic& tab)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(tab.get_vtype(),tab.get_elem_size())){
    int i,tmp_len;
    if(tab.len()<=this->vl[cur_x]->get_array_size())
     tmp_len=tab.len();
    else
      tmp_len=this->vl[cur_x]->get_array_size();
    this->vl[cur_x]->set_pl_tab_len(tmp_len);
    if(tab.get_vtype()==otl_var_char){
     int i2;
     for(i2=0;i2<tmp_len;++i2){
       int overflow;
       otl_strcpy4
         (OTL_RCAST(unsigned char*,this->vl[cur_x]->val(i2)),
          OTL_RCAST(unsigned char*,tab.val(i2)),
          overflow,
          this->vl[cur_x]->get_elem_size());
       if(overflow){
         char tmp_var_info[256];
         otl_var_info_var
           (this->vl[cur_x]->get_name(),
            this->vl[cur_x]->get_ftype(),
            otl_var_char,
            tmp_var_info,
            sizeof(tmp_var_info));
         if(this->adb)this->adb->increment_throw_count();
         if(this->adb&&this->adb->get_throw_count()>1)return *this;
         if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
         throw OTL_TMPL_EXCEPTION
           (otl_error_msg_4,
            otl_error_code_4,
            this->stm_label?this->stm_label:this->stm_text,
            tmp_var_info,
            OTL_RCAST(const void*,tab.val(i2)),
            OTL_SCAST(int,this->vl[cur_x]->get_elem_size()));
#else
         throw OTL_TMPL_EXCEPTION
           (otl_error_msg_4,
            otl_error_code_4,
            this->stm_label?this->stm_label:this->stm_text,
            tmp_var_info);
#endif
       }
     }
    }else if(tab.get_vtype()==otl_var_timestamp){
      otl_datetime* ext_dt=OTL_RCAST(otl_datetime*,tab.get_p_v());
      otl_oracle_date* int_dt=OTL_RCAST(otl_oracle_date*,
                                        this->vl[cur_x]->val());
      int j;
      for(j=0;j<tmp_len;++j){
        convert_date(*int_dt,*ext_dt);
        ++int_dt;
        ++ext_dt;
      }
    }else
     memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
            OTL_RCAST(char*,tab.val()),
            tab.get_elem_size()*tmp_len);
    for(i=0;i<tmp_len;++i){
     if(tab.is_null(i))
      this->vl[cur_x]->set_null(i);
     else
      this->vl[cur_x]->set_not_null(i);
    }
   }
   check_buf();
  }
  return *this;
 }
#endif

#if defined(OTL_PL_TAB) && defined(OTL_STL)
 OTL_TMPL_OUT_STREAM& operator<<(otl_pl_vec_generic& vec)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(vec.get_vtype(),vec.get_elem_size())){
    int i,tmp_len;
    if(vec.len()<=this->vl[cur_x]->get_array_size())
      tmp_len=vec.len();
    else
      tmp_len=this->vl[cur_x]->get_array_size();
    this->vl[cur_x]->set_pl_tab_len(tmp_len);
    switch(vec.get_vtype()){
    case otl_var_char:
     int i2;
     for(i2=0;i2<tmp_len;++i2){
      int overflow;
      otl_strcpy
       (OTL_RCAST(unsigned char*,this->vl[cur_x]->val(i2)),
        OTL_RCAST(unsigned char*,
                  OTL_CCAST(char*,(*OTL_RCAST(STD_NAMESPACE_PREFIX 
                                              vector<OTL_STRING_CONTAINER>*,
                                              vec.get_p_v()))[i2].c_str())),
        overflow,
        this->vl[cur_x]->get_elem_size(),
        OTL_SCAST(int,(*OTL_RCAST(STD_NAMESPACE_PREFIX 
                                  vector<OTL_STRING_CONTAINER>*,
                                  vec.get_p_v()))[i2].length())
       );
      if(overflow){
       char temp_var_info[256];
       otl_var_info_var
         (this->vl[cur_x]->get_name(),
          this->vl[cur_x]->get_ftype(),
          otl_var_char,
          temp_var_info,
          sizeof(temp_var_info));
       if(this->adb)this->adb->increment_throw_count();
       if(this->adb&&this->adb->get_throw_count()>1)return *this;
       if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
       throw OTL_TMPL_EXCEPTION
         (otl_error_msg_4,
          otl_error_code_4,
          this->stm_label?this->stm_label:this->stm_text,
          temp_var_info,
          OTL_RCAST(const void*,
                    (*OTL_RCAST(STD_NAMESPACE_PREFIX 
                                vector<OTL_STRING_CONTAINER>*,
                                vec.get_p_v()))[i2].c_str()),
          OTL_SCAST(int,this->vl[cur_x]->get_elem_size()));
#else
       throw OTL_TMPL_EXCEPTION
         (otl_error_msg_4,
          otl_error_code_4,
          this->stm_label?this->stm_label:this->stm_text,
          temp_var_info);
#endif

      }
     }
     break;
    case otl_var_timestamp:
     {
      otl_oracle_date* int_dt=OTL_RCAST(otl_oracle_date*,this->vl[cur_x]->val());
      int j;
      otl_datetime* ext_dt;
      for(j=0;j<tmp_len;++j){
       ext_dt=OTL_RCAST(otl_datetime*,
                        &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<otl_datetime>*,
                                     vec.get_p_v()))[j]);
       convert_date(*int_dt,*ext_dt);
       ++int_dt;
      }
     }
     break;
    case otl_var_int:
      if(tmp_len>0)
        memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
               OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<int>*,
                                      vec.get_p_v()))[0]),
               sizeof(int)*tmp_len);
      break;
    case otl_var_double:
      if(tmp_len>0)
        memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
               OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<double>*,
                                      vec.get_p_v()))[0]),
               sizeof(double)*tmp_len);
      break;
    case otl_var_bdouble:
      if(tmp_len>0)
        memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
               OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<double>*,
                                      vec.get_p_v()))[0]),
               sizeof(double)*tmp_len);
      break;
    case otl_var_float:
      if(tmp_len>0)
        memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
               OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<float>*,
                                      vec.get_p_v()))[0]),
               sizeof(float)*tmp_len);
      break;
    case otl_var_bfloat:
      if(tmp_len>0)
        memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
               OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<float>*,
                                      vec.get_p_v()))[0]),
               sizeof(float)*tmp_len);
      break;
    case otl_var_unsigned_int:
      if(tmp_len>0)
        memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
               OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<unsigned>*,
                                      vec.get_p_v()))[0]),
               sizeof(unsigned)*tmp_len);
      break;
    case otl_var_short:
      if(tmp_len>0)
        memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
               OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<short>*,
                                      vec.get_p_v()))[0]),
                 sizeof(short)*tmp_len);
      break;
    case otl_var_long_int:
      if(tmp_len>0)
        memcpy(OTL_RCAST(char*,this->vl[cur_x]->val()),
               OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<long int>*,
                                      vec.get_p_v()))[0]),
               sizeof(long int)*tmp_len);
      break;
    }
    for(i=0;i<tmp_len;++i){
      if(vec.is_null(i))
        this->vl[cur_x]->set_null(i);
      else
        this->vl[cur_x]->set_not_null(i);
    }
   }
   check_buf();
  }
  return *this;
 }
#endif

 OTL_TMPL_OUT_STREAM& operator<<(const otl_null& /* n */)
 {
  if(this->vl_len>0){
   get_next();
   this->vl[cur_x]->set_null(cur_y);
   check_buf();
  }
  return *this;
 }

 OTL_TMPL_OUT_STREAM& operator<<(const TTimestampStruct& t)
 {
  if(this->vl_len>0){
   get_next();
   if(check_type(otl_var_timestamp,sizeof(TTimestampStruct))){
    TTimestampStruct* tm=
     OTL_RCAST(TTimestampStruct*,this->vl[cur_x]->val(cur_y));
    this->vl[cur_x]->set_not_null(cur_y);
    int rc=this->vl[cur_x]->get_var_struct().write_dt
      (tm,&t,sizeof(TTimestampStruct));
    if(rc==0){
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return *this;
      if(otl_uncaught_exception()) return *this; 
      throw OTL_TMPL_EXCEPTION
        (this->adb->get_connect_struct(),
         this->stm_label?this->stm_label:
         this->stm_text);
    }
   }
   check_buf();
  }
  return *this;
 }

 OTL_TMPL_OUT_STREAM& operator<<(const otl_long_string& s)
 {
  if(this->vl_len>0){
   get_next();

   switch(this->vl[cur_x]->get_ftype()){
   case otl_var_varchar_long:
   case otl_var_raw_long:
   case otl_var_raw:
     {
       bool in_unicode_mode=sizeof(OTL_CHAR)>1;
       if(!s.get_unicode_flag() && in_unicode_mode && 
          this->vl[cur_x]->get_ftype()==otl_var_varchar_long){
         throw OTL_TMPL_EXCEPTION
           (otl_error_msg_37,
            otl_error_code_37,
            this->stm_label?this->stm_label:
            this->stm_text);
       }else if(s.get_unicode_flag() && 
                this->vl[cur_x]->get_ftype()!=otl_var_varchar_long){
         throw OTL_TMPL_EXCEPTION
           (otl_error_msg_38,
            otl_error_code_38,
            this->stm_label?this->stm_label:
            this->stm_text);
       }
       unsigned char* c=OTL_RCAST(unsigned char*,this->vl[cur_x]->val(cur_y));
       int len=OTL_CCAST(otl_long_string*,&s)->len();
       this->vl[cur_x]->set_not_null(cur_y);
       if(len>this->vl[cur_x]->actual_elem_size()){
         otl_var_info_var
           (this->vl[cur_x]->get_name(),
            this->vl[cur_x]->get_ftype(),
            otl_var_long_string,
            var_info,
            sizeof(var_info));
         if(this->adb)this->adb->increment_throw_count();
         if(this->adb&&this->adb->get_throw_count()>1)return *this;
         if(otl_uncaught_exception()) return *this; 
         throw OTL_TMPL_EXCEPTION
           (otl_error_msg_5,
            otl_error_code_5,
            this->stm_label?this->stm_label:
            this->stm_text,
            var_info);
       }
       if((this->vl[cur_x]->get_var_struct().get_otl_adapter()==otl_ora7_adapter||
           this->vl[cur_x]->get_var_struct().get_otl_adapter()==otl_ora8_adapter) &&
          this->vl[cur_x]->get_ftype()==otl_var_raw){
         otl_memcpy
           (c+sizeof(unsigned short),
            s.v,
            len,
            this->vl[cur_x]->get_ftype());
         *OTL_RCAST(unsigned short*,
                    this->vl[cur_x]->val(cur_y))=OTL_SCAST(unsigned short,len);
         this->vl[cur_x]->set_len(len,cur_y);
       }else{
         otl_memcpy(c,s.v,len,this->vl[cur_x]->get_ftype());
         this->vl[cur_x]->set_len(len,cur_y);
       }
     }
     break;
   case otl_var_blob:
   case otl_var_clob:
     {
       bool in_unicode_mode=sizeof(OTL_CHAR)>1;
       if(!s.get_unicode_flag() && in_unicode_mode && 
          this->vl[cur_x]->get_ftype()==otl_var_clob){
         throw OTL_TMPL_EXCEPTION
           (otl_error_msg_37,
            otl_error_code_37,
            this->stm_label?this->stm_label:
            this->stm_text);
       }else if(s.get_unicode_flag() && this->vl[cur_x]->get_ftype()==otl_var_blob){
         throw OTL_TMPL_EXCEPTION
           (otl_error_msg_38,
            otl_error_code_38,
            this->stm_label?this->stm_label:
            this->stm_text);
       }
       int len=OTL_CCAST(otl_long_string*,&s)->len();
       if(len>this->vl[cur_x]->actual_elem_size()){
         otl_var_info_var
           (this->vl[cur_x]->get_name(),
            this->vl[cur_x]->get_ftype(),
            otl_var_long_string,
            var_info,
            sizeof(var_info));
         if(this->adb)this->adb->increment_throw_count();
         if(this->adb&&this->adb->get_throw_count()>1)return *this;
         if(otl_uncaught_exception()) return *this; 
         throw OTL_TMPL_EXCEPTION
           (otl_error_msg_5,
            otl_error_code_5,
            this->stm_label?this->stm_label:
            this->stm_text,
            var_info);
       }
       this->vl[cur_x]->set_not_null(cur_y);
       this->vl[cur_x]->get_var_struct().save_blob
         (s.v,len,s.get_extern_buffer_flag());
     }
     break;
   }
   check_buf();
  }
  return *this;
 }

#if defined(OTL_ORA8)||defined(OTL_ODBC)
#define OTL_TMPL_CUR_DUMMY OTL_TMPL_CURSOR

 OTL_TMPL_OUT_STREAM& operator<<
  (otl_lob_stream_generic& s)
 {
  if(this->vl_len>0){
   get_next();
   if(((s.get_ora_lob() &&
        this->vl[cur_x]->get_ftype()==otl_var_blob)||
       this->vl[cur_x]->get_ftype()==otl_var_clob)||
      (this->vl[cur_x]->get_ftype()==otl_var_varchar_long||
       this->vl[cur_x]->get_ftype()==otl_var_raw_long)){
    s.init
     (this->vl[cur_x],
      this->adb,
      OTL_RCAST(OTL_TMPL_CUR_DUMMY*,this),
      0,
      otl_lob_stream_write_mode);
    if(!s.get_ora_lob())
     this->vl[cur_x]->set_not_null(cur_y);
   }
   check_buf();
  }else{
   char temp_var_info[256];
   otl_var_info_var
     (this->vl[cur_x]->get_name(),
      this->vl[cur_x]->get_ftype(),
      otl_var_long_string,
      temp_var_info,
     sizeof(temp_var_info));
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return *this;
   if(otl_uncaught_exception()) return *this; 
   throw OTL_TMPL_EXCEPTION
    (otl_error_msg_0,
     otl_error_code_0,
     this->stm_label?this->stm_label:
     this->stm_text,
     temp_var_info);
  }
  return *this;
 }
#undef OTL_TMPL_CUR_DUMMY
#endif

 otl_tmpl_out_stream():
   OTL_TMPL_CURSOR(),
   auto_commit_flag(0),
   dirty(0),
   cur_x(0),
   cur_y(0),
   array_size(0),
   in_exception_flag(0),
   in_destruct_flag(0),
   should_delete_flag(0),
   var_info(),
   flush_flag(0),
   flush_flag2(0),
   lob_stream_mode(0),
   master_stream_ptr_(nullptr)
 {
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_tmpl_out_stream(const otl_tmpl_out_stream&) = delete;
 otl_tmpl_out_stream& operator=(const otl_tmpl_out_stream&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_tmpl_out_stream(otl_tmpl_out_stream&&) = delete;
 otl_tmpl_out_stream& operator=(otl_tmpl_out_stream&&) = delete;
#endif
private:
#else
 otl_tmpl_out_stream
 (const otl_tmpl_out_stream&):
   OTL_TMPL_CURSOR(),
   auto_commit_flag(0),
   dirty(0),
   cur_x(0),
   cur_y(0),
   array_size(0),
   in_exception_flag(0),
   in_destruct_flag(0),
   should_delete_flag(0),
   var_info(),
   flush_flag(0),
   flush_flag2(0),
   lob_stream_mode(0),
   master_stream_ptr_(nullptr)
 {
 }

 otl_tmpl_out_stream& operator=
 (const otl_tmpl_out_stream&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_tmpl_out_stream
 (otl_tmpl_out_stream&&):
   OTL_TMPL_CURSOR(),
   auto_commit_flag(0),
   dirty(0),
   cur_x(0),
   cur_y(0),
   array_size(0),
   in_exception_flag(0),
   in_destruct_flag(0),
   should_delete_flag(0),
   var_info(),
   flush_flag(0),
   flush_flag2(0),
   lob_stream_mode(0),
   master_stream_ptr_(nullptr)
 {
 }

 otl_tmpl_out_stream& operator=
 (otl_tmpl_out_stream&&)
 {
   return *this;
 }
#endif
#endif

};

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct,
          OTL_TYPE_NAME TTimestampStruct>
class otl_tmpl_inout_stream: public OTL_TMPL_OUT_STREAM{

protected:

  otl_tmpl_variable<TVariableStruct>** in_vl;
  int iv_len;
  int cur_in_x;
  int cur_in_y;
  int in_y_len;
  int null_fetched;
  otl_tmpl_variable<TVariableStruct>** avl;
  int avl_len;
  char var_info[256];

public:

  int get_iv_len() const {return iv_len;}
  otl_tmpl_variable<TVariableStruct>** get_in_vl(){return in_vl;}

 void cleanup(void)
 {int i;
  for(i=0;i<avl_len;++i){
   delete avl[i];
  }
  delete[] avl;
  delete[] in_vl;
 }

 otl_tmpl_inout_stream
 (otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  OTL_TMPL_CONNECT& pdb,
  void* master_stream_ptr,
  const bool alob_stream_mode=false,
  const char* sqlstm_label=nullptr)
   : OTL_TMPL_OUT_STREAM
     (pdb,
      master_stream_ptr,
      alob_stream_mode,
      sqlstm_label),
     in_vl(nullptr),
     iv_len(0),
     cur_in_x(0),
     cur_in_y(0),
     in_y_len(0),
     null_fetched(0),
     avl(nullptr),
     avl_len(0),
     var_info()
 {
  int i,j;
  this->dirty=0;
  this->auto_commit_flag=1;
  this->adb=&pdb;
  this->in_exception_flag=0;
  this->stm_text=nullptr;
  this->array_size=arr_size;
  this->should_delete_flag=0;

  {
   size_t len=strlen(sqlstm)+1;
   this->stm_text=new char[len];
   OTL_STRCPY_S(this->stm_text,len,sqlstm);
   otl_tmpl_ext_hv_decl
    <TVariableStruct,TTimestampStruct,TExceptionStruct,
     TConnectStruct,TCursorStruct> hvd(this->stm_text,arr_size);
   if(hvd.has_plsql_tabs_or_refcur() && arr_size>1){
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return;
      if(otl_uncaught_exception()) return; 
      throw OTL_TMPL_EXCEPTION
       (otl_error_msg_33,
        otl_error_code_33,
        this->stm_label?this->stm_label:
        this->stm_text);
   }
   if(hvd.has_space_in_bind_variable()){
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return;
      if(otl_uncaught_exception()) return; 
      throw OTL_TMPL_EXCEPTION
       (otl_error_msg_36,
        otl_error_code_36,
        this->stm_label?this->stm_label:
        this->stm_text);
   }
   if(hvd.get_vst(otl_tmpl_ext_hv_decl
                  <TVariableStruct,TTimestampStruct,TExceptionStruct,
                  TConnectStruct,TCursorStruct>::def)==hvd.get_len()){
     this->should_delete_flag=1;
     hvd.alloc_host_var_list(this->vl,this->vl_len,pdb);
   }else{
     for(i=0;i<hvd.get_len();++i){
       if(hvd.get_inout(i)==
          otl_tmpl_ext_hv_decl
          <TVariableStruct,
          TTimestampStruct,
          TExceptionStruct,
          TConnectStruct,
          TCursorStruct>::in)
      ++this->vl_len;
       else if(hvd.get_inout(i)==
               otl_tmpl_ext_hv_decl
               <TVariableStruct,
               TTimestampStruct,
               TExceptionStruct,
               TConnectStruct,
               TCursorStruct>::out)
         ++iv_len;
       else if(hvd.get_inout(i)==
               otl_tmpl_ext_hv_decl
               <TVariableStruct,
               TTimestampStruct,
               TExceptionStruct,
               TConnectStruct,
               TCursorStruct>::io){
         ++this->vl_len;
         ++iv_len;
       }
     }
     if(this->vl_len>0){
       this->vl=new otl_tmpl_variable<TVariableStruct>*[this->vl_len];
     }
     if(iv_len>0)
       in_vl=new otl_tmpl_variable<TVariableStruct>*[iv_len];
     avl_len=hvd.get_len();
     if(hvd.get_len()>0)
       avl=new otl_tmpl_variable<TVariableStruct>*[avl_len];
    iv_len=0; 
    this->vl_len=0; 
    for(j=0;j<avl_len;++j){
#if defined(OTL_STREAM_LEGACY_BUFFER_SIZE_TYPE)
     if(hvd.pl_tab_size[j]>32767){
      char tmp_var_info[256];
      otl_var_info_var
       (hvd[j],
        otl_var_none,
        otl_var_none,
        tmp_var_info,
        sizeof(tmp_var_info));
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return;
      if(otl_uncaught_exception()) return; 
      throw OTL_TMPL_EXCEPTION
       (otl_error_msg_6,
        otl_error_code_6,
        this->stm_label?this->stm_label:
        this->stm_text,
        tmp_var_info);
      }
#endif
     otl_tmpl_variable<TVariableStruct>* v=hvd.alloc_var
      (hvd[j],
       hvd.get_inout(j),
       otl_tmpl_ext_hv_decl
        <TVariableStruct,TTimestampStruct,TExceptionStruct,
         TConnectStruct,TCursorStruct>::def,
       pdb,
       hvd.get_pl_tab_size(j));
     if(v==nullptr){
       int k;
       if(avl!=nullptr)
         for(k=0;k<j;++k){
           delete avl[k];
           avl[k]=nullptr;
         }
       delete[] avl;
       avl=nullptr;
       this->vl_len=0;
       throw OTL_TMPL_EXCEPTION
         (otl_error_msg_12,
          otl_error_code_12,
          hvd.stm_label()?hvd.stm_label():hvd.stm_text(),
          hvd[j]);
     }
     if(v!=nullptr)v->set_name_pos(j+1);
     if(avl!=nullptr)avl[j]=v;
     if(hvd.get_inout(j)==
        otl_tmpl_ext_hv_decl
        <TVariableStruct,
        TTimestampStruct,
        TExceptionStruct,
        TConnectStruct,
        TCursorStruct>::in){
       ++this->vl_len;
       this->vl[this->vl_len-1]=v;
       v->set_param_type(otl_input_param);
     }else if(hvd.get_inout(j)==
              otl_tmpl_ext_hv_decl
              <TVariableStruct,
              TTimestampStruct,
              TExceptionStruct,
              TConnectStruct,
              TCursorStruct>::out){
       ++iv_len;
       in_vl[iv_len-1]=v;
       v->set_param_type(otl_output_param);
     }else if(hvd.get_inout(j)==
              otl_tmpl_ext_hv_decl
              <TVariableStruct,
              TTimestampStruct,
              TExceptionStruct,
              TConnectStruct,
              TCursorStruct>::io){
       ++this->vl_len;
       ++iv_len;
       this->vl[this->vl_len-1]=v;
       if(in_vl!=nullptr)in_vl[iv_len-1]=v;
       v->set_param_type(otl_inout_param);
     }
    }
   }
  }
  try{
   this->parse();
   for(i=0;i<this->vl_len;++i){
     if(this->vl[i]->get_var_struct().get_otl_adapter()==otl_odbc_adapter){
       this->vl[i]->get_var_struct().set_lob_stream_mode(this->lob_stream_mode);
       this->vl[i]->get_var_struct().set_vparam_type(this->vl[i]->get_param_type());
       if(this->vl[i]->get_ftype()==otl_var_varchar_long||
          this->vl[i]->get_ftype()==otl_var_raw_long){
         this->vl[i]->set_not_null(0);
       }
    }
    this->bind(*(this->vl[i]));
   }
   for(j=0;j<iv_len;++j)
    this->bind(*in_vl[j]);
   rewind();
  }catch(OTL_CONST_EXCEPTION OTL_TMPL_EXCEPTION&){
   cleanup();
   if(this->adb)this->adb->increment_throw_count();
   throw;
  }

 }

 virtual ~otl_tmpl_inout_stream()
 {this->in_destructor=1;
  if(!this->in_exception_flag)
   flush();
  cleanup();
 }

 int eof(void)
 {
  if(iv_len==0)return 1;
  if(in_y_len==0)return 1;
  if(cur_in_y<=in_y_len-1)return 0;
  return 1;
 }

 void flush(const int rowoff=0, const bool force_flush=false)
 {
  if(this->vl_len==0)return;
  in_y_len=this->cur_y+1;
  cur_in_y=0;
  cur_in_x=0;
  if(!this->in_exception_flag)
   OTL_TMPL_OUT_STREAM::flush(rowoff,force_flush);
 }

  void clean(const int clean_up_error_flag=0)
 {
  if(this->vl_len!=0) {
    in_y_len=this->cur_y+1;
    cur_in_y=0;
    cur_in_x=0;
  }
  OTL_TMPL_OUT_STREAM::clean(clean_up_error_flag);
 }

 void rewind(void)
 {
  flush();
  cur_in_x=0;
  cur_in_y=0;
  this->cur_x=-1;
  this->cur_y=0;
  in_y_len=0;
  null_fetched=0;
  if(this->vl_len==0){
    this->exec(this->array_size,0,otl_sql_exec_from_cursor_class);
   in_y_len=this->array_size;
   cur_in_y=0;
   cur_in_x=0;
  }
 }

 int is_null(void)
 {
  return null_fetched;
 }

  void skip_to_end_of_row()
  {
    if(eof())return;
    if(iv_len==0)return;
    if(in_y_len==0)return;
    if(cur_in_y<in_y_len-1){
      ++cur_in_y;
      cur_in_x=0;
    }else{
      cur_in_y=0;
      cur_in_x=0;
      in_y_len=0;
    }
  }

 void get_in_next(void)
 {
  if(iv_len==0)return;
  if(in_y_len==0)return;
  if(cur_in_x<iv_len-1)
   ++cur_in_x;
  else{
   if(cur_in_y<in_y_len-1){
    ++cur_in_y;
    cur_in_x=0;
   }else{
    cur_in_y=0;
    cur_in_x=0;
    in_y_len=0;
   }
  }
 }

  int check_in_type_throw(int type_code)
  {
    this->in_exception_flag=1;
    otl_var_info_var
      (in_vl[cur_in_x]->get_name(),
       in_vl[cur_in_x]->get_ftype(),
       type_code,
       var_info,
       sizeof(var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return 0;
    if(otl_uncaught_exception()) return 0; 
    throw OTL_TMPL_EXCEPTION
      (otl_error_msg_0,
       otl_error_code_0,
       this->stm_label?this->stm_label:
       this->stm_text,
       var_info);
  }

  int check_in_type(int type_code,int tsize)
  {
    switch(in_vl[cur_in_x]->get_ftype()){
    case otl_var_refcur:
      if(type_code==otl_var_refcur)
        return 1;
    case otl_var_db2time:
    case otl_var_db2date:
      if(type_code==otl_var_timestamp)
        return 1;
    case otl_var_char:
      if(type_code==otl_var_char)
        return 1;
    case otl_var_bdouble:
      if(type_code==otl_var_double)
        return 1;
    case otl_var_bfloat:
      if(type_code==otl_var_float)
        return 1;
    default:
      if(in_vl[cur_in_x]->get_ftype()==type_code &&
         in_vl[cur_in_x]->get_elem_size()==tsize)
        return 1;
    }
    return check_in_type_throw(type_code);
 }

 int is_null_intern(void)
 {
  if(iv_len==0)return 0;
  if(in_y_len==0)return 0;
  if(in_y_len>0)
   return in_vl[cur_in_x]->is_null(cur_in_y);
  return 0;
 }


 OTL_TMPL_INOUT_STREAM& operator>>(char& c)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_char,1)){
   c=*OTL_RCAST(char*,in_vl[cur_in_x]->val(cur_in_y));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

 OTL_TMPL_INOUT_STREAM& operator>>(unsigned char& c)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_char,1)){
   c=*OTL_RCAST(unsigned char*,in_vl[cur_in_x]->val(cur_in_y));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 OTL_TMPL_INOUT_STREAM& operator>>(OTL_STRING_CONTAINER& s)
 {
  if(eof())return *this;
  switch(in_vl[cur_in_x]->get_ftype()){
  case otl_var_char:
    {
#if defined(OTL_ACE)
      s.set(OTL_RCAST(char*,in_vl[cur_in_x]->val(cur_in_y)),1);
#else
      s=OTL_RCAST(char*,in_vl[cur_in_x]->val(cur_in_y));
#endif
      null_fetched=is_null_intern();
    }
    break;
#if defined(USER_DEFINED_STRING_CLASS) || \
    defined(OTL_STL) || defined(OTL_ACE)
  case otl_var_varchar_long:
  case otl_var_raw_long:
    {
      unsigned char* c=
        OTL_RCAST(unsigned char*,in_vl[cur_in_x]->val(cur_in_y));
      int len=in_vl[cur_in_x]->get_len();
#if (defined(USER_DEFINED_STRING_CLASS) || defined(OTL_STL)) && !defined(OTL_ACE)
      s.assign(OTL_RCAST(char*,c),len);
#elif defined(OTL_ACE)
      s.set(OTL_RCAST(char*,c),len,1);
#endif
      null_fetched=is_null_intern();
    }
    break;
  case otl_var_blob:
  case otl_var_clob:
    {
      int len=0;
      int max_long_sz=this->adb->get_max_long_size();
      otl_auto_array_ptr<unsigned char> loc_ptr(max_long_sz);
      unsigned char* temp_buf=loc_ptr.get_ptr();
      int rc=in_vl[cur_in_x]->get_var_struct().get_blob
        (cur_in_y,
         temp_buf,
         max_long_sz,
         len);
      if(rc==0){
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
        throw OTL_TMPL_EXCEPTION
          (this->adb->get_connect_struct(),
           this->stm_label?this->stm_label:
           this->stm_text);
      }
#if (defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)) && !defined(OTL_ACE)
      s.assign(OTL_RCAST(char*,temp_buf),len);
#elif defined(OTL_ACE)
      s.set(OTL_RCAST(char*,temp_buf),len,1);
#endif

      null_fetched=is_null_intern();
    }
    break;
#endif
  default:
    check_in_type(otl_var_char,1);
  } // switch
  get_in_next();
  return *this;
 }
#endif

 OTL_TMPL_INOUT_STREAM& operator>>(char* s)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_char,1)){
   otl_strcpy(OTL_RCAST(unsigned char*,s),
              OTL_RCAST(const unsigned char*,in_vl[cur_in_x]->val(cur_in_y)));
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

#if defined(OTL_UNICODE_STRING_TYPE)
  OTL_TMPL_INOUT_STREAM& operator>>(OTL_UNICODE_STRING_TYPE& s)
  {
    if(eof())return *this;
    if(check_in_type(otl_var_char,1)){
#if defined(OTL_ODBC) || defined(DB2_CLI)
      s=OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,in_vl[cur_in_x]->val(cur_in_y));
#else
#if defined(OTL_UNICODE_STRING_TYPE_CAST_FROM_CHAR)
      OTL_UNICODE_CHAR_TYPE* temp_s=OTL_RCAST
        (OTL_UNICODE_CHAR_TYPE*,in_vl[cur_in_x]->val(cur_in_y));
      OTL_UNICODE_STRING_TYPE_CAST_FROM_CHAR(s,temp_s+1,*temp_s);
#else
      OTL_UNICODE_CHAR_TYPE* temp_s=OTL_RCAST
        (OTL_UNICODE_CHAR_TYPE*,in_vl[cur_in_x]->val(cur_in_y));
      s.assign(temp_s+1,*temp_s);
#endif
#endif


      null_fetched=is_null_intern();
    }
    get_in_next();
    return *this;
  }
#endif

 OTL_TMPL_INOUT_STREAM& operator>>(unsigned char* s)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_char,1)){
   otl_strcpy2(OTL_RCAST(unsigned char*,s),
               OTL_RCAST(unsigned char*,in_vl[cur_in_x]->val(cur_in_y)),
               in_vl[cur_in_x]->get_len(cur_in_y)
              );
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

#define OTL_D4(T,T_type)                                \
 OTL_TMPL_INOUT_STREAM& operator>>(T& n)                \
 {                                                      \
  if(eof())return *this;                                \
  if(check_in_type(T_type,sizeof(T))){                  \
   n=*OTL_RCAST(T*,in_vl[cur_in_x]->val(cur_in_y));     \
   null_fetched=is_null_intern();                       \
  }                                                     \
  get_in_next();                                        \
  return *this;                                         \
 }

#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  OTL_D4(int,otl_var_int)
  OTL_D4(unsigned,otl_var_unsigned_int)
#if defined(OTL_BIGINT)
  OTL_D4(OTL_BIGINT,otl_var_bigint)
#endif
#if defined(OTL_UBIGINT)
  OTL_D4(OTL_UBIGINT,otl_var_ubigint)
#endif
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
  OTL_D4(OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1)
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
  OTL_D4(OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2)
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
  OTL_D4(OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3)
#endif
  OTL_D4(long,otl_var_long_int)
  OTL_D4(short,otl_var_short)
  OTL_D4(float,otl_var_float)
  OTL_D4(double,otl_var_double)
#else
    template<OTL_TYPE_NAME T,const int T_type> OTL_D4(T,T_type)
#endif

#if defined(OTL_PL_TAB)
 OTL_TMPL_INOUT_STREAM& operator>>(otl_pl_tab_generic& tab)
 {
  if(eof())return *this;
  if(check_in_type(tab.get_vtype(),tab.get_elem_size())){
    int i,tmp_len;
    tmp_len=in_vl[cur_in_x]->get_pl_tab_len();
    if(tab.get_tab_size()<tmp_len)
      tmp_len=tab.get_tab_size();
    tab.set_len(tmp_len);
    if(tab.get_vtype()==otl_var_char){
     for(i=0;i<tmp_len;++i){
      int overflow;
      otl_strcpy3
       (OTL_RCAST(unsigned char*,tab.val(i)),
        OTL_RCAST(unsigned char*,in_vl[cur_in_x]->val(i)),
        in_vl[cur_in_x]->get_len(i),
        overflow,
        tab.get_elem_size()
       );
     }
    }else if(tab.get_vtype()==otl_var_timestamp){
      otl_datetime* ext_dt=OTL_RCAST(otl_datetime*,tab.get_p_v());
      otl_oracle_date* int_dt=
        OTL_RCAST(otl_oracle_date*,in_vl[cur_in_x]->val());
      int j;
      for(j=0;j<tmp_len;++j){
        convert_date(*ext_dt,*int_dt);
        ++int_dt;
        ++ext_dt;
      }
    }else
      memcpy(OTL_RCAST(char*,tab.val()),
             OTL_RCAST(char*,in_vl[cur_in_x]->val()),
             tab.get_elem_size()*tmp_len);
    for(i=0;i<tmp_len;++i){
     if(in_vl[cur_in_x]->is_null(i))
      tab.set_null(i);
     else
      tab.set_non_null(i);
    }
   null_fetched=0;
  }
  get_in_next();
  return *this;
 }
#endif

#if defined(OTL_PL_TAB) && defined(OTL_STL)
 OTL_TMPL_INOUT_STREAM& operator>>(otl_pl_vec_generic& vec)
 {
  if(eof())return *this;
  if(check_in_type(vec.get_vtype(),vec.get_elem_size())){
    int i,tmp_len;
    tmp_len=in_vl[cur_in_x]->get_pl_tab_len();
    vec.set_len(tmp_len);
    if(tmp_len>0){
      switch(vec.get_vtype()){
      case otl_var_char:
        for(i=0;i<tmp_len;++i){
          (*OTL_RCAST(STD_NAMESPACE_PREFIX vector<OTL_STRING_CONTAINER>*,vec.get_p_v()))[i]=
            OTL_RCAST(char*,in_vl[cur_in_x]->val(i));
        }
        break;
      case otl_var_timestamp:
        {
          otl_datetime* ext_dt;
          otl_oracle_date* int_dt=
            OTL_RCAST(otl_oracle_date*,in_vl[cur_in_x]->val());
          int j;
          for(j=0;j<tmp_len;++j){
            ext_dt=OTL_RCAST
              (otl_datetime*,
               &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<otl_datetime>*,vec.get_p_v()))[j]);
            convert_date(*ext_dt,*int_dt);
            ++int_dt;
          }
        }
        break;
      case otl_var_int:
        memcpy(OTL_RCAST
               (char*,
                &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<int>*,vec.get_p_v()))[0]),
               OTL_RCAST(char*,in_vl[cur_in_x]->val()),
               sizeof(int)*tmp_len);
        break;
      case otl_var_double:
        memcpy(OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<double>*,
                                      vec.get_p_v()))[0]),
               OTL_RCAST(char*,in_vl[cur_in_x]->val()),
               sizeof(double)*tmp_len);
        break;
      case otl_var_bdouble:
        memcpy(OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<double>*,
                                      vec.get_p_v()))[0]),
               OTL_RCAST(char*,in_vl[cur_in_x]->val()),
               sizeof(double)*tmp_len);
        break;
      case otl_var_float:
        memcpy(OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<float>*,
                                      vec.get_p_v()))[0]),
               OTL_RCAST(char*,in_vl[cur_in_x]->val()),
               sizeof(float)*tmp_len);
        break;
      case otl_var_bfloat:
        memcpy(OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<float>*,
                                      vec.get_p_v()))[0]),
               OTL_RCAST(char*,in_vl[cur_in_x]->val()),
               sizeof(float)*tmp_len);
        break;
      case otl_var_unsigned_int:
        memcpy(OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<unsigned>*,
                                      vec.get_p_v()))[0]),
               OTL_RCAST(char*,in_vl[cur_in_x]->val()),
               sizeof(unsigned)*tmp_len);
        break;
      case otl_var_short:
        memcpy(OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<short>*,
                                      vec.get_p_v()))[0]),
               OTL_RCAST(char*,in_vl[cur_in_x]->val()),
               sizeof(short)*tmp_len);
        break;
      case otl_var_long_int:
        memcpy(OTL_RCAST(char*,
                         &(*OTL_RCAST(STD_NAMESPACE_PREFIX vector<long int>*,
                                      vec.get_p_v()))[0]),
               OTL_RCAST(char*,in_vl[cur_in_x]->val()),
               sizeof(long int)*tmp_len);
        break;
      }
      
    }
    for(i=0;i<tmp_len;++i){
     if(in_vl[cur_in_x]->is_null(i))
      vec.set_null(i);
     else
      vec.set_non_null(i);
    }
   null_fetched=0;
  }
  get_in_next();
  return *this;
 }
#endif


 OTL_TMPL_INOUT_STREAM& operator>>(TTimestampStruct& t)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_timestamp,sizeof(TTimestampStruct))){
   TTimestampStruct* tm=
    OTL_RCAST(TTimestampStruct*,in_vl[cur_in_x]->val(cur_in_y));
   int rc=in_vl[cur_in_x]->get_var_struct().read_dt
     (&t,tm,sizeof(TTimestampStruct));
   if(rc==0){
     if(this->adb)this->adb->increment_throw_count();
     if(this->adb&&this->adb->get_throw_count()>1)return *this;
     if(otl_uncaught_exception()) return *this; 
     throw OTL_TMPL_EXCEPTION
       (this->adb->get_connect_struct(),
        this->stm_label?this->stm_label:
        this->stm_text);
   }
   null_fetched=is_null_intern();
  }
  get_in_next();
  return *this;
 }

 OTL_TMPL_INOUT_STREAM& operator>>(otl_long_string& s)
 {
  int len=0;
  if(eof())return *this;
  switch(in_vl[cur_in_x]->get_ftype()){
  case otl_var_raw:
  case otl_var_varchar_long:
  case otl_var_raw_long:
    {
      bool in_unicode_mode=sizeof(OTL_CHAR)>1;
      if(!s.get_unicode_flag() && in_unicode_mode &&
         in_vl[cur_in_x]->get_ftype()==otl_var_varchar_long){
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_37,
           otl_error_code_37,
           this->stm_label?this->stm_label:
           this->stm_text);
      }else if(s.get_unicode_flag() &&
               in_vl[cur_in_x]->get_ftype()!=otl_var_varchar_long){
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_38,
           otl_error_code_38,
           this->stm_label?this->stm_label:
           this->stm_text);
      }
      unsigned char* c=OTL_RCAST(unsigned char*,in_vl[cur_in_x]->val(cur_in_y));
      len=in_vl[cur_in_x]->get_len();
      if(len>s.get_buf_size())
        len=s.get_buf_size();
      otl_memcpy(s.v,c,len,in_vl[cur_in_x]->get_ftype());
      s.set_len(len);
      if(in_vl[cur_in_x]->get_ftype()==otl_var_varchar_long)
        s.null_terminate_string(len);
      null_fetched=is_null_intern();
    }
    break;
  case otl_var_clob:
  case otl_var_blob:
    {
      bool in_unicode_mode=sizeof(OTL_CHAR)>1;
      if(!s.get_unicode_flag() && in_unicode_mode &&
         in_vl[cur_in_x]->get_ftype()==otl_var_clob){
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_37,
           otl_error_code_37,
           this->stm_label?this->stm_label:
           this->stm_text);
      }else if(s.get_unicode_flag() && in_vl[cur_in_x]->get_ftype()==otl_var_blob){
        throw OTL_TMPL_EXCEPTION
          (otl_error_msg_38,
           otl_error_code_38,
           this->stm_label?this->stm_label:
           this->stm_text);
      }
      int rc=in_vl[cur_in_x]->get_var_struct().get_blob
        (cur_in_y,s.v,s.get_buf_size(),len);
      if(rc==0){
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
        throw OTL_TMPL_EXCEPTION
          (this->adb->get_connect_struct(),
           this->stm_label?this->stm_label:
           this->stm_text);
      }
      if(len>s.get_buf_size())
        len=s.get_buf_size();
      s.set_len(len);
      if(in_vl[cur_in_x]->get_ftype()==otl_var_clob)
        s.null_terminate_string(len);
      null_fetched=is_null_intern();
    }
    break;
  default:
    {
      char temp_var_info[256];
      otl_var_info_var
        (in_vl[cur_in_x]->get_name(),
         in_vl[cur_in_x]->get_ftype(),
         otl_var_long_string,
         temp_var_info,
         sizeof(temp_var_info));
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return *this;
      if(otl_uncaught_exception()) return *this; 
      throw OTL_TMPL_EXCEPTION
        (otl_error_msg_0,
         otl_error_code_0,
         this->stm_label?this->stm_label:
         this->stm_text,
         temp_var_info);
    }
  }
  get_in_next();
  return *this;
 }

#if defined(OTL_ORA8)||defined(OTL_ODBC)
 OTL_TMPL_INOUT_STREAM& operator>>
  (otl_lob_stream_generic& s)
 {
  if(eof())return *this;
  if((s.get_ora_lob() &&
      in_vl[cur_in_x]->get_ftype()==otl_var_clob)||
     in_vl[cur_in_x]->get_ftype()==otl_var_blob){
    null_fetched=is_null_intern();
    s.init
      (OTL_RCAST(void*,in_vl[cur_in_x]),
       OTL_RCAST(void*,this->adb),
       OTL_RCAST(void*,this),
       0,
       otl_lob_stream_read_mode,
       this->is_null());
  }else if(in_vl[cur_in_x]->get_ftype()==otl_var_varchar_long||
           in_vl[cur_in_x]->get_ftype()==otl_var_raw_long){
   s.init
    (OTL_RCAST(void*,in_vl[cur_in_x]),
     OTL_RCAST(void*,this->adb),
     OTL_RCAST(void*,this),
     0,
     otl_lob_stream_read_mode);
  }else{
   char tmp_var_info[256];
   otl_var_info_var
     (in_vl[cur_in_x]->get_name(),
      in_vl[cur_in_x]->get_ftype(),
     otl_var_long_string,
     tmp_var_info,
     sizeof(tmp_var_info));
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return *this;
   if(otl_uncaught_exception()) return *this; 
   throw OTL_TMPL_EXCEPTION
    (otl_error_msg_0,
     otl_error_code_0,
     this->stm_label?this->stm_label:
     this->stm_text,
     tmp_var_info);
  }
  get_in_next();
  return *this;
 }
#endif

 otl_tmpl_inout_stream(): 
   OTL_TMPL_OUT_STREAM(),
   in_vl(nullptr),
   iv_len(0),
   cur_in_x(0),
   cur_in_y(0),
   in_y_len(0),
   null_fetched(0),
   avl(nullptr),
   avl_len(0),
   var_info()
 {
 }


private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_tmpl_inout_stream(const otl_tmpl_inout_stream&) = delete;
 otl_tmpl_inout_stream& operator=(const otl_tmpl_inout_stream&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_tmpl_inout_stream(otl_tmpl_inout_stream&&) = delete;
 otl_tmpl_inout_stream& operator=(otl_tmpl_inout_stream&&) = delete;
#endif
private:
#else
 otl_tmpl_inout_stream
 (const otl_tmpl_inout_stream&): 
   OTL_TMPL_OUT_STREAM(),
   in_vl(nullptr),
   iv_len(0),
   cur_in_x(0),
   cur_in_y(0),
   in_y_len(0),
   null_fetched(0),
   avl(nullptr),
   avl_len(0),
   var_info()
 {
 }

 otl_tmpl_inout_stream& operator=(const otl_tmpl_inout_stream&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_tmpl_inout_stream
 (otl_tmpl_inout_stream&&): 
   OTL_TMPL_OUT_STREAM(),
   in_vl(nullptr),
   iv_len(0),
   cur_in_x(0),
   cur_in_y(0),
   in_y_len(0),
   null_fetched(0),
   avl(nullptr),
   avl_len(0),
   var_info()
 {
 }

 otl_tmpl_inout_stream& operator=(otl_tmpl_inout_stream&&)
 {
   return *this;
 }
#endif
#endif

};

// ==================== OTL-Adapter for ODBC/CLI =========================

#if defined(OTL_ODBC)

#if defined(OTL_ODBC_USES_SQL_FETCH_SCROLL_WHEN_SPECIFIED_IN_OTL_CONNECT) && \
  !defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON)
#error OTL_ODBC_USES_SQL_FETCH_SCROLL_WHEN_SPECIFIED_IN_OTL_CONNECT can only be used when \
OTL_ODBC_SQL_EXTENDED_FETCH_ON is defined
#endif

#if defined(OTL_ANSI_CPP_11_NULLPTR_SUPPORT)
#if !defined(OTL_DB2_CLI)
#define OTL_SQL_NULL_HANDLE SQL_NULL_HANDLE
#define OTL_SQL_NULL_HANDLE_VAL nullptr
#else
#define OTL_SQL_NULL_HANDLE SQL_NULL_HANDLE
#define OTL_SQL_NULL_HANDLE_VAL 0
#endif
#else
#define OTL_SQL_NULL_HANDLE SQL_NULL_HANDLE
#define OTL_SQL_NULL_HANDLE_VAL 0
#endif

#if !defined(OTL_DB2_CLI) && !defined(OTL_ODBC_zOS)

// in case if it's ODBC for Windows (!OTL_ODBC_UNIX), and windows.h is
// not included yet (_WINDOWS_ not defined yet), then include the file
// explicitly
#if !defined(OTL_ODBC_UNIX) && !defined(_WINDOWS_)
#include <windows.h>
#endif

#if defined(OTL_ODBC_UNIX) && defined(OTL_INFORMIX_CLI)
#include <infxsql.h>

#define OTL_HENV SQLHANDLE
#define OTL_HDBC SQLHANDLE
#define OTL_SQLHANDLE SQLHANDLE
#define OTL_SQLRETURN SQLRETURN
#define OTL_SQLSMALLINT SQLSMALLINT
#define OTL_SQLCHAR_PTR SQLCHAR*
#define OTL_SQLINTEGER_PTR SQLINTEGER*
#define OTL_SQLSMALLINT_PTR SQLSMALLINT*
#define OTL_SQLINTEGER SQLINTEGER
#define OTL_SQLHSTMT SQLHSTMT
#define OTL_SQLUSMALLINT SQLUSMALLINT
#define OTL_SQLPOINTER SQLPOINTER
#define OTL_SQLCHAR SQLCHAR
#define OTL_SQLUINTEGER SQLUINTEGER
#define OTL_SQLLEN SQLINTEGER
#define OTL_SQLLEN_PTR SQLINTEGER*
#define OTL_SQLULEN SQLUINTEGER
#define OTL_SQLULEN_PTR SQLUINTEGER*

#else
#include <sql.h>
#include <sqlext.h>
#endif

#else
#include <sqlcli1.h>
#endif

#if defined(OTL_ODBC) && !defined(OTL_DB2_CLI)
#define OTL_SQL_XML (-152)
#endif

#if defined(OTL_ODBC) && defined(OTL_DB2_CLI)
#if defined(SQL_XML)
#define OTL_SQL_XML SQL_XML
#else
#define OTL_SQL_XML (-370)
#endif
#endif

#if defined(OTL_ODBC)

#if (ODBCVER >= 0x0300)
#define OTL_SQL_TIMESTAMP_STRUCT SQL_TIMESTAMP_STRUCT
#define OTL_SQL_TIME_STRUCT SQL_TIME_STRUCT
#define OTL_SQL_DATE_STRUCT SQL_DATE_STRUCT
#else
#define OTL_SQL_TIMESTAMP_STRUCT TIMESTAMP_STRUCT
#define OTL_SQL_TIME_STRUCT TIME_STRUCT
#define OTL_SQL_DATE_STRUCT DATE_STRUCT
#endif

#if defined(OTL_DB2_CLI)

#define OTL_HENV SQLHANDLE
#define OTL_HDBC SQLHANDLE
#define OTL_SQLHANDLE SQLHANDLE
#define OTL_SQLRETURN SQLRETURN
#define OTL_SQLSMALLINT SQLSMALLINT
#define OTL_SQLCHAR_PTR SQLCHAR*
#define OTL_SQLINTEGER_PTR SQLINTEGER*
#define OTL_SQLSMALLINT_PTR SQLSMALLINT*
#define OTL_SQLINTEGER SQLINTEGER
#define OTL_SQLHSTMT SQLHSTMT
#define OTL_SQLUSMALLINT SQLUSMALLINT
#define OTL_SQLPOINTER SQLPOINTER
#define OTL_SQLCHAR SQLCHAR
#define OTL_SQLUINTEGER SQLUINTEGER

#if defined(OTL_IODBC_BSD)||defined(_WIN64)

#define OTL_SQLLEN SQLLEN
#define OTL_SQLULEN SQLULEN
#define OTL_SQLULEN_PTR SQLULEN*
#define OTL_SQLLEN_PTR SQLLEN*

#else // #if defined(OTL_IODBC_BSD)

#define OTL_SQLLEN SQLINTEGER
#define OTL_SQLLEN_PTR SQLINTEGER*
#define OTL_SQLULEN SQLUINTEGER
#define OTL_SQLULEN_PTR SQLUINTEGER*

#endif // #if defined(OTL_IODBC_BSD)

#else // #if defined(OTL_DB2_CLI)

#if (ODBCVER >= 0x0300)

#define OTL_HENV SQLHANDLE
#define OTL_HDBC SQLHANDLE
#define OTL_SQLHANDLE SQLHANDLE
#define OTL_SQLRETURN SQLRETURN
#define OTL_SQLSMALLINT SQLSMALLINT
#define OTL_SQLCHAR_PTR SQLCHAR*
#define OTL_SQLINTEGER_PTR SQLINTEGER*
#define OTL_SQLSMALLINT_PTR SQLSMALLINT*
#define OTL_SQLINTEGER SQLINTEGER
#define OTL_SQLHSTMT SQLHSTMT
#define OTL_SQLUSMALLINT SQLUSMALLINT
#define OTL_SQLPOINTER SQLPOINTER
#define OTL_SQLCHAR SQLCHAR
#define OTL_SQLUINTEGER SQLUINTEGER

#if defined(OTL_IODBC_BSD)

#define OTL_SQLLEN SQLLEN
#define OTL_SQLLEN_PTR SQLLEN*
#define OTL_SQLULEN SQLULEN
#define OTL_SQLULEN_PTR SQLULEN*

#else // #if defined(OTL_IODBC_BSD)

#if (defined(_MSC_VER)&&(_MSC_VER==1200)||defined(__MVS__)) // VC 6++ or C++ in MVS
#define OTL_SQLLEN SQLINTEGER
#define OTL_SQLLEN_PTR SQLINTEGER*
#define OTL_SQLULEN SQLUINTEGER
#define OTL_SQLULEN_PTR SQLUINTEGER*
#else

#if !defined(OTL_SQLLEN)
#define OTL_SQLLEN SQLLEN
#define OTL_SQLLEN_PTR SQLLEN*
#define OTL_SQLULEN SQLULEN
#define OTL_SQLULEN_PTR SQLULEN*
#endif

#endif

#endif // #if defined(OTL_IODBC_BSD)

#else // #if (ODBCVER >= 0x0300)

#define OTL_HENV HENV
#define OTL_HDBC HDBC
#define OTL_SQLHANDLE HSTMT
#define OTL_SQLRETURN SQLRETURN
#define OTL_SQLSMALLINT SQLSMALLINT
#define OTL_SQLCHAR_PTR SQLCHAR*
#define OTL_SQLINTEGER_PTR SQLINTEGER*
#define OTL_SQLSMALLINT_PTR SQLSMALLINT*
#define OTL_SQLINTEGER SQLINTEGER
#define OTL_SQLHSTMT SQLHSTMT
#define OTL_SQLUSMALLINT SQLUSMALLINT
#define OTL_SQLPOINTER SQLPOINTER
#define OTL_SQLCHAR SQLCHAR
#define OTL_SQLUINTEGER SQLUINTEGER

#if defined(OTL_IODBC_BSD)

#define OTL_SQLLEN SQLLEN
#define OTL_SQLLEN_PTR SQLLEN*
#define OTL_SQLULEN SQLULEN
#define OTL_SQLULEN_PTR SQLULEN*

#else // #if defined(OTL_IODBC_BSD)

#if (defined(_MSC_VER)&&(_MSC_VER==1200)) // VC 6++
#define OTL_SQLLEN SQLINTEGER
#define OTL_SQLLEN_PTR SQLINTEGER*
#define OTL_SQLULEN SQLUINTEGER
#define OTL_SQLULEN_PTR SQLUINTEGER*
#else
#define OTL_SQLLEN SQLLEN
#define OTL_SQLLEN_PTR SQLLEN*
#define OTL_SQLULEN SQLULEN
#define OTL_SQLULEN_PTR SQLULEN*
#endif

#endif // #if defined(OTL_IODBC_BSD)

#endif

#endif

#endif

OTL_ODBC_NAMESPACE_BEGIN

#if (defined(UNICODE) || defined(_UNICODE)) && defined(OTL_ODBC)

inline void otl_convert_char_to_SQLWCHAR(SQLWCHAR* dst, const unsigned char* src)
{
  while(*src)
    *dst++=OTL_SCAST(SQLWCHAR,*src++);
  *dst=0;
}

inline void otl_convert_SQLWCHAR_to_char(unsigned char* dst, const SQLWCHAR*src)
{
  while(*src)
    *dst++=OTL_SCAST(unsigned char,*src++);
  *dst=0;
}

inline size_t otl_strlen(const SQLWCHAR* s)
{
  size_t len=0;
  while(*s){
    ++s;
    ++len;
  }
  return len;
}

#endif

typedef OTL_SQL_TIMESTAMP_STRUCT otl_time;
const int otl_odbc_date_prec=23;
#if defined(OTL_ODBC_MSSQL_2008)
const int otl_odbc_date_scale=7;
#elif defined(OTL_ODBC_MSSQL_2005)
const int otl_odbc_date_scale=3;
#else
const int otl_odbc_date_scale=0;
#endif

const int OTL_MAX_MSG_ARR=512;

class otl_exc{
public:

#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON)
  SQLWCHAR msg[1000];
  SQLWCHAR sqlstate[1000];
#else
  unsigned char msg[1000];
  unsigned char sqlstate[1000];
#endif
  int code;
  
#if defined(OTL_EXTENDED_EXCEPTION)

#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON)
  SQLWCHAR** msg_arr;
  SQLWCHAR** sqlstate_arr;
#else
  char** msg_arr;
  char** sqlstate_arr;
#endif

  int* code_arr;
  int arr_len;

#endif

 enum{disabled=0,enabled=1};

  otl_exc():
    msg(),
    sqlstate(),
    code(0)
#if defined(OTL_EXTENDED_EXCEPTION)
#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON)
    ,msg_arr(nullptr),
    sqlstate_arr(nullptr),
#else
    ,msg_arr(nullptr),
    sqlstate_arr(nullptr),
#endif
    code_arr(nullptr),
    arr_len(0)
#endif
 {
  sqlstate[0]=0;
  msg[0]=0;
 }

#if defined(OTL_EXTENDED_EXCEPTION)
  otl_exc(const otl_exc& ex):
#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON)
    msg(),
    sqlstate(),
#else
    msg(),
    sqlstate(),
#endif
    code(0),
#if defined(OTL_EXTENDED_EXCEPTION)
#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON)
    msg_arr(nullptr),
    sqlstate_arr(nullptr),
#else
    msg_arr(nullptr),
    sqlstate_arr(nullptr),
#endif
    code_arr(nullptr),
    arr_len(0)
#endif
  {
#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON)
    otl_strcpy(OTL_RCAST(unsigned char*,msg),
               OTL_RCAST(const unsigned char*,
                         OTL_CCAST(SQLWCHAR*,ex.msg)));
    otl_strcpy(OTL_RCAST(unsigned char*,
                         OTL_CCAST(SQLWCHAR*,sqlstate)),
               OTL_RCAST(const unsigned char*,
                         OTL_CCAST(SQLWCHAR*,ex.sqlstate)));
    code=ex.code;
    arr_len=0;
    msg_arr=nullptr;
    sqlstate_arr=nullptr;
    code_arr=nullptr;
    if(ex.arr_len>0){
      sqlstate_arr=new SQLWCHAR*[ex.arr_len];
      msg_arr=new SQLWCHAR*[ex.arr_len];
      code_arr=new int[ex.arr_len];
      int i;
      size_t msg_len, sqlstate_len;
      for(i=0;i<ex.arr_len;++i){
        msg_len=otl_strlen(ex.msg_arr[i]);
        sqlstate_len=otl_strlen(ex.sqlstate_arr[i]);
        msg_arr[i]=new SQLWCHAR[msg_len+1];
        sqlstate_arr[i]=new SQLWCHAR[sqlstate_len+1];
        otl_strcpy(OTL_RCAST(unsigned char*,msg_arr[i]),
                   OTL_RCAST(const unsigned char*,
                             OTL_CCAST(SQLWCHAR*,ex.msg_arr[i])));
        otl_strcpy(OTL_RCAST(unsigned char*,sqlstate_arr[i]),
                   OTL_RCAST(const unsigned char*,
                             OTL_CCAST(SQLWCHAR*,ex.sqlstate_arr[i])));
        code_arr[i]=ex.code_arr[i];
      }
      arr_len=ex.arr_len;
    }
#else
    OTL_STRCPY_S(OTL_RCAST(char*,msg),
                 sizeof(msg),
                 OTL_RCAST(const char*,ex.msg));
    OTL_STRCPY_S(OTL_RCAST(char*,sqlstate),
                 sizeof(sqlstate),
                 OTL_RCAST(const char*,ex.sqlstate));
    code=ex.code;
    arr_len=0;
    msg_arr=nullptr;
    sqlstate_arr=nullptr;
    code_arr=nullptr;
    if(ex.arr_len>0){
      sqlstate_arr=new char*[ex.arr_len];
      msg_arr=new char*[ex.arr_len];
      code_arr=new int[ex.arr_len];
      int i;
      size_t msg_len, sqlstate_len;
      for(i=0;i<ex.arr_len;++i){
        msg_len=strlen(ex.msg_arr[i]);
        sqlstate_len=strlen(ex.sqlstate_arr[i]);
        msg_arr[i]=new char[msg_len+1];
        sqlstate_arr[i]=new char[sqlstate_len+1];
        OTL_STRCPY_S(msg_arr[i],msg_len+1,ex.msg_arr[i]);
        OTL_STRCPY_S(sqlstate_arr[i],sqlstate_len+1,ex.sqlstate_arr[i]);
        code_arr[i]=ex.code_arr[i];
      }
      arr_len=ex.arr_len;
    }
#endif
  }
#endif

 void init(const char* amsg, const int acode)
 {
#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON)
   otl_convert_char_to_SQLWCHAR(msg,OTL_RCAST(unsigned char*,OTL_CCAST(char*,amsg)));
#else
  OTL_STRCPY_S(OTL_RCAST(char*,msg),sizeof(msg),amsg);
#endif
   code=acode;
   sqlstate[0]=0;
#if defined(OTL_EXTENDED_EXCEPTION)
   msg_arr=nullptr;
   sqlstate_arr=nullptr;
   code_arr=nullptr;
   arr_len=0;
#endif
 }

  virtual ~otl_exc()
  {
#if defined(OTL_EXTENDED_EXCEPTION)
    int i;
    if(arr_len>0){
      for(i=0;i<arr_len;++i){
        delete[] msg_arr[i];
        delete[] sqlstate_arr[i];
      }
      delete[] msg_arr;
      delete[] sqlstate_arr;
      delete[] code_arr;
      arr_len=0;
      msg_arr=nullptr;
      sqlstate_arr=nullptr;
      code_arr=nullptr;
    }
#endif
  }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_exc& operator=(const otl_exc&) = delete;
private:
#else
  otl_exc& operator=(const otl_exc&)
  {
    return *this;
  }

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_exc(otl_exc&&):
    msg(),
    sqlstate(),
    code(0)
#if defined(OTL_EXTENDED_EXCEPTION)
    ,msg_arr(nullptr)
    ,sqlstate_arr(nullptr)
    ,code_arr(nullptr)
    ,arr_len(0)
#endif
  {
  }

  otl_exc& operator=(otl_exc&&)
  {
    return *this;
  }
#endif
#endif
#endif

};

#if (ODBCVER >= 0x0300)
#if defined(OTL_EXTENDED_EXCEPTION)
inline void otl_fill_exception(
  otl_exc& exception_struct,
  OTL_SQLHANDLE handle,
  OTL_SQLSMALLINT htype
)
{
#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON)
  OTL_SQLRETURN rc;
  OTL_SQLSMALLINT msg_len=0;
  SQLWCHAR* tmp_msg_arr[OTL_MAX_MSG_ARR];
  SQLWCHAR* tmp_sqlstate_arr[OTL_MAX_MSG_ARR];
  int tmp_code_arr[OTL_MAX_MSG_ARR];
  int tmp_arr_len=0;
  OTL_SQLSMALLINT tmp_msg_len=0;
  OTL_SQLSMALLINT tmp_sqlstate_len=0;
  int tmp_code;
  SQLWCHAR tmp_msg[SQL_MAX_MESSAGE_LENGTH];
  SQLWCHAR tmp_sqlstate[1000];

  otl_strcpy(OTL_RCAST(unsigned char*,tmp_msg),
             OTL_RCAST(const unsigned char*,
                       OTL_CCAST(SQLWCHAR*,exception_struct.msg)));
  otl_strcpy(OTL_RCAST(unsigned char*,tmp_sqlstate),
             OTL_RCAST(const unsigned char*,
                       OTL_CCAST(SQLWCHAR*,exception_struct.sqlstate)));
  tmp_code=exception_struct.code;

  do{
    tmp_sqlstate_len=OTL_SCAST(OTL_SQLSMALLINT,otl_strlen(tmp_sqlstate));
    tmp_msg_len=OTL_SCAST(OTL_SQLSMALLINT,otl_strlen(tmp_msg));
    ++tmp_arr_len;
    tmp_msg_arr[tmp_arr_len-1]=new SQLWCHAR[tmp_msg_len+1];
    tmp_sqlstate_arr[tmp_arr_len-1]=new SQLWCHAR[tmp_sqlstate_len+1];
    otl_strcpy(OTL_RCAST(unsigned char*,tmp_msg_arr[tmp_arr_len-1]),
               OTL_RCAST(const unsigned char*,
                         OTL_CCAST(SQLWCHAR*,tmp_msg)));
    otl_strcpy(OTL_RCAST(unsigned char*,tmp_sqlstate_arr[tmp_arr_len-1]),
               OTL_RCAST(const unsigned char*,
                         OTL_CCAST(SQLWCHAR*,tmp_sqlstate)));
    tmp_code_arr[tmp_arr_len-1]=tmp_code;
    rc=SQLGetDiagRec
      (htype,
       handle,
       OTL_SCAST(OTL_SQLSMALLINT,tmp_arr_len+1),
       OTL_RCAST(SQLWCHAR*,tmp_sqlstate),
       OTL_RCAST(OTL_SQLINTEGER_PTR,&tmp_code),
       OTL_RCAST(SQLWCHAR*,tmp_msg),
       SQL_MAX_MESSAGE_LENGTH-1,
       OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
    tmp_msg[msg_len]=0;
    if((rc==SQL_NO_DATA||rc==SQL_INVALID_HANDLE||
        rc==SQL_ERROR)&&tmp_arr_len==1){
      int i;
      for(i=0;i<tmp_arr_len;++i){
        delete[] tmp_msg_arr[i];
        delete[] tmp_sqlstate_arr[i];
      }
      return;
    }
  }while(rc!=SQL_NO_DATA&&rc!=SQL_INVALID_HANDLE&&
         rc!=SQL_ERROR&&tmp_arr_len<OTL_MAX_MSG_ARR);
    
  exception_struct.arr_len=tmp_arr_len;
  exception_struct.msg_arr=new SQLWCHAR*[tmp_arr_len];
  exception_struct.sqlstate_arr=new SQLWCHAR*[tmp_arr_len];
  exception_struct.code_arr=new int[tmp_arr_len];
  memcpy(exception_struct.msg_arr,tmp_msg_arr,tmp_arr_len*sizeof(SQLWCHAR*));
  memcpy(exception_struct.sqlstate_arr,
         tmp_sqlstate_arr,
         tmp_arr_len*sizeof(SQLWCHAR*));
  memcpy(exception_struct.code_arr,
         tmp_code_arr,
         tmp_arr_len*sizeof(int));
#elif defined(UNICODE)||defined(_UNICODE)
  OTL_SQLRETURN rc;
  OTL_SQLSMALLINT msg_len=0;
  char* tmp_msg_arr[OTL_MAX_MSG_ARR];
  char* tmp_sqlstate_arr[OTL_MAX_MSG_ARR];
  int tmp_code_arr[OTL_MAX_MSG_ARR];
  int tmp_arr_len=0;
  OTL_SQLSMALLINT tmp_msg_len=0;
  OTL_SQLSMALLINT tmp_sqlstate_len=0;
  int tmp_code;
  SQLWCHAR tmp_msg[SQL_MAX_MESSAGE_LENGTH];
  SQLWCHAR tmp_sqlstate[1000];

  otl_convert_char_to_SQLWCHAR
    (tmp_msg,
     OTL_RCAST(const unsigned char*,exception_struct.msg));
  otl_convert_char_to_SQLWCHAR
    (tmp_sqlstate,
     OTL_RCAST(const unsigned char*,exception_struct.sqlstate));
  tmp_code=exception_struct.code;

  do{
    tmp_sqlstate_len=OTL_SCAST(OTL_SQLSMALLINT,otl_strlen(tmp_sqlstate));
    tmp_msg_len=OTL_SCAST(OTL_SQLSMALLINT,otl_strlen(tmp_msg));
    ++tmp_arr_len;
    tmp_msg_arr[tmp_arr_len-1]=new char[tmp_msg_len+1];
    tmp_sqlstate_arr[tmp_arr_len-1]=new char[tmp_sqlstate_len+1];
    otl_convert_SQLWCHAR_to_char
      (OTL_RCAST(unsigned char*,tmp_msg_arr[tmp_arr_len-1]),tmp_msg);
    otl_convert_SQLWCHAR_to_char
      (OTL_RCAST(unsigned char*,tmp_sqlstate_arr[tmp_arr_len-1]),tmp_sqlstate);
    tmp_code_arr[tmp_arr_len-1]=tmp_code;
    rc=SQLGetDiagRec
      (htype,
       handle,
       OTL_SCAST(OTL_SQLSMALLINT,tmp_arr_len+1),
       tmp_sqlstate,
       OTL_RCAST(OTL_SQLINTEGER_PTR,&tmp_code),
       tmp_msg,
       SQL_MAX_MESSAGE_LENGTH-1,
       OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
    tmp_msg[msg_len]=0;
    if((rc==SQL_NO_DATA||rc==SQL_INVALID_HANDLE||
        rc==SQL_ERROR)&&tmp_arr_len==1){
      int i;
      for(i=0;i<tmp_arr_len;++i){
        delete[] tmp_msg_arr[i];
        delete[] tmp_sqlstate_arr[i];
      }
      return;
    }
  }while(rc!=SQL_NO_DATA&&rc!=SQL_INVALID_HANDLE&&
         rc!=SQL_ERROR&&tmp_arr_len<OTL_MAX_MSG_ARR);
    
  exception_struct.arr_len=tmp_arr_len;
  exception_struct.msg_arr=new char*[tmp_arr_len];
  exception_struct.sqlstate_arr=new char*[tmp_arr_len];
  exception_struct.code_arr=new int[tmp_arr_len];
  memcpy(exception_struct.msg_arr,tmp_msg_arr,tmp_arr_len*sizeof(char*));
  memcpy(exception_struct.sqlstate_arr,tmp_sqlstate_arr,tmp_arr_len*sizeof(char*));
  memcpy(exception_struct.code_arr,tmp_code_arr,tmp_arr_len*sizeof(int));

#else

  OTL_SQLRETURN rc;
  OTL_SQLSMALLINT msg_len=0;
  char* tmp_msg_arr[OTL_MAX_MSG_ARR];
  char* tmp_sqlstate_arr[OTL_MAX_MSG_ARR];
  int tmp_code_arr[OTL_MAX_MSG_ARR];
  int tmp_arr_len=0;
  OTL_SQLSMALLINT tmp_msg_len=0;
  OTL_SQLSMALLINT tmp_sqlstate_len=0;
  int tmp_code;
  char tmp_msg[SQL_MAX_MESSAGE_LENGTH];
  char tmp_sqlstate[1000];

  OTL_STRCPY_S(tmp_msg,
               sizeof(tmp_msg),
               OTL_RCAST(char*,exception_struct.msg));
  OTL_STRCPY_S(tmp_sqlstate,
               sizeof(tmp_sqlstate),
               OTL_RCAST(char*,exception_struct.sqlstate));
  tmp_code=exception_struct.code;

  do{
    tmp_sqlstate_len=OTL_SCAST(OTL_SQLSMALLINT,strlen(tmp_sqlstate));
    tmp_msg_len=OTL_SCAST(OTL_SQLSMALLINT,strlen(tmp_msg));
    ++tmp_arr_len;
    tmp_msg_arr[tmp_arr_len-1]=new char[tmp_msg_len+1];
    tmp_sqlstate_arr[tmp_arr_len-1]=new char[tmp_sqlstate_len+1];
    OTL_STRCPY_S(tmp_msg_arr[tmp_arr_len-1],tmp_msg_len+1,tmp_msg);
    OTL_STRCPY_S(tmp_sqlstate_arr[tmp_arr_len-1],
                 tmp_sqlstate_len+1,
                 tmp_sqlstate);
    tmp_code_arr[tmp_arr_len-1]=tmp_code;
    void* temp_ptr=&tmp_code;
    rc=SQLGetDiagRec
      (htype,
       handle,
       OTL_SCAST(OTL_SQLSMALLINT,tmp_arr_len+1),
       OTL_RCAST(OTL_SQLCHAR_PTR,tmp_sqlstate),
       OTL_RCAST(OTL_SQLINTEGER_PTR,temp_ptr),
       OTL_RCAST(OTL_SQLCHAR_PTR,tmp_msg),
       SQL_MAX_MESSAGE_LENGTH-1,
       OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
    tmp_msg[msg_len]=0;
    if((rc==SQL_NO_DATA||rc==SQL_INVALID_HANDLE||
        rc==SQL_ERROR)&&tmp_arr_len==1){
      int i;
      for(i=0;i<tmp_arr_len;++i){
        delete[] tmp_msg_arr[i];
        delete[] tmp_sqlstate_arr[i];
      }
      return;
    }
  }while(rc!=SQL_NO_DATA&&rc!=SQL_INVALID_HANDLE&&
         rc!=SQL_ERROR&&tmp_arr_len<OTL_MAX_MSG_ARR);
    
  exception_struct.arr_len=tmp_arr_len;
  exception_struct.msg_arr=new char*[tmp_arr_len];
  exception_struct.sqlstate_arr=new char*[tmp_arr_len];
  exception_struct.code_arr=new int[tmp_arr_len];
  memcpy(exception_struct.msg_arr,tmp_msg_arr,tmp_arr_len*sizeof(char*));
  memcpy(exception_struct.sqlstate_arr,tmp_sqlstate_arr,tmp_arr_len*sizeof(char*));
  memcpy(exception_struct.code_arr,tmp_code_arr,tmp_arr_len*sizeof(int));
#endif
}
#endif
#endif

const int OTL_DEFAULT_ODBC_CONNECT=1;
const int OTL_TIMESTEN_ODBC_CONNECT=2;
const int OTL_MSSQL_2005_ODBC_CONNECT=3;
const int OTL_POSTGRESQL_ODBC_CONNECT=4;
const int OTL_ENTERPRISE_DB_ODBC_CONNECT=5;
const int OTL_MYODBC35_ODBC_CONNECT=6;
const int OTL_MSSQL_2008_ODBC_CONNECT=7;

class otl_cur;
class otl_connect;
class otl_sel;

class otl_conn{
protected:

  friend class otl_connect;
  friend class otl_cur;
  friend class otl_sel;

  OTL_HENV henv;
  OTL_HDBC hdbc;
  int timeout;
  int cursor_type;
  int status;
  int long_max_size;
  bool extern_lda;

#if defined(OTL_ODBC_zOS)
  bool logoff_commit;
#endif

#if defined(OTL_THROWS_ON_SQL_SUCCESS_WITH_INFO)
  bool throws_on_sql_success_with_info;
#endif

  int connection_type;

public:


  enum bigint_type
  {
#if defined(OTL_BIGINT) && !defined(OTL_STR_TO_BIGINT) && !defined(OTL_BIGINT_TO_STR)
    var_bigint = otl_var_bigint,
    bigint_size = sizeof(OTL_BIGINT)
#else
    var_bigint = otl_var_char,
    bigint_size = otl_bigint_str_size
#endif
  };

  enum ubigint_type
  {
#if defined(OTL_UBIGINT)
    var_ubigint = otl_var_ubigint,
    ubigint_size = sizeof(OTL_UBIGINT)
#else
    var_ubigint = otl_var_char,
    ubigint_size = otl_ubigint_str_size
#endif
  };

  void cleanup(void){}

  OTL_HENV& get_henv(){return henv;}
  OTL_HDBC& get_hdbc(){return hdbc;}

  int get_connection_type(void)
  {
    return connection_type;
  }

  static int initialize(const int /* threaded_mode */=0)
  {
    return 1;
  }

 otl_conn():
   henv(OTL_SQL_NULL_HANDLE_VAL),
   hdbc(OTL_SQL_NULL_HANDLE_VAL),
   timeout(0),
   cursor_type(0),
   status(SQL_SUCCESS),
   long_max_size(otl_short_int_max),
   extern_lda(false)
#if defined(OTL_ODBC_zOS)
   ,logoff_commit(true)
#endif
#if defined(OTL_THROWS_ON_SQL_SUCCESS_WITH_INFO)
   ,throws_on_sql_success_with_info(false)
#endif
   ,connection_type(OTL_DEFAULT_ODBC_CONNECT)
 {
 }

#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON)
 int rlogon(const SQLWCHAR* username,
            const SQLWCHAR* passwd,
            const SQLWCHAR* tnsname,
            const int auto_commit)
  {
    if(extern_lda){
      extern_lda=false;
      henv=nullptr;
      hdbc=nullptr;
    }
    OTL_TRACE_RLOGON_ODBC_W
      (0x1,
       L"otl_connect",
       L"rlogon",
       OTL_RCAST(const OTL_UNICODE_CHAR_TYPE*,tnsname),
       OTL_RCAST(const OTL_UNICODE_CHAR_TYPE*,username),
       OTL_RCAST(const OTL_UNICODE_CHAR_TYPE*,passwd),
       auto_commit);
    if(henv==nullptr||hdbc==nullptr){
      status=SQLAllocHandle(SQL_HANDLE_ENV,OTL_SQL_NULL_HANDLE,&henv);
      if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
      status=SQLSetEnvAttr
        (henv,
         SQL_ATTR_ODBC_VERSION,
         OTL_RCAST(void*,SQL_OV_ODBC3),
         SQL_NTS);
      if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
      status=SQLAllocHandle(SQL_HANDLE_DBC,henv,&hdbc);
      if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
    }
    if(auto_commit){
      status=SQLSetConnectAttr
        (hdbc,
         SQL_ATTR_AUTOCOMMIT,
         OTL_RCAST(SQLPOINTER,SQL_AUTOCOMMIT_ON),
         SQL_IS_POINTER);
      if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
    }else{
      status=SQLSetConnectAttr
        (hdbc,
         SQL_ATTR_AUTOCOMMIT,
         OTL_RCAST(SQLPOINTER,SQL_AUTOCOMMIT_OFF),
         SQL_IS_POINTER);
      if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
    }
    if(timeout>0){
      status=SQLSetConnectAttr
        (hdbc,
         SQL_ATTR_LOGIN_TIMEOUT,
         OTL_RCAST(void*,OTL_SCAST(size_t,timeout)),
         0);
      if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
    }

#if defined(OTL_DB2_CLI)
    status=SQLSetConnectAttr
      (hdbc,
       SQL_ATTR_LONGDATA_COMPAT,
       OTL_RCAST(SQLPOINTER,SQL_LD_COMPAT_YES),
       SQL_IS_INTEGER);
    if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
#endif
#if defined(OTL_ENABLE_MSSQL_MARS)
#define OTL_SQL_COPT_SS_BASE 1200
#define OTL_SQL_COPT_SS_MARS_ENABLED (OTL_SQL_COPT_SS_BASE+24)
#define OTL_SQL_MARS_ENABLED_YES 1L
#if !defined(OTL_DB2_CLI) && (ODBCVER >= 0x0300)
    status=SQLSetConnectAttr
      (hdbc,
    OTL_SQL_COPT_SS_MARS_ENABLED,
       OTL_RCAST(SQLPOINTER,OTL_SQL_MARS_ENABLED_YES),
       SQL_IS_UINTEGER);
    if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
      return 0;
#endif
#endif 
    status=SQLConnect
      (hdbc,
       OTL_CCAST(SQLWCHAR*,tnsname),SQL_NTS,
       OTL_CCAST(SQLWCHAR*,username),SQL_NTS,
       OTL_CCAST(SQLWCHAR*,passwd),SQL_NTS);
    if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
      return 0;
    else
      return 1;
  }
#endif

#if defined(OTL_DB2_CLI)
  int set_prog_name(const char* prog_name)
  {
    if(henv==OTL_SQL_NULL_HANDLE_VAL||hdbc==OTL_SQL_NULL_HANDLE_VAL){
      status=SQLAllocHandle(SQL_HANDLE_ENV,OTL_SQL_NULL_HANDLE,&henv);
      if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
      status=SQLSetEnvAttr
        (henv,
         SQL_ATTR_ODBC_VERSION,
         OTL_RCAST(void*,SQL_OV_ODBC3),
         SQL_NTS);
      if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
      status=SQLAllocHandle(SQL_HANDLE_DBC,henv,&hdbc);
      if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
    }

#if !defined(SQL_ATTR_INFO_PROGRAMNAME)
#define SQL_ATTR_INFO_PROGRAMNAME 2516
#endif

    status=SQLSetConnectAttr
      (hdbc,
       SQL_ATTR_INFO_PROGRAMNAME,
       OTL_RCAST(SQLPOINTER,OTL_CCAST(char*,prog_name)),
       SQL_NTS);
    if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
      return 0;
    else
      return 1;
  } 
#endif

 int ext_logon(OTL_HENV ahenv,
               OTL_HDBC ahdbc,
               const int 
#ifndef OTL_ODBC_MYSQL
               auto_commit
#endif
              )
 {
  if(!extern_lda){
#if (ODBCVER >= 0x0300)
    if(hdbc!=OTL_SQL_NULL_HANDLE_VAL){
      status=SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
    }
#else
    if(hdbc!=nullptr)
      status=SQLFreeConnect(hdbc);
#endif
    hdbc=OTL_SQL_NULL_HANDLE_VAL;
#if (ODBCVER >= 0x0300)
    if(henv!=OTL_SQL_NULL_HANDLE_VAL){
      status=SQLFreeHandle(SQL_HANDLE_ENV,henv);
    }
#else
   if(henv!=nullptr)
     status=SQLFreeEnv(henv);
#endif
   henv=OTL_SQL_NULL_HANDLE_VAL;
  }
  extern_lda=true;
  henv=ahenv;
  hdbc=ahdbc;
#ifndef OTL_ODBC_MYSQL
#if (ODBCVER >= 0x0300)
  if(auto_commit)
   status=SQLSetConnectAttr
    (hdbc,
     SQL_ATTR_AUTOCOMMIT,
     OTL_RCAST(SQLPOINTER,SQL_AUTOCOMMIT_ON),
     SQL_IS_POINTER);
  else
   status=SQLSetConnectAttr
    (hdbc,
     SQL_ATTR_AUTOCOMMIT,
#if defined(OTL_ANSI_CPP_11_NULLPTR_SUPPORT)
     nullptr,
#else
     OTL_RCAST(SQLPOINTER,SQL_AUTOCOMMIT_OFF),
#endif
     SQL_IS_POINTER);
#else
  if(auto_commit)
   status=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,1); 
  else
   status=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,0); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
#endif

#if defined(OTL_DB2_CLI)
  status=SQLSetConnectAttr
   (hdbc,
    SQL_ATTR_LONGDATA_COMPAT,
    OTL_RCAST(SQLPOINTER,SQL_LD_COMPAT_YES),
    SQL_IS_INTEGER);
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
#endif

  return 1;

 }

 virtual ~otl_conn()
 {
  if(extern_lda){
   hdbc=OTL_SQL_NULL_HANDLE_VAL;
   henv=OTL_SQL_NULL_HANDLE_VAL;
   extern_lda=false;
  }else{
#if (ODBCVER >= 0x0300)
    if(hdbc!=OTL_SQL_NULL_HANDLE_VAL){
      status=SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
    }
#else
    if(hdbc!=nullptr)
      status=SQLFreeConnect(hdbc);
#endif
   hdbc=OTL_SQL_NULL_HANDLE_VAL;
#if (ODBCVER >= 0x0300)
   if(henv!=OTL_SQL_NULL_HANDLE_VAL){
     status=SQLFreeHandle(SQL_HANDLE_ENV,henv);
   }
#else
   if(henv!=nullptr)
     status=SQLFreeEnv(henv);
#endif
   henv=OTL_SQL_NULL_HANDLE_VAL;
  }
 }
 
 void set_timeout(const int atimeout=0)
 {
  timeout=atimeout;
 }

 void set_cursor_type(const int acursor_type=0)
 {
  cursor_type=acursor_type;
 }

 int rlogon(const char* connect_str,
            const int 
#ifndef OTL_ODBC_MYSQL
            auto_commit
#endif
           )
 {
  char username[256];
  char passwd[256];
  char tnsname[1024];
  char* tnsname_ptr=nullptr;
  char* c=OTL_CCAST(char*,connect_str);
  char* username_ptr=username;
  char* passwd_ptr=passwd;
  char temp_connect_str[512];

  if(extern_lda){
   extern_lda=false;
   henv=OTL_SQL_NULL_HANDLE_VAL;
   hdbc=OTL_SQL_NULL_HANDLE_VAL;
  }
  memset(username,0,sizeof(username));
  memset(passwd,0,sizeof(passwd));
  memset(tnsname,0,sizeof(tnsname));

  char* c1=OTL_CCAST(char*,connect_str);
  int oracle_format=0;
  char prev_c=' ';
  while(*c1){
   if(*c1=='@' && prev_c!='\\'){
    oracle_format=1;
    break;
   }
   prev_c=*c1;
   ++c1;
  }

  if(oracle_format){
   while(*c && *c!='/' && (OTL_SCAST(unsigned,username_ptr-username)<
                           sizeof(username)-1)){
    *username_ptr=*c;
    ++c;
    ++username_ptr;
   }
   *username_ptr=0;

   if(*c=='/')++c;
   prev_c=' ';
   while(*c && !(*c=='@' && prev_c!='\\') && 
         (OTL_SCAST(unsigned,passwd_ptr-passwd)<sizeof(passwd)-1)){
     if(prev_c=='\\')--passwd_ptr;
    *passwd_ptr=*c;
    prev_c=*c;
    ++c;
    ++passwd_ptr;
   }
   *passwd_ptr=0;

   if(*c=='@'){
    ++c;
    tnsname_ptr=tnsname;
    while(*c && (OTL_SCAST(unsigned,tnsname_ptr-tnsname)<sizeof(tnsname)-1)){
     *tnsname_ptr=*c;
     ++c;
     ++tnsname_ptr;
    }
    *tnsname_ptr=0;
   }
  }else{
   c1=OTL_CCAST(char*,connect_str);
   char* c2=temp_connect_str;
   while(*c1 && (OTL_SCAST(unsigned,c2-temp_connect_str)
                 <sizeof(temp_connect_str)-1)){
    *c2=otl_to_upper(*c1);
    ++c1;
    ++c2;
   }
   *c2=0;
   c1=temp_connect_str;
   char entry_name[256];
   char entry_value[256];
   while(*c1 && (OTL_SCAST(unsigned,c1-temp_connect_str)<
                 sizeof(temp_connect_str)-1)){
    c2=entry_name;
    while(*c1 && *c1!='=' && 
          (OTL_SCAST(unsigned,c1-temp_connect_str)<
           sizeof(temp_connect_str)-1)){
     *c2=*c1;
     ++c1;
     ++c2;
    }
    *c2=0;
#if defined(_MSC_VER) && (_MSC_VER==1600)
    entry_name[c2-entry_name]=0;
#endif
    if(*c1) ++c1;
    c2=entry_value;
    prev_c=' ';
    while(*c1&&*c1!=';' && 
          (OTL_SCAST(unsigned,c2-entry_value)<
           sizeof(entry_value)-1)){
      if(prev_c=='\\')
        --c2;
      *c2=*c1;
      prev_c=*c1;
      ++c1;
      ++c2;
    }
    *c2=0;
#if defined(_MSC_VER) && (_MSC_VER==1600)
    entry_value[c2-entry_value]=0;
#endif
    if(*c1) ++c1;
    if(strcmp(entry_name,"DSN")==0)
      OTL_STRCPY_S(tnsname,sizeof(tnsname),entry_value);
    if(strcmp(entry_name,"UID")==0)
      OTL_STRCPY_S(username,sizeof(username),entry_value);
    if(strcmp(entry_name,"PWD")==0)
      OTL_STRCPY_S(passwd,sizeof(passwd),entry_value);
   }
  }
#ifndef OTL_ODBC_MYSQL
  OTL_TRACE_RLOGON_ODBC
    (0x1,
     "otl_connect",
     "rlogon",
     tnsname,
     username,
     passwd,
     auto_commit)
#else
  OTL_TRACE_RLOGON_ODBC
    (0x1,
     "otl_connect",
     "rlogon",
     tnsname,
     username,
     passwd,
     0)
#endif
  if(henv==OTL_SQL_NULL_HANDLE_VAL||hdbc==OTL_SQL_NULL_HANDLE_VAL){
#if (ODBCVER >= 0x0300)
    status=SQLAllocHandle(SQL_HANDLE_ENV,OTL_SQL_NULL_HANDLE_VAL,&henv);
#else
   status=SQLAllocEnv(&henv);
#endif
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;

#if (ODBCVER >= 0x0300)
   status=SQLSetEnvAttr
     (henv,
      SQL_ATTR_ODBC_VERSION,
      OTL_RCAST(void*,SQL_OV_ODBC3),
      SQL_NTS);
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
#endif

#if (ODBCVER >= 0x0300)
   status=SQLAllocHandle(SQL_HANDLE_DBC,henv,&hdbc);
#else
   status=SQLAllocConnect(henv, &hdbc);
#endif
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
  }else
    status=SQL_SUCCESS;

#ifndef OTL_ODBC_MYSQL
#if (ODBCVER >= 0x0300)
  if(auto_commit)
   status=SQLSetConnectAttr
    (hdbc,
     SQL_ATTR_AUTOCOMMIT,
     OTL_RCAST(SQLPOINTER,SQL_AUTOCOMMIT_ON),
     SQL_IS_POINTER);
  else
   status=SQLSetConnectAttr
    (hdbc,
     SQL_ATTR_AUTOCOMMIT,
#if defined(OTL_ANSI_CPP_11_NULLPTR_SUPPORT)
     nullptr,
#else
     OTL_RCAST(SQLPOINTER,SQL_AUTOCOMMIT_OFF),
#endif
     SQL_IS_POINTER);
#else
  if(auto_commit)
   status=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,1); 
  else
   status=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,0); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
#endif
#if (ODBCVER >= 0x0300)
  if(timeout>0)
   status=SQLSetConnectAttr
    (hdbc,
     SQL_ATTR_LOGIN_TIMEOUT,
     OTL_RCAST(void*,OTL_SCAST(size_t,timeout)),
     0);
#else
  if (timeout>0)
    status=SQLSetConnectOption(hdbc,SQL_LOGIN_TIMEOUT,timeout); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;


#if defined(OTL_DB2_CLI)
  status=SQLSetConnectAttr
   (hdbc,
    SQL_ATTR_LONGDATA_COMPAT,
    OTL_RCAST(SQLPOINTER,SQL_LD_COMPAT_YES),
    SQL_IS_INTEGER);
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
#endif

#if defined(OTL_ENABLE_MSSQL_MARS)
#define OTL_SQL_COPT_SS_BASE 1200
#define OTL_SQL_COPT_SS_MARS_ENABLED (OTL_SQL_COPT_SS_BASE+24)
#define OTL_SQL_MARS_ENABLED_YES 1L
#if !defined(OTL_DB2_CLI) && (ODBCVER >= 0x0300)
 status=SQLSetConnectAttr
   (hdbc,
    OTL_SQL_COPT_SS_MARS_ENABLED,
    OTL_RCAST(SQLPOINTER,OTL_SQL_MARS_ENABLED_YES),
    SQL_IS_UINTEGER);
  if(status!=SQL_SUCCESS_WITH_INFO && status!=SQL_SUCCESS)
    return 0;
#endif
#endif 

  if(oracle_format){
#if defined(OTL_ODBC_zOS)
    if(tnsname[0]==0 && username[0]==0 && passwd[0]==0){
      status=SQLConnect
        (hdbc,
         0L,SQL_NTS,
         0L,SQL_NTS,
         0L,SQL_NTS);
      logoff_commit = false;
    }else
      status=SQLConnect
        (hdbc,
         OTL_RCAST(unsigned char*,tnsname),SQL_NTS,
         OTL_RCAST(unsigned char*,username),SQL_NTS,
         OTL_RCAST(unsigned char*,passwd),SQL_NTS);
#else

#if defined(UNICODE) || defined(_UNICODE)
 {
   SQLWCHAR* temp_tnsname=new SQLWCHAR[strlen(tnsname)+1];
   SQLWCHAR* temp_username=new SQLWCHAR[strlen(username)+1];
   SQLWCHAR* temp_passwd=new SQLWCHAR[strlen(passwd)+1];
   otl_convert_char_to_SQLWCHAR(temp_tnsname,OTL_RCAST(unsigned char*,tnsname));
   otl_convert_char_to_SQLWCHAR(temp_username,OTL_RCAST(unsigned char*,username));
   otl_convert_char_to_SQLWCHAR(temp_passwd,OTL_RCAST(unsigned char*,passwd));
   status=SQLConnect
     (hdbc,
      temp_tnsname,SQL_NTS,
      temp_username,SQL_NTS,
      temp_passwd,SQL_NTS);
    delete[] temp_tnsname;
    delete[] temp_username;
    delete[] temp_passwd;
 }
#else
    status=SQLConnect
      (hdbc,
       OTL_RCAST(unsigned char*,tnsname),SQL_NTS,
       OTL_RCAST(unsigned char*,username),SQL_NTS,
       OTL_RCAST(unsigned char*,passwd),SQL_NTS);
#endif

#endif
  }else{
   char* tc2=temp_connect_str;
   const char* tc1=connect_str;
   prev_c=' ';
   while(*tc1 && (OTL_SCAST(unsigned,tc2-temp_connect_str)<
                  sizeof(temp_connect_str)-1)){
     if(*tc1=='@' && prev_c=='\\')
       --tc2;
     *tc2=*tc1;
     prev_c=*tc1;
     ++tc1;
     ++tc2;
   }
   *tc2=0;
#if defined(_MSC_VER) && (_MSC_VER==1600)
   temp_connect_str[tc2-temp_connect_str]=0;
#endif
   SQLSMALLINT out_len=0;
#if (defined(UNICODE)||defined(_UNICODE))
 {
   size_t len=strlen(temp_connect_str);
   SQLWCHAR* temp_connect_str2=new SQLWCHAR[len+1];
   SQLWCHAR out_str[2048];
   otl_convert_char_to_SQLWCHAR(temp_connect_str2,OTL_RCAST(unsigned char*,temp_connect_str));
   status=SQLDriverConnect
    (hdbc,
     0,
     temp_connect_str2,
     OTL_SCAST(short,len),
     out_str,
     sizeof(out_str)/sizeof(SQLWCHAR),
     &out_len,
     SQL_DRIVER_NOPROMPT);
    delete[] temp_connect_str2;
 }
#else
   SQLCHAR out_str[2048];
   status=SQLDriverConnect
    (hdbc,
     nullptr,
     OTL_RCAST(SQLCHAR*,OTL_CCAST(char*,temp_connect_str)),
     OTL_SCAST(short,strlen(temp_connect_str)),
     out_str,
     OTL_SCAST(SQLSMALLINT,sizeof(out_str)),
     &out_len,
     SQL_DRIVER_NOPROMPT);
#endif
  }

  if(status == SQL_SUCCESS_WITH_INFO || status == SQL_SUCCESS)
    return 1;
  else
    return 0;

 }

  int set_transaction_isolation_level
  (const long int 
#ifndef OTL_ODBC_MYSQL
   level
#endif
  )
  {
#ifndef OTL_ODBC_MYSQL
#if (ODBCVER >= 0x0300)
   status=SQLSetConnectAttr
    (hdbc,
     SQL_ATTR_TXN_ISOLATION,
     OTL_RCAST(SQLPOINTER,OTL_SCAST(size_t,level)),
     SQL_IS_POINTER);
#else
   status=SQLSetConnectOption(hdbc,SQL_TXN_ISOLATION,level);
#endif
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
    return 0;
   else
    return 1;
#else
   return 1;
#endif

  }

 int auto_commit_on(void)
 {
#if defined(OTL_ODBC_MYSQL)
  return 1;
#else
#if (ODBCVER >= 0x0300)
  status=SQLSetConnectAttr
   (hdbc,
    SQL_ATTR_AUTOCOMMIT,
    OTL_RCAST(SQLPOINTER,SQL_AUTOCOMMIT_ON),
    SQL_IS_POINTER);
#else
  status=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,1); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  else
   return 1;
#endif
 }

 int auto_commit_off(void)
 {
#if defined(OTL_ODBC_MYSQL)
  return 1;
#else
#if (ODBCVER >= 0x0300)
  status=SQLSetConnectAttr
   (hdbc,
    SQL_ATTR_AUTOCOMMIT,
#if defined(OTL_ANSI_CPP_11_NULLPTR_SUPPORT)
    nullptr,
#else
    OTL_RCAST(SQLPOINTER,SQL_AUTOCOMMIT_OFF),
#endif
    SQL_IS_POINTER);
#else
 status=SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,0); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  else
   return 1;
#endif
 }


 int logoff(void)
 {
  if(extern_lda){
   extern_lda=false;
   henv=OTL_SQL_NULL_HANDLE_VAL;
   hdbc=OTL_SQL_NULL_HANDLE_VAL;
   return 1;
  }else{
#if defined(OTL_ODBC_zOS)
   if(logoff_commit) 
     commit();
#else
   commit();
#endif
   status=SQLDisconnect(hdbc);
#if defined(OTL_ODBC_LOGOFF_FREES_HANDLES)
#if (ODBCVER >= 0x0300)
   if(hdbc!=nullptr){
     SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
     hdbc=nullptr;
   }
   if(henv!=nullptr){
     SQLFreeHandle(SQL_HANDLE_ENV, henv);
     henv=nullptr;
   }
   #else
   if(hdbc!=nullptr){
     SQLFreeConnect(hdbc);
     hdbc=nullptr;
   }
   if(henv!=nullptr){
     SQLFreeEnv(henv);
     henv=nullptr;
   }
#endif
#endif
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
    return 0;
   else
    return 1;

  }
 }

 void error(otl_exc& exception_struct)
 {OTL_SQLRETURN rc;
  OTL_SQLSMALLINT msg_len=0;

#if (ODBCVER >= 0x0300)

#if (defined(UNICODE)||defined(_UNICODE))

#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON)

   rc=SQLGetDiagRec
#if defined(OTL_ODBC_zOS)
     (hdbc==nullptr?SQL_HANDLE_ENV:SQL_HANDLE_DBC,
      hdbc==nullptr?henv:hdbc,
#else
      (SQL_HANDLE_DBC,
       hdbc,
#endif
       1,
       &exception_struct.sqlstate[0],
       OTL_RCAST(OTL_SQLINTEGER_PTR,&exception_struct.code),
       &exception_struct.msg[0],
       SQL_MAX_MESSAGE_LENGTH-1,
       OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
   exception_struct.msg[msg_len]=0;

#else

 {
   SQLWCHAR temp_msg[SQL_MAX_MESSAGE_LENGTH];
   SQLWCHAR temp_sqlstate[1000];

   rc=SQLGetDiagRec
#if defined(OTL_ODBC_zOS)
     (hdbc==nullptr?SQL_HANDLE_ENV:SQL_HANDLE_DBC,
      hdbc==nullptr?henv:hdbc,
#else
      (SQL_HANDLE_DBC,
       hdbc,
#endif
       1,
       temp_sqlstate,
       OTL_RCAST(OTL_SQLINTEGER_PTR,&exception_struct.code),
       temp_msg,
       SQL_MAX_MESSAGE_LENGTH-1,
       OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
   temp_msg[msg_len]=0;
   otl_convert_SQLWCHAR_to_char
      (OTL_RCAST(unsigned char*,&exception_struct.sqlstate[0]),
       temp_sqlstate);
   otl_convert_SQLWCHAR_to_char
      (OTL_RCAST(unsigned char*,&exception_struct.msg[0]),
       temp_msg);
  }

#endif

#else
   void* temp_ptr=&exception_struct.code;
   rc=SQLGetDiagRec
#if defined(OTL_ODBC_zOS)
     (hdbc==nullptr?SQL_HANDLE_ENV:SQL_HANDLE_DBC,
      hdbc==nullptr?henv:hdbc,
#else
      (SQL_HANDLE_DBC,
       hdbc,
#endif
       1,
       OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.sqlstate[0]),
       OTL_RCAST(OTL_SQLINTEGER_PTR,temp_ptr),
       OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.msg[0]),
       SQL_MAX_MESSAGE_LENGTH-1,
       OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
#endif

#else
 rc=SQLError(henv, 
             hdbc, 
             0, // hstmt
             OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.sqlstate[0]),
             OTL_RCAST(OTL_SQLINTEGER_PTR,&exception_struct.code),
             OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.msg[0]),
             SQL_MAX_MESSAGE_LENGTH-1,
             OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
#endif
  exception_struct.msg[msg_len]=0;

  if(rc==SQL_INVALID_HANDLE||rc==SQL_ERROR)
    exception_struct.msg[0]=0;
#if (ODBCVER >= 0x0300)
#if defined(OTL_EXTENDED_EXCEPTION)
  else if(rc!=SQL_NO_DATA)
#if defined(OTL_ODBC_zOS)
    {
      if(hdbc)
        otl_fill_exception(exception_struct,hdbc,SQL_HANDLE_DBC);
      else
        otl_fill_exception(exception_struct,henv,SQL_HANDLE_ENV);
    }
#else
    otl_fill_exception(exception_struct,hdbc,SQL_HANDLE_DBC);
#endif
#endif
#endif

 }

 int commit(void)
 {
#ifndef OTL_ODBC_MYSQL
#if (ODBCVER >= 0x0300)
  status=SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_COMMIT);
#else
 status=SQLTransact(henv,hdbc,SQL_COMMIT); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  else
  return 1;
#else
  return 1;
#endif
 }

 int rollback(void)
 {
#ifndef OTL_ODBC_MYSQL
#if (ODBCVER >= 0x0300)
  status=SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_ROLLBACK);
#else
 status=SQLTransact(henv,hdbc,SQL_ROLLBACK); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  else
  return 1;
#else
  return 1;
#endif
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
 public:
 otl_conn(const otl_conn&) = delete;
 otl_conn& operator=(const otl_conn&) = delete;
 private:
#else
 otl_conn(const otl_conn&):
   henv(OTL_SQL_NULL_HANDLE_VAL),
   hdbc(OTL_SQL_NULL_HANDLE_VAL),
   timeout(0),
   cursor_type(0),
   status(SQL_SUCCESS),
   long_max_size(otl_short_int_max),
   extern_lda(false)
#if defined(OTL_ODBC_zOS)
   ,logoff_commit(true)
#endif
#if defined(OTL_THROWS_ON_SQL_SUCCESS_WITH_INFO)
   ,throws_on_sql_success_with_info(false)
#endif
   ,connection_type(OTL_DEFAULT_ODBC_CONNECT)
 {
 }

 otl_conn& operator=(const otl_conn&)
 {
   return *this;
 }
#endif


};

class otl_var;
class otl_cur;
class otl_sel;

class otl_cur0{
protected:

  friend class otl_sel;
  friend class otl_var;
  OTL_SQLHSTMT cda;
  int last_param_data_token;
  int last_sql_param_data_status;
  int sql_param_data_count;
  
public:

 otl_cur0():
   cda(OTL_SQL_NULL_HANDLE_VAL),
   last_param_data_token(0),
   last_sql_param_data_status(0),
   sql_param_data_count(0)
 {
 }

 virtual ~otl_cur0(){}

  OTL_SQLHSTMT get_cda(){return cda;}  

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_cur0(const otl_cur0&) = delete;
 otl_cur0& operator=(const otl_cur0&) = delete;
private:
#else
 otl_cur0(const otl_cur0&):
   cda(OTL_SQL_NULL_HANDLE_VAL),
   last_param_data_token(0),
   last_sql_param_data_status(0),
   sql_param_data_count(0)
 {
 }

 otl_cur0& operator=(const otl_cur0&)
 {
   return *this;
 }
#endif

};

class otl_cur;

class otl_var{
private:

  friend class otl_cur;
  unsigned char* p_v;
  OTL_SQLLEN_PTR p_len;
  int ftype;
  int act_elem_size;
  bool lob_stream_mode;
  int lob_stream_flag;
  int vparam_type;
  int lob_len;
  int lob_pos;
  int lob_ftype;
  int otl_adapter;
  bool charz_flag;

public:

#if defined(OTL_CONTAINER_CLASSES_HAVE_OPTIONAL_MEMBERS)
  void set_nls_flag(const bool){}
#endif

  void reset_lob_len()
  {
    lob_len=0;
  }

  int get_otl_adapter() const {return otl_adapter;}

  void set_lob_stream_mode(const bool alob_stream_mode)
  {
    lob_stream_mode=alob_stream_mode;
  }

  void set_vparam_type(const int avparam_type)
  {
    vparam_type=avparam_type;
  }

  void set_charz_flag(const bool acharz_flag)
  {
    charz_flag=acharz_flag;
  }


  otl_var():
    p_v(nullptr),
    p_len(nullptr),
    ftype(0),
    act_elem_size(0),
    lob_stream_mode(false),
    lob_stream_flag(0),
    vparam_type(-1),
    lob_len(0),
    lob_pos(0),
    lob_ftype(0),
    otl_adapter(otl_odbc_adapter),
    charz_flag(false)
 {
 }

 virtual ~otl_var()
 {
  delete[] p_v;
  delete[] p_len;
 }

  int write_dt(void* trg, const void* src, const int sz)
  {
    memcpy(trg,src,sz);
    return 1;
  }

  int read_dt(void* trg, const void* src, const int sz)
  {
    memcpy(trg,src,sz);
    return 1;
  }

 void set_lob_stream_flag(const int flg=1)
 {
  lob_stream_flag=flg;
 }

 int get_pl_tab_len(void)
 {
  return 0;
 }

 int get_max_pl_tab_len(void)
 {
  return 0;
 }

  void set_pl_tab_len(const int /* pl_tab_len */)
  {
  }

 int write_blob
 (const otl_long_string& s,
  const int /* alob_len */,
  int& aoffset,
  otl_cur0& cur)
 {
  SQLRETURN rc=0;
  SQLINTEGER temp_len=0;
  SQLPOINTER pToken=nullptr;
  int param_number=0;

  if(!lob_stream_flag&&!lob_stream_mode)return 1;
  if(aoffset==1){
   if(cur.sql_param_data_count==0){
    rc=SQLParamData(cur.cda, &pToken);
    param_number=OTL_SCAST(int,OTL_RCAST(size_t,pToken));
    ++cur.sql_param_data_count;
    cur.last_sql_param_data_status=rc;
    cur.last_param_data_token=param_number;
    if(rc!=SQL_SUCCESS&&rc!=SQL_SUCCESS_WITH_INFO&&
       rc!=SQL_NEED_DATA)
     return 0;
   }
  }
  if(ftype==otl_var_raw_long)
    temp_len=s.len();
  else
    temp_len=s.len()*sizeof(OTL_CHAR);
  rc=SQLPutData(cur.cda,s.v,temp_len);
  if(rc!=SQL_SUCCESS&&rc!=SQL_SUCCESS_WITH_INFO)
    return 0;
   else{
     aoffset+=s.len();
    return 1;
   }
 }

 int clob_blob(otl_cur0& cur)
 {
  SQLRETURN rc=0;
  SQLPOINTER pToken=nullptr;
  int param_number=0;

  if(!(cur.last_param_data_token==0&&cur.sql_param_data_count>0)){
    rc=SQLParamData(cur.cda, &pToken);
    param_number=OTL_SCAST(int,OTL_RCAST(size_t,pToken));
    ++cur.sql_param_data_count;
    cur.last_sql_param_data_status=rc;
    cur.last_param_data_token=param_number;
    if(rc!=SQL_SUCCESS&&rc!=SQL_SUCCESS_WITH_INFO&&
#if (ODBCVER >= 0x0300)
       rc!=SQL_NO_DATA &&
#endif
       rc!=SQL_NEED_DATA)
      return 0;
  }
  return 1;
 }

 int read_blob
 (otl_cur0& cur,
  otl_long_string& s,
  const int andx,
  int& aoffset,
  int& eof_flag)
 {
  SQLRETURN rc=0;
  OTL_SQLLEN retLen=0;
  int chunkLen=0;
  if(!lob_stream_flag&&!lob_stream_mode)return 1;
  int buf_size=s.get_buf_size()*sizeof(OTL_CHAR);
  if(ftype==otl_var_raw_long)
    buf_size=s.get_buf_size();
  rc=SQLGetData
   (cur.cda,
    OTL_SCAST(SQLSMALLINT,lob_pos),
    OTL_SCAST(SQLSMALLINT,lob_ftype),
    s.v, 
    buf_size,
    &retLen);
  if(rc==SQL_SUCCESS_WITH_INFO||rc==SQL_SUCCESS){
   if(retLen==SQL_NULL_DATA){
    chunkLen=0;
    p_len[andx]=SQL_NULL_DATA;
   }else if(retLen>buf_size||retLen==SQL_NO_TOTAL)
     chunkLen=s.get_buf_size();
   else{
     if(ftype==otl_var_raw_long)
       chunkLen=OTL_SCAST(int,retLen);
     else
       chunkLen=OTL_SCAST(int,retLen)/sizeof(OTL_CHAR);
   }
#if defined(OTL_UNICODE)
   if(lob_ftype==SQL_C_WCHAR)
    s.set_len(chunkLen-1);
#else
   if(lob_ftype==SQL_C_CHAR)
    s.set_len(chunkLen-1);
#endif
   else
    s.set_len(chunkLen);
   if(lob_len==0&&aoffset==1&&
      retLen!=SQL_NULL_DATA&&
      retLen!=SQL_NO_TOTAL)
    lob_len=OTL_SCAST(int,retLen);
   aoffset+=chunkLen;
   if(chunkLen<s.get_buf_size()||rc==SQL_SUCCESS){
    s.set_len(chunkLen);
    eof_flag=1;
   }else
    eof_flag=0;
   return 1;
  }
#if (ODBCVER >= 0x0300)
  else if(rc==SQL_NO_DATA)
#else
  else if(rc==SQL_NO_DATA_FOUND)
#endif
   return 1;
  else
   return 0;
 }


  int get_blob_len(const int /* ndx */,int& alen)
  {
    alen=lob_len;
    return 1;
  }
  
 int put_blob(void)
 {
  return 1;
 }

 int get_blob
 (const int /* ndx */,
  unsigned char* /* abuf */,
  const int /* buf_size */,
  int& /* len */)
 {
  return 1;
 }

 int save_blob
 (const unsigned char* /* abuf  */,
  const int /* len */,
  const int /* extern_buffer_flag */)
 {
  return 1;
 }

 int actual_elem_size(void)
 {
  return act_elem_size;
 }

 void init
 (const bool,
  const int aftype,
  int& aelem_size,
  const otl_stream_buffer_size_type aarray_size,
  const void* /* connect_struct */=nullptr,
  const int /*apl_tab_size*/=0)
 {int i;
  size_t byte_size=0;
  ftype=aftype;
  act_elem_size=aelem_size;
  byte_size=aelem_size*OTL_SCAST(size_t,aarray_size);
#if defined(OTL_UNICODE)
  if(aftype==otl_var_char||aftype==otl_var_varchar_long){
    byte_size*=sizeof(OTL_CHAR);
    p_v=new unsigned char[byte_size];
  }
  else
  p_v=new unsigned char[byte_size];
#else
  p_v=new unsigned char[byte_size];
#endif
  p_len=new OTL_SQLLEN[aarray_size];
  memset(p_v,0,byte_size);
  for(i=0;i<aarray_size;++i){
   if(ftype==otl_var_char)
    p_len[i]=OTL_SCAST(OTL_SQLLEN,SQL_NTS);
   else if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long)
    p_len[i]=0;
   else
    p_len[i]=OTL_SCAST(OTL_SQLLEN,aelem_size);
  }
 }

 void set_null(int ndx)
 {
  p_len[ndx]=SQL_NULL_DATA;
 }

 void set_not_null(int ndx, int pelem_size)
 {
   set_len(pelem_size,ndx);
 }

 void set_len(int len, int ndx)
 {
   switch(ftype){
   case otl_var_char:
     p_len[ndx]=SQL_NTS;
     break;
   case otl_var_varchar_long:
     if(lob_stream_mode && 
        (vparam_type==otl_input_param||
         vparam_type==otl_inout_param))
       p_len[ndx]=SQL_DATA_AT_EXEC;
     else
#if defined(OTL_UNICODE)
       p_len[ndx]=OTL_SCAST(OTL_SQLLEN,len*sizeof(OTL_CHAR));
#else
       p_len[ndx]=OTL_SCAST(OTL_SQLLEN,len);
#endif
     break;
   case otl_var_raw_long:
     if(lob_stream_mode && 
        (vparam_type==otl_input_param||
         vparam_type==otl_inout_param))
       p_len[ndx]=SQL_DATA_AT_EXEC;
     else
       p_len[ndx]=OTL_SCAST(OTL_SQLLEN,len);       
     break;
   default:
     p_len[ndx]=OTL_SCAST(OTL_SQLLEN,len);
     break;
   }
 }

 int get_len(int ndx)
 {
  if(p_len[ndx]==SQL_NULL_DATA)
   return 0;
  else
#if defined(OTL_UNICODE)
    if(ftype==otl_var_varchar_long || ftype==otl_var_char)
      return OTL_SCAST(int,p_len[ndx])/sizeof(OTL_CHAR);
    else
      return OTL_SCAST(int,p_len[ndx]);
#else
   return OTL_SCAST(int,p_len[ndx]);
#endif
 }

 int is_null(int ndx)
 {
   return p_len[ndx]==SQL_NULL_DATA;
 }

 void* val(int ndx,int pelem_size)
 {
#if defined(OTL_UNICODE)
   switch(ftype){
   case otl_var_char:
   case otl_var_varchar_long:
     return OTL_RCAST(void*,
                      &p_v[(OTL_SCAST(size_t,ndx))*pelem_size*sizeof(OTL_CHAR)]);
   default:
     return OTL_RCAST(void*,&p_v[(OTL_SCAST(size_t,ndx))*pelem_size]);
   }
#else
   return OTL_RCAST(void*,&p_v[(OTL_SCAST(size_t,ndx))*pelem_size]);
#endif
 }

#define OTL_SQL_UNICODE_CHAR                        (-95)
#define OTL_SQL_UNICODE_VARCHAR                     (-96)
#define OTL_SQL_UNICODE_LONGVARCHAR                 (-97)

#define OTL_SQL_SS_TIME2 (-154)
#define OTL_SQL_SS_TIMESTAMPOFFSET (-155)

 static int int2ext(int int_type)
 {
   switch(int_type){
#if defined(OTL_UNICODE)
   case SQL_VARCHAR: return SQL_C_WCHAR;
   case SQL_WVARCHAR: return SQL_C_WCHAR;
   case SQL_CHAR: return SQL_C_WCHAR;
   case SQL_WCHAR: return SQL_C_WCHAR;
   case SQL_LONGVARCHAR: return SQL_WLONGVARCHAR;
   case SQL_WLONGVARCHAR: return SQL_WLONGVARCHAR;
   case OTL_SQL_UNICODE_VARCHAR: return SQL_C_WCHAR;
   case OTL_SQL_UNICODE_CHAR: return SQL_C_WCHAR;
   case OTL_SQL_UNICODE_LONGVARCHAR: return SQL_WLONGVARCHAR;
#else
   case SQL_CHAR: return SQL_C_CHAR;
   case SQL_VARCHAR: return SQL_C_CHAR;
#if defined(SQL_WCHAR)
   case SQL_WCHAR: return SQL_C_CHAR;
#else
   case -8: return SQL_C_CHAR;
#endif
#if defined(SQL_WVARCHAR)
   case SQL_WVARCHAR: return SQL_C_CHAR;
#else
   case -9: return SQL_C_CHAR;
#endif
   case SQL_LONGVARCHAR: return SQL_LONGVARCHAR;
#if defined(SQL_WLONGVARCHAR)
   case SQL_WLONGVARCHAR: return SQL_LONGVARCHAR;
#else
   case -10: return SQL_LONGVARCHAR;
#endif
   case OTL_SQL_UNICODE_VARCHAR: return SQL_C_CHAR;
   case OTL_SQL_UNICODE_CHAR: return SQL_C_CHAR;
   case OTL_SQL_UNICODE_LONGVARCHAR: return SQL_LONGVARCHAR;
#endif
#if (ODBCVER >= 0x0300)
   case SQL_TYPE_DATE: return SQL_C_TIMESTAMP;
   case SQL_TYPE_TIMESTAMP: return SQL_C_TIMESTAMP;
   case SQL_TYPE_TIME: return SQL_C_TIMESTAMP;
   case OTL_SQL_SS_TIME2: return SQL_C_TIMESTAMP;
#if defined(OTL_UNICODE)
   case OTL_SQL_SS_TIMESTAMPOFFSET: return SQL_C_WCHAR;
#else
   case OTL_SQL_SS_TIMESTAMPOFFSET: return SQL_C_CHAR;
#endif
#else
   case SQL_DATE: return SQL_C_TIMESTAMP;
   case SQL_TIMESTAMP: return SQL_C_TIMESTAMP;
   case SQL_TIME: return SQL_C_TIMESTAMP;
#endif
#if defined(OTL_BIGINT)
   case SQL_BIGINT: return SQL_C_SBIGINT;
#else
   case SQL_BIGINT: return SQL_C_DOUBLE;
#endif
#if defined(OTL_MAP_SQL_DECIMAL_TO_OTL_BIGINT) && !defined(OTL_BIGINT)
#error OTL_BIGINT needs to be defined for OTL_MAP_SQL_DECIMAL_TO_OTL_BIGINT \
to function
#elif defined(OTL_MAP_SQL_DECIMAL_TO_OTL_BIGINT) && defined(OTL_BIGINT)
   case SQL_DECIMAL: return SQL_C_SBIGINT;
#else
   case SQL_DECIMAL: return SQL_C_DOUBLE;
#endif
   case SQL_DOUBLE: return SQL_C_DOUBLE;
   case SQL_FLOAT: return SQL_C_DOUBLE;
   case SQL_INTEGER: return SQL_C_SLONG;
#if defined(OTL_MAP_SQL_NUMERIC_TO_OTL_BIGINT) && !defined(OTL_BIGINT)
#error OTL_BIGINT needs to be defined for OTL_MAP_SQL_NUMERIC_TO_OTL_BIGINT \
to function
#elif defined(OTL_MAP_SQL_NUMERIC_TO_OTL_BIGINT) && defined(OTL_BIGINT)
   case SQL_NUMERIC: return SQL_C_SBIGINT;
#else
   case SQL_NUMERIC: return SQL_C_DOUBLE;
#endif
   case SQL_REAL: return SQL_C_DOUBLE;
   case SQL_SMALLINT: return SQL_C_SSHORT;
   case SQL_BIT: return SQL_C_SSHORT;
   case SQL_TINYINT: return SQL_C_SSHORT;
   case SQL_LONGVARBINARY: return SQL_LONGVARBINARY;
#if defined(OTL_MAP_SQL_VARBINARY_TO_RAW_LONG)
   case SQL_VARBINARY: return SQL_LONGVARBINARY;
#else
   case SQL_VARBINARY: return SQL_C_BINARY;
#endif
#if (ODBCVER >= 0x0350)
#if defined(OTL_MAP_SQL_GUID_TO_CHAR)
#if defined(OTL_UNICODE)
   case SQL_GUID: return SQL_C_WCHAR;
#else
   case SQL_GUID: return SQL_C_CHAR;
#endif
#else
   case SQL_GUID: return SQL_C_BINARY;
#endif
#endif
#if defined(OTL_MAP_SQL_BINARY_TO_CHAR)
#if defined(OTL_UNICODE)
   case SQL_BINARY: // MS SQL TIMESTAMP, BINARY
     return SQL_C_WCHAR;
#else
   case SQL_BINARY: // MS SQL TIMESTAMP, BINARY
     return SQL_C_CHAR;
#endif
#else
   case SQL_BINARY:
     return SQL_C_BINARY;
#endif
#if (ODBCVER >= 0x0350)
   case OTL_SQL_XML:
#if defined(OTL_UNICODE)
     return SQL_C_WCHAR;
#else
     return SQL_C_CHAR;
#endif
#endif
   default: return otl_unsupported_type;
   }
 }

 static int datatype_size(int ftype,int maxsz,int int_type,int max_long_size)
 {
  switch(ftype){
#if defined(OTL_UNICODE)
  case SQL_C_WCHAR:
#endif
  case SQL_C_CHAR:
   switch(int_type){
   case SQL_BINARY: // MS SQL TIMESTAMP
     return 17;
#if defined(OTL_UNICODE)
   case SQL_WLONGVARCHAR:
#endif
   case SQL_LONGVARCHAR:
     return max_long_size*sizeof(OTL_CHAR);
   case SQL_LONGVARBINARY:
    return max_long_size;
   case SQL_DATE:
    return 40;
#if (ODBCVER >= 0x0300)
   case SQL_TYPE_TIMESTAMP:
#else
   case SQL_TIMESTAMP:
#endif
    return 40;
#if (ODBCVER >= 0x0300)
   case SQL_TYPE_TIME:
#else
   case SQL_TIME:
#endif
    return 40;
#if (ODBCVER >= 0x0350)
#if defined(OTL_MAP_SQL_GUID_TO_SQL_VARBINARY)
   case SQL_GUID:
    return 16;
#else
   case SQL_GUID:
    return 40;
#endif
#endif
   default:
     return (maxsz+1);
   }
#if defined(OTL_BIGINT)
  case SQL_C_SBIGINT:
   return sizeof(OTL_BIGINT);
#endif
#if defined(OTL_UBIGINT)
  case SQL_C_UBIGINT:
   return sizeof(OTL_UBIGINT);
#endif
  case SQL_C_DOUBLE:
   return sizeof(double);
  case SQL_C_SLONG:
   return sizeof(int);
  case SQL_C_SSHORT:
   return sizeof(short int);
  case SQL_C_TIMESTAMP:
   return sizeof(OTL_SQL_TIMESTAMP_STRUCT);
  case SQL_C_TIME:
   return sizeof(OTL_SQL_TIME_STRUCT);
  case SQL_C_DATE:
   return sizeof(OTL_SQL_DATE_STRUCT);
#if defined(OTL_UNICODE)
  case SQL_WLONGVARCHAR:
    return max_long_size;
#endif
  case SQL_LONGVARCHAR:
   return max_long_size;
  case SQL_LONGVARBINARY:
   return max_long_size;
  case SQL_C_BINARY:
   return maxsz;
  default:
   return 0;
  }
 }

 static void map_ftype
 (otl_column_desc& desc,
  const int max_long_size,
  int& ftype,
  int& elem_size,
  otl_select_struct_override& a_override,
  const int column_ndx,
  const int 
#if !defined(OTL_ODBC_TIMESTEN) && defined(OTL_ODBC_MULTI_MODE)
  connection_type
#endif
  )
 {
  int ndx=a_override.find(column_ndx);
  if(ndx==-1){
#if defined(OTL_ODBC_MSSQL_2005) && !defined(OTL_ODBC_MULTI_MODE)
   if(desc.prec==0 && desc.dbtype==SQL_VARBINARY)
     ftype=SQL_LONGVARBINARY;
   else
#elif defined(OTL_ODBC_MULTI_MODE)
     if((connection_type==OTL_MSSQL_2005_ODBC_CONNECT ||
         connection_type==OTL_MSSQL_2008_ODBC_CONNECT)&& 
      desc.prec==0 && desc.dbtype==SQL_VARBINARY)
     ftype=SQL_LONGVARBINARY;
   else
#endif
     ftype=int2ext(desc.dbtype);
   if(desc.dbsize==0){
#if !defined(OTL_UNICODE)
     if(ftype==SQL_C_CHAR)
       ftype=SQL_LONGVARCHAR;
#else
     if(ftype==SQL_C_CHAR)
       ftype=SQL_LONGVARCHAR;
     else if(ftype==SQL_C_WCHAR)
       ftype=SQL_WLONGVARCHAR;
#endif
     elem_size=max_long_size*sizeof(OTL_CHAR);
   }else{
     elem_size=datatype_size
       (ftype,
        OTL_SCAST(int,desc.dbsize),
        desc.dbtype,
        max_long_size);
   }
   switch(ftype){
#if defined(OTL_UNICODE)
   case SQL_C_WCHAR:
    ftype=otl_var_char;
    break;
   case SQL_WLONGVARCHAR:
    ftype=otl_var_varchar_long;
    break;
#else
   case SQL_C_CHAR:
    ftype=otl_var_char;
    break;
   case SQL_LONGVARCHAR:
    ftype=otl_var_varchar_long;
    break;
#endif
   case SQL_C_DOUBLE:
     if(a_override.get_all_mask() & otl_all_num2str){
     ftype=otl_var_char;
     elem_size=otl_num_str_size;
    }else
     ftype=otl_var_double;
    break;
#if defined(OTL_BIGINT)
   case SQL_C_SBIGINT:
     if(a_override.get_all_mask() & otl_all_num2str){
     ftype=otl_var_char;
     elem_size=otl_num_str_size;
    }else
     ftype=otl_var_bigint;
    break;
#endif
#if defined(OTL_UBIGINT)
   case SQL_C_UBIGINT:
     if(a_override.get_all_mask() & otl_all_num2str){
     ftype=otl_var_char;
     elem_size=otl_num_str_size;
    }else
     ftype=otl_var_ubigint;
    break;
#endif
   case SQL_C_SLONG:
     if(a_override.get_all_mask() & otl_all_num2str){
     ftype=otl_var_char;
     elem_size=otl_num_str_size;
    }else
     ftype=otl_var_int;
    break;
   case SQL_C_SSHORT:
     if(a_override.get_all_mask() & otl_all_num2str){
     ftype=otl_var_char;
     elem_size=otl_num_str_size;
    }else
     ftype=otl_var_short;
    break;
   case SQL_LONGVARBINARY:
    ftype=otl_var_raw_long;
    break;
   case SQL_C_DATE:
   case SQL_C_TIME:
   case SQL_C_TIMESTAMP:
     if(a_override.get_all_mask() & otl_all_date2str){
     ftype=otl_var_char;
     elem_size=otl_date_str_size;
    }else
     ftype=otl_var_timestamp;
    break;
   case SQL_C_BINARY:
    ftype=otl_var_raw;
    break;
   default:
    ftype=0;
    break;
   }
  }else{
    ftype=a_override.get_col_type(ndx);
   switch(ftype){
   case otl_var_char:
     elem_size=a_override.get_col_size(ndx)*sizeof(OTL_CHAR);
    break;
   case otl_var_raw:
     elem_size=a_override.get_col_size(ndx);
    break;
   case otl_var_double:
    elem_size=sizeof(double);
    break;
   case otl_var_bdouble:
    elem_size=sizeof(double);
    break;
   case otl_var_float:
    elem_size=sizeof(float);
    break;
   case otl_var_bfloat:
    elem_size=sizeof(float);
    break;
   case otl_var_int:
    elem_size=sizeof(int);
    break;
#if defined(OTL_BIGINT)
   case otl_var_bigint:
    elem_size=sizeof(OTL_BIGINT);
    break;
#endif
#if defined(OTL_UBIGINT)
   case otl_var_ubigint:
    elem_size=sizeof(OTL_UBIGINT);
    break;
#endif
   case otl_var_unsigned_int:
    elem_size=sizeof(unsigned);
    break;
   case otl_var_short:
    elem_size=sizeof(short);
    break;
   case otl_var_long_int:
    elem_size=sizeof(double);
    break;
   default:
     elem_size=a_override.get_col_size(ndx);
    break;
   }
  }
  desc.otl_var_dbtype=ftype;
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_var(const otl_var&) = delete;
 otl_var& operator=(const otl_var&) = delete;
private:
#else
  otl_var(const otl_var&):
    p_v(nullptr),
    p_len(nullptr),
    ftype(0),
    act_elem_size(0),
    lob_stream_mode(false),
    lob_stream_flag(0),
    vparam_type(-1),
    lob_len(0),
    lob_pos(0),
    lob_ftype(0),
    otl_adapter(otl_odbc_adapter),
    charz_flag(false)
 {
 }

 otl_var& operator=(const otl_var&)
 {
   return *this;
 }
#endif

};

#if defined(OTL_ODBC_zOS)||defined(OTL_ODBC_TIMESTEN)|| \
  (defined(SQL_TXN_READ_COMMITTED) && \
   !defined(SQL_TRANSACTION_READ_COMMITTED))
const long otl_tran_read_uncommitted=SQL_TXN_READ_UNCOMMITTED;
const long otl_tran_read_committed=SQL_TXN_READ_COMMITTED;
const long otl_tran_repeatable_read=SQL_TXN_REPEATABLE_READ;
const long otl_tran_serializable=SQL_TXN_SERIALIZABLE;
#else
const long otl_tran_read_uncommitted=SQL_TRANSACTION_READ_UNCOMMITTED;
const long otl_tran_read_committed=SQL_TRANSACTION_READ_COMMITTED;
const long otl_tran_repeatable_read=SQL_TRANSACTION_REPEATABLE_READ;
const long otl_tran_serializable=SQL_TRANSACTION_SERIALIZABLE;
#endif

class otl_sel;

class otl_cur: public otl_cur0{
private:

  friend class otl_sel;
  int status;
  otl_conn* adb;
  int direct_exec_flag;
  long _rpc;
  bool canceled;
  int last_iters;

public:

  void set_canceled(const bool acanceled)
  {
    canceled=acanceled;
  }

  void reset_last_param_data_token()
  {
    last_param_data_token=0;
  }

  void reset_last_sql_param_data_status()
  {
    last_sql_param_data_status=0;
  }

  void reset_sql_param_data_count()
  {
    sql_param_data_count=0;
  }


  otl_cur():
    otl_cur0(),
    status(0),
    adb(nullptr),
    direct_exec_flag(0),
    _rpc(0),
    canceled(false),
    last_iters(0)
 {
  cda=OTL_SQL_NULL_HANDLE_VAL;
  last_param_data_token=0;
  last_sql_param_data_status=0;
  sql_param_data_count=0;
 }

 virtual ~otl_cur(){}

  int cancel(void)
  {
    status=SQLCancel(cda);
    canceled=true;
    if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
      return 0;
    else
      return 1;
  }

  int open(otl_conn& /* connect */,otl_var* /* var */)
  {
    return 1;
  }

 void set_direct_exec(const int flag)
 {
  direct_exec_flag=flag;
 }

  void set_parse_only(const int /*flag*/){}

 int open(otl_conn& connect)
 {
   last_iters=0;
  direct_exec_flag=0;
  adb=&connect;
#if (ODBCVER >= 0x0300)
  status=SQLAllocHandle(SQL_HANDLE_STMT,connect.hdbc,&cda);
#else
 status=SQLAllocStmt(connect.hdbc,&cda); 
#endif
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
  if(connect.timeout>0){
#if (ODBCVER >= 0x0300)
   status=SQLSetStmtAttr
    (cda,
     SQL_ATTR_QUERY_TIMEOUT,
     OTL_RCAST(void*,OTL_SCAST(size_t,connect.timeout)),
     SQL_NTS);
#else
  status=SQLSetStmtOption(cda,SQL_QUERY_TIMEOUT,connect.timeout);
#endif
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
    return 0;
  }
  if(connect.cursor_type!=0){ // other than default
#if (ODBCVER >= 0x0300)
   status=SQLSetStmtAttr
    (cda,
     SQL_ATTR_CURSOR_TYPE,
     OTL_RCAST(void*,OTL_SCAST(size_t,connect.cursor_type)),
     SQL_NTS);
#else
  status=SQLSetStmtOption(cda,SQL_CURSOR_TYPE,connect.cursor_type);
#endif
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
    return 0;
  }
  return 1;
 }

 int close(void)
 {
   last_iters=0;
#if (ODBCVER >= 0x0300)
  status=SQLFreeHandle(SQL_HANDLE_STMT,cda);
#else
  status=SQLFreeStmt(cda,SQL_DROP);
#endif
  adb=nullptr;
  cda=OTL_SQL_NULL_HANDLE_VAL;
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  else
   return 1;
 }

SQLRETURN sql_row_count(OTL_SQLLEN* total_rpc)
{
#if defined(OTL_ODBC_ALTERNATE_RPC)
  OTL_SQLLEN rpc=0;
  SQLRETURN rc;
  do{
    rc=SQLRowCount(cda,&rpc);
    if(rc!=SQL_SUCCESS)
      return rc;
    *total_rpc+=rpc;
    rc=SQLMoreResults(cda);
  }while(rc==SQL_SUCCESS);
  return SQL_SUCCESS;
#else
  return SQLRowCount(cda,total_rpc);
#endif
}


 int parse(char* stm_text)
 {
#if !defined(OTL_ODBC_TIMESTEN)
   short in_str=0;
#endif
   bool do_not_call_sql_row_count=false;
#if defined(OTL_ODBC_SQL_STATEMENT_WITH_DIAG_REC_OUTPUT)
   if(OTL_ODBC_SQL_STATEMENT_WITH_DIAG_REC_OUTPUT (stm_text)){
     do_not_call_sql_row_count=true;
     direct_exec_flag=1;
   }
#endif
  char *c=stm_text;
  if(*c=='$'){
    ++c;
    _rpc=0;
    direct_exec_flag=1;
    const int ctl_arr_size=6;
    struct{
      OTL_SQLCHAR_PTR name_ptr;
      OTL_SQLSMALLINT name_len;
      OTL_SQLCHAR name[512];
    } ctl_arr[ctl_arr_size];
#if (defined(UNICODE)||defined(_UNICODE))
    struct{
      SQLWCHAR name_ptr;
      OTL_SQLSMALLINT name_len;
      SQLWCHAR name[512];
    } ctl_arr_W[ctl_arr_size];
#endif
    int i=0;
    for(i=0;i<ctl_arr_size;++i){
      ctl_arr[i].name_ptr=nullptr;
      ctl_arr[i].name_len=0;
      ctl_arr[i].name[0]=0;
#if (defined(UNICODE)||defined(_UNICODE))
      ctl_arr_W[i].name_ptr=0;
      ctl_arr_W[i].name_len=0;
      ctl_arr_W[i].name[0]=0;
#endif
    }
    char func_name[256];
    int par_num=0;
    char par_val[512];
    size_t par_val_len=0;
    size_t fn_len=0;
    bool func_found=false;
    while(*c && *c!=' ' && fn_len<sizeof(func_name)){
      ++fn_len;
      func_name[fn_len-1]=*c;
      ++c;
    }
    if(fn_len<sizeof(func_name)-1){
      ++fn_len;
      func_name[fn_len-1]=0;
    }else
      func_name[sizeof(func_name)-1]=0;
    while(*c==' ')++c;
    while(*c){
      if(*c=='$'){
        ++c;
        par_num=OTL_SCAST(int,*c-'0')-1;
        ++c;
        while(*c && *c==' ')++c;
        if(*c==':' && ((c>stm_text && *(c-1)!='\\' )|| c==stm_text)){
          ++c;
          while(*c && *c==' ')++c;
          if(*c=='\''){
            par_val_len=0;
            ++c;
            while(*c && *c!='\'' && par_val_len<sizeof(par_val)){
              ++par_val_len;
              par_val[par_val_len-1]=*c;
              ++c;
            }
            if(par_val_len<sizeof(par_val)-1){
              ++par_val_len;
              par_val[par_val_len-1]=0;
            }else
              par_val[sizeof(par_val)-1]=0;
            if(par_num>=0 && par_num<ctl_arr_size){
              ctl_arr[par_num].name_ptr=ctl_arr[par_num].name;
              ctl_arr[par_num].name_len=SQL_NTS;
              OTL_STRCPY_S(OTL_RCAST(char*,ctl_arr[par_num].name),
                           sizeof(ctl_arr[par_num].name),
                           OTL_RCAST(const char*,par_val));
            }
          }
          ++c;
          while(*c && *c==' ')++c;
        }
      }else
        ++c;
    }
    status=SQL_SUCCESS;
    if(strcmp(func_name,"SQLTables")==0){
#if (defined(UNICODE)||defined(_UNICODE))
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[0].name,OTL_RCAST(unsigned char*,ctl_arr[0].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[1].name,OTL_RCAST(unsigned char*,ctl_arr[1].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[2].name,OTL_RCAST(unsigned char*,ctl_arr[2].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[3].name,OTL_RCAST(unsigned char*,ctl_arr[3].name));
#if !defined(OTL_UNICODE_USE_ANSI_ODBC_FUNCS_FOR_DATA_DICT)
      status=SQLTables
        (cda, 
         ctl_arr_W[0].name,SQL_NTS,
         ctl_arr_W[1].name,SQL_NTS,
         ctl_arr_W[2].name,SQL_NTS,
         ctl_arr_W[3].name,SQL_NTS);
#else
      status=SQLTablesA
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len);
#endif

#else
      status=SQLTables
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len);
#endif
      func_found=true;
    }else if(strcmp(func_name,"SQLStatistics")==0){
#if (defined(UNICODE)||defined(_UNICODE))
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[0].name,OTL_RCAST(unsigned char*,ctl_arr[0].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[1].name,OTL_RCAST(unsigned char*,ctl_arr[1].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[2].name,OTL_RCAST(unsigned char*,ctl_arr[2].name));
#if !defined(OTL_UNICODE_USE_ANSI_ODBC_FUNCS_FOR_DATA_DICT)
      status=SQLStatistics
        (cda, 
         ctl_arr_W[0].name,SQL_NTS,
         ctl_arr_W[1].name,SQL_NTS,
         ctl_arr_W[2].name,SQL_NTS,
         SQL_INDEX_ALL, SQL_QUICK);
#else
      status=SQLStatisticsA
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         SQL_INDEX_ALL, SQL_QUICK);
#endif

#else
      status=SQLStatistics
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         SQL_INDEX_ALL, SQL_QUICK);
#endif
      func_found=true;
    }else if(strcmp(func_name,"SQLGetTypeInfo")==0){
      status=SQLGetTypeInfo
        (cda, 
         SQL_ALL_TYPES);
      func_found=true;
    }else if(strcmp(func_name,"SQLColumns")==0){
#if (defined(UNICODE)||defined(_UNICODE))
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[0].name,OTL_RCAST(unsigned char*,ctl_arr[0].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[1].name,OTL_RCAST(unsigned char*,ctl_arr[1].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[2].name,OTL_RCAST(unsigned char*,ctl_arr[2].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[3].name,OTL_RCAST(unsigned char*,ctl_arr[3].name));

#if !defined(OTL_UNICODE_USE_ANSI_ODBC_FUNCS_FOR_DATA_DICT)
      status=SQLColumns
        (cda, 
         ctl_arr_W[0].name,SQL_NTS,
         ctl_arr_W[1].name,SQL_NTS,
         ctl_arr_W[2].name,SQL_NTS,
         ctl_arr_W[3].name,SQL_NTS);
#else
      status=SQLColumnsA
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len);
#endif

#else
      status=SQLColumns
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len);
#endif
      func_found=true;
    }else if(strcmp(func_name,"SQLProcedures")==0){
#if (defined(UNICODE)||defined(_UNICODE))
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[0].name,OTL_RCAST(unsigned char*,ctl_arr[0].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[1].name,OTL_RCAST(unsigned char*,ctl_arr[1].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[2].name,OTL_RCAST(unsigned char*,ctl_arr[2].name));

#if !defined(OTL_UNICODE_USE_ANSI_ODBC_FUNCS_FOR_DATA_DICT)
      status=SQLProcedures
        (cda, 
         ctl_arr_W[0].name,SQL_NTS,
         ctl_arr_W[1].name,SQL_NTS,
         ctl_arr_W[2].name,SQL_NTS);
#else
      status=SQLProceduresA
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len);
#endif

#else
      status=SQLProcedures
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len);
#endif
      func_found=true;
    }else if(strcmp(func_name,"SQLColumnPrivileges")==0){
#if (defined(UNICODE)||defined(_UNICODE))
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[0].name,OTL_RCAST(unsigned char*,ctl_arr[0].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[1].name,OTL_RCAST(unsigned char*,ctl_arr[1].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[2].name,OTL_RCAST(unsigned char*,ctl_arr[2].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[3].name,OTL_RCAST(unsigned char*,ctl_arr[3].name));
#if !defined(OTL_UNICODE_USE_ANSI_ODBC_FUNCS_FOR_DATA_DICT)
     status=SQLColumnPrivileges
       (cda, 
        ctl_arr_W[0].name,SQL_NTS,
        ctl_arr_W[1].name,SQL_NTS,
        ctl_arr_W[2].name,SQL_NTS,
        ctl_arr_W[3].name,SQL_NTS);
#else
     status=SQLColumnPrivilegesA
       (cda, 
        ctl_arr[0].name_ptr,ctl_arr[0].name_len,
        ctl_arr[1].name_ptr,ctl_arr[1].name_len,
        ctl_arr[2].name_ptr,ctl_arr[2].name_len,
        ctl_arr[3].name_ptr,ctl_arr[3].name_len);
#endif

#else
      status=SQLColumnPrivileges
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len);
#endif
      func_found=true;
    }else if(strcmp(func_name,"SQLTablePrivileges")==0){
#if (defined(UNICODE)||defined(_UNICODE))
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[0].name,OTL_RCAST(unsigned char*,ctl_arr[0].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[1].name,OTL_RCAST(unsigned char*,ctl_arr[1].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[2].name,OTL_RCAST(unsigned char*,ctl_arr[2].name));

#if !defined(OTL_UNICODE_USE_ANSI_ODBC_FUNCS_FOR_DATA_DICT)
      status=SQLTablePrivileges
        (cda, 
         ctl_arr_W[0].name,SQL_NTS,
         ctl_arr_W[1].name,SQL_NTS,
         ctl_arr_W[2].name,SQL_NTS);
#else
      status=SQLTablePrivilegesA
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len);
#endif

#else
      status=SQLTablePrivileges
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len);
#endif
      func_found=true;
    }else if(strcmp(func_name,"SQLPrimaryKeys")==0){
#if (defined(UNICODE)||defined(_UNICODE))
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[0].name,OTL_RCAST(unsigned char*,ctl_arr[0].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[1].name,OTL_RCAST(unsigned char*,ctl_arr[1].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[2].name,OTL_RCAST(unsigned char*,ctl_arr[2].name));

#if !defined(OTL_UNICODE_USE_ANSI_ODBC_FUNCS_FOR_DATA_DICT)
      status=SQLPrimaryKeys
        (cda, 
         ctl_arr_W[0].name,SQL_NTS,
         ctl_arr_W[1].name,SQL_NTS,
         ctl_arr_W[2].name,SQL_NTS);
#else
      status=SQLPrimaryKeysA
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len);
#endif

#else
      status=SQLPrimaryKeys
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len);
#endif
      func_found=true;
    }else if(strcmp(func_name,"SQLProcedureColumns")==0){
#if (defined(UNICODE)||defined(_UNICODE))
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[0].name,OTL_RCAST(unsigned char*,ctl_arr[0].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[1].name,OTL_RCAST(unsigned char*,ctl_arr[1].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[2].name,OTL_RCAST(unsigned char*,ctl_arr[2].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[3].name,OTL_RCAST(unsigned char*,ctl_arr[3].name));

#if !defined(OTL_UNICODE_USE_ANSI_ODBC_FUNCS_FOR_DATA_DICT)
      status=SQLProcedureColumns
        (cda, 
         ctl_arr_W[0].name,SQL_NTS,
         ctl_arr_W[1].name,SQL_NTS,
         ctl_arr_W[2].name,SQL_NTS,
         ctl_arr_W[3].name,SQL_NTS);
#else
      status=SQLProcedureColumnsA
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len);
#endif

#else
      status=SQLProcedureColumns
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len);
#endif
      func_found=true;
    }else if(strcmp(func_name,"SQLForeignKeys")==0){
#if (defined(UNICODE)||defined(_UNICODE))
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[0].name,OTL_RCAST(unsigned char*,ctl_arr[0].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[1].name,OTL_RCAST(unsigned char*,ctl_arr[1].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[2].name,OTL_RCAST(unsigned char*,ctl_arr[2].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[3].name,OTL_RCAST(unsigned char*,ctl_arr[3].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[4].name,OTL_RCAST(unsigned char*,ctl_arr[4].name));
      otl_convert_char_to_SQLWCHAR
        (ctl_arr_W[5].name,OTL_RCAST(unsigned char*,ctl_arr[5].name));

#if !defined(OTL_UNICODE_USE_ANSI_ODBC_FUNCS_FOR_DATA_DICT)
      status=SQLForeignKeys
        (cda, 
         ctl_arr_W[0].name,SQL_NTS,
         ctl_arr_W[1].name,SQL_NTS,
         ctl_arr_W[2].name,SQL_NTS,
         ctl_arr_W[3].name,SQL_NTS,
         ctl_arr_W[4].name,SQL_NTS,
         ctl_arr_W[5].name,SQL_NTS);
#else
      status=SQLForeignKeysA
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len,
         ctl_arr[4].name_ptr,ctl_arr[4].name_len,
         ctl_arr[5].name_ptr,ctl_arr[5].name_len);
#endif

#else
      status=SQLForeignKeys
        (cda, 
         ctl_arr[0].name_ptr,ctl_arr[0].name_len,
         ctl_arr[1].name_ptr,ctl_arr[1].name_len,
         ctl_arr[2].name_ptr,ctl_arr[2].name_len,
         ctl_arr[3].name_ptr,ctl_arr[3].name_len,
         ctl_arr[4].name_ptr,ctl_arr[4].name_len,
         ctl_arr[5].name_ptr,ctl_arr[5].name_len);
#endif
      func_found=true;
    }
    if(!func_found)return 2;
    if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
      return 0;
    else
      return 1;
  }

  if(direct_exec_flag){
   _rpc=0;
#if (defined(UNICODE)||defined(_UNICODE))
 {
   SQLWCHAR* temp_stm_text=new SQLWCHAR[strlen(stm_text)+1];
   otl_convert_char_to_SQLWCHAR(temp_stm_text,OTL_RCAST(unsigned char*,stm_text));
   status=SQLExecDirect
    (cda,
     temp_stm_text,
     SQL_NTS);
   delete[] temp_stm_text;
 }
#else
   status=SQLExecDirect
    (cda,
     OTL_RCAST(OTL_SQLCHAR_PTR,stm_text),
     SQL_NTS);
#endif

#if defined(OTL_THROWS_ON_SQL_SUCCESS_WITH_INFO)
   if(adb && adb->throws_on_sql_success_with_info &&
      status==SQL_SUCCESS_WITH_INFO)
     return 0;
#endif

   if(status!=SQL_SUCCESS&&
      status!=SQL_SUCCESS_WITH_INFO&&
#if (ODBCVER >= 0x0300)
      status!=SQL_NO_DATA
#else
      status!=SQL_NO_DATA_FOUND
#endif
     )
    return 0;
   else{
     _rpc=0;
     if(!do_not_call_sql_row_count){
       OTL_SQLLEN tmp_rpc=0;
       SQLRETURN diag_status=sql_row_count(&tmp_rpc);
       if(diag_status==SQL_SUCCESS||
          diag_status==SQL_SUCCESS_WITH_INFO)
         _rpc=OTL_SCAST(long,tmp_rpc);
     }
    return 1;
   }
  }

#if !defined(OTL_ODBC_TIMESTEN)
  // Converting : notation into ODBC's native notation ?
  while(*c){
   if(*c=='\''){
    if(!in_str)
     in_str=1;
    else{
     if(c[1]=='\'')
      ++c;
     else
      in_str=0;
    }
   }
   if(*c==':' && !in_str && 
      ((c>stm_text && *(c-1)!='\\' )|| c==stm_text)){
    *c='?';
    ++c;
    while(isalnum(OTL_SCAST(unsigned char,*c))||*c=='_'){
     *c=' ';
     ++c;
    }
   }else if(*c==':' && !in_str && 
            ((c>stm_text && *(c-1)=='\\' )|| c==stm_text)){
     char* c_1=c-1;
     char* c_=c;
     while(*c_){
       *c_1=*c_;
       ++c_1;
       ++c_;
     }
     if(c_1>c-1)
       *c_1=0;
     --c;
   }
   ++c;
  }
#endif

#if defined(OTL_DB2_CLI)

  OTL_SQLINTEGER temp_isolation_level=0;
  status=SQLGetStmtAttr
   (cda,
    SQL_ATTR_TXN_ISOLATION,
    OTL_RCAST(SQLPOINTER,&temp_isolation_level),
    SQL_IS_POINTER,
    nullptr);
  if(OTL_SCAST(long,temp_isolation_level)==otl_tran_read_committed||
     OTL_SCAST(long,temp_isolation_level)==otl_tran_read_uncommitted){
    status=SQLSetStmtAttr
      (cda,
       SQL_ATTR_CLOSE_BEHAVIOR,
       OTL_RCAST(void*,SQL_CC_RELEASE),
       SQL_NTS);
    if(status!=SQL_SUCCESS&&
       status!=SQL_SUCCESS_WITH_INFO)
      return 0;
  }
#endif 
  
#if (defined(UNICODE)||defined(_UNICODE))
 {
   SQLWCHAR* temp_stm_text=new SQLWCHAR[strlen(stm_text)+1];
   otl_convert_char_to_SQLWCHAR(temp_stm_text,OTL_RCAST(unsigned char*,stm_text));
   status=SQLPrepare
     (cda,
      temp_stm_text,
      SQL_NTS);
   delete[] temp_stm_text;
 }
#else
 status=SQLPrepare
  (cda,
   OTL_RCAST(OTL_SQLCHAR_PTR,stm_text),
   SQL_NTS);
#endif

 if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
  return 0;
 else
  return 1;
 }

 int exec(const int iters, 
          const int /*rowoff*/,
#if defined(OTL_ODBC_ALTERNATE_RPC)
          const int otl_sql_exec_from_class)
#else
          const int /*otl_sql_exec_from_class*/)
#endif
 {
#if (ODBCVER >= 0x0300)
#else
#if defined(OTL_ODBC_TIMESTEN_WIN) && defined(_WIN64)
  OTL_SQLULEN irows;
#else
#if defined(OTL_ODBC_UNIX)
#if !defined(BUILD_LEGACY_64_BIT_MODE) && defined(SIZEOF_LONG_INT) && (SIZEOF_LONG_INT==8)
  OTL_SQLULEN irows;
#else
  OTL_SQLUINTEGER irows;
#endif
#else
  OTL_SQLUINTEGER irows;
#endif
#endif
#endif
  if(direct_exec_flag){
   return 1;
  }else{
#if !defined(OTL_ODBC_MYSQL) && !defined(OTL_ODBC_XTG_IBASE6)
#if (ODBCVER >= 0x0300)
   if(last_iters>1||iters>1||_rpc>1){
     last_iters=iters;
     size_t temp_iters=OTL_SCAST(size_t,iters);
     status=SQLSetStmtAttr
       (cda,
        SQL_ATTR_PARAMSET_SIZE,
        OTL_RCAST(void*,temp_iters),
        SQL_NTS);
     if(status!=SQL_SUCCESS&&
        status!=SQL_SUCCESS_WITH_INFO)
       return 0;
   }
#else
   if(last_iters>1||iters>1||_rpc>1){
     last_iters=iters;
     status=SQLParamOptions
       (cda,
#if defined(OTL_ODBC_UNIX)
#if !defined(BUILD_LEGACY_64_BIT_MODE) && defined(SIZEOF_LONG_INT) && (SIZEOF_LONG_INT==8)
        OTL_SCAST(OTL_SQLULEN,iters),
#else
        OTL_SCAST(OTL_SQLUINTEGER,iters),
#endif
#else
        OTL_SCAST(OTL_SQLUINTEGER,iters),
#endif
        &irows); 
     if(status!=SQL_SUCCESS&&
        status!=SQL_SUCCESS_WITH_INFO)
       return 0;
   }
#endif
#endif
   _rpc=0;

   last_param_data_token=0;
   last_sql_param_data_status=0;
   sql_param_data_count=0;
   
   status=SQLExecute(cda);
   if(canceled)return 0;
#if defined(OTL_THROWS_ON_SQL_SUCCESS_WITH_INFO)
   if(adb && adb->throws_on_sql_success_with_info && 
      status==SQL_SUCCESS_WITH_INFO)
     return 0;
#endif
   if(status!=SQL_SUCCESS&&
      status!=SQL_SUCCESS_WITH_INFO&&
#if (ODBCVER >= 0x0300)
      status!=SQL_NO_DATA&&
#else
      status!=SQL_NO_DATA_FOUND&&
#endif
      status!=SQL_NEED_DATA)return 0;
   if(status==SQL_NEED_DATA){
    _rpc=iters;
    return 1;
   }
   OTL_SQLLEN tmp_rpc=0;
   SQLRETURN diag_status=0;
#if defined(OTL_ODBC_ALTERNATE_RPC)
   if(otl_sql_exec_from_class==otl_sql_exec_from_cursor_class){
     diag_status=sql_row_count(&tmp_rpc);
     if(diag_status==SQL_SUCCESS||diag_status==SQL_SUCCESS_WITH_INFO)
       _rpc=OTL_SCAST(long,tmp_rpc);
     return 1;
   }else{
     _rpc=0;
     return 1;
   }
#else
   diag_status=sql_row_count(&tmp_rpc);
   if(diag_status==SQL_SUCCESS||diag_status==SQL_SUCCESS_WITH_INFO)
     _rpc=OTL_SCAST(long,tmp_rpc);
   return 1;
#endif
  }
 }

 long get_rpc() OTL_NO_THROW
 {
  return _rpc;
 }

 int tmpl_ftype2odbc_ftype(const int ftype)
 {
  switch(ftype){
#if defined(OTL_UNICODE)
  case otl_var_char:
   return SQL_C_WCHAR;
  case otl_var_varchar_long:
   return SQL_WLONGVARCHAR;
#else
  case otl_var_char:
   return SQL_C_CHAR;
  case otl_var_varchar_long:
   return SQL_LONGVARCHAR;
#endif
  case otl_var_double:
   return SQL_C_DOUBLE;
#if defined(OTL_BIGINT)
  case otl_var_bigint:
   return SQL_C_SBIGINT;
#endif
#if defined(OTL_UBIGINT)
  case otl_var_ubigint:
   return SQL_C_UBIGINT;
#endif
  case otl_var_float:
   return SQL_C_FLOAT;
  case otl_var_int:
    return SQL_C_SLONG;
  case otl_var_long_int:
#if defined(OTL_MAP_LONG_TO_SQL_C_SBIGINT) && \
    ((ODBCVER >= 0x0300) || defined(OTL_ODBC_TIMESTEN))
  {
    static bool long_is_8_bytes=sizeof(long)==8;
    if(long_is_8_bytes)
      return SQL_C_SBIGINT;
    else
      return SQL_C_SLONG;
  }
#else
  return SQL_C_SLONG;
#endif
  case otl_var_unsigned_int:
   return SQL_C_ULONG;
  case otl_var_short:
   return SQL_C_SSHORT;
  case otl_var_timestamp:
  case otl_var_db2time:
  case otl_var_db2date:
   return SQL_C_TIMESTAMP;
  case otl_var_raw_long:
   return SQL_LONGVARBINARY;
  case otl_var_raw:
   return SQL_C_BINARY;
  default:
   return 0;
  }
 }

 int otl_map_ext2int(int ftype)
 {
  switch(ftype){
#if defined(OTL_UNICODE)
  case SQL_WLONGVARCHAR: return SQL_WLONGVARCHAR;
  case SQL_C_WCHAR: return SQL_WVARCHAR;
#else
  case SQL_LONGVARCHAR: return SQL_LONGVARCHAR;
  case SQL_C_CHAR: return SQL_VARCHAR;
#endif
  case SQL_LONGVARBINARY: return SQL_LONGVARBINARY;
  case SQL_C_DATE: return SQL_DATE;
#if (ODBCVER >= 0x0300)
  case SQL_C_TIME: return SQL_TYPE_TIME;
  case SQL_C_TIMESTAMP: return SQL_TYPE_TIMESTAMP;
#else
  case SQL_C_TIME: return SQL_TIME;
  case SQL_C_TIMESTAMP: return SQL_TIMESTAMP;
#endif
  case SQL_C_DOUBLE: return SQL_DOUBLE;
#if defined(OTL_BIGINT)
  case SQL_C_SBIGINT: return SQL_BIGINT;
#endif
#if defined(OTL_UBIGINT)
  case SQL_C_UBIGINT: return SQL_BIGINT;
#endif
  case SQL_C_FLOAT: return SQL_FLOAT;
  case SQL_C_SLONG: return SQL_INTEGER;
  case SQL_C_SSHORT: return SQL_SMALLINT;
  case SQL_C_ULONG: return SQL_DOUBLE;
  case SQL_C_BINARY: return SQL_VARBINARY;
  default: return -1;
  }
 }

 int bind
 (const char* /* name */,
  otl_var& v,
  const int aelem_size,
  const int aftype,
  const int aparam_type,
  const int name_pos,
  const int 
#if !defined(OTL_ODBC_TIMESTEN) && defined(OTL_ODBC_MULTI_MODE)
  connection_type
#endif
  ,
  const int /* apl_tab_size */)
 {OTL_SQLSMALLINT ftype=OTL_SCAST(OTL_SQLSMALLINT,tmpl_ftype2odbc_ftype(aftype));
  OTL_SQLSMALLINT ftype_save=ftype;
  int param_type;
  int parm_pos=name_pos;
  v.vparam_type=aparam_type;
  switch(aparam_type){
  case otl_input_param:
   param_type=SQL_PARAM_INPUT;
   break;
  case otl_output_param:
   param_type=SQL_PARAM_OUTPUT;
   break;
  case otl_inout_param:
   param_type=SQL_PARAM_INPUT_OUTPUT;
   break;
  default:
   param_type=SQL_PARAM_INPUT;
   break;
  }
#if defined(OTL_UNICODE)
  if(ftype==SQL_WLONGVARCHAR){
   ftype=SQL_C_WCHAR;
#else
  if(ftype==SQL_LONGVARCHAR){
   ftype=SQL_C_CHAR;
#endif
  }else if(ftype==SQL_LONGVARBINARY){
   ftype=SQL_C_BINARY;
  }
  int sqltype=otl_map_ext2int(ftype_save);
  int mapped_sqltype=sqltype;

  if(aftype==otl_var_db2date)
#if (ODBCVER >= 0x0300)
   mapped_sqltype=SQL_TYPE_DATE;
#else
   mapped_sqltype=SQL_DATE;
#endif
  else if(aftype==otl_var_db2time)
#if (ODBCVER >= 0x0300)
   mapped_sqltype=SQL_TYPE_TIME;
#else
   mapped_sqltype=SQL_TIME;
#endif
  if(v.lob_stream_mode&&
     (ftype_save==SQL_LONGVARBINARY||
#if defined(OTL_UNICODE)
      ftype_save==SQL_WLONGVARCHAR)){
#else
      ftype_save==SQL_LONGVARCHAR)){
#endif
   // in case of "stream mode" the variable
   // gets bound in a special way
#if defined(OTL_ODBC_MSSQL_2005) && !defined(OTL_ODBC_MULTI_MODE)
    switch(ftype_save){
    case SQL_LONGVARBINARY:
      mapped_sqltype=SQL_VARBINARY;
      break;
#if defined(OTL_UNICODE)
    case SQL_WLONGVARCHAR:
      mapped_sqltype=SQL_WVARCHAR;
      break;
#else
    case SQL_LONGVARCHAR:
      mapped_sqltype=SQL_VARCHAR;
      break;
#endif
    }
#elif defined(OTL_ODBC_MULTI_MODE)
    if(connection_type==OTL_MSSQL_2005_ODBC_CONNECT||
       connection_type==OTL_MSSQL_2008_ODBC_CONNECT){
      switch(ftype_save){
      case SQL_LONGVARBINARY:
        mapped_sqltype=SQL_VARBINARY;
        break;
#if defined(OTL_UNICODE)
      case SQL_WLONGVARCHAR:
        mapped_sqltype=SQL_WVARCHAR;
        break;
#else
      case SQL_LONGVARCHAR:
        mapped_sqltype=SQL_VARCHAR;
        break;
#endif
      }
    }
#endif
    int temp_int_val=
#if (ODBCVER >= 0x0300)
     sqltype==SQL_TYPE_TIMESTAMP ? 
#if defined(OTL_ODBC_MULTI_MODE)
     ((connection_type==OTL_MSSQL_2008_ODBC_CONNECT)? 7 : 
      (connection_type==OTL_MSSQL_2005_ODBC_CONNECT)? 3 :
      otl_odbc_date_scale) : 0;
#else
     otl_odbc_date_scale : 0;
#endif
#else
     sqltype==SQL_TIMESTAMP?otl_odbc_date_scale:0;
#endif
     short int temp_val=OTL_SCAST(OTL_SQLSMALLINT,temp_int_val);
   status=SQLBindParameter
    (cda,                                             
     OTL_SCAST(OTL_SQLUSMALLINT,parm_pos),            
     OTL_SCAST(OTL_SQLSMALLINT,param_type),           
     ftype,                                           
     OTL_SCAST(OTL_SQLSMALLINT,mapped_sqltype),
#if (ODBCVER >= 0x0300)

#if defined(OTL_ODBC_MSSQL_2005) && !defined(OTL_ODBC_MULTI_MODE)
     0,
#elif defined(OTL_ODBC_MULTI_MODE)
     (connection_type==OTL_MSSQL_2005_ODBC_CONNECT||
      connection_type==OTL_MSSQL_2008_ODBC_CONNECT) ? 0 :
     (sqltype==SQL_TYPE_TIMESTAMP?otl_odbc_date_prec:aelem_size),
#else
     sqltype==SQL_TYPE_TIMESTAMP?otl_odbc_date_prec:aelem_size,
#endif
#else
     sqltype==SQL_TIMESTAMP?otl_odbc_date_prec:aelem_size,
#endif
     temp_val,
     OTL_RCAST(OTL_SQLPOINTER,OTL_SCAST(size_t,parm_pos)),
     0,                     
     v.p_len);                                        
  }else{
    int temp_column_size=0;
#if (ODBCVER >= 0x0300)
    if(sqltype==SQL_TYPE_TIMESTAMP)
      temp_column_size=otl_odbc_date_prec;
#if defined(OTL_UNICODE)
    else if(ftype==SQL_C_WCHAR)
      temp_column_size=(aelem_size-1);
#else
    else if(ftype==SQL_C_CHAR)
      temp_column_size=aelem_size-1;
#endif
    else
      temp_column_size=aelem_size;
#else
    if(sqltype==SQL_TIMESTAMP)
      temp_column_size=otl_odbc_date_prec;
    else if(ftype==SQL_C_CHAR)
      temp_column_size=aelem_size-1;
    else
      temp_column_size=aelem_size;
#endif
    OTL_SQLINTEGER buflen=0;
#if defined(OTL_UNICODE)
    if(ftype==SQL_C_WCHAR)
      buflen=aelem_size*sizeof(OTL_CHAR);
    else
#endif
      buflen=aelem_size;
    int temp_int_val2=
#if (ODBCVER >= 0x0300)
     sqltype==SQL_TYPE_TIMESTAMP ?
#if defined(OTL_ODBC_MULTI_MODE)
     ((connection_type==OTL_MSSQL_2008_ODBC_CONNECT)? 7 : 
      (connection_type==OTL_MSSQL_2005_ODBC_CONNECT)? 3 :
      otl_odbc_date_scale) : 0;
#else
    otl_odbc_date_scale : 0;
#endif
#else
    sqltype==SQL_TIMESTAMP?otl_odbc_date_scale:0;
#endif
    short int temp_val2=OTL_SCAST(OTL_SQLSMALLINT,temp_int_val2);
    status=SQLBindParameter
    (cda,
     OTL_SCAST(OTL_SQLUSMALLINT,parm_pos),
     OTL_SCAST(OTL_SQLSMALLINT,param_type),
     ftype,
     OTL_SCAST(OTL_SQLSMALLINT,mapped_sqltype),
     temp_column_size,
     temp_val2,
     OTL_RCAST(OTL_SQLPOINTER,v.p_v),
     buflen,
     v.p_len);
  }
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  else
   return 1;
 }

  int bind
  (const int column_num,
   otl_var& v,
   const int elem_size,
   const int aftype,
   const int param_type)
  {SWORD ftype=OTL_SCAST(SWORD,tmpl_ftype2odbc_ftype(aftype));
  v.vparam_type=param_type;
  SWORD ftype_save=ftype;
#if defined(OTL_UNICODE)
  if(ftype==SQL_WLONGVARCHAR){
   ftype=SQL_C_WCHAR;
#else
  if(ftype==SQL_LONGVARCHAR){
   ftype=SQL_C_CHAR;
#endif
  }else if(ftype==SQL_LONGVARBINARY){
   ftype=SQL_C_BINARY;
  }
  if(v.lob_stream_mode&&
     (ftype_save==SQL_LONGVARBINARY||
#if defined(OTL_UNICODE)
      ftype_save==SQL_WLONGVARCHAR)){
#else
      ftype_save==SQL_LONGVARCHAR)){
#endif
   // in case of "stream mode" the variable
   // remains unbound
   v.lob_ftype=ftype;
   v.lob_pos=column_num;
   return 1;
  }else{
    SQLINTEGER buflen=elem_size;
#if defined(OTL_UNICODE)
    if(ftype==SQL_C_WCHAR||ftype==SQL_WLONGVARCHAR)
      buflen=elem_size*sizeof(OTL_CHAR); 
#endif
   status=SQLBindCol
    (cda,
     OTL_SCAST(unsigned short,column_num),
     ftype,
     OTL_RCAST(PTR,v.p_v),
     buflen,
     &v.p_len[0]);
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)
    return 0;
   else
    return 1;
  }

 }

 int describe_column
 (otl_column_desc& col,
  const int column_num,
  int& eof_desc)
 {
  OTL_SQLCHAR name[256];
  OTL_SQLSMALLINT nlen;
  OTL_SQLSMALLINT dbtype;
  OTL_SQLLEN dbsize;
  OTL_SQLSMALLINT scale;
  OTL_SQLULEN prec;
  OTL_SQLSMALLINT nullok;
  OTL_SQLSMALLINT icols;

  eof_desc=0;
  status=SQLNumResultCols(cda,&icols);
  if(status!=SQL_SUCCESS&&
     status!=SQL_SUCCESS_WITH_INFO)
   return 0;
  if(column_num>icols){
   eof_desc=1;
   return 0;
  }
#if (defined(UNICODE)||defined(_UNICODE))
 {
   SQLWCHAR temp_name[256];
   status=SQLDescribeCol
     (cda,
      OTL_SCAST(unsigned short,column_num),
      temp_name,
      sizeof(temp_name)/sizeof(SQLWCHAR),
      &nlen,
      &dbtype,
      &prec,
      &scale,
      &nullok);
   otl_convert_SQLWCHAR_to_char(OTL_RCAST(unsigned char*,name),temp_name);
 }
#else
  status=SQLDescribeCol
   (cda,
    OTL_SCAST(unsigned short,column_num),
    name,
    OTL_SCAST(SQLSMALLINT,sizeof(name)),
    &nlen,
    &dbtype,
    &prec,
    &scale,
    &nullok);
#endif
  if(!(status == SQL_SUCCESS || 
       status == SQL_SUCCESS_WITH_INFO))
    return 0;
  dbsize=prec;
  col.set_name(OTL_RCAST(char*,name));

#if defined(OTL_DB2_CLI) && defined(OTL_DB2_CLI_MAP_LONG_VARCHAR_TO_VARCHAR)
#if defined(OTL_UNICODE)
#error OTL_DB2_CLI_MAP_LONG_VARCHAR_TO_VARCHAR is not supported when \
OTL_UNICODE is defined
#else
  if(dbtype==SQL_LONGVARCHAR && 
     dbsize <= OTL_DB2_CLI_MAP_LONG_VARCHAR_TO_VARCHAR){
    dbtype=SQL_VARCHAR;
  }
#endif
#endif

  col.dbtype=dbtype;
  col.dbsize=dbsize;
  col.scale=scale;
  col.prec=prec;
  col.nullok=nullok;
  return 1;
 }

 void error(otl_exc& exception_struct)
 {OTL_SQLRETURN rc;
  OTL_SQLSMALLINT msg_len=0;
#if (ODBCVER >= 0x0300)

#if (defined(UNICODE)||defined(_UNICODE))
 {
#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON)
   rc=SQLGetDiagRec
     (SQL_HANDLE_STMT,
      cda,
      1,
      &exception_struct.sqlstate[0],
      OTL_RCAST(OTL_SQLINTEGER_PTR,&exception_struct.code),
      &exception_struct.msg[0],
      SQL_MAX_MESSAGE_LENGTH-1,
      OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
   exception_struct.msg[msg_len]=0;
#else
   SQLWCHAR temp_sqlstate[1000];
   SQLWCHAR temp_msg[SQL_MAX_MESSAGE_LENGTH];
   rc=SQLGetDiagRec
     (SQL_HANDLE_STMT,
      cda,
      1,
      temp_sqlstate,
      OTL_RCAST(OTL_SQLINTEGER_PTR,&exception_struct.code),
      temp_msg,
      SQL_MAX_MESSAGE_LENGTH-1,
      OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
   temp_msg[msg_len]=0;
   otl_convert_SQLWCHAR_to_char
     (OTL_RCAST(unsigned char*,&exception_struct.sqlstate[0]),
      temp_sqlstate);
   otl_convert_SQLWCHAR_to_char
     (OTL_RCAST(unsigned char*,&exception_struct.msg[0]),
      temp_msg);
#endif
 }
#else
 void* temp_ptr=&exception_struct.code;
 rc=SQLGetDiagRec
   (SQL_HANDLE_STMT,
    cda,
    1,
    OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.sqlstate[0]),
    OTL_RCAST(OTL_SQLINTEGER_PTR,temp_ptr),
    OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.msg[0]),
    SQL_MAX_MESSAGE_LENGTH-1,
    OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
#endif

#else
 rc=SQLError(adb->henv, 
             adb->hdbc, 
             cda,
             OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.sqlstate[0]),
             OTL_RCAST(OTL_SQLINTEGER_PTR,&exception_struct.code),
             OTL_RCAST(OTL_SQLCHAR_PTR,&exception_struct.msg[0]),
             SQL_MAX_MESSAGE_LENGTH-1,
             OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
#endif

  exception_struct.msg[msg_len]=0;

  if(rc==SQL_INVALID_HANDLE||rc==SQL_ERROR)
    exception_struct.msg[0]=0;    
#if (ODBCVER >= 0x0300)
#if defined(OTL_EXTENDED_EXCEPTION)
  else if(rc!=SQL_NO_DATA)
    otl_fill_exception(exception_struct,cda,SQL_HANDLE_STMT);
#endif
#endif
 }
private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
     public:
 otl_cur(const otl_cur&) = delete;
 otl_cur& operator=(const otl_cur&) = delete;
     private:
#else
  otl_cur(const otl_cur&):
    otl_cur0(),
    status(0),
    adb(nullptr),
    direct_exec_flag(0),
    _rpc(0),
    canceled(false),
    last_iters(0)
 {
 }

 otl_cur& operator=(const otl_cur&)
 {
   return *this;
 }
#endif

};

class otl_sel{
public:

#if defined(OTL_ODBC_USES_SQL_FETCH_SCROLL_WHEN_SPECIFIED_IN_OTL_CONNECT)
  void set_fetch_scroll_flag()
  {
    use_fetch_scroll_=true;
  }
#endif
private:

#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON)
  bool use_fetch_scroll_;
#endif

  int implicit_cursor;
  int status;
  int prefetch_array_size;

#if defined(OTL_ODBC_UNIX)
#if defined(SIZEOF_LONG)
#if (SIZEOF_LONG==8)
#if !defined(BUILD_REAL_64_BIT_MODE)
  OTL_SQLULEN crow;
#else
  OTL_SQLUINTEGER crow;
#endif
#else // (SIZEOF_LONG==8)
  OTL_SQLULEN crow;
#endif
#else // defined(SIZEOF_LONG)
  OTL_SQLULEN crow;
#endif
#else // defined(OTL_ODBC_UNIX)
  OTL_SQLULEN crow;
#endif

  int in_sequence;
#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON) || (ODBCVER<0x0300)
  OTL_SQLUSMALLINT* row_status;
  int row_status_arr_size;
#endif

public:

 int get_implicit_cursor() const {return implicit_cursor;}

 void set_arr_size
 (const int input_arr_size,
  int& out_array_size,
  int& out_prefetch_array_size)
 {
#if defined(OTL_ODBC_TIMESTEN)
   out_array_size=1;
   out_prefetch_array_size=input_arr_size;
#else
   out_array_size=input_arr_size;
   out_prefetch_array_size=0;
#endif
 }
 
 int close_select(otl_cur& cur)
 {
  if(!in_sequence)return 1;
#if defined(OTL_DB2_CLI)
  status=SQLCloseCursor(cur.cda);
#else
  status=SQLFreeStmt(cur.cda,SQL_CLOSE);
#endif
  in_sequence=0;
  if(status==SQL_ERROR)
   return 0;
  else
  return 1;
 }

 otl_sel():
#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON)
   use_fetch_scroll_(false),
#endif
   implicit_cursor(0),
   status(0),
   prefetch_array_size(0),
   crow(0),
   in_sequence(0)
#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON) || (ODBCVER<0x0300)
   ,row_status(nullptr)
   ,row_status_arr_size(0)
#endif
 {
 }

  virtual ~otl_sel()
  {
#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON) || (ODBCVER<0x0300)
    if(row_status!=0){
      delete[] row_status;
      row_status=nullptr;
      row_status_arr_size=0;
    }
#endif
  }

#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON) || (ODBCVER<0x0300)
  void alloc_row_status(const int array_size)
  {
    if(row_status==nullptr){
      row_status=new OTL_SQLUSMALLINT[array_size];
      row_status_arr_size=array_size;
      memset(row_status,0,sizeof(OTL_SQLUSMALLINT)*array_size);
    }else if(row_status!=nullptr && array_size!=row_status_arr_size){
      delete[] row_status;
      row_status=new OTL_SQLUSMALLINT[array_size];
      row_status_arr_size=array_size;
      memset(row_status,0,sizeof(OTL_SQLUSMALLINT)*array_size);
    }
  }
#endif

 void set_select_type(const int atype)
 {
  implicit_cursor=atype;
 }

  void init(const int /* array_size */)
  {
  }

  void set_prefetch_size(const int aprefetch_array_size)
  {
    prefetch_array_size=aprefetch_array_size;
  }

 int first
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
  const int 
#if !defined(OTL_ODBC_XTG_IBASE6)
  array_size
#endif
 )
 {
#if defined(OTL_ODBC_XTG_IBASE6)

  cur_row=-1;
  eof_data=0;
  if(!implicit_cursor){
   status=SQLExecute(cur.cda);
   if(cur.canceled)return 0;
#if defined(OTL_THROWS_ON_SQL_SUCCESS_WITH_INFO)
   if(cur.adb && cur.adb->throws_on_sql_success_with_info &&
      status==SQL_SUCCESS_WITH_INFO)
     return 0;
#endif
   if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
  }
  crow=0;
  status=SQLFetch(cur.cda);
  if(cur.canceled)return 0;
  if(status==SQL_SUCCESS||status==SQL_SUCCESS_WITH_INFO){
   crow=1;
   in_sequence=1;
  }

#else

#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON)
  alloc_row_status(array_size);
#endif
  cur_row=-1;
  eof_data=0;
#if (ODBCVER >= 0x0300)
  status=SQLSetStmtAttr
   (cur.cda,
    SQL_ATTR_ROW_ARRAY_SIZE,
    OTL_RCAST(void*,OTL_SCAST(size_t,array_size)),
    SQL_NTS);
#else
#if defined(OTL_ODBC_TIMESTEN)
  status=SQLSetStmtOption(cur.cda,TT_PREFETCH_COUNT,prefetch_array_size); 
#else
 status=SQLSetStmtOption(cur.cda,SQL_ROWSET_SIZE,array_size); 
#endif
#endif
  if(cur.canceled)return 0;
  if(status!=SQL_SUCCESS&&
     status!=SQL_SUCCESS_WITH_INFO)
   return 0;
#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON)
#if (ODBCVER >= 0x0300)
  if(use_fetch_scroll_){
    status=SQLSetStmtAttr(cur.cda,SQL_ATTR_ROWS_FETCHED_PTR,&crow,SQL_NTS);
    if(cur.canceled)return 0;
    if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
  }
#endif
#else
#if (ODBCVER >= 0x0300)

  status=SQLSetStmtAttr(cur.cda,SQL_ATTR_ROWS_FETCHED_PTR,&crow,SQL_NTS);
  if(cur.canceled)return 0;
  if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;

#else
#endif
#endif
  if(!implicit_cursor){
    status=SQLExecute(cur.cda);
    if(cur.canceled)return 0;
#if defined(OTL_THROWS_ON_SQL_SUCCESS_WITH_INFO)
    if(cur.adb && cur.adb->throws_on_sql_success_with_info &&
      status==SQL_SUCCESS_WITH_INFO)
     return 0;
#endif
    if(status!=SQL_SUCCESS&&status!=SQL_SUCCESS_WITH_INFO)return 0;
  }
#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON)
  if(array_size==1){
   crow=0;
   status=SQLFetch(cur.cda);
   if(cur.canceled)return 0;
   if(status==SQL_SUCCESS||status==SQL_SUCCESS_WITH_INFO){
    crow=1;
    in_sequence=1;
   }
  }else{
    if(use_fetch_scroll_){
#if (ODBCVER >= 0x0300)
      status=SQLFetchScroll(cur.cda,SQL_FETCH_NEXT,1);
#endif
    }else{
      status=SQLExtendedFetch
        (cur.cda, 
         SQL_FETCH_NEXT,
         1,
         &crow, 
         row_status); 
    }
  }
#else
#if (ODBCVER >= 0x0300)
  status=SQLFetchScroll(cur.cda,SQL_FETCH_NEXT,1);
#else
  {
    alloc_row_status(array_size);
    status=SQLExtendedFetch
      (cur.cda, 
       SQL_FETCH_NEXT,
       1,
       &crow, 
       row_status); 
  }
#endif

#endif

#endif
  
  in_sequence=1;
  if(cur.canceled)return 0;
  if(status==SQL_ERROR||
     status==SQL_INVALID_HANDLE)
   return 0;
  if(status==SQL_NO_DATA_FOUND){
   eof_data=1;
   cur_row=-1;
   crow=0;
   row_count=0;
   cur_size=0;
#if defined(OTL_DB2_CLI)
  status=SQLCloseCursor(cur.cda);
#else
   status=SQLFreeStmt(cur.cda,SQL_CLOSE);
#endif
   in_sequence=0;
   if(status==SQL_ERROR)return 0;
   return 1;
  }
  row_count=OTL_SCAST(int,crow);
  cur_size=row_count;
  if(cur_size!=0)cur_row=0;
  return 1;
 }

#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON)
 int next
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
  const int array_size)
 {
   alloc_row_status(array_size);
#else
 int next
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
#if (ODBCVER >= 0x0300)
  const int /* array_size */)
#else
  const int array_size)
#endif
 {
#endif
  if(cur_row<cur_size-1){
   ++cur_row;
   return 1;
  }else{
   if(eof_data){
    cur_row=-1;
    cur_size=0;
    in_sequence=0;
#if defined(OTL_DB2_CLI)
    status=SQLCloseCursor(cur.cda);
#else
    status=SQLFreeStmt(cur.cda,SQL_CLOSE);
#endif
    if(status==SQL_ERROR)return 0;
    return 1;
   }
#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON)
  if(array_size==1){
   crow=0;
   status=SQLFetch(cur.cda);
   if(cur.canceled)return 0;
   if(status==SQL_SUCCESS||status==SQL_SUCCESS_WITH_INFO){
    crow=1;
    in_sequence=1;
   }
  }else{
    if(use_fetch_scroll_){
#if (ODBCVER >= 0x0300)
      status=SQLFetchScroll(cur.cda,SQL_FETCH_NEXT,1);
#endif
    }else{
      status=SQLExtendedFetch
        (cur.cda, 
         SQL_FETCH_NEXT,
         1,
         &crow, 
         row_status); 
    }
 }
#else
#if (ODBCVER >= 0x0300)
  status=SQLFetchScroll(cur.cda,SQL_FETCH_NEXT,1);
#else
  {
    alloc_row_status(array_size);
    status=SQLExtendedFetch
      (cur.cda, 
       SQL_FETCH_NEXT,
       1,
       &crow, 
       row_status); 
  }
#endif
#endif
   in_sequence=1;
   if(cur.canceled)return 0;
   if(status==SQL_ERROR||
//    status==SQL_SUCCESS_WITH_INFO||
      status==SQL_INVALID_HANDLE)
    return 0;
   if(status==SQL_NO_DATA_FOUND){
    eof_data=1;
    cur_row=-1;
    cur_size=0;
    in_sequence=0;
#if defined(OTL_DB2_CLI)
    status=SQLCloseCursor(cur.cda);
#else
    status=SQLFreeStmt(cur.cda,SQL_CLOSE);
#endif
    if(status==SQL_ERROR)return 0;
    return 1;
   }
   cur_size=OTL_SCAST(int,crow);
   row_count+=OTL_SCAST(int,crow);
   if(cur_size!=0)cur_row=0;
   return 1;
  }
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_sel(const otl_sel&) = delete;
 otl_sel& operator=(const otl_sel&) = delete;
private:
#else
 otl_sel(const otl_sel&):
#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON)
   use_fetch_scroll_(false),
#endif
   implicit_cursor(0),
   status(0),
   prefetch_array_size(0),
   crow(0),
   in_sequence(0)
#if defined(OTL_ODBC_SQL_EXTENDED_FETCH_ON) || (ODBCVER<0x0300)
   ,row_status(nullptr)
   ,row_status_arr_size(0)
#endif
 {
 }

 otl_sel& operator=(const otl_sel&)
 {
   return *this;
 }
#endif

};

typedef otl_tmpl_connect
  <otl_exc,
   otl_conn,
   otl_cur> otl_odbc_connect;

typedef otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> otl_cursor;

typedef otl_tmpl_exception
  <otl_exc,
   otl_conn,
   otl_cur> otl_exception;

typedef otl_tmpl_select_stream
 <otl_exc,
  otl_conn,
  otl_cur,
  otl_var,
  otl_sel,
  otl_time> otl_select_stream;

typedef otl_tmpl_inout_stream
 <otl_exc,
  otl_conn,
  otl_cur,
  otl_var,
  otl_time> otl_inout_stream;

class otl_stream;
class otl_connect: public otl_odbc_connect{
public:

  void set_connection_mode(const int connection_mode)
  {
    connect_struct.connection_type=connection_mode;
  }

#if defined(OTL_DB2_CLI)
  void set_prog_name(const char* prog_name)
  {
    retcode=connect_struct.set_prog_name(prog_name);
    if(!retcode){
      increment_throw_count();
      if(get_throw_count()>1)return;
      if(otl_uncaught_exception()) return; 
      throw otl_exception(connect_struct);
   }
  } 
#endif

  int get_connection_mode(void)
  {
    return connect_struct.connection_type;
  }

protected:

  friend class otl_stream;

#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
  otl_stream_pool sc;
  bool pool_enabled_;
#endif  

public:

#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)

  void set_stream_pool_size(const int max_size=otl_max_default_pool_size)
  {
    sc.init(max_size);
  }

  void stream_pool_enable()
  {
    pool_enabled_=true;
  }

  void stream_pool_disable()
  {
    pool_enabled_=false;
  }

  bool get_stream_pool_enabled_flag() const
  {
    return pool_enabled_;
  }

#endif

  void commit(void)
  {
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    if(!auto_commit_){
      (*this)<<"commit tran";
      otl_odbc_connect::commit();
      (*this)<<"begin tran";
    }
#else
    otl_odbc_connect::commit();
#endif
  }

  void rollback(void)
  {
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    if(!auto_commit_){
      (*this)<<"rollback tran";
      otl_odbc_connect::rollback();
      (*this)<<"begin tran";
    }
#else
    otl_odbc_connect::rollback();
#endif
  }

  long direct_exec
  (const char* sqlstm,
   const int exception_enabled=1)
   OTL_THROWS_OTL_EXCEPTION
  {
    return otl_cursor::direct_exec(*this,sqlstm,exception_enabled);
  }

  void syntax_check(const char* sqlstm)
   OTL_THROWS_OTL_EXCEPTION
  {
    otl_cursor::syntax_check(*this,sqlstm);
  }

 otl_connect() OTL_NO_THROW : 
    otl_odbc_connect(),
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    sc(),
    pool_enabled_(true),
#endif
    cmd_(nullptr)
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    ,auto_commit_(false)
#endif
  {
  }

 otl_connect(const char* connect_str, const int aauto_commit=0)
   OTL_THROWS_OTL_EXCEPTION: 
   otl_odbc_connect(connect_str, aauto_commit),
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    sc(),
    pool_enabled_(true),
#endif
    cmd_(nullptr)
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    ,auto_commit_(false)
#endif
  {
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    if(aauto_commit)
      auto_commit_=true;
    else
      auto_commit_=false;
#endif
  }
  
 otl_connect(OTL_HENV ahenv,OTL_HDBC ahdbc,const int auto_commit=0)
   OTL_THROWS_OTL_EXCEPTION:
    otl_odbc_connect(),
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    sc(),
    pool_enabled_(true),
#endif
    cmd_(nullptr)
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    ,auto_commit_(false)
#endif
  {
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    if(auto_commit)
      auto_commit_=true;
    else
      auto_commit_=false;
#endif
    rlogon(ahenv,ahdbc,auto_commit);
  }

  const char* getCmd(void) const
  {
    return cmd_;
  }

  otl_connect& operator<<(const char* cmd)
  {
    if(!connected){
      this->rlogon(cmd);
    }else{
      otl_cursor::direct_exec(*this,cmd);
    }
    return *this;
  }

  otl_connect& operator<<=(const char* cmd)
  {
    if(cmd_){
      delete[] cmd_;
      cmd_=nullptr;
    }
    size_t cmd_len=strlen(cmd);
    cmd_=new char[cmd_len+1];
    OTL_STRCPY_S(cmd_,cmd_len+1,cmd);
    return *this;
  }

#if defined(OTL_THROWS_ON_SQL_SUCCESS_WITH_INFO)
  void set_throw_on_sql_success_with_info(const bool throw_flag=false)
  {
    this->get_connect_struct().throws_on_sql_success_with_info=throw_flag;
  }
#endif

  void rlogon(OTL_HENV ahenv,OTL_HDBC ahdbc,const int auto_commit=0)
    OTL_THROWS_OTL_EXCEPTION
  {
    if(this->connected){
     throw otl_exception(otl_error_msg_30,otl_error_code_30);
    }
    if(cmd_){
      delete[] cmd_;
      cmd_=nullptr;
    }
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    if(auto_commit)
      auto_commit_=true;
    else
      auto_commit_=false;
#endif
    retcode=connect_struct.ext_logon(ahenv,ahdbc,auto_commit);
    if(retcode)
      connected=1;
   else{
     connected=0;
     increment_throw_count();
     if(get_throw_count()>1)return;
     if(otl_uncaught_exception()) return; 
     throw otl_exception(connect_struct);
   }
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    if(!auto_commit_){
      (*this)<<"begin tran";
    }
#endif
  }

#if defined(OTL_UNICODE_EXCEPTION_AND_RLOGON)
  void rlogon(const OTL_UNICODE_CHAR_TYPE* username,
              const OTL_UNICODE_CHAR_TYPE* passwd,
              const OTL_UNICODE_CHAR_TYPE* dns,
              const int auto_commit=0)
    OTL_THROWS_OTL_EXCEPTION
  {
    if(this->connected){
     throw otl_exception(otl_error_msg_30,otl_error_code_30);
    }
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    if(auto_commit)
      auto_commit_=true;
    else
      auto_commit_=false;
#endif
    retcode=connect_struct.rlogon
      (OTL_RCAST(const SQLWCHAR*,username),
       OTL_RCAST(const SQLWCHAR*,passwd),
       OTL_RCAST(const SQLWCHAR*,dns),
       auto_commit);
    if(retcode)
      connected=1;
   else{
     connected=0;
     increment_throw_count();
     if(get_throw_count()>1)return;
     if(otl_uncaught_exception()) return; 
     throw otl_exception(connect_struct);
   }
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    if(!auto_commit_){
      (*this)<<"begin tran";
    }
#endif
  }
#endif
  
 virtual ~otl_connect() 
  {
    if(cmd_){
      delete[] cmd_;
      cmd_=nullptr;
    }
#if defined(OTL_DESTRUCTORS_DO_NOT_THROW)
    try{
      logoff();
    }catch(OTL_CONST_EXCEPTION otl_exception&){
    }
#endif
  }

 void rlogon(const char* connect_str, const int aauto_commit=0)
   OTL_THROWS_OTL_EXCEPTION
 {
    if(this->connected){
     throw otl_exception(otl_error_msg_30,otl_error_code_30);
    }
    if(cmd_){
      delete[] cmd_;
      cmd_=nullptr;
   }
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    if(aauto_commit)
      auto_commit_=true;
    else
      auto_commit_=false;
#endif
    otl_odbc_connect::rlogon(connect_str,aauto_commit);
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    if(!auto_commit_){
      (*this)<<"begin tran";
    }
#endif
 }
  
 void logoff(void) 
   OTL_THROWS_OTL_EXCEPTION
 {
#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
  if(connected)
    sc.init(sc.get_max_size());
#endif
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
  if(!auto_commit_) rollback();
#endif
#if defined(OTL_ROLLS_BACK_BEFORE_LOGOFF)
  otl_odbc_connect::rollback();
#endif
  otl_odbc_connect::logoff();
 }

  void set_transaction_isolation_level(const long int level)
    OTL_THROWS_OTL_EXCEPTION
  {
    retcode=connect_struct.set_transaction_isolation_level(level);
    if(!retcode){
     increment_throw_count();
     if(get_throw_count()>1)return;
     if(otl_uncaught_exception()) return; 
     throw otl_exception(connect_struct);
    }
  }

private:

  char* cmd_;
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
  bool auto_commit_;
#endif

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_connect(const otl_connect&) = delete;
 otl_connect& operator=(const otl_connect&) = delete;
private:
#else
 otl_connect(const otl_connect&) OTL_NO_THROW : 
    otl_odbc_connect(),
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    sc(),
    pool_enabled_(true),
#endif
    cmd_(nullptr)
#if defined(OTL_FREETDS_ODBC_WORKAROUNDS)
    ,auto_commit_(false)
#endif
  {
  }

 otl_connect& operator=(const otl_connect&)
 {
   return *this;
 }
#endif

};

const int otl_odbc_no_stream=0;
const int otl_odbc_io_stream=1;
const int otl_odbc_select_stream=2;

class otl_stream_shell: public otl_stream_shell_generic{
public:

  otl_select_stream* ss;
  otl_inout_stream* io;
  otl_connect* adb;
  
  int auto_commit_flag;
  
  otl_var_desc* iov;
  int iov_len;
  int next_iov_ndx;
  
  otl_var_desc* ov;
  int ov_len;
  int next_ov_ndx;
  
  bool flush_flag;
  int stream_type;
  bool lob_stream_flag;

 otl_select_struct_override override_;

#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
 OTL_STRING_CONTAINER orig_sql_stm;
#endif

#if defined(OTL_UNICODE_STRING_TYPE) && defined(OTL_STREAM_POOLING_ON)
  std::string orig_sql_stm;
#endif

  otl_stream_shell():
    otl_stream_shell_generic(),
    ss(nullptr),
    io(nullptr),
    adb(nullptr),
    auto_commit_flag(0),
    iov(nullptr),
    iov_len(0),
    next_iov_ndx(0),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    flush_flag(false),
    stream_type(otl_odbc_no_stream),
    lob_stream_flag(0),
    override_()
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    ,orig_sql_stm()
#endif
 {
  should_delete=0;
 }

  otl_stream_shell(const int ashould_delete):
    otl_stream_shell_generic(),
    ss(nullptr),
    io(nullptr),
    adb(nullptr),
    auto_commit_flag(0),
    iov(nullptr),
    iov_len(0),
    next_iov_ndx(0),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    flush_flag(true),
    stream_type(otl_odbc_no_stream),
    lob_stream_flag(false),
    override_()
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    ,orig_sql_stm()
#endif
 {
  should_delete=ashould_delete;
 }

 virtual ~otl_stream_shell()
 {
  if(should_delete){
   delete[] iov;
   delete[] ov;

   iov=nullptr; iov_len=0;
   ov=nullptr; ov_len=0;
   next_iov_ndx=0;
   next_ov_ndx=0;
   override_.setLen(0);
   flush_flag=true;

   delete ss;
   delete io;
   ss=nullptr; 
   io=nullptr;
   adb=nullptr;
  }
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_stream_shell(const otl_stream_shell&) = delete;
  otl_stream_shell& operator=(const otl_stream_shell&) = delete;
private:
#else
  otl_stream_shell(const otl_stream_shell&):
    otl_stream_shell_generic(),
    ss(nullptr),
    io(nullptr),
    adb(nullptr),
    auto_commit_flag(0),
    iov(nullptr),
    iov_len(0),
    next_iov_ndx(0),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    flush_flag(false),
    stream_type(otl_odbc_no_stream),
    lob_stream_flag(0),
    override_()
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    ,orig_sql_stm()
#endif
 {
  should_delete=0;
 }

  otl_stream_shell& operator=(const otl_stream_shell&)
  {
    return *this;
  }
#endif

};

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct>
class otl_tmpl_lob_stream: public otl_lob_stream_generic{
public:

  typedef otl_tmpl_exception
  <TExceptionStruct,
   TConnectStruct,
   TCursorStruct> otl_exception;

  typedef otl_tmpl_variable<TVariableStruct>* p_bind_var;
  typedef otl_tmpl_connect
  <TExceptionStruct,
   TConnectStruct,
   TCursorStruct>* p_connect;

 typedef otl_tmpl_cursor
 <TExceptionStruct,
  TConnectStruct,
  TCursorStruct,
  TVariableStruct>* p_cursor;

private:
  
  p_bind_var bind_var;
  p_connect connect;
  p_cursor cursor;
  otl_long_string* temp_buf;
  char* temp_char_buf;
  bool written_to_flag;
  bool closed_flag;
  int last_read_lob_len;

public:

  void reset_last_read_lob_len()
  {
    last_read_lob_len=0;
  }

 void init
 (void* avar,void* aconnect,void* acursor,
  int andx,int amode,const int alob_is_null=0)
   OTL_THROWS_OTL_EXCEPTION
 {
   closed_flag=false;
   if(written_to_flag){
     retcode=bind_var->get_var_struct().clob_blob(cursor->get_cursor_struct());
      written_to_flag=false;
      if(!retcode){
        if(this->connect)this->connect->increment_throw_count();
        if(this->connect&&this->connect->get_throw_count()>1)return;
        if(otl_uncaught_exception()) return; 
        throw OTL_TMPL_EXCEPTION
          (cursor->get_cursor_struct(),
           cursor->get_stm_label()?cursor->get_stm_label():
           cursor->get_stm_text());
      }
     
   }
  connect=OTL_RCAST(p_connect,aconnect);
  bind_var=OTL_RCAST(p_bind_var,avar);
  cursor=OTL_RCAST(p_cursor,acursor);
  mode=amode;
  retcode=0;
  lob_is_null=alob_is_null;
  ndx=andx;
  offset=0;
  lob_len=0;
  eof_flag=0;
  in_destructor=0;
  if(bind_var){
    bind_var->get_var_struct().set_lob_stream_flag();
    bind_var->get_var_struct().reset_lob_len();
  }
 }

 void set_len(void) OTL_NO_THROW
 {
 }

 void set_len(const int /*new_len*/) OTL_NO_THROW
 {
 }

 otl_tmpl_lob_stream() OTL_NO_THROW:
   otl_lob_stream_generic(false),
   bind_var(nullptr),
   connect(nullptr),
   cursor(nullptr),
   temp_buf(nullptr),
   temp_char_buf(nullptr),
   written_to_flag(false),
   closed_flag(false),
   last_read_lob_len(0)  
 {
  init(nullptr,nullptr,nullptr,0,otl_lob_stream_zero_mode);
 }

 ~otl_tmpl_lob_stream() 
 {
   in_destructor=1;
   if(temp_buf){
     delete temp_buf;
     temp_buf=nullptr;
   }
   if(temp_char_buf){
     delete[] temp_char_buf;
     temp_char_buf=nullptr;
   }
#if defined(OTL_DESTRUCTORS_DO_NOT_THROW)
   try{
     if(!closed_flag)
       close();
   }catch(OTL_CONST_EXCEPTION otl_exception&){
   }
#else
   if(!closed_flag)
   close();
#endif
 }

#if (defined(OTL_STL) || defined(OTL_ACE) || \
     defined(OTL_USER_DEFINED_STRING_CLASS_ON)) && !defined(OTL_UNICODE)
  otl_lob_stream_generic& operator<<(const OTL_STRING_CONTAINER& s)
    OTL_THROWS_OTL_EXCEPTION
  {
    otl_long_string temp_s(s.c_str(),                  
                           OTL_SCAST(int,s.length()),
                           OTL_SCAST(int,s.length()));
    (*this)<<temp_s;
    return *this;
  }

  void setStringBuffer(const int chunk_size)
  {
    delete[] temp_char_buf;
    temp_char_buf=nullptr;
    delete temp_buf;
    temp_buf=nullptr;
    temp_char_buf=new char[chunk_size+1];
    temp_buf=new otl_long_string(temp_char_buf,chunk_size);
  }

  otl_lob_stream_generic& operator>>(OTL_STRING_CONTAINER& s)
    OTL_THROWS_OTL_EXCEPTION
  {
    const int TEMP_BUF_SIZE=4096;
    if(!temp_char_buf)temp_char_buf=new char[TEMP_BUF_SIZE];
    if(!temp_buf)temp_buf=new otl_long_string(temp_char_buf,TEMP_BUF_SIZE-1);
    int iters=0;
    while(!this->eof()){
      ++iters;
      (*this)>>(*temp_buf);
      temp_char_buf[temp_buf->len()]=0;
      if(iters>1)
        s.append(temp_char_buf,OTL_SCAST(size_t,temp_buf->len()));
      else
#if (defined(OTL_USER_DEFINED_STRING_CLASS_ON) || defined(OTL_STL)) \
     && !defined(OTL_ACE)
        s.assign(temp_char_buf,OTL_SCAST(size_t,temp_buf->len()));
#elif defined(OTL_ACE)
        s.set(temp_char_buf,OTL_SCAST(size_t,temp_buf->len()),1);
#endif
    }
    return *this;
  }

#endif

 otl_lob_stream_generic& operator<<(const otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   bool in_unicode_mode=sizeof(OTL_CHAR)>1;
   if(bind_var && bind_var->get_ftype()==otl_var_raw_long)
     in_unicode_mode=false;
   if(s.get_unicode_flag() != in_unicode_mode){
     throw OTL_TMPL_EXCEPTION
       (otl_error_msg_37,
        otl_error_code_37,
        "otl_lob_stream_generic::operator<<(const otl_long_string&)"
       );
   }
   if(mode!=otl_lob_stream_write_mode){
     const char* stm=nullptr;
     char var_info[256];
     var_info[0]=0;
     if(cursor!=nullptr){
       if(cursor->get_stm_label())
         stm=cursor->get_stm_label();
       else
         stm=cursor->get_stm_text();
     }
     if(bind_var!=nullptr){
       otl_var_info_var
         (bind_var->get_name(),
          bind_var->get_ftype(),
          otl_var_long_string,
          var_info,
          sizeof(var_info));
     }
     char* vinfo=nullptr;
     if(var_info[0]!=0)
       vinfo=&var_info[0];
   if(this->connect)this->connect->increment_throw_count();
   if(this->connect&&this->connect->get_throw_count()>1)return *this;
   if(otl_uncaught_exception()) return *this; 
   throw otl_tmpl_exception
     <TExceptionStruct,
       TConnectStruct,
       TCursorStruct>
     (otl_error_msg_9,
      otl_error_code_9,
      stm,
      vinfo);
   }
   if(offset==0)offset=1;
   if(bind_var!=nullptr)
     retcode=bind_var->get_var_struct().write_blob
       (s,lob_len,offset,cursor->get_cursor_struct());
   written_to_flag=true;
   if(retcode)
     return *this;
   if(this->connect)this->connect->increment_throw_count();
   if(this->connect&&this->connect->get_throw_count()>1)return *this;
   if(otl_uncaught_exception()) return *this; 
   throw OTL_TMPL_EXCEPTION
     (cursor->get_cursor_struct(),
      cursor->get_stm_label()?cursor->get_stm_label():
      cursor->get_stm_text());
 }

 otl_lob_stream_generic& operator>>(otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   bool in_unicode_mode=sizeof(OTL_CHAR)>1;
   if(bind_var && bind_var->get_ftype()==otl_var_raw_long)
     in_unicode_mode=false;
   if(s.get_unicode_flag() != in_unicode_mode){
     throw OTL_TMPL_EXCEPTION
       (otl_error_msg_37,
        otl_error_code_37,
        "otl_lob_stream_generic::operator>>(otl_long_string&)"
       );
   }
   if(mode!=otl_lob_stream_read_mode){
     const char* stm=nullptr;
     char var_info[256];
     var_info[0]=0;
     if(cursor!=nullptr){
       if(cursor->get_stm_label())
         stm=cursor->get_stm_label();
       else
         stm=cursor->get_stm_text();
     }
     if(bind_var!=nullptr){
       otl_var_info_var
         (bind_var->get_name(),
          bind_var->get_ftype(),
          otl_var_long_string,
          var_info,
          sizeof(var_info));
     }
     char* vinfo=nullptr;
     if(var_info[0]!=0)
       vinfo=&var_info[0];
     if(this->connect)this->connect->increment_throw_count();
     if(this->connect&&this->connect->get_throw_count()>1)return *this;
     if(otl_uncaught_exception()) return *this; 
     throw OTL_TMPL_EXCEPTION
       (otl_error_msg_10,
        otl_error_code_10,
        stm,
        vinfo);
   }
   if(offset==0)offset=1;
   if(bind_var!=nullptr)
     retcode=bind_var->get_var_struct().read_blob
       (cursor->get_cursor_struct(),s,ndx,offset,eof_flag);
   len();
   if(retcode){
     if(eof())
       close();
     return *this;
  }
   if(this->connect)this->connect->increment_throw_count();
   if(this->connect&&this->connect->get_throw_count()>1)return *this;
   if(otl_uncaught_exception()) return *this; 
   throw OTL_TMPL_EXCEPTION
     (cursor->get_cursor_struct(),
      cursor->get_stm_label()?cursor->get_stm_label():
      cursor->get_stm_text());
 }

 int eof(void) OTL_NO_THROW
 {
  if(mode!=otl_lob_stream_read_mode)return 1;
  if(lob_is_null)return 1;
  return eof_flag;
 }

 int len(void) OTL_THROWS_OTL_EXCEPTION
 {
   if(last_read_lob_len>0)
     return last_read_lob_len;
   if(cursor==nullptr||connect==nullptr||
      bind_var==nullptr||lob_is_null)return 0;
   int alen;
  retcode=bind_var->get_var_struct().get_blob_len(ndx,alen);
  if(retcode){
    last_read_lob_len=alen;
    return alen;
  }
  if(this->connect)this->connect->increment_throw_count();
  if(this->connect&&this->connect->get_throw_count()>1)return 0;
  if(otl_uncaught_exception()) return 0; 
  throw OTL_TMPL_EXCEPTION
    (connect->get_connect_struct(),
     cursor->get_stm_label()?cursor->get_stm_label():
     cursor->get_stm_text());
 }

 bool is_initialized(void) OTL_THROWS_OTL_EXCEPTION
 {
  if(cursor==nullptr||connect==nullptr||
     bind_var==nullptr||lob_is_null)
    return false;
  else
    return true;
 }

 void close(bool=false) OTL_THROWS_OTL_EXCEPTION
 {
  if(in_destructor){
   if(mode==otl_lob_stream_read_mode){
     bind_var->get_var_struct().set_lob_stream_flag(0);
    bind_var->set_not_null(0);
   } if(mode==otl_lob_stream_write_mode){
     retcode=bind_var->get_var_struct().clob_blob(cursor->get_cursor_struct());
     closed_flag=true;
     written_to_flag=false;
     if(!retcode){
       if(this->connect)this->connect->increment_throw_count();
       if(this->connect&&this->connect->get_throw_count()>1)return;
       if(otl_uncaught_exception()) return; 
       throw OTL_TMPL_EXCEPTION
         (cursor->get_cursor_struct(),
          cursor->get_stm_label()?cursor->get_stm_label():
          cursor->get_stm_text());
     }
   }
   return;
  }
  if(mode==otl_lob_stream_zero_mode)return;
  if(mode==otl_lob_stream_read_mode){
    bind_var->get_var_struct().set_lob_stream_flag(0);
    bind_var->set_not_null(0);
    init(nullptr,nullptr,nullptr,0,otl_lob_stream_zero_mode);
  }else{
    // write mode
    if(mode==otl_lob_stream_write_mode){
      retcode=bind_var->get_var_struct().clob_blob(cursor->get_cursor_struct());
      written_to_flag=false;
      closed_flag=true;
      if(!retcode){
        if(this->connect)this->connect->increment_throw_count();
        if(this->connect&&this->connect->get_throw_count()>1)return;
        if(otl_uncaught_exception()) return; 
        throw OTL_TMPL_EXCEPTION
          (cursor->get_cursor_struct(),
           cursor->get_stm_label()?cursor->get_stm_label():
           cursor->get_stm_text());
      }
    }
    bind_var->get_var_struct().set_lob_stream_flag(0);
    bind_var->set_not_null(0);
  }
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_tmpl_lob_stream(const otl_tmpl_lob_stream&) = delete;
 otl_tmpl_lob_stream& operator=(const otl_tmpl_lob_stream&) = delete;
private:
#else
 otl_tmpl_lob_stream(const otl_tmpl_lob_stream&) OTL_NO_THROW:
   otl_lob_stream_generic(false),
   bind_var(nullptr),
   connect(nullptr),
   cursor(nullptr),
   temp_buf(nullptr),
   temp_char_buf(nullptr),
   written_to_flag(false),
   closed_flag(false),
   last_read_lob_len(0)  
 {
 }

 otl_tmpl_lob_stream& operator=(const otl_tmpl_lob_stream&)
 {
   return *this;
 }
#endif

};

typedef otl_tmpl_lob_stream
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> otl_lob_stream;

class otl_stream{
private:

  otl_stream_shell* shell;
  otl_ptr<otl_stream_shell> shell_pt;
  int connected;
  
  otl_select_stream** ss;
  otl_inout_stream** io;
  otl_connect** adb;
  
  int* auto_commit_flag;
  
  otl_var_desc** iov;
  int* iov_len;
  int* next_iov_ndx;
  
  otl_var_desc** ov;
  int* ov_len;
  int* next_ov_ndx;
  
  otl_select_struct_override* override_;
  int end_marker;
  int oper_int_called;
  int last_eof_rc;
  bool last_oper_was_read_op;

public:

  int get_auto_commit_flag() const
  {
    if(!auto_commit_flag)
      return 0;
    else
      return *auto_commit_flag;
  }

#if defined(OTL_ODBC_SQL_STATEMENT_WITH_DIAG_REC_OUTPUT)

  OTL_SQLHSTMT get_stm_handle(){return (*io)->get_cursor_struct().get_cda();}

#if defined(_UNICODE) || defined(UNICODE)
  typedef SQLWCHAR sqlchar_type;
#else
  typedef SQLCHAR sqlchar_type;
#endif

  bool get_next_diag_rec
  (short int& rec_ndx,
   sqlchar_type* sqlstate_buf,
   sqlchar_type* msg_buf,
   short int msg_buf_size,
   int& native_err)
  {
    OTL_SQLINTEGER rc;
    SQLSMALLINT msg_len;
    rc=SQLGetDiagRec
      (SQL_HANDLE_STMT,
       get_stm_handle(),
       OTL_SCAST(OTL_SQLSMALLINT,rec_ndx),
       OTL_RCAST(sqlchar_type*,sqlstate_buf),
       OTL_RCAST(OTL_SQLINTEGER_PTR,&native_err),
       OTL_RCAST(sqlchar_type*,msg_buf),
       msg_buf_size,
       OTL_RCAST(OTL_SQLSMALLINT_PTR,&msg_len));
    bool result=rc==SQL_NO_DATA||rc<0;
    msg_buf[msg_len]=0;
    ++rec_ndx;
    return result;
  }

#endif

protected:

  int buf_size_;

  void reset_end_marker(void)
  {
    last_eof_rc=0;
    end_marker=-1;
    oper_int_called=0;
  }

  void throw_end_of_row()
#if defined(__GNUC__) && (__GNUC__>=4)
    __attribute__ ((noreturn))
#endif
  {
      throw otl_exception
        (otl_error_msg_34,
         otl_error_code_34,
         this->get_stm_text());
  }

 void inc_next_ov(void)
 {
  if((*ov_len)==0)return;
  if((*next_ov_ndx)<(*ov_len)-1)
   ++(*next_ov_ndx);
  else
   (*next_ov_ndx)=0;
 }
 
 void inc_next_iov(void)
 {
  if((*iov_len)==0)return;
  if((*next_iov_ndx)<(*iov_len)-1)
   ++(*next_iov_ndx);
  else
   (*next_iov_ndx)=0;
 }

public:

  int get_prefetched_row_count() const 
  {
    if(*ss){
      (*adb)->reset_throw_count();
      return (*ss)->get_prefetched_row_count();
    }
    return 0;
  }

  bool get_lob_stream_flag() const 
  {
    if(!shell)
      return false;
    else
      return shell->lob_stream_flag;
  }

  int get_adb_max_long_size() const 
  {
    return this->shell->adb->get_max_long_size();
  }

  void check_end_of_row()
  {
    if(next_ov_ndx==nullptr||(*next_ov_ndx)!=0)
      throw_end_of_row();
    if(next_iov_ndx==nullptr||(*next_iov_ndx)!=0)
      throw_end_of_row();
  }

  otl_stream_shell* get_shell(){return shell;}
  int get_connected() const {return connected;}

  int get_dirty_buf_len() const
  {
    switch(shell->stream_type){
    case otl_odbc_no_stream:
      return 0;
    case otl_odbc_io_stream:
      return (*io)->get_dirty_buf_len();
    case otl_odbc_select_stream:
      return (*ss)->get_select_row_count();
    default:
      return 0;
    }
  }

  const char* get_stm_text(void)
  {
    const char* no_stm_text=OTL_NO_STM_TEXT;
    switch(shell->stream_type){
    case otl_odbc_no_stream:
      return no_stm_text;
    case otl_odbc_io_stream:
      return (*io)->get_stm_label()?(*io)->get_stm_label():(*io)->get_stm_text();
    case otl_odbc_select_stream:
      return (*ss)->get_stm_label()?(*ss)->get_stm_label():(*ss)->get_stm_text();
    default:
      return no_stm_text;
    }
  }

  

  void setBufSize(int buf_size)
  {
    buf_size_=buf_size;
  }
  
  int getBufSize(void) const
  {
    return buf_size_;
  }

 long get_rpc() OTL_NO_THROW
 {
  if((*io)){
   (*adb)->reset_throw_count();
   return (*io)->get_rpc();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   return (*ss)->get_rfc();
  }else
   return 0;
 }

  void skip_to_end_of_row()
  {
    if(next_ov_ndx==nullptr)
      return;
    if((*ov_len)==0)return;
    last_oper_was_read_op=true;
    switch(shell->stream_type){
    case otl_odbc_no_stream:
      break;
    case otl_odbc_io_stream:
      last_eof_rc=(*io)->eof();
      (*io)->skip_to_end_of_row();
      break;
    case otl_odbc_select_stream:
      last_eof_rc=(*ss)->eof();
      (*ss)->skip_to_end_of_row();
      break;
    }
    *next_ov_ndx=0;
  }


  operator int(void) OTL_THROWS_OTL_EXCEPTION
  {
    if(shell && shell->lob_stream_flag){
      if(this->adb&&*this->adb)(*this->adb)->increment_throw_count();
      if(this->adb&&*this->adb&&(*this->adb)->get_throw_count()>1)return 0;
      const char* stm_label=nullptr;
      const char* stm_text=nullptr;
      if((*io)){
        stm_label=(*io)->get_stm_label();
        stm_text=(*io)->get_stm_text();
      }else if((*ss)){
        stm_label=(*ss)->get_stm_label();
        stm_text=(*ss)->get_stm_text();
      }
      throw otl_exception
        (otl_error_msg_24,
         otl_error_code_24,
         stm_label?stm_label:stm_text);
    }
    if(!last_oper_was_read_op){
      if(this->adb&&*this->adb)(*this->adb)->increment_throw_count();
      if(this->adb&&*this->adb&&(*this->adb)->get_throw_count()>1)return 0;
      const char* stm_label=nullptr;
      const char* stm_text=nullptr;
      if((*io)){
        stm_label=(*io)->get_stm_label();
        stm_text=(*io)->get_stm_text();
      }else if((*ss)){
        stm_label=(*ss)->get_stm_label();
        stm_text=(*ss)->get_stm_text();
      }
      throw otl_exception
        (otl_error_msg_18,
         otl_error_code_18,
         stm_label?stm_label:stm_text);
    }
    if(end_marker==1)return 0;
    int rc=0;
    int temp_eof=eof();
    if(temp_eof && end_marker==-1 && oper_int_called==0){
      end_marker=1;
      if(last_eof_rc==1)
        rc=0;
      else
        rc=1;
    }else if(!temp_eof && end_marker==-1)
      rc=1;
    else if(temp_eof && end_marker==-1){
      end_marker=0;
      rc=1;
    }else if(temp_eof && end_marker==0){
      end_marker=1;
      rc=0;
    }
    if(!oper_int_called)oper_int_called=1;
    return rc;
  }

  void cancel(void) OTL_THROWS_OTL_EXCEPTION
  {
    if((*ss)){
      (*adb)->reset_throw_count();
      int status=(*ss)->get_cursor_struct().cancel();
      if(status==0)
        throw otl_exception((*ss)->get_cursor_struct());
    }else if((*io)){
      (*adb)->reset_throw_count();
      int status=(*io)->get_cursor_struct().cancel();
      if(status==0)
        throw otl_exception((*io)->get_cursor_struct());
    }
  }

 void create_var_desc(void)
 {int i;
  delete[] (*iov);
  delete[] (*ov);
  (*iov)=nullptr; (*iov_len)=0;
  (*ov)=nullptr; (*ov_len)=0;
  if((*ss)){
    if((*ss)->get_vl_len()>0){
      (*iov)=new otl_var_desc[(*ss)->get_vl_len()];
      (*iov_len)=(*ss)->get_vl_len();
      for(i=0;i<(*ss)->get_vl_len();++i)
        (*ss)->get_vl()[i]->copy_var_desc((*iov)[i]);
    }
    if((*ss)->get_sl_len()>0){
      (*ov)=new otl_var_desc[(*ss)->get_sl_len()];
      (*ov_len)=(*ss)->get_sl_len();
      for(i=0;i<(*ss)->get_sl_len();++i){
        (*ss)->get_sl()[i].copy_var_desc((*ov)[i]);
        if((*ss)->get_sl_desc()!=nullptr)
          (*ov)[i].copy_name((*ss)->get_sl_desc()[i].name);
      }
   }
  }else if((*io)){
    int temp_vl_len=(*io)->get_vl_len();
    int temp_iv_len=(*io)->get_iv_len();
    if(temp_vl_len>0){
      (*iov)=new otl_var_desc[temp_vl_len];
      (*iov_len)=temp_vl_len;
      for(i=0;i<temp_vl_len;++i)
        (*io)->get_vl()[i]->copy_var_desc((*iov)[i]);
    }
    if(temp_iv_len>0){
      (*ov)=new otl_var_desc[temp_iv_len];
      (*ov_len)=temp_iv_len;
      for(i=0;i<temp_iv_len;++i)
        (*io)->get_in_vl()[i]->copy_var_desc((*ov)[i]);
    }
  }
 }

 void set_column_type(const int column_ndx,
                      const int col_type,
                      const int col_size=0)
   OTL_NO_THROW
 {
   if(shell==nullptr){
     init_stream();
     shell->flush_flag=true;
   }
  override_->add_override(column_ndx,col_type,col_size);
 }

  void set_all_column_types(const unsigned mask=0)
    OTL_NO_THROW
  {
    if(shell==nullptr){
      init_stream();
      shell->flush_flag=true;
    }
    override_->set_all_column_types(mask);
  }

 void set_flush(const bool flush_flag=true)
   OTL_NO_THROW
 {
   if(shell==nullptr)
     init_stream();
   shell->flush_flag=flush_flag;
 }

 void set_lob_stream_mode(const bool lob_stream_flag=false)
   OTL_NO_THROW
 {
  if(shell==nullptr)return;
  shell->lob_stream_flag=lob_stream_flag;
 }

 otl_var_desc* describe_in_vars(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if(shell==nullptr)return nullptr;
  if(shell->iov==nullptr)return nullptr;
  desc_len=shell->iov_len;
  return shell->iov;
 }

 otl_var_desc* describe_out_vars(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if(shell==nullptr)return nullptr;
  if(shell->ov==nullptr)return nullptr;
  desc_len=shell->ov_len;
  return shell->ov;
 }

 otl_var_desc* describe_next_in_var(void)
   OTL_NO_THROW
 {
  if(shell==nullptr)return nullptr;
  if(shell->iov==nullptr)return nullptr;
  return &(shell->iov[shell->next_iov_ndx]);
 }

 otl_var_desc* describe_next_out_var(void)
   OTL_NO_THROW
 {
  if(shell==nullptr)return nullptr;
  if(shell->ov==nullptr)return nullptr;
  return &(shell->ov[shell->next_ov_ndx]);
 }

 void init_stream(void)
 {
   buf_size_=1;
   last_oper_was_read_op=false;
   shell=nullptr;
   shell=new otl_stream_shell(0);
   shell_pt.assign(&shell);
   connected=0;
   
   ss=&(shell->ss);
   io=&(shell->io);
   adb=&(shell->adb);
   auto_commit_flag=&(shell->auto_commit_flag);
   iov=&(shell->iov);
   iov_len=&(shell->iov_len);
   next_iov_ndx=&(shell->next_iov_ndx);
   ov=&(shell->ov);
   ov_len=&(shell->ov_len);
   next_ov_ndx=&(shell->next_ov_ndx);
    override_=&(shell->override_);
   (*io)=nullptr;
   (*ss)=nullptr;
   (*adb)=nullptr;
   (*ov)=nullptr; 
   (*ov_len)=0;
   (*next_iov_ndx)=0;
   (*next_ov_ndx)=0;
   (*auto_commit_flag)=1;
   (*iov)=nullptr; 
   (*iov_len)=0;

 }

 otl_stream
 (const otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  otl_connect& db,
  const int implicit_select=otl_explicit_select,
  const char* sqlstm_label=nullptr)
   OTL_THROWS_OTL_EXCEPTION:
   shell(nullptr),
   shell_pt(),
   connected(0),
   ss(nullptr),
   io(nullptr),
   adb(nullptr),
   auto_commit_flag(nullptr),
   iov(nullptr),
   iov_len(nullptr),
   next_iov_ndx(nullptr),
   ov(nullptr),
   ov_len(nullptr),
   next_ov_ndx(nullptr),
   override_(nullptr),
   end_marker(0),
   oper_int_called(0),
   last_eof_rc(0),
   last_oper_was_read_op(false),
   buf_size_(0)
 {
  init_stream();

  (*io)=nullptr; 
  (*ss)=nullptr;
  (*iov)=nullptr; 
  (*iov_len)=0;
  (*ov)=nullptr; 
  (*ov_len)=0;
  (*auto_commit_flag)=1;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  (*adb)=&db;
  shell->flush_flag=true;
  open(arr_size,sqlstm,db,implicit_select,sqlstm_label);
 }

 otl_stream() OTL_NO_THROW:
   shell(nullptr),
   shell_pt(),
   connected(0),
   ss(nullptr),
   io(nullptr),
   adb(nullptr),
   auto_commit_flag(nullptr),
   iov(nullptr),
   iov_len(nullptr),
   next_iov_ndx(nullptr),
   ov(nullptr),
   ov_len(nullptr),
   next_ov_ndx(nullptr),
   override_(nullptr),
   end_marker(0),
   oper_int_called(0),
   last_eof_rc(0),
   last_oper_was_read_op(false),
   buf_size_(0)
 {
  init_stream();
  shell->flush_flag=true;
 }

 virtual ~otl_stream()
 {
  if(!connected)return;
  try{
   if((*io)!=nullptr && shell->flush_flag==false)
     (*io)->set_flush_flag2(false);
   close();
   if(shell!=nullptr){
    if((*io)!=nullptr)
      (*io)->set_flush_flag2(true);
   }
  }catch(OTL_CONST_EXCEPTION otl_exception&){
   if(shell!=nullptr){
    if((*io)!=nullptr)
      (*io)->set_flush_flag2(true);
   }
#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
   clean(1);
   if(shell!=nullptr)
     shell->set_should_delete(1);
   shell_pt.destroy();
#else
   shell_pt.destroy();
#endif
#if !defined(OTL_DESTRUCTORS_DO_NOT_THROW)
   throw;
#endif
  }
#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
  if((adb && (*adb) && (*adb)->get_throw_count()>0)
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     || otl_uncaught_exception()
#endif
     ){
   //
  }
#elif defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(otl_uncaught_exception()){
     //
   }
#else
   shell_pt.destroy();
#endif
 }

 int eof(void)
#if defined(OTL_SELECT_STREAM_ALTERNATE_FETCH)
   OTL_THROWS_OTL_EXCEPTION
#else
   OTL_NO_THROW
#endif
 {
  if((*io)){
   (*adb)->reset_throw_count();
   return (*io)->eof();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   return (*ss)->eof();
  }else
   return 1;
 }

  void reset_to_last_valid_row()
  {
    if((*io)){
      (*adb)->reset_throw_count();
      (*io)->reset_to_last_valid_row();
    }
  }

 void flush(void) OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->flush();
  }
 }

 bool get_error_state(void) const
 {
   if((*adb)->get_throw_count()>0)
     return true;
   else if((*io))
     return (*io)->get_error_state();
   else
    return false;
 }

 void clean(const int clean_up_error_flag=0) 
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->clean(clean_up_error_flag);
  }else if((*ss)){
    (*adb)->reset_throw_count();
    (*ss)->clean();
  }
 }

 void rewind(void) OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->rewind();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   (*ss)->rewind();
  }
 }

 
 int is_null(void) OTL_NO_THROW
 {
  if((*io))
   return (*io)->is_null();
  else if((*ss))
   return (*ss)->is_null();
  else
   return 0;
 }

 void set_commit(int auto_commit=0) OTL_NO_THROW
 {
  (*auto_commit_flag)=auto_commit;
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->set_commit(auto_commit);
  }
 }

 const char* assign_stream_type
 (const char* stm_text,
  const char* stm_label)
 {
   const char* temp_stm_text=nullptr;
   temp_stm_text=stm_label?stm_label:stm_text;
   return temp_stm_text;
 }
 
 void open
 (const otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  otl_connect& db,
  const int implicit_select=otl_explicit_select,
  const char* sqlstm_label=nullptr)
   OTL_THROWS_OTL_EXCEPTION
 {
   if(arr_size<=0){
     throw otl_exception
       (otl_error_msg_40,
        otl_error_code_40,
        sqlstm);
   }
#if defined(OTL_STREAM_THROWS_NOT_CONNECTED_TO_DATABASE_EXCEPTION)
   if(!db.connected){
     throw otl_exception
       (otl_error_msg_35,
        otl_error_code_35,
        sqlstm);
   }
#endif
   reset_end_marker();
   otl_stream_buffer_size_type temp_arr_size=arr_size;
   if(this->good()){
     const char* temp_stm_text=assign_stream_type(sqlstm,sqlstm_label);
     throw otl_exception
       (otl_error_msg_29,
        otl_error_code_29,
        temp_stm_text);
   }
  if(shell==nullptr)
   init_stream();
  buf_size_=arr_size;
  OTL_TRACE_STREAM_OPEN

#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    if(*adb==nullptr)*adb=&db;
    if((*adb) && (**adb).get_stream_pool_enabled_flag()){
      char temp_buf[128];
      otl_itoa(arr_size,temp_buf);

      const char delimiter=';';
#if defined(OTL_STREAM_POOL_USES_STREAM_LABEL_AS_KEY)
      const char* temp_label=sqlstm_label?sqlstm_label:sqlstm;
#if defined(OTL_UNICODE_STRING_TYPE)
      std::string sql_stm(temp_label);
#else
      OTL_STRING_CONTAINER sql_stm(temp_label);
#endif
      sql_stm+=delimiter;
#if defined(OTL_UNICODE_STRING_TYPE)
      sql_stm+=std::string(temp_buf);
#else
      sql_stm+=OTL_STRING_CONTAINER(temp_buf);
#endif
#else
#if defined(OTL_UNICODE_STRING_TYPE)
      std::string sql_stm(sqlstm);
#else
      OTL_STRING_CONTAINER sql_stm(sqlstm);
#endif
      sql_stm+=delimiter;
#if defined(OTL_UNICODE_STRING_TYPE)
      sql_stm+=std::string(temp_buf);
#else
      sql_stm+=OTL_STRING_CONTAINER(temp_buf);
#endif
#endif
      if(shell!=nullptr){
        otl_select_struct_override& temp_override=shell->override_;
        for(int i=0;i<temp_override.getLen();++i){
          otl_itoa(OTL_SCAST(int,temp_override.get_col_type(i)),temp_buf);
          sql_stm+=delimiter;
#if defined(OTL_UNICODE_STRING_TYPE)
          sql_stm+=std::string(temp_buf);
#else
          sql_stm+=OTL_STRING_CONTAINER(temp_buf);
#endif
        }    
      }
      otl_stream_shell* temp_shell=OTL_RCAST(otl_stream_shell*,db.sc.find(sql_stm));
      if(temp_shell){
        if(shell!=nullptr)shell_pt.destroy();
        shell=temp_shell;
        ss=&(shell->ss);
        io=&(shell->io);
        if((*io)!=nullptr)(*io)->set_flush_flag2(true);
        adb=&(shell->adb);
        if(*adb==nullptr)*adb=&db;
        auto_commit_flag=&(shell->auto_commit_flag);
        iov=&(shell->iov);
        iov_len=&(shell->iov_len);
        next_iov_ndx=&(shell->next_iov_ndx);
        ov=&(shell->ov);
        ov_len=&(shell->ov_len);
        next_ov_ndx=&(shell->next_ov_ndx);
        override_=&(shell->override_);
        try{
          if((*iov_len)==0)this->rewind();
        }catch(OTL_CONST_EXCEPTION otl_exception&){
          if((*adb))
            (*adb)->sc.remove(shell,shell->orig_sql_stm);
          intern_cleanup();
          shell_pt.destroy();
          connected=0;
          throw;     
        }
        connected=1;
        return;
      }
      shell->orig_sql_stm=sql_stm;
    }
#endif

  delete[] (*iov);
  delete[] (*ov);

  (*iov)=nullptr; (*iov_len)=0;
  (*ov)=nullptr; (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;

  char tmp[7];
  char* c=OTL_CCAST(char*,sqlstm);

  override_->set_lob_stream_mode(shell->lob_stream_flag);
  while(otl_isspace(*c)||(*c)=='(')++c;
  OTL_STRNCPY_S(tmp,sizeof(tmp),c,6);
  tmp[6]=0;
  c=tmp;
  while(*c){
   *c=OTL_SCAST(char,otl_to_upper(*c));
   ++c;
  }
  if(adb==nullptr)adb=&(shell->adb);
  (*adb)=&db;
  (*adb)->reset_throw_count();
  try{
#if (defined(OTL_ODBC_POSTGRESQL) && !defined(OTL_ODBC_ALTERNATE_RPC) || \
     defined(OTL_ODBC_SELECT_STM_EXECUTE_BEFORE_DESCRIBE)) && \
  !defined(OTL_ODBC_MULTI_MODE)
   if((strncmp(tmp,"SELECT",6)==0||
       strncmp(tmp,"WITH",4)==0)){
     override_->set_master_stream_ptr(OTL_RCAST(void*,this));
     (*ss)=new otl_select_stream(override_,
                                 temp_arr_size,
                                 sqlstm,
                                 db,otl_implicit_select,
                                 sqlstm_label);
     shell->stream_type=otl_odbc_select_stream;
   }
#elif defined(OTL_ODBC_MULTI_MODE)
#if defined(OTL_ODBC_ALTERNATE_RPC)
   bool alternate_rpc=true;
#else
   bool alternate_rpc=false;
#endif
   int connect_type=(*adb)->get_connect_struct().get_connection_type();
   if(((connect_type==OTL_POSTGRESQL_ODBC_CONNECT&&!alternate_rpc) ||
       connect_type==OTL_ENTERPRISE_DB_ODBC_CONNECT ||
       connect_type==OTL_MYODBC35_ODBC_CONNECT) &&
     (strncmp(tmp,"SELECT",6)==0||strncmp(tmp,"WITH",4)==0)){
     override_->set_master_stream_ptr(OTL_RCAST(void*,this));
     (*ss)=new otl_select_stream(override_,
                                 temp_arr_size,
                                 sqlstm,
                                 db,otl_implicit_select,
                                 sqlstm_label);
     shell->stream_type=otl_odbc_select_stream;
   }else if((strncmp(tmp,"SELECT",6)==0||
             strncmp(tmp,"WITH",4)==0)&&
            !implicit_select){
     (*ss)=new otl_select_stream(override_,temp_arr_size,sqlstm,
                                 db,otl_explicit_select,
                                 sqlstm_label);
     shell->stream_type=otl_odbc_select_stream;
   }
#else
   if((strncmp(tmp,"SELECT",6)==0||
       strncmp(tmp,"WITH",4)==0)&&
      !implicit_select){
#if defined(OTL_ODBC_TIMESTEN)
   if(temp_arr_size>128||temp_arr_size<0){
     const char* temp_stm_text=assign_stream_type(sqlstm,sqlstm_label);
     throw otl_exception
       (otl_error_msg_31,
        otl_error_code_31,
        temp_stm_text);
   }
#endif     
     override_->set_master_stream_ptr(OTL_RCAST(void*,this));
     (*ss)=new otl_select_stream(override_,temp_arr_size,sqlstm,
                                 db,otl_explicit_select,
                                 sqlstm_label);
     shell->stream_type=otl_odbc_select_stream;
   }
#endif
   else if(tmp[0]=='$'){
     override_->set_master_stream_ptr(OTL_RCAST(void*,this));
     (*ss)=new otl_select_stream
       (override_,temp_arr_size,
        sqlstm,db,
        1,sqlstm_label);
     shell->stream_type=otl_odbc_select_stream;
   }else{
     if(implicit_select){
#if defined(OTL_ODBC_TIMESTEN)
   if(temp_arr_size>128||temp_arr_size<0){
     const char* temp_stm_text=assign_stream_type(sqlstm,sqlstm_label);
     throw otl_exception
       (otl_error_msg_31,
        otl_error_code_31,
        temp_stm_text);
   }
#endif     
   override_->set_master_stream_ptr(OTL_RCAST(void*,this));
   (*ss)=new otl_select_stream(override_,temp_arr_size,
                               sqlstm,db,
                               1,sqlstm_label);
   shell->stream_type=otl_odbc_select_stream;
     }else{
       (*io)=new otl_inout_stream
         (arr_size,sqlstm,db,
          OTL_RCAST(void*,this),
          shell->lob_stream_flag,
          sqlstm_label);
       (*io)->set_flush_flag(shell->flush_flag);
       shell->stream_type=otl_odbc_io_stream;
     }
   }
  }catch(OTL_CONST_EXCEPTION otl_exception&){
   shell_pt.destroy();
   throw;
  }
  if((*io))(*io)->set_commit((*auto_commit_flag));
  create_var_desc();
  connected=1;
#if defined(OTL_ODBC_USES_SQL_FETCH_SCROLL_WHEN_SPECIFIED_IN_OTL_CONNECT)
  if((*ss) && db.get_fetch_scroll_mode()){
    (*ss)->get_select_cursor_struct().set_fetch_scroll_flag();
  }
#endif
 }

 void intern_cleanup(void)
 {
  delete[] (*iov);
  delete[] (*ov);

  (*iov)=nullptr; (*iov_len)=0;
  (*ov)=nullptr; (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  override_->setLen(0);
  override_->set_lob_stream_mode(false);

  switch(shell->stream_type){
  case otl_odbc_no_stream:
    break;
  case otl_odbc_io_stream:
    try{
      (*io)->flush();
      (*io)->close();
    }catch(OTL_CONST_EXCEPTION otl_exception&){
      clean(1);
      (*io)->close();
      delete (*io);
      (*io)=nullptr;
      shell->stream_type=otl_odbc_no_stream;
      throw;
    }
    delete (*io);
    (*io)=nullptr;
    shell->stream_type=otl_odbc_no_stream;
    break;
  case otl_odbc_select_stream:
    try{
      (*ss)->close();
    }catch(OTL_CONST_EXCEPTION otl_exception&){
      delete (*ss);
      (*ss)=nullptr;
      shell->stream_type=otl_odbc_no_stream;
      throw;
    }
    delete (*ss);
    (*ss)=nullptr;
    shell->stream_type=otl_odbc_no_stream;
    break;
  }
  (*ss)=nullptr; (*io)=nullptr;
  if(adb!=nullptr)(*adb)=nullptr; 
  adb=nullptr;
 }

#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
 void close(const bool save_in_stream_pool=true)
   OTL_THROWS_OTL_EXCEPTION
#else
 void close(void)
   OTL_THROWS_OTL_EXCEPTION
#endif
 {
  if(shell==nullptr)return;

  OTL_TRACE_FUNC(0x4,"otl_stream","close","")

#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
  if(save_in_stream_pool && (*adb) && (**adb).get_stream_pool_enabled_flag() &&
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     !(otl_uncaught_exception())&&
#elif defined(OTL_UNCAUGHT_EXCEPTION_ON)
     !(otl_uncaught_exception())&&
#endif
     (*adb)->get_throw_count()==0){
   try{
    this->flush();
    this->clean(1);
   }catch(OTL_CONST_EXCEPTION otl_exception&){
    this->clean(1);
    throw;
   }
   if((*adb) && (*adb)->get_throw_count()>0){
    (*adb)->sc.remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return;
   }
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(otl_uncaught_exception()){
    if((*adb))
     (*adb)->sc.remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return; 
   }
#elif defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(otl_uncaught_exception()){
    if((*adb))
     (*adb)->sc.remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return; 
   }
#endif
   (*adb)->sc.add(shell,shell->orig_sql_stm.c_str());
   shell_pt.disconnect();
   connected=0;
  }else{
   if((*adb))
    (*adb)->sc.remove(shell,shell->orig_sql_stm);
   intern_cleanup();
   shell_pt.destroy();
   connected=0;
  }
#else
  intern_cleanup();
  connected=0;
#endif
 }

 otl_column_desc* describe_select(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if((*ss)){
   (*adb)->reset_throw_count();
   desc_len=(*ss)->get_sl_len();
   return (*ss)->get_sl_desc();
  }
  return nullptr;
 }

 int good(void) OTL_NO_THROW
 {
  if(!connected)return 0;
  if((*io)||(*ss)){
   (*adb)->reset_throw_count();
   return 1;
  }else
   return 0;
 }

 otl_stream& operator<<(otl_lob_stream& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=false;
   reset_end_marker();
  if((*io)){
    (*io)->operator<<(s);
   inc_next_iov();
  }else if(*ss){
      throw otl_exception
        (otl_error_msg_41,
         otl_error_code_41,
         (*ss)->get_stm_label()?
         (*ss)->get_stm_label():
         (*ss)->get_stm_text());
  }
  return *this;
 }

  otl_stream& operator>>(otl_stream& (*pf) (otl_stream&))
  {
    (*pf)(*this);
    return *this;
  }

  otl_stream& operator<<(otl_stream& (*pf) (otl_stream&))
  {
    (*pf)(*this);
    return *this;
  }
  
 otl_stream& operator>>(otl_lob_stream& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_odbc_select_stream:
     s.reset_last_read_lob_len();
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   }
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(otl_time& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   }
   return *this;
 }

 otl_stream& operator<<(const otl_time& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     (*io)->operator<<(n);
     break;
   case otl_odbc_select_stream:
     (*ss)->operator<<(n);
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   return *this;
 }

 otl_stream& operator>>(otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
#if defined(OTL_ODBC_STRING_TO_TIMESTAMP)
   otl_var_desc* temp_next_var=describe_next_out_var();
  if(temp_next_var!=nullptr && temp_next_var->ftype==otl_var_char){
#if defined(OTL_UNICODE)
#if defined(OTL_UNICODE_CHAR_TYPE)
    OTL_UNICODE_CHAR_TYPE tmp_str[100];
#else
    OTL_CHAR tmp_str[100];
#endif
#else
    char tmp_str[100];
#endif    
    (*this)>>tmp_str;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
    if((*this).is_null())
      s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
    else
      OTL_ODBC_STRING_TO_TIMESTAMP(tmp_str,s);
#else
    OTL_ODBC_STRING_TO_TIMESTAMP(tmp_str,s);
#endif
#if defined(OTL_ODBC_TIME_ZONE)
    OTL_TRACE_WRITE
      (OTL_TRACE_FORMAT_TZ_DATETIME(s),
       "operator >>",
       "otl_datetime&");
#else
    OTL_TRACE_WRITE
      (OTL_TRACE_FORMAT_DATETIME(s),
       "operator >>",
       "otl_datetime&");
#endif
    return *this;
  }else{
    otl_time tmp;
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__*100+__GNUC_MINOR__>=405)
    tmp.year=1900; 
    tmp.month=1; 
    tmp.day=1;
    tmp.hour=0; 
    tmp.minute=0; 
    tmp.second=0;
    tmp.fraction=0;
#endif
    (*this)>>tmp;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
    if((*this).is_null())
      s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
    else{
      s.year=tmp.year;
      s.month=tmp.month;
      s.day=tmp.day;
      s.hour=tmp.hour;
      s.minute=tmp.minute;
      s.second=tmp.second;
      s.fraction=otl_from_fraction(tmp.fraction,s.frac_precision);
    }
#else
    s.year=tmp.year;
    s.month=tmp.month;
    s.day=tmp.day;
    s.hour=tmp.hour;
    s.minute=tmp.minute;
    s.second=tmp.second;
    s.fraction=otl_from_fraction(tmp.fraction,s.frac_precision);
#endif
  }
#else
   otl_time tmp;
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__*100+__GNUC_MINOR__>=405)
   tmp.year=1900; 
   tmp.month=1; 
   tmp.day=1;
   tmp.hour=0; 
   tmp.minute=0; 
   tmp.second=0;
   tmp.fraction=0;
#endif
   (*this)>>tmp;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
   if((*this).is_null())
     s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
   else{
     s.year=tmp.year;
     s.month=tmp.month;
     s.day=tmp.day;
     s.hour=tmp.hour;
     s.minute=tmp.minute;
     s.second=tmp.second;
     s.fraction=otl_from_fraction(tmp.fraction,s.frac_precision);
   }
#else
  s.year=tmp.year;
  s.month=tmp.month;
  s.day=tmp.day;
  s.hour=tmp.hour;
  s.minute=tmp.minute;
  s.second=tmp.second;
  s.fraction=otl_from_fraction(tmp.fraction,s.frac_precision);
#endif

#endif
  OTL_TRACE_WRITE
    (OTL_TRACE_FORMAT_DATETIME(s),
     "operator >>",
     "otl_datetime&");
  inc_next_ov();
  return *this;
 }

 otl_stream& operator<<(const otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   otl_time tmp;
   last_oper_was_read_op=false;
   reset_end_marker();
#if defined(OTL_ODBC_TIMESTAMP_TO_STRING)
    otl_var_desc* temp_next_var=describe_next_in_var();
    if(temp_next_var!=nullptr && temp_next_var->ftype==otl_var_char){
#if defined(OTL_UNICODE)
#if defined(OTL_UNICODE_CHAR_TYPE)
     OTL_UNICODE_CHAR_TYPE tmp_str[100];
#else
      OTL_CHAR tmp_str[100];
#endif
#else
      char tmp_str[100];
#endif      
     OTL_ODBC_TIMESTAMP_TO_STRING(s,tmp_str);
#if defined(OTL_ODBC_TIME_ZONE)
     OTL_TRACE_READ
       (OTL_TRACE_FORMAT_TZ_DATETIME(s),
        "operator <<",
        "otl_datetime&");
#else
     OTL_TRACE_READ
       (OTL_TRACE_FORMAT_DATETIME(s),
        "operator <<",
        "otl_datetime&");
#endif
     (*this)<<tmp_str;
     return *this;
    }else{
      tmp.year=OTL_SCAST(SQLSMALLINT,s.year);
      tmp.month=OTL_SCAST(SQLSMALLINT,s.month);
      tmp.day=OTL_SCAST(SQLSMALLINT,s.day);
      tmp.hour=OTL_SCAST(SQLSMALLINT,s.hour);
      tmp.minute=OTL_SCAST(SQLSMALLINT,s.minute);
      tmp.second=OTL_SCAST(SQLSMALLINT,s.second);
      tmp.fraction=otl_to_fraction(s.fraction,s.frac_precision);
      (*this)<<tmp;  
      OTL_TRACE_READ(OTL_TRACE_FORMAT_DATETIME(s),
                     "operator >>",
                     "otl_datetime&");
      inc_next_iov();
      return *this;
    }
#else
    tmp.year=OTL_SCAST(SQLSMALLINT,s.year);
    tmp.month=OTL_SCAST(SQLSMALLINT,s.month);
    tmp.day=OTL_SCAST(SQLSMALLINT,s.day);
    tmp.hour=OTL_SCAST(SQLSMALLINT,s.hour);
    tmp.minute=OTL_SCAST(SQLSMALLINT,s.minute);
    tmp.second=OTL_SCAST(SQLSMALLINT,s.second);
    tmp.fraction=otl_to_fraction(s.fraction,s.frac_precision);
    (*this)<<tmp;  
    OTL_TRACE_READ(OTL_TRACE_FORMAT_DATETIME(s),
                   "operator <<",
                   "otl_datetime&");
    inc_next_iov();
    return *this;
#endif
 }

#if !defined(OTL_UNICODE)
 otl_stream& operator>>(char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(c);
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(c);
     break;
   }
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   OTL_TRACE_WRITE("'"<<c<<"'","operator >>","char&")
   inc_next_ov();
   return *this;
 }
#endif

#if !defined(OTL_UNICODE)
 otl_stream& operator>>(unsigned char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(c);
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(c);
     break;
   }
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   OTL_TRACE_WRITE("'"<<c<<"'","operator >>","unsigned char&")
   inc_next_ov();
   return *this;
 }
#endif

#if !defined(OTL_UNICODE)
#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_stream& operator>>(OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   }

#if defined(OTL_USER_DEFINED_STRING_CLASS_DEFAULT_NULL_TO_VAL)
   if((*this).is_null()){
     OTL_USER_DEFINED_STRING_CLASS_DEFAULT_NULL_TO_VAL(s);
   }
#elif defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     s=OTL_DEFAULT_STRING_NULL_TO_VAL;
#endif

   OTL_TRACE_WRITE(s,"operator >>","OTL_STRING_CONTAINER&")
   inc_next_ov();
   return *this;
 }
#endif
#endif

#if !defined(OTL_UNICODE)
 otl_stream& operator>>(char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     otl_strcpy(OTL_RCAST(unsigned char*,s),
                OTL_RCAST(const unsigned char*,OTL_DEFAULT_STRING_NULL_TO_VAL));
#endif
   OTL_TRACE_WRITE(s,"operator >>","char*")
   inc_next_ov();
   return *this;
 }
#endif

#if defined(OTL_UNICODE_STRING_TYPE)
 otl_stream& operator>>(OTL_UNICODE_STRING_TYPE& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   }

#if defined(OTL_USER_DEFINED_STRING_CLASS_DEFAULT_NULL_TO_VAL)
   if((*this).is_null()){
     OTL_USER_DEFINED_STRING_CLASS_DEFAULT_NULL_TO_VAL(s);
   }
#elif defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     s=OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,OTL_DEFAULT_STRING_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(s.c_str(),
                   "operator >>",
                   "OTL_UNICODE_STRING_TYPE&");
   inc_next_ov();
   return *this;
 }
#endif

 otl_stream& operator>>(unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     otl_strcpy
       (OTL_RCAST(unsigned char*,s),
        OTL_RCAST(unsigned char*,
                  OTL_CCAST(char*,OTL_DEFAULT_STRING_NULL_TO_VAL)));
#endif
#if defined(OTL_UNICODE)
   OTL_TRACE_WRITE(OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,s),
                   "operator >>",
                   OTL_UNICODE_CHAR_TYPE_TRACE_NAME "*")
#else
   OTL_TRACE_WRITE(s,"operator >>","unsigned char*")
#endif
   inc_next_ov();
   return *this;
 }

#if defined(OTL_UNICODE)

 otl_stream& operator>>(OTL_UNICODE_CHAR_TYPE& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   OTL_UNICODE_CHAR_TYPE s[1024];
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(OTL_RCAST(unsigned char*,s));
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(OTL_RCAST(unsigned char*,s));
     break;
   }
#if defined(_MSC_VER) && (_MSC_VER==1700)
#pragma warning(push)
#pragma warning (disable:6001)
#endif
   c=s[0];
#if defined(_MSC_VER) && (_MSC_VER==1700)
#pragma warning(pop)
#endif
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   OTL_TRACE_WRITE(c,"operator >>",
                   OTL_UNICODE_CHAR_TYPE_TRACE_NAME "")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(OTL_UNICODE_CHAR_TYPE* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(OTL_RCAST(unsigned char*,s));
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(OTL_RCAST(unsigned char*,s));
     break;
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     otl_strcpy(OTL_RCAST(unsigned char*,s),
                OTL_RCAST(const unsigned char*,OTL_DEFAULT_STRING_NULL_TO_VAL));
#endif
   OTL_TRACE_WRITE(OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,s),
                   "operator >>",
                   OTL_UNICODE_CHAR_TYPE_TRACE_NAME "*")
   inc_next_ov();
   return *this;
 }

#endif

 otl_stream& operator>>(int& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(n);
#else
     (*io)->operator>><int,otl_var_int>(n);
#endif
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(n);
#else
     (*ss)->operator>><int,otl_var_int>(n);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(n,"operator >>","int&")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(unsigned& u)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(u);
#else
     (*io)->operator>><unsigned,otl_var_unsigned_int>(u);
#endif
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(u);
#else
     (*ss)->operator>><unsigned,otl_var_unsigned_int>(u);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     u=OTL_SCAST(unsigned int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(u,"operator >>","unsigned&")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(short& sh)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(sh);
#else
     (*io)->operator>><short,otl_var_short>(sh);
#endif
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(sh);
#else
     (*ss)->operator>><short,otl_var_short>(sh);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     sh=OTL_SCAST(short int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(sh,"operator >>","short int&")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(long int& l)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(l);
#else
     (*io)->operator>><long,otl_var_long_int>(l);
#endif
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(l);
#else
     (*ss)->operator>><long,otl_var_long_int>(l);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     l=OTL_SCAST(long int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(l,"operator >>","long int&")
   inc_next_ov();
   return *this;
 }


#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
 otl_stream& operator>>(OTL_NUMERIC_TYPE_1& l)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_STR_TO_NUMERIC_TYPE_1) && defined(OTL_NUMERIC_TYPE_1_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_out_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_numeric_type_1_str_size];
#if defined(OTL_UNICODE)
           OTL_CHAR unitemp_val[otl_numeric_type_1_str_size];
           (*ss)->operator>>(OTL_RCAST(unsigned char*,unitemp_val));
           OTL_CHAR* uc=unitemp_val;
           char* c=temp_val;
           while(*uc){
             *c=OTL_SCAST(char,*uc);
             ++c; ++uc;
           }
           *c=0;
#else
           (*ss)->operator>>(temp_val);
#endif
           OTL_STR_TO_NUMERIC_TYPE_1(temp_val,l);
         }else{
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*io)->operator>>(l);
#else
           (*io)->operator>><OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1>(l);
#endif
         }
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(l);
#else
     (*io)->operator>><OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1>(l);
#endif
#endif
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_STR_TO_NUMERIC_TYPE_1) && defined(OTL_NUMERIC_TYPE_1_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_out_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_numeric_type_1_str_size];
           (*ss)->operator>>(temp_val);
           OTL_STR_TO_NUMERIC_TYPE_1(temp_val,l);
         }else{
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*ss)->operator>>(l);
#else
           (*ss)->operator>><OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1>(l);
#endif
         }
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(l);
#else
     (*ss)->operator>><OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1>(l);
#endif
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     l=OTL_SCAST(OTL_NUMERIC_TYPE_1,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif

#if defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   // VC ++
   {
     char temp_str[otl_numeric_type_1_str_size];
     OTL_NUMERIC_TYPE_1_TO_STR(l,temp_str);
     OTL_TRACE_WRITE(temp_str,"operator >>", OTL_NUMERIC_TYPE_1_ID "&")
   }
#elif !defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   OTL_TRACE_WRITE(l,"operator >>", OTL_NUMERIC_TYPE_1_ID "&")
#endif
   inc_next_ov();
   return *this;
 }
#endif

#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
 otl_stream& operator>>(OTL_NUMERIC_TYPE_2& l)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_STR_TO_NUMERIC_TYPE_2) && defined(OTL_NUMERIC_TYPE_2_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_out_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_numeric_type_2_str_size];
#if defined(OTL_UNICODE)
           OTL_CHAR unitemp_val[otl_numeric_type_2_str_size];
           (*ss)->operator>>(OTL_RCAST(unsigned char*,unitemp_val));
           OTL_CHAR* uc=unitemp_val;
           char* c=temp_val;
           while(*uc){
             *c=OTL_SCAST(char,*uc);
             ++c; ++uc;
           }
           *c=0;
#else
           (*ss)->operator>>(temp_val);
#endif
           OTL_STR_TO_NUMERIC_TYPE_2(temp_val,l);
         }else{
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*io)->operator>>(l);
#else
           (*io)->operator>><OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2>(l);
#endif
         }
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(l);
#else
     (*io)->operator>><OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2>(l);
#endif
#endif
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_STR_TO_NUMERIC_TYPE_2) && defined(OTL_NUMERIC_TYPE_2_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_out_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_numeric_type_2_str_size];
           (*ss)->operator>>(temp_val);
           OTL_STR_TO_NUMERIC_TYPE_2(temp_val,l);
         }else{
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*ss)->operator>>(l);
#else
           (*ss)->operator>><OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2>(l);
#endif
         }
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(l);
#else
     (*ss)->operator>><OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2>(l);
#endif
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     l=OTL_SCAST(OTL_NUMERIC_TYPE_2,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif

#if defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   // VC ++
   {
     char temp_str[otl_numeric_type_2_str_size];
     OTL_NUMERIC_TYPE_2_TO_STR(l,temp_str);
     OTL_TRACE_WRITE(temp_str,"operator >>", OTL_NUMERIC_TYPE_2_ID "&")
   }
#elif !defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   OTL_TRACE_WRITE(l,"operator >>", OTL_NUMERIC_TYPE_2_ID "&")
#endif
   inc_next_ov();
   return *this;
 }
#endif

#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
 otl_stream& operator>>(OTL_NUMERIC_TYPE_3& l)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_STR_TO_NUMERIC_TYPE_3) && defined(OTL_NUMERIC_TYPE_3_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_out_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_numeric_type_3_str_size];
#if defined(OTL_UNICODE)
           OTL_CHAR unitemp_val[otl_numeric_type_3_str_size];
           (*ss)->operator>>(OTL_RCAST(unsigned char*,unitemp_val));
           OTL_CHAR* uc=unitemp_val;
           char* c=temp_val;
           while(*uc){
             *c=OTL_SCAST(char,*uc);
             ++c; ++uc;
           }
           *c=0;
#else
           (*ss)->operator>>(temp_val);
#endif
           OTL_STR_TO_NUMERIC_TYPE_3(temp_val,l);
         }else{
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*io)->operator>>(l);
#else
           (*io)->operator>><OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3>(l);
#endif
         }
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(l);
#else
     (*io)->operator>><OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3>(l);
#endif
#endif
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_STR_TO_NUMERIC_TYPE_3) && defined(OTL_NUMERIC_TYPE_3_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_out_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_numeric_type_3_str_size];
           (*ss)->operator>>(temp_val);
           OTL_STR_TO_NUMERIC_TYPE_3(temp_val,l);
         }else{
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*ss)->operator>>(l);
#else
           (*ss)->operator>><OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3>(l);
#endif
         }
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(l);
#else
     (*ss)->operator>><OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3>(l);
#endif
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     l=OTL_SCAST(OTL_NUMERIC_TYPE_3,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif

#if defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   // VC ++
   {
     char temp_str[otl_numeric_type_3_str_size];
     OTL_NUMERIC_TYPE_3_TO_STR(l,temp_str);
     OTL_TRACE_WRITE(temp_str,"operator >>", OTL_NUMERIC_TYPE_3_ID "&")
   }
#elif !defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   OTL_TRACE_WRITE(l,"operator >>", OTL_NUMERIC_TYPE_3_ID "&")
#endif
   inc_next_ov();
   return *this;
 }
#endif

#if defined(OTL_BIGINT)
 otl_stream& operator>>(OTL_BIGINT& l)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_STR_TO_BIGINT) && defined(OTL_BIGINT_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_out_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_bigint_str_size];
#if defined(OTL_UNICODE)
           OTL_CHAR unitemp_val[otl_bigint_str_size];
           (*ss)->operator>>(OTL_RCAST(unsigned char*,unitemp_val));
           OTL_CHAR* uc=unitemp_val;
           char* c=temp_val;
           while(*uc){
             *c=OTL_SCAST(char,*uc);
             ++c; ++uc;
           }
           *c=0;
#else
           (*ss)->operator>>(temp_val);
#endif
           OTL_STR_TO_BIGINT(temp_val,l);
         }else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*io)->operator>>(l);
#else
           (*io)->operator>><OTL_BIGINT,otl_var_bigint>(l);
#endif
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(l);
#else
     (*io)->operator>><OTL_BIGINT,otl_var_bigint>(l);
#endif
#endif
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_STR_TO_BIGINT) && defined(OTL_BIGINT_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_out_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_bigint_str_size];
           (*ss)->operator>>(temp_val);
           OTL_STR_TO_BIGINT(temp_val,l);
         }else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*ss)->operator>>(l);
#else
           (*ss)->operator>><OTL_BIGINT,otl_var_bigint>(l);
#endif
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(l);
#else
     (*ss)->operator>><OTL_BIGINT,otl_var_bigint>(l);
#endif
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     l=OTL_SCAST(OTL_BIGINT,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif

#if defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   // VC ++
   {
     char temp_str[otl_bigint_str_size];
     _i64toa(l,temp_str,10);
     OTL_TRACE_WRITE(temp_str,"operator >>","BIGINT&")
   }
#elif !defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   OTL_TRACE_WRITE(l,"operator >>","BIGINT&")
#endif
   inc_next_ov();
   return *this;
 }
#endif

#if defined(OTL_UBIGINT)
 otl_stream& operator>>(OTL_UBIGINT& l)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(l);
#else
     (*io)->operator>><OTL_UBIGINT,otl_var_ubigint>(l);
#endif
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(l);
#else
     (*ss)->operator>><OTL_UBIGINT,otl_var_ubigint>(l);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     l=OTL_SCAST(OTL_UBIGINT,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif

#if defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   // VC ++
   {
     char temp_str[otl_ubigint_str_size];
     _ui64toa(l,temp_str,10);
     OTL_TRACE_WRITE(temp_str,"operator >>","UBIGINT&")
   }
#elif !defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   OTL_TRACE_WRITE(l,"operator >>","UBIGINT&")
#endif
   inc_next_ov();
   return *this;
 }
#endif

 otl_stream& operator>>(float& f)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(f);
#else
     (*io)->operator>><float,otl_var_float>(f);
#endif
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(f);
#else
     (*ss)->operator>><float,otl_var_float>(f);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     f=OTL_SCAST(float,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(f,"operator >>","float&")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(double& d)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(d);
#else
     (*io)->operator>><double,otl_var_double>(d);
#endif
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(d);
#else
     (*ss)->operator>><double,otl_var_double>(d);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     d=OTL_SCAST(double,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(d,"operator >>","double&")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_odbc_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   }
   OTL_TRACE_WRITE(" string length: "<<s.len(),"operator >>","otl_long_string&")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator<<(const char c)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     (*io)->operator<<(c);
     break;
   case otl_odbc_select_stream:
     (*ss)->operator<<(c);
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   OTL_TRACE_READ("'"<<c<<"'","operator <<","char");
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const unsigned char c)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("'"<<c<<"'","operator <<","unsigned char");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     (*io)->operator<<(c);
     break;
   case otl_odbc_select_stream:
     (*ss)->operator<<(c);
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

#if defined(OTL_UNICODE)

 otl_stream& operator<<(const OTL_UNICODE_CHAR_TYPE* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ
     ("\""<<OTL_RCAST(const OTL_UNICODE_CHAR_TYPE*,s)<<"\"",
      "operator <<",
      OTL_UNICODE_CHAR_TYPE_TRACE_NAME "*");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     (*io)->operator<<(OTL_RCAST(const unsigned char*,s));
     break;
   case otl_odbc_select_stream:
     (*ss)->operator<<(OTL_RCAST(const unsigned char*,s));
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const OTL_UNICODE_CHAR_TYPE c)
   OTL_THROWS_OTL_EXCEPTION
 {
   OTL_UNICODE_CHAR_TYPE s[2];
   s[0]=c;
   s[1]=0;
   (*this)<<s;
   return *this;
 }

#endif

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_stream& operator<<(const OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("\""<<s<<"\"","operator <<","OTL_STRING_CONTAINER&");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     (*io)->operator<<(s);
     break;
   case otl_odbc_select_stream:
     (*ss)->operator<<(s);
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

#if defined(OTL_UNICODE_STRING_TYPE)
 otl_stream& operator<<(const OTL_UNICODE_STRING_TYPE& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("\""<<s.c_str()<<"\"","operator <<","OTL_UNICODE_STRING_TYPE&");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     (*io)->operator<<(s);
     break;
   case otl_odbc_select_stream:
     (*ss)->operator<<(s);
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

#if !defined(OTL_UNICODE)
 otl_stream& operator<<(const char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("\""<<s<<"\"","operator <<","char*");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     (*io)->operator<<(s);
     break;
   case otl_odbc_select_stream:
     (*ss)->operator<<(s);
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

 otl_stream& operator<<(const unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
#if defined(OTL_UNICODE)
   OTL_TRACE_READ
     ("\""<<OTL_RCAST(const OTL_UNICODE_CHAR_TYPE*,s)<<"\"",
      "operator <<",
      OTL_UNICODE_CHAR_TYPE_TRACE_NAME "*");
#else
   OTL_TRACE_READ("\""<<s<<"\"","operator <<","unsigned char*");
#endif
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     (*io)->operator<<(s);
     break;
   case otl_odbc_select_stream:
     (*ss)->operator<<(s);
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const int n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(n,"operator <<","int");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(n);
#else
     (*io)->operator<<<int,otl_var_int>(n);
#endif
     break;
   case otl_odbc_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(n);
#else
     (*ss)->operator<<<int,otl_var_int>(n);
#endif
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
 otl_stream& operator<<(const OTL_NUMERIC_TYPE_1& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
#if defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   // VC ++
   {
     char temp_str[otl_numeric_type_1_str_size];
     OTL_NUMERIC_TYPE_1_TO_STR(n,temp_str);
     OTL_TRACE_READ(temp_str,"operator <<",OTL_NUMERIC_TYPE_1_ID)
   }
#elif !defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   OTL_TRACE_READ(n,"operator <<",OTL_NUMERIC_TYPE_1_ID);
#endif
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
#if defined(OTL_STR_TO_NUMERIC_TYPE_1) && defined(OTL_NUMERIC_TYPE_1_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_in_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_numeric_type_1_str_size];
           OTL_NUMERIC_TYPE_1_TO_STR(n,temp_val);
#if defined(OTL_UNICODE)
           OTL_CHAR unitemp_val[otl_numeric_type_1_str_size];
           OTL_CHAR* uc=unitemp_val;
           char* c=temp_val;
           while(*c){
             *uc=OTL_SCAST(OTL_CHAR,*c);
             ++c; ++uc;
           }
           *uc=0;
           (*io)->operator<<(OTL_RCAST(const unsigned char*,unitemp_val));
#else
           (*io)->operator<<(temp_val);
#endif
         }else{
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*io)->operator<<(n);
#else
           (*io)->operator<<<OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1>(n);
#endif
         }
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(n);
#else
     (*io)->operator<<<OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1>(n);
#endif
#endif
     break;
   case otl_odbc_select_stream:
#if defined(OTL_STR_TO_NUMERIC_TYPE_1) && defined(OTL_NUMERIC_TYPE_1_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_in_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_numeric_type_1_str_size];
           OTL_NUMERIC_TYPE_1_TO_STR(n,temp_val);
           (*ss)->operator<<(temp_val);
         }else{
#if 1
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*ss)->operator<<(n);
#else
           (*ss)->operator<<<OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1>(n);
#endif
#endif
         }
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(n);
#else
     (*ss)->operator<<<OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1>(n);
#endif
#endif
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
 otl_stream& operator<<(const OTL_NUMERIC_TYPE_2& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
#if defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   // VC ++
   {
     char temp_str[otl_numeric_type_2_str_size];
     OTL_NUMERIC_TYPE_2_TO_STR(n,temp_str);
     OTL_TRACE_READ(temp_str,"operator <<",OTL_NUMERIC_TYPE_2_ID)
   }
#elif !defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   OTL_TRACE_READ(n,"operator <<",OTL_NUMERIC_TYPE_2_ID);
#endif
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
#if defined(OTL_STR_TO_NUMERIC_TYPE_2) && defined(OTL_NUMERIC_TYPE_2_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_in_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_numeric_type_2_str_size];
           OTL_NUMERIC_TYPE_2_TO_STR(n,temp_val);
#if defined(OTL_UNICODE)
           OTL_CHAR unitemp_val[otl_numeric_type_2_str_size];
           OTL_CHAR* uc=unitemp_val;
           char* c=temp_val;
           while(*c){
             *uc=OTL_SCAST(OTL_CHAR,*c);
             ++c; ++uc;
           }
           *uc=0;
           (*io)->operator<<(OTL_RCAST(const unsigned char*,unitemp_val));
#else
           (*io)->operator<<(temp_val);
#endif
         }else{
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*io)->operator<<(n);
#else
           (*io)->operator<<<OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2>(n);
#endif
         }
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(n);
#else
     (*io)->operator<<<OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2>(n);
#endif
#endif
     break;
   case otl_odbc_select_stream:
#if defined(OTL_STR_TO_NUMERIC_TYPE_2) && defined(OTL_NUMERIC_TYPE_2_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_in_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_numeric_type_2_str_size];
           OTL_NUMERIC_TYPE_2_TO_STR(n,temp_val);
           (*ss)->operator<<(temp_val);
         }else{
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*ss)->operator<<(n);
#else
           (*ss)->operator<<<OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2>(n);
#endif
         }
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(n);
#else
     (*ss)->operator<<<OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2>(n);
#endif
#endif
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
 otl_stream& operator<<(const OTL_NUMERIC_TYPE_3& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
#if defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   // VC ++
   {
     char temp_str[otl_numeric_type_3_str_size];
     OTL_NUMERIC_TYPE_3_TO_STR(n,temp_str);
     OTL_TRACE_READ(temp_str,"operator <<",OTL_NUMERIC_TYPE_3_ID)
   }
#elif !defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   OTL_TRACE_READ(n,"operator <<",OTL_NUMERIC_TYPE_3_ID);
#endif
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
#if defined(OTL_STR_TO_NUMERIC_TYPE_3) && defined(OTL_NUMERIC_TYPE_3_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_in_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_numeric_type_3_str_size];
           OTL_NUMERIC_TYPE_3_TO_STR(n,temp_val);
#if defined(OTL_UNICODE)
           OTL_CHAR unitemp_val[otl_numeric_type_3_str_size];
           OTL_CHAR* uc=unitemp_val;
           char* c=temp_val;
           while(*c){
             *uc=OTL_SCAST(OTL_CHAR,*c);
             ++c; ++uc;
           }
           *uc=0;
           (*io)->operator<<(OTL_RCAST(const unsigned char*,unitemp_val));
#else
           (*io)->operator<<(temp_val);
#endif
         }else{
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*io)->operator<<(n);
#else
           (*io)->operator<<<OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3>(n);
#endif
         }
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(n);
#else
     (*io)->operator<<<OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3>(n);
#endif
#endif
     break;
   case otl_odbc_select_stream:
#if defined(OTL_STR_TO_NUMERIC_TYPE_3) && defined(OTL_NUMERIC_TYPE_3_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_in_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_numeric_type_3_str_size];
           OTL_NUMERIC_TYPE_3_TO_STR(n,temp_val);
           (*ss)->operator<<(temp_val);
         }else{
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*ss)->operator<<(n);
#else
           (*ss)->operator<<<OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3>(n);
#endif
         }
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(n);
#else
     (*ss)->operator<<<OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3>(n);
#endif
#endif
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

#if defined(OTL_BIGINT)
 otl_stream& operator<<(const OTL_BIGINT n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
#if defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   // VC ++
   {
     char temp_str[otl_bigint_str_size];
     _i64toa(n,temp_str,10);
     OTL_TRACE_READ(temp_str,"operator <<","BIGINT")
   }
#elif !defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   OTL_TRACE_READ(n,"operator <<","BIGINT");
#endif
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
#if defined(OTL_STR_TO_BIGINT) && defined(OTL_BIGINT_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_in_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_bigint_str_size];
           OTL_BIGINT_TO_STR(n,temp_val);
#if defined(OTL_UNICODE)
           OTL_CHAR unitemp_val[otl_bigint_str_size];
           OTL_CHAR* uc=unitemp_val;
           char* c=temp_val;
           while(*c){
             *uc=OTL_SCAST(OTL_CHAR,*c);
             ++c; ++uc;
           }
           *uc=0;
           (*io)->operator<<(OTL_RCAST(const unsigned char*,unitemp_val));
#else
           (*io)->operator<<(temp_val);
#endif
         }else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*io)->operator<<(n);
#else
           (*io)->operator<<<OTL_BIGINT,otl_var_bigint>(n);
#endif
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(n);
#else
     (*io)->operator<<<OTL_BIGINT,otl_var_bigint>(n);
#endif
#endif
     break;
   case otl_odbc_select_stream:
#if defined(OTL_STR_TO_BIGINT) && defined(OTL_BIGINT_TO_STR)
     {
       otl_var_desc* var_desc=describe_next_in_var();
       if(var_desc){
         if(var_desc->ftype==otl_var_char){
           char temp_val[otl_bigint_str_size];
           OTL_BIGINT_TO_STR(n,temp_val);
           (*ss)->operator<<(temp_val);
         }else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
           (*ss)->operator<<(n);
#else
           (*ss)->operator<<<OTL_BIGINT,otl_var_bigint>(n);
#endif
       }
     }
#else
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(n);
#else
     (*ss)->operator<<<OTL_BIGINT,otl_var_bigint>(n);
#endif
#endif
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

#if defined(OTL_UBIGINT)
 otl_stream& operator<<(const OTL_UBIGINT n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
#if defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   // VC ++
   {
     char temp_str[otl_ubigint_str_size];
     _ui64toa(n,temp_str,10);
     OTL_TRACE_READ(temp_str,"operator <<","UBIGINT")
   }
#elif !defined(_MSC_VER) && defined(OTL_TRACE_LEVEL)
   OTL_TRACE_READ(n,"operator <<","UBIGINT");
#endif
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(n);
#else
     (*io)->operator<<<OTL_UBIGINT,otl_var_ubigint>(n);
#endif
     break;
   case otl_odbc_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(n);
#else
     (*ss)->operator<<<OTL_UBIGINT,otl_var_ubigint>(n);
#endif
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

 otl_stream& operator<<(const unsigned u)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(u,"operator <<","unsigned int");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(u);
#else
     (*io)->operator<<<unsigned,otl_var_unsigned_int>(u);
#endif
     break;
   case otl_odbc_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(u);
#else
     (*ss)->operator<<<unsigned,otl_var_unsigned_int>(u);
#endif
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const short sh)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(sh,"operator <<","short int");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(sh);
#else
     (*io)->operator<<<short,otl_var_short>(sh);
#endif
     break;
   case otl_odbc_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(sh);
#else
     (*ss)->operator<<<short,otl_var_short>(sh);
#endif
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const long int l)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(l,"operator <<","long int");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(l);
#else
     (*io)->operator<<<long,otl_var_long_int>(l);
#endif
     break;
   case otl_odbc_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(l);
#else
     (*ss)->operator<<<long,otl_var_long_int>(l);
#endif
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const float f)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(f,"operator <<","float");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(f);
#else
     (*io)->operator<<<float,otl_var_float>(f);
#endif
     break;
   case otl_odbc_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(f);
#else
     (*ss)->operator<<<float,otl_var_float>(f);
#endif
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const double d)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(d,"operator <<","double");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(d);
#else
     (*io)->operator<<<double,otl_var_double>(d);
#endif
     break;
   case otl_odbc_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(d);
#else
     (*ss)->operator<<<double,otl_var_double>(d);
#endif
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

  otl_stream& operator<<(const otl_null& n)
    OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("NULL","operator <<","otl_null&");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     (*io)->operator<<(n);
     break;
   case otl_odbc_select_stream:
     (*ss)->operator<<(n);
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const otl_long_string& d)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(" len= "<<d.len(),"operator <<","otl_long_string&");
   switch(shell->stream_type){
   case otl_odbc_no_stream:
     break;
   case otl_odbc_io_stream:
     (*io)->operator<<(d);
     break;
   case otl_odbc_select_stream:
     (*ss)->operator<<(d);
     if(!(*ov)&&(*ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_stream& operator=(const otl_stream&) = delete;
  otl_stream(const otl_stream&) = delete;
#if !defined(OTL_STREAM_NO_PRIVATE_BOOL_OPERATORS)
  otl_stream& operator>>(bool&) = delete;
  otl_stream& operator<<(const bool) = delete;
#endif
#if !defined(OTL_STREAM_NO_PRIVATE_UNSIGNED_LONG_OPERATORS)
  otl_stream& operator>>(unsigned long int&) = delete;
  otl_stream& operator<<(const unsigned long int) = delete;
#endif
private:
#else
  otl_stream& operator=(const otl_stream&)
  {
    return *this;
  }

  otl_stream(const otl_stream&):
   shell(nullptr),
   shell_pt(),
   connected(0),
   ss(nullptr),
   io(nullptr),
   adb(nullptr),
   auto_commit_flag(nullptr),
   iov(nullptr),
   iov_len(nullptr),
   next_iov_ndx(nullptr),
   ov(nullptr),
   ov_len(nullptr),
   next_ov_ndx(nullptr),
   override_(nullptr),
   end_marker(0),
   oper_int_called(0),
   last_eof_rc(0),
   last_oper_was_read_op(false),
   buf_size_(0)
  {
  }

#if !defined(OTL_STREAM_NO_PRIVATE_BOOL_OPERATORS)
  otl_stream& operator>>(bool&)
    OTL_NO_THROW
  {
   return *this;
  }

  otl_stream& operator<<(const bool)
    OTL_NO_THROW
  {
   return *this;
  }

#endif

#if !defined(OTL_STREAM_NO_PRIVATE_UNSIGNED_LONG_OPERATORS)
  otl_stream& operator>>(unsigned long int&)
    OTL_NO_THROW
  {
   return *this;
  }

  otl_stream& operator<<(const unsigned long int)
    OTL_NO_THROW
  {
   return *this;
  }
#endif
#endif

};

inline otl_connect& operator>>(otl_connect& connect, otl_stream& s)
{
  const char* cmd=connect.getCmd();
  const char* invalid_cmd="*** INVALID COMMAND ***";
  if(!cmd)
    cmd=invalid_cmd;
  s.open(s.getBufSize(),cmd,connect);
  return connect;
}

#if (defined(OTL_STL)||defined(OTL_VALUE_TEMPLATE_ON)) && defined(OTL_VALUE_TEMPLATE)
template <OTL_TYPE_NAME TData>
otl_stream& operator<<(otl_stream& s, const otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
 if(var.ind)
  s<<otl_null();
 else
  s<<var.v;
 return s;
}

template <OTL_TYPE_NAME TData>
otl_stream& operator>>(otl_stream& s, otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
  s>>var.v;
  if(s.is_null())
    var.ind=true;
  else
    var.ind=false;
  return s;
}

#endif


class otl_nocommit_stream: public otl_stream{
public:

 otl_nocommit_stream() OTL_NO_THROW
   : otl_stream() 
 {
  set_commit(0);
 }

 otl_nocommit_stream
 (const otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  otl_connect& db,
  const int implicit_select=otl_explicit_select)
   OTL_THROWS_OTL_EXCEPTION
  : otl_stream(arr_size,sqlstm,db,implicit_select)
 {
  set_commit(0);
 }

 void open
 (otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  otl_connect& db,
  const int implicit_select=otl_explicit_select)
   OTL_THROWS_OTL_EXCEPTION
 {
  otl_stream::open(arr_size,sqlstm,db,implicit_select);
  set_commit(0);
 }

};

inline otl_stream& endr(otl_stream& s)
{
  s.check_end_of_row();
  return s;
}

OTL_ODBC_NAMESPACE_END
#endif

// ==================== OTL-Adapter for Oracle 7 =====================
#if defined(OTL_ORA7)

#if defined(OTL_UNICODE)
#error OTL_ORA7 and OTL_UNICODE are incompatible
#endif

#if defined(OTL_ORA_TEXT_ON)
#define text OTL_ORA_TEXT
#endif

extern "C"{
#include <ociapr.h>
}

OTL_ORA7_NAMESPACE_BEGIN

 const int inVarChar2=1;
 const int inNumber=2;
 const int inLong=8;
 const int inRowId=11;
 const int inDate=12;
 const int inRaw=23;
 const int inLongRaw=24;
 const int inChar=96;
 const int inMslabel=106;
 const int inCLOB=112;
 const int inBLOB=113;

 const int  extVarChar2=inVarChar2;
 const int  extNumber=inNumber;
 const int  extInt=3;
 const int  extFloat=4;
 const int  extCChar=5;
 const int  extVarNum=6;
 const int  extLong=inLong;
 const int  extVarChar=9;
 const int  extRowId=inRowId;
 const int  extDate=inDate;
 const int  extVarRaw=15;
 const int  extRaw=extVarRaw;
 const int  extLongRaw=inLongRaw;
 const int  extUInt=68;
 const int  extLongVarChar=94;
 const int  extLongVarRaw=95;
 const int  extChar=inChar;
 const int  extCharZ=97;
 const int  extMslabel=inMslabel;
 const int  extCLOB=inCLOB;
 const int  extBLOB=inBLOB;

typedef otl_oracle_date otl_time0;

class otl_exc{
public:
 unsigned char msg[1000];
 int code;
 char sqlstate[32];

#if defined(OTL_EXCEPTION_ENABLE_ERROR_OFFSET)
  int error_offset;
#endif

#if defined(OTL_EXTENDED_EXCEPTION)
 char** msg_arr;
 char** sqlstate_arr;
 int* code_arr;
 int arr_len;
#endif


 enum{disabled=0,enabled=1};

  otl_exc():
    msg(),
    code(0),
    sqlstate()
#if defined(OTL_EXCEPTION_ENABLE_ERROR_OFFSET)
    ,error_offset(0)
#endif
#if defined(OTL_EXTENDED_EXCEPTION)
    ,msg_arr(nullptr),
    sqlstate_arr(nullptr),
    code_arr(nullptr),
    arr_len(0)
#endif
 {
  sqlstate[0]=0;
  msg[0]=0;
  code=0;
#if defined(OTL_EXTENDED_EXCEPTION)
  msg_arr=nullptr;
  sqlstate_arr=nullptr;
  code_arr=nullptr;
  arr_len=0;
#endif
 }

  virtual ~otl_exc(){}

 void init(const char* amsg, const int acode)
 {
  OTL_STRCPY_S(OTL_RCAST(char*,msg),sizeof(msg),amsg);
  code=acode;
#if defined(OTL_EXTENDED_EXCEPTION)
  msg_arr=nullptr;
  sqlstate_arr=nullptr;
  code_arr=nullptr;
  arr_len=0;
#endif
 }

};

class otl_cur;

class otl_conn{
private:

  friend class otl_cur;
  Lda_Def* lda;
  unsigned char hda[512];
  int extern_lda;

public:

  enum bigint_type
  {
#if defined(OTL_BIGINT) && defined(OTL_ORA_MAP_BIGINT_TO_LONG)
    var_bigint = otl_var_long_int,
    bigint_size = sizeof(long)
#else
    var_bigint = otl_var_char,
    bigint_size = otl_bigint_str_size
#endif
  };

  int get_connection_type(void)
  {
    return 0;
  }

  void cleanup(void){}

 static int initialize(const int threaded_mode=0)
 {
  if(threaded_mode)
   return !opinit(1);
  else
   return 1;
 }

  otl_conn():
    lda(new Lda_Def),
    hda(),
    extern_lda(0)
 {
  memset(lda,0,sizeof(*lda));
  memset(hda,0,sizeof(hda));
 }

 virtual ~otl_conn()
 {
  delete lda;
 }

  void set_timeout(const int /*atimeout*/=0){}
  void set_cursor_type(const int /*acursor_type*/=0){}

 int ext_logon(Lda_Def* ext_lda,const int auto_commit)
 {int rc;
  if(!extern_lda)delete lda;
  lda=ext_lda;
  extern_lda=1;
  if(auto_commit){
   rc=ocon(lda);
   if(rc)
    return 0;
   else
    return 1;
  }
  return 1;
 }

 int rlogon(const char* connect_str,const int auto_commit)
 {
  if(!extern_lda)delete lda;
  OTL_TRACE_RLOGON_ORA7
    (0x1,
     "otl_connect",
     "rlogon",
     connect_str,
     auto_commit)
  lda=new Lda_Def;
  extern_lda=0;
  memset(lda,0,sizeof(*lda));
  memset(hda,0,sizeof(hda));
  int rc=olog(lda,
              hda,
              OTL_RCAST(unsigned char*,OTL_CCAST(char*,connect_str)),
              -1,
              nullptr,
              -1,
              nullptr,
              -1,
              0
             );
  if(rc)return 0;
  if(!auto_commit)return 1;
  rc=ocon(lda);
  if(rc)
   return 0;
  else
   return 1;
 }

 int logoff(void)
 {
  if(extern_lda){
   lda=nullptr;
   extern_lda=0;
   return 1;
  }else{
   if(!lda)return 1;
   if(lda->rc==3113||lda->rc==1041||lda->rc==1033||lda->rc==1034){
    delete lda;
    lda=nullptr;
    return 1;
   }
   int rc=ologof(lda);
   delete lda;
   lda=nullptr;
   return !rc;
  }
 }

 void error(otl_exc& exception_struct)
 {
  if(!lda){
   exception_struct.code=3113;
   OTL_STRCPY_S(OTL_RCAST(char*,exception_struct.msg),
                sizeof(exception_struct.msg),
                "ORA-03113: end-of-file on communication channel"
               );
   return;
  }
  size_t len;
  exception_struct.code=lda->rc;
  oerhms
   (lda,
    lda->rc,
    exception_struct.msg,
    sizeof(exception_struct.msg)
    );
  len = strlen(OTL_RCAST(const char*,exception_struct.msg));
  exception_struct.msg[len]=0;
 }

 int commit(void)
 {
  return !ocom(lda);
 }

 int auto_commit_on(void)
 {
  return !ocon(lda);
 }

 int auto_commit_off(void)
 {
  return !ocof(lda);
 }

 int rollback(void)
 {
  return !orol(lda);
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_conn(const otl_conn&) = delete;
  otl_conn& operator=(const otl_conn&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_conn(otl_conn&&) = delete;
  otl_conn& operator=(otl_conn&&) = delete;
#endif
private:
#else
  otl_conn(const otl_conn&):
    lda(nullptr),
    hda(),
    extern_lda(0)
  {
  }

  otl_conn& operator=(const otl_conn&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_conn(otl_conn&&):
    lda(nullptr),
    hda(),
    extern_lda(0)
  {
  }

  otl_conn& operator=(otl_conn&&)
  {
    return *this;
  }
#endif
#endif

};

class otl_cur;

class otl_var{
private:

  friend class otl_cur;
  ub1* p_v;
  sb2* p_ind;
  ub2* p_rlen;
  ub2* p_rcode;
  int ftype;
  int act_elem_size;
  int array_size;
  ub4 max_tab_len;
  ub4 cur_tab_len;
  int pl_tab_flag;
  int vparam_type;
  int lob_len;
  int lob_pos;
  int lob_ftype;
  int otl_adapter;
  bool lob_stream_mode;
  bool charz_flag;
  int null_ind;

public:

  int get_otl_adapter() const {return otl_adapter;}
  void set_lob_stream_mode(const bool alob_stream_mode)
  {
    lob_stream_mode=alob_stream_mode;
  }

  void set_vparam_type(const int avparam_type)
  {
    vparam_type=avparam_type;
  }

  void set_charz_flag(const bool acharz_flag)
  {
    charz_flag=acharz_flag;
  }

  otl_var():
    p_v(nullptr),
    p_ind(nullptr),
    p_rlen(nullptr),
    p_rcode(nullptr),
    ftype(0),
    act_elem_size(0),
    array_size(0),
    max_tab_len(0),
    cur_tab_len(0),
    pl_tab_flag(0),
    vparam_type(-1),
    lob_len(0),
    lob_pos(0),
    lob_ftype(0),
    otl_adapter(otl_ora7_adapter),
    lob_stream_mode(false),
    charz_flag(false),
    null_ind(0)
 {
 }

 virtual ~otl_var()
 {
  delete[] p_v;
  delete[] p_ind;
  delete[] p_rlen;
  delete[] p_rcode;
 }

  int write_dt(void* trg, const void* src, const int sz)
  {
    memcpy(trg,src,sz);
    return 1;
  }

  int read_dt(void* trg, const void* src, const int sz)
  {
    memcpy(trg,src,sz);
    return 1;
  }

 int actual_elem_size(void)
 {
  return act_elem_size;
 }

 void init
 (const bool,
  const int aftype,
  int& aelem_size,
  const otl_stream_buffer_size_type aarray_size,
  const void* /*connect_struct*/=nullptr,
  const int apl_tab_flag=0)
 {
   int i,elem_size;
   ftype=aftype;
   pl_tab_flag=apl_tab_flag;
   act_elem_size=aelem_size;
   if(aftype==otl_var_varchar_long||aftype==otl_var_raw_long){
     elem_size=aelem_size+sizeof(sb4);
     array_size=1;
   }else if(aftype==otl_var_raw){
     elem_size=aelem_size+sizeof(short int);
     array_size=aarray_size;
   }else{
     elem_size=aelem_size;
     array_size=aarray_size;
   }
   
   p_v=new ub1[elem_size*OTL_SCAST(unsigned,array_size)];
   p_ind=new sb2[array_size];
   p_rlen=new ub2[array_size];
   p_rcode=new ub2[array_size];
   memset(p_v,0,elem_size*OTL_SCAST(unsigned,array_size));
   
   if(aftype==otl_var_varchar_long||aftype==otl_var_raw_long||aftype==otl_var_raw){
     if(aelem_size>otl_short_int_max)
       p_ind[0]=0;
     else
       p_ind[0]=OTL_SCAST(short,aelem_size);
     p_rcode[0]=0;
   }else{
     for(i=0;i<array_size;++i){
       p_ind[i]=OTL_SCAST(short,aelem_size);
       p_rlen[i]=OTL_SCAST(short,elem_size);
       p_rcode[i]=0;
     }
   }
   max_tab_len=OTL_SCAST(ub4,array_size);
   cur_tab_len=OTL_SCAST(ub4,array_size);
   switch(ftype){
   case otl_var_varchar_long:
   case otl_var_raw_long:
     null_ind=0;
     break;
   case otl_var_raw:
     null_ind=OTL_SCAST(int,aelem_size);
     break;
   default:
     null_ind=OTL_SCAST(int,aelem_size);
     break;
   }
 }

 void set_pl_tab_len(const int apl_tab_len)
 {
  max_tab_len=OTL_SCAST(ub4,array_size);
  cur_tab_len=OTL_SCAST(ub4,apl_tab_len);
 }

 int get_pl_tab_len(void)
 {
  return OTL_SCAST(int,cur_tab_len);
 }

 int get_max_pl_tab_len(void)
 {
  return OTL_SCAST(int,max_tab_len);
 }

 int put_blob(void)
 {
  return 1;
 }

 int get_blob
 (const int /* ndx */,
  unsigned char* /* abuf */,
  const int /* buf_size */,
  int& /* len */)
 {
  return 1;
 }

 int save_blob
 (const unsigned char* /* abuf */,
  const int /* len */,
  const int /* extern_buffer_flag */)
 {
  return 1;
 }

 void set_null(int ndx)
 {
  p_ind[ndx]=-1;
 }

  void set_not_null(int ndx, int /*pelem_size*/)
  {
    if(null_ind>otl_short_int_max)
      p_ind[ndx]=0;
    else
      p_ind[ndx]=OTL_SCAST(short,null_ind);

  }

 void set_len(int len, int ndx)
 {
   switch(ftype){
   case otl_var_varchar_long:
   case otl_var_raw_long:
     *OTL_RCAST(sb4*,p_v)=len;
     break;
   default:
     p_rlen[ndx]=OTL_SCAST(short,len);
   }
 }


 int get_len(int ndx)
 {
  if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long)
   return *OTL_RCAST(sb4*,p_v);
  else
   return p_rlen[ndx];
 }

 int is_null(int ndx)
 {
  return p_ind[ndx]==-1;
 }

 void* val(int ndx,int pelem_size)
 {
   switch(ftype){
   case otl_var_varchar_long:
   case otl_var_raw_long:
     return OTL_RCAST(void*,(p_v+sizeof(sb4)));
   case otl_var_raw:
     return OTL_RCAST(void*,&p_v[(OTL_SCAST(unsigned,ndx))*
                                 (pelem_size+sizeof(short int))]);
   default:
     return OTL_RCAST(void*,&p_v[(OTL_SCAST(unsigned,ndx))*pelem_size]);
   }
 }

 static int int2ext(int int_type)
 {
  switch(int_type){
  case inVarChar2: return extCChar;
  case inNumber:   return extFloat;
  case inLong:     return extLongVarChar;
  case inRowId:    return extCChar;
  case inDate:     return extDate;
  case inRaw:      return extRaw;
  case inLongRaw:  return extLongVarRaw;
  case inChar:     return extCChar;
  case inBLOB:   return extFloat;
  default:
   return otl_unsupported_type;
  }
 }

 static int datatype_size(int aftype,int maxsz,int int_type,int max_long_size)
 {
  switch(aftype){
  case extCChar:
   switch(int_type){
   case inRowId:
    return 30;
   case inDate:
    return otl_oracle_date_size;
   case inRaw:
     return max_long_size;
   default:
    return maxsz+1;
   }
  case extLongVarChar:
   return max_long_size;
  case extLongVarRaw:
   return max_long_size;
  case extFloat:
   return sizeof(double);
  case extDate:
   return otl_oracle_date_size;
  case extRaw:
    return maxsz;
  default:
   return 0;
  }
 }

 static void map_ftype
 (otl_column_desc& desc,
  const int max_long_size,
  int& aftype,
  int& elem_size,
  otl_select_struct_override& a_override,
  const int column_ndx,
  const int /*connection_type*/)
 {int ndx=a_override.find(column_ndx);
  if(ndx==-1){
   aftype=int2ext(desc.dbtype);
   elem_size=datatype_size
     (aftype,
      OTL_SCAST(int,desc.dbsize),
      desc.dbtype,
      max_long_size);
   switch(aftype){
   case extCChar:
    aftype=otl_var_char;
    break;
   case extRaw:
    aftype=otl_var_raw;
    break;
   case extFloat:
     if(a_override.get_all_mask() & otl_all_num2str){
     aftype=otl_var_char;
     elem_size=otl_num_str_size;
     }else{
#if defined(OTL_ORA_CUSTOM_MAP_NUMBER_ON_SELECT)
       OTL_ORA_CUSTOM_MAP_NUMBER_ON_SELECT(aftype,elem_size,desc);
#else       
       aftype=otl_var_double;
#endif       
     }
    break;
   case extLongVarChar:
    aftype=otl_var_varchar_long;
    break;
   case extLongVarRaw:
    aftype=otl_var_raw_long;
    break;
   case extDate:
     if(a_override.get_all_mask() & otl_all_date2str){
     aftype=otl_var_char;
     elem_size=otl_date_str_size;
    }else
     aftype=otl_var_timestamp;
    break;
   }
  }else{
    aftype=a_override.get_col_type(ndx);
   switch(aftype){
   case otl_var_char:
     elem_size=a_override.get_col_size(ndx);
     break;
   case otl_var_raw:
     elem_size=a_override.get_col_size(ndx);
     break;
   case otl_var_double:
     elem_size=sizeof(double);
     break;
   case otl_var_bdouble:
     elem_size=sizeof(double);
     break;
   case otl_var_float:
     elem_size=sizeof(float);
     break;
   case otl_var_bfloat:
     elem_size=sizeof(float);
     break;
   case otl_var_int:
     elem_size=sizeof(int);
     break;
   case otl_var_unsigned_int:
     elem_size=sizeof(unsigned);
     break;
   case otl_var_short:
     elem_size=sizeof(short);
     break;
   case otl_var_long_int:
     elem_size=sizeof(long);
     break;
   default:
     elem_size=a_override.get_col_size(ndx);
     break;
   }
  }
  desc.otl_var_dbtype=aftype;
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_var(const otl_var&) = delete;
  otl_var& operator=(const otl_var&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_var(otl_var&&) = delete;
  otl_var& operator=(otl_var&&) = delete;
#endif
private:
#else
  otl_var(const otl_var&):
    p_v(nullptr),
    p_ind(nullptr),
    p_rlen(nullptr),
    p_rcode(nullptr),
    ftype(0),
    act_elem_size(0),
    array_size(0),
    max_tab_len(0),
    cur_tab_len(0),
    pl_tab_flag(0),
    vparam_type(-1),
    lob_len(0),
    lob_pos(0),
    lob_ftype(0),
    otl_adapter(otl_ora7_adapter),
    lob_stream_mode(false),
    charz_flag(false),
    null_ind(0)
 {
 }

 otl_var& operator=(const otl_var&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_var(otl_var&&):
    p_v(nullptr),
    p_ind(nullptr),
    p_rlen(nullptr),
    p_rcode(nullptr),
    ftype(0),
    act_elem_size(0),
    array_size(0),
    max_tab_len(0),
    cur_tab_len(0),
    pl_tab_flag(0),
    vparam_type(-1),
    lob_len(0),
    lob_pos(0),
    lob_ftype(0),
    otl_adapter(otl_ora7_adapter),
    lob_stream_mode(false),
    charz_flag(false),
    null_ind(0)
 {
 }

 otl_var& operator=(otl_var&&)
 {
   return *this;
 }
#endif
#endif

};

class otl_sel;
class otl_ref_cursor;
class otl_ref_select_stream;

class otl_cur{
private:

  friend class otl_sel;
  friend class otl_ref_cursor;
  friend class otl_ref_select_stream;

  Cda_Def cda;
  
  ub4* rpc; // reference to "rows processed count"
  ub2* ft; // reference to "OCI function code"
  ub2* rc; // reference to "V7 return code"
  ub2* peo; // reference to "parse error offset"
  int last_param_data_token;
  int last_sql_param_data_status;
  int sql_param_data_count;
  bool canceled;
  
public:

  void set_canceled(const bool acanceled)
  {
    canceled=acanceled;
  }

  void reset_last_param_data_token()
  {
    last_param_data_token=0;
  }

  void reset_last_sql_param_data_status()
  {
    last_sql_param_data_status=0;
  }

  void reset_sql_param_data_count()
  {
    sql_param_data_count=0;
  }

 otl_cur& operator=(const otl_cur& cur)
 {
   if(this==&cur)return *this;
   *rpc=*cur.rpc;
   *ft=*cur.ft;
   *rc=*cur.rc;
   *peo=*cur.peo;
   memcpy(OTL_RCAST(void*,&cda),
          OTL_RCAST(void*,OTL_CCAST(cda_def *,&cur.cda)),
          sizeof(cda));
   return *this;
 }

 otl_cur():
   cda(),
   rpc(&cda.rpc),
   ft(&cda.ft),
   rc(&cda.rc),
   peo(&cda.peo),
   last_param_data_token(),
   last_sql_param_data_status(0),
   sql_param_data_count(0),
   canceled(false)
 {
  memset(&cda,0,sizeof(cda));
 }

 virtual ~otl_cur(){}

  int open(otl_conn& /* connect */,otl_var* /* var */)
  {
    return 1;
  }

 long get_rpc() OTL_NO_THROW
 {
  return *rpc;
 }

  void set_direct_exec(const int /* flag */){}
  void set_parse_only(const int /*flag*/){}


 int open(otl_conn& connect)
 {
  memset(&cda,0,sizeof(cda));
  return !oopen(&cda,connect.lda,nullptr,-1,-1,nullptr,-1);
 }

 int close(void)
 {
  if(cda.rc==3113||cda.rc==1041||cda.rc==1033||cda.rc==1034)
   return 1;
  return !oclose(&cda);
 }

 int parse(const char* stm_text)
 {
  return !oparse(&cda,OTL_RCAST(unsigned char*,OTL_CCAST(char*,stm_text)),-1,0,1);
 }

  int exec(const int iters, 
           const int /*rowoff*/,
           const int /*otl_sql_exec_from_class*/)
 {
  int temp_rc=oexn(&cda,iters,0);
  return !temp_rc;
 }

 int fetch(const otl_stream_buffer_size_type iters,int& eof_data)
 {int temp_rc=ofen(&cda,iters);
  eof_data=0;
  if(cda.rc==1403){
   eof_data=1;
   return 1;
  }else if(temp_rc==0)
   return 1;
  else
   return 0;
 }

 int tmpl_ftype2ora_ftype(const int ftype)
 {
   switch(ftype){
   case otl_var_char:
     return extCChar;
   case otl_var_double:
     return extFloat;
   case otl_var_float:
     return extFloat;
   case otl_var_int:
     return extInt;
   case otl_var_long_int:
     return extInt;
   case otl_var_unsigned_int:
     return extUInt;
   case otl_var_short:
     return extInt;
   case otl_var_timestamp:
     return extDate;
   case otl_var_varchar_long:
     return extLongVarChar;
   case otl_var_raw_long:
     return extLongVarRaw;
   case otl_var_raw:
     return extRaw;
   default:
     return 0;
   }
 }

 int bind
 (const char* name,
  otl_var& v,
  const int elem_size,
  const int ftype,
  const int /* param_type */,
  const int /* name_pos */,
  const int /*connection_type*/,
  const int apl_tab_flag)
 {
  if(apl_tab_flag)
   return !obndra(&cda,
                  OTL_RCAST(unsigned char*,OTL_CCAST(char*,name)),
                  -1,
                  OTL_RCAST(ub1*,v.p_v),
                  ftype==otl_var_raw?elem_size+sizeof(short):elem_size,
                  v.charz_flag?extCharZ:tmpl_ftype2ora_ftype(ftype),
                  -1,
                  v.p_ind,
                  v.p_rlen,
                  v.p_rcode,
                  v.max_tab_len,
                  &v.cur_tab_len,
                  nullptr,
                  -1,
                  -1);
  else
   return !obndrv
    (&cda,
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,name)),
     -1,
     OTL_RCAST(ub1*,v.p_v),
     ftype==otl_var_raw?elem_size+sizeof(short):elem_size,
     tmpl_ftype2ora_ftype(ftype),
     -1,
     v.p_ind,
     nullptr,
     -1,
     -1);
 }

 int bind
 (const int column_num,
  otl_var& v,
  const int elem_size,
  const int ftype,
  const int /* param_type */)
 {
  return !odefin
   (&cda,
    column_num,
    OTL_RCAST(ub1*,v.p_v),
    ftype==otl_var_raw?elem_size+sizeof(short):elem_size,
    tmpl_ftype2ora_ftype(ftype),
    -1,
    v.p_ind,
    nullptr,
    -1,
    -1,
    v.p_rlen,
    v.p_rcode);
 }

 int describe_column
 (otl_column_desc& col,
  const int column_num,
  int& eof_desc)
 {
  sb1  name[241];
  sb4  nlen;
  sb4  dbsize;
  sb2  dbtype;

  sb2  scale;
  sb2  prec;
  sb4  dsize;
  sb2  nullok;

  nlen=sizeof(name)-1;
  int temp_rc=odescr
   (&cda,
    column_num,
    &dbsize,
    &dbtype,
    &name[0],
    &nlen,
    &dsize,
    &prec,
    &scale,
    &nullok);
  if(temp_rc==0)name[nlen]=0;
  eof_desc=0;
  if(cda.rc==1007){
   eof_desc=1;
   return 0;
  }
  if(temp_rc==0){
    col.set_name(OTL_RCAST(char*,name));
   col.dbtype=dbtype;
   col.dbsize=dbsize;
   col.scale=scale;
   col.prec=prec;
   col.nullok=nullok;
   return 1;
  }else
   return 0;
 }

 void error(otl_exc& exception_struct)
 {
  size_t len;
  exception_struct.code=cda.rc;
  oerhms
   (&cda,
    cda.rc,
    exception_struct.msg,
    sizeof(exception_struct.msg)
    );
  len=strlen(OTL_RCAST(const char*,exception_struct.msg));
  exception_struct.msg[len]=0;
#if defined(OTL_EXCEPTION_ENABLE_ERROR_OFFSET)
  ub2 error_offset=cda.peo;
  exception_struct.error_offset=OTL_SCAST(int,error_offset);
#endif
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_cur(const otl_cur&) = delete;
private:
#else
 otl_cur(const otl_cur&):
   cda(),
   rpc(&cda.rpc),
   ft(&cda.ft),
   rc(&cda.rc),
   peo(&cda.peo),
   last_param_data_token(),
   last_sql_param_data_status(0),
   sql_param_data_count(0),
   canceled(false)
 {
 }
#endif

};

class otl_sel{
private:

 int implicit_cursor;

public:

  int get_implicit_cursor() const {return implicit_cursor;}

 void set_arr_size
 (const int input_arr_size,
  int& out_array_size,
  int& out_prefetch_array_size)
 {
   out_array_size=input_arr_size;
   out_prefetch_array_size=0;
 }

  void set_prefetch_size(const int /*aprefetch_array_size*/)
  {
  }

  int close_select(otl_cur& /* cur */)
  {
    int rc=1;
    return rc;
  }

 otl_sel():
   implicit_cursor(0)
 {
 }

 virtual ~otl_sel(){}

  void set_select_type(const int /* atype */)
 {
  implicit_cursor=0;
 }

  void init(const int /* array_size */){}

 int first
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
  const int array_size)
 {int rc;
  eof_data=0;
  cur_row=-1;
  rc=cur.exec(1,0,otl_sql_exec_from_select_cursor_class);
  if(rc==0)return 0;
  rc=cur.fetch(OTL_SCAST(otl_stream_buffer_size_type,array_size),eof_data);
  if(rc==0)return 0;
  row_count=*cur.rpc;
  cur_size=row_count;
  if(cur_size!=0)cur_row=0;
  return 1;
 }

 int next
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
  const int array_size)
 {int rc;
  if(cur_row<cur_size-1){
   ++cur_row;
   return 1;
  }else{
   if(eof_data){
    cur_row=-1;
    cur_size=0;
    return 1;
   }
   rc=cur.fetch(OTL_SCAST(otl_stream_buffer_size_type,array_size),eof_data);
   if(rc==0)return 0;
   cur_size=*cur.rpc-row_count;
   row_count=*cur.rpc;
   if(cur_size!=0)cur_row=0;
   return 1;
  }
 }

};

typedef otl_tmpl_connect
  <otl_exc,
   otl_conn,
   otl_cur> otl_ora7_connect;

typedef otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> otl_cursor;

typedef otl_tmpl_exception
  <otl_exc,
   otl_conn,
   otl_cur> otl_exception;

typedef otl_tmpl_inout_stream
 <otl_exc,
  otl_conn,
  otl_cur,
  otl_var,
  otl_time0> otl_inout_stream;

typedef otl_tmpl_select_stream
 <otl_exc,
  otl_conn,
  otl_cur,
  otl_var,
  otl_sel,
  otl_time0> otl_select_stream;


typedef otl_tmpl_ext_hv_decl
 <otl_var,
  otl_time0,
  otl_exc,
  otl_conn,
  otl_cur> otl_ext_hv_decl;

class otl_stream_shell;

class otl_connect: public otl_ora7_connect{
public:

#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
  otl_stream_pool sc;
  bool pool_enabled_;

 void set_stream_pool_size(const int max_size=otl_max_default_pool_size)
 {
  sc.init(max_size);
 }

  void stream_pool_enable()
  {
    pool_enabled_=true;
  }

  void stream_pool_disable()
  {
    pool_enabled_=false;
  }

  bool get_stream_pool_enabled_flag() const
  {
    return pool_enabled_;
  }

#endif

  long direct_exec
  (const char* sqlstm,
   const int exception_enabled=1)
   OTL_THROWS_OTL_EXCEPTION
  {
    return otl_cursor::direct_exec(*this,sqlstm,exception_enabled);
  }

  void syntax_check(const char* sqlstm)
   OTL_THROWS_OTL_EXCEPTION
  {
    otl_cursor::syntax_check(*this,sqlstm);
  }

 otl_connect() OTL_NO_THROW :
   otl_ora7_connect(), cmd_(nullptr)
  {
  }

 otl_connect(const char* connect_str, const int aauto_commit=0)
   OTL_THROWS_OTL_EXCEPTION
   : otl_ora7_connect(connect_str, aauto_commit), cmd_(nullptr)
  {
  }

  virtual ~otl_connect() 
  {
    if(cmd_){
      delete[] cmd_;
      cmd_=nullptr;
    }
#if defined(OTL_DESTRUCTORS_DO_NOT_THROW)
    try{
      logoff();
    }catch(OTL_CONST_EXCEPTION otl_exception&){
    }
#endif
  }

  void rlogon(Lda_Def* alda)
   OTL_THROWS_OTL_EXCEPTION
  {
    if(this->connected){
     throw otl_exception(otl_error_msg_30,otl_error_code_30);
    }
    if(cmd_){
      delete[] cmd_;
      cmd_=nullptr;
    }
    connected=0;
    long_max_size=otl_short_int_max;
    retcode=connect_struct.ext_logon(alda,0);
    if(retcode)
      connected=1;
    else{
      connected=0;
      increment_throw_count();
      if(get_throw_count()>1)return;
      if(otl_uncaught_exception()) return; 
      throw otl_exception(connect_struct);
    }
  }

 void rlogon(const char* connect_str, const int aauto_commit=0)
   OTL_THROWS_OTL_EXCEPTION
 {
    if(this->connected){
     throw otl_exception(otl_error_msg_30,otl_error_code_30);
    }
    otl_ora7_connect::rlogon(connect_str,aauto_commit);
 }

 void logoff(void)
 {
#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
  if(connected)
    sc.init(sc.get_max_size());
#endif
#if defined(OTL_ROLLS_BACK_BEFORE_LOGOFF)
  otl_ora7_connect::rollback();
#endif
  otl_ora7_connect::logoff();
 }

  const char* getCmd(void) const
  {
    return cmd_;
  }

  otl_connect& operator<<(const char* cmd)
  {
    if(!connected){
      this->rlogon(cmd);
    }else{
      otl_cursor::direct_exec(*this,cmd);
    }
    return *this;
  }

  otl_connect& operator<<=(const char* cmd)
  {
    if(cmd_){
      delete[] cmd_;
      cmd_=nullptr;
    }
    size_t cmd_len=strlen(cmd);
    cmd_=new char[cmd_len+1];
    OTL_STRCPY_S(cmd_,cmd_len+1,cmd);
    return *this;
  }
  
private:

  char* cmd_;
  
#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_connect& operator=(const otl_connect&) = delete;
  otl_connect(const otl_connect&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_connect& operator=(otl_connect&&) = delete;
  otl_connect(otl_connect&&) = delete;
#endif
private:
#else
  otl_connect& operator=(const otl_connect&)
  {
    return *this;
  }

  otl_connect(const otl_connect&)
    :otl_ora7_connect(),cmd_(nullptr){}

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_connect& operator=(otl_connect&&)
  {
    return *this;
  }

  otl_connect(otl_connect&&):otl_ora7_connect(),cmd_(nullptr){}
#endif
#endif

};


// ============ OTL Reference Cursor Streams for Oracle 7 =================

typedef otl_tmpl_variable<otl_var> otl_generic_variable;
typedef otl_generic_variable* otl_p_generic_variable;

class otl_ref_select_stream;

class otl_ref_cursor: public
otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>{
private:

  friend class otl_ref_select_stream;
  int cur_row;
  int cur_size;
  int row_count;
  int array_size;
  otl_select_struct_override local_override;
  
public:

 otl_ref_cursor
 (otl_connect& db,
  const char* cur_placeholder_name,
  void* master_stream_ptr,
  const otl_stream_buffer_size_type arr_size=1)
  :otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(db),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(arr_size),
   local_override(),
   sel_cur(),
   rvl_len(otl_var_list_size),
   rvl(new otl_p_generic_variable[rvl_len]),
   vl_cur_len(0),
   cur_placeholder(),
   master_stream_ptr_(master_stream_ptr)
 {
  local_override.reset();
  for(int i=0;i<rvl_len;++i)
    rvl[i]=nullptr;
  OTL_STRCPY_S(cur_placeholder,sizeof(cur_placeholder),cur_placeholder_name);
 }

 otl_ref_cursor():
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(0),
   local_override(),
   sel_cur(),
   rvl_len(0),
   rvl(nullptr),
   vl_cur_len(0),
   cur_placeholder(),
   master_stream_ptr_(nullptr)
 {
   local_override.reset();
 }

 virtual ~otl_ref_cursor()
 {
  this->in_destructor=1;
  delete[] rvl;
  rvl=nullptr;
 }

 void open
 (otl_connect& db,
  const char* cur_placeholder_name,
  const otl_stream_buffer_size_type arr_size=1)
 {
  int i;
  local_override.reset();
  cur_row=-1;
  row_count=0;
  cur_size=0;
  array_size=arr_size;
  rvl_len=otl_var_list_size;
  vl_cur_len=0;
  rvl=new otl_p_generic_variable[rvl_len];
  for(i=0;i<rvl_len;++i)rvl[i]=nullptr;
  OTL_STRCPY_S(cur_placeholder,sizeof(cur_placeholder),cur_placeholder_name);
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::open(db);
 }

 void close(void)
 {
   local_override.reset();
   delete[] rvl;
   rvl=nullptr;
   if(sel_cur.get_connected() && sel_cur.get_adb()==nullptr)
     sel_cur.set_adb(adb);
   sel_cur.close();
   otl_tmpl_cursor
     <otl_exc,
     otl_conn,
     otl_cur,
     otl_var>::close();
 }

 int first(void)
 {int i,rc;
  rc=obndrv
   (&this->cursor_struct.cda,
    OTL_RCAST(unsigned char*,cur_placeholder),
    -1,
    OTL_RCAST(ub1*,&sel_cur.get_cursor_struct_ref().cda),
    sizeof(sel_cur.get_cursor_struct_ref().cda),
    102,-1,nullptr,nullptr,-1,-1);
  if(rc!=0){
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return 0;
  if(otl_uncaught_exception()) return 0; 
   throw otl_exception(cursor_struct,stm_label?stm_label:stm_text);
  }
  if(cur_row==-2)
   ; // Special case -- calling describe_select() between parse() and first()
  else{
// Executing the PLSQL master block
    exec(1,0,otl_sql_exec_from_select_cursor_class); 
   sel_cur.set_connected(1);
  }
  cur_row=-1;
  for(i=0;i<vl_cur_len;++i)
   sel_cur.bind(i+1,*rvl[i]);
  rc=sel_cur.get_cursor_struct_ref().fetch
   (OTL_SCAST(otl_stream_buffer_size_type,array_size),
    sel_cur.get_eof_data_ref());
  if(rc==0){
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return 0;
  if(otl_uncaught_exception()) return 0; 
  throw otl_exception(sel_cur.get_cursor_struct_ref(),stm_label?stm_label:stm_text);
  }
  row_count=sel_cur.get_cursor_struct_ref().cda.rpc;
  OTL_TRACE_FIRST_FETCH
  cur_size=row_count;
  if(cur_size!=0)cur_row=0;
  return cur_size!=0;
 }

 int next(void)
 {int rc;
  if(cur_row<0)return first();
  if(cur_row<cur_size-1)
   ++cur_row;
  else{
    if(sel_cur.eof()){
      cur_row=-1;
      return 0;
    }
    rc=sel_cur.get_cursor_struct_ref().fetch
      (OTL_SCAST(otl_stream_buffer_size_type,array_size),
       sel_cur.get_eof_data_ref());
    if(rc==0){
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return 0;
      if(otl_uncaught_exception()) return 0; 
      throw otl_exception(sel_cur.get_cursor_struct_ref(),
                          stm_label?stm_label:stm_text);
    }
    cur_size=sel_cur.get_cursor_struct_ref().cda.rpc-row_count;
    row_count=sel_cur.get_cursor_struct_ref().cda.rpc;
    OTL_TRACE_NEXT_FETCH2
    if(cur_size!=0)cur_row=0;
  }
  return cur_size!=0;
 }

 void bind
 (const int column_num,
  otl_generic_variable& v)
 {
  if(!connected)return;
  ++vl_cur_len;
  if(vl_cur_len==rvl_len){
    int temp_rvl_len=rvl_len*2;
    otl_p_generic_variable* temp_rvl=
      new otl_p_generic_variable[temp_rvl_len];
    int i;
    for(i=0;i<rvl_len;++i)
      temp_rvl[i]=rvl[i];
    for(i=rvl_len+1;i<temp_rvl_len;++i)
      temp_rvl[i]=nullptr;
    delete[] rvl;
    rvl=temp_rvl;
    rvl_len=temp_rvl_len;
  }
  rvl[vl_cur_len-1]=&v;
  v.set_pos(column_num);
 }

 void bind(otl_generic_variable& v)
 {
   if(v.get_pos())
     bind(v.get_pos(),v);
   else if(v.get_name())
     otl_tmpl_cursor
       <otl_exc,
     otl_conn,
     otl_cur,
     otl_var>::bind(v);
 }

 void bind
 (const char* name,
  otl_generic_variable& v)
 {
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::bind(name,v);
 }

 int describe_select
 (otl_column_desc* desc,
  int& desc_len)
 {int i,rc;
  rc=obndrv
   (&cursor_struct.cda,
    OTL_RCAST(unsigned char*,cur_placeholder),
    -1,
    OTL_RCAST(ub1*,&sel_cur.get_cursor_struct_ref().cda),
    sizeof(sel_cur.get_cursor_struct_ref().cda),
    102,-1,nullptr,nullptr,-1,-1);
  if(rc!=0){
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return 0;
   if(otl_uncaught_exception()) return 0; 
   throw otl_exception(cursor_struct,stm_label?stm_label:stm_text);
  }
// Executing the PLSQL master block
  exec(1,0,otl_sql_exec_from_select_cursor_class); 
  sel_cur.set_connected(1);
  cur_row=-2; // Special case -- describe_select() before first() or next()
  desc_len=0;
  for(i=1;sel_cur.describe_column(desc[i-1],i);++i)
   ++desc_len;
  return 1;
 }


protected:

  otl_cursor sel_cur;
  int rvl_len;
  otl_p_generic_variable* rvl;
  int vl_cur_len;
  char cur_placeholder[64];
  void* master_stream_ptr_;

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_ref_cursor(const otl_ref_cursor&) = delete;
 otl_ref_cursor& operator=(const otl_ref_cursor&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_ref_cursor(otl_ref_cursor&&) = delete;
 otl_ref_cursor& operator=(otl_ref_cursor&&) = delete;
#endif
private:
#else
 otl_ref_cursor(const otl_ref_cursor&) :
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(0),
   local_override(),
   sel_cur(),
   rvl_len(0),
   rvl(nullptr),
   vl_cur_len(0),
  cur_placeholder(),
  master_stream_ptr_(nullptr)
 {
 }

 otl_ref_cursor& operator=(const otl_ref_cursor&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_ref_cursor(otl_ref_cursor&&) :
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(0),
   local_override(),
   sel_cur(),
   rvl_len(0),
   rvl(nullptr),
   vl_cur_len(0),
  cur_placeholder(),
  master_stream_ptr_(nullptr)
 {
 }

 otl_ref_cursor& operator=(otl_ref_cursor&&)
 {
   return *this;
 }
#endif
#endif

};

class otl_stream;

class otl_ref_select_stream: public otl_ref_cursor{
protected:

 otl_column_desc* sl_desc;
 int sl_len;
 otl_generic_variable* sl;
 int null_fetched;
 int ret_code;
 int cur_col;
 int cur_in;
 int executed;
 char var_info[256];

private:

  friend class otl_stream;
  otl_select_struct_override* override_;
  long _rfc;

public:

  int get_select_row_count() const 
  {
    return this->cur_row==-1?0:this->cur_size-this->cur_row;
  }

  int get_prefetched_row_count() const {return this->row_count;}

 void cleanup(void)
 {int i;
  delete[] sl;
  for(i=0;i<vl_len;++i)
   delete vl[i];
  delete[] vl;
  delete[] sl_desc;
 }

 otl_ref_select_stream
 (otl_select_struct_override* aoverride,
  const otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  const char* acur_placeholder,
  otl_connect& db,
  const char* sqlstm_label=nullptr)
  :otl_ref_cursor
   (db,
    acur_placeholder,
    aoverride->get_master_stream_ptr(),
    arr_size),
   sl_desc(nullptr),
   sl_len(0),
   sl(nullptr),
   null_fetched(0),
   ret_code(0),
   cur_col(0),
   cur_in(0),
   executed(0),
   var_info(),
   override_(nullptr),
   _rfc(0)
 {
   if(sqlstm_label!=nullptr){
     if(stm_label!=nullptr){
       delete[] stm_label;
       stm_label=nullptr;
     }
     size_t len=strlen(sqlstm_label)+1;
     stm_label=new char[len];
     OTL_STRCPY_S(stm_label,len,sqlstm_label);
   }
  _rfc=0;
  init();
  {
   size_t len=strlen(sqlstm)+1;
   stm_text=new char[len];
   OTL_STRCPY_S(stm_text,len,sqlstm);
   otl_select_struct_override* temp_local_override=&this->local_override;
   otl_ext_hv_decl hvd
     (this->stm_text,
      1,
      this->stm_label,
      &temp_local_override,
      adb
     );
   hvd.alloc_host_var_list(vl,vl_len,*adb);
   if(temp_local_override!=&this->local_override)
     delete temp_local_override;
  }
  override_=aoverride;
  try{
   parse();
   if(vl_len==0){
    rewind();
    null_fetched=0;
   }
  }catch(OTL_CONST_EXCEPTION otl_exception&){
   cleanup();
   if(this->adb)this->adb->increment_throw_count();
   throw;
  }

 }

 virtual ~otl_ref_select_stream()
 {
  cleanup();
 }

 void rewind(void)
 {
   OTL_TRACE_STREAM_EXECUTION
  _rfc=0;
  get_select_list();
  ret_code=first();
  null_fetched=0;
  cur_col=-1;
  cur_in=0;
  executed=1;
 }

  void clean(void)
  {
    _rfc=0;
    null_fetched=0;
    cur_col=-1;
    cur_in=0;
    executed=0;
  }

 int is_null(void)
 {
  return null_fetched;
 }

 int eof(void)
 {
  return !ret_code;
 }

 void skip_to_end_of_row()
 {
   check_if_executed();
   if(eof())return;
   while(cur_col<sl_len-1){
     ++cur_col;
     null_fetched=sl[cur_col].is_null(this->cur_row);
   }
   ret_code=this->next();
   cur_col=0;
   if(!eof())
     cur_col=-1;
  ++_rfc;
 }


 otl_ref_select_stream& operator>>(otl_time0& t)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(check_type(otl_var_timestamp)&&!eof()){
   otl_time0* tm=OTL_RCAST(otl_time0*,sl[cur_col].val(cur_row));
   memcpy(OTL_RCAST(void*,&t),tm,otl_oracle_date_size);
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(char& c)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof()){
   c=*OTL_RCAST(char*,sl[cur_col].val(cur_row));
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(unsigned char& c)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof()){
   c=*OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
   look_ahead();
  }
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_ref_select_stream& operator>>(OTL_STRING_CONTAINER& s)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  switch(sl[cur_col].get_ftype()){
  case otl_var_raw:
    {
      if(!eof()){
        unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
        int len=OTL_SCAST(int,*OTL_RCAST(unsigned short*,c));
        c+=sizeof(short int);
#if (defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)) && !defined(OTL_ACE)
        s.assign(OTL_RCAST(char*,c),len);
#elif defined(OTL_ACE)
        s.set(OTL_RCAST(char*,c),len,1);
#endif
        look_ahead();
      }  
    }
    break;
  case otl_var_char:
    if(!eof()){
#if defined(OTL_ACE)
      s.set(OTL_RCAST(char*,sl[cur_col].val(cur_row)),1);
#else
      s=OTL_RCAST(char*,sl[cur_col].val(cur_row));
#endif
      look_ahead();
    }
    break;
#if defined(USER_DEFINED_STRING_CLASS) || \
    defined(OTL_STL) || defined(OTL_ACE)
  case otl_var_varchar_long:
  case otl_var_raw_long:
    if(!eof()){
      unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
      int len=sl[cur_col].get_len(cur_row);
#if (defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)) && !defined(OTL_ACE)
      s.assign(OTL_RCAST(char*,c),len);
#elif defined(OTL_ACE)
      s.set(OTL_RCAST(char*,c),len,1);
#endif
      look_ahead();
    }
    break;
#endif
  default:
    check_type(otl_var_char);
  } // switch
  return *this;
 }
#endif

 otl_ref_select_stream& operator>>(char* s)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof()){
   otl_strcpy(OTL_RCAST(unsigned char*,s),
              OTL_RCAST(const unsigned char*,sl[cur_col].val(cur_row)));
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(unsigned char* s)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof()){
   otl_strcpy2(OTL_RCAST(unsigned char*,s),
               OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row)),
               sl[cur_col].get_len(cur_row)
             );
   look_ahead();
  }
  return *this;
 }

#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
#define OTL_D5(T,T_type)                                \
  otl_ref_select_stream& operator>>(T& n)               \
  {                                                     \
    check_if_executed();                                \
    if(eof())return *this;                              \
    get_next();                                         \
    if(!eof()){                                         \
      int match_found=otl_numeric_convert_T<T,T_type>   \
        (sl[cur_col].get_ftype(),                       \
         sl[cur_col].val(cur_row),                      \
         n);                                            \
      if(!match_found)                                  \
        strict_check_throw(T_type);                     \
      look_ahead();                                     \
    }                                                   \
    return *this;                                       \
  }
#else
#define OTL_D5(T,T_type)                                        \
  otl_ref_select_stream& operator>>(T& n)                       \
  {                                                             \
    check_if_executed();                                        \
    if(eof())return *this;                                      \
    get_next();                                                 \
    if(!eof()){                                                 \
      int match_found=otl_numeric_convert_T                     \
        (sl[cur_col].get_ftype(),                               \
         sl[cur_col].val(cur_row),                              \
         n);                                                    \
      if(!match_found){                                         \
        if(check_type(otl_var_double,T_type))                   \
          n=OTL_PCONV(T,double,sl[cur_col].val(cur_row));       \
      }                                                         \
      look_ahead();                                             \
    }                                                           \
    return *this;                                               \
  }
#endif

#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  OTL_D5(int,otl_var_int)
  OTL_D5(unsigned,otl_var_unsigned_int)
  OTL_D5(long,otl_var_long_int)
  OTL_D5(short,otl_var_short)
  OTL_D5(float,otl_var_float)
  OTL_D5(double,otl_var_double)
#else
    template<OTL_TYPE_NAME T,const int T_type> OTL_D5(T,T_type)
#endif

 otl_ref_select_stream& operator>>(otl_long_string& s)
 {
  check_if_executed();
  if(eof())return *this;
  get_next();
  switch(sl[cur_col].get_ftype()){
  case otl_var_varchar_long: 
  case otl_var_raw_long:
    {
      bool in_unicode_mode=sizeof(OTL_CHAR)>1;
      if(!s.get_unicode_flag() && in_unicode_mode &&
         sl[cur_col].get_ftype()==otl_var_varchar_long){
        throw otl_exception
          (otl_error_msg_37,
           otl_error_code_37,
           this->stm_label?this->stm_label:
           this->stm_text);
      }else if(s.get_unicode_flag() && sl[cur_col].get_ftype()==otl_var_raw_long){
        throw otl_exception
          (otl_error_msg_38,
           otl_error_code_38,
           this->stm_label?this->stm_label:
           this->stm_text);
      }
      if(!eof()){
        unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
        int len=sl[cur_col].get_len(cur_row);
        if(len>s.get_buf_size())
          len=s.get_buf_size();
        otl_memcpy(s.v,c,len,sl[cur_col].get_ftype());
        if(sl[cur_col].get_ftype()==otl_var_varchar_long)
          s.null_terminate_string(len);
        s.set_len(len);
        look_ahead();
      }
    }
    break;
  case otl_var_raw:
    {
      if(s.get_unicode_flag()){
        throw otl_exception
          (otl_error_msg_38,
           otl_error_code_38,
           this->stm_label?this->stm_label:
           this->stm_text);
      }      
      if(!eof()){
        unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
        int len=OTL_SCAST(int,*OTL_RCAST(unsigned short*,c));
        if(len>s.get_buf_size())
          len=s.get_buf_size();
        otl_memcpy(s.v,c+sizeof(short int),len,sl[cur_col].get_ftype());
        s.set_len(len);
        look_ahead();
      }
    }
    break;
  }
  return *this;
 }


 otl_ref_select_stream& operator<<(const otl_time0& t)
 {
  check_in_var();
  if(check_in_type(otl_var_timestamp,otl_oracle_date_size)){
   otl_time0* tm=OTL_RCAST(otl_time0*,vl[cur_in]->val());
   memcpy(tm,OTL_RCAST(void*,OTL_CCAST(otl_time0*,&t)),otl_oracle_date_size);
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }


  otl_ref_select_stream& operator<<(const otl_null& /* n */)
 {
  check_in_var();
  this->vl[cur_in]->set_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const otl_long_string& s)
 {
   if(s.get_unicode_flag()){
     throw otl_exception
       (otl_error_msg_38,
        otl_error_code_38,
        this->stm_label?this->stm_label:
        this->stm_text);
   }
   check_in_var();
   if(check_in_type(otl_var_raw,1)){
      unsigned char* c=OTL_RCAST(unsigned char*,vl[cur_in]->val());
      int len=OTL_CCAST(otl_long_string*,&s)->len();
      if(len>this->vl[cur_in]->actual_elem_size()){
        otl_var_info_var
          (this->vl[cur_in]->get_name(),
           this->vl[cur_in]->get_ftype(),
           otl_var_raw,
           var_info,
           sizeof(var_info));
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
        throw otl_exception
          (otl_error_msg_5,
           otl_error_code_5,
           this->stm_label?this->stm_label:
           this->stm_text,
           var_info);
      }
      this->vl[cur_in]->set_not_null(0);
      otl_memcpy
        (c+sizeof(unsigned short),
         s.v,
         len,
         this->vl[cur_in]->get_ftype());
      *OTL_RCAST(unsigned short*,
                 this->vl[cur_in]->val(0))=OTL_SCAST(unsigned short,len);
      this->vl[cur_in]->set_len(len,0);
   }
   get_in_next();
   return *this;
 }


 otl_ref_select_stream& operator<<(const char c)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){
   char* tmp=OTL_RCAST(char*,vl[cur_in]->val());
   tmp[0]=c;
   tmp[1]=0;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const unsigned char c)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){
   unsigned char* tmp=OTL_RCAST(unsigned char*,vl[cur_in]->val());
   tmp[0]=c;
   tmp[1]=0;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }


#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_ref_select_stream& operator<<(const OTL_STRING_CONTAINER& s)
 {
  check_in_var();
  if(this->vl[cur_in]->get_ftype()==otl_var_raw){
    unsigned char* c=OTL_RCAST(unsigned char*,vl[cur_in]->val());
    int len=OTL_SCAST(int,s.length());
    if(len>this->vl[cur_in]->actual_elem_size()){
      otl_var_info_var
        (this->vl[cur_in]->get_name(),
         this->vl[cur_in]->get_ftype(),
         otl_var_raw,
         var_info,
         sizeof(var_info));
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return *this;
      if(otl_uncaught_exception()) return *this; 
      throw otl_exception
        (otl_error_msg_5,
         otl_error_code_5,
         this->stm_label?this->stm_label:
         this->stm_text,
         var_info);
    }
    this->vl[cur_in]->set_not_null(0);
    otl_memcpy
      (c+sizeof(unsigned short),
       OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
       len,
       this->vl[cur_in]->get_ftype());
    *OTL_RCAST(unsigned short*,
               this->vl[cur_in]->val(0))=OTL_SCAST(unsigned short,len);
    this->vl[cur_in]->set_len(len,0);
  }else if(this->vl[cur_in]->get_ftype()==otl_var_char){
   int overflow;
   otl_strcpy
    (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
     overflow,
     vl[cur_in]->get_elem_size(),
     OTL_SCAST(int,s.length())
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
      (vl[cur_in]->get_name(),
       vl[cur_in]->get_ftype(),
      otl_var_char,
      temp_var_info,
      sizeof(temp_var_info));
    if(this->adb&&this->adb->get_throw_count()>1)return *this;
    if(this->adb)this->adb->increment_throw_count();
    if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
    throw otl_exception
      (otl_error_msg_4,
       otl_error_code_4,
       stm_label?stm_label:stm_text,
       temp_var_info,
       OTL_RCAST(const void*,s.c_str()),
       OTL_SCAST(int,vl[cur_in]->get_elem_size()));
#else
    throw otl_exception
     (otl_error_msg_4,
      otl_error_code_4,
      stm_label?stm_label:stm_text,
      temp_var_info);
#endif
   }
  }else
    check_in_type_throw(otl_var_char);
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }
#endif

 otl_ref_select_stream& operator<<(const char* s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy
    (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,s)),
     overflow,
     vl[cur_in]->get_elem_size()
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
      (vl[cur_in]->get_name(),
       vl[cur_in]->get_ftype(),
       otl_var_char,
       temp_var_info,
       sizeof(temp_var_info));
    if(this->adb&&this->adb->get_throw_count()>1)return *this;
    if(this->adb)this->adb->increment_throw_count();
    if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
    throw otl_exception
      (otl_error_msg_4,
       otl_error_code_4,
       stm_label?stm_label:stm_text,
       temp_var_info,
       OTL_RCAST(const void*,s),
       OTL_SCAST(int,vl[cur_in]->get_elem_size()));
#else
    throw otl_exception
      (otl_error_msg_4,
       otl_error_code_4,
       stm_label?stm_label:stm_text,
       temp_var_info);
#endif
   }

  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const unsigned char* s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy4
    (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
     OTL_CCAST(unsigned char*,s),
     overflow,
     vl[cur_in]->get_elem_size()
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
      (vl[cur_in]->get_name(),
       vl[cur_in]->get_ftype(),
       otl_var_char,
       temp_var_info,
       sizeof(temp_var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return *this;
    if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
    throw otl_exception
      (otl_error_msg_4,
       otl_error_code_4,
       stm_label?stm_label:stm_text,
       temp_var_info,
       OTL_RCAST(const void*,s),
       OTL_SCAST(int,vl[cur_in]->get_elem_size()));
#else
    throw otl_exception
     (otl_error_msg_4,
      otl_error_code_4,
      stm_label?stm_label:stm_text,
      temp_var_info);
#endif
   }

  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

#define OTL_D6(T,T_type)                        \
  otl_ref_select_stream& operator<<(const T n)  \
  {                                             \
    check_in_var();                             \
    if(check_in_type(T_type,sizeof(T))){        \
      *OTL_RCAST(T*,vl[cur_in]->val())=n;       \
    }                                           \
    this->vl[cur_in]->set_not_null(0);          \
    get_in_next();                              \
    return *this;                               \
  }


#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  OTL_D6(int,otl_var_int)
  OTL_D6(unsigned,otl_var_unsigned_int)
  OTL_D6(long,otl_var_long_int)
  OTL_D6(short,otl_var_short)
  OTL_D6(float,otl_var_float)
  OTL_D6(double,otl_var_double)
#else
  template<OTL_TYPE_NAME T,const int T_type> OTL_D6(T,T_type)
#endif


 int select_list_len(void)
 {
  return sl_len;
 }

 int column_ftype(int ndx=0)
 {
   return sl[ndx].get_ftype();
 }

 int column_size(int ndx=0)
 {
   return sl[ndx].get_elem_size();
 }

protected:

 void init(void)
 {
  sl=nullptr;
  sl_len=0;
  null_fetched=0;
  ret_code=0;
  sl_desc=nullptr;
  executed=0;
  cur_in=0;
  stm_text=nullptr;
 }

 void get_next(void)
 {
  if(cur_col<sl_len-1){
   ++cur_col;
   null_fetched=sl[cur_col].is_null(cur_row);
  }else{
   ret_code=next();
   cur_col=0;
  }
 }

#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
  void strict_check_throw(int type_code)
  {
   otl_var_info_col
     (sl[cur_col].get_pos(),
      sl[cur_col].get_ftype(),
      type_code,
      var_info,
      sizeof(var_info));
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw otl_exception
     (otl_error_msg_0,
      otl_error_code_0,
      this->stm_label?
      this->stm_label:
      this->stm_text,
      var_info);
  }
#endif

  int check_type_throw(int type_code,int actual_data_type)
  {
    int out_type_code;
    if(actual_data_type!=0)
      out_type_code=actual_data_type;
    else
      out_type_code=type_code;
    otl_var_info_col
      (sl[cur_col].get_pos(),
       sl[cur_col].get_ftype(),
       out_type_code,
       var_info,
       sizeof(var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return 0;
    if(otl_uncaught_exception()) return 0; 
    throw otl_exception
      (otl_error_msg_0,
       otl_error_code_0,
       stm_label?stm_label:stm_text,
       var_info);
  }

  int check_type(int type_code, int actual_data_type=0)
  {
    if(sl[cur_col].get_ftype()==type_code)
      return 1;
    else
      return check_type_throw(type_code,actual_data_type);
  }
    
 void look_ahead(void)
 {
  if(cur_col==sl_len-1){
   ret_code=next();
   cur_col=-1;
   ++_rfc;
  }
 }

 void get_select_list(void)
 {int i,j,rc;

  otl_auto_array_ptr<otl_column_desc> loc_ptr(otl_var_list_size);
  otl_column_desc* sl_desc_tmp=loc_ptr.get_ptr();
  int sld_tmp_len=0;
  int ftype,elem_size;

  rc=obndrv
   (&cursor_struct.cda,
    OTL_RCAST(unsigned char*,cur_placeholder),
    -1,
    OTL_RCAST(ub1*,&sel_cur.get_cursor_struct_ref().cda),
    sizeof(sel_cur.get_cursor_struct_ref().cda),
    102,-1,nullptr,nullptr,-1,-1);
  if(rc!=0){
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw otl_exception(cursor_struct,stm_label?stm_label:stm_text);
  }
  for(i=0;i<vl_len;++i)
    otl_tmpl_cursor
      <otl_exc,
     otl_conn,
     otl_cur,
     otl_var>::bind(*vl[i]);
 // Executing the PLSQL master block
  otl_tmpl_cursor
    <otl_exc,
     otl_conn,
     otl_cur,
     otl_var>::exec(1,0,otl_sql_exec_from_select_cursor_class);
  sel_cur.set_connected(1);
  cur_row=-2;
  sld_tmp_len=0;
  for(i=1;sel_cur.describe_column(sl_desc_tmp[i-1],i);++i){
    ++sld_tmp_len;
    if(sld_tmp_len==loc_ptr.get_arr_size()){
      loc_ptr.double_size();
      sl_desc_tmp=loc_ptr.get_ptr();
    }
  }
  sl_len=sld_tmp_len;
  if(sl){
   delete[] sl;
   sl=nullptr;
  }
  sl=new otl_generic_variable[sl_len==0?1:sl_len];
  int max_long_size=this->adb->get_max_long_size();
  for(j=0;j<sl_len;++j){
   otl_generic_variable::map_ftype
    (sl_desc_tmp[j],
     max_long_size,
     ftype,
     elem_size,
     this->local_override.getLen()>0?this->local_override:*override_,
     j+1,
     adb->get_connect_struct().get_connection_type());
   sl[j].copy_pos(j+1);
   sl[j].init(true,ftype,
              elem_size,
              OTL_SCAST(otl_stream_buffer_size_type,array_size),
              &adb->get_connect_struct()
             );
  }
  if(sl_desc){
   delete[] sl_desc;
   sl_desc=nullptr;
  }
  sl_desc=new otl_column_desc[sl_len==0?1:sl_len];
  for(i=0;i<sl_len;++i)
    sl_desc[i]=sl_desc_tmp[i];
  for(i=0;i<sl_len;++i)sel_cur.bind(sl[i]);
 }

 void get_in_next(void)
 {
  if(cur_in==vl_len-1)
   rewind();
  else{
   ++cur_in;
   executed=0;
  }
 }

  int check_in_type_throw(int type_code)
  {
    otl_var_info_var
      (vl[cur_in]->get_name(),
       vl[cur_in]->get_ftype(),
       type_code,
       var_info,
       sizeof(var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return 0;
    if(otl_uncaught_exception()) return 0; 

    throw otl_exception
      (otl_error_msg_0,
       otl_error_code_0,
       stm_label?stm_label:stm_text,
       var_info);
  }

  int check_in_type(int type_code,int tsize)
  {
    switch(vl[cur_in]->get_ftype()){
    case otl_var_char:
      if(type_code==otl_var_char)
        return 1;
    case otl_var_raw:
      if(type_code==otl_var_raw)
        return 1;
    default:
      if(vl[cur_in]->get_ftype()==type_code && 
         vl[cur_in]->get_elem_size()==tsize)
        return 1;
    }
    return check_in_type_throw(type_code);
  }

  void check_in_var_throw(void)
  {
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return;
    if(otl_uncaught_exception()) return; 
    throw otl_exception
      (otl_error_msg_1,
       otl_error_code_1,
       stm_label?stm_label:stm_text,
       nullptr);
  }

 void check_in_var(void)
 {
  if(vl_len==0)
    check_in_var();
 }

  void check_if_executed_throw(void)
  {
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return;
    if(otl_uncaught_exception()) return; 
    throw otl_exception
      (otl_error_msg_2,
       otl_error_code_2,
       stm_label?stm_label:stm_text,
       nullptr);
  }

  void check_if_executed(void)
  {
    if(!executed)
      check_if_executed_throw();
  }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_ref_select_stream(const otl_ref_select_stream&) = delete;
  otl_ref_select_stream& operator=(const otl_ref_select_stream&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_ref_select_stream(otl_ref_select_stream&&) = delete;
  otl_ref_select_stream& operator=(otl_ref_select_stream&&) = delete;
#endif
private:
#else
  otl_ref_select_stream(const otl_ref_select_stream&):
    otl_ref_cursor(),
   sl_desc(nullptr),
   sl_len(0),
   sl(nullptr),
   null_fetched(0),
   ret_code(0),
   cur_col(0),
   cur_in(0),
   executed(0),
    var_info(),
    override_(nullptr),
    _rfc(0)
  {
  }

  otl_ref_select_stream& operator=(const otl_ref_select_stream&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_ref_select_stream(otl_ref_select_stream&&):
    otl_ref_cursor(),
   sl_desc(nullptr),
   sl_len(0),
   sl(nullptr),
   null_fetched(0),
   ret_code(0),
   cur_col(0),
   cur_in(0),
   executed(0),
    var_info(),
    override_(nullptr),
    _rfc(0)
  {
  }

  otl_ref_select_stream& operator=(otl_ref_select_stream&&)
  {
    return *this;
  }
#endif
#endif

};

const int otl_ora7_no_stream=0;
const int otl_ora7_io_stream=1;
const int otl_ora7_select_stream=2;
const int otl_ora7_refcur_select_stream=3;

class otl_stream_shell: public otl_stream_shell_generic{
public:

  otl_ref_select_stream* ref_ss;
  otl_select_stream* ss;
  otl_inout_stream* io;
  otl_connect* adb;
  bool lob_stream_flag;
  
  int auto_commit_flag;
  
  otl_var_desc* iov;
  int iov_len;
  int next_iov_ndx;
  
  otl_var_desc* ov;
  int ov_len;
  int next_ov_ndx;
  
  bool flush_flag;
  int stream_type;
  
  otl_select_struct_override override_;
  
#if (defined(OTL_STL)||defined(OTL_ACE)) && defined(OTL_STREAM_POOLING_ON)
  OTL_STRING_CONTAINER orig_sql_stm;
#endif

#if defined(OTL_UNICODE_STRING_TYPE) && defined(OTL_STREAM_POOLING_ON)
  std::string orig_sql_stm;
#endif


  otl_stream_shell():
    otl_stream_shell_generic(),
    ref_ss(nullptr),
    ss(nullptr),
    io(nullptr),
    adb(nullptr),
    lob_stream_flag(false),
    auto_commit_flag(0),
    iov(nullptr),
    iov_len(0),
    next_iov_ndx(0),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    flush_flag(false),
    stream_type(otl_ora7_no_stream),
    override_()
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    ,orig_sql_stm()
#endif
 {
 }

 otl_stream_shell(const int ashould_delete):
    otl_stream_shell_generic(),
    ref_ss(nullptr),
    ss(nullptr),
    io(nullptr),
    adb(nullptr),
    lob_stream_flag(false),
    auto_commit_flag(0),
    iov(nullptr),
    iov_len(0),
    next_iov_ndx(0),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    flush_flag(true),
    stream_type(otl_ora7_no_stream),
    override_()
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    ,orig_sql_stm()
#endif
 {
   override_.setLen(0);
  should_delete=ashould_delete;
 }

 virtual ~otl_stream_shell()
 {
  if(should_delete){
   delete[] iov;
   delete[] ov;

   iov=nullptr; iov_len=0;
   ov=nullptr; ov_len=0;
   next_iov_ndx=0;
   next_ov_ndx=0;
   override_.setLen(0);
   flush_flag=true;
   stream_type=otl_ora7_no_stream;

   delete ss;
   delete io;
   delete ref_ss;
   ss=nullptr; 
   io=nullptr; 
   ref_ss=nullptr;
   adb=nullptr;
  }
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_stream_shell(const otl_stream_shell&) = delete;
  otl_stream_shell& operator=(const otl_stream_shell&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_stream_shell(otl_stream_shell&&) = delete;
  otl_stream_shell& operator=(otl_stream_shell&&) = delete;
#endif
private:
#else
  otl_stream_shell(const otl_stream_shell&):
    otl_stream_shell_generic(),
    ref_ss(nullptr),
    ss(nullptr),
    io(nullptr),
    adb(nullptr),
    lob_stream_flag(false),
    auto_commit_flag(0),
    iov(nullptr),
    iov_len(0),
    next_iov_ndx(0),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    flush_flag(false),
    stream_type(otl_ora7_no_stream),
    override_()
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    ,orig_sql_stm()
#endif
 {
 }

  otl_stream_shell& operator=(const otl_stream_shell&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_stream_shell(otl_stream_shell&&):
    otl_stream_shell_generic(),
    ref_ss(nullptr),
    ss(nullptr),
    io(nullptr),
    adb(nullptr),
    lob_stream_flag(false),
    auto_commit_flag(0),
    iov(nullptr),
    iov_len(0),
    next_iov_ndx(0),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    flush_flag(false),
    stream_type(otl_ora7_no_stream),
    override_()
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    ,orig_sql_stm()
#endif
 {
 }

  otl_stream_shell& operator=(otl_stream_shell&&)
  {
    return *this;
  }
#endif
#endif

};

class otl_stream{
private:
  
  otl_stream_shell* shell;
  otl_ptr<otl_stream_shell> shell_pt;
  int connected;
  
  otl_ref_select_stream** ref_ss;
  otl_select_stream** ss;
  otl_inout_stream** io;
  otl_connect** adb;
  
  int* auto_commit_flag;
  
  otl_var_desc** iov;
  int* iov_len;
  int* next_iov_ndx;
  
  otl_var_desc** ov;
  int* ov_len;
  int* next_ov_ndx;

  otl_select_struct_override* override_;

  int end_marker;
  int oper_int_called;
  int last_eof_rc;
  bool last_oper_was_read_op;

protected:

  void reset_end_marker(void)
  {
    last_eof_rc=0;
    end_marker=-1;
    oper_int_called=0;
  }

  int buf_size_;


  void throw_end_of_row()
#if defined(__GNUC__) && (__GNUC__>=4)
    __attribute__ ((noreturn))
#endif
  {
    throw otl_exception
      (otl_error_msg_34,
       otl_error_code_34,
       this->get_stm_text());
  }

 void inc_next_ov(void)
 {
  if((*ov_len)==0)return;
  if((*next_ov_ndx)<(*ov_len)-1)
   ++(*next_ov_ndx);
  else
   (*next_ov_ndx)=0;
 }
 
 void inc_next_iov(void)
 {
  if((*iov_len)==0)return;
  if((*next_iov_ndx)<(*iov_len)-1)
   ++(*next_iov_ndx);
  else
   (*next_iov_ndx)=0;
 }

public:

  int get_prefetched_row_count() const 
  {
    (*adb)->reset_throw_count();
    switch(shell->stream_type){
    case otl_ora7_no_stream:
    case otl_ora7_io_stream:
      return 0;
    case otl_ora7_select_stream:
      return (*ss)->get_prefetched_row_count();
    case otl_ora7_refcur_select_stream:
      return (*ref_ss)->get_prefetched_row_count();
    default:
      return 0;
    }
  }

  int get_auto_commit_flag() const
  {
    if(!auto_commit_flag)
      return 0;
    else
      return *auto_commit_flag;
  }

  void skip_to_end_of_row()
  {
    if(next_ov_ndx==nullptr)
      return;
    if((*ov_len)==0)return;
    last_oper_was_read_op=true;
    switch(shell->stream_type){
    case otl_ora7_no_stream:
      break;
    case otl_ora7_io_stream:
      last_eof_rc=(*io)->eof();
      (*io)->skip_to_end_of_row();
      break;
    case otl_ora7_select_stream:
      last_eof_rc=(*ss)->eof();
      (*ss)->skip_to_end_of_row();
      break;
    case otl_ora7_refcur_select_stream:
      last_eof_rc=(*ref_ss)->eof();
      (*ref_ss)->skip_to_end_of_row();
      break;
    }
    *next_ov_ndx=0;
  }

  void reset_to_last_valid_row()
  {
    if((*io)){
      (*adb)->reset_throw_count();
      (*io)->reset_to_last_valid_row();
    }
  }

  bool get_lob_stream_flag() const 
  {
    if(!shell)
      return false;
    else
      return shell->lob_stream_flag;
  }

  int get_adb_max_long_size() const 
  {
    return this->shell->adb->get_max_long_size();
  }


  void check_end_of_row()
  {
    if(next_ov_ndx==nullptr||(*next_ov_ndx)!=0)
      throw_end_of_row();
    if(next_iov_ndx==nullptr||(*next_iov_ndx)!=0)
      throw_end_of_row();
  }

  otl_stream_shell* get_shell(){return shell;}
  int get_connected() const {return connected;}

  int get_dirty_buf_len() const
  {
    switch(shell->stream_type){
    case otl_ora7_no_stream:
      return 0;
    case otl_ora7_io_stream:
      return (*io)->get_dirty_buf_len();
    case otl_ora7_select_stream:
    {
      int rc=(*ss)->get_select_row_count();
      if(rc<0)
        return 0;
      else
        return rc;
    }
    case otl_ora7_refcur_select_stream:
      return (*ref_ss)->get_select_row_count();
    default:
      return 0;
   }
  }

  const char* get_stm_text(void)
  {
    const char* no_stm_text=OTL_NO_STM_TEXT;
    switch(shell->stream_type){
    case otl_ora7_no_stream:
      return no_stm_text;
    case otl_ora7_io_stream:
      return (*io)->get_stm_label()?(*io)->get_stm_label():(*io)->get_stm_text();
    case otl_ora7_select_stream:
      return (*ss)->get_stm_label()?(*ss)->get_stm_label():(*ss)->get_stm_text();
    case otl_ora7_refcur_select_stream:
      return (*ref_ss)->get_stm_label()?
             (*ref_ss)->get_stm_label():(*ref_ss)->get_stm_text();
    default:
      return no_stm_text;
    }
  }

  void setBufSize(int buf_size)
  {
    buf_size_=buf_size;
  }
  
  int getBufSize(void) const
  {
    return buf_size_;
  }

 long get_rpc() OTL_NO_THROW
 {
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     return 0;
   case otl_ora7_io_stream:
     (*adb)->reset_throw_count();
     return (*io)->get_rpc();
   case otl_ora7_select_stream:
     (*adb)->reset_throw_count();
     return (*ss)->get_rfc();
   case otl_ora7_refcur_select_stream:
     (*adb)->reset_throw_count();
     return (*ref_ss)->_rfc;
   default:
     return 0;
   }
 }

  operator int(void) OTL_THROWS_OTL_EXCEPTION
  {
    if(!last_oper_was_read_op){
      if(this->adb&&*this->adb)(*this->adb)->increment_throw_count();
      if(this->adb&&*this->adb&&(*this->adb)->get_throw_count()>1)return 0;
      const char* stm_label=nullptr;
      const char* stm_text=nullptr;
      switch(shell->stream_type){
      case otl_ora7_no_stream:
        break;
      case otl_ora7_io_stream:
        stm_label=(*io)->get_stm_label();
        stm_text=(*io)->get_stm_text();
        break;
      case otl_ora7_select_stream:
        stm_label=(*ss)->get_stm_label();
        stm_text=(*ss)->get_stm_text();
        break;
      case otl_ora7_refcur_select_stream:
        stm_label=(*ref_ss)->get_stm_label();
        stm_text=(*ref_ss)->get_stm_text();
        break;
      }
      throw otl_exception
        (otl_error_msg_18,
         otl_error_code_18,
         stm_label?stm_label:stm_text);
    }
    if(end_marker==1)return 0;
    int rc=0;
    int temp_eof=eof();
    if(temp_eof && end_marker==-1 && oper_int_called==0){
      end_marker=1;
      if(last_eof_rc==1)
        rc=0;
      else
        rc=1;
    }else if(!temp_eof && end_marker==-1)
      rc=1;
    else if(temp_eof && end_marker==-1){
      end_marker=0;
      rc=1;
    }else if(temp_eof && end_marker==0){
      end_marker=1;
      rc=0;
    }
    if(!oper_int_called)oper_int_called=1;
    return rc;
  }

 void create_var_desc(void)
 {int i;
  delete[] (*iov);
  delete[] (*ov);
  (*iov)=nullptr; (*iov_len)=0;
  (*ov)=nullptr; (*ov_len)=0;
  switch(shell->stream_type){
  case otl_ora7_no_stream:
    break;
  case otl_ora7_io_stream:
    if((*io)->get_vl_len()>0){
      (*iov)=new otl_var_desc[(*io)->get_vl_len()];
      (*iov_len)=(*io)->get_vl_len();
      for(i=0;i<(*io)->get_vl_len();++i)
        (*io)->get_vl()[i]->copy_var_desc((*iov)[i]);
    }
    if((*io)->get_iv_len()>0){
      int temp_iv_len=(*io)->get_iv_len();
      (*ov)=new otl_var_desc[temp_iv_len];
      (*ov_len)=temp_iv_len;
      for(i=0;i<temp_iv_len;++i)
        (*io)->get_in_vl()[i]->copy_var_desc((*ov)[i]);
    }
    break;
  case otl_ora7_select_stream:
    if((*ss)->get_vl_len()>0){
      (*iov)=new otl_var_desc[(*ss)->get_vl_len()];
      (*iov_len)=(*ss)->get_vl_len();
      for(i=0;i<(*ss)->get_vl_len();++i)
        (*ss)->get_vl()[i]->copy_var_desc((*iov)[i]);
    }
    if((*ss)->get_sl_len()>0){
      (*ov)=new otl_var_desc[(*ss)->get_sl_len()];
      (*ov_len)=(*ss)->get_sl_len();
      for(i=0;i<(*ss)->get_sl_len();++i){
        (*ss)->get_sl()[i].copy_var_desc((*ov)[i]);
        (*ov)[i].copy_name((*ss)->get_sl_desc()[i].name);
      }
    }
    break;
  case otl_ora7_refcur_select_stream:
    if((*ref_ss)->vl_len>0){
      (*iov)=new otl_var_desc[(*ref_ss)->vl_len];
      (*iov_len)=(*ref_ss)->vl_len;
      for(i=0;i<(*ref_ss)->vl_len;++i)
        (*ref_ss)->vl[i]->copy_var_desc((*iov)[i]);
    }
    if((*ref_ss)->sl_len>0){
      (*ov)=new otl_var_desc[(*ref_ss)->sl_len];
      (*ov_len)=(*ref_ss)->sl_len;
      for(i=0;i<(*ref_ss)->sl_len;++i){
        (*ref_ss)->sl[i].copy_var_desc((*ov)[i]);
        (*ov)[i].copy_name((*ref_ss)->sl_desc[i].name);
      }
    }
    break;
  }

 }

 void set_column_type(const int column_ndx,
                      const int col_type,
                      const int col_size=0)
   OTL_NO_THROW
 {
   if(shell==nullptr){
     init_stream();
     shell->flush_flag=true;
   }
   override_->add_override(column_ndx,col_type,col_size);
 }

 void set_all_column_types(const unsigned mask=0)
   OTL_NO_THROW
 {
   if(shell==nullptr){
     init_stream();
     shell->flush_flag=true;
   }
   override_->set_all_column_types(mask);
 }

 void set_flush(const bool flush_flag=true)
   OTL_NO_THROW
 {
   if(shell==nullptr)init_stream();
  if(shell==nullptr)return;
  shell->flush_flag=flush_flag;
 }

 otl_var_desc* describe_in_vars(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if(shell==nullptr)return nullptr;
  if(shell->iov==nullptr)return nullptr;
  desc_len=shell->iov_len;
  return shell->iov;
 }

 otl_var_desc* describe_out_vars(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if(shell==nullptr)return nullptr;
  if(shell->ov==nullptr)return nullptr;
  desc_len=shell->ov_len;
  return shell->ov;
 }

 otl_var_desc* describe_next_in_var(void)
   OTL_NO_THROW
 {
  if(shell==nullptr)return nullptr;
  if(shell->iov==nullptr)return nullptr;
  return &(shell->iov[shell->next_iov_ndx]);
 }

  otl_var_desc* describe_next_out_var(void)
   OTL_NO_THROW
 {
  if(shell==nullptr)return nullptr;
  if(shell->ov==nullptr)return nullptr;
  return &(shell->ov[shell->next_ov_ndx]);
 }

 void init_stream(void)
 {
   buf_size_=1;
   last_oper_was_read_op=false;
   shell=nullptr;
   shell=new otl_stream_shell(0);
   shell_pt.assign(&shell);
   connected=0;
   
   ref_ss=&(shell->ref_ss);
   ss=&(shell->ss);
   io=&(shell->io);
   adb=&(shell->adb);
   auto_commit_flag=&(shell->auto_commit_flag);
   iov=&(shell->iov);
   iov_len=&(shell->iov_len);
   next_iov_ndx=&(shell->next_iov_ndx);
   ov=&(shell->ov);
   ov_len=&(shell->ov_len);
   next_ov_ndx=&(shell->next_ov_ndx);
   override_=&(shell->override_);
   
   (*ref_ss)=nullptr;
   (*io)=nullptr;
   (*ss)=nullptr;
   (*adb)=nullptr;
   (*ov)=nullptr; 
   (*ov_len)=0;
   (*next_iov_ndx)=0;
   (*next_ov_ndx)=0;
   (*auto_commit_flag)=1;
   (*iov)=nullptr; 
   (*iov_len)=0;

 }

 otl_stream
 (const otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  otl_connect& db,
  const char* ref_cur_placeholder=nullptr,
  const char* sqlstm_label=nullptr)
 OTL_THROWS_OTL_EXCEPTION:
   shell(nullptr),
   shell_pt(),
   connected(0),
   ref_ss(nullptr),
   ss(nullptr),
   io(nullptr),
   adb(nullptr),
   auto_commit_flag(nullptr),
   iov(nullptr),
   iov_len(nullptr),
   next_iov_ndx(nullptr),
   ov(nullptr),
   ov_len(nullptr),
   next_ov_ndx(nullptr),
   override_(nullptr),
   end_marker(0),
   oper_int_called(0),
   last_eof_rc(0),
   last_oper_was_read_op(0),
   buf_size_(0)
 {
  init_stream();

  (*io)=nullptr; (*ss)=nullptr; (*ref_ss)=nullptr;
  (*iov)=nullptr; (*iov_len)=0;
  (*ov)=nullptr; (*ov_len)=0;
  (*auto_commit_flag)=1;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  (*adb)=&db;
  shell->flush_flag=true;
  open(arr_size,sqlstm,db,ref_cur_placeholder,sqlstm_label);
 }

  otl_stream() OTL_NO_THROW:
   shell(nullptr),
   shell_pt(),
   connected(0),
   ref_ss(nullptr),
   ss(nullptr),
   io(nullptr),
   adb(nullptr),
   auto_commit_flag(nullptr),
   iov(nullptr),
   iov_len(nullptr),
   next_iov_ndx(nullptr),
   ov(nullptr),
   ov_len(nullptr),
   next_ov_ndx(nullptr),
   override_(nullptr),
   end_marker(0),
   oper_int_called(0),
   last_eof_rc(0),
   last_oper_was_read_op(0),
   buf_size_(0)
 {
  init_stream();
  shell->flush_flag=true;
 }

 virtual ~otl_stream()
 {
  if(!connected)return;
  try{
   if((*io)!=nullptr&&shell->flush_flag==false)
     (*io)->set_flush_flag2(false);
   close();
   if(shell!=nullptr){
    if((*io)!=nullptr)
      (*io)->set_flush_flag2(true);
   }
  }catch(OTL_CONST_EXCEPTION otl_exception&){
   if(shell!=nullptr){
    if((*io)!=nullptr)
      (*io)->set_flush_flag2(true);
   }
#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
   clean(1);
   if(shell!=nullptr)
     shell->set_should_delete(1);
   shell_pt.destroy();
#else
   shell_pt.destroy();
#endif

#if !defined(OTL_DESTRUCTORS_DO_NOT_THROW)
   throw;
#endif

  }
#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
  if(adb && (*adb) && (*adb)->get_throw_count()>0
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     || otl_uncaught_exception()
#elif defined(OTL_UNCAUGHT_EXCEPTION_ON)
     || otl_uncaught_exception()
#endif
     ){
   //
  }
#else
   shell_pt.destroy();
#endif
 }

 int eof(void)
#if defined(OTL_SELECT_STREAM_ALTERNATE_FETCH)
   OTL_THROWS_OTL_EXCEPTION
#else
   OTL_NO_THROW
#endif
 {
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     return 1;
   case otl_ora7_io_stream:
     (*adb)->reset_throw_count();
     return (*io)->eof();
   case otl_ora7_select_stream:
     (*adb)->reset_throw_count();
     return (*ss)->eof();
   case otl_ora7_refcur_select_stream:
     (*adb)->reset_throw_count();
     return (*ref_ss)->eof();
   default:
     return 0;
   }
 }

 void flush(void) OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->flush();
  }
 }

 bool get_error_state(void) const
 {
   if((*adb)->get_throw_count()>0)
     return true;
   else if((*io))
     return (*io)->get_error_state();
   else
    return false;
 }

 void clean(const int clean_up_error_flag=0)
   OTL_THROWS_OTL_EXCEPTION
 {
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     (*adb)->reset_throw_count();
     (*io)->clean(clean_up_error_flag);
     break;
   case otl_ora7_select_stream:
     (*adb)->reset_throw_count();
     (*ss)->clean();
     break;
   case otl_ora7_refcur_select_stream:
     (*adb)->reset_throw_count();
     (*ref_ss)->clean();
     break;
   }
 }

 void rewind(void)
   OTL_THROWS_OTL_EXCEPTION
 {
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     (*adb)->reset_throw_count();
     (*io)->rewind();
     break;
   case otl_ora7_select_stream:
     (*adb)->reset_throw_count();
     (*ss)->rewind();
     break;
   case otl_ora7_refcur_select_stream:
     (*adb)->reset_throw_count();
     (*ref_ss)->rewind();
     break;
   }
 }

 int is_null(void) OTL_NO_THROW
 {
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     return 0;
   case otl_ora7_io_stream:
     return (*io)->is_null();
   case otl_ora7_select_stream:
     return (*ss)->is_null();
   case otl_ora7_refcur_select_stream:
     return (*ref_ss)->is_null();
   default:
     return 0;
   }
 }

 void set_commit(int auto_commit=0)
   OTL_NO_THROW
 {
  (*auto_commit_flag)=auto_commit;
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->set_commit(auto_commit);
  }
 }

 void intern_cleanup(void)
 {
  delete[] (*iov);
  delete[] (*ov);

  (*iov)=nullptr; (*iov_len)=0;
  (*ov)=nullptr; (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  override_->setLen(0);

  switch(shell->stream_type){
  case otl_ora7_no_stream:
    break;
  case otl_ora7_io_stream:
    try{
      (*io)->flush();
      (*io)->close();
    }catch(OTL_CONST_EXCEPTION otl_exception&){
      clean(1);
      (*io)->close();
      delete (*io);
      (*io)=nullptr;
      shell->stream_type=otl_ora7_no_stream;
      throw;
    }
    delete (*io);
    (*io)=nullptr;
    shell->stream_type=otl_ora7_no_stream;
    break;
  case otl_ora7_select_stream:
    try{
      (*ss)->close();
    }catch(OTL_CONST_EXCEPTION otl_exception&){
      delete (*ss);
      (*ss)=nullptr;
      shell->stream_type=otl_ora7_no_stream;
      throw;
    }
    delete (*ss);
    (*ss)=nullptr;
    shell->stream_type=otl_ora7_no_stream;
    break;
  case otl_ora7_refcur_select_stream:
    try{
      (*ref_ss)->close();
    }catch(OTL_CONST_EXCEPTION otl_exception&){
      delete (*ref_ss);
      (*ref_ss)=nullptr;
      shell->stream_type=otl_ora7_no_stream;
      throw;
    }
    delete (*ref_ss);
    (*ref_ss)=nullptr;
    shell->stream_type=otl_ora7_no_stream;
    break;
  }
  (*ss)=nullptr; (*io)=nullptr; (*ref_ss)=nullptr;
  if(adb!=nullptr)(*adb)=nullptr; 
  adb=nullptr;
 }
 
 void open
 (const otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  otl_connect& db,
  const char* ref_cur_placeholder=nullptr,
  const char* sqlstm_label=nullptr)
   OTL_THROWS_OTL_EXCEPTION
 {
   if(arr_size<=0){
     throw otl_exception
       (otl_error_msg_40,
        otl_error_code_40,
        sqlstm);
   }
#if defined(OTL_STREAM_THROWS_NOT_CONNECTED_TO_DATABASE_EXCEPTION)
   if(!db.connected){
     throw otl_exception
       (otl_error_msg_35,
        otl_error_code_35,
        sqlstm);
   }
#endif
   reset_end_marker();
   if(this->good()){
     const char* temp_stm_text=nullptr;
     switch(shell->stream_type){
     case otl_ora7_no_stream:
       temp_stm_text=OTL_NO_STM_TEXT;
       break;
     case otl_ora7_io_stream:
       temp_stm_text=(*io)->get_stm_label()?
                     (*io)->get_stm_label():(*io)->get_stm_text();
       break;
     case otl_ora7_select_stream:
       temp_stm_text=(*ss)->get_stm_label()?
                     (*ss)->get_stm_label():(*ss)->get_stm_text();
     case otl_ora7_refcur_select_stream:
       temp_stm_text=(*ref_ss)->get_stm_label()?
                     (*ref_ss)->get_stm_label():(*ref_ss)->get_stm_text();
       break;
     default:
       temp_stm_text=OTL_NO_STM_TEXT;
       break;
     }
     throw otl_exception
       (otl_error_msg_29,
        otl_error_code_29,
        temp_stm_text);
   }
  if(shell==nullptr)
   init_stream();
  buf_size_=arr_size;
  OTL_TRACE_STREAM_OPEN2

#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    if(*adb==nullptr)*adb=&db;
    if((*adb) && (**adb).get_stream_pool_enabled_flag()){
      char temp_buf[128];
      otl_itoa(arr_size,temp_buf);
      const char delimiter=';';
#if defined(OTL_STREAM_POOL_USES_STREAM_LABEL_AS_KEY)
      const char* temp_label=sqlstm_label?sqlstm_label:sqlstm;
      OTL_STRING_CONTAINER sql_stm(temp_label);
      sql_stm+=delimiter;
      sql_stm+=OTL_STRING_CONTAINER(temp_buf);
#else
      OTL_STRING_CONTAINER sql_stm(sqlstm);
      sql_stm+=delimiter;
      sql_stm+=OTL_STRING_CONTAINER(temp_buf);
#endif
      if(shell!=nullptr){
        otl_select_struct_override& temp_override=shell->override_;
        for(int i=0;i<temp_override.getLen();++i){
          otl_itoa(OTL_SCAST(int,temp_override.get_col_type(i)),temp_buf);
          sql_stm+=delimiter;
          sql_stm+=OTL_STRING_CONTAINER(temp_buf);
        }    
      }
      otl_stream_shell* temp_shell=OTL_RCAST(otl_stream_shell*,
                                             db.sc.find(sql_stm));
      if(temp_shell){
        if(shell!=nullptr)
          shell_pt.destroy();
        shell=temp_shell;
        ref_ss=&(shell->ref_ss);
        ss=&(shell->ss);
        io=&(shell->io); 
        if((*io)!=nullptr)(*io)->set_flush_flag2(true);
        adb=&(shell->adb);
        auto_commit_flag=&(shell->auto_commit_flag);
        iov=&(shell->iov);
        iov_len=&(shell->iov_len);
        next_iov_ndx=&(shell->next_iov_ndx);
        ov=&(shell->ov);
        ov_len=&(shell->ov_len);
        next_ov_ndx=&(shell->next_ov_ndx);
        override_=&(shell->override_);
        override_->set_master_stream_ptr(OTL_RCAST(void*,this));
        try{
          if((*iov_len)==0)this->rewind();
        }catch(OTL_CONST_EXCEPTION otl_exception&){
          if((*adb))
            (*adb)->sc.remove(shell,shell->orig_sql_stm);
          intern_cleanup();
          shell_pt.destroy();
          connected=0;
          throw;     
        }

        connected=1;
        return;
      }
      shell->orig_sql_stm=sql_stm;
    }
#endif

  delete[] (*iov);
  delete[] (*ov);

  (*iov)=nullptr; (*iov_len)=0;
  (*ov)=nullptr; (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;

  char tmp[7];
  char* c=OTL_CCAST(char*,sqlstm);

  while(otl_isspace(*c)||(*c)=='(')++c;
  OTL_STRNCPY_S(tmp,sizeof(tmp),c,6);
  tmp[6]=0;
  c=tmp;
  while(*c){
   *c=OTL_SCAST(char,otl_to_upper(*c));
   ++c;
  }
  if(adb==nullptr)adb=&(shell->adb);
  (*adb)=&db;
  (*adb)->reset_throw_count();
  try{
   if((strncmp(tmp,"SELECT",6)==0||
       strncmp(tmp,"WITH",4)==0)&&
      ref_cur_placeholder==nullptr){
     override_->set_master_stream_ptr(OTL_RCAST(void*,this));
     (*ss)=new otl_select_stream
       (override_,
        arr_size,
        sqlstm,
        db,
        otl_explicit_select,
        sqlstm_label);
     shell->stream_type=otl_ora7_select_stream;
   }else if(ref_cur_placeholder!=nullptr){
     override_->set_master_stream_ptr(OTL_RCAST(void*,this));
     (*ref_ss)=new otl_ref_select_stream
       (override_,arr_size,sqlstm,
        ref_cur_placeholder,db,
        sqlstm_label);
     shell->stream_type=otl_ora7_refcur_select_stream;
   }else{
     (*io)=new otl_inout_stream
       (arr_size,sqlstm,db,
        OTL_RCAST(void*,this),
        false,sqlstm_label);
     (*io)->set_flush_flag(shell->flush_flag);
     shell->stream_type=otl_ora7_io_stream;
   }
  }catch(OTL_CONST_EXCEPTION otl_exception&){
   shell_pt.destroy();
   throw;
  }
  if((*io))(*io)->set_commit((*auto_commit_flag));
  create_var_desc();
  connected=1;
 }

#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
 void close(const bool save_in_stream_pool=true)
   OTL_THROWS_OTL_EXCEPTION
#else
 void close(void)
   OTL_THROWS_OTL_EXCEPTION
#endif
 {
  if(shell==nullptr)return;
  OTL_TRACE_FUNC(0x4,"otl_stream","close","")

#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
  if(save_in_stream_pool && (*adb) && (**adb).get_stream_pool_enabled_flag() &&
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     !(otl_uncaught_exception())&&
#elif defined(OTL_UNCAUGHT_EXCEPTION_ON)
     !(otl_uncaught_exception())&& 
#endif
     (*adb)->get_throw_count()==0){
   try{
    this->flush();
    this->clean(1);
   }catch(OTL_CONST_EXCEPTION otl_exception&){
    this->clean(1);
    throw;
   }
   if((*adb) && (*adb)->get_throw_count()>0){
    (*adb)->sc.remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return;
   }
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(otl_uncaught_exception()){
    if((*adb))
     (*adb)->sc.remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return; 
   }
#elif defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(otl_uncaught_exception()){
     if((*adb))
       (*adb)->sc.remove(shell,shell->orig_sql_stm);
     intern_cleanup();
     shell_pt.destroy();
     connected=0;
     return; 
   }
#endif
   (*adb)->sc.add(shell,shell->orig_sql_stm.c_str());
   shell_pt.disconnect();
   connected=0;
  }else{
   if((*adb))
    (*adb)->sc.remove(shell,shell->orig_sql_stm);
   intern_cleanup();
   shell_pt.destroy();
   connected=0;
  }
#else
  intern_cleanup();
  connected=0;
#endif
 }

 otl_column_desc* describe_select(int& desc_len) OTL_NO_THROW
 {
   desc_len=0;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     return nullptr;
   case otl_ora7_io_stream:
     return nullptr;
   case otl_ora7_select_stream:
     (*adb)->reset_throw_count();
     desc_len=(*ss)->get_sl_len();
     return (*ss)->get_sl_desc();
   case otl_ora7_refcur_select_stream:
     (*adb)->reset_throw_count();
     desc_len=(*ref_ss)->sl_len;
     return (*ref_ss)->sl_desc;
   default:
     return nullptr;
   }
 }

 int good(void) OTL_NO_THROW
 {
   if(!connected)return 0;
   if((*io)||(*ss)||(*ref_ss)){
     (*adb)->reset_throw_count();
     return 1;
   }else
     return 0;
 }

  otl_stream& operator>>(otl_stream& (*pf) (otl_stream&))
  {
    (*pf)(*this);
    return *this;
  }

  otl_stream& operator<<(otl_stream& (*pf) (otl_stream&))
  {
    (*pf)(*this);
    return *this;
  }

 otl_stream& operator>>(otl_pl_tab_generic& tab)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=true;
  if((*io)){
    last_eof_rc=(*io)->eof();
   (*io)->operator>>(tab);
   OTL_TRACE_WRITE(", tab len="<<tab.len(),"operator >>","PL/SQL Tab&")
   inc_next_ov();
  }
  return *this;
 }

 otl_stream& operator<<(otl_pl_tab_generic& tab)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=false;
   reset_end_marker();
  if((*io)){
    OTL_TRACE_READ(", tab len="<<tab.len(),"operator <<","PL/SQL Tab&")
   (*io)->operator<<(tab);
   inc_next_iov();
  }
  return *this;
 }

#if defined(OTL_PL_TAB) && defined(OTL_STL)

 otl_stream& operator>>(otl_pl_vec_generic& vec)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=true;
   if((*io)){
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(vec);
     OTL_TRACE_WRITE(", tab len="<<vec.len(),"operator >>","PL/SQL Tab&")
     inc_next_ov();
   }
   return *this;
 }

 otl_stream& operator<<(otl_pl_vec_generic& vec)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=false;
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(vec);
   OTL_TRACE_READ(", tab len="<<vec.len(),"operator <<","PL/SQL Tab&")
   inc_next_iov();
  }
  return *this;
 }

#endif

 otl_stream& operator>>(otl_time0& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
     break;
   }
   return *this;
 }

 otl_stream& operator<<(const otl_time0& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     (*io)->operator<<(n);
     break;
   case otl_ora7_select_stream:
     (*ss)->operator<<(n);
     break;
   case otl_ora7_refcur_select_stream:
     (*ref_ss)->operator<<(n);
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
   return *this;
 }

 otl_stream& operator>>(otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=true;
#if defined(OTL_ORA7) && defined(OTL_ORA7_STRING_TO_TIMESTAMP)
  otl_var_desc* temp_next_var=describe_next_out_var();
  if(temp_next_var!=nullptr && temp_next_var->ftype==otl_var_char){
    char tmp_str[100];
    (*this)>>tmp_str;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
    if((*this).is_null())
      s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
    else
      OTL_ORA7_STRING_TO_TIMESTAMP(tmp_str,s);
#else
    OTL_ORA7_STRING_TO_TIMESTAMP(tmp_str,s);
#endif
    OTL_TRACE_WRITE
      (OTL_TRACE_FORMAT_DATETIME(s),
       "operator >>",
       "otl_datetime&");
    return *this;
  }else{
    otl_time0 tmp;
    (*this)>>tmp;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
    if((*this).is_null())
      s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
    else{
      s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
      s.month=tmp.month;
      s.day=tmp.day;
      s.hour=tmp.hour-1;
      s.minute=tmp.minute-1;
      s.second=tmp.second-1;
    }
#else
    s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
    s.month=tmp.month;
    s.day=tmp.day;
    s.hour=tmp.hour-1;
    s.minute=tmp.minute-1;
    s.second=tmp.second-1;
#endif
    OTL_TRACE_WRITE
      (OTL_TRACE_FORMAT_DATETIME(s),
       "operator >>",
       "otl_datetime&")
      inc_next_ov();
    return *this;
  }
#else
  otl_time0 tmp;
  (*this)>>tmp;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
  if((*this).is_null())
   s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
  else{
   s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
   s.month=tmp.month;
   s.day=tmp.day;
   s.hour=tmp.hour-1;
   s.minute=tmp.minute-1;
   s.second=tmp.second-1;
  }
#else
  s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
  s.month=tmp.month;
  s.day=tmp.day;
  s.hour=tmp.hour-1;
  s.minute=tmp.minute-1;
  s.second=tmp.second-1;
#endif
  OTL_TRACE_WRITE
    (OTL_TRACE_FORMAT_DATETIME(s),
     "operator >>",
     "otl_datetime&")
  inc_next_ov();
  return *this;
#endif
 }

 otl_stream& operator<<(const otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
#if defined(OTL_ORA7) && defined(OTL_ORA7_TIMESTAMP_TO_STRING)
    otl_var_desc* temp_next_var=describe_next_in_var();
    if(temp_next_var!=nullptr && temp_next_var->ftype==otl_var_char){
     char tmp_str[100];
     OTL_ORA7_TIMESTAMP_TO_STRING(s,tmp_str);
     OTL_TRACE_READ
       (OTL_TRACE_FORMAT_DATETIME(s),
        "operator <<",
        "otl_datetime&");
     (*this)<<tmp_str;
     return *this;
   }else{
     otl_time0 tmp;
     tmp.year=OTL_SCAST(unsigned char, ((s.year%100)+100));
     tmp.century=OTL_SCAST(unsigned char, ((s.year/100)+100));
     tmp.month=OTL_SCAST(unsigned char, s.month);
     tmp.day=OTL_SCAST(unsigned char, s.day);
     tmp.hour=OTL_SCAST(unsigned char, (s.hour+1));
     tmp.minute=OTL_SCAST(unsigned char, (s.minute+1));
     tmp.second=OTL_SCAST(unsigned char, (s.second+1));
     OTL_TRACE_READ
       (OTL_TRACE_FORMAT_DATETIME(s),
        "operator <<",
        "otl_datetime&");
     (*this)<<tmp;
     inc_next_iov();
     return *this;
   }
#else
     otl_time0 tmp;
     tmp.year=OTL_SCAST(unsigned char, ((s.year%100)+100));
     tmp.century=OTL_SCAST(unsigned char, ((s.year/100)+100));
     tmp.month=OTL_SCAST(unsigned char, s.month);
     tmp.day=OTL_SCAST(unsigned char, s.day);
     tmp.hour=OTL_SCAST(unsigned char, (s.hour+1));
     tmp.minute=OTL_SCAST(unsigned char, (s.minute+1));
     tmp.second=OTL_SCAST(unsigned char, (s.second+1));
     OTL_TRACE_READ
       (OTL_TRACE_FORMAT_DATETIME(s),
        "operator <<",
        "otl_datetime&");
     (*this)<<tmp;
     inc_next_iov();
     return *this;
#endif
 }

 otl_stream& operator>>(char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(c);
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(c);
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(c);
     break;
   }
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
  if((*this).is_null())
   c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
  OTL_TRACE_WRITE("'"<<c<<"'","operator >>","char&")
  inc_next_ov();
  return *this;
 }

 otl_stream& operator>>(unsigned char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(c);
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(c);
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(c);
     break;
   }
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
  if((*this).is_null())
   c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
  OTL_TRACE_WRITE("'"<<c<<"'","operator >>","unsigned char&")
  inc_next_ov();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_stream& operator>>(OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
     break;
   }

#if defined(OTL_USER_DEFINED_STRING_CLASS_DEFAULT_NULL_TO_VAL)
   if((*this).is_null()){
     OTL_USER_DEFINED_STRING_CLASS_DEFAULT_NULL_TO_VAL(s);
   }
#elif defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     s=OTL_DEFAULT_STRING_NULL_TO_VAL;
#endif

   OTL_TRACE_WRITE("\""<<s<<"\"","operator >>","OTL_STRING_CONTAINER&")
   inc_next_ov();
   return *this;
 }
#endif

 otl_stream& operator>>(char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
     break;
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     otl_strcpy(OTL_RCAST(unsigned char*,s),
                OTL_RCAST(const unsigned char*,OTL_DEFAULT_STRING_NULL_TO_VAL)
               );
#endif
   OTL_TRACE_WRITE("\""<<s<<"\"","operator >>","char*")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
     break;
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     otl_strcpy(OTL_RCAST(unsigned char*,s),
                OTL_RCAST(const unsigned char*,OTL_DEFAULT_STRING_NULL_TO_VAL)
               );
#endif

#if defined(OTL_UNICODE)
   OTL_TRACE_WRITE
     ("\""<<OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,s)<<"\"",
      "operator >>",
      OTL_UNICODE_CHAR_TYPE_TRACE_NAME "*")
#else
   OTL_TRACE_WRITE("\""<<s<<"\"","operator >>","unsigned char*")
#endif
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(int& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(n);
#else
     (*io)->operator>><int,otl_var_int>(n);
#endif
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(n);
#else
     (*ss)->operator>><int,otl_var_int>(n);
#endif
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(n);
#else
     (*ref_ss)->operator>><int,otl_var_int>(n);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(n,"operator >>","int&")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(unsigned& u)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(u);
#else
     (*io)->operator>><unsigned,otl_var_unsigned_int>(u);
#endif
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(u);
#else
     (*ss)->operator>><unsigned,otl_var_unsigned_int>(u);
#endif
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(u);
#else
     (*ref_ss)->operator>><unsigned,otl_var_unsigned_int>(u);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     u=OTL_SCAST(unsigned int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(u,"operator >>","unsigned&")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(short& sh)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(sh);
#else
     (*io)->operator>><short,otl_var_short>(sh);
#endif
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(sh);
#else
     (*ss)->operator>><short,otl_var_short>(sh);
#endif
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(sh);
#else
     (*ref_ss)->operator>><short,otl_var_short>(sh);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     sh=OTL_SCAST(short int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(sh,"operator >>","short int&")
  inc_next_ov();
  return *this;
 }

 otl_stream& operator>>(long int& l)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(l);
#else
     (*io)->operator>><long,otl_var_long_int>(l);
#endif
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(l);
#else
     (*ss)->operator>><long,otl_var_long_int>(l);
#endif
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(l);
#else
     (*ref_ss)->operator>><long,otl_var_long_int>(l);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
   l=OTL_SCAST(long int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(l,"operator >>","long int&")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(float& f)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(f);
#else
     (*io)->operator>><float,otl_var_float>(f);
#endif
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(f);
#else
     (*ss)->operator>><float,otl_var_float>(f);
#endif
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(f);
#else
     (*ref_ss)->operator>><float,otl_var_float>(f);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     f=OTL_SCAST(float,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(f,"operator >>","float&")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator>>(double& d)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator>>(d);
#else
     (*io)->operator>><double,otl_var_double>(d);
#endif
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(d);
#else
     (*ss)->operator>><double,otl_var_double>(d);
#endif
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(d);
#else
     (*ref_ss)->operator>><double,otl_var_double>(d);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
  if((*this).is_null())
    d=OTL_SCAST(double,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
  OTL_TRACE_WRITE(d,"operator >>","double&")
  inc_next_ov();
  return *this;
 }

 otl_stream& operator>>(otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_ora7_select_stream:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   case otl_ora7_refcur_select_stream:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
     break;
   }
   OTL_TRACE_WRITE(" len="<<s.len(),"operator >>","otl_long_string&")
   inc_next_ov();
   return *this;
 }

 otl_stream& operator<<(const char c)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("'"<<c<<"'","operator <<","char")
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
    (*io)->operator<<(c);
     break;
   case otl_ora7_select_stream:
     (*ss)->operator<<(c);
     break;
   case otl_ora7_refcur_select_stream:
     (*ref_ss)->operator<<(c);
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const unsigned char c)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("'"<<c<<"'","operator <<","unsigned char")
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     (*io)->operator<<(c);
     break;
   case otl_ora7_select_stream:
     (*ss)->operator<<(c);
     break;
   case otl_ora7_refcur_select_stream:
     (*ref_ss)->operator<<(c);
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
  inc_next_iov();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_stream& operator<<(const OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("\""<<s<<"\"","operator <<","OTL_STRING_CONTAINER&")
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     (*io)->operator<<(s);
     break;
   case otl_ora7_select_stream:
     (*ss)->operator<<(s);
     break;
   case otl_ora7_refcur_select_stream:
     (*ref_ss)->operator<<(s);
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
  inc_next_iov();
  return *this;
 }
#endif


 otl_stream& operator<<(const char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("\""<<s<<"\"","operator <<","char*")
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     (*io)->operator<<(s);
     break;
   case otl_ora7_select_stream:
     (*ss)->operator<<(s);
     break;
   case otl_ora7_refcur_select_stream:
     (*ref_ss)->operator<<(s);
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
#if defined(OTL_UNICODE)
   OTL_TRACE_READ
     ("\""<<OTL_RCAST(const OTL_UNICODE_CHAR_TYPE*,s)<<"\"",
      "operator <<",
      OTL_UNICODE_CHAR_TYPE_TRACE_NAME "*")
#else
   OTL_TRACE_READ("\""<<s<<"\"","operator <<","unsigned char*")
#endif
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     (*io)->operator<<(s);
     break;
   case otl_ora7_select_stream:
     (*ss)->operator<<(s);
     break;
   case otl_ora7_refcur_select_stream:
     (*ref_ss)->operator<<(s);
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const int n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(n,"operator <<","int")
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(n);
#else
     (*io)->operator<<<int,otl_var_int>(n);
#endif
     break;
   case otl_ora7_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(n);
#else
     (*ss)->operator<<<int,otl_var_int>(n);
#endif
     break;
   case otl_ora7_refcur_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator<<(n);
#else
     (*ref_ss)->operator<<<int,otl_var_int>(n);
#endif
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const unsigned u)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(u,"operator <<","unsigned")
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(u);
#else
     (*io)->operator<<<unsigned,otl_var_unsigned_int>(u);
#endif
     break;
   case otl_ora7_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(u);
#else
     (*ss)->operator<<<unsigned,otl_var_unsigned_int>(u);
#endif
     break;
   case otl_ora7_refcur_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator<<(u);
#else
     (*ref_ss)->operator<<<unsigned,otl_var_unsigned_int>(u);
#endif
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const short sh)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(sh,"operator <<","short int")
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(sh);
#else
     (*io)->operator<<<short,otl_var_short>(sh);
#endif
     break;
   case otl_ora7_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(sh);
#else
     (*ss)->operator<<<short,otl_var_short>(sh);
#endif
     break;
   case otl_ora7_refcur_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator<<(sh);
#else
     (*ref_ss)->operator<<<short,otl_var_short>(sh);
#endif
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const long int l)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(l,"operator <<","long int")
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(l);
#else
     (*io)->operator<<<long,otl_var_long_int>(l);
#endif
     break;
   case otl_ora7_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(l);
#else
     (*ss)->operator<<<long,otl_var_long_int>(l);
#endif
     break;
   case otl_ora7_refcur_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator<<(l);
#else
     (*ref_ss)->operator<<<long,otl_var_long_int>(l);
#endif
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const float f)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(f,"operator <<","float")
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(f);
#else
     (*io)->operator<<<float,otl_var_float>(f);
#endif
     break;
   case otl_ora7_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(f);
#else
     (*ss)->operator<<<float,otl_var_float>(f);
#endif
     break;
   case otl_ora7_refcur_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator<<(f);
#else
     (*ref_ss)->operator<<<float,otl_var_float>(f);
#endif
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const double d)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(d,"operator <<","double")
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*io)->operator<<(d);
#else
     (*io)->operator<<<double,otl_var_double>(d);
#endif
     break;
   case otl_ora7_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(d);
#else
     (*ss)->operator<<<double,otl_var_double>(d);
#endif
     break;
   case otl_ora7_refcur_select_stream:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator<<(d);
#else
     (*ref_ss)->operator<<<double,otl_var_double>(d);
#endif
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

  otl_stream& operator<<(const otl_null& n)
    OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("NULL","operator <<","otl_null&")
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     (*io)->operator<<(n);
     break;
   case otl_ora7_select_stream:
     (*ss)->operator<<(n);
     break;
   case otl_ora7_refcur_select_stream:
     (*ref_ss)->operator<<(n);
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const otl_long_string& d)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(" len="<<d.len(),"operator <<","otl_long_string&")
   switch(shell->stream_type){
   case otl_ora7_no_stream:
     break;
   case otl_ora7_io_stream:
     (*io)->operator<<(d);
     break;
   case otl_ora7_select_stream:
     (*ss)->operator<<(d);
     break;
   case otl_ora7_refcur_select_stream:
     (*ref_ss)->operator<<(d);
     if(!(*ov)&&(*ref_ss)->sl) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_stream& operator=(const otl_stream&) = delete;
  otl_stream(const otl_stream&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_stream& operator=(otl_stream&&) = delete;
  otl_stream(otl_stream&&) = delete;
#endif

#if !defined(OTL_STREAM_NO_PRIVATE_BOOL_OPERATORS)
  otl_stream& operator>>(bool&) = delete;
  otl_stream& operator<<(const bool) = delete;
#endif

#if !defined(OTL_STREAM_NO_PRIVATE_UNSIGNED_LONG_OPERATORS)
  otl_stream& operator>>(unsigned long int&) = delete;
  otl_stream& operator<<(const unsigned long int) = delete;
#endif
private:
#else
  otl_stream& operator=(const otl_stream&)
  {
    return *this;
  }

  otl_stream(const otl_stream&):
   shell(nullptr),
   shell_pt(),
   connected(0),
   ref_ss(nullptr),
   ss(nullptr),
   io(nullptr),
   adb(nullptr),
   auto_commit_flag(nullptr),
   iov(nullptr),
   iov_len(nullptr),
   next_iov_ndx(nullptr),
   ov(nullptr),
   ov_len(nullptr),
   next_ov_ndx(nullptr),
   override_(nullptr),
   end_marker(0),
   oper_int_called(0),
   last_eof_rc(0),
   last_oper_was_read_op(0),
   buf_size_(0)
  {
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_stream& operator=(otl_stream&&)
  {
    return *this;
  }

  otl_stream(otl_stream&&):
   shell(nullptr),
   shell_pt(),
   connected(0),
   ref_ss(nullptr),
   ss(nullptr),
   io(nullptr),
   adb(nullptr),
   auto_commit_flag(nullptr),
   iov(nullptr),
   iov_len(nullptr),
   next_iov_ndx(nullptr),
   ov(nullptr),
   ov_len(nullptr),
   next_ov_ndx(nullptr),
   override_(nullptr),
   end_marker(0),
   oper_int_called(0),
   last_eof_rc(0),
   last_oper_was_read_op(0),
   buf_size_(0)
  {
  }
#endif

#if !defined(OTL_STREAM_NO_PRIVATE_BOOL_OPERATORS)
  otl_stream& operator>>(bool&)
    OTL_NO_THROW
  {
   return *this;
  }

  otl_stream& operator<<(const bool)
    OTL_NO_THROW
  {
   return *this;
  }

#endif

#if !defined(OTL_STREAM_NO_PRIVATE_UNSIGNED_LONG_OPERATORS)
  otl_stream& operator>>(unsigned long int&)
    OTL_NO_THROW
  {
   return *this;
  }

  otl_stream& operator<<(const unsigned long int)
    OTL_NO_THROW
  {
   return *this;
  }
#endif
#endif

};

inline otl_connect& operator>>(otl_connect& connect, otl_stream& s)
{
  const char* cmd=connect.getCmd();
  const char* invalid_cmd="*** INVALID COMMAND ***";
  if(!cmd)
    cmd=invalid_cmd;
  s.open(s.getBufSize(),cmd,connect);
  return connect;
}

#if (defined(OTL_STL)||defined(OTL_VALUE_TEMPLATE_ON)) && defined(OTL_VALUE_TEMPLATE)
template <OTL_TYPE_NAME TData>
otl_stream& operator<<(otl_stream& s, const otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
 if(var.ind)
  s<<otl_null();
 else
  s<<var.v;
 return s;
}

template <OTL_TYPE_NAME TData>
otl_stream& operator>>(otl_stream& s, otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
  s>>var.v;
  if(s.is_null())
    var.ind=true;
  else
    var.ind=false;
  return s;
}

#endif

typedef otl_tmpl_nocommit_stream
<otl_stream,
 otl_connect,
 otl_exception> otl_nocommit_stream;

#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
inline otl_stream& operator>>(otl_stream& s, OTL_NUMERIC_TYPE_1& n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_numeric_type_1_str_size];

  s>>temp_val;
  if(s.is_null()){
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if(s.is_null())
     n=OTL_SCAST(OTL_NUMERIC_TYPE_1,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
    return s;
  }
  OTL_STR_TO_NUMERIC_TYPE_1(temp_val,n)
  return s;
}

inline otl_stream& operator<<(otl_stream& s, const OTL_NUMERIC_TYPE_1& n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_numeric_type_1_str_size];
  OTL_NUMERIC_TYPE_1_TO_STR(n,temp_val);
  s<<temp_val;
  return s;
}
#endif

#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
inline otl_stream& operator>>(otl_stream& s, OTL_NUMERIC_TYPE_2& n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_numeric_type_2_str_size];

  s>>temp_val;
  if(s.is_null()){
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if(s.is_null())
     n=OTL_SCAST(OTL_NUMERIC_TYPE_2,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
    return s;
  }
  OTL_STR_TO_NUMERIC_TYPE_2(temp_val,n)
  return s;
}

inline otl_stream& operator<<(otl_stream& s, const OTL_NUMERIC_TYPE_2& n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_numeric_type_2_str_size];
  OTL_NUMERIC_TYPE_2_TO_STR(n,temp_val);
  s<<temp_val;
  return s;
}
#endif

#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
inline otl_stream& operator>>(otl_stream& s, OTL_NUMERIC_TYPE_3& n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_numeric_type_3_str_size];

  s>>temp_val;
  if(s.is_null()){
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if(s.is_null())
     n=OTL_SCAST(OTL_NUMERIC_TYPE_3,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
    return s;
  }
  OTL_STR_TO_NUMERIC_TYPE_3(temp_val,n)
  return s;
}

inline otl_stream& operator<<(otl_stream& s, const OTL_NUMERIC_TYPE_3& n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_numeric_type_3_str_size];
  OTL_NUMERIC_TYPE_3_TO_STR(n,temp_val);
  s<<temp_val;
  return s;
}
#endif

#if defined(OTL_BIGINT) && defined(OTL_STR_TO_BIGINT) && \
    defined(OTL_BIGINT_TO_STR)
inline otl_stream& operator>>(otl_stream& s, OTL_BIGINT& n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_bigint_str_size];

  s>>temp_val;
  if(s.is_null()){
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if(s.is_null())
     n=OTL_SCAST(OTL_BIGINT,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
    return s;
  }
  OTL_STR_TO_BIGINT(temp_val,n)
  return s;
}

inline otl_stream& operator<<(otl_stream& s, const OTL_BIGINT n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_bigint_str_size];
  OTL_BIGINT_TO_STR(n,temp_val);
  s<<temp_val;
  return s;
}
#elif defined(OTL_BIGINT) && defined(OTL_ORA_MAP_BIGINT_TO_LONG)
inline otl_stream& operator>>(otl_stream& s, OTL_BIGINT& n)
  OTL_THROWS_OTL_EXCEPTION
{
  long temp_val=0;

  s>>temp_val;
  if(s.is_null()){
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if(s.is_null())
     n=OTL_SCAST(OTL_BIGINT,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
    return s;
  }
  n=OTL_SCAST(OTL_BIGINT,temp_val);
  return s;
}

inline otl_stream& operator<<(otl_stream& s, const OTL_BIGINT n)
  OTL_THROWS_OTL_EXCEPTION
{
  long temp_val=OTL_SCAST(long,n);
  s<<temp_val;
  return s;
}

#endif

inline otl_stream& endr(otl_stream& s)
{
  s.check_end_of_row();
  return s;
}

OTL_ORA7_NAMESPACE_END
#endif

// ==================== OTL-Adapter for Oracle 8 =====================
#if defined(OTL_ORA8)
#if defined(__STDC__)
#define __STDC__DEFINED
#else
#define __STDC__ 1 // making OCI function prototypes show up in oci.h
#endif
#if defined(OTL_ORA_TEXT_ON)
#define text OTL_ORA_TEXT
#endif
#include <oci.h>

#if !defined(OTL_ORA_DOES_NOT_UNDEF_MIN_MAX)

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#endif

#define OTL_UTF8_BYTES_PER_CHAR (4)

#if defined(OTL_ORA8_PROC)
extern "C" {
#include <sql2oci.h>
}
#endif

OTL_ORA8_NAMESPACE_BEGIN

const int inVarChar2=1;
const int inNumber=2;
const int inLong=8;
const int inRowId=104;
const int inDate=12;
const int inRaw=23;
const int inLongRaw=24;
const int inChar=96;
#if defined(OTL_ORA10G)||defined(OTL_ORA10G_R2)
const int inBFloat=SQLT_IBFLOAT;
const int inBDouble=SQLT_IBDOUBLE;
#endif
const int inMslabel=105;
const int inUserDefinedType=108;
const int inRef=111;
const int inCLOB=112;
const int inBLOB=113;

#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
const int inTimestamp=SQLT_TIMESTAMP;
const int inTimestamp_TZ=SQLT_TIMESTAMP_TZ;
const int inTimestamp_LTZ=SQLT_TIMESTAMP_LTZ;
const int inIntervalYM=SQLT_INTERVAL_YM;
const int inIntervalDS=SQLT_INTERVAL_DS;
#endif

const int  extVarChar2=1;
const int  extNumber=2;
const int  extInt=3;
const int  extFloat=4;
#if defined(OTL_ORA_MAP_STRINGS_TO_CHARZ)
const int  extCChar=97;
#else
const int  extCChar=5;
#endif
const int  extVarNum=6;
const int  extLong=8;
const int  extVarChar=9;
const int  extRowId=11;
const int  extDate=12;
const int  extVarRaw=15;
const int  extRaw=extVarRaw;
const int  extLongRaw=24;
const int  extUInt=68;
const int  extLongVarChar=94;
const int  extLongVarRaw=95;
const int  extChar=96;
const int  extCharZ=97;
const int  extMslabel=105;
const int  extCLOB=inCLOB;
const int  extBLOB=inBLOB;

#if (defined(OTL_ORA10G)||defined(OTL_ORA10G_R2))&&!defined(OTL_ORA_LEGACY_NUMERIC_TYPES)
const int extBFloat=SQLT_BFLOAT;
const int extBDouble=SQLT_BDOUBLE;
#endif


#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
const int extTimestamp=SQLT_TIMESTAMP;
const int extTimestamp_TZ=SQLT_TIMESTAMP_TZ;
const int extTimestamp_LTZ=SQLT_TIMESTAMP_LTZ;
#endif


#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
typedef otl_datetime otl_time0;
#else
typedef otl_oracle_date otl_time0;
#endif


class otl_exc{
public:
  unsigned char msg[1000];
  int code;
  char sqlstate[32];

#if defined(OTL_EXCEPTION_ENABLE_ERROR_OFFSET)
  int error_offset;
#endif

#if defined(OTL_EXTENDED_EXCEPTION)
  char** msg_arr;
  char** sqlstate_arr;
  int* code_arr;
  int arr_len;
#endif

  enum{disabled=0,enabled=1};

  otl_exc():
    msg(),
    code(0),
    sqlstate()
#if defined(OTL_EXCEPTION_ENABLE_ERROR_OFFSET)
    ,error_offset(-1)
#endif
#if defined(OTL_EXTENDED_EXCEPTION)
    ,msg_arr(nullptr),
    sqlstate_arr(nullptr),
    code_arr(nullptr),
    arr_len(0)
#endif
  {
    sqlstate[0]=0;
    msg[0]=0;
  }

  virtual ~otl_exc(){}

 void init(const char* amsg, const int acode)
 {
   OTL_STRCPY_S(OTL_RCAST(char*,msg),sizeof(msg),amsg);
   code=acode;
#if defined(OTL_EXCEPTION_ENABLE_ERROR_OFFSET)
   error_offset=-1;
#endif
#if defined(OTL_EXTENDED_EXCEPTION)
   msg_arr=nullptr;
   sqlstate_arr=nullptr;
   code_arr=nullptr;
   arr_len=0;
#endif
 }

};

class otl_cur;
class otl_var;

class otl_conn{
private:

  friend class otl_cur;
  friend class otl_var;
  
  OCIEnv *envhp; // OCI environment handle
  OCIServer *srvhp; // OCI Server handle
  OCIError *errhp; // OCI Error handle
  OCISvcCtx *svchp; // OCI Service context handle
  OCISession *authp; // OCI Session handle
  int auto_commit;
  int extern_lda;
  int attached;
  int in_session;
  int char_set_;
  int session_begin_count;
  int session_mode_;
  int ext_cred;
  int last_status;
  char* xa_server_external_name;
  char* xa_server_internal_name;
  
#if defined(OTL_ORA_OCI_ENV_CREATE)
  bool threaded_mode;
#endif
  
public:
  
  enum bigint_type
  {
#if defined(OTL_BIGINT) && \
    (defined(OTL_ORA11G_R2)&&!defined(OTL_STR_TO_BIGINT)&&\
     !defined(OTL_BIGINT_TO_STR))
    var_bigint = otl_var_bigint,
    bigint_size = sizeof(OTL_BIGINT)
#elif defined(OTL_BIGINT) && defined(OTL_ORA_MAP_BIGINT_TO_LONG)
    var_bigint = otl_var_long_int,
    bigint_size = sizeof(long)
#else
    var_bigint = otl_var_char,
    bigint_size = otl_bigint_str_size
#endif
  };

  enum ubigint_type
  {
#if defined(OTL_UBIGINT) && defined(OTL_ORA11G_R2)
    var_ubigint = otl_var_ubigint,
    ubigint_size = sizeof(OTL_UBIGINT)
#else
    var_ubigint = otl_var_char,
    ubigint_size = otl_ubigint_str_size
#endif
  };

#if defined(OTL_ORA_OCI_ENV_CREATE)
  void set_threaded_mode(const bool athreaded_mode) 
  {
    threaded_mode=athreaded_mode;
  }
#endif


  void cleanup(void)
  {
    session_end();
    server_detach();
  }

  int get_session_begin_count() const
  {
    return session_begin_count;
  }

  int get_extern_lda() const
  {
    return extern_lda;
  }

  int get_last_status() const
  {
    return last_status;
  }

  int get_auto_commit() const
  {
    return auto_commit;
  }

  OCIEnv* get_envhp()
  {
    return envhp;
  }

  OCIError* get_errhp()
  {
    return errhp;
  }

  OCISvcCtx* get_svchp()
  {
    return svchp;
  }

  OCIServer* get_srvhp()
  {
    return srvhp;
  }

  OCISession* get_authp()
  {
    return authp;
  }

  int get_char_set() const
  {
    return char_set_;
  }

  int get_connection_type(void)
  {
    return 0;
  }

#if !defined(OTL_ORA_OCI_ENV_CREATE)
  static int initialize(const int threaded_mode=0)
  {
    int status;
    int mode;
    if(threaded_mode)
      mode=OCI_THREADED;
    else
      mode=OCI_DEFAULT;
    status=OCIInitialize
      (OTL_SCAST(ub4,mode),
#if defined(OTL_ANSI_CPP_11_NULLPTR_SUPPORT)
       nullptr,
#else
       OTL_RCAST(dvoid*,0),
#endif
       nullptr,
       nullptr,
       nullptr);
    if(status!=OCI_SUCCESS)
      return 0;
    else
      return 1;
#else
  static int initialize(const int /*threaded_mode*/)
  {
    return 1;
#endif
  }

  otl_conn():
    envhp(nullptr),
    srvhp(nullptr),
    errhp(nullptr),
    svchp(nullptr),
    authp(nullptr),
    auto_commit(0),
    extern_lda(0),
    attached(0),
    in_session(0),
    char_set_(SQLCS_IMPLICIT),
    session_begin_count(0),
    session_mode_(OCI_DEFAULT),
    ext_cred(0),
    last_status(OCI_SUCCESS),
    xa_server_external_name(nullptr),
    xa_server_internal_name(nullptr)
#if defined(OTL_ORA_OCI_ENV_CREATE)
    ,threaded_mode(false)
#endif
 {
 }

#if defined(OTL_ORA_OCI_ENV_CREATE)
  void set_connect_mode(bool mode)
  {
    threaded_mode=mode;
  }
#endif

 void set_char_set(const int char_set)
 {
  char_set_=char_set;
 }

  void set_xa_server_external_name(const char* name)
  {
   if(xa_server_external_name){
     delete[] xa_server_external_name;
     xa_server_external_name=nullptr;
   }
   size_t len=strlen(name)+1;
   xa_server_external_name=new char[len];
   OTL_STRCPY_S(xa_server_external_name,len,name);
  }

  void set_xa_server_internal_name(const char* name)
  {
   if(xa_server_internal_name){
     delete[] xa_server_internal_name;
     xa_server_internal_name=nullptr;
   }
   size_t len=strlen(name)+1;
   xa_server_internal_name=new char[len];
   OTL_STRCPY_S(xa_server_internal_name,len,name);
  }

  void delete_xa_strings(void )
  {
    if(xa_server_external_name){
      delete[] xa_server_external_name;
      xa_server_external_name=nullptr;
    }
    if(xa_server_internal_name){
      delete[] xa_server_internal_name;
      xa_server_internal_name=nullptr;
    }
  }

 virtual ~otl_conn()
 {
   delete_xa_strings();
 }

  void set_timeout(const int /*atimeout*/=0){}
  void set_cursor_type(const int /*acursor_type*/=0){}

 int cancel(void)
 {int status;
  status=OCIBreak(srvhp,errhp);
  if(status)
   return 0;
  else
   return 1;
 }

 int server_attach(const char* tnsname)
 {int& status=last_status;

  envhp=nullptr;
  srvhp=nullptr;
  errhp=nullptr;
  svchp=nullptr;
  authp=nullptr;
  extern_lda=0;
  attached=0;
  in_session=0;
  session_begin_count=0;

#if !defined(OTL_ORA_OCI_ENV_CREATE)
  status=OCIEnvInit
   (OTL_RCAST(OCIEnv**,&envhp),
    OCI_DEFAULT,
    0,
    nullptr);
#else
  status=OCIEnvCreate
    (OTL_RCAST(OCIEnv**,&envhp), 
#if defined(OTL_ORA_OCI_ENV_CREATE_MODE)
     OTL_ORA_OCI_ENV_CREATE_MODE,
#else
     threaded_mode?OCI_THREADED:OCI_DEFAULT,
#endif
     nullptr, 
     nullptr, 
     nullptr, 
     nullptr, 
     0, 
     nullptr);
#endif

  if(status)return 0;

#if defined(__GNUC__) && (__GNUC__>=4)
  void* temp_errhp=&errhp;
#endif
  status=OCIHandleAlloc
   (OTL_RCAST(dvoid*,envhp),
#if defined(__GNUC__) && (__GNUC__>=4)
    OTL_RCAST(dvoid**,temp_errhp),
#else
    OTL_RCAST(dvoid**,&errhp),
#endif
    OCI_HTYPE_ERROR,
    0,
    nullptr);
  if(status)return 0;

#if defined(__GNUC__) && (__GNUC__>=4)
  void* temp_srvhp=&srvhp;
#endif
  status=OCIHandleAlloc
   (OTL_RCAST(dvoid*,envhp),
#if defined(__GNUC__) && (__GNUC__>=4)
    OTL_RCAST(dvoid**,temp_srvhp),
#else
    OTL_RCAST(dvoid**,&srvhp),
#endif
    OCI_HTYPE_SERVER,
    0,
    nullptr);
  if(status)return 0;

#if defined(__GNUC__) && (__GNUC__>=4)
  void * temp_svchp=&svchp;
#endif
  status=OCIHandleAlloc
   (OTL_RCAST(dvoid*,envhp),
#if defined(__GNUC__) && (__GNUC__>=4)
    OTL_RCAST(dvoid**,temp_svchp),
#else
    OTL_RCAST(dvoid**,&svchp),
#endif
    OCI_HTYPE_SVCCTX,
    0,
    nullptr);
  if(status)return 0;

  status=OCIServerAttach
   (srvhp,
    errhp,
    tnsname==nullptr?
      OTL_RCAST(text*,OTL_CCAST(char*,"")):
      OTL_RCAST(text*,OTL_CCAST(char*,tnsname)),
    tnsname==nullptr?
      0:OTL_SCAST(sb4,strlen(OTL_CCAST(char*,tnsname))),
    0);
  if(status)return 0;
  status=OCIAttrSet
   (OTL_RCAST(dvoid*,svchp),
    OCI_HTYPE_SVCCTX,
    OTL_RCAST(dvoid*,srvhp),
    0,
    OCI_ATTR_SERVER,
    OTL_RCAST(OCIError*,errhp));
  if(status)return 0;

  if(xa_server_external_name!=nullptr && xa_server_internal_name!=nullptr){
    status=OCIAttrSet
      (OTL_RCAST(dvoid*,srvhp),
       OCI_HTYPE_SERVER,
       OTL_RCAST(dvoid*,xa_server_external_name),
       0,
       OCI_ATTR_EXTERNAL_NAME,
       errhp);        
    if(status)return 0;
    
    status=OCIAttrSet
      (OTL_RCAST(dvoid*,srvhp),
       OCI_HTYPE_SERVER,
       OTL_RCAST(dvoid*,xa_server_internal_name),
       0,
       OCI_ATTR_INTERNAL_NAME,
       errhp);   
    if(status)return 0; 
  }

#if defined(__GNUC__) && (__GNUC__>=4)
  void* temp_authp=&authp;
#endif
  status=OCIHandleAlloc
   (OTL_RCAST(dvoid*,envhp),
#if defined(__GNUC__) && (__GNUC__>=4)
    OTL_RCAST(dvoid **,temp_authp),
#else
    OTL_RCAST(dvoid **,&authp),
#endif
    OTL_SCAST(ub4,OCI_HTYPE_SESSION),
    0,
    nullptr);
  if(status)return 0;

  attached=1;
  return 1;

 }

 int session_begin(const int aauto_commit)
  {int& status=last_status;
  int cred_type;

  if(!attached)return 0;
  if(session_begin_count==0)return 0;

  if(ext_cred)
   cred_type=OCI_CRED_EXT;
  else
   cred_type=OCI_CRED_RDBMS;
  status=OCISessionBegin
   (svchp,
    errhp,
    authp,
    cred_type,
    OTL_SCAST(ub4,session_mode_));
  if(status!=OCI_SUCCESS && 
     status!=OCI_SUCCESS_WITH_INFO)
    return 0;

  in_session=1;
  auto_commit=aauto_commit;
  ++session_begin_count;
  return 1;

 }

 int session_begin
   (const char* userid,
    const char* password,
    const int aauto_commit,
    const int session_mode=OCI_DEFAULT)
 {int& status=last_status;
  int cred_type;

  if(!attached)return 0;

  if(userid[0]==0&&password[0]==0){
   ext_cred=1;
   cred_type=OCI_CRED_EXT;
  }else{
   ext_cred=0;
   cred_type=OCI_CRED_RDBMS;
   status=OCIAttrSet
    (OTL_RCAST(dvoid*,authp),
     OTL_SCAST(ub4,OCI_HTYPE_SESSION),
     OTL_RCAST(dvoid*,OTL_CCAST(char*,userid)),
     OTL_SCAST(ub4,strlen(OTL_CCAST(char*,userid))),
     OTL_SCAST(ub4,OCI_ATTR_USERNAME),
     errhp);
   if(status)return 0;
   status=OCIAttrSet
    (OTL_RCAST(dvoid*,authp),
     OTL_SCAST(ub4,OCI_HTYPE_SESSION),
     OTL_RCAST(dvoid*,OTL_CCAST(char*,password)),
     OTL_SCAST(ub4,strlen(OTL_CCAST(char*,password))),
     OTL_SCAST(ub4,OCI_ATTR_PASSWORD),
     errhp);
   if(status)return 0;
  }
  session_mode_=session_mode;
  status=OCISessionBegin
   (svchp,
    errhp,
    authp,
    cred_type,
    OTL_SCAST(ub4,session_mode_));
  if(status!=OCI_SUCCESS &&
     status!=OCI_SUCCESS_WITH_INFO)
    return 0;
  status=OCIAttrSet
   (OTL_RCAST(dvoid*,svchp),
    OTL_SCAST(ub4,OCI_HTYPE_SVCCTX),
    OTL_RCAST(dvoid*,authp),
    0,
    OTL_SCAST(ub4,OCI_ATTR_SESSION),
    errhp);
  if(status)return 0;
  in_session=1;
  auto_commit=aauto_commit;
  ++session_begin_count;
  return 1;

 }

#if defined(OTL_ORA8I) || defined(OTL_ORA9I)
  int change_password
  (const char* user_name,
   const char* password,
   const char* new_password)
  {int& status=last_status;
    
    OCIAttrSet
      (OTL_RCAST(dvoid*,svchp),
       OTL_SCAST(ub4,OCI_HTYPE_SVCCTX),
       OTL_RCAST(dvoid *,authp),
       0,
       OTL_SCAST(ub4,OCI_ATTR_SESSION),
       errhp);
    
    status=OCIPasswordChange 
      (svchp,  
       errhp,
       OTL_RCAST(text*,OTL_CCAST(char*,user_name)),
       OTL_SCAST(ub4,strlen(user_name)),
       OTL_RCAST(text*,OTL_CCAST(char*,password)),
       OTL_SCAST(ub4,strlen(password)),
       OTL_RCAST(text*,OTL_CCAST(char*,new_password)),
       OTL_SCAST(ub4,strlen(new_password)),
       OCI_AUTH);
    if(status)
      return 0;
    else
      return 1;
  
  }
#endif

 int server_detach(void)
 {int rc=0;
  if(attached){
   OCIServerDetach(srvhp,errhp,OTL_SCAST(ub4,OCI_DEFAULT));
   rc=1;
  }
  if(authp!=nullptr)OCIHandleFree(OTL_RCAST(dvoid*,authp),
                            OTL_SCAST(ub4,OCI_HTYPE_SESSION));
  if(errhp!=nullptr)OCIHandleFree(OTL_RCAST(dvoid*,errhp),
                            OTL_SCAST(ub4,OCI_HTYPE_ERROR));
  if(svchp!=nullptr)OCIHandleFree(OTL_RCAST(dvoid*,svchp),
                            OTL_SCAST(ub4,OCI_HTYPE_SVCCTX));
  if(srvhp!=nullptr)OCIHandleFree(OTL_RCAST(dvoid*,srvhp),
                            OTL_SCAST(ub4,OCI_HTYPE_SERVER));
  if(envhp!=nullptr)OCIHandleFree(OTL_RCAST(dvoid*,envhp),
                            OTL_SCAST(ub4,OCI_HTYPE_ENV));
  auto_commit=0;
  attached=0;
  in_session=0;
  envhp=nullptr;
  srvhp=nullptr;
  errhp=nullptr;
  svchp=nullptr;
  authp=nullptr;
  delete_xa_strings();
  return rc;
 }

 int session_end(void)
 {int& status=last_status;
  if(!in_session)return 0;
  status=OCISessionEnd(svchp,errhp,authp,0);
  if(status)return 0;

  in_session=0;
  auto_commit=0;
  return 1;
 }

 int auto_commit_on(void)
 {
  auto_commit=1;
  return 1;
 }

 int auto_commit_off(void)
 {
  auto_commit=0;
  return 1;
 }

 int rlogon(const char* connect_str,const int aauto_commit)
 {
   int status;
   char username[256];
   char passwd[256];
   char tnsname[1024];
   char* tnsname_ptr=nullptr;
   char* username_ptr=username;
   char* c=OTL_CCAST(char*,connect_str);
   char* passwd_ptr=passwd;
   char prev_c=' ';
   
   auto_commit=aauto_commit;
   
   username[0]=0;
   passwd[0]=0;
   tnsname[0]=0;
   
   while(*c&&*c!='/'&&(OTL_SCAST(unsigned,username_ptr-username)<
                       sizeof(username)-1)){
    *username_ptr=*c;
    ++c;
    ++username_ptr;
   }
   *username_ptr=0;
   if(*c=='/')++c;
   prev_c=' ';
   while(*c && !(*c=='@' && prev_c!='\\') &&
         (OTL_SCAST(unsigned,passwd_ptr-passwd)<sizeof(passwd)-1)){
     if(prev_c=='\\')--passwd_ptr;
     *passwd_ptr=*c;
     prev_c=*c;
     ++c;
     ++passwd_ptr;
   }
   *passwd_ptr=0;
   
   if(*c=='@'){
     ++c;
     tnsname_ptr=tnsname;
     while(*c&&(OTL_SCAST(unsigned,tnsname_ptr-tnsname)<sizeof(tnsname)-1)){
       *tnsname_ptr=*c;
       ++c;
       ++tnsname_ptr;
     }
     *tnsname_ptr=0;
   }
   
   envhp=nullptr;
   srvhp=nullptr;
   errhp=nullptr;
   svchp=nullptr;
   authp=nullptr;
   extern_lda=0;
   attached=0;
   in_session=0;

  OTL_TRACE_RLOGON_ORA8
    (0x1,
     "otl_connect",
     "rlogon",
     tnsname,
     username,
     passwd,
     auto_commit)
   
   status=server_attach(tnsname);
   if(!status)return 0;
   
   status=session_begin(username,passwd,aauto_commit);
   if(!status)return 0;
   
   return 1;

 }

 int ext_logon(OCIEnv *a_envhp,OCISvcCtx *a_svchp,const int aauto_commit=0)
 {int& status=last_status;

  envhp=a_envhp;
  svchp=a_svchp;
  errhp=nullptr;
  srvhp=nullptr;
  authp=nullptr;
  extern_lda=1;
  auto_commit=aauto_commit;

#if defined(__GNUC__) && (__GNUC__>=4)
  void* temp_errhp=&errhp;
#endif
  status=OCIHandleAlloc
   (OTL_RCAST(dvoid*,envhp),
#if defined(__GNUC__) && (__GNUC__>=4)
    OTL_RCAST(dvoid**,temp_errhp),
#else
    OTL_RCAST(dvoid**,&errhp),
#endif
    OCI_HTYPE_ERROR,
    0,
    nullptr);
  if(status)return 0;

  return 1;

 }

 int logoff(void)
 {
  int rc;
  if(extern_lda){
   OCIHandleFree(OTL_RCAST(dvoid*,errhp), OTL_SCAST(ub4,OCI_HTYPE_ERROR));
   envhp=nullptr;
   svchp=nullptr;
   errhp=nullptr;
   extern_lda=0;
  }else{
   rc=session_end();
   if(!rc)return 0;
   rc=server_detach();
   if(!rc)return 0;
  }
  auto_commit=0;
  return 1;
 }

 void error(otl_exc& exception_struct)
 {sb4 errcode;
  size_t len;
  OCIErrorGet
   (OTL_RCAST(dvoid*,errhp),
    OTL_SCAST(ub4,1),
    nullptr,
    &errcode,
    OTL_RCAST(text*,exception_struct.msg),
    OTL_SCAST(ub4,sizeof(exception_struct.msg)),
    OCI_HTYPE_ERROR);
  exception_struct.code=errcode;
  len=strlen(OTL_RCAST(char*,exception_struct.msg));
  exception_struct.msg[len]=0;
 }

 int commit(void)
 {
   last_status=OCITransCommit(svchp,errhp,OTL_SCAST(ub4,OCI_DEFAULT));
   return !last_status;
 }

#if defined(OTL_ORA10G_R2)

 int commit_nowait(void)
 {
#if defined(OCI_TRANS_WRITENOWAIT)
   last_status=OCITransCommit
     (svchp,
      errhp,
      OTL_SCAST(ub4,OCI_TRANS_WRITENOWAIT));
#else
   last_status=OCITransCommit
     (svchp,
      errhp,
      OTL_SCAST(ub4,0x00000008));
#endif
   return !last_status;
 }

#endif

 int rollback(void)
 {
   last_status=OCITransRollback(svchp,errhp,OTL_SCAST(ub4,OCI_DEFAULT));
   return !last_status;
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
  public:
  otl_conn(const otl_conn&) = delete;
  otl_conn& operator=(const otl_conn&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_conn(otl_conn&&) = delete;
  otl_conn& operator=(otl_conn&&) = delete;
#endif
  private:
#else
  otl_conn(const otl_conn&):
    envhp(nullptr),
    srvhp(nullptr),
    errhp(nullptr),
    svchp(nullptr),
    authp(nullptr),
    auto_commit(0),
    extern_lda(0),
    attached(0),
    in_session(0),
    char_set_(SQLCS_IMPLICIT),
    session_begin_count(0),
    session_mode_(OCI_DEFAULT),
    ext_cred(0),
    last_status(OCI_SUCCESS),
    xa_server_external_name(nullptr),
    xa_server_internal_name(nullptr)
#if defined(OTL_ORA_OCI_ENV_CREATE)
    ,threaded_mode(false)
#endif
 {
 }

  otl_conn& operator=(const otl_conn&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_conn(otl_conn&&):
    envhp(nullptr),
    srvhp(nullptr),
    errhp(nullptr),
    svchp(nullptr),
    authp(nullptr),
    auto_commit(0),
    extern_lda(0),
    attached(0),
    in_session(0),
    char_set_(SQLCS_IMPLICIT),
    session_begin_count(0),
    session_mode_(OCI_DEFAULT),
    ext_cred(0),
    last_status(OCI_SUCCESS),
    xa_server_external_name(nullptr),
    xa_server_internal_name(nullptr)
#if defined(OTL_ORA_OCI_ENV_CREATE)
    ,threaded_mode(false)
#endif
 {
 }

  otl_conn& operator=(otl_conn&&)
  {
    return *this;
  }
#endif
#endif

};

class otl_cur0{
public:
  virtual ~otl_cur0(){}
};

class otl_cur;
class otl_inout_stream;
class otl_refcur_stream;
class otl_ref_select_stream;

class otl_var{
private:

  friend class otl_cur;
  friend class otl_inout_stream;
  friend class otl_refcur_stream;
  friend class otl_ref_select_stream;

  ub1* p_v;
  sb2* p_ind;
  ub2* p_rlen;
  ub2* p_rcode;
  int ftype;
  int array_size;
  int elem_size;
  bool nls_flag;
  OCILobLocator** lob;
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  OCIDateTime** timestamp;
#endif
  OCIStmt* cda;
  otl_conn* connect;
  ub1* buf;
  int buf_len;
  int real_buf_len;
  int ext_buf_flag;
  int act_elem_size;
  ub4 max_tab_len;
  ub4 cur_tab_len;
  int pl_tab_flag;
  int lob_stream_flag;
  int vparam_type;
  int lob_len;
  int lob_pos;
  int lob_ftype;
  int otl_adapter;
  bool lob_stream_mode;
  sb4 unicode_var_len;
  ub2 csid; 
  ub1 csfrm;
  ub4 read_blob_amt;
  ub4 total_read_blob_amt;
  bool charz_flag;
  bool select_stm_flag;

public:

  void set_total_read_blob_amt(const int new_total_read_blob_amt)
  {
    total_read_blob_amt=OTL_SCAST(ub4,new_total_read_blob_amt);
  }

  int get_otl_adapter() const {return otl_adapter;}
  OCIStmt* get_cda(){return cda;}
  OCIStmt** get_cda_ptr(){return &cda;}
  void set_nls_flag(const bool anls_flag)
  {
    nls_flag=anls_flag;
  }

  void set_lob_stream_mode(const bool alob_stream_mode)
  {
    lob_stream_mode=alob_stream_mode;
  }

  void set_vparam_type(const int avparam_type)
  {
    vparam_type=avparam_type;
  }

  void set_charz_flag(const bool acharz_flag)
  {
    charz_flag=acharz_flag;
  }

  otl_var():
    p_v(nullptr),
    p_ind(nullptr),
    p_rlen(nullptr),
    p_rcode(nullptr),
    ftype(0),
    array_size(0),
    elem_size(0),
    nls_flag(false),
    lob(nullptr),
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
    timestamp(nullptr),
#endif
    cda(nullptr),
    connect(nullptr),
    buf(nullptr),
    buf_len(0),
    real_buf_len(0),
    ext_buf_flag(0),
    act_elem_size(0),
    max_tab_len(0),
    cur_tab_len(0),
    pl_tab_flag(0),
    lob_stream_flag(0),
    vparam_type(-1),
    lob_len(0),
    lob_pos(0),
    lob_ftype(0),
    otl_adapter(otl_ora8_adapter),
    lob_stream_mode(false),
    unicode_var_len(0),
    csid(0),
    csfrm(SQLCS_IMPLICIT),
    read_blob_amt(0),
    total_read_blob_amt(0),
    charz_flag(false),
    select_stm_flag(false)
 {
 }

 virtual ~otl_var()
 {int i;
  if(ftype==otl_var_refcur&&cda!=nullptr){
    OCIHandleFree(OTL_RCAST(dvoid*,cda),OCI_HTYPE_STMT);
    cda=nullptr;
  }
  if(ftype==otl_var_blob||(ftype==otl_var_clob&&lob!=nullptr)){
   for(i=0;i<array_size;++i)
    OCIDescriptorFree(OTL_RCAST(dvoid*,lob[i]),
                      OTL_SCAST(ub4,OCI_DTYPE_LOB));
  }
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  if((ftype==otl_var_timestamp ||
      ftype==otl_var_tz_timestamp ||
      ((ftype==otl_var_ltz_timestamp)&&
       timestamp!=nullptr))){
    ub4 dtype=0;
    switch(ftype){
    case otl_var_timestamp:
      dtype=OCI_DTYPE_TIMESTAMP;
      break;
    case otl_var_ltz_timestamp:
      dtype=OCI_DTYPE_TIMESTAMP_LTZ;
      break;
    case otl_var_tz_timestamp:
      dtype=OCI_DTYPE_TIMESTAMP_TZ;
      break;
    }
   for(i=0;i<array_size;++i)
    OCIDescriptorFree(OTL_RCAST(dvoid*,timestamp[i]),dtype);
  }
#endif
  delete[] p_v;
  delete[] p_ind;
  delete[] p_rlen;
  delete[] p_rcode;
  if(!ext_buf_flag)
   delete[] buf;
 }

  int write_dt(void* trg, 
               const void* src, 
               const int 
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
#else
               sz
#endif
              )
  {
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
    OCIDateTime* trg_ptr=OTL_RCAST(OCIDateTime*,trg);
    otl_datetime* src_ptr=OTL_RCAST(otl_datetime*,OTL_CCAST(void*,src));
    int rc=0;
    if(ftype!=otl_var_tz_timestamp){
      rc=OCIDateTimeConstruct 
        (connect->envhp,
         connect->errhp,
         trg_ptr,
         OTL_SCAST(sb2,src_ptr->year),
         OTL_SCAST(ub1,src_ptr->month),
         OTL_SCAST(ub1,src_ptr->day),
         OTL_SCAST(ub1,src_ptr->hour),
         OTL_SCAST(ub1,src_ptr->minute),
         OTL_SCAST(ub1,src_ptr->second),
         OTL_SCAST
         (ub4,otl_to_fraction(src_ptr->fraction,
                              src_ptr->frac_precision)),
         nullptr,
         0);
    }else{
      int tz_hour=src_ptr->tz_hour;
      int tz_minute=src_ptr->tz_minute;
      char tz_str[60];
      char* tzc=otl_itoa(tz_hour,tz_str);
      *tzc=':';
      ++tzc;
      tzc=otl_itoa(tz_minute,tzc);
      size_t tz_len=tzc-tz_str;
      rc=OCIDateTimeConstruct 
        (connect->envhp,
         connect->errhp,
         trg_ptr,
         OTL_SCAST(sb2,src_ptr->year),
         OTL_SCAST(ub1,src_ptr->month),
         OTL_SCAST(ub1,src_ptr->day),
         OTL_SCAST(ub1,src_ptr->hour),
         OTL_SCAST(ub1,src_ptr->minute),
         OTL_SCAST(ub1,src_ptr->second),
         OTL_SCAST
         (ub4,otl_to_fraction(src_ptr->fraction,
                              src_ptr->frac_precision)),
         OTL_RCAST(text*,tz_str),
         tz_len);
    }
    if(rc!=0)return 0;
    return 1;
#else
    memcpy(trg,src,sz);
    return 1;
#endif
  }

  int read_dt(void* trg, 
              const void* src, 
              const int 
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
#else
              sz
#endif
             )
  {
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
    OCIDateTime* src_ptr=OTL_RCAST(OCIDateTime*,OTL_CCAST(void*,src));
    otl_datetime* trg_ptr=OTL_RCAST(otl_datetime*,OTL_CCAST(void*,trg));
    sb2 year;
    ub1 month, day, hour, minute, sec;
    ub4 fsec;
    sb1 tz_hour;
    sb1 tz_minute;
    int rc=OCIDateTimeGetDate 
      (connect->envhp,
       connect->errhp, 
       src_ptr,
       &year,
       &month,
       &day);
    if(rc!=0)return 0;
    rc=OCIDateTimeGetTime
      (connect->envhp,
       connect->errhp, 
       src_ptr,
       &hour,
       &minute,
       &sec,
       &fsec);
    if(rc!=0)return 0;
    trg_ptr->year=year;
    trg_ptr->month=month;
    trg_ptr->day=day;
    trg_ptr->hour=hour;
    trg_ptr->minute=minute;
    trg_ptr->second=sec;
    trg_ptr->fraction=otl_from_fraction(fsec,trg_ptr->frac_precision);
    trg_ptr->tz_hour=0;
    trg_ptr->tz_minute=0;
    if(ftype==otl_var_tz_timestamp ||
       ftype==otl_var_ltz_timestamp){
      rc=OCIDateTimeGetTimeZoneOffset
        (connect->envhp,
         connect->errhp, 
         src_ptr,
         &tz_hour,
         &tz_minute);
      if(rc!=0)return 0;
      trg_ptr->tz_hour=tz_hour;
      trg_ptr->tz_minute=tz_minute;
    }
    return 1;
#else
    memcpy(trg,src,sz);
    return 1;
#endif
  }


 int actual_elem_size(void)
 {
  return act_elem_size;
 }

 void init
 (const bool aselect_stm_flag,
  const int aftype,
  int& aelem_size,
  const otl_stream_buffer_size_type aarray_size,
  const void* connect_struct=nullptr,
  const int apl_tab_flag=0)
 {
  int i;
  ub4 lobEmpty=0;
  select_stm_flag=aselect_stm_flag;
  connect=OTL_RCAST(otl_conn*,OTL_CCAST(void*,connect_struct));
  ftype=aftype;
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
  if(ftype==otl_var_nchar){
    ftype=otl_var_char;
    nls_flag=true;
  }else if(ftype==otl_var_nclob){
    ftype=otl_var_clob;
    nls_flag=true;
  }
#endif
  pl_tab_flag=apl_tab_flag;
  act_elem_size=aelem_size;
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  if((ftype==otl_var_timestamp ||
      ftype==otl_var_tz_timestamp ||
      ftype==otl_var_ltz_timestamp) &&
      apl_tab_flag)
    act_elem_size=sizeof(otl_oracle_date);
#endif
  if(ftype==otl_var_refcur){
   array_size=aarray_size;
   elem_size=1;
#if defined(__GNUC__) && (__GNUC__>=4)
   void* temp_cda=&cda;
#endif
   OCIHandleAlloc
     (OTL_RCAST(dvoid*,connect->get_envhp()),
#if defined(__GNUC__) && (__GNUC__>=4)
     OTL_RCAST(dvoid**,temp_cda),
#else
     OTL_RCAST(dvoid**,&cda),
#endif
     OCI_HTYPE_STMT,
     0,
     nullptr);
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  }else if((ftype==otl_var_timestamp ||
            ftype==otl_var_tz_timestamp ||
            ftype==otl_var_ltz_timestamp) &&
           !apl_tab_flag){
   array_size=aarray_size;
   elem_size=sizeof(OCIDateTime*);
   act_elem_size=elem_size;
   timestamp=new OCIDateTime*[array_size];
   p_v=OTL_RCAST(ub1*,timestamp); 
   p_ind=new sb2[array_size];
   p_rlen=new ub2[array_size];
   p_rcode=new ub2[array_size];
   for(i=0;i<array_size;++i){
     p_ind[i]=OTL_SCAST(short,elem_size);
     p_rlen[i]=OTL_SCAST(unsigned short,elem_size);
     p_rcode[i]=0;
   }
   if(connect!=nullptr){
     otl_datetime dt;
     ub4 dtype=0;
     switch(ftype){
     case otl_var_timestamp:
       dtype=OCI_DTYPE_TIMESTAMP;
       break;
     case otl_var_ltz_timestamp:
       dtype=OCI_DTYPE_TIMESTAMP_LTZ;
       break;
     case otl_var_tz_timestamp:
       dtype=OCI_DTYPE_TIMESTAMP_TZ;
       break;
     }
     for(i=0;i<array_size;++i){
       OCIDescriptorAlloc
         (OTL_RCAST(dvoid*,connect->envhp),
          OTL_RCAST(dvoid**,&timestamp[i]),
          dtype,
          0,
          nullptr);
       write_dt(timestamp[i],&dt,1);
    }
   }else
    timestamp=nullptr;
#endif
  }else if(ftype==otl_var_blob||ftype==otl_var_clob){
   array_size=aarray_size;
   elem_size=aelem_size;
   lob=new OCILobLocator*[array_size];
   p_v=OTL_RCAST(ub1*,lob);
   p_ind=new sb2[array_size];
   p_rlen=nullptr;
   p_rcode=nullptr;
   if(connect!=nullptr){
    for(i=0;i<array_size;++i){
     OCIDescriptorAlloc
       (OTL_RCAST(dvoid*,connect->get_envhp()),
       OTL_RCAST(dvoid**,&lob[i]),
       OTL_SCAST(ub4,OCI_DTYPE_LOB),
       0,
       nullptr);
     lobEmpty=0;
     OCIAttrSet
      (OTL_RCAST(dvoid*,lob[i]),
       OCI_DTYPE_LOB,
       OTL_RCAST(dvoid*,&lobEmpty),
       0,
       OCI_ATTR_LOBEMPTY,
       OTL_RCAST(OCIError*,connect->get_errhp()));
    }
   }else
    lob=nullptr;
  }else{
   if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long){
    elem_size=aelem_size+sizeof(sb4);
    array_size=1;
   }else if(ftype==otl_var_raw){
    elem_size=aelem_size+sizeof(short int);
    array_size=aarray_size;
   }else{
    elem_size=aelem_size;
    array_size=aarray_size;
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
    if((ftype==otl_var_timestamp ||
        ftype==otl_var_tz_timestamp ||
        ftype==otl_var_ltz_timestamp) && 
       apl_tab_flag){
      elem_size=sizeof(otl_oracle_date);
      aelem_size=elem_size; // sending feedback back to the template class
    }
#endif
   }
#if defined(OTL_UNICODE)
   if(ftype==otl_var_char){
     unsigned unicode_buffer_size=
       elem_size*OTL_SCAST(unsigned,array_size)*sizeof(OTL_WCHAR);
     p_v=new ub1[unicode_buffer_size];
     memset(p_v,0,unicode_buffer_size);
   } else if(ftype==otl_var_varchar_long){
     unsigned unicode_buffer_size=elem_size;
     p_v=new ub1[unicode_buffer_size];
     memset(p_v,0,unicode_buffer_size);
   }else{
     p_v=new ub1[elem_size*OTL_SCAST(unsigned,array_size)];
     memset(p_v,0,elem_size*OTL_SCAST(unsigned,array_size));
   }
#elif defined(OTL_ORA_UTF8)
   if(ftype==otl_var_char){
     unsigned buffer_size=elem_size*OTL_SCAST(unsigned,array_size);
     if(select_stm_flag)
       buffer_size*=OTL_UTF8_BYTES_PER_CHAR; // 3 bytes per UTF8 char on SELECT by default
     p_v=new ub1[buffer_size];
     memset(p_v,0,buffer_size);
   } else if(ftype==otl_var_varchar_long){
     p_v=new ub1[elem_size];
     memset(p_v,0,elem_size);
   }else{
     p_v=new ub1[elem_size*OTL_SCAST(unsigned,array_size)];
     memset(p_v,0,elem_size*OTL_SCAST(unsigned,array_size));
   }
#else
   p_v=new ub1[elem_size*OTL_SCAST(unsigned,array_size)];
   memset(p_v,0,elem_size*OTL_SCAST(unsigned,array_size));
#endif
   p_ind=new sb2[array_size];
   p_rlen=new ub2[array_size];
   p_rcode=new ub2[array_size];
   if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long||ftype==otl_var_raw){
    if(aelem_size>otl_short_int_max)
     p_ind[0]=0;
    else
      p_ind[0]=OTL_SCAST(short,aelem_size);
    p_rcode[0]=0;
   }else{
    for(i=0;i<array_size;++i){
#if defined(OTL_UNICODE)
      if(ftype==otl_var_char){
        p_ind[i]=OTL_SCAST(short,elem_size*sizeof(OTL_WCHAR));
        p_rlen[i]=OTL_SCAST(unsigned short,elem_size*sizeof(OTL_WCHAR));
        p_rcode[i]=0;
      }else{
        p_ind[i]=OTL_SCAST(short,elem_size);
        p_rlen[i]=OTL_SCAST(unsigned short,elem_size);
        p_rcode[i]=0;
      }
#else
      if(ftype==otl_var_raw){
        p_ind[i]=OTL_SCAST(short,elem_size);
        p_rlen[i]=OTL_SCAST(unsigned short,elem_size);
        p_rcode[i]=0;
      }else{
        if(elem_size>otl_short_int_max)
          p_ind[i]=0;
        else{
          p_ind[i]=OTL_SCAST(short,elem_size);
          p_rlen[i]=OTL_SCAST(unsigned short,elem_size);
        }
        p_rcode[i]=0;
      }
#endif
    }
   }
  }
  max_tab_len=OTL_SCAST(ub4,array_size);
  cur_tab_len=0;
 }

 void set_pl_tab_len(const int apl_tab_len)
 {
  max_tab_len=OTL_SCAST(ub4,array_size);
  cur_tab_len=OTL_SCAST(ub4,apl_tab_len);
 }

 int get_pl_tab_len(void)
 {
   return OTL_SCAST(int,cur_tab_len);
 }

 int get_max_pl_tab_len(void)
 {
  return OTL_SCAST(int,max_tab_len);
 }

 int get_blob_len(const int ndx,int& alen)
 {
   ub4 blen=0;
   int rc;
   alen=0;
   rc=OCILobGetLength
     (connect->get_svchp(),
      connect->get_errhp(),
      lob[ndx],
      &blen);
   alen=OTL_SCAST(int,blen);
   if(rc!=OCI_SUCCESS)return 0;
   return 1;

 }

  int is_blob_initialized(const int ndx,int& is_init)
  {
    int rc;
    is_init=0;
    rc=OCILobLocatorIsInit
      (connect->get_envhp(),
       connect->get_errhp(),
       lob[ndx],
       &is_init);
    if(rc!=OCI_SUCCESS)
      return 0;
    else
      return 1;
  }

 int get_blob
 (const int ndx,
  unsigned char* abuf,
  const int buf_size,
  int& len)
 {
  int byte_buf_size=buf_size;
#if defined(OTL_UNICODE)
  if(ftype==otl_var_clob)
   byte_buf_size=buf_size*sizeof(OTL_CHAR);
#endif  
  ub4 amt=byte_buf_size;
  ub4 offset=1;
  int rc;
#if defined(OTL_UNICODE)
  if(ftype==otl_var_clob||ftype==otl_var_nclob)
    memset(OTL_RCAST(void*,abuf),0,OTL_SCAST(size_t,buf_size));
#endif
  int is_init=0;
  rc=OCILobLocatorIsInit
    (connect->get_envhp(),
     connect->get_errhp(),
     lob[ndx],
     &is_init);
  if (rc!=0) return 0;
  if (!is_init){
   len=0;
   return 1;
  }
#if defined(OTL_UNICODE)
  if(ftype==otl_var_clob)
    csid=OTL_UNICODE_ID;
  else
    csid=0;
#else
  csid=0;
#endif
  do{
    rc=OCILobRead
      (connect->get_svchp(),
       connect->get_errhp(),
       lob[ndx],
       &amt,
       offset,
       OTL_RCAST(dvoid*,abuf+offset-1),
       OTL_SCAST(ub4,byte_buf_size-offset+1),
       nullptr,
       nullptr,
       csid,
       OTL_SCAST(ub1,nls_flag?SQLCS_NCHAR:connect->get_char_set()));
    offset+=amt;
  }while(rc==OCI_NEED_DATA);
  len=offset-1;
  if(rc!=OCI_SUCCESS){
    len=0;
    return 0;
  }
  return 1;
 }

 void set_lob_stream_flag(const int flg=1)
 {
  lob_stream_flag=flg;
 }

  int close_lob(void)
  {
#if defined(OTL_ORA8I)||defined(OTL_ORA9I)
    int rc;
    boolean flag=0;
    rc=OCILobIsOpen
      (connect->get_svchp(),
       connect->get_errhp(),
       lob[0],
       &flag);
    if(rc!=OCI_SUCCESS)return 0;
    if(flag!=TRUE)return 1;
    rc=OCILobClose
      (connect->get_svchp(),
       connect->get_errhp(),
       lob[0]);
    if(rc!=OCI_SUCCESS)return 0;
#endif
    return 1;
  }

  void close_temporary_lob(void)
  {
#ifdef OTL_ORA_CUSTOM_FREE_TEMP_LOB
    OCILobFreeTemporary(connect->svchp, connect->errhp, lob[0]);
#endif
  }


 int put_blob(void)
 {
   if((ftype!=otl_var_clob&&ftype!=otl_var_blob)||
     lob_stream_flag||buf==nullptr||buf_len==0)return 1;
  int rc;
  int byte_buf_len=buf_len;
#if defined(OTL_UNICODE)
  if(ftype==otl_var_clob)
   byte_buf_len=buf_len*sizeof(OTL_CHAR);
#endif
  ub4 amt=OTL_SCAST(ub4,buf_len);
  ub4 offset=1;
#if defined(OTL_UNICODE)
  if(ftype==otl_var_clob)
    csid=OTL_UNICODE_ID;
  else
    csid=0;
#else
  csid=0;
#endif
  rc=OCILobWrite
    (connect->get_svchp(),
     connect->get_errhp(),
     lob[0],
     &amt,
     offset,
     OTL_RCAST(dvoid*,buf),
     OTL_SCAST(ub4,byte_buf_len),
     OCI_ONE_PIECE,
     nullptr,
     nullptr,
     csid,
     OTL_SCAST(ub1,nls_flag?SQLCS_NCHAR:connect->get_char_set()));
  if(rc!=0)return 0;
  return 1;
 }

 int read_blob
 (otl_long_string& s,
  const int andx,
  int& aoffset,
  int alob_len)
 {
   ub4 byte_buf_size=s.get_buf_size();

#if defined(OTL_UNICODE)
  if(ftype==otl_var_clob)
   byte_buf_size=byte_buf_size*sizeof(OTL_CHAR);
#endif

  ub4& amt=read_blob_amt;
  amt=0;
  if(aoffset==1)total_read_blob_amt=0;
  ub4& offset=total_read_blob_amt;
  if(offset==0)offset=1;
  int rc;
  int is_init=0;
  rc=OCILobLocatorIsInit
    (connect->get_envhp(),
     connect->get_errhp(),
      lob[0],
      &is_init);
  if(rc!=OCI_SUCCESS)return 0;
  if(!is_init){
   s.set_len(0);
   return 1;
  }
#if defined(OTL_UNICODE)
  if(ftype==otl_var_clob)
    csid=OTL_UNICODE_ID;
  else
    csid=0;
#else
  csid=0;
#endif

  rc=OCILobRead
    (connect->get_svchp(),
     connect->get_errhp(),
    lob[andx],
    &amt,
    offset,
    OTL_RCAST(dvoid*,s.v),
    OTL_SCAST(ub4,byte_buf_size),
    nullptr,
    nullptr,
    csid,
     OTL_SCAST(ub1,nls_flag?SQLCS_NCHAR:connect->get_char_set()));

#if defined(OTL_UNICODE)
  if(ftype==otl_var_clob && aoffset>1 && amt==byte_buf_size)
    amt/=sizeof(OTL_CHAR);
#endif

#if defined(OTL_ORA_UTF8)
  switch(rc){
  case OCI_SUCCESS:
    s.set_len(amt);
    offset+=amt;
    if(ftype==otl_var_blob)
      aoffset+=s.len();
    else
      aoffset=alob_len+1;
    return 1;
  case OCI_NEED_DATA:
    s.set_len(amt);
    offset+=amt;
    if(ftype==otl_var_blob)
      aoffset+=s.len();
    else
      aoffset=2;
    return 1;
  case OCI_ERROR:
  default:
    s.set_len(0);
    return 0;
  }
#else
  switch(rc){
  case OCI_SUCCESS:
    if(aoffset==1)
      s.set_len(alob_len);
    else
      s.set_len(alob_len-aoffset+1);
    break;
  case OCI_NEED_DATA:
    s.set_len(amt);
    break;
  case OCI_ERROR:
    s.set_len(0);
    break;
  }
  if(rc==OCI_NEED_DATA||rc==OCI_SUCCESS){
    aoffset+=s.len();
    return 1;
  }else
    return 0;
#endif

 }

 int write_blob
 (const otl_long_string& s,
  const int alob_len,
  int& aoffset,
  otl_cur0& /* cur */)
 {
  if(!lob_stream_flag)return 1;
  int rc;
  int byte_s_length=s.len();
#if defined(OTL_UNICODE)
  if(ftype==otl_var_clob)
   byte_s_length=s.len()*sizeof(OTL_CHAR);
#endif
  ub4 offset=aoffset;
  ub4 amt=0;
  ub1 mode;
  if(aoffset==1 && alob_len>s.len())
   mode=OCI_FIRST_PIECE;
  else if(aoffset==1 && alob_len<=s.len()){
   mode=OCI_ONE_PIECE;
   amt=s.len();
  }else if((aoffset-1)+s.len()<alob_len)
   mode=OCI_NEXT_PIECE;
  else
   mode=OCI_LAST_PIECE;
#if defined(OTL_UNICODE)
  if(ftype==otl_var_clob)
    csid=OTL_UNICODE_ID;
  else
    csid=0;
#else
  csid=0;
#endif
  if(mode==OCI_FIRST_PIECE || mode==OCI_ONE_PIECE){
    rc=OCILobTrim 
      (connect->get_svchp(), 
       connect->get_errhp(), 
       lob[0], 
       0);
    if(rc!=OCI_SUCCESS)
      return 0;
  }
  if(alob_len==0)return 1;
  rc=OCILobWrite
    (connect->get_svchp(),
     connect->get_errhp(),
     lob[0],
     OTL_RCAST(ub4*,&amt),
     offset,
     OTL_RCAST(dvoid*,s.v),
     OTL_SCAST(ub4,byte_s_length),
     mode,
    nullptr,
     nullptr,
     csid,
     OTL_SCAST(ub1,nls_flag?SQLCS_NCHAR:connect->get_char_set()));
  if(rc==OCI_NEED_DATA||
     rc==OCI_SUCCESS||
     rc==OCI_SUCCESS_WITH_INFO){
    aoffset+=s.len();
   return 1;
  }
  return 0;
 }

 int save_blob
 (const unsigned char* abuf,
  const int len,
  const int extern_buffer_flag)
 {
   if(extern_buffer_flag){
     if(buf!=nullptr && !ext_buf_flag){
       delete[] buf;
       buf=nullptr;
     }
     ext_buf_flag=1;
     buf_len=len;
     real_buf_len=len;
     buf=OTL_CCAST(unsigned char*,abuf);
   }else{
     if(!ext_buf_flag && buf!=nullptr && real_buf_len>=len){
       ext_buf_flag=0;
       buf_len=len;
#if defined(OTL_UNICODE)
       memcpy(buf,abuf,buf_len*sizeof(OTL_CHAR));
#else
       memcpy(buf,abuf,buf_len);
#endif
     }else{
       if(buf!=nullptr && !ext_buf_flag){
         delete[] buf;
         buf=nullptr;
       }
       ext_buf_flag=0;
       buf_len=len;
       real_buf_len=len;
#if defined(OTL_UNICODE)
       buf=new ub1[buf_len*sizeof(OTL_CHAR)];
       memcpy(buf,abuf,buf_len*sizeof(OTL_CHAR));
#else
       buf=new ub1[buf_len];
       memcpy(buf,abuf,buf_len);
#endif
     }
   }
   return 1;
 }

 void set_null(int ndx)
 {
  p_ind[ndx]=-1;
 }

 void set_not_null(int ndx, int pelem_size)
 {

   switch(ftype){
   case otl_var_char:
     if(pelem_size>otl_short_int_max)
       p_ind[ndx]=0;
     else
       p_ind[ndx]=OTL_SCAST(short,pelem_size);
     break;
   case otl_var_varchar_long:
   case otl_var_raw_long:
     p_ind[0]=0;
     break;
   case otl_var_raw:
     if(pelem_size>otl_short_int_max)
       p_ind[ndx]=0;
     else
       p_ind[ndx]=OTL_SCAST(short,pelem_size);
     break;
   case otl_var_clob:
   case otl_var_blob:
     if(lob_stream_flag==0){
       ub4 lobEmpty=0;
       OCIAttrSet
         (OTL_RCAST(dvoid*,lob[ndx]),
          OCI_DTYPE_LOB,
          OTL_RCAST(dvoid*,&lobEmpty),
          0,
          OCI_ATTR_LOBEMPTY,
          OTL_RCAST(OCIError*,connect->get_errhp()));
     }
     break;
   default:
     p_ind[ndx]=OTL_SCAST(short,pelem_size);
     break;
   }
 }

 void set_len(int len, int ndx)
 {
  if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long){
#if defined(OTL_UNICODE)
   if(ftype==otl_var_varchar_long)
    *OTL_RCAST(sb4*,p_v)=len*sizeof(OTL_CHAR);
   else
    *OTL_RCAST(sb4*,p_v)=len;
#else
   *OTL_RCAST(sb4*,p_v)=len;
#endif
  }else
   p_rlen[ndx]=OTL_SCAST(unsigned short,len);
 }

 int get_len(int ndx)
 {
  if(ftype==otl_var_varchar_long||ftype==otl_var_raw_long){
   if(p_ind[0]==-1)
    return 0;
   else{
#if defined(OTL_UNICODE)
#if defined(OTL_ORA_UNICODE_LONG_LENGTH_IN_BYTES)
     if(ftype==otl_var_varchar_long)
       return *OTL_RCAST(sb4*,p_v);
     else
       return *OTL_RCAST(sb4*,p_v);
#else
     if(ftype==otl_var_varchar_long)
       return (*OTL_RCAST(sb4*,p_v))/sizeof(OTL_CHAR);
     else
      return *OTL_RCAST(sb4*,p_v);
#endif
#else
    return *OTL_RCAST(sb4*,p_v);
#endif
   }
  }else
   return p_rlen[ndx];
 }

 int is_null(int ndx)
 {
  return p_ind[ndx]==-1;
 }

 void* val(int ndx,int pelem_size)
 {
   switch(ftype){
#if defined(OTL_UNICODE)
   case otl_var_char:
     return OTL_RCAST(void*,&p_v[OTL_SCAST(unsigned,ndx)*
                                 pelem_size*sizeof(OTL_WCHAR)]);
#endif
#if defined(OTL_ORA_UTF8)
   case otl_var_char:
     if(select_stm_flag)
       return OTL_RCAST(void*,&p_v[OTL_SCAST(unsigned,ndx)*
                                   pelem_size*OTL_UTF8_BYTES_PER_CHAR]);
     else
       return OTL_RCAST(void*,&p_v[OTL_SCAST(unsigned,ndx)*pelem_size]);
#endif
   case otl_var_raw:
     return OTL_RCAST(void*,&p_v[(OTL_SCAST(unsigned,ndx))*
                                 (pelem_size+sizeof(short int))]);
   case otl_var_varchar_long:
   case otl_var_raw_long:
     return OTL_RCAST(void*,p_v+sizeof(sb4));
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
   case otl_var_timestamp:
   case otl_var_tz_timestamp:
   case otl_var_ltz_timestamp:
     if(!pl_tab_flag)
       return OTL_RCAST(void*,timestamp[ndx]);
#endif
   default:
     return OTL_RCAST(void*,&p_v[OTL_SCAST(unsigned,ndx)*pelem_size]);
   }
 }

 static int int2ext(int int_type)
 {
  switch(int_type){
  case inVarChar2: return extCChar;
  case inNumber:   return extFloat;
  case inLong:     return extLongVarChar;
  case inRowId:    return extCChar;
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  case inDate:     return extTimestamp;
  case inTimestamp:return extTimestamp;
  case inTimestamp_TZ:return extTimestamp_TZ;
  case inTimestamp_LTZ:return extTimestamp_LTZ;
  case inIntervalYM:return extCChar;
  case inIntervalDS:return extCChar;
#else
  case inDate:     return extDate;
#endif
  case inRaw:      return extRaw;
  case inLongRaw:  return extLongVarRaw;
  case inChar:     return extCChar;
#if defined(OTL_ORA10G)||defined(OTL_ORA10G_R2)
#if defined(OTL_ORA_NATIVE_TYPES) && !defined(OTL_ORA_LEGACY_NUMERIC_TYPES)
  case inBFloat:   return extBFloat;
  case inBDouble:  return extBDouble;
#else
  case inBFloat:   return extFloat;
  case inBDouble:  return extFloat;
#endif
#endif
  case inCLOB:     return extCLOB;
  case inBLOB:     return extBLOB;
  default:
   return otl_unsupported_type;
  }
 }

 static int datatype_size(int aftype,int maxsz,int int_type,int max_long_size)
 {
  switch(aftype){
  case extCChar:
   switch(int_type){
#if defined(OTL_ORA_TIMESTAMP)
   case inIntervalYM:
     return 30;
   case inIntervalDS:
     return 30;
#endif
   case inRowId:
    return 30;
   case inDate:
    return otl_oracle_date_size;
   case inRaw:
    return max_long_size;
   default:
    return maxsz+1;
   }
#if (defined(OTL_ORA10G)||defined(OTL_ORA10G_R2)) && defined(OTL_ORA_NATIVE_TYPES) \
    && !defined(OTL_ORA_LEGACY_NUMERIC_TYPES)
  case extBFloat:
  case extBDouble:
    return sizeof(double);
#endif
  case extLongVarChar:
   return max_long_size;
  case extLongVarRaw:
   return max_long_size;
  case extRaw:
   return maxsz;
  case extCLOB:
   return max_long_size;
  case extBLOB:
   return max_long_size;
  case extFloat:
   return sizeof(double);

#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  case extDate:
   return sizeof(OCIDateTime*);
  case extTimestamp:
  case extTimestamp_TZ:
  case extTimestamp_LTZ:
   return sizeof(OCIDateTime*);
#else
  case extDate:
   return otl_oracle_date_size;
#endif
  default:
   return 0;
  }
 }

 static void map_ftype
 (otl_column_desc& desc,
  const int max_long_size,
  int& aftype,
  int& aelem_size,
  otl_select_struct_override& a_override,
  const int column_ndx,
  const int /*connection_type*/)
 {int ndx=a_override.find(column_ndx);
  if(ndx==-1){
   aftype=int2ext(desc.dbtype);
   aelem_size=datatype_size
     (aftype,
      OTL_SCAST(int,desc.dbsize),
      desc.dbtype,
      max_long_size);
   switch(aftype){
#if (defined(OTL_ORA10G)||defined(OTL_ORA10G_R2))&&defined(OTL_ORA_NATIVE_TYPES)\
    &&!defined(OTL_ORA_LEGACY_NUMERIC_TYPES)
  case extBFloat:
  case extBDouble:
    if(a_override.get_all_mask() & otl_all_num2str){
     aftype=otl_var_char;
     aelem_size=otl_num_str_size;
    }else
     aftype=otl_var_double;
    break;
#endif
   case extCChar:
    aftype=otl_var_char;
    break;
   case extRaw:
    aftype=otl_var_raw;
    break;
   case extFloat:
     if(a_override.get_all_mask() & otl_all_num2str){
       aftype=otl_var_char;
       aelem_size=otl_num_str_size;
     }else{
#if defined(OTL_ORA_CUSTOM_MAP_NUMBER_ON_SELECT)
       OTL_ORA_CUSTOM_MAP_NUMBER_ON_SELECT(aftype,aelem_size,desc);
#else       
       aftype=otl_var_double;
#endif   
     }    
    break;
   case extLongVarChar:
    aftype=otl_var_varchar_long;
    break;
   case extLongVarRaw:
    aftype=otl_var_raw_long;
    break;
   case extCLOB:
    aftype=otl_var_clob;
    break;
   case extBLOB:
    aftype=otl_var_blob;
    break;
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
   case extDate:
   case extTimestamp:
     if(a_override.get_all_mask() & otl_all_date2str){
     aftype=otl_var_char;
     aelem_size=otl_date_str_size;
    }else
      aftype=otl_var_timestamp;
    break;
   case extTimestamp_TZ:
     if(a_override.get_all_mask() & otl_all_date2str){
     aftype=otl_var_char;
     aelem_size=otl_date_str_size;
    }else
      aftype=otl_var_tz_timestamp;
    break;
   case extTimestamp_LTZ:
     if(a_override.get_all_mask() & otl_all_date2str){
     aftype=otl_var_char;
     aelem_size=otl_date_str_size;
    }else
      aftype=otl_var_ltz_timestamp;
    break;
#else
   case extDate:
     if(a_override.get_all_mask() & otl_all_date2str){
     aftype=otl_var_char;
     aelem_size=otl_date_str_size;
    }else
     aftype=otl_var_timestamp;
    break;
#endif
   }
  }else{
    aftype=a_override.get_col_type(ndx);
   switch(aftype){
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
   case otl_var_nchar:
#endif
   case otl_var_char:
     aelem_size=a_override.get_col_size(ndx);
    break;
   case otl_var_raw:
     aelem_size=a_override.get_col_size(ndx);
    break;
   case otl_var_double:
    aelem_size=sizeof(double);
    break;
   case otl_var_float:
    aelem_size=sizeof(float);
    break;
   case otl_var_int:
    aelem_size=sizeof(int);
    break;
#if defined(OTL_BIGINT) && defined(OTL_ORA11G_R2)
   case otl_var_bigint:
    aelem_size=sizeof(OTL_BIGINT);
    break;
#endif
#if defined(OTL_UBIGINT) && defined(OTL_ORA11G_R2)
   case otl_var_ubigint:
    aelem_size=sizeof(OTL_UBIGINT);
    break;
#endif
   case otl_var_unsigned_int:
    aelem_size=sizeof(unsigned);
    break;
   case otl_var_short:
    aelem_size=sizeof(short);
    break;
   case otl_var_long_int:
    aelem_size=sizeof(long);
    break;
   default:
     aelem_size=a_override.get_col_size(ndx);
    break;
   }
  }
  desc.otl_var_dbtype=aftype;
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_var(const otl_var&) = delete;
  otl_var& operator=(const otl_var&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_var(otl_var&&) = delete;
  otl_var& operator=(otl_var&&) = delete;
#endif
private:
#else
  otl_var(const otl_var&):
    p_v(nullptr),
    p_ind(nullptr),
    p_rlen(nullptr),
    p_rcode(nullptr),
    ftype(0),
    array_size(0),
    elem_size(0),
    nls_flag(false),
    lob(nullptr),
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
    timestamp(nullptr),
#endif
    cda(nullptr),
    connect(nullptr),
    buf(nullptr),
    buf_len(0),
    real_buf_len(0),
    ext_buf_flag(0),
    act_elem_size(0),
    max_tab_len(0),
    cur_tab_len(0),
    pl_tab_flag(0),
    lob_stream_flag(0),
    vparam_type(-1),
    lob_len(0),
    lob_pos(0),
    lob_ftype(0),
    otl_adapter(otl_ora8_adapter),
    lob_stream_mode(false),
    unicode_var_len(0),
    csid(0),
    csfrm(SQLCS_IMPLICIT),
    read_blob_amt(0),
    total_read_blob_amt(0),
    charz_flag(false),
    select_stm_flag(false)
 {
 }

  otl_var& operator=(const otl_var&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_var(otl_var&&):
    p_v(nullptr),
    p_ind(nullptr),
    p_rlen(nullptr),
    p_rcode(nullptr),
    ftype(0),
    array_size(0),
    elem_size(0),
    nls_flag(false),
    lob(nullptr),
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
    timestamp(nullptr),
#endif
    cda(nullptr),
    connect(nullptr),
    buf(nullptr),
    buf_len(0),
    real_buf_len(0),
    ext_buf_flag(0),
    act_elem_size(0),
    max_tab_len(0),
    cur_tab_len(0),
    pl_tab_flag(0),
    lob_stream_flag(0),
    vparam_type(-1),
    lob_len(0),
    lob_pos(0),
    lob_ftype(0),
    otl_adapter(otl_ora8_adapter),
    lob_stream_mode(false),
    unicode_var_len(0),
    csid(0),
    csfrm(SQLCS_IMPLICIT),
    read_blob_amt(0),
    total_read_blob_amt(0),
    charz_flag(false),
    select_stm_flag(false)
 {
 }

  otl_var& operator=(otl_var&&)
  {
    return *this;
  }
#endif
#endif

};

class otl_sel;
class otl_refcur_base_cursor;
class otl_refcur_stream;
class otl_ref_cursor;
class otl_ref_select_stream;
#if defined(OTL_ORA_SUBSCRIBE)
  class otl_subscriber;
#endif

class otl_cur: public otl_cur0{
private:

#if defined(OTL_ORA_SUBSCRIBE)
  friend class otl_subscriber;
#endif

  friend class otl_sel;
  friend class otl_refcur_base_cursor;
  friend class otl_refcur_stream;
  friend class otl_ref_cursor;
  friend class otl_ref_select_stream;

  OCIStmt* cda; // Statement handle
  OCIError* errhp; // Error handle
  bool extern_cda;
  int status;
  int eof_status;
  otl_conn* db;
  int straight_select;
  int pos_nbr;
  int commit_on_success;
  int last_param_data_token;
  int last_sql_param_data_status;
  int sql_param_data_count;
  bool canceled;
  int direct_exec_flag;
  int parse_only_flag;
  int stm_executed;

public:

  void set_canceled(const bool acanceld)
  {
    canceled=acanceld;
  }

  void reset_last_param_data_token()
  {
    last_param_data_token=0;
  }

  void reset_last_sql_param_data_status()
  {
    last_sql_param_data_status=0;
  }

  void reset_sql_param_data_count()
  {
    sql_param_data_count=0;
  }

  otl_cur():
    cda(nullptr),
    errhp(nullptr),
    extern_cda(false),
    status(0),
    eof_status(0),
    db(nullptr),
    straight_select(1),
    pos_nbr(0),
    commit_on_success(0),
    last_param_data_token(0),
    last_sql_param_data_status(0),
    sql_param_data_count(0),
    canceled(false),
    direct_exec_flag(0),
    parse_only_flag(0),
    stm_executed(0)
 {
 }

 virtual ~otl_cur(){}

  void set_direct_exec(const int flag)
  {
    direct_exec_flag=flag;
  }

  void set_parse_only(const int flag)
  {
    parse_only_flag=flag;
  }

  ub4 rpc(void)
  {
    sb4 arpc;
    status=OCIAttrGet
      (OTL_RCAST(dvoid *,cda),
       OTL_SCAST(ub4,OCI_HTYPE_STMT),
       OTL_RCAST(dvoid *,&arpc),
       nullptr,
       OTL_SCAST(ub4,OCI_ATTR_ROW_COUNT),
       errhp);
    if(status)return 0;
    return arpc;
  }

 int open(otl_conn& connect,otl_var* var=nullptr)
 {
  db=&connect;
  commit_on_success=db->get_auto_commit();
  if(var!=nullptr){
   extern_cda=true;
   cda=var->get_cda();
   status=OCI_SUCCESS;
  }else{
#if defined(__GNUC__) && (__GNUC__>=4)
    void* temp_cda=&cda;
#endif
   status=OCIHandleAlloc
     (OTL_RCAST(dvoid *,db->get_envhp()),
#if defined(__GNUC__) && (__GNUC__>=4)
     OTL_RCAST(dvoid **,temp_cda),
#else
     OTL_RCAST(dvoid **,&cda),
#endif
     OCI_HTYPE_STMT,
     0,
     nullptr);
   if(status)return 0;
  }
#if defined(__GNUC__) && (__GNUC__>=4)
  void* temp_errhp=&errhp;
#endif
  status=OCIHandleAlloc
    (OTL_RCAST(dvoid *,db->get_envhp()),
#if defined(__GNUC__) && (__GNUC__>=4)
    OTL_RCAST(dvoid **,temp_errhp),
#else
    OTL_RCAST(dvoid **,&errhp),
#endif
    OCI_HTYPE_ERROR,
    0,
    nullptr);
  if(status)return 0;
  straight_select=1;
  pos_nbr=0;
  return 1;
 }

 int close(void)
 {
  if(!extern_cda)
   status=OCIHandleFree(OTL_RCAST(dvoid*,cda),OCI_HTYPE_STMT);
  status=OCIHandleFree(OTL_RCAST(dvoid*,errhp),OCI_HTYPE_ERROR);
  cda=nullptr;
  errhp=nullptr;
  commit_on_success=0;
  return 1;
 }

 int parse(const char* stm_text)
 {
   status=OCIStmtPrepare
     (cda,
      errhp,
      OTL_RCAST(text*,OTL_CCAST(char*,stm_text)),
      OTL_SCAST(ub4,strlen(stm_text)),
      OTL_SCAST(ub4,OCI_NTV_SYNTAX),
      OTL_SCAST(ub4,OCI_DEFAULT));
   if(status)return 0;

    if(direct_exec_flag && parse_only_flag){
#if !defined(OCI_PARSE_ONLY)
      status=OCIStmtExecute
        (db->svchp,
         cda,
         errhp,
         OTL_SCAST(ub4,1),
         OTL_SCAST(ub4,0),
         0,
         0,
         0x100);
 #else
      status=OCIStmtExecute
        (db->get_svchp(),
         cda,
         errhp,
         OTL_SCAST(ub4,0),
         OTL_SCAST(ub4,0),
         nullptr,
         nullptr,
         OCI_PARSE_ONLY);
 #endif
      if(status)
        return 0;
      else
        return 1;
    }else if(direct_exec_flag && !parse_only_flag){
      ub4 mode;
      if(commit_on_success)
        mode=OCI_COMMIT_ON_SUCCESS;
      else
        mode=OCI_DEFAULT;
     
      status=OCIStmtExecute
        (db->get_svchp(),
         cda,
         errhp,
         OTL_SCAST(ub4,1),
         OTL_SCAST(ub4,0),
         nullptr,
         nullptr,
         mode);
      stm_executed=1;
      if(status)
        return 0;
      else
        return 1;
    }
    return 1;
 }

 int exec(const int iters, 
          const int rowoff,
          const int /*otl_sql_exec_from_class*/)
 {
   if(parse_only_flag){
     parse_only_flag=0;
     return 1;
   }else if(!stm_executed){
     ub4 mode;
     if(commit_on_success)
       mode=OCI_COMMIT_ON_SUCCESS;
     else
       mode=OCI_DEFAULT;
     status=OCIStmtExecute
       (db->get_svchp(),
        cda,
        errhp,
        OTL_SCAST(ub4,iters),
        OTL_SCAST(ub4,rowoff),
        nullptr,
        nullptr,
        mode);
     stm_executed=0;
     if(status!=OCI_SUCCESS)
       return 0;
     return 1;
   }
   return 1;
 }

 long get_rpc() OTL_NO_THROW
 {
  return rpc();
 }

 int fetch(const otl_stream_buffer_size_type iters,int& eof_data)
 {
  eof_data=0;
  status=OCIStmtFetch
   (cda,
    errhp,
    OTL_SCAST(ub4,iters),
    OTL_SCAST(ub4,OCI_FETCH_NEXT),
    OTL_SCAST(ub4,OCI_DEFAULT));
  eof_status=status;
  if(status!=OCI_SUCCESS&&
     status!=OCI_SUCCESS_WITH_INFO&&
     status!=OCI_NO_DATA)
   return 0;
  if(status==OCI_NO_DATA){
   eof_data=1;
   return 1;
  }
  return 1;
 }

 int tmpl_ftype2ora_ftype(const int ftype)
 {
  switch(ftype){
  case otl_var_char:
   return extCChar;
#if (defined(OTL_ORA10G)||defined(OTL_ORA10G_R2))&&defined(OTL_ORA_NATIVE_TYPES)\
    &&!defined(OTL_ORA_LEGACY_NUMERIC_TYPES)
  case otl_var_double:
    return extBDouble;
  case otl_var_bdouble:
    return extBDouble;
  case otl_var_float:
    return extBFloat;
  case otl_var_bfloat:
    return extBFloat;
#else
  case otl_var_double:
   return extFloat;
  case otl_var_float:
   return extFloat;
#endif
  case otl_var_int:
   return extInt;
  case otl_var_unsigned_int:
   return extUInt;
  case otl_var_short:
   return extInt;
  case otl_var_long_int:
   return extInt;
#if defined(OTL_BIGINT) && (defined(OTL_ORA11G_R2)&&!defined(OTL_STR_TO_BIGINT)&&\
    !defined(OTL_BIGINT_TO_STR))
  case otl_var_bigint:
   return extInt;
#endif
#if defined(OTL_UBIGINT) && defined(OTL_ORA11G_R2)
  case otl_var_ubigint:
   return extUInt;
#endif
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  case otl_var_timestamp:
   return extTimestamp;
  case otl_var_tz_timestamp:
   return extTimestamp_TZ;
  case otl_var_ltz_timestamp:
   return extTimestamp_LTZ;
#else
  case otl_var_timestamp:
   return extDate;
#endif
 case otl_var_varchar_long:
   return extLongVarChar;
  case otl_var_raw_long:
   return extLongVarRaw;
  case otl_var_raw:
   return extRaw;
  case otl_var_clob:
   return SQLT_CLOB;
  case otl_var_blob:
   return SQLT_BLOB;
  default:
   return 0;
  }
 }

 int bind
 (const char* name,
  otl_var& v,
  const int elem_size,
  const int ftype,
  const int /*param_type*/,
  const int /*name_pos*/,
  const int /*connection_type*/,
  const int apl_tab_flag)
 {OCIBind* bindpp;

  int db_ftype=0;
    
   if(ftype==otl_var_refcur){
    status=OCIBindByName
     (cda,
      &bindpp,
      errhp,
      OTL_RCAST(text*,OTL_CCAST(char*,name)),
      OTL_SCAST(sb4,strlen(name)),
      OTL_RCAST(dvoid*,v.get_cda_ptr()),
      0,
      SQLT_RSET,
      nullptr,
      nullptr,
      nullptr,
      0,
      nullptr,
      OTL_SCAST(ub4,OCI_DEFAULT));
   }else if(ftype!=otl_var_clob&&ftype!=otl_var_blob){
     int var_elem_size;
#if defined(OTL_UNICODE)
     if(ftype==otl_var_char){
       var_elem_size=elem_size*sizeof(OTL_WCHAR); // ###
     }
     else if(ftype==otl_var_varchar_long)
      var_elem_size=elem_size;
     else
      var_elem_size=elem_size;
#else
     if(ftype==otl_var_varchar_long)
       var_elem_size=elem_size+sizeof(sb4);
     else
       var_elem_size=elem_size;
#endif
     db_ftype=tmpl_ftype2ora_ftype(ftype);
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
     if(ftype==otl_var_timestamp || 
        ftype==otl_var_tz_timestamp ||
        ftype==otl_var_ltz_timestamp){
       if(!apl_tab_flag)
         var_elem_size=sizeof(OCIDateTime*);
       else if(db_ftype==extTimestamp)
         db_ftype=extDate;
     }
#endif
#if defined(OTL_UNICODE)
     if(ftype==otl_var_char)
       db_ftype=SQLT_VCS;
#endif
     if(apl_tab_flag){
       if(ftype==otl_var_float||ftype==otl_var_double)
         db_ftype=extFloat;
       status=OCIBindByName
         (cda,
          &bindpp,
          errhp,
          OTL_RCAST(text*,OTL_CCAST(char*,name)),
          OTL_SCAST(sb4,strlen(name)),
          OTL_RCAST(dvoid*,v.p_v),
          ftype==otl_var_raw?var_elem_size+sizeof(short):var_elem_size,
          OTL_SCAST(ub2,v.charz_flag?extCharZ:db_ftype),
          OTL_RCAST(dvoid*,v.p_ind),
          nullptr,
          nullptr,
          OTL_SCAST(ub4,v.max_tab_len),
          OTL_RCAST(ub4*,&v.cur_tab_len),
          OTL_SCAST(ub4,OCI_DEFAULT));
     }else{
       status=OCIBindByName
         (cda,
          &bindpp,
          errhp,
          OTL_RCAST(text*,OTL_CCAST(char*,name)),
          OTL_SCAST(sb4,strlen(name)),
          OTL_RCAST(dvoid*,v.p_v),
          ftype==otl_var_raw?var_elem_size+sizeof(short):var_elem_size,
          OTL_SCAST(ub2,db_ftype),
          OTL_RCAST(dvoid*,v.p_ind),
          nullptr,
          nullptr,
          0,
          nullptr,
          OTL_SCAST(ub4,OCI_DEFAULT));
     }
    if(status)return 0;
#if defined(OTL_UNICODE)
    if(ftype==otl_var_char||ftype==otl_var_varchar_long){
      if(ftype!=otl_var_varchar_long){
        if(v.nls_flag)
          v.csfrm=OTL_SCAST(ub1,SQLCS_NCHAR);
        else
          v.csfrm=OTL_SCAST(ub1,db->char_set_);
        status=OCIAttrSet
          (bindpp, 
           OCI_HTYPE_BIND, 
           &v.csfrm, 
           0, 
           OTL_SCAST(ub4,OCI_ATTR_CHARSET_FORM),
           errhp);
        if(status)return 0;
      }
      v.csid=OTL_UNICODE_ID;
      status=OCIAttrSet
        (bindpp, 
         OCI_HTYPE_BIND, 
         &v.csid, 
         0, 
         OCI_ATTR_CHARSET_ID,
         errhp);
      if(status)return 0;
      if(ftype==otl_var_varchar_long)
        v.unicode_var_len=elem_size-sizeof(sb4);
      else{
#if defined(OTL_ORA_MAX_UNICODE_VARCHAR_SIZE)
        if(var_elem_size>OTL_ORA_MAX_UNICODE_VARCHAR_SIZE)
          v.unicode_var_len=OTL_ORA_MAX_UNICODE_VARCHAR_SIZE;
        else
          v.unicode_var_len=var_elem_size;
#else
        v.unicode_var_len=var_elem_size;
#endif
      }
      status=OCIAttrSet
        (bindpp, 
         OCI_HTYPE_BIND, 
         &v.unicode_var_len, 
         0, 
         OCI_ATTR_MAXDATA_SIZE, 
         errhp);
      if(status)return 0;
    }
#endif

#if defined(OTL_ORA_UTF8)
    if(ftype==otl_var_char){
      if(v.nls_flag)
        v.csfrm=OTL_SCAST(ub1,SQLCS_NCHAR);
      else
        v.csfrm=OTL_SCAST(ub1,db->char_set_);
      status=OCIAttrSet
        (bindpp, 
         OCI_HTYPE_BIND, 
         &v.csfrm, 
         0, 
         OTL_SCAST(ub4,OCI_ATTR_CHARSET_FORM),
         errhp);
      if(status)return 0;
    }
#endif

    return 1;
   }else{
    status=OCIBindByName
     (cda,
      &bindpp,
      errhp,
      OTL_RCAST(text*,OTL_CCAST(char*,name)),
      OTL_SCAST(sb4,strlen(name)),
      OTL_RCAST(dvoid*,v.p_v),
      OTL_SCAST(sb4,-1),
      OTL_SCAST(ub2,tmpl_ftype2ora_ftype(ftype)),
      OTL_RCAST(dvoid*,v.p_ind),
      nullptr,
      nullptr,
      0,
      nullptr,
     OTL_SCAST(ub4,OCI_DEFAULT));
    if(status)return 0;
#if defined(OTL_UNICODE)
    if(ftype==otl_var_clob){
      v.csid=OTL_UNICODE_ID;
      status=OCIAttrSet
        (bindpp, 
         OCI_HTYPE_BIND, 
         &v.csid, 
         0, 
         OCI_ATTR_CHARSET_ID,
         errhp);
      if(status)return 0;
      if(v.nls_flag)
        v.csfrm=OTL_SCAST(ub1,SQLCS_NCHAR);
      else
        v.csfrm=OTL_SCAST(ub1,db->char_set_);
      status=OCIAttrSet
        (bindpp, 
         OCI_HTYPE_BIND, 
         &v.csfrm, 
         0, 
         OTL_SCAST(ub4,OCI_ATTR_CHARSET_FORM),
         errhp);
      if(status)return 0;
    }
#endif

#if defined(OTL_ORA_UTF8)
    if(ftype==otl_var_clob){
      if(v.nls_flag)
        v.csfrm=OTL_SCAST(ub1,SQLCS_NCHAR);
      else
        v.csfrm=OTL_SCAST(ub1,db->char_set_);
      status=OCIAttrSet
        (bindpp, 
         OCI_HTYPE_BIND, 
         &v.csfrm, 
         0, 
         OTL_SCAST(ub4,OCI_ATTR_CHARSET_FORM),
         errhp);
      if(status)return 0;
    }
#endif
    return 1;
   }
  if(status)return 0;
  return 1;

 }

 int bind
 (const int column_num,
  otl_var& v,
  const int elem_size,
  const int ftype,
  const int /*param_type*/)
 {OCIDefine *defnp;
  int db_ftype=0;

  if(ftype!=otl_var_clob&&ftype!=otl_var_blob){
    int var_elem_size;
#if defined(OTL_UNICODE)
    if(ftype==otl_var_char)
     var_elem_size=elem_size*sizeof(OTL_WCHAR);
    else if(ftype==otl_var_varchar_long)
     var_elem_size=elem_size+sizeof(sb4);
    else
     var_elem_size=elem_size;
#elif defined(OTL_ORA_UTF8)
    if(ftype==otl_var_char && v.select_stm_flag)
      var_elem_size=elem_size*OTL_UTF8_BYTES_PER_CHAR; // 3 bytes per UTF8 char
    else if(ftype==otl_var_varchar_long)
     var_elem_size=elem_size+sizeof(sb4);
    else
     var_elem_size=elem_size;
#else
    if(ftype==otl_var_varchar_long)
      var_elem_size=elem_size+sizeof(sb4);
    else
      var_elem_size=elem_size;
#endif
    db_ftype=tmpl_ftype2ora_ftype(ftype);
#if defined(OTL_UNICODE)
     if(ftype==otl_var_char)
       db_ftype=SQLT_VCS;
#endif
   status=OCIDefineByPos
    (cda,
     &defnp,
     errhp,
     OTL_SCAST(ub4,column_num),
     OTL_RCAST(dvoid*,v.p_v),
     OTL_SCAST(sb4,ftype==otl_var_raw?var_elem_size+sizeof(short):var_elem_size),
     OTL_SCAST(ub2,db_ftype),
     OTL_RCAST(dvoid*,v.p_ind),
     OTL_RCAST(ub2*,v.p_rlen),
     OTL_RCAST(ub2*,v.p_rcode),
     OCI_DEFAULT);
   if(status)return 0;

#if defined(OTL_ORA_UTF8)
   if(ftype==otl_var_char||ftype==otl_var_varchar_long){
     if(v.nls_flag)
       v.csfrm=OTL_SCAST(ub1,SQLCS_NCHAR);
     else
       v.csfrm=OTL_SCAST(ub1,db->char_set_);
     status=OCIAttrSet
       (defnp, 
        OCI_HTYPE_DEFINE, 
        OTL_RCAST(void*,&v.csfrm), 
        OTL_SCAST(ub4,0),
        OTL_SCAST(ub4,OCI_ATTR_CHARSET_FORM), 
        errhp); 
     if(status)return 0;
   }
#endif

#if defined(OTL_UNICODE)
   if(ftype==otl_var_char||ftype==otl_var_varchar_long){
     if(v.nls_flag)
       v.csfrm=OTL_SCAST(ub1,SQLCS_NCHAR);
     else
       v.csfrm=OTL_SCAST(ub1,db->char_set_);
     status=OCIAttrSet
       (defnp, 
        OCI_HTYPE_DEFINE, 
        OTL_RCAST(void*,&v.csfrm), 
        OTL_SCAST(ub4,0),
        OTL_SCAST(ub4,OCI_ATTR_CHARSET_FORM), 
        errhp); 
     if(status)return 0;
     v.csid=OTL_UNICODE_ID;
     status=OCIAttrSet
       (defnp, 
        OCI_HTYPE_DEFINE, 
        &v.csid, 
        0, 
        OCI_ATTR_CHARSET_ID, 
        errhp);
     if(status)return 0;
   }
#endif
   return 1;
  }else{
   status=OCIDefineByPos
    (cda,
     &defnp,
     errhp,
     OTL_SCAST(ub4,column_num),
     OTL_RCAST(dvoid*,v.p_v),
     OTL_SCAST(sb4,-1),
     OTL_SCAST(ub2,tmpl_ftype2ora_ftype(ftype)),
     OTL_RCAST(dvoid*,v.p_ind),
     OTL_RCAST(ub2*,v.p_rlen),
     OTL_RCAST(ub2*,v.p_rcode),
     OCI_DEFAULT);
   if(status)return 0;
#if defined(OTL_UNICODE)
   if(ftype==otl_var_char||ftype==otl_var_varchar_long){
     v.csid=OTL_UNICODE_ID;
     status=OCIAttrSet
       (defnp, 
        OCI_HTYPE_DEFINE, 
        &v.csid, 
        0, 
        OCI_ATTR_CHARSET_ID, 
        errhp);
     if(status)return 0;
   }
#endif
#if defined(OTL_ORA_UTF8)
   if(ftype==otl_var_clob){
     if(v.nls_flag)
       v.csfrm=OTL_SCAST(ub1,SQLCS_NCHAR);
     else
       v.csfrm=OTL_SCAST(ub1,db->char_set_);
     status=OCIAttrSet
       (defnp, 
        OCI_HTYPE_DEFINE, 
        OTL_RCAST(void*,&v.csfrm), 
        OTL_SCAST(ub4,0),
        OTL_SCAST(ub4,OCI_ATTR_CHARSET_FORM), 
        errhp); 
     if(status)return 0;
   }
#endif

   return 1;
  }
 }

 void set_select_type(const int select_type)
 {
  straight_select=select_type;
 }

 int describe_column
 (otl_column_desc& col,
  const int column_num,
  int& eof_desc)
 {
  OCIParam* pard;
  ub2 dtype;
  ub2 dbsize;
  sb2 prec;

#if defined(OTL_ORA8_8I_DESC_COLUMN_SCALE)
  ub1 scale;
#else
  sb2 scale;
#endif

  ub1 nullok;
  text* col_name;
  ub4 col_name_len;
  ub4 pos_num;

  eof_desc=0;
  if(straight_select&&pos_nbr==0){
   status=OCIStmtExecute
     (db->get_svchp(),
     cda,
     errhp,
     0,
     0,
     nullptr,
     nullptr,
     OCI_DESCRIBE_ONLY);
   if(status!=OCI_SUCCESS)return 0;
   status=OCIAttrGet
    (cda,
     OCI_HTYPE_STMT,
     OTL_RCAST(ub4*,&pos_num),
     nullptr,
     OTL_SCAST(ub4,OCI_ATTR_PARAM_COUNT),
     errhp);
   if(status!=OCI_SUCCESS)return 0;
   pos_nbr=OTL_SCAST(int,pos_num);
  }
  if(!straight_select&&pos_nbr==0){
   status=OCIAttrGet
    (cda,
     OCI_HTYPE_STMT,
     OTL_RCAST(ub4*,&pos_num),
     nullptr,
     OTL_SCAST(ub4,OCI_ATTR_PARAM_COUNT),
     errhp);
   if(status!=OCI_SUCCESS)return 0;
   pos_nbr=OTL_SCAST(int,pos_num);
  }
  if(column_num<1||column_num>pos_nbr){
   eof_desc=1;
   return 0;
  }
#if defined(__GNUC__) && (__GNUC__>=4)
  void* temp_pard=&pard;
#endif
  status=OCIParamGet
   (cda,
    OCI_HTYPE_STMT,
    errhp,
#if defined(__GNUC__) && (__GNUC__>=4)
    OTL_RCAST(void**,temp_pard),
#else
    OTL_RCAST(void**,&pard),
#endif
    OTL_SCAST(ub4,column_num));
  if(status!=OCI_SUCCESS&&status!=OCI_NO_DATA)
   return 0;
  if(status==OCI_NO_DATA){
   eof_desc=1;
   return 1;
  }
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid*,&dtype),
    nullptr,
    OTL_SCAST(ub4,OCI_ATTR_DATA_TYPE),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
#if !defined(OTL_ORA8I)
  ub1 charset_form;
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid*,&charset_form),
    nullptr,
    OTL_SCAST(ub4,OCI_ATTR_CHARSET_FORM),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  col.charset_form=OTL_SCAST(int,charset_form);
  ub2 char_size;
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid*,&char_size),
    nullptr,
    OTL_SCAST(ub4,OCI_ATTR_CHAR_SIZE),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  col.char_size=OTL_SCAST(int,char_size);
#else
  col.char_size=0;
#endif
#endif
  col.dbtype=dtype;
#if defined(__GNUC__) && (__GNUC__>=4)
  void* temp_col_name=&col_name;
#endif
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
#if defined(__GNUC__) && (__GNUC__>=4)
    OTL_RCAST(dvoid**,temp_col_name),
#else
    OTL_RCAST(dvoid**,&col_name),
#endif
    OTL_RCAST(ub4*,&col_name_len),
    OTL_SCAST(ub4,OCI_ATTR_NAME),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  col.set_name(OTL_RCAST(char*,col_name),col_name_len);
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid*,&dbsize),
#if defined(OTL_ANSI_CPP_11_NULLPTR_SUPPORT)
    nullptr,
#else
    OTL_RCAST(ub4*,0),
#endif
    OTL_SCAST(ub4,OCI_ATTR_DATA_SIZE),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  col.dbsize=dbsize;
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid*,&prec),
    nullptr,
    OTL_SCAST(ub4,OCI_ATTR_PRECISION),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  col.prec=prec;
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid*,&scale),
    nullptr,
    OTL_SCAST(ub4,OCI_ATTR_SCALE),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  col.scale=scale;
  status=OCIAttrGet
   (OTL_RCAST(dvoid*,pard),
    OTL_SCAST(ub4,OCI_DTYPE_PARAM),
    OTL_RCAST(dvoid*,&nullok),
    nullptr,
    OTL_SCAST(ub4,OCI_ATTR_IS_NULL),
    OTL_RCAST(OCIError*,errhp));
  if(status!=OCI_SUCCESS)return 0;
  col.nullok=nullok;
  return 1;
 }

 void error(otl_exc& exception_struct)
 {sb4 errcode;
  size_t len;

  OTL_STRCPY_S(OTL_RCAST(char*,exception_struct.msg),
               sizeof(exception_struct.msg),
               "123456789");
  OCIErrorGet
   (OTL_RCAST(dvoid*,errhp),
    OTL_SCAST(ub4,1),
    nullptr,
    &errcode,
    OTL_RCAST(text*,exception_struct.msg),
    OTL_SCAST(ub4,sizeof(exception_struct.msg)),
    OCI_HTYPE_ERROR);
  exception_struct.code=errcode;
  len=strlen(OTL_RCAST(char*,exception_struct.msg));
  exception_struct.msg[len]=0;
#if defined(OTL_EXCEPTION_ENABLE_ERROR_OFFSET)
  ub2 error_offset;
  if(OCIAttrGet
     (cda,
      OCI_HTYPE_STMT,
      OTL_RCAST(ub2*,&error_offset),
      0,
      OTL_SCAST(ub4,OCI_ATTR_PARSE_ERROR_OFFSET),
      errhp)==OCI_SUCCESS)
    exception_struct.error_offset=OTL_SCAST(int,error_offset);
#endif
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_cur(const otl_cur&) = delete;
  otl_cur& operator=(const otl_cur&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_cur(otl_cur&&) = delete;
  otl_cur& operator=(otl_cur&&) = delete;
#endif
private:
#else

  otl_cur(const otl_cur&):
    otl_cur0(),
    cda(nullptr),
    errhp(nullptr),
    extern_cda(false),
    status(0),
    eof_status(0),
    db(nullptr),
    straight_select(1),
    pos_nbr(0),
    commit_on_success(0),
    last_param_data_token(0),
    last_sql_param_data_status(0),
    sql_param_data_count(0),
    canceled(false),
    direct_exec_flag(0),
    parse_only_flag(0),
    stm_executed(0)
 {
 }

  otl_cur& operator=(const otl_cur&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_cur(otl_cur&&):
    otl_cur0(),
    cda(nullptr),
    errhp(nullptr),
    extern_cda(false),
    status(0),
    eof_status(0),
    db(nullptr),
    straight_select(1),
    pos_nbr(0),
    commit_on_success(0),
    last_param_data_token(0),
    last_sql_param_data_status(0),
    sql_param_data_count(0),
    canceled(false),
    direct_exec_flag(0),
    parse_only_flag(0),
    stm_executed(0)
 {
 }

  otl_cur& operator=(otl_cur&&)
  {
    return *this;
  }
#endif
#endif

};


class otl_ref_cursor;

class otl_sel{
private:

  friend class otl_ref_cursor;
  int implicit_cursor;

public:

  int get_implicit_cursor() const {return implicit_cursor;}

 void set_arr_size
 (const int input_arr_size,
  int& out_array_size,
  int& out_prefetch_array_size)
 {
   out_array_size=input_arr_size;
   out_prefetch_array_size=0;
 }

  void set_prefetch_size(const int /*aprefetch_array_size*/)
  {
  }

 int close_select(otl_cur& /*cur*/)
 {
  int i=1;
  return i;
 }

  otl_sel():
    implicit_cursor(0)
 {
 }

 virtual ~otl_sel(){}

  void set_select_type(const int /*atype*/)
 {
  implicit_cursor=0;
 }

  void init(const int /*array_size*/){}

 int first
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
  const int array_size)
 {int rc;
  eof_data=0;
  cur_row=-1;
  cur.commit_on_success=0;
  rc=cur.exec(0,0,otl_sql_exec_from_select_cursor_class);
  if(rc==0)return 0;
  rc=cur.fetch(OTL_SCAST(otl_stream_buffer_size_type,array_size),eof_data);
  if(rc==0)return 0;
  row_count=cur.rpc();
  cur_size=row_count;
  if(cur_size!=0)cur_row=0;
  return 1;
 }

 int next
 (otl_cur& cur,
  int& cur_row,
  int& cur_size,
  int& row_count,
  int& eof_data,
  const int array_size)
 {int rc;
  if(cur_row<cur_size-1){
   ++cur_row;
   return 1;
  }else{
   if(eof_data){
    cur_row=-1;
    cur_size=0;
    return 1;
   }
   cur.commit_on_success=0;
   rc=cur.fetch(OTL_SCAST(otl_stream_buffer_size_type,array_size),eof_data);
   if(rc==0)return 0;
   int temp_rpc=cur.rpc();
   cur_size=temp_rpc-row_count;
   row_count=temp_rpc;
   if(cur_size!=0)cur_row=0;
   return 1;
  }
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_sel(const otl_sel&) = delete;
  otl_sel& operator=(const otl_sel&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_sel(otl_sel&&) = delete;
  otl_sel& operator=(otl_sel&&) = delete;
#endif
private:
#else
  otl_sel(const otl_sel&):
    implicit_cursor(0)
 {
 }

  otl_sel& operator=(const otl_sel&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_sel(otl_sel&&):
    implicit_cursor(0)
 {
 }

  otl_sel& operator=(otl_sel&&)
  {
    return *this;
  }
#endif
#endif

};


typedef otl_tmpl_connect
  <otl_exc,
   otl_conn,
   otl_cur> otl_ora8_connect;


typedef otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> otl_cursor;

template <OTL_TYPE_NAME TExceptionStruct,
          OTL_TYPE_NAME TConnectStruct,
          OTL_TYPE_NAME TCursorStruct,
          OTL_TYPE_NAME TVariableStruct>
class otl_tmpl_lob_stream: public otl_lob_stream_generic{
public:

  typedef otl_tmpl_exception
  <TExceptionStruct,
   TConnectStruct,
   TCursorStruct> otl_exception;

 typedef otl_tmpl_variable<TVariableStruct>* p_bind_var;
 typedef otl_tmpl_connect
         <TExceptionStruct,
         TConnectStruct,
         TCursorStruct>* p_connect;

 typedef otl_tmpl_cursor
         <TExceptionStruct,
          TConnectStruct,
          TCursorStruct,
          TVariableStruct>* p_cursor;

private:

 p_bind_var bind_var;
 p_connect connect;
 p_cursor cursor;
 otl_long_string* temp_buf;
 char* temp_char_buf;

public:

  void setInitialReadOffset(const int initial_offset)
  {
    if(lob_len==0)lob_len=len();
    if((initial_offset-1)>=lob_len){
      eof_flag=1;
      return;
    }
    offset=initial_offset+1;
    if(bind_var)
      bind_var->get_var_struct().set_total_read_blob_amt
        (initial_offset+1);
  }

 void init
 (void* avar,void* aconnect,void* acursor,
  int andx,
  int amode,
  const int alob_is_null=0) OTL_NO_THROW
 {
  connect=OTL_RCAST(p_connect,aconnect);
  bind_var=OTL_RCAST(p_bind_var,avar);
  cursor=OTL_RCAST(p_cursor,acursor);
  mode=amode;
  retcode=0;
  lob_is_null=alob_is_null;
  ndx=andx;
  offset=0;
  if(amode==otl_lob_stream_write_mode)
    lob_len=2147483647;
  else
    lob_len=0;
  eof_flag=0;
  in_destructor=0;
  if(bind_var)
    bind_var->get_var_struct().set_lob_stream_flag();
 }

 void set_len(const int new_len=0) OTL_NO_THROW
 {
  lob_len=new_len;
 }

  otl_tmpl_lob_stream() OTL_NO_THROW:
  otl_lob_stream_generic(true),
   bind_var(nullptr),
   connect(nullptr),
   cursor(nullptr),
   temp_buf(nullptr),
   temp_char_buf(nullptr)
  {
    init(nullptr,nullptr,nullptr,0,otl_lob_stream_zero_mode);
  }

  virtual ~otl_tmpl_lob_stream()
 {
   in_destructor=1;
   if(temp_buf){
     delete temp_buf;
     temp_buf=nullptr;
   }
   if(temp_char_buf){
     delete[] temp_char_buf;
     temp_char_buf=nullptr;
   }
#if defined(OTL_DESTRUCTORS_DO_NOT_THROW)
   try{
     close();
   }catch(OTL_CONST_EXCEPTION otl_exception&){
   }
#else
   close();
#endif
 }

#if (defined(OTL_STL) || defined(OTL_ACE) || \
     defined(OTL_USER_DEFINED_STRING_CLASS_ON)) && !defined(OTL_UNICODE)
  otl_lob_stream_generic& operator<<(const OTL_STRING_CONTAINER& s)
    OTL_THROWS_OTL_EXCEPTION
  {
    otl_long_string temp_s(s.c_str(),                  
                           OTL_SCAST(int,s.length()),
                           OTL_SCAST(int,s.length()));
    (*this)<<temp_s;
    return *this;
  }

  void setStringBuffer(const int chunk_size)
  {
    delete[] temp_char_buf;
    temp_char_buf=nullptr;
    delete temp_buf;
    temp_buf=nullptr;
    temp_char_buf=new char[chunk_size+1];
    temp_buf=new otl_long_string(temp_char_buf,chunk_size);
  }

  otl_lob_stream_generic& operator>>(OTL_STRING_CONTAINER& s)
    OTL_THROWS_OTL_EXCEPTION
  {
    const int TEMP_BUF_SIZE=4096;
    if(!temp_char_buf)temp_char_buf=new char[TEMP_BUF_SIZE];
    if(!temp_buf)temp_buf=new otl_long_string(temp_char_buf,TEMP_BUF_SIZE-1);
    int iters=0;
    while(!this->eof()){
      ++iters;
      (*this)>>(*temp_buf);
      temp_char_buf[temp_buf->len()]=0;
      if(iters>1)
        s.append(temp_char_buf,OTL_SCAST(size_t,temp_buf->len()));
      else
#if (defined(OTL_USER_DEFINED_STRING_CLASS_ON) || defined(OTL_STL)) \
     && !defined(OTL_ACE)
        s.assign(temp_char_buf,OTL_SCAST(size_t,temp_buf->len()));
#elif defined(OTL_ACE)
      s.set(temp_char_buf,OTL_SCAST(size_t,temp_buf->len()),1);
#endif
    }
    return *this;
  }
#endif


 otl_lob_stream_generic& operator<<(const otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   bool in_unicode_mode=sizeof(OTL_CHAR)>1;
   if(bind_var && bind_var->get_ftype()==otl_var_blob)
     in_unicode_mode=false;
   if(s.get_unicode_flag() != in_unicode_mode){
     throw OTL_TMPL_EXCEPTION
       (otl_error_msg_37,
        otl_error_code_37,
        "otl_lob_stream_generic::operator<<(const otl_long_string&)"
       );
   }
   if(mode!=otl_lob_stream_write_mode){
     const char* stm=nullptr;
     char var_info[256];
     var_info[0]=0;
     if(cursor!=nullptr){
       if(cursor->get_stm_label())
         stm=cursor->get_stm_label();
       else
         stm=cursor->get_stm_text();
     }
     if(bind_var!=nullptr){
       otl_var_info_var
         (bind_var->get_name(),
          bind_var->get_ftype(),
          otl_var_long_string,
          var_info,
          sizeof(var_info));
     }
     char* vinfo=nullptr;
     if(var_info[0]!=0)
       vinfo=&var_info[0];
     if(this->connect)this->connect->increment_throw_count();
     if(this->connect&&this->connect->get_throw_count()>1)return *this;
     if(otl_uncaught_exception()) return *this; 
     throw otl_tmpl_exception
       <TExceptionStruct,
       TConnectStruct,
       TCursorStruct>
       (otl_error_msg_9,
        otl_error_code_9,
        stm,
        vinfo);
   }
   if(offset==0)offset=1; 
   if((offset-1)+s.len()>lob_len){
     char var_info[256];
     if(bind_var!=nullptr)
       otl_var_info_var
         (bind_var->get_name(),
          bind_var->get_ftype(),
          otl_var_long_string,
          var_info,
          sizeof(var_info));
     if(this->connect)this->connect->increment_throw_count();
     if(this->connect&&this->connect->get_throw_count()>1)return *this;
     if(otl_uncaught_exception()) return *this; 
     char err_msg[1024];
     char temp_num[64];
     OTL_STRCPY_S(err_msg,sizeof(err_msg),otl_error_msg_7);
     OTL_STRCAT_S(err_msg,sizeof(err_msg),", trying to store ");
     otl_itoa(s.len(),temp_num);
     OTL_STRCAT_S(err_msg,sizeof(err_msg),temp_num);
#if defined(OTL_UNICODE)
     OTL_STRCAT_S(err_msg,sizeof(err_msg)," Unicode characters at offset ");
#else
     OTL_STRCAT_S(err_msg,sizeof(err_msg)," bytes at offset ");
#endif
     otl_itoa(offset,temp_num);
     OTL_STRCAT_S(err_msg,sizeof(err_msg),temp_num);
     OTL_STRCAT_S(err_msg,sizeof(err_msg),". New length: ");
     otl_itoa((offset-1)+s.len(),temp_num);
     OTL_STRCAT_S(err_msg,sizeof(err_msg),temp_num);
     OTL_STRCAT_S(err_msg,sizeof(err_msg)," would be bigger than length of lob: ");
     otl_itoa(lob_len,temp_num);
     OTL_STRCAT_S(err_msg,sizeof(err_msg),temp_num);
     throw otl_tmpl_exception
       <TExceptionStruct,
       TConnectStruct,
       TCursorStruct>
       (err_msg,
        otl_error_code_7,
        cursor->get_stm_label()?cursor->get_stm_label():
        cursor->get_stm_text(),
        var_info); 
   }
   if(s.is_last_piece())
     lob_len=(offset+s.len()-1);
   if(bind_var!=nullptr)
     retcode=bind_var->get_var_struct().write_blob
       (s,lob_len,offset,cursor->get_cursor_struct());
   if(retcode){
     if((offset-1)==lob_len)
       close();
     return *this;
   }
   if(this->connect)this->connect->increment_throw_count();
   if(this->connect&&this->connect->get_throw_count()>1)return *this;
   if(otl_uncaught_exception()) return *this; 
   throw otl_tmpl_exception
     <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect->get_connect_struct(),
                    cursor->get_stm_label()?cursor->get_stm_label():
                    cursor->get_stm_text());
 }

 otl_lob_stream_generic& operator>>(otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   bool in_unicode_mode=sizeof(OTL_CHAR)>1;
   if(bind_var && bind_var->get_ftype()==otl_var_blob)
     in_unicode_mode=false;
   if(s.get_unicode_flag() != in_unicode_mode){
     throw OTL_TMPL_EXCEPTION
       (otl_error_msg_37,
        otl_error_code_37,
        "otl_lob_stream_generic::operator>>(otl_long_string&)"
       );
   }
   if(mode!=otl_lob_stream_read_mode){
     const char* stm=nullptr;
     char var_info[256];
     var_info[0]=0;
     if(cursor!=nullptr){
       if(cursor->get_stm_label())
         stm=cursor->get_stm_label();
       else
       stm=cursor->get_stm_text();
     }
     if(bind_var!=nullptr){
       otl_var_info_var
      (bind_var->get_name(),
       bind_var->get_ftype(),
       otl_var_long_string,
       var_info,
       sizeof(var_info));
     }
     char* vinfo=nullptr;
     if(var_info[0]!=0)
       vinfo=&var_info[0];
     if(this->connect)this->connect->increment_throw_count();
     if(this->connect&&this->connect->get_throw_count()>1)return *this;
     if(otl_uncaught_exception()) return *this; 
     throw otl_tmpl_exception
       <TExceptionStruct,
       TConnectStruct,
       TCursorStruct>
       (otl_error_msg_10,
        otl_error_code_10,
        stm,
        vinfo);
   }
   if(offset==0&&lob_len==0)
     lob_len=len();
   if(lob_len==0||(offset-1)==lob_len){
     s.set_len(0);
     eof_flag=1;
     return *this;
   }
   if(offset==0)offset=1;
   if(bind_var!=nullptr)
     retcode=bind_var->get_var_struct().read_blob(s,ndx,offset,lob_len);
   if((offset-1)==lob_len)eof_flag=1;
   if(retcode){
     if(eof()){
       close();
       eof_flag=1;
     }
     return *this;
   }
   if(this->connect)this->connect->increment_throw_count();
   if(this->connect&&this->connect->get_throw_count()>1)return *this;
   if(otl_uncaught_exception()) return *this; 
   throw otl_tmpl_exception
     <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect->get_connect_struct(),
                    cursor->get_stm_label()?cursor->get_stm_label():
                    cursor->get_stm_text());
 }
  
 int eof(void) OTL_NO_THROW
 {
  if(lob_is_null)return 1;
  return eof_flag;
 }

 bool is_initialized(void) OTL_THROWS_OTL_EXCEPTION
 {
  if(cursor==nullptr||connect==nullptr||
     bind_var==nullptr||lob_is_null)return false;
  int is_init=0;
  retcode=bind_var->get_var_struct().is_blob_initialized(ndx,is_init);
  if(retcode) return is_init!=0;
  if(this->connect)this->connect->increment_throw_count();
  if(this->connect&&this->connect->get_throw_count()>1)return false;
  if(otl_uncaught_exception()) return false; 
  throw OTL_TMPL_EXCEPTION
    (connect->get_connect_struct(),
     cursor->get_stm_label()?cursor->get_stm_label():
     cursor->get_stm_text());
 }

 int len(void) OTL_THROWS_OTL_EXCEPTION
 {
  if(cursor==nullptr||connect==nullptr||
     bind_var==nullptr||lob_is_null)return 0;
  int alen;
  retcode=bind_var->get_var_struct().get_blob_len(ndx,alen);
  if(retcode)return alen;
  if(this->connect)this->connect->increment_throw_count();
  if(this->connect&&this->connect->get_throw_count()>1)return 0;
  if(otl_uncaught_exception()) return 0; 
  throw otl_tmpl_exception
    <TExceptionStruct,
     TConnectStruct,
     TCursorStruct>(connect->get_connect_struct(),
                    cursor->get_stm_label()?cursor->get_stm_label():
                    cursor->get_stm_text());
 }

 void close(bool dont_throw_size_doesnt_match_exception=false) OTL_THROWS_OTL_EXCEPTION
 {
  if(in_destructor){
   if(mode==otl_lob_stream_read_mode){
     bind_var->get_var_struct().set_lob_stream_flag(0);
    bind_var->set_not_null(0);
   }
   return;
  }
  if(mode==otl_lob_stream_zero_mode)return;
  if(mode==otl_lob_stream_read_mode){
    if(offset<lob_len-1)
      bind_var->get_var_struct().close_lob();
    bind_var->get_var_struct().close_temporary_lob();
    bind_var->get_var_struct().set_lob_stream_flag(0);
    bind_var->set_not_null(0);
    init(nullptr,nullptr,nullptr,0,otl_lob_stream_zero_mode);
  }else{
   // write mode
   if(!(offset==0&&lob_len==0)&&
      (offset-1)!=lob_len&&
      !dont_throw_size_doesnt_match_exception){
     bind_var->get_var_struct().close_lob();     
     char var_info[256];
     char msg_buf[1024];
     OTL_STRCPY_S(msg_buf,sizeof(msg_buf),otl_error_msg_8);
     otl_var_info_var
       (bind_var->get_name(),
        bind_var->get_ftype(),
        otl_var_long_string,
        var_info,
        sizeof(var_info));
     if(this->connect)this->connect->increment_throw_count();
     if(this->connect&&this->connect->get_throw_count()>1)return;
     if(otl_uncaught_exception()) return; 
     throw otl_tmpl_exception
       <TExceptionStruct,
       TConnectStruct,
       TCursorStruct>
       (msg_buf,
        otl_error_code_8,
        cursor->get_stm_label()?cursor->get_stm_label():
        cursor->get_stm_text(),
        var_info);
   }
   bind_var->get_var_struct().set_lob_stream_flag(0);
   bind_var->set_not_null(0);
  }
 }
private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_tmpl_lob_stream(const otl_tmpl_lob_stream&) = delete;
  otl_tmpl_lob_stream& operator=(const otl_tmpl_lob_stream&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_tmpl_lob_stream(otl_tmpl_lob_stream&&) = delete;
  otl_tmpl_lob_stream& operator=(otl_tmpl_lob_stream&&) = delete;
#endif
private:
#else
  otl_tmpl_lob_stream(const otl_tmpl_lob_stream&) OTL_NO_THROW:
  otl_lob_stream_generic(true),
   bind_var(nullptr),
   connect(nullptr),
   cursor(nullptr),
   temp_buf(nullptr),
   temp_char_buf(nullptr)
  {
  }

  otl_tmpl_lob_stream& operator=(const otl_tmpl_lob_stream&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_tmpl_lob_stream(otl_tmpl_lob_stream&&) OTL_NO_THROW:
  otl_lob_stream_generic(true),
   bind_var(nullptr),
   connect(nullptr),
   cursor(nullptr),
   temp_buf(nullptr),
   temp_char_buf(nullptr)
  {
  }

  otl_tmpl_lob_stream& operator=(otl_tmpl_lob_stream&&)
  {
    return *this;
  }
#endif
#endif

};

typedef otl_tmpl_lob_stream
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> otl_lob_stream;

typedef otl_tmpl_exception
  <otl_exc,
   otl_conn,
   otl_cur> otl_exception;

typedef otl_tmpl_inout_stream
 <otl_exc,
  otl_conn,
  otl_cur,
  otl_var,
  otl_time0> otl_ora8_inout_stream;

typedef otl_tmpl_select_stream
 <otl_exc,
  otl_conn,
  otl_cur,
  otl_var,
  otl_sel,
  otl_time0> otl_select_stream;


typedef otl_tmpl_ext_hv_decl
 <otl_var,
  otl_time0,
  otl_exc,
  otl_conn,
  otl_cur> otl_ext_hv_decl;

const int otl_no_stream_type=0;
const int otl_inout_stream_type=1;
const int otl_refcur_stream_type=2;
const int otl_select_stream_type=3;
const int otl_constant_sql_type=4;
const int otl_mixed_refcur_stream_type=5;

class otl_connect: public otl_ora8_connect{
protected:

#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
 otl_stream_pool sc;
  bool pool_enabled_;
#endif

public:

#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)

  otl_stream_pool& get_sc(){return sc;}
  
  void set_stream_pool_size(const int max_size=otl_max_default_pool_size)
  {
    sc.init(max_size);
  }

  void stream_pool_enable()
  {
    pool_enabled_=true;
  }

  void stream_pool_disable()
  {
    pool_enabled_=false;
  }

  bool get_stream_pool_enabled_flag() const
  {
    return pool_enabled_;
  }

#endif

public:

  long direct_exec
  (const char* sqlstm,
   const int exception_enabled=1)
   OTL_THROWS_OTL_EXCEPTION
  {
    return otl_cursor::direct_exec(*this,sqlstm,exception_enabled);
  }

  void syntax_check(const char* sqlstm)
   OTL_THROWS_OTL_EXCEPTION
  {
    otl_cursor::syntax_check(*this,sqlstm);
  }

  otl_connect() OTL_NO_THROW:
    otl_ora8_connect(),
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    sc(),
    pool_enabled_(true),
#endif
    cmd_(nullptr)
  {
  }

#if defined(OTL_ORA_OCI_ENV_CREATE)
  void set_connect_mode(bool threaded_mode=false)
  {
    connect_struct.set_threaded_mode(threaded_mode);
  }
#endif

#if defined(OTL_UNICODE) || defined(OTL_ORA_UTF8)
 void set_character_set(const int char_set=SQLCS_IMPLICIT)
   OTL_THROWS_OTL_EXCEPTION
 {
  connect_struct.set_char_set(char_set);
 }
#endif 

 otl_connect(const char* connect_str, 
             const int aauto_commit=0
#if defined(OTL_ORA_OCI_ENV_CREATE)
             ,bool threaded_mode=false
#endif
            )
   OTL_THROWS_OTL_EXCEPTION
   : otl_ora8_connect(),
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
     sc(),
    pool_enabled_(true),
#endif
     cmd_(nullptr)
  {
#if defined(OTL_ORA_OCI_ENV_CREATE)
    set_connect_mode(threaded_mode);
#endif
    rlogon(connect_str,aauto_commit);
  }

 virtual ~otl_connect() 
  {
    if(cmd_){
      delete[] cmd_;
      cmd_=nullptr;
    }
#if defined(OTL_DESTRUCTORS_DO_NOT_THROW)
    try{
      logoff();
    }catch(OTL_CONST_EXCEPTION otl_exception&){
    }
#else
    logoff();
#endif
  }

  const char* getCmd(void) const
  {
    return cmd_;
  }

  otl_connect& operator<<(const char* cmd)
  {
    if(!connected){
      this->rlogon(cmd);
    }else{
      otl_cursor::direct_exec(*this,cmd);
    }
    return *this;
  }

  otl_connect& operator<<=(const char* cmd)
  {
    if(cmd_){
      delete[] cmd_;
      cmd_=nullptr;
    }
    size_t cmd_len=strlen(cmd);
    cmd_=new char[cmd_len+1];
    OTL_STRCPY_S(cmd_,cmd_len+1,cmd);
    return *this;
  }

 static int otl_terminate(void)
   OTL_THROWS_OTL_EXCEPTION
 {
#if defined(OTL_ORA8)&&!defined(OTL_ORA8I)&&!defined(OTL_ORA9I)
   return 1;
#else
  return OCITerminate(OCI_DEFAULT)==OCI_SUCCESS;
#endif
 }

 void cancel(void)
   OTL_THROWS_OTL_EXCEPTION
 {
  if(!connected)return;
  retcode=connect_struct.cancel();
  if(!retcode){
   increment_throw_count();
   if(get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw otl_exception(connect_struct);
  }
 }

#if defined(OTL_ORA10G_R2)
 void commit_nowait(void)
   OTL_THROWS_OTL_EXCEPTION
 {
  if(!connected)return;
  retcode=connect_struct.commit_nowait();
  if(!retcode){
   increment_throw_count();
   if(get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw otl_exception(connect_struct);
  }
 }
#endif

#if defined(OTL_ORA8I) || defined(OTL_ORA9I)
  void change_password
  (const char* user_name,
   const char* old_password,
   const char* new_password)
    OTL_THROWS_OTL_EXCEPTION
  {
    throw_count=0;
    retcode=connect_struct.change_password
      (user_name,
       old_password,
       new_password);
    if(!retcode){
      increment_throw_count();
      if(get_throw_count()>1)return;
      if(otl_uncaught_exception()) return; 
      throw otl_exception(connect_struct);
    }
  }
#endif

  void auto_commit_off(void)
    OTL_THROWS_OTL_EXCEPTION
  {
    otl_ora8_connect::auto_commit_off();
  }

  void auto_commit_on(void)
    OTL_THROWS_OTL_EXCEPTION
  {
    otl_ora8_connect::auto_commit_on();
  }

 void rlogon(OCIEnv *envhp,OCISvcCtx *svchp)
   OTL_THROWS_OTL_EXCEPTION
 {
   if(this->connected){
     throw otl_exception(otl_error_msg_30,otl_error_code_30);
   }
   if(cmd_){
     delete[] cmd_;
     cmd_=nullptr;
   }
   connected=0;
   long_max_size=otl_short_int_max;
   retcode=connect_struct.ext_logon(envhp,svchp,0);
   if(retcode)
     connected=1;
   else{
     connected=0;
     increment_throw_count();
     if(get_throw_count()>1)return;
     if(otl_uncaught_exception()) return; 
     throw otl_exception(connect_struct);
   }
 }

 void rlogon(const char* connect_str, 
             const int aauto_commit=0,
             const char* xa_server_external_name=nullptr,
             const char* xa_server_internal_name=nullptr
#if defined(OTL_ORA_OCI_ENV_CREATE)
             ,bool threaded_mode=false
#endif
            )
   OTL_THROWS_OTL_EXCEPTION
 {
    if(this->connected){
     throw otl_exception(otl_error_msg_30,otl_error_code_30);
    }
   if(cmd_){
     delete[] cmd_;
     cmd_=nullptr;
   }
   if(xa_server_external_name!=nullptr && xa_server_internal_name!=nullptr){
     connect_struct.set_xa_server_external_name
       (xa_server_external_name);
     connect_struct.set_xa_server_internal_name
       (xa_server_internal_name);
   }
#if defined(OTL_ORA_OCI_ENV_CREATE)
   set_connect_mode(threaded_mode);
#endif
   otl_ora8_connect::rlogon(connect_str,aauto_commit);
   if(connect_struct.get_last_status()==OCI_SUCCESS_WITH_INFO){
     otl_exception ex(connect_struct);
     if(ex.code!=0){
       increment_throw_count();
       if(get_throw_count()>1)return;
       if(otl_uncaught_exception()) return; 
       throw ex;
     }
   }
 }

 void logoff(void)
   OTL_THROWS_OTL_EXCEPTION
 {
#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
  if(connected)
    sc.init(sc.get_max_size());
#endif
  if(!connected){
   connect_struct.session_end();
   connect_struct.server_detach();
  }else{
#if defined(OTL_ROLLS_BACK_BEFORE_LOGOFF)
    otl_ora8_connect::rollback();
#endif
    OTL_TRACE_FUNC(0x1,"otl_connect","logoff","")
      if(connect_struct.get_extern_lda())
        connect_struct.logoff();
    else{
      session_end();
      server_detach();
    }
    connected=0;
  }
 }

 void server_attach(const char* tnsname=nullptr,
                    const char* xa_server_external_name=nullptr,
                    const char* xa_server_internal_name=nullptr
#if defined(OTL_ORA_OCI_ENV_CREATE)
                    ,bool threaded_mode=false
#endif
                   )
   OTL_THROWS_OTL_EXCEPTION
 {
   if(cmd_){
     delete[] cmd_;
     cmd_=nullptr;
   }
   if(xa_server_external_name!=nullptr && xa_server_internal_name!=nullptr){
     connect_struct.set_xa_server_external_name
       (xa_server_external_name);
     connect_struct.set_xa_server_internal_name
       (xa_server_internal_name);
   }
   connected=0;
   long_max_size=otl_short_int_max;
   throw_count=0;
#if defined(OTL_ORA_OCI_ENV_CREATE)
   set_connect_mode(threaded_mode);
#endif
   retcode=connect_struct.server_attach(tnsname);
   if(!retcode){
     increment_throw_count();
     if(get_throw_count()>1)return;
     if(otl_uncaught_exception()) return; 
     throw otl_exception(connect_struct);
   }
 }

 void server_detach(void)
   OTL_THROWS_OTL_EXCEPTION
 {
  retcode=connect_struct.server_detach();
  if(!retcode){
   increment_throw_count();
   if(get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw otl_exception(connect_struct);
  }
 }

 void session_begin
  (const char* username,
   const char* password,
   const int auto_commit=0,
   const int session_mode=OCI_DEFAULT
   // OCI_SYSDBA -- in this mode, the user is authenticated for SYSDBA
   // access.  
   // OCI_SYSOPER -- in this mode, the user is authenticated
   // for SYSOPER access.
  ) OTL_THROWS_OTL_EXCEPTION
 {
   if(cmd_){
     delete[] cmd_;
     cmd_=nullptr;
   }
   throw_count=0;
   retcode=connect_struct.session_begin
     (username,password,auto_commit,session_mode);
   if(retcode)
     connected=1;
   else{
     connected=0;
     increment_throw_count();
     if(get_throw_count()>1)return;
     if(otl_uncaught_exception()) return; 
     throw otl_exception(connect_struct);
   }
   if(connect_struct.get_last_status()==OCI_SUCCESS_WITH_INFO){
     otl_exception ex(connect_struct);
     if(ex.code!=0){
       increment_throw_count();
       if(get_throw_count()>1)return;
       if(otl_uncaught_exception()) return; 
       throw ex;
     }
   }
 }

 void session_reopen(const int auto_commit=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  throw_count=0;
  if(connect_struct.get_session_begin_count()==0){
   connected=0;
   increment_throw_count();
   if(get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw otl_exception(otl_error_msg_11,otl_error_code_11);
  }
  retcode=connect_struct.session_begin(auto_commit);
  if(retcode)
   connected=1;
  else{
   connected=0;
   increment_throw_count();
   if(get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw otl_exception(connect_struct);
  }
  if(connect_struct.get_last_status()==OCI_SUCCESS_WITH_INFO){
    otl_exception ex(connect_struct);
    if(ex.code!=0){
      increment_throw_count();
      if(get_throw_count()>1)return;
      if(otl_uncaught_exception()) return; 
      throw ex;
    }
  }
 }

 void session_end(void)
   OTL_THROWS_OTL_EXCEPTION
 {
#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
  if(connected)
    sc.init(sc.get_max_size());
#endif
  connected=0;
  retcode=connect_struct.session_end();
  if(!retcode){
   increment_throw_count();
   if(get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw otl_exception(connect_struct);
  }
 }

private:

  char* cmd_;

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_connect& operator=(const otl_connect&) = delete;
  otl_connect(const otl_connect&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_connect& operator=(otl_connect&&) = delete;
  otl_connect(otl_connect&&) = delete;
#endif
private:
#else
  otl_connect& operator=(const otl_connect&)
  {
    return *this;
  }

  otl_connect(const otl_connect&)
    : otl_ora8_connect(),
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
      sc(),
      pool_enabled_(true),
#endif
      cmd_(nullptr)
  {
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_connect& operator=(otl_connect&&)
  {
    return *this;
  }

  otl_connect(otl_connect&&)
    : otl_ora8_connect(),
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
      sc(),
      pool_enabled_(true),
#endif
      cmd_(nullptr)
  {
  }
#endif
#endif

};

typedef otl_tmpl_variable<otl_var> otl_generic_variable;
typedef otl_generic_variable* otl_p_generic_variable;

class otl_refcur_base_cursor: public
 otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> {

protected:

 int cur_row;
 int cur_size;
 int row_count;
 int array_size;

public:

 otl_refcur_base_cursor
 (otl_connect& db,
  otl_var* var,
  const char* master_plsql_block,
  const otl_stream_buffer_size_type arr_size=1)
  :otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(db,var),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(arr_size)
 {
  size_t len=strlen(master_plsql_block)+1;
  stm_text=new char[len];
  OTL_STRCPY_S(stm_text,len,master_plsql_block);
 }

 otl_refcur_base_cursor():
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(0)
 {
 }

 virtual ~otl_refcur_base_cursor()
 {
  delete[] stm_text;
  stm_text=nullptr;
 }

 void open
 (otl_connect& db,
  otl_var* var,
  const char* master_plsql_block,
  const otl_stream_buffer_size_type arr_size=1)
 {
  cur_row=-1;
  row_count=0;
  cur_size=0;
  array_size=arr_size;
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::open(db,var);
  size_t len=strlen(master_plsql_block)+1;
  stm_text=new char[len];
  OTL_STRCPY_S(stm_text,len,master_plsql_block);
 }

 void close(void)
 {
  delete[] stm_text;
  stm_text=nullptr;
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::close();
 }

 int first(void)
 {int rc;

  cur_row=-1;
  rc=cursor_struct.fetch(OTL_SCAST
                         (otl_stream_buffer_size_type,
                          array_size),
                         eof_data);
  if(rc==0){
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return 0;
  if(otl_uncaught_exception()) return 0; 
   throw otl_exception(cursor_struct,stm_label?stm_label:stm_text);
  }
  row_count=cursor_struct.rpc();
  cur_size=row_count;
  if(cur_size!=0)cur_row=0;
  return cur_size!=0;
 }

 int next(void)
 {int rc;
  if(cur_row<0)return first();
  if(cur_row<cur_size-1)
   ++cur_row;
  else{
   if(otl_tmpl_cursor<otl_exc,otl_conn,otl_cur,otl_var>::eof()){
    cur_row=-1;
    return 0;
   }
   rc=cursor_struct.fetch(OTL_SCAST(otl_stream_buffer_size_type,
                                    array_size),eof_data);
   if(rc==0){
     if(this->adb)this->adb->increment_throw_count();
     if(this->adb&&this->adb->get_throw_count()>1)return 0;
     if(otl_uncaught_exception()) return 0; 
     throw otl_exception(cursor_struct,stm_label?stm_label:stm_text);
   }
   cur_size=cursor_struct.rpc()-row_count;
   row_count=cursor_struct.rpc();
   if(cur_size!=0)cur_row=0;
  }
  return cur_size!=0;
 }

 void bind_col
 (const int column_num,
  otl_generic_variable& v)
 {
  if(!connected)return;
  v.set_pos(column_num);
  otl_refcur_base_cursor::bind(column_num,v);
 }

 int describe_select
 (otl_column_desc* desc,
  int& desc_len)
 {int i;
  desc_len=0;
  cursor_struct.straight_select=0;
  for(i=1;describe_column(desc[i-1],i);++i)
   ++desc_len;
  return 1;
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_refcur_base_cursor(const otl_refcur_base_cursor&) = delete;
 otl_refcur_base_cursor& operator=(const otl_refcur_base_cursor&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_refcur_base_cursor(otl_refcur_base_cursor&&) = delete;
 otl_refcur_base_cursor& operator=(otl_refcur_base_cursor&&) = delete;
#endif
private:
#else
 otl_refcur_base_cursor(const otl_refcur_base_cursor&):
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(0)
 {
 }

 otl_refcur_base_cursor& operator=(const otl_refcur_base_cursor&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_refcur_base_cursor(otl_refcur_base_cursor&&):
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(0)
 {
 }

 otl_refcur_base_cursor& operator=(otl_refcur_base_cursor&&)
 {
   return *this;
 }
#endif
#endif

};

#if defined(OTL_ORA_DECLARE_COMMON_READ_STREAM_INTERFACE)

#define OTL_ORA_COMMON_READ_STREAM otl_read_stream_interface
#define OTL_ORA_REFCUR_COMMON_READ_STREAM otl_read_stream_interface

class otl_read_stream_interface{
public:

  virtual ~otl_read_stream_interface(){}

  virtual int is_null(void) OTL_NO_THROW = 0;
  virtual void rewind(void) OTL_THROWS_OTL_EXCEPTION = 0;
  virtual int eof(void) OTL_NO_THROW = 0;
  virtual void skip_to_end_of_row(void) OTL_NO_THROW = 0;

  virtual otl_var_desc* describe_out_vars(int& desc_len) OTL_NO_THROW = 0;
  virtual otl_var_desc* describe_next_out_var(void) OTL_NO_THROW = 0;

  virtual otl_read_stream_interface& 
  operator>>(otl_datetime& s) OTL_THROWS_OTL_EXCEPTION = 0;

#if !defined(OTL_UNICODE)
  virtual otl_read_stream_interface& 
  operator>>(char& c) OTL_THROWS_OTL_EXCEPTION = 0;
#endif

  virtual otl_read_stream_interface& 
  operator>>(unsigned char& c) OTL_THROWS_OTL_EXCEPTION = 0;

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
  virtual otl_read_stream_interface& 
  operator>>(OTL_STRING_CONTAINER& s) OTL_THROWS_OTL_EXCEPTION = 0;
#endif

#if defined(OTL_UNICODE_STRING_TYPE)
  virtual otl_read_stream_interface& 
  operator>>(OTL_UNICODE_STRING_TYPE& s) OTL_THROWS_OTL_EXCEPTION = 0;
#endif

#if !defined(OTL_UNICODE)
  virtual otl_read_stream_interface& 
  operator>>(char* s) OTL_THROWS_OTL_EXCEPTION = 0;
#endif

#if defined(OTL_UNICODE)

 virtual otl_read_stream_interface& 
 operator>>(OTL_UNICODE_CHAR_TYPE* s) OTL_THROWS_OTL_EXCEPTION = 0;

#endif

  virtual otl_read_stream_interface& 
  operator>>(unsigned char* s) OTL_THROWS_OTL_EXCEPTION = 0;

#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
  virtual otl_read_stream_interface& 
  operator>>(OTL_NUMERIC_TYPE_1& f) OTL_THROWS_OTL_EXCEPTION = 0;
#endif

#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
  virtual otl_read_stream_interface& 
  operator>>(OTL_NUMERIC_TYPE_2& f) OTL_THROWS_OTL_EXCEPTION = 0;
#endif

#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
  virtual otl_read_stream_interface& 
  operator>>(OTL_NUMERIC_TYPE_3& f) OTL_THROWS_OTL_EXCEPTION = 0;
#endif

#if defined(OTL_BIGINT) && (defined(OTL_ORA11G_R2)&&!defined(OTL_STR_TO_BIGINT)&&\
    !defined(OTL_BIGINT_TO_STR))
  virtual otl_read_stream_interface& 
  operator>>(OTL_BIGINT& f) OTL_THROWS_OTL_EXCEPTION = 0;
#endif


#if defined(OTL_BIGINT) && defined(OTL_STR_TO_BIGINT) && defined(OTL_BIGINT_TO_STR)
  virtual otl_read_stream_interface& 
  operator>>(OTL_BIGINT& f) OTL_THROWS_OTL_EXCEPTION = 0;
#endif

#if defined(OTL_UBIGINT) && defined(OTL_ORA11G_R2)
  virtual otl_read_stream_interface& 
  operator>>(OTL_UBIGINT& f) OTL_THROWS_OTL_EXCEPTION = 0;
#endif

  virtual otl_read_stream_interface& 
  operator>>(int& n) OTL_THROWS_OTL_EXCEPTION = 0;

  virtual otl_read_stream_interface& 
  operator>>(unsigned& u) OTL_THROWS_OTL_EXCEPTION = 0;

  virtual otl_read_stream_interface& 
  operator>>(short& sh) OTL_THROWS_OTL_EXCEPTION = 0;

  virtual otl_read_stream_interface& 
  operator>>(long int& l) OTL_THROWS_OTL_EXCEPTION = 0;

  virtual otl_read_stream_interface& 
  operator>>(float& f) OTL_THROWS_OTL_EXCEPTION = 0;

  virtual otl_read_stream_interface& 
  operator>>(double& d) OTL_THROWS_OTL_EXCEPTION = 0;

  virtual otl_read_stream_interface& 
  operator>>(otl_long_string& s) OTL_THROWS_OTL_EXCEPTION = 0;

  virtual otl_read_stream_interface& 
  operator>>(otl_lob_stream& s) OTL_THROWS_OTL_EXCEPTION = 0;

  virtual otl_column_desc* 
  describe_select(int& desc_len) OTL_NO_THROW = 0;

};
#else

#define OTL_ORA_COMMON_READ_STREAM otl_stream
#define OTL_ORA_REFCUR_COMMON_READ_STREAM otl_refcur_stream

#endif

class otl_refcur_stream:
#if defined(OTL_ORA_DECLARE_COMMON_READ_STREAM_INTERFACE)
  public otl_read_stream_interface,
#endif
 public otl_refcur_base_cursor{

protected:

  int delay_next;
  int same_sl_flag;
  otl_select_struct_override override_;

  otl_var_desc* ov;  
  int ov_len;
  int next_ov_ndx;

  void inc_next_ov(void)
  {
    if(ov_len==0)return;
    if(next_ov_ndx<ov_len-1)
      ++next_ov_ndx;
    else
      next_ov_ndx=0;
  }

public:

  void skip_to_end_of_row() OTL_NO_THROW
  {
    check_if_executed();
    if(eof())return;
    while(cur_col<sl_len-1){
      ++cur_col;
      null_fetched=sl[cur_col].is_null(this->cur_row);
    }
    ret_code=this->next();
    cur_col=0;
    if(!eof())
      cur_col=-1;
  }


  bool good() const
  {
    return get_connected()==1;
  }

  bool get_lob_stream_flag() const 
  {
    return true;
  }

  int get_adb_max_long_size() const 
  {
    return this->adb->get_max_long_size();
  }

  void set_column_type(const int column_ndx,
                       const int col_type,
                       const int col_size=0)
    OTL_NO_THROW
  {
    override_.add_override(column_ndx,col_type,col_size);
  }
  
  void set_all_column_types(const unsigned mask=0)
    OTL_NO_THROW
  {
    override_.set_all_column_types(mask);
  }

 void cleanup(void)
 {int i;
  delete[] sl;
  delete[] ov;
  for(i=0;i<vl_len;++i)
   delete vl[i];
  delete[] vl;
  delete[] sl_desc;
 }

  otl_refcur_stream() OTL_NO_THROW:
   otl_refcur_base_cursor(),
   delay_next(0),
   same_sl_flag(0),
   override_(),
   ov(nullptr),
   ov_len(0),
   next_ov_ndx(0),
   sl_desc(nullptr),
   sl_len(),
   sl(nullptr),
   null_fetched(0),
   ret_code(0),
   cur_col(0),
   cur_in(0),
   executed(0),
   var_info()
 {
   init();
 }

 otl_refcur_stream
 (const otl_stream_buffer_size_type arr_size,
  const char* master_plsql_block,
  otl_var* var,
  otl_connect& db)
 OTL_THROWS_OTL_EXCEPTION:
   otl_refcur_base_cursor(db,var,master_plsql_block,arr_size),
   delay_next(0),
   same_sl_flag(0),
   override_(),
   ov(nullptr),
   ov_len(0),
   next_ov_ndx(0),
   sl_desc(nullptr),
    sl_len(),
    sl(nullptr),
   null_fetched(0),
   ret_code(0),
   cur_col(0),
   cur_in(0),
   executed(0),
   var_info()
 {
  init();
  try{
   rewind();
   null_fetched=0;
  }catch(OTL_CONST_EXCEPTION otl_exception&){
   cleanup();
   if(this->adb)this->adb->increment_throw_count();
   throw;
  }
 }

 virtual ~otl_refcur_stream()
 {
  cleanup();
  close();
 }


 int is_null(void) OTL_NO_THROW
 {
  return null_fetched;
 }

 int eof(void)
#if defined(OTL_SELECT_STREAM_ALTERNATE_FETCH)
   OTL_THROWS_OTL_EXCEPTION
#else
   OTL_NO_THROW
#endif
 {
#if defined(OTL_SELECT_STREAM_ALTERNATE_FETCH)
   if(cur_col==sl_len-1){
     get_next();
     cur_col=-1;
   }else{
     if(delay_next){
       look_ahead();
       delay_next=0;
     }
   }
  return !ret_code;
#else
  if(delay_next){
   look_ahead();
   delay_next=0;
  }
  return !ret_code;
#endif
 }

 int eof_intern(void)
 {
  return !ret_code;
 }

 void check_if_executed(void){}

  void open
  (otl_connect& db,
   otl_var* var,
   const char* master_plsql_block,
   const otl_stream_buffer_size_type arr_size=1)
    OTL_THROWS_OTL_EXCEPTION
  {
    otl_refcur_base_cursor::open(db,var,master_plsql_block,arr_size);
    get_select_list();
    rewind();
    delete[] ov;
    ov=new otl_var_desc[sl_len];
    ov_len=sl_len;
    for(int i=0;i<sl_len;++i){
      sl[i].copy_var_desc(ov[i]);
      if(sl_desc!=nullptr)
        ov[i].copy_name(sl_desc[i].name);
    }
  }
  
  void close(void)
    OTL_THROWS_OTL_EXCEPTION
  {
    override_.reset();
    otl_refcur_base_cursor::close();
  }

 otl_refcur_stream& operator>>(otl_time0& t)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_timestamp)&&!eof_intern()){
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
   void* tm=OTL_RCAST(void*,sl[cur_col].val(this->cur_row));
   int rc=sl[cur_col].get_var_struct().read_dt(&t,tm,sizeof(otl_time0));
   if(rc==0){
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return *this;
    if(otl_uncaught_exception()) return *this; 
    throw otl_exception(adb->get_connect_struct(),stm_label?stm_label:stm_text);
   }
#else
   otl_time0* tm=OTL_RCAST(otl_time0*,sl[cur_col].val(cur_row));
   memcpy(OTL_RCAST(void*,&t),tm,otl_oracle_date_size);
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }

#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  // already declared
#else
 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {otl_time0 tmp;
  (*this)>>tmp;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
  if((*this).is_null())
   s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
  else{
    s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
    s.month=tmp.month;
    s.day=tmp.day;
    s.hour=tmp.hour-1;
    s.minute=tmp.minute-1;
    s.second=tmp.second-1;
  }
#else
  s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
  s.month=tmp.month;
  s.day=tmp.day;
  s.hour=tmp.hour-1;
  s.minute=tmp.minute-1;
  s.second=tmp.second-1;
#endif
  inc_next_ov();
  return *this;
 }
#endif

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   c=*OTL_RCAST(char*,sl[cur_col].val(cur_row));
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(unsigned char& c)
   OTL_THROWS_OTL_EXCEPTION
 {

  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   c=*OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  switch(sl[cur_col].get_ftype()){
  case otl_var_char:
    if(!eof_intern()){
#if defined(OTL_USER_DEFINED_STRING_CLASS_DEFAULT_NULL_TO_VAL)
      if((*this).is_null()){
        OTL_USER_DEFINED_STRING_CLASS_DEFAULT_NULL_TO_VAL(s);
      }else
#elif defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
      if((*this).is_null())
        s=OTL_DEFAULT_STRING_NULL_TO_VAL;
      else
#endif
#if defined(OTL_ACE)
        s.set(OTL_RCAST(char*,sl[cur_col].val(cur_row)),1);
#else
        s=OTL_RCAST(char*,sl[cur_col].val(cur_row));
#endif
      look_ahead();
    }
    break;
#if defined(USER_DEFINED_STRING_CLASS) || \
    defined(OTL_STL) || defined(OTL_ACE)
  case otl_var_varchar_long:
  case otl_var_raw_long:
    if(!eof_intern()){
      unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
      int len=sl[cur_col].get_len(cur_row);
#if (defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)) && !defined(OTL_ACE)
      s.assign(OTL_RCAST(char*,c),len);
#elif defined(OTL_ACE)
      s.set(OTL_RCAST(char*,c),len,1);
#endif
      look_ahead();
    }
    break;
  case otl_var_blob:
  case otl_var_clob:
    if(!eof_intern()){
      int len=0;
      int max_long_sz=this->adb->get_max_long_size();
      otl_auto_array_ptr<unsigned char> loc_ptr(max_long_sz);
      unsigned char* temp_buf=loc_ptr.get_ptr();
    
      int rc=sl[cur_col].get_var_struct().get_blob
        (cur_row,
         temp_buf,
         max_long_sz,
         len);
      if(rc==0){
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
        throw otl_exception(adb->get_connect_struct(),
                            stm_label?stm_label:stm_text);
      }
#if (defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)) && !defined(OTL_ACE)
      s.assign(OTL_RCAST(char*,temp_buf),len);
#elif defined(OTL_ACE)
      s.set(OTL_RCAST(char*,temp_buf),len,1);
#endif
      look_ahead();
    }
    break;
#endif
  default:
    check_type(otl_var_char);
  } // switch
  inc_next_ov();
  return *this;
 }
#endif

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   otl_strcpy(OTL_RCAST(unsigned char*,s),
              OTL_RCAST(const unsigned char*,sl[cur_col].val(cur_row)));
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     otl_strcpy(OTL_RCAST(unsigned char*,s),
                OTL_RCAST(const unsigned char*,OTL_DEFAULT_STRING_NULL_TO_VAL)
               );
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }

#if defined(OTL_UNICODE_STRING_TYPE)

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(OTL_UNICODE_STRING_TYPE& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
#if defined(OTL_UNICODE_STRING_TYPE_CAST_FROM_CHAR)
    OTL_UNICODE_CHAR_TYPE* temp_s=OTL_RCAST
      (OTL_UNICODE_CHAR_TYPE*,sl[cur_col].val(cur_row));
    OTL_UNICODE_STRING_TYPE_CAST_FROM_CHAR(s,temp_s+1,*temp_s);
#else
    OTL_UNICODE_CHAR_TYPE* temp_s=OTL_RCAST
      (OTL_UNICODE_CHAR_TYPE*,sl[cur_col].val(cur_row));
    s.assign(temp_s+1,*temp_s);
#endif

#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
    s=OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,OTL_DEFAULT_STRING_NULL_TO_VAL);
#endif

    look_ahead();
  }
  inc_next_ov();
  return *this;
 }

#endif

#if defined(OTL_UNICODE)

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(OTL_UNICODE_CHAR_TYPE* s)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   otl_strcpy2(OTL_RCAST(unsigned char*,s),
               OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row)),
               sl[cur_col].get_len(cur_row)
             );
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     otl_strcpy(OTL_RCAST(unsigned char*,s),
                OTL_RCAST(const unsigned char*,OTL_DEFAULT_STRING_NULL_TO_VAL)
               );
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }

#endif

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   otl_strcpy2(OTL_RCAST(unsigned char*,s),
               OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row)),
               sl[cur_col].get_len(cur_row)
             );
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     otl_strcpy(OTL_RCAST(unsigned char*,s),
                OTL_RCAST(const unsigned char*,OTL_DEFAULT_STRING_NULL_TO_VAL)
               );
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(int& n)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T2
      (sl[cur_col].get_ftype(),
       sl[cur_col].val(cur_row),
       n);
#else
    int match_found=otl_numeric_convert_T
      (sl[cur_col].get_ftype(),
       sl[cur_col].val(cur_row),
       n);
#endif
   if(!match_found){
    if(check_type(otl_var_double,otl_var_int))
      n=OTL_PCONV(int,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }

#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(OTL_NUMERIC_TYPE_1& n)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T2
      (sl[cur_col].get_ftype(),
       sl[cur_col].val(cur_row),
       n);
#else
    int match_found=otl_numeric_convert_T
      (sl[cur_col].get_ftype(),
       sl[cur_col].val(cur_row),
       n);
#endif
   if(!match_found){
    if(check_type(otl_var_double,otl_var_int))
      n=OTL_PCONV(OTL_NUMERIC_TYPE_1,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(OTL_NUMERIC_TYPE_1,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }
#endif

#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(OTL_NUMERIC_TYPE_2& n)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T2
      (sl[cur_col].get_ftype(),
       sl[cur_col].val(cur_row),
       n);
#else
    int match_found=otl_numeric_convert_T
      (sl[cur_col].get_ftype(),
       sl[cur_col].val(cur_row),
       n);
#endif
   if(!match_found){
    if(check_type(otl_var_double,otl_var_int))
      n=OTL_PCONV(OTL_NUMERIC_TYPE_2,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(OTL_NUMERIC_TYPE_2,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }
#endif

#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(OTL_NUMERIC_TYPE_3& n)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T2
      (sl[cur_col].get_ftype(),
       sl[cur_col].val(cur_row),
       n);
#else
    int match_found=otl_numeric_convert_T
      (sl[cur_col].get_ftype(),
       sl[cur_col].val(cur_row),
       n);
#endif
   if(!match_found){
    if(check_type(otl_var_double,otl_var_int))
      n=OTL_PCONV(OTL_NUMERIC_TYPE_3,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(OTL_NUMERIC_TYPE_3,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }
#endif

#if defined(OTL_BIGINT) && (defined(OTL_ORA11G_R2)&&!defined(OTL_STR_TO_BIGINT)&&\
    !defined(OTL_BIGINT_TO_STR))
 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(OTL_BIGINT& n)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T2
      (sl[cur_col].get_ftype(),
       sl[cur_col].val(cur_row),
       n);
#else
    int match_found=otl_numeric_convert_T
      (sl[cur_col].get_ftype(),
       sl[cur_col].val(cur_row),
       n);
#endif
   if(!match_found){
    if(check_type(otl_var_double,otl_var_int))
      n=OTL_PCONV(OTL_BIGINT,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(OTL_BIGINT,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }
#endif

#if defined(OTL_BIGINT) && defined(OTL_STR_TO_BIGINT) && \
    defined(OTL_BIGINT_TO_STR)

otl_refcur_stream& operator>>(OTL_BIGINT& n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_bigint_str_size];
  (*this)>>temp_val;
  if(this->is_null()){
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if(this->is_null())
     n=OTL_SCAST(OTL_BIGINT,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
    return *this;
  }
  OTL_STR_TO_BIGINT(temp_val,n)
  return *this;
}

#elif defined(OTL_BIGINT) && defined(OTL_ORA_MAP_BIGINT_TO_LONG)

  otl_refcur_stream& operator>>(OTL_BIGINT& n)
    OTL_THROWS_OTL_EXCEPTION
  {
    long temp_val;
    (*this)>>temp_val;
    if(this->is_null()){
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
      if(this->is_null())
        n=OTL_SCAST(OTL_BIGINT,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
      return *this;
    }
    n=OTL_SCAST(OTL_BIGINT,temp_val);
    return *this;
  }


#endif

#if defined(OTL_UBIGINT) && defined(OTL_ORA11G_R2)
 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(OTL_UBIGINT& n)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T2
      (sl[cur_col].get_ftype(),
       sl[cur_col].val(cur_row),
       n);
#else
    int match_found=otl_numeric_convert_T
      (sl[cur_col].get_ftype(),
       sl[cur_col].val(cur_row),
       n);
#endif
   if(!match_found){
    if(check_type(otl_var_double,otl_var_int))
      n=OTL_PCONV(OTL_UBIGINT,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(OTL_UBIGINT,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }
#endif

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(unsigned& u)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
   int match_found=otl_numeric_convert_T2
     (sl[cur_col].get_ftype(),
      sl[cur_col].val(cur_row),
      u);
#else
   int match_found=otl_numeric_convert_T
     (sl[cur_col].get_ftype(),
     sl[cur_col].val(cur_row),
     u);
#endif
   if(!match_found){
    if(check_type(otl_var_double,otl_var_unsigned_int))
      u=OTL_PCONV(unsigned,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     u=OTL_SCAST(unsigned int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(short& sh)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
   int match_found=otl_numeric_convert_T2
     (sl[cur_col].get_ftype(),
      sl[cur_col].val(cur_row),
      sh);
#else
   int match_found=otl_numeric_convert_T
     (sl[cur_col].get_ftype(),
     sl[cur_col].val(cur_row),
     sh);
#endif
   if(!match_found){
    if(check_type(otl_var_double,otl_var_short))
      sh=OTL_PCONV(short,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     sh=OTL_SCAST(short int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(long int& l)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
   int match_found=otl_numeric_convert_T2
     (sl[cur_col].get_ftype(),
     sl[cur_col].val(cur_row),
     l);
#else
   int match_found=otl_numeric_convert_T
     (sl[cur_col].get_ftype(),
     sl[cur_col].val(cur_row),
     l);
#endif
   if(!match_found){
    if(check_type(otl_var_double,otl_var_long_int))
      l=OTL_PCONV(long int,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     l=OTL_SCAST(long int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(float& f)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
   int match_found=otl_numeric_convert_T2
     (sl[cur_col].get_ftype(),
     sl[cur_col].val(cur_row),
     f);
#else
   int match_found=otl_numeric_convert_T
     (sl[cur_col].get_ftype(),
     sl[cur_col].val(cur_row),
     f);
#endif
   if(!match_found){
    if(check_type(otl_var_double,otl_var_float))
      f=OTL_PCONV(float,double,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     f=OTL_SCAST(float,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(double& d)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(!eof_intern()){
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
   int match_found=otl_numeric_convert_T2
     (sl[cur_col].get_ftype(),
      sl[cur_col].val(cur_row),
      d);
#else
   int match_found=otl_numeric_convert_T
     (sl[cur_col].get_ftype(),
     sl[cur_col].val(cur_row),
     d);
#endif
   if(!match_found){
    if(check_type(otl_var_double,otl_var_double))
     d=*OTL_RCAST(double*,sl[cur_col].val(cur_row));
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     d=OTL_SCAST(double,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   look_ahead();
  }
  inc_next_ov();
  return *this;
 }

OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   check_if_executed();
   if(eof_intern())return *this;
   get_next();
   if(!eof_intern()){
     switch(sl[cur_col].get_ftype()){
     case otl_var_raw_long:
     case otl_var_varchar_long:
     {
       bool in_unicode_mode=sizeof(OTL_CHAR)>1;
       if(!s.get_unicode_flag() && in_unicode_mode &&
          sl[cur_col].get_ftype()==otl_var_varchar_long){
         throw otl_exception
           (otl_error_msg_37,
            otl_error_code_37,
            this->stm_label?this->stm_label:
            this->stm_text);
       }else if(s.get_unicode_flag() && sl[cur_col].get_ftype()==otl_var_raw_long){
         throw otl_exception
           (otl_error_msg_38,
            otl_error_code_38,
            this->stm_label?this->stm_label:
            this->stm_text);
       }
       unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
       int len=sl[cur_col].get_len(cur_row);
       if(len>s.get_buf_size())
         len=s.get_buf_size();
       otl_memcpy(s.v,c,len,sl[cur_col].get_ftype());
       if(sl[cur_col].get_ftype()==otl_var_varchar_long)
         s.null_terminate_string(len);
       s.set_len(len);
       look_ahead();
     }
     break;
     case otl_var_blob:
     case otl_var_clob:
     {
       bool in_unicode_mode=sizeof(OTL_CHAR)>1;
       if(!s.get_unicode_flag() && in_unicode_mode &&
          sl[cur_col].get_ftype()==otl_var_clob){
         throw otl_exception
           (otl_error_msg_37,
            otl_error_code_37,
            this->stm_label?this->stm_label:
            this->stm_text);
       }else if(s.get_unicode_flag() && sl[cur_col].get_ftype()==otl_var_blob){
         throw otl_exception
           (otl_error_msg_38,
            otl_error_code_38,
            this->stm_label?this->stm_label:
            this->stm_text);
       }
       int len;
       int rc=sl[cur_col].get_var_struct().get_blob(cur_row,s.v,s.get_buf_size(),len);
       if(rc==0){
         if(this->adb)this->adb->increment_throw_count();
         if(this->adb&&this->adb->get_throw_count()>1)return *this;
         if(otl_uncaught_exception()) return *this;
         throw otl_exception(adb->get_connect_struct(),
                             stm_label?stm_label:stm_text);
       }
       s.set_len(len);
       if(sl[cur_col].get_ftype()==otl_var_clob)
         s.null_terminate_string(len);
       look_ahead();
     }     
     break;
     case otl_var_raw:
     {
       if(s.get_unicode_flag()){
         throw otl_exception
           (otl_error_msg_38,
            otl_error_code_38,
            this->stm_label?this->stm_label:
            this->stm_text);
       }       
       unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(this->cur_row));
       int len2=OTL_SCAST(int,*OTL_RCAST(unsigned short*,c));
       otl_memcpy(s.v,c+sizeof(short int),len2,sl[cur_col].get_ftype());
       s.set_len(len2);
       look_ahead();
     }
     break;
     }
   }
   inc_next_ov();
   return *this;
 }

 OTL_ORA_REFCUR_COMMON_READ_STREAM& operator>>(otl_lob_stream& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if((sl[cur_col].get_ftype()==otl_var_blob||
      sl[cur_col].get_ftype()==otl_var_clob)&&
     !eof_intern()){
   s.init
    (&sl[cur_col],
     adb,
     OTL_RCAST(otl_refcur_base_cursor*,this),
     cur_row,
     otl_lob_stream_read_mode,
     this->is_null());
   delay_next=1;
  }
  inc_next_ov();
  return *this;
 }

 otl_var_desc* describe_out_vars(int& desc_len)
   OTL_NO_THROW
 {
   desc_len=0;
   if(ov==nullptr)return nullptr;
   desc_len=ov_len;
   return ov;
 }

 otl_var_desc* describe_next_out_var(void)
   OTL_NO_THROW
 {
   if(ov==nullptr)return nullptr;
   return &ov[next_ov_ndx];
 }

 int select_list_len(void) OTL_NO_THROW
 {
  return sl_len;
 }

 int column_ftype(int ndx=0) OTL_NO_THROW
 {
   return sl[ndx].get_ftype();
 }

 int column_size(int ndx=0) OTL_NO_THROW
 {
   return sl[ndx].get_elem_size();
 }

 otl_column_desc* describe_select(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  desc_len=sl_len;
  return sl_desc;
 }

protected:

 otl_column_desc* sl_desc;
 int sl_len;
 otl_generic_variable* sl;

 int null_fetched;
 int ret_code;
 int cur_col;
 int cur_in;
 int executed;
 char var_info[256];

 void init(void)
 {
   ov=nullptr;
   ov_len=0;
   next_ov_ndx=0;
   same_sl_flag=0;
   sl=nullptr;
   sl_len=0;
   null_fetched=0;
   ret_code=0;
   sl_desc=nullptr;
   executed=0;
   cur_in=0;
   cur_col=-1;
   executed=1;
   stm_text=nullptr;
   delay_next=0;
 }

 void get_next(void)
 {
  if(cur_col<sl_len-1){
   ++cur_col;
   null_fetched=sl[cur_col].is_null(cur_row);
  }else{
   ret_code=next();
   cur_col=0;
  }
 }

  int check_type_throw(int type_code, int actual_data_type)
  {
    int out_type_code;
    if(actual_data_type!=0)
      out_type_code=actual_data_type;
    else
      out_type_code=type_code;
    otl_var_info_col
      (sl[cur_col].get_pos(),
       sl[cur_col].get_ftype(),
       out_type_code,
       var_info,
       sizeof(var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return 0;
    if(otl_uncaught_exception()) return 0; 
    throw otl_exception
      (otl_error_msg_0,
       otl_error_code_0,
       stm_label?stm_label:stm_text,
       var_info);
  }
  
  int check_type(int type_code, int actual_data_type=0)
  {
    switch(sl[cur_col].get_ftype()){
    case otl_var_timestamp:
    case otl_var_tz_timestamp:
    case otl_var_ltz_timestamp:
      if(type_code==otl_var_timestamp)
        return 1;
      break;
    default:
      if(sl[cur_col].get_ftype()==type_code)
        return 1;
      break;
    }
    return check_type_throw(type_code,actual_data_type);
  }
  
 void look_ahead(void)
 {
#if defined(OTL_SELECT_STREAM_ALTERNATE_FETCH)
#else
  if(cur_col==sl_len-1){
   ret_code=next();
   cur_col=-1;
  }
#endif
 }

 void get_select_list(void)
 {
   int i,j;

   otl_auto_array_ptr<otl_column_desc> loc_ptr(otl_var_list_size);
   otl_column_desc* sl_desc_tmp=loc_ptr.get_ptr();
   int sld_tmp_len=0;
   int ftype,elem_size;
   
   sld_tmp_len=0;
   cursor_struct.straight_select=0;
   for(i=1;describe_column(sl_desc_tmp[i-1],i);++i){
     ++sld_tmp_len;
     if(sld_tmp_len==loc_ptr.get_arr_size()){
       loc_ptr.double_size();
       sl_desc_tmp=loc_ptr.get_ptr();
     }
   }
   sl_len=sld_tmp_len;
   if(sl){
     delete[] sl;
     sl=nullptr;
   }
   sl=new otl_generic_variable[sl_len==0?1:sl_len];
   int max_long_size=this->adb->get_max_long_size();
   for(j=0;j<sl_len;++j){
     otl_generic_variable::map_ftype
       (sl_desc_tmp[j],
        max_long_size,
        ftype,
        elem_size,
        override_,
        j+1,
        this->adb->get_connect_struct().get_connection_type());
     sl[j].copy_pos(j+1);
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
     if(sl_desc_tmp[j].charset_form==2)
       sl[j].get_var_struct().nls_flag=true;
#endif
     sl[j].init(true,
                ftype,
                elem_size,
                OTL_SCAST(otl_stream_buffer_size_type,array_size),
                &adb->get_connect_struct()
               );
   }
   if(sl_desc){
     delete[] sl_desc;
     sl_desc=nullptr;
   }
   sl_desc=new otl_column_desc[sl_len==0?1:sl_len];
   for(i=0;i<sl_len;++i)
     sl_desc[i]=sl_desc_tmp[i];
   for(i=0;i<sl_len;++i)bind_col(i+1,sl[i]);
 }

private:

 void rewind(void)
   OTL_THROWS_OTL_EXCEPTION
 {
  ret_code=first();
  null_fetched=0;
  cur_col=-1;
  cur_in=0;
  executed=1;
  delay_next=0;
 }

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_refcur_stream& operator=(const otl_refcur_stream&) = delete;
  otl_refcur_stream(const otl_refcur_stream&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_refcur_stream& operator=(otl_refcur_stream&&) = delete;
  otl_refcur_stream(otl_refcur_stream&&) = delete;
#endif
private:
#else
  otl_refcur_stream& operator=(const otl_refcur_stream&)
  {
    return *this;
  }

  otl_refcur_stream(const otl_refcur_stream&) : 
#if defined(OTL_ORA_DECLARE_COMMON_READ_STREAM_INTERFACE)
    otl_read_stream_interface(), 
#endif
    otl_refcur_base_cursor(),
    delay_next(0),
    same_sl_flag(0),
    override_(),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    sl_desc(nullptr),
    sl_len(),
    sl(nullptr),
    null_fetched(0),
    ret_code(0),
    cur_col(0),
    cur_in(0),
    executed(0),
    var_info()
 {
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_refcur_stream& operator=(otl_refcur_stream&&)
  {
    return *this;
  }

  otl_refcur_stream(otl_refcur_stream&&) : 
#if defined(OTL_ORA_DECLARE_COMMON_READ_STREAM_INTERFACE)
    otl_read_stream_interface(), 
#endif
    otl_refcur_base_cursor(),
    delay_next(0),
    same_sl_flag(0),
    override_(),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    sl_desc(nullptr),
    sl_len(),
    sl(nullptr),
    null_fetched(0),
    ret_code(0),
    cur_col(0),
    cur_in(0),
    executed(0),
    var_info()
 {
 }
#endif
#endif

};

class otl_inout_stream: public otl_ora8_inout_stream{
public:

 otl_inout_stream
 (otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  otl_connect& db,
  void* master_stream_ptr,
  const bool alob_stream_mode=false,
  const char* sqlstm_label=nullptr)
  : otl_ora8_inout_stream(arr_size,sqlstm,db,
                          master_stream_ptr,
                          alob_stream_mode,sqlstm_label),
    adb2(&db)
 {
 } 

 otl_inout_stream& operator>>(otl_refcur_stream& str)
 {
  if(eof())return *this;
  if(check_in_type(otl_var_refcur,1)){
    if(str.get_connected())str.close();
    str.open(*adb2,
             &(in_vl[cur_in_x]->get_var_struct()),
             stm_text,
             OTL_SCAST
             (/*const*/ otl_stream_buffer_size_type, 
              in_vl[cur_in_x]->get_var_struct().array_size)
            );
    null_fetched=0;
  }
  get_in_next();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)

 otl_inout_stream& operator>>(OTL_STRING_CONTAINER& s)
 {
  otl_ora8_inout_stream::operator>>(s);
  return *this;
 }

 otl_inout_stream& operator<<(const OTL_STRING_CONTAINER& s)
 {
  otl_ora8_inout_stream::operator<<(s);
  return *this;
 }

#endif

 otl_inout_stream& operator<<(const otl_null& n)
 {
  otl_ora8_inout_stream::operator<<(n);
  return *this;
 }


#if defined(OTL_PL_TAB) && defined(OTL_STL)

 otl_inout_stream& operator>>(otl_pl_vec_generic& tab)
 {
  otl_ora8_inout_stream::operator>>(tab);
  return *this;
 }

 otl_inout_stream& operator<<(otl_pl_vec_generic& tab)
 {
  otl_ora8_inout_stream::operator<<(tab);
  return *this;
 }

#endif

 otl_inout_stream& operator>>(otl_pl_tab_generic& tab)
 {
  otl_ora8_inout_stream::operator>>(tab);
  return *this;
 }

 otl_inout_stream& operator<<(otl_pl_tab_generic& tab)
 {
  otl_ora8_inout_stream::operator<<(tab);
  return *this;
 }


 otl_inout_stream& operator>>(otl_time0& s)
 {
  otl_ora8_inout_stream::operator>>(s);
  return *this;
 }

 otl_inout_stream& operator<<(const otl_time0& s)
 {
  otl_ora8_inout_stream::operator<<(s);
  return *this;
 }

 otl_inout_stream& operator>>(char& c)
 {
  otl_ora8_inout_stream::operator>>(c);
  return *this;
 }

 otl_inout_stream& operator<<(const char c)
 {
  otl_ora8_inout_stream::operator<<(c);
  return *this;
 }


 otl_inout_stream& operator>>(unsigned char& c)
 {
  otl_ora8_inout_stream::operator>>(c);
  return *this;
 }

#if defined(OTL_UNICODE_STRING_TYPE)
 otl_inout_stream& operator>>(OTL_UNICODE_STRING_TYPE& s)
 {
  otl_ora8_inout_stream::operator>>(s);
  return *this;
 }

 otl_inout_stream& operator<<(const OTL_UNICODE_STRING_TYPE& s)
 {
  otl_ora8_inout_stream::operator<<(s);
  return *this;
 }
#endif

 otl_inout_stream& operator<<(const unsigned char c)
 {
  otl_ora8_inout_stream::operator<<(c);
  return *this;
 }


 otl_inout_stream& operator>>(char* s)
 {
  otl_ora8_inout_stream::operator>>(s);
  return *this;
 }

 otl_inout_stream& operator<<(const char* s)
 {
  otl_ora8_inout_stream::operator<<(s);
  return *this;
 }

 otl_inout_stream& operator>>(unsigned char* s)
 {
  otl_ora8_inout_stream::operator>>(s);
  return *this;
 }

 otl_inout_stream& operator<<(const unsigned char* s)
 {
  otl_ora8_inout_stream::operator<<(s);
  return *this;
 }

#if defined(OTL_BIGINT) && (defined(OTL_ORA11G_R2)&&!defined(OTL_STR_TO_BIGINT)&&\
    !defined(OTL_BIGINT_TO_STR))
 otl_inout_stream& operator>>(OTL_BIGINT& n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator>>(n);
#else
  otl_ora8_inout_stream::operator>><OTL_BIGINT,otl_var_bigint>(n);
#endif
  return *this;
 }
#endif

#if defined(OTL_UBIGINT) && defined(OTL_ORA11G_R2)
 otl_inout_stream& operator>>(OTL_UBIGINT& n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator>>(n);
#else
  otl_ora8_inout_stream::operator>><OTL_UBIGINT,otl_var_ubigint>(n);
#endif
  return *this;
 }
#endif

 otl_inout_stream& operator>>(int& n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator>>(n);
#else
  otl_ora8_inout_stream::operator>><int,otl_var_int>(n);
#endif
  return *this;
 }

#if defined(OTL_BIGINT) && (defined(OTL_ORA11G_R2)&&!defined(OTL_STR_TO_BIGINT)&&\
    !defined(OTL_BIGINT_TO_STR))
 otl_inout_stream& operator<<(const OTL_BIGINT n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
   otl_ora8_inout_stream::operator<<(n);
#else
  otl_ora8_inout_stream::operator<<<OTL_BIGINT,otl_var_bigint>(n);
#endif
  return *this;
 }
#endif

#if defined(OTL_UBIGINT) && defined(OTL_ORA11G_R2)
 otl_inout_stream& operator<<(const OTL_UBIGINT n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
   otl_ora8_inout_stream::operator<<(n);
#else
  otl_ora8_inout_stream::operator<<<OTL_UBIGINT,otl_var_ubigint>(n);
#endif
  return *this;
 }
#endif

 otl_inout_stream& operator<<(const int n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
   otl_ora8_inout_stream::operator<<(n);
#else
  otl_ora8_inout_stream::operator<<<int,otl_var_int>(n);
#endif
  return *this;
 }

 otl_inout_stream& operator>>(float& n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator>>(n);
#else
  otl_ora8_inout_stream::operator>><float,otl_var_float>(n);
#endif
  return *this;
 }

 otl_inout_stream& operator<<(const float n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator<<(n);
#else
  otl_ora8_inout_stream::operator<<<float,otl_var_float>(n);
#endif
  return *this;
 }

 otl_inout_stream& operator>>(double& n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator>>(n);
#else
  otl_ora8_inout_stream::operator>><double,otl_var_double>(n);
#endif
  return *this;
 }

 otl_inout_stream& operator<<(const double n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator<<(n);
#else
  otl_ora8_inout_stream::operator<<<double,otl_var_double>(n);
#endif
  return *this;
 }


 otl_inout_stream& operator>>(short int& n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator>>(n);
#else
  otl_ora8_inout_stream::operator>><short,otl_var_short>(n);
#endif
  return *this;
 }

 otl_inout_stream& operator<<(const short int n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator<<(n);
#else
  otl_ora8_inout_stream::operator<<<short,otl_var_short>(n);
#endif
  return *this;
 }

 otl_inout_stream& operator>>(unsigned int& n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator>>(n);
#else
  otl_ora8_inout_stream::operator>><unsigned,otl_var_unsigned_int>(n);
#endif
  return *this;
 }

 otl_inout_stream& operator<<(const unsigned int n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator<<(n);
#else
  otl_ora8_inout_stream::operator<<<unsigned,otl_var_unsigned_int>(n);
#endif
  return *this;
 }


 otl_inout_stream& operator>>(long int& n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator>>(n);
#else
  otl_ora8_inout_stream::operator>><long,otl_var_long_int>(n);
#endif
  return *this;
 }

 otl_inout_stream& operator<<(const long int n)
 {
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  otl_ora8_inout_stream::operator<<(n);
#else
  otl_ora8_inout_stream::operator<<<long,otl_var_long_int>(n);
#endif
  return *this;
 }

 otl_inout_stream& operator>>(otl_long_string& n)
 {
  otl_ora8_inout_stream::operator>>(n);
  return *this;
 }

 otl_inout_stream& operator<<(const otl_long_string& n)
 {
  otl_ora8_inout_stream::operator<<(n);
  return *this;
 }

 otl_inout_stream& operator>>(otl_lob_stream& n)
 {
  otl_ora8_inout_stream::operator>>(n);
  return *this;
 }

 otl_inout_stream& operator<<(otl_lob_stream& s)
 {
  otl_ora8_inout_stream::operator<<(s);
  return *this;
 }

protected:

 otl_connect* adb2;

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_inout_stream(const otl_inout_stream&) = delete;
  otl_inout_stream& operator=(const otl_inout_stream&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_inout_stream(otl_inout_stream&&) = delete;
  otl_inout_stream& operator=(otl_inout_stream&&) = delete;
#endif

private:
#else
  otl_inout_stream(const otl_inout_stream&):
    otl_ora8_inout_stream(),
    adb2(nullptr)
 {
 }

 otl_inout_stream& operator=(const otl_inout_stream&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_inout_stream(otl_inout_stream&&):
    otl_ora8_inout_stream(),
    adb2(nullptr)
 {
 }

 otl_inout_stream& operator=(otl_inout_stream&&)
 {
   return *this;
 }
#endif

#endif

};


// ============ OTL Reference Cursor Streams for Oracle 8 =================

class otl_cur;

class otl_ref_cursor: public
 otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var> {

protected:


  friend class otl_cur;
  int cur_row;
  int cur_size;
  int row_count;
  int array_size;
  otl_select_struct_override local_override;

public:

 otl_ref_cursor
 (otl_connect& db,
  const char* cur_placeholder_name,
  void* master_stream_ptr,
  const otl_stream_buffer_size_type arr_size=1)
  :otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(db),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(arr_size),
   local_override(),
   sel_cur(),
   rvl_len(otl_var_list_size),
   rvl(new otl_p_generic_variable[rvl_len]),
   vl_cur_len(0),
   cur_placeholder(),
   master_stream_ptr_(master_stream_ptr)
 {int i;
  local_override.reset();
  for(i=0;i<rvl_len;++i)
    rvl[i]=nullptr;
  OTL_STRCPY_S(cur_placeholder,sizeof(cur_placeholder),cur_placeholder_name);
 }

 otl_ref_cursor():
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(0),
   local_override(),
   sel_cur(),
   rvl_len(0),
   rvl(nullptr),
   vl_cur_len(0),
  cur_placeholder(),
  master_stream_ptr_(nullptr)
 {
   local_override.reset();
 }

 virtual ~otl_ref_cursor()
 {
  delete[] rvl;
  rvl=nullptr;
 }

 void open
 (otl_connect& db,
  const char* cur_placeholder_name,
  const otl_stream_buffer_size_type arr_size=1)
 {int i;
  local_override.reset();
  cur_row=-1;
  row_count=0;
  cur_size=0;
  array_size=arr_size;
  rvl_len=otl_var_list_size;
  vl_cur_len=0;
  rvl=new otl_p_generic_variable[rvl_len];
  for(i=0;i<rvl_len;++i)rvl[i]=nullptr;
  OTL_STRCPY_S(cur_placeholder,sizeof(cur_placeholder),cur_placeholder_name);
  if(!sel_cur.get_connected())sel_cur.open(db);
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::open(db);
 }

 void release_sel_cur(void)
 {
#if defined(OTL_ORA8_8I_REFCUR)
  return;
#else
  char tmp_buf[256];
  OCIBind* bindpp;
  int rc;

  if(!sel_cur.get_connected())return;
  OTL_STRCPY_S(tmp_buf,sizeof(tmp_buf),"begin close ");
  OTL_STRCAT_S(tmp_buf,sizeof(tmp_buf),cur_placeholder);
  OTL_STRCAT_S(tmp_buf,sizeof(tmp_buf),"; end;");
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::parse(tmp_buf);
  rc=OCIBindByName
   (cursor_struct.cda,
    &bindpp,
    cursor_struct.errhp,
    OTL_RCAST(text*,cur_placeholder),
    OTL_SCAST(sb4,strlen(cur_placeholder)),
    OTL_RCAST(dvoid*,&sel_cur.get_cursor_struct().cda),
    0,
    SQLT_RSET,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    OTL_SCAST(ub4,OCI_DEFAULT));
  if(rc!=0){
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return;
   if(otl_uncaught_exception()) return;
   throw otl_exception(cursor_struct,
                       stm_label?stm_label:stm_text);
  }
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
     otl_var>::exec(1,0,otl_sql_exec_from_select_cursor_class);
#endif
 }

 void close(void)
 {
   local_override.reset();
   delete[] rvl;
   rvl=nullptr;
   release_sel_cur();
   sel_cur.close();
   otl_tmpl_cursor
     <otl_exc,
     otl_conn,
     otl_cur,
     otl_var>::close();
 }

 int first(void)
 {int i,rc;
  OCIBind* bindpp;

  if(!sel_cur.get_connected()){
   sel_cur.open(*adb);
   rc=OCIBindByName
    (cursor_struct.cda,
     &bindpp,
     cursor_struct.errhp,
     OTL_RCAST(text*,cur_placeholder),
     OTL_SCAST(sb4,strlen(cur_placeholder)),
     OTL_RCAST(dvoid*,&sel_cur.get_cursor_struct().cda),
     0,
     SQLT_RSET,
     nullptr,
     nullptr,
     nullptr,
     0,
     nullptr,
     OTL_SCAST(ub4,OCI_DEFAULT));
   if(rc!=0){
     if(this->adb)this->adb->increment_throw_count();
     if(this->adb&&this->adb->get_throw_count()>1)return 0;
     if(otl_uncaught_exception()) return 0; 
     throw otl_exception(cursor_struct,
                         stm_label?stm_label:stm_text);
   }
  }

  if(cur_row==-2)
   ; // Special case -- calling describe_select() between parse() and first()
  else{
// Executing the PLSQL master block
    exec(1,0,otl_sql_exec_from_select_cursor_class); 
   sel_cur.set_connected(1);
  }
  cur_row=-1;
  for(i=0;i<vl_cur_len;++i)
   sel_cur.bind(i+1,*rvl[i]);
  rc=sel_cur.get_cursor_struct().fetch
    (OTL_SCAST(otl_stream_buffer_size_type,
               array_size),sel_cur.get_eof_data_ref());
  if(rc==0){
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return 0;
    if(otl_uncaught_exception()) return 0; 
    throw otl_exception(sel_cur.get_cursor_struct(),
                        stm_label?stm_label:stm_text);
  }
  row_count=sel_cur.get_cursor_struct().rpc();
  OTL_TRACE_FIRST_FETCH
  cur_size=row_count;
  if(cur_size!=0)cur_row=0;
  return cur_size!=0;
 }

 int next(void)
 {int rc;
  if(cur_row<0)return first();
  if(cur_row<cur_size-1)
   ++cur_row;
  else{
   if(sel_cur.eof()){
     cur_row=-1;
     return 0;
   }
   rc=sel_cur.get_cursor_struct().fetch
     (OTL_SCAST(otl_stream_buffer_size_type,
                array_size),sel_cur.get_eof_data_ref());
   if(rc==0){
     if(this->adb)this->adb->increment_throw_count();
     if(this->adb&&this->adb->get_throw_count()>1)return 0;
     if(otl_uncaught_exception()) return 0; 
     throw otl_exception(sel_cur.get_cursor_struct(),
                         stm_label?stm_label:stm_text);
   }
   cur_size=sel_cur.get_cursor_struct().rpc()-row_count;
   row_count=sel_cur.get_cursor_struct().rpc();
   OTL_TRACE_NEXT_FETCH2
   if(cur_size!=0)cur_row=0;
  }
  return cur_size!=0;
 }

 void bind
 (const int column_num,
  otl_generic_variable& v)
 {
  if(!connected)return;
  ++vl_cur_len;
  if(vl_cur_len==rvl_len){
    int temp_rvl_len=rvl_len*2;
    otl_p_generic_variable* temp_rvl=
      new otl_p_generic_variable[temp_rvl_len];
    int i;
    for(i=0;i<rvl_len;++i)
      temp_rvl[i]=rvl[i];
    for(i=rvl_len+1;i<temp_rvl_len;++i)
      temp_rvl[i]=nullptr;
    delete[] rvl;
    rvl=temp_rvl;
    rvl_len=temp_rvl_len;
  }
  rvl[vl_cur_len-1]=&v;
  v.set_pos(column_num);
 }

 void bind(otl_generic_variable& v)
 {
   if(v.get_pos())
     bind(v.get_pos(),v);
   else if(v.get_name())
   otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::bind(v);
 }

 void bind
 (const char* name,
  otl_generic_variable& v)
 {
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>::bind(name,v);
 }

 int describe_select
 (otl_column_desc* desc,
  int& desc_len)
 {int i,rc;
  OCIBind* bindpp;

  if(!sel_cur.get_connected()){
   sel_cur.open(*adb);
   rc=OCIBindByName
    (cursor_struct.cda,
     &bindpp,
     cursor_struct.errhp,
     OTL_RCAST(text*,cur_placeholder),
     OTL_SCAST(sb4,strlen(cur_placeholder)),
     OTL_RCAST(dvoid*,&sel_cur.get_cursor_struct().cda),
     0,
     SQLT_RSET,
     nullptr,
     nullptr,
     nullptr,
     0,
     nullptr,
     OTL_SCAST(ub4,OCI_DEFAULT));
   if(rc!=0){
     if(this->adb)this->adb->increment_throw_count();
     if(this->adb&&this->adb->get_throw_count()>1)return 0;
     if(otl_uncaught_exception()) return 0; 
     throw otl_exception(cursor_struct,
                         stm_label?stm_label:stm_text);
   }
  }
// Executing the PLSQL master block
  exec(1,0,otl_sql_exec_from_select_cursor_class); 
  sel_cur.set_connected(1);
  cur_row=-2; // Special case -- describe_select() before first() or next()
  desc_len=0;
  sel_cur.get_cursor_struct().straight_select=0;
  for(i=1;sel_cur.describe_column(desc[i-1],i);++i)
   ++desc_len;
  return 1;
 }

public:

 otl_cursor sel_cur;

protected:

  int rvl_len;
  otl_p_generic_variable* rvl;
  int vl_cur_len;
  char cur_placeholder[64];
  void* master_stream_ptr_;

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_ref_cursor(const otl_ref_cursor&) = delete;
 otl_ref_cursor& operator=(const otl_ref_cursor&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_ref_cursor(otl_ref_cursor&&) = delete;
 otl_ref_cursor& operator=(otl_ref_cursor&&) = delete;
#endif
private:
#else
 otl_ref_cursor(const otl_ref_cursor&)
  :otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(0),
   local_override(),
   sel_cur(),
   rvl_len(0),
   rvl(nullptr),
   vl_cur_len(0),
   cur_placeholder(),
   master_stream_ptr_(nullptr)
 {
 }

 otl_ref_cursor& operator=(const otl_ref_cursor&)
 {
   return *this;
 }


#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_ref_cursor(otl_ref_cursor&&)
  :otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
   otl_var>(),
   cur_row(-1),
   cur_size(0),
   row_count(0),
   array_size(0),
   local_override(),
   sel_cur(),
   rvl_len(0),
   rvl(nullptr),
   vl_cur_len(0),
   cur_placeholder(),
   master_stream_ptr_(nullptr)
 {
 }

 otl_ref_cursor& operator=(otl_ref_cursor&&)
 {
   return *this;
 }
#endif
#endif

};

class otl_ref_select_stream: public otl_ref_cursor{

protected:

 otl_select_struct_override *override_;
 int delay_next;
 int same_sl_flag;
 long _rfc;

public:

  void skip_to_end_of_row()
  {
    check_if_executed();
    if(eof())return;
    while(cur_col<sl_len-1){
      ++cur_col;
      null_fetched=sl[cur_col].is_null(this->cur_row);
    }
    ret_code=this->next();
    cur_col=0;
    if(!eof())
      cur_col=-1;
    ++_rfc;
  }


  int get_select_row_count() const 
  {
    return this->cur_row==-1?0:this->cur_size-this->cur_row;
  }

  int get_prefetched_row_count() const {return this->row_count;}

  long get_rfc() const {return _rfc;}

 void cleanup(void)
 {int i;
  delete[] sl;
  for(i=0;i<vl_len;++i)
   delete vl[i];
  delete[] vl;
  delete[] sl_desc;
 }

 otl_ref_select_stream
 (otl_select_struct_override* aoverride,
  const otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  const char* acur_placeholder,
  otl_connect& db,
  const char* sqlstm_label=nullptr)
   :otl_ref_cursor
    (db,
     acur_placeholder,
     aoverride->get_master_stream_ptr(),
     arr_size),
    override_(nullptr),
    delay_next(0),
    same_sl_flag(0),
    _rfc(0),
    sl_desc(nullptr),
    sl_len(0),
    sl(nullptr),
    null_fetched(0),
    ret_code(0),
    cur_col(0),
    cur_in(0),
    executed(0),
    var_info()
 {
   if(sqlstm_label!=nullptr){
     if(stm_label!=nullptr){
       delete[] stm_label;
       stm_label=nullptr;
     }
     size_t len=strlen(sqlstm_label)+1;
     stm_label=new char[len];
     OTL_STRCPY_S(stm_label,len,sqlstm_label);
   }

  init();

  override_=aoverride;
  {
   size_t len=strlen(sqlstm)+1;
   stm_text=new char[len];
   OTL_STRCPY_S(stm_text,len,sqlstm);
   otl_select_struct_override* temp_local_override=&this->local_override;
   otl_ext_hv_decl hvd
     (this->stm_text,
      1,
      this->stm_label,
      &temp_local_override,
      adb
     );
   hvd.alloc_host_var_list(vl,vl_len,*adb);
   if(temp_local_override!=&this->local_override)
     delete temp_local_override;
  }

  try{

   parse();
   if(vl_len==0){
    rewind();
    null_fetched=0;
   }
  }catch(OTL_CONST_EXCEPTION otl_exception&){
   cleanup();
   if(this->adb)this->adb->increment_throw_count();
   throw;
  }

 }

 virtual ~otl_ref_select_stream()
 {
  cleanup();
  close();
 }

 void rewind(void)
 {
   OTL_TRACE_STREAM_EXECUTION
   _rfc=0;
   get_select_list();
   ret_code=first();
   null_fetched=0;
   cur_col=-1;
   cur_in=0;
   executed=1;
   delay_next=0;
 }
  
  void clean(void)
  {
    _rfc=0;
    null_fetched=0;
    cur_col=-1;
    cur_in=0;
    executed=0;
    delay_next=0;
  }

 int is_null(void)
 {
  return null_fetched;
 }

 int eof(void)
 {
#if defined(OTL_SELECT_STREAM_ALTERNATE_FETCH)
   if(cur_col==sl_len-1){
     get_next();
     cur_col=-1;
   }else{
     if(delay_next){
       look_ahead();
       delay_next=0;
     }
   }
  return !ret_code;
#else
  if(delay_next){
   look_ahead();
   delay_next=0;
  }
  return !ret_code;
#endif
 }

 int eof_intern(void)
 {
  return !ret_code;
 }


 otl_ref_select_stream& operator>>(otl_time0& t)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_timestamp)&&!eof_intern()){
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
   void* tm=OTL_RCAST(void*,sl[cur_col].val(this->cur_row));
   int rc=sl[cur_col].get_var_struct().read_dt(&t,tm,sizeof(otl_time0));
   if(rc==0){
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return *this;
    if(otl_uncaught_exception()) return *this; 
    throw otl_exception(adb->get_connect_struct(),
                        stm_label?stm_label:stm_text);
   }
#else
   otl_time0* tm=OTL_RCAST(otl_time0*,sl[cur_col].val(cur_row));
   memcpy(OTL_RCAST(void*,&t),tm,otl_oracle_date_size);
#endif
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator<<(const otl_time0& t)
 {
  check_in_var();
  if(check_in_type(otl_var_timestamp,otl_oracle_date_size)){
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
   void* tm=OTL_RCAST(void*,vl[cur_in]->val());
   int rc=vl[cur_in]->get_var_struct().write_dt(tm,&t,sizeof(otl_time0));
   if(rc==0){
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return *this;
    if(otl_uncaught_exception()) return *this; 
    throw otl_exception(adb->get_connect_struct(),
                        stm_label?stm_label:stm_text);
   }
#else
   otl_time0* tm=OTL_RCAST(otl_time0*,vl[cur_in]->val());
   memcpy(tm,OTL_RCAST(void*,OTL_CCAST(otl_time0*,&t)),otl_oracle_date_size);
#endif
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const otl_long_string& s)
 {
   if(s.get_unicode_flag()){
     throw otl_exception
       (otl_error_msg_37,
        otl_error_code_37,
        this->stm_label?this->stm_label:
        this->stm_text);
   }
   check_in_var();
   if(check_in_type(otl_var_raw,1)){
      unsigned char* c=OTL_RCAST(unsigned char*,vl[cur_in]->val());
      int len=OTL_CCAST(otl_long_string*,&s)->len();
      if(len>this->vl[cur_in]->actual_elem_size()){
        otl_var_info_var
          (this->vl[cur_in]->get_name(),
           this->vl[cur_in]->get_ftype(),
           otl_var_raw,
           var_info,
           sizeof(var_info));
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
        throw otl_exception
          (otl_error_msg_5,
           otl_error_code_5,
           this->stm_label?this->stm_label:
           this->stm_text,
           var_info);
      }
      this->vl[cur_in]->set_not_null(0);
      otl_memcpy
        (c+sizeof(unsigned short),
         s.v,
         len,
         this->vl[cur_in]->get_ftype());
      *OTL_RCAST(unsigned short*,
                 this->vl[cur_in]->val(0))=OTL_SCAST(unsigned short,len);
      this->vl[cur_in]->set_len(len,0);
   }
   get_in_next();
   return *this;
 }



 otl_ref_select_stream& operator>>(char& c)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   c=*OTL_RCAST(char*,sl[cur_col].val(cur_row));
   look_ahead();
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(unsigned char& c)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   c=*OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
   look_ahead();
  }
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_ref_select_stream& operator>>(OTL_STRING_CONTAINER& s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  switch(sl[cur_col].get_ftype()){
  case otl_var_char:
    if(!eof_intern()){
#if defined(OTL_ACE)
      s.set(OTL_RCAST(char*,sl[cur_col].val(cur_row)),1);
#else
      s=OTL_RCAST(char*,sl[cur_col].val(cur_row));
#endif
      look_ahead();
    }
    break;
#if defined(USER_DEFINED_STRING_CLASS) || \
    defined(OTL_STL) || defined(OTL_ACE)
  case otl_var_varchar_long:
  case otl_var_raw_long:
    if(!eof_intern()){
      unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
      int len=sl[cur_col].get_len(cur_row);
#if (defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)) && !defined(OTL_ACE)
      s.assign(OTL_RCAST(char*,c),len);
#elif defined(OTL_ACE)
      s.set(OTL_RCAST(char*,c),len,1);
#endif
      look_ahead();
    }
    break;
  case otl_var_raw:
    {
      if(!eof_intern()){
        unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
        int len=OTL_SCAST(int,*OTL_RCAST(unsigned short*,c));
        c+=sizeof(short int);
#if (defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)) && !defined(OTL_ACE)
        s.assign(OTL_RCAST(char*,c),len);
#elif defined(OTL_ACE)
        s.set(OTL_RCAST(char*,c),len,1);
#endif
        look_ahead();
      }  
    }    
    break;
  case otl_var_blob:
  case otl_var_clob:
    if(!eof_intern()){
      int len=0;
      int max_long_sz=this->adb->get_max_long_size();
      otl_auto_array_ptr<unsigned char> loc_ptr(max_long_sz);
      unsigned char* temp_buf=loc_ptr.get_ptr();
    
      int rc=sl[cur_col].get_var_struct().get_blob
        (cur_row,
         temp_buf,
         max_long_sz,
         len);
      if(rc==0){
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count()>1)return *this;
        if(otl_uncaught_exception()) return *this; 
        throw otl_exception(adb->get_connect_struct(),
                            stm_label?stm_label:stm_text);
      }
#if (defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)) && !defined(OTL_ACE)
      s.assign(OTL_RCAST(char*,temp_buf),len);
#elif defined(OTL_ACE)
      s.set(OTL_RCAST(char*,temp_buf),len,1);
#endif
      look_ahead();
    }
    break;
#endif
  default:
    check_type(otl_var_char);
  } // switch
  return *this;
 }
#endif

 otl_ref_select_stream& operator>>(char* s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   otl_strcpy(OTL_RCAST(unsigned char*,s),
              OTL_RCAST(const unsigned char*,sl[cur_col].val(cur_row)));
   look_ahead();
  }
  return *this;
 }

#if defined(OTL_UNICODE_STRING_TYPE)
 otl_ref_select_stream& operator>>(OTL_UNICODE_STRING_TYPE& s)
 {

    check_if_executed();
    if(eof_intern())return *this;
    get_next();
    switch(sl[cur_col].get_ftype()){
    case otl_var_char:
      if(!eof_intern()){

#if defined(OTL_UNICODE_STRING_TYPE_CAST_FROM_CHAR)
        OTL_UNICODE_CHAR_TYPE* temp_s=OTL_RCAST
          (OTL_UNICODE_CHAR_TYPE*,sl[cur_col].val(this->cur_row));
        OTL_UNICODE_STRING_TYPE_CAST_FROM_CHAR(s,temp_s+1,*temp_s);
#else
        OTL_UNICODE_CHAR_TYPE* temp_s=OTL_RCAST
          (OTL_UNICODE_CHAR_TYPE*,sl[cur_col].val(this->cur_row));
        s.assign(temp_s+1,*temp_s);
#endif

        look_ahead();
      }
      break;
    case otl_var_varchar_long:
      if(!eof_intern()){
        s=OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,sl[cur_col].val(this->cur_row));
        look_ahead();
      }
      break;
    case otl_var_clob:
      if(!eof_intern()){
        int len=0;
        int max_long_sz=this->adb->get_max_long_size();
        otl_auto_array_ptr<unsigned short> loc_ptr(max_long_sz);
        unsigned char* temp_buf=OTL_RCAST(unsigned char*,loc_ptr.get_ptr());

        int rc=sl[cur_col].get_var_struct().get_blob
          (this->cur_row,
           temp_buf,
           max_long_sz,
           len);
        if(rc==0){
          if(this->adb)this->adb->increment_throw_count();
          if(this->adb&&this->adb->get_throw_count()>1)return *this;
          if(otl_uncaught_exception()) return *this; 
          throw otl_exception
            (this->adb->get_connect_struct(),
             this->stm_label?this->stm_label:
             this->stm_text);
        }
        s=OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,temp_buf);
        look_ahead();
      }
      break;
    default:
      check_type(otl_var_char);
    }
    return *this;
 }
#endif

 otl_ref_select_stream& operator>>(unsigned char* s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if(check_type(otl_var_char)&&!eof_intern()){
   otl_strcpy2(OTL_RCAST(unsigned char*,s),
               OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row)),
               sl[cur_col].get_len(cur_row)
             );
   look_ahead();
  }
  return *this;
 }

#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
#define OTL_D7(T,T_type)                                \
  otl_ref_select_stream& operator>>(T& n)               \
  {                                                     \
    check_if_executed();                                \
    if(eof_intern())return *this;                       \
    get_next();                                         \
    if(!eof_intern()){                                  \
      int match_found=otl_numeric_convert_T<T,T_type>   \
        (sl[cur_col].get_ftype(),                       \
         sl[cur_col].val(cur_row),                      \
         n);                                            \
      if(!match_found)                                  \
        strict_check_throw(T_type);                     \
      look_ahead();                                     \
    }                                                   \
    return *this;                                       \
  }
#else
#define OTL_D7(T,T_type)                                        \
  otl_ref_select_stream& operator>>(T& n)                       \
  {                                                             \
    check_if_executed();                                        \
    if(eof_intern())return *this;                               \
    get_next();                                                 \
    if(!eof_intern()){                                          \
      int match_found=otl_numeric_convert_T                     \
        (sl[cur_col].get_ftype(),                               \
         sl[cur_col].val(cur_row),                              \
         n);                                                    \
      if(!match_found){                                         \
        if(check_type(otl_var_double,T_type))                   \
          n=OTL_PCONV(T,double,sl[cur_col].val(cur_row));       \
      }                                                         \
      look_ahead();                                             \
    }                                                           \
    return *this;                                               \
  }
#endif

#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  OTL_D7(int,otl_var_int)
  OTL_D7(unsigned,otl_var_unsigned_int)
  OTL_D7(long,otl_var_long_int)
  OTL_D7(short,otl_var_short)
  OTL_D7(float,otl_var_float)
  OTL_D7(double,otl_var_double)
#if defined(OTL_BIGINT) && (defined(OTL_ORA11G_R2)&&!defined(OTL_STR_TO_BIGINT)&&\
    !defined(OTL_BIGINT_TO_STR))
  OTL_D7(OTL_BIGINT,otl_var_bigint)
#endif
#if defined(OTL_UBIGINT) && defined(OTL_ORA11G_R2)
  OTL_D7(OTL_UBIGINT,otl_var_ubigint)
#endif
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
  OTL_D7(OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1)
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
  OTL_D7(OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2)
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
  OTL_D7(OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3)
#endif
#else
  template<OTL_TYPE_NAME T,const int T_type> OTL_D7(T,T_type)
#endif

 otl_ref_select_stream& operator>>(otl_long_string& s)
 {
   check_if_executed();
   if(eof_intern())return *this;
   get_next();
   switch(sl[cur_col].get_ftype()){
   case otl_var_varchar_long:
   case otl_var_raw_long:
   {
     bool in_unicode_mode=sizeof(OTL_CHAR)>1;
     if(!s.get_unicode_flag() && in_unicode_mode &&
        sl[cur_col].get_ftype()==otl_var_varchar_long){
       throw otl_exception
         (otl_error_msg_37,
          otl_error_code_37,
          this->stm_label?this->stm_label:
          this->stm_text);
     }else if(s.get_unicode_flag() && sl[cur_col].get_ftype()==otl_var_raw_long){
       throw otl_exception
         (otl_error_msg_38,
          otl_error_code_38,
          this->stm_label?this->stm_label:
          this->stm_text);
     }
     if(!eof_intern()){
       unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
       int len=sl[cur_col].get_len(cur_row);
       if(len>s.get_buf_size())
          len=s.get_buf_size();
        otl_memcpy(s.v,c,len,sl[cur_col].get_ftype());
        s.v[len]=0;
        s.set_len(len);
        look_ahead();
      }  
    }    
    break;
  case otl_var_raw:
    {
      if(s.get_unicode_flag()){
        throw otl_exception
          (otl_error_msg_38,
           otl_error_code_38,
           this->stm_label?this->stm_label:
           this->stm_text);
      }
      if(!eof_intern()){
        unsigned char* c=OTL_RCAST(unsigned char*,sl[cur_col].val(cur_row));
        int len=OTL_SCAST(int,*OTL_RCAST(unsigned short*,c));
        if(len>s.get_buf_size())
          len=s.get_buf_size();
        otl_memcpy(s.v,c+sizeof(short int),len,sl[cur_col].get_ftype());
        s.set_len(len);
        look_ahead();
      }  
    }    
    break;
  case otl_var_blob:
  case otl_var_clob:
    {
      bool in_unicode_mode=sizeof(OTL_CHAR)>1;
      if(!s.get_unicode_flag() && in_unicode_mode &&
         sl[cur_col].get_ftype()==otl_var_clob){
        throw otl_exception
          (otl_error_msg_37,
           otl_error_code_37,
           this->stm_label?this->stm_label:
           this->stm_text);
      }else if(s.get_unicode_flag() && sl[cur_col].get_ftype()==otl_var_blob){
        throw otl_exception
          (otl_error_msg_38,
           otl_error_code_38,
           this->stm_label?this->stm_label:
           this->stm_text);
      }
      if(!eof_intern()){
        int len;
        int rc=sl[cur_col].get_var_struct().get_blob
          (cur_row,s.v,s.get_buf_size(),len);
        if(rc==0){
          if(this->adb)this->adb->increment_throw_count();
          if(this->adb&&this->adb->get_throw_count()>1)return *this;
          if(otl_uncaught_exception()) return *this; 
          throw otl_exception(adb->get_connect_struct(),
                              stm_label?stm_label:stm_text);
        }
        s.set_len(len);
        if(sl[cur_col].get_ftype()==otl_var_clob)
          s.null_terminate_string(len);
        look_ahead();
      }
    }
    break;
  }
  return *this;
 }

 otl_ref_select_stream& operator>>(otl_lob_stream& s)
 {
  check_if_executed();
  if(eof_intern())return *this;
  get_next();
  if((sl[cur_col].get_ftype()==otl_var_blob||
      sl[cur_col].get_ftype()==otl_var_clob)&&
     !eof_intern()){
   s.init
    (&sl[cur_col],
     adb,
     OTL_RCAST(otl_ref_cursor*,this),
     cur_row,
     otl_lob_stream_read_mode,
     this->is_null());
   delay_next=1;
  }
  return *this;
 }

  otl_ref_select_stream& operator<<(const otl_null& /*n*/)
  {
    check_in_var();
    this->vl[cur_in]->set_null(0);
    get_in_next();
    return *this;
  }

 otl_ref_select_stream& operator<<(const char c)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){
   char* tmp=OTL_RCAST(char*,vl[cur_in]->val());
   tmp[0]=c;
   tmp[1]=0;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const unsigned char c)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){
   unsigned char* tmp=OTL_RCAST(unsigned char*,vl[cur_in]->val());
   tmp[0]=c;
   tmp[1]=0;
  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_ref_select_stream& operator<<(const OTL_STRING_CONTAINER& s)
 {
  check_in_var();
  if(this->vl[cur_in]->get_ftype()==otl_var_raw){
    unsigned char* c=OTL_RCAST(unsigned char*,vl[cur_in]->val());
    int len=OTL_SCAST(int,s.length());
    if(len>this->vl[cur_in]->actual_elem_size()){
      otl_var_info_var
        (this->vl[cur_in]->get_name(),
         this->vl[cur_in]->get_ftype(),
         otl_var_raw,
         var_info,
         sizeof(var_info));
      if(this->adb)this->adb->increment_throw_count();
      if(this->adb&&this->adb->get_throw_count()>1)return *this;
      if(otl_uncaught_exception()) return *this; 
      throw otl_exception
        (otl_error_msg_5,
         otl_error_code_5,
         this->stm_label?this->stm_label:
         this->stm_text,
         var_info);
    }
    this->vl[cur_in]->set_not_null(0);
    otl_memcpy
      (c+sizeof(unsigned short),
       OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
       len,
       this->vl[cur_in]->get_ftype());
    *OTL_RCAST(unsigned short*,
               this->vl[cur_in]->val(0))=OTL_SCAST(unsigned short,len);
    this->vl[cur_in]->set_len(len,0);
  }else if(this->vl[cur_in]->get_ftype()==otl_var_char){
   int overflow;
   otl_strcpy
    (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,s.c_str())),
     overflow,
     vl[cur_in]->get_elem_size(),
     OTL_SCAST(int,s.length())
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
      (vl[cur_in]->get_name(),
       vl[cur_in]->get_ftype(),
       otl_var_char,
       temp_var_info,
       sizeof(temp_var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return *this;
    if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
    throw otl_exception
      (otl_error_msg_4,
       otl_error_code_4,
       stm_label?stm_label:stm_text,
       temp_var_info,
       OTL_RCAST(const void*,s.c_str()),
       OTL_SCAST(int,vl[cur_in]->get_elem_size()));
#else
    throw otl_exception
     (otl_error_msg_4,
      otl_error_code_4,
      stm_label?stm_label:stm_text,
      temp_var_info);
#endif
   }

  }else
    check_in_type_throw(otl_var_char);
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }
#endif

#if defined(OTL_UNICODE_STRING_TYPE)
  otl_ref_select_stream& operator<<(const OTL_UNICODE_STRING_TYPE& s)
  {
    check_in_var();
    if(check_in_type(otl_var_char,1)){
      
      int overflow;
#if defined(OTL_C_STR_FOR_UNICODE_STRING_TYPE)
      otl_strcpy4
        (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
         OTL_RCAST(unsigned char*,
                   OTL_CCAST(OTL_UNICODE_CHAR_TYPE*,s.OTL_C_STR_FOR_UNICODE_STRING_TYPE())),
#else
      otl_strcpy4
        (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
         OTL_RCAST(unsigned char*,
                   OTL_CCAST(OTL_UNICODE_CHAR_TYPE*,s.c_str())),
#endif
         overflow,
         vl[cur_in]->get_elem_size(),
         OTL_SCAST(int,s.length())
         );
      if(overflow){
        char temp_var_info[256];
        otl_var_info_var
          (vl[cur_in]->get_name(),
           vl[cur_in]->get_ftype(),
           otl_var_char,
           temp_var_info,
           sizeof(temp_var_info));
        if(this->adb)this->adb->increment_throw_count();
        if(this->adb&&this->adb->get_throw_count())return *this;
        if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
        throw otl_exception
          (otl_error_msg_4,
           otl_error_code_4,
           stm_label?stm_label:stm_text,
           temp_var_info,
           OTL_RCAST(const void*,s.c_str()),
           OTL_SCAST(int,vl[cur_in]->get_elem_size()));
#else
        throw otl_exception
          (otl_error_msg_4,
           otl_error_code_4,
           stm_label?stm_label:stm_text,
           temp_var_info);
#endif
      }
    }
    this->vl[cur_in]->set_not_null(0);
    get_in_next();
    return *this;
  }
#endif

 otl_ref_select_stream& operator<<(const char* s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy
    (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
     OTL_RCAST(unsigned char*,OTL_CCAST(char*,s)),
     overflow,
     vl[cur_in]->get_elem_size()
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
      (vl[cur_in]->get_name(),
       vl[cur_in]->get_ftype(),
       otl_var_char,
       temp_var_info,
       sizeof(temp_var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return *this;
    if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
    throw otl_exception
      (otl_error_msg_4,
       otl_error_code_4,
       stm_label?stm_label:stm_text,
       temp_var_info,
       OTL_RCAST(const void*,s),
       OTL_SCAST(int,vl[cur_in]->get_elem_size()));
#else
    throw otl_exception
     (otl_error_msg_4,
      otl_error_code_4,
      stm_label?stm_label:stm_text,
      temp_var_info);
#endif
   }

  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

 otl_ref_select_stream& operator<<(const unsigned char* s)
 {
  check_in_var();
  if(check_in_type(otl_var_char,1)){

   int overflow;
   otl_strcpy4
    (OTL_RCAST(unsigned char*,vl[cur_in]->val()),
     OTL_CCAST(unsigned char*,s),
     overflow,
     vl[cur_in]->get_elem_size()
    );
   if(overflow){
    char temp_var_info[256];
    otl_var_info_var
      (vl[cur_in]->get_name(),
       vl[cur_in]->get_ftype(),
       otl_var_char,
       temp_var_info,
       sizeof(temp_var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return *this;
    if(otl_uncaught_exception()) return *this; 
#if defined(OTL_EXCEPTION_COPIES_INPUT_STRING_IN_CASE_OF_OVERFLOW)
    throw otl_exception
      (otl_error_msg_4,
       otl_error_code_4,
       stm_label?stm_label:stm_text,
       temp_var_info,
       OTL_RCAST(const void*,s),
       OTL_SCAST(int,vl[cur_in]->get_elem_size()));
#else
    throw otl_exception
      (otl_error_msg_4,
       otl_error_code_4,
       stm_label?stm_label:stm_text,
       temp_var_info);
#endif
   }

  }
  this->vl[cur_in]->set_not_null(0);
  get_in_next();
  return *this;
 }

#define OTL_D8(T,T_type)                        \
  otl_ref_select_stream& operator<<(const T n)  \
  {                                             \
    check_in_var();                             \
    if(check_in_type(T_type,sizeof(T))){        \
      *OTL_RCAST(T*,vl[cur_in]->val())=n;       \
    }                                           \
    this->vl[cur_in]->set_not_null(0);          \
    get_in_next();                              \
    return *this;                               \
  }

#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
  OTL_D8(int,otl_var_int)
  OTL_D8(unsigned,otl_var_unsigned_int)
  OTL_D8(long,otl_var_long_int)
  OTL_D8(short,otl_var_short)
  OTL_D8(float,otl_var_float)
  OTL_D8(double,otl_var_double)
#if defined(OTL_BIGINT) && (defined(OTL_ORA11G_R2)&&!defined(OTL_STR_TO_BIGINT)&&\
    !defined(OTL_BIGINT_TO_STR))
  OTL_D8(OTL_BIGINT,otl_var_bigint)
#endif
#if defined(OTL_UBIGINT) && defined(OTL_ORA11G_R2)
  OTL_D8(OTL_UBIGINT,otl_var_ubigint)
#endif
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
  OTL_D8(OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1)
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
  OTL_D8(OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2)
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
  OTL_D8(OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3)
#endif
#else
  template<OTL_TYPE_NAME T,const int T_type> OTL_D8(T,T_type)
#endif

 int select_list_len(void) const 
 {
  return sl_len;
 }

 int column_ftype(int ndx=0) const
 {
   return sl[ndx].get_ftype();
 }

 int column_size(int ndx=0) const
 {
   return sl[ndx].get_elem_size();
 }

  int get_sl_len() const {return sl_len; }
  otl_generic_variable* get_sl() const {return sl;}
  otl_column_desc* get_sl_desc() const {return sl_desc;}

protected:

 otl_column_desc* sl_desc;
 int sl_len;
 otl_generic_variable* sl;

 int null_fetched;
 int ret_code;
 int cur_col;
 int cur_in;
 int executed;
 char var_info[256];

 void init(void)
 {
  same_sl_flag=0;
  sl=nullptr;
  sl_len=0;
  null_fetched=0;
  ret_code=0;
  sl_desc=nullptr;
  executed=0;
  cur_in=0;
  stm_text=nullptr;
 }

 void get_next(void)
 {
  if(cur_col<sl_len-1){
   ++cur_col;
   null_fetched=sl[cur_col].is_null(cur_row);
  }else{
   ret_code=next();
   cur_col=0;
  }
 }

  int check_type_throw(int type_code, int actual_data_type)
 {
    int out_type_code;
    if(actual_data_type!=0)
      out_type_code=actual_data_type;
    else
      out_type_code=type_code;
    otl_var_info_col
      (sl[cur_col].get_pos(),
       sl[cur_col].get_ftype(),
       out_type_code,
       var_info,
       sizeof(var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return 0;
  if(otl_uncaught_exception()) return 0; 

    throw otl_exception
      (otl_error_msg_0,
       otl_error_code_0,
       stm_label?stm_label:stm_text,
       var_info);
  }

  int check_type(int type_code, int actual_data_type=0)
  {
    switch(sl[cur_col].get_ftype()){
    case otl_var_timestamp:
    case otl_var_tz_timestamp:
    case otl_var_ltz_timestamp:
      if(type_code==otl_var_timestamp)
        return 1;
      break;
    default:
      if(sl[cur_col].get_ftype()==type_code)
        return 1;
      break;
    }
    return check_type_throw(type_code,actual_data_type);
  }

 void look_ahead(void)
 {
  if(cur_col==sl_len-1){
#if defined(OTL_SELECT_STREAM_ALTERNATE_FETCH)
#else
   ret_code=next();
   cur_col=-1;
#endif
   ++_rfc;
  }
 }

 void get_select_list(void)
 {int i,j,rc;

  otl_auto_array_ptr<otl_column_desc> loc_ptr(otl_var_list_size);
  otl_column_desc* sl_desc_tmp=loc_ptr.get_ptr();
  int sld_tmp_len=0;
  int ftype,elem_size;
  OCIBind* bindpp;

  if(!sel_cur.get_connected()){
   sel_cur.open(*adb);
   rc=OCIBindByName
    (cursor_struct.cda,
     &bindpp,
     cursor_struct.errhp,
     OTL_RCAST(text*,cur_placeholder),
     OTL_SCAST(sb4,strlen(cur_placeholder)),
     OTL_RCAST(dvoid*,&sel_cur.get_cursor_struct().cda),
     0,
     SQLT_RSET,
     nullptr,
     nullptr,
     nullptr,
     0,
     nullptr,
     OTL_SCAST(ub4,OCI_DEFAULT));
   if(rc!=0){
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw otl_exception(cursor_struct,
                       stm_label?stm_label:stm_text);
   }
  }

  for(i=0;i<vl_len;++i)
   otl_tmpl_cursor
    <otl_exc,
    otl_conn,
    otl_cur,
    otl_var>::bind(*vl[i]);
// Executing the PLSQL master block
  otl_tmpl_cursor
  <otl_exc,
   otl_conn,
   otl_cur,
     otl_var>::exec(1,0,otl_sql_exec_from_select_cursor_class); 
  sel_cur.set_connected(1);
  cur_row=-2;
  if(same_sl_flag && sl){
   // assuming that ref.cur's select list is the same as
   // in previous executions of the master block.
   for(i=0;i<sl_len;++i)sel_cur.bind(sl[i]);
   return;
  }else{
   sld_tmp_len=0;
   sel_cur.get_cursor_struct().straight_select=0;
   for(i=1;sel_cur.describe_column(sl_desc_tmp[i-1],i);++i){
    ++sld_tmp_len;
    if(sld_tmp_len==loc_ptr.get_arr_size()){
      loc_ptr.double_size();
      sl_desc_tmp=loc_ptr.get_ptr();
    }
   }
   sl_len=sld_tmp_len;
   if(sl){
    delete[] sl;
    sl=nullptr;
   }
   sl=new otl_generic_variable[sl_len==0?1:sl_len];
   int max_long_size=this->adb->get_max_long_size();
   for(j=0;j<sl_len;++j){
    otl_generic_variable::map_ftype
     (sl_desc_tmp[j],
      max_long_size,
      ftype,
      elem_size,
      this->local_override.getLen()>0?this->local_override:*override_,
      j+1,
      this->adb->get_connect_struct().get_connection_type());
    sl[j].copy_pos(j+1);
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
     if(sl_desc_tmp[j].charset_form==2)
       sl[j].get_var_struct().nls_flag=true;
#endif
     sl[j].init(true,
                ftype,
                elem_size,
                OTL_SCAST(otl_stream_buffer_size_type,array_size),
                &adb->get_connect_struct()
               );
   }
   if(sl_desc){
    delete[] sl_desc;
    sl_desc=nullptr;
   }
   sl_desc=new otl_column_desc[sl_len==0?1:sl_len];
   for(i=0;i<sl_len;++i)
     sl_desc[i]=sl_desc_tmp[i];
   for(i=0;i<sl_len;++i)sel_cur.bind(sl[i]);
#if defined(OTL_ORA_STREAM_POOL_ASSUMES_SAME_REF_CUR_STRUCT_ON_REUSE)
   same_sl_flag=1;
#endif
  }
 }

 void get_in_next(void)
 {
  if(cur_in==vl_len-1)
   rewind();
  else{
   ++cur_in;
   executed=0;
  }
 }

  int check_in_type_throw(int type_code)
  {
    otl_var_info_var
      (vl[cur_in]->get_name(),
       vl[cur_in]->get_ftype(),
       type_code,
       var_info,
       sizeof(var_info));
    if(this->adb)this->adb->increment_throw_count();
    if(this->adb&&this->adb->get_throw_count()>1)return 0;
    if(otl_uncaught_exception()) return 0; 

    throw otl_exception
      (otl_error_msg_0,
       otl_error_code_0,
       stm_label?stm_label:stm_text,
       var_info);
  }

  int check_in_type(int type_code,int tsize)
  {
    switch(vl[cur_in]->get_ftype()){
    case otl_var_char:
      if(type_code==otl_var_char)
        return 1;
    case otl_var_raw:
      if(type_code==otl_var_raw)
        return 1;
    case otl_var_timestamp:
    case otl_var_tz_timestamp:
    case otl_var_ltz_timestamp:
      if(type_code==otl_var_timestamp)
        return 1;
    default:
      if(vl[cur_in]->get_ftype()==type_code &&
         vl[cur_in]->get_elem_size()==tsize)
        return 1; 
    }
    return check_in_type_throw(type_code);
  }
  
 void check_in_var(void)
 {
  if(vl_len==0){
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw otl_exception
    (otl_error_msg_1,
     otl_error_code_1,
     stm_label?stm_label:stm_text,
     nullptr);
  }
 }

 void check_if_executed(void)
 {
  if(!executed){
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw otl_exception
    (otl_error_msg_2,
     otl_error_code_2,
     stm_label?stm_label:stm_text,
     nullptr);
  }
 }

#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
  void strict_check_throw(int type_code)
  {
   otl_var_info_col
     (sl[cur_col].get_pos(),
      sl[cur_col].get_ftype(),
      type_code,
      var_info,
      sizeof(var_info));
   if(this->adb)this->adb->increment_throw_count();
   if(this->adb&&this->adb->get_throw_count()>1)return;
   if(otl_uncaught_exception()) return; 
   throw otl_exception
     (otl_error_msg_0,
      otl_error_code_0,
      this->stm_label?
      this->stm_label:
      this->stm_text,
      var_info);
  }
#endif

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_ref_select_stream(const otl_ref_select_stream&) = delete;
 otl_ref_select_stream& operator=(const otl_ref_select_stream&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_ref_select_stream(otl_ref_select_stream&&) = delete;
 otl_ref_select_stream& operator=(otl_ref_select_stream&&) = delete;
#endif
private:
#else
 otl_ref_select_stream
 (const otl_ref_select_stream&)
   :otl_ref_cursor(),
    override_(nullptr),
    delay_next(0),
    same_sl_flag(0),
    _rfc(0),
    sl_desc(nullptr),
    sl_len(0),
    sl(nullptr),
    null_fetched(0),
    ret_code(0),
    cur_col(0),
    cur_in(0),
    executed(0),
    var_info()
 {
 }

 otl_ref_select_stream& operator=(const otl_ref_select_stream&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_ref_select_stream
 (otl_ref_select_stream&&)
   :otl_ref_cursor(),
    override_(nullptr),
    delay_next(0),
    same_sl_flag(0),
    _rfc(0),
    sl_desc(nullptr),
    sl_len(0),
    sl(nullptr),
    null_fetched(0),
    ret_code(0),
    cur_col(0),
    cur_in(0),
    executed(0),
    var_info()
 {
 }

 otl_ref_select_stream& operator=(otl_ref_select_stream&&)
 {
   return *this;
 }
#endif
#endif

};

class otl_stream_shell: public otl_stream_shell_generic{
public:

  otl_ref_select_stream* ref_ss;
  otl_select_stream* ss;
  otl_inout_stream* io;
  otl_connect* adb;
  
  int auto_commit_flag;
  bool lob_stream_flag;
  
  otl_var_desc* iov;
  int iov_len;
  int next_iov_ndx;
  
  otl_var_desc* ov;
  int ov_len;
  int next_ov_ndx;
  
  bool flush_flag;
  int stream_type;

 otl_select_struct_override override_;

#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
#if defined(OTL_UNICODE_STRING_TYPE)
 std::string orig_sql_stm;
#else
 OTL_STRING_CONTAINER orig_sql_stm;
#endif
#endif

  otl_stream_shell():
    otl_stream_shell_generic(),
    ref_ss(nullptr),
    ss(nullptr),
    io(nullptr),
    adb(nullptr),
    auto_commit_flag(0),
    lob_stream_flag(false),
    iov(nullptr),
    iov_len(0),
    next_iov_ndx(0),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    flush_flag(false),
    stream_type(otl_no_stream_type),
    override_()
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    ,orig_sql_stm()
#endif
 {
   should_delete=0;
 }

  otl_stream_shell(const int ashould_delete):
    otl_stream_shell_generic(),
    ref_ss(nullptr),
    ss(nullptr),
    io(nullptr),
    adb(nullptr),
    auto_commit_flag(0),
    lob_stream_flag(false),
    iov(nullptr),
    iov_len(0),
    next_iov_ndx(0),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    flush_flag(true),
    stream_type(otl_no_stream_type),
    override_()
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    ,orig_sql_stm()
#endif
 {
  should_delete=ashould_delete;
 }

 virtual ~otl_stream_shell()
 {
  if(should_delete){
   delete[] iov;
   delete[] ov;

   iov=nullptr; iov_len=0;
   ov=nullptr; ov_len=0;
   next_iov_ndx=0;
   next_ov_ndx=0;
   override_.setLen(0);
   flush_flag=true;

   delete ss;
   delete io;
   delete ref_ss;
   ss=nullptr; io=nullptr; ref_ss=nullptr;
   adb=nullptr;
  }
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
 otl_stream_shell(const otl_stream_shell&) = delete;
 otl_stream_shell& operator=(const otl_stream_shell&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
 otl_stream_shell(otl_stream_shell&&) = delete;
 otl_stream_shell& operator=(otl_stream_shell&&) = delete;
#endif
private:
#else
  otl_stream_shell(const otl_stream_shell&):
    otl_stream_shell_generic(),
    ref_ss(nullptr),
    ss(nullptr),
    io(nullptr),
    adb(nullptr),
    auto_commit_flag(0),
    lob_stream_flag(false),
    iov(nullptr),
    iov_len(0),
    next_iov_ndx(0),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    flush_flag(false),
    stream_type(otl_no_stream_type),
    override_()
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    ,orig_sql_stm()
#endif
 {
 }

 otl_stream_shell& operator=(const otl_stream_shell&)
 {
   return *this;
 }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_stream_shell(otl_stream_shell&&):
    otl_stream_shell_generic(),
    ref_ss(nullptr),
    ss(nullptr),
    io(nullptr),
    adb(nullptr),
    auto_commit_flag(0),
    lob_stream_flag(false),
    iov(nullptr),
    iov_len(0),
    next_iov_ndx(0),
    ov(nullptr),
    ov_len(0),
    next_ov_ndx(0),
    flush_flag(false),
    stream_type(otl_no_stream_type),
    override_()
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    ,orig_sql_stm()
#endif
 {
 }

 otl_stream_shell& operator=(otl_stream_shell&&)
 {
   return *this;
 }
#endif
#endif

};

class otl_sp_parm_desc{
public:

 int position;
 char arg_name[40];
 char in_out[20];
 char data_type[40];
 char bind_var[128];

  otl_sp_parm_desc():
    position(-1),
    arg_name(),
    in_out(),
    data_type(),
    bind_var()
 {
  arg_name[0]=0;
  in_out[0]=0;
  data_type[0]=0;
  bind_var[0]=0;
 }

 otl_sp_parm_desc(const otl_sp_parm_desc& r):
    position(-1),
    arg_name(),
    in_out(),
    data_type(),
    bind_var()
 {
  copy(r);
 }

 otl_sp_parm_desc& operator=(const otl_sp_parm_desc& r)
 {
  copy(r);
  return *this;
 }

 ~otl_sp_parm_desc(){}

protected:

 void copy(const otl_sp_parm_desc& r)
 {
  position=r.position;
  OTL_STRCPY_S(arg_name,sizeof(arg_name),r.arg_name);
  OTL_STRCPY_S(in_out,sizeof(in_out),r.in_out);
  OTL_STRCPY_S(data_type,sizeof(data_type),r.data_type);
  OTL_STRCPY_S(bind_var,sizeof(bind_var),r.bind_var);
 }

};

class otl_stream
#if defined(OTL_ORA_DECLARE_COMMON_READ_STREAM_INTERFACE)
  : public otl_read_stream_interface
#endif
{
protected:

  otl_stream_shell* shell;
  otl_ptr<otl_stream_shell> shell_pt;
  int connected;
  
  otl_ref_select_stream** ref_ss;
  otl_select_stream** ss;
  otl_inout_stream** io;
  otl_connect** adb;
  
  int* auto_commit_flag;
  
  otl_var_desc** iov;
  int* iov_len;
  int* next_iov_ndx;

  otl_var_desc** ov;
  int* ov_len;
  int* next_ov_ndx;
  int end_marker;
  int oper_int_called;
  int last_eof_rc;
  bool last_oper_was_read_op;

  otl_select_struct_override* override_;

  int buf_size_;

 void inc_next_ov(void)
 {
  if((*ov_len)==0)return;
  if((*next_ov_ndx)<(*ov_len)-1)
   ++(*next_ov_ndx);
  else
   (*next_ov_ndx)=0;
 }
 
 void inc_next_iov(void)
 {
  if((*iov_len)==0)return;
  if((*next_iov_ndx)<(*iov_len)-1)
   ++(*next_iov_ndx);
  else
   (*next_iov_ndx)=0;
 }

  void reset_end_marker(void)
  {
    last_eof_rc=0;
    end_marker=-1;
    oper_int_called=0;
  }

 static void convert_bind_var_datatype
 (char* out_buf,
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400) // VC++ 8.0 or higher
  const size_t out_buf_sz,
#else
  const size_t /*out_buf_sz*/,
#endif
#else
  const size_t /*out_buf_sz*/,
#endif
  const char* datatype,
  const int varchar_size,
  const int all_num2type,
  const int refcur_buf_size)
 {
  out_buf[0]=0;
  if(strcmp(datatype,"BINARY_INTEGER")==0||
     strcmp(datatype,"NATIVE INTEGER")==0||
     strcmp(datatype,"BINARY_FLOAT")==0||
     strcmp(datatype,"BINARY_DOUBLE")==0||
     strcmp(datatype,"FLOAT")==0||
     strcmp(datatype,"NUMBER")==0){
   switch(all_num2type){
   case otl_var_char:
    OTL_STRCPY_S(out_buf,out_buf_sz,"char[50]");
    break;
   case otl_var_double:
    OTL_STRCPY_S(out_buf,out_buf_sz,"double");
    break;
   case otl_var_float:
    OTL_STRCPY_S(out_buf,out_buf_sz,"float");
    break;
   case otl_var_long_int:
    OTL_STRCPY_S(out_buf,out_buf_sz,"long");
    break;
   case otl_var_int:
    OTL_STRCPY_S(out_buf,out_buf_sz,"int");
    break;
   case otl_var_unsigned_int:
    OTL_STRCPY_S(out_buf,out_buf_sz,"unsigned");
    break;
   case otl_var_short:
    OTL_STRCPY_S(out_buf,out_buf_sz,"short");
    break;
   case otl_var_bfloat:
    OTL_STRCPY_S(out_buf,out_buf_sz,"bfloat");
    break;
   case otl_var_bdouble:
    OTL_STRCPY_S(out_buf,out_buf_sz,"bdouble");
    break;
   default:
    break;
   }
  }else if(strcmp(datatype,"RAW")==0){
#if defined(OTL_ORA_CREATE_STORED_PROC_CALL_MAPS_RAW_TO_RAW_LONG)
    OTL_STRCPY_S(out_buf,out_buf_sz,"raw_long");
#elif defined(OTL_ORA_CREATE_STORED_PROC_CALL_MAPS_RAW_TO_RAW)
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
   sprintf_s(out_buf,out_buf_sz,"raw[%d]",varchar_size);
#else
   sprintf(out_buf,"raw[%d]",varchar_size);
#endif
#else
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
   sprintf_s(out_buf,out_buf_sz,"char[%d]",varchar_size);
#else
   sprintf(out_buf,"char[%d]",varchar_size);
#endif
#endif
  }else if(strcmp(datatype,"LONG RAW")==0){
    OTL_STRCPY_S(out_buf,out_buf_sz,"raw_long");
  }else if(strcmp(datatype,"DATE")==0)
    OTL_STRCPY_S(out_buf,out_buf_sz,"timestamp");
#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
else if(strcmp(datatype,"TIMESTAMP")==0)
   OTL_STRCPY_S(out_buf,out_buf_sz,"timestamp");
#endif
  else if(strcmp(datatype,"VARCHAR2")==0)
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
   sprintf_s(out_buf,out_buf_sz,"char[%d]",varchar_size);
#else
   sprintf(out_buf,"char[%d]",varchar_size);
#endif
#else
   sprintf(out_buf,"char[%d]",varchar_size);
#endif
  else if(strcmp(datatype,"CHAR")==0)
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
   sprintf_s(out_buf,out_buf_sz,"char[%d]",varchar_size);
#else
   sprintf(out_buf,"char[%d]",varchar_size);
#endif
#else
   sprintf(out_buf,"char[%d]",varchar_size);
#endif
   else if(strcmp(datatype,"REF CURSOR")==0)
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
   sprintf_s(out_buf,out_buf_sz,"refcur,out[%d]",refcur_buf_size);
#else
   sprintf(out_buf,"refcur,out[%d]",refcur_buf_size);
#endif
#else
   sprintf(out_buf,"refcur,out[%d]",refcur_buf_size);
#endif

 }

  void throw_end_of_row()
#if defined(__GNUC__) && (__GNUC__>=4)
    __attribute__ ((noreturn))
#endif
  {
    throw otl_exception
      (otl_error_msg_34,
       otl_error_code_34,
       this->get_stm_text());
  }
 
public:

  int get_auto_commit_flag() const
  {
    if(!auto_commit_flag)
      return 0;
    else
      return *auto_commit_flag;
  }

  bool get_lob_stream_flag() const 
  {
    if(!shell)
      return false;
    else
      return shell->lob_stream_flag;
  }

  int get_adb_max_long_size() const 
  {
    return this->shell->adb->get_max_long_size();
  }
  

  void check_end_of_row()
  {
    if(next_ov_ndx==nullptr||(*next_ov_ndx)!=0)
      throw_end_of_row();
    if(next_iov_ndx==nullptr||(*next_iov_ndx)!=0)
      throw_end_of_row();
  }

  int get_prefetched_row_count() const 
  {
    (*adb)->reset_throw_count();
    switch(shell->stream_type){
    case otl_no_stream_type:
    case otl_inout_stream_type:
      return 0;
    case otl_select_stream_type:
      return (*ss)->get_prefetched_row_count();
    case otl_refcur_stream_type:
      return (*ref_ss)->get_prefetched_row_count();
    default:
      return 0;
    }
  }

  int get_dirty_buf_len() const
  {
    switch(shell->stream_type){
    case otl_no_stream_type:
      return 0;
    case otl_inout_stream_type:
      return (*io)->get_dirty_buf_len();
    case otl_select_stream_type:
    {
      int rc=(*ss)->get_select_row_count();
      if(rc<0)
        return 0;
      else
        return rc;
    }
    case otl_refcur_stream_type:
      return (*ref_ss)->get_select_row_count();
    default:
      return 0;
    }
  }

  otl_stream_shell* get_shell(){return shell;}

  int get_connected() const {return connected;}


  void setBufSize(int buf_size)
  {
    buf_size_=buf_size;
  }
  
  int getBufSize(void) const
  {
    return buf_size_;
  }

  operator int(void) OTL_THROWS_OTL_EXCEPTION
  {
    if(shell && shell->lob_stream_flag){
      if(this->adb&&*this->adb)(*this->adb)->increment_throw_count();
      if(this->adb&&*this->adb&&(*this->adb)->get_throw_count()>1)return 0;
      const char* stm_label=nullptr;
      const char* stm_text=nullptr;
      switch(shell->stream_type){
      case otl_no_stream_type:
        break;
      case otl_inout_stream_type:
        stm_label=(*io)->get_stm_label();
        stm_text=(*io)->get_stm_text();
        break;
      case otl_select_stream_type:
        stm_label=(*ss)->get_stm_label();
        stm_text=(*ss)->get_stm_text();
        break;
      case otl_refcur_stream_type:
        stm_label=(*ref_ss)->get_stm_label();
        stm_text=(*ref_ss)->get_stm_text();
        break;
      }
      throw otl_exception
        (otl_error_msg_24,
         otl_error_code_24,
         stm_label?stm_label:stm_text);
    }
    if(!last_oper_was_read_op){
      if(this->adb&&*this->adb)(*this->adb)->increment_throw_count();
      if(this->adb&&*this->adb&&(*this->adb)->get_throw_count()>1)return 0;
      const char* stm_label=nullptr;
      const char* stm_text=nullptr;
      switch(shell->stream_type){
      case otl_no_stream_type:
        break;
      case otl_inout_stream_type:
        stm_label=(*io)->get_stm_label();
        stm_text=(*io)->get_stm_text();
        break;
      case otl_select_stream_type:
        stm_label=(*ss)->get_stm_label();
        stm_text=(*ss)->get_stm_text();
        break;
      case otl_refcur_stream_type:
        stm_label=(*ref_ss)->get_stm_label();
        stm_text=(*ref_ss)->get_stm_text();
        break;
      }
      throw otl_exception
        (otl_error_msg_18,
         otl_error_code_18,
         stm_label?stm_label:stm_text);
    }
    if(end_marker==1)return 0;
    int rc=0;
    int temp_eof=eof();
    if(temp_eof && end_marker==-1 && oper_int_called==0){
      end_marker=1;
      if(last_eof_rc==1)
        rc=0;
      else
        rc=1;
    }else if(!temp_eof && end_marker==-1)
      rc=1;
    else if(temp_eof && end_marker==-1){
      end_marker=0;
      rc=1;
    }else if(temp_eof && end_marker==0){
      end_marker=1;
      rc=0;
    }
    if(!oper_int_called)oper_int_called=1;
    return rc;
  }

#if !defined(OTL_UNICODE)
 static void create_stored_proc_call
 (otl_connect& db,
  otl_stream& args_strm,
  char* sql_stm,
  int& stm_type,
  char* refcur_placeholder,
  const char* proc_name,
  const char* package_name,
  const char* schema_name=nullptr,
  const bool schema_name_included=false,
  const int varchar_size=2001,
  const int all_num2type=otl_var_double,
  const int refcur_buf_size=1)
 {
  sql_stm[0]=0;
  stm_type=otl_no_stream_type;
  refcur_placeholder[0]=0;

  char full_name[1024];
  char temp_buf[1024];
  char temp_buf2[1024];
  int i;

  if(package_name==nullptr)
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
    sprintf_s(full_name,sizeof(full_name),"%s",proc_name);
#else
    sprintf(full_name,"%s",proc_name);
#endif
#else
    sprintf(full_name,"%s",proc_name);
#endif
  else
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
    sprintf_s(full_name,sizeof(full_name),"%s.%s",package_name,proc_name);
#else
    sprintf(full_name,"%s.%s",package_name,proc_name);
#endif
#else
    sprintf(full_name,"%s.%s",package_name,proc_name);
#endif
  if(schema_name_included&&schema_name!=nullptr){
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
    sprintf_s(temp_buf,sizeof(temp_buf),"%s.%s",schema_name,full_name);
#else
    sprintf(temp_buf,"%s.%s",schema_name,full_name);
#endif
#else
    sprintf(temp_buf,"%s.%s",schema_name,full_name);
#endif
    OTL_STRCPY_S(full_name,sizeof(full_name),temp_buf);
  }

  if(!args_strm.good()){
   args_strm.open
    (50,
#if defined(OTL_ORA11G)||defined(OTL_ORA11G_R2)
     "select position,  "
     "  lower(':'||nvl(argument_name,'rc__')) argument_name,  "
     "  in_out,  "
     "  nvl(data_type,'*') data_type,  "
     "  ':'||lower(nvl(argument_name,'rc__'))||  "
     "    decode(data_type,'REF CURSOR',' ',  "
     "      '<'||'%s,'||  "
     "      decode(in_out,'IN','in',  "
     "        'OUT','out',  "
     "        'IN/OUT','inout',  "
     "        'xxx')  "
     "      ||'>'  "
     "    ) || decode(position,0,' := ',' ')  "
     "    bind_var  "
     "from all_arguments a, all_procedures p  "
     "where upper(:obj_name/*char[50]*/) in (p.procedure_name,p.object_name) "
     "and (:pkg_name/*char[50]*/ is null and  "
     "     p.procedure_name is null or  "
     "     :pkg_name is not NULL and "
     "     (p.object_name=upper(:pkg_name) or  "
     "      p.object_name =  "
     "  (select package_name from  "
     "  (select table_name as package_name  "
     "    from user_synonyms  "
     "    where synonym_name=upper(:pkg_name)  "
     "    union all  "
     "      select table_name as package_name  "
     "      from all_synonyms  "
     "      where synonym_name=upper(:pkg_name)  "
     "    )  "
     "  where rownum = 1)  "
     "))  "
     "and ((:owner/*char[50]*/ is null  "
     "  and p.owner =  "
     "    (select owner from  "
     "      (select user as owner  "
     "      from user_objects  "
     "      where object_name = upper(nvl(:pkg_name, :obj_name))  "
     "        and object_type in ('PACKAGE','FUNCTION','PROCEDURE')  "
     "      union all  "
     "        select table_owner as owner  "
     "        from user_synonyms "
     "        where synonym_name = upper(nvl(:pkg_name, :obj_name))  "
     "        union all  "
     "          select table_owner as owner  "
     "          from all_synonyms  "
     "          where synonym_name = upper(nvl(:pkg_name, :obj_name))  "
     "      )  "
     "    where rownum = 1)  "
     "  or  "
     "  (:owner is not null and p.owner=upper(:owner)))  "
     "   and a.data_level=0  "
     " )  "
     "and a.object_id = p.object_id  "
     "and a.subprogram_id = p.subprogram_id  "
     "and a.owner = p.owner  "
     "order by a.position  ",
#else
     "select position, "
     "       lower(':'||nvl(argument_name,'rc__')) argument_name, "
     "       in_out, "
     "       nvl(data_type,'*') data_type, "
     "       ':'||lower(nvl(argument_name,'rc__'))|| "
     "       decode(data_type,'REF CURSOR',' ', "
     "              '<'||'%s,'|| "
     "              decode(in_out,'IN','in', "
     "                     'OUT','out', "
     "                     'IN/OUT','inout', "
     "                     'xxx') "
     "              ||'>' "
     "             ) || decode(position,0,' := ',' ') "
     "       bind_var "
     "from all_arguments "
     "where object_name=upper(:obj_name<char[50]>) "
     "  and (:pkg_name<char[50]> is null and package_name is null or "
     " :pkg_name is not null " 
     " and (package_name=upper(:pkg_name) " 
     " or package_name = " 
     " (select package_name from " 
     " (select table_name as package_name " 
     " from user_synonyms " 
     " where synonym_name=upper(:pkg_name) " 
     " union all " 
     " select table_name as package_name " 
     " from all_synonyms " 
     " where synonym_name=upper(:pkg_name) " 
     " )" 
     " where rownum = 1)" 
     " )) " 
     "  and ((:owner<char[50]> is null "
     "        and owner= "
     "            (select owner from "
     "              (select user as owner "
     "               from user_objects "
     "               where (:pkg_name is not null "
     "                      and object_name = upper(:pkg_name) "
     "                      and object_type = 'PACKAGE') "
     "               or (:pkg_name is null "
     "                   and object_name = upper(:obj_name) "
     "                   and object_type in ('FUNCTION','PROCEDURE')) "
     "               union all "
     "               select table_owner as owner "
     "               from user_synonyms "
     "               where (:pkg_name is not null "
     "                      and synonym_name = upper(:pkg_name)) "
     "               or (:pkg_name is null "
     "                   and synonym_name = upper(:obj_name)) "
     "               union all "
     "               select table_owner as owner "
     "               from all_synonyms "
     "               where (:pkg_name is not null "
     "                      and synonym_name = upper(:pkg_name)) "
     "               or (:pkg_name is null "
     "                   and synonym_name = upper(:obj_name)) "
     "              ) "
     "             where rownum = 1) "
     "      or "
     "       (:owner is not null and owner=upper(:owner))) "
     "  and data_level=0 )"
     "order by position",
#endif
     db);
  }
  
  otl_auto_array_ptr< otl_Tptr<otl_sp_parm_desc> > desc(otl_var_list_size);
  int desc_len=0;
  otl_sp_parm_desc parm;
  
  args_strm<<proc_name;
  if(package_name==nullptr)
   args_strm<<otl_null();
  else
   args_strm<<package_name;
  if(schema_name==nullptr)
   args_strm<<otl_null();
  else
   args_strm<<schema_name;
  while(!args_strm.eof()){
   args_strm>>parm.position;
   args_strm>>parm.arg_name;
   args_strm>>parm.in_out;
   args_strm>>parm.data_type;
   args_strm>>parm.bind_var;
   ++desc_len;
   if(desc_len==desc.get_arr_size()){
     int j;
     for(j=0;j<desc.get_arr_size();++j)
       desc.get_ptr()[j].set_do_not_destroy(true);
     desc.double_size();
     for(j=0;j<desc.get_arr_size();++j)
       desc.get_ptr()[j].set_do_not_destroy(false);
   }
   desc.get_ptr()[desc_len-1].assign(new otl_sp_parm_desc(parm));
  }

  if(desc_len==0){
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
   sprintf_s(temp_buf,sizeof(temp_buf),"procedure %s",full_name);
#else
   sprintf(temp_buf,"procedure %s",full_name);
#endif
#else
   sprintf(temp_buf,"procedure %s",full_name);
#endif
   {
     // last ditch attemp to identify a global SP with no parms
     bool global_sp_no_parms=false;
     if(schema_name==nullptr){
       // schema name is not specified
       otl_stream s2
         (1,
          "select 1 cnt "
          "from user_procedures "
          "where object_name=UPPER(:obj_name<char[50]>) "
          "  and procedure_name is null "
#if defined(OTL_ORA11G) || defined(OTL_ORA11G_R2)
          "  and object_type='PROCEDURE' "
#endif
          "union all "
          "select 1 cnt "
          "from user_synonyms syn "
          "where syn.synonym_name=UPPER(:obj_name) "
          "  and exists "
          "   (select 'x' "
          "    from all_procedures proc "
          "	where proc.owner=syn.table_owner "
          "	   and proc.object_name=syn.table_name) "
          "union all "
          "select 1 cnt "
          "from all_synonyms syn "
          "where syn.synonym_name=UPPER(:obj_name) "
          "  and syn.owner='PUBLIC' "
          "  and exists "
          "   (select 'x' "
          "    from all_procedures proc "
          "	where proc.owner=syn.table_owner "
          "	   and proc.object_name=syn.table_name)",
          db);
       s2<<proc_name;
       global_sp_no_parms=!s2.eof();
     }else{
       // schema name is specified
       otl_stream s2
         (1,
          "select object_name  "
          "from all_procedures "
          "where object_name=UPPER(:obj_name<char[50]>) "
          "  and procedure_name is null "
          "  and object_type='PROCEDURE' "
          "  AND owner=UPPER(:owner<char[50]>) "
          "union all "
          "select synonym_name "
          "from all_synonyms syn "
          "where syn.synonym_name=UPPER(:obj_name) "
          "  AND syn.owner=UPPER(:owner) "
          "  and exists "
          "   (select 'x' "
          "    from all_procedures proc "
          "	where proc.owner=syn.table_owner "
          "	   and proc.object_name=syn.table_name)",
          db);
       s2<<proc_name;
       s2<<schema_name;
       global_sp_no_parms=!s2.eof();
     }
     if(global_sp_no_parms){
       // procedure without any parameters
       otl_strcat(sql_stm,"BEGIN ");
       otl_strcat(sql_stm,full_name);
       otl_strcat(sql_stm,"; END;");
       stm_type=otl_constant_sql_type;
       return;
     }else{
       throw otl_exception
         (otl_error_msg_13,
          otl_error_code_13,
          temp_buf);
     }
   }
  }

  if(desc_len==1){
    if(desc.get_ptr()[0].get_ptr()->position==1 
       && desc.get_ptr()[0].get_ptr()->data_type[0]=='*'){
    // procedure without any parameters
     otl_strcat(sql_stm,"BEGIN ");
     otl_strcat(sql_stm,full_name);
     otl_strcat(sql_stm,"; END;");
     stm_type=otl_constant_sql_type;
    return;
    }if(desc.get_ptr()[0].get_ptr()->position==1 && 
        desc.get_ptr()[0].get_ptr()->data_type[0]!='*'){
    // procedure with one parameter
      if(strcmp(desc.get_ptr()[0].get_ptr()->data_type,"REF CURSOR")==0){
     // procedure with one parameter of refcur type
        if(strcmp(desc.get_ptr()[0].get_ptr()->in_out,"IN")==0){
      // refcur parameter should be either OUT or IN OUT, not IN.
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
      sprintf_s(temp_buf,sizeof(temp_buf),"procedure %s",full_name);
#else
      sprintf(temp_buf,"procedure %s",full_name);
#endif
#else
      sprintf(temp_buf,"procedure %s",full_name);
#endif
      throw otl_exception
       (otl_error_msg_15,
        otl_error_code_15,
        temp_buf,nullptr);
     }
     otl_strcat(sql_stm,"BEGIN ");
     otl_strcat(sql_stm,full_name);
     otl_strcat(sql_stm,"(");
     otl_strcat(sql_stm,desc.get_ptr()[0].get_ptr()->bind_var);
     otl_strcat(sql_stm,"); END;");
     stm_type=otl_refcur_stream_type;
     otl_strcpy(OTL_RCAST(unsigned char*,refcur_placeholder),
                OTL_RCAST(const unsigned char*,desc.get_ptr()[0].get_ptr()->arg_name));
     return;
    }else{
     // procedure with one scalar parameter
     convert_bind_var_datatype
       (temp_buf,sizeof(temp_buf),desc.get_ptr()[0].get_ptr()->data_type,
        varchar_size,all_num2type,refcur_buf_size);
     if(temp_buf[0]==0){
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
      sprintf_s(temp_buf,sizeof(temp_buf),"procedure %s, parameter %s",
                full_name,desc.get_ptr()[0].get_ptr()->arg_name);
#else
      sprintf(temp_buf,"procedure %s, parameter %s",
              full_name,desc.get_ptr()[0].get_ptr()->arg_name);
#endif
#else
      sprintf(temp_buf,"procedure %s, parameter %s",
              full_name,desc.get_ptr()[0].get_ptr()->arg_name);
#endif
      throw otl_exception
       (otl_error_msg_14,
        otl_error_code_14,
        temp_buf,nullptr);
     }
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
     sprintf_s(temp_buf2,sizeof(temp_buf2),desc.get_ptr()[0].get_ptr()->bind_var,temp_buf);
#else
     sprintf(temp_buf2,desc.get_ptr()[0].get_ptr()->bind_var,temp_buf);
#endif
#else
     sprintf(temp_buf2,desc.get_ptr()[0].get_ptr()->bind_var,temp_buf);
#endif
     otl_strcat(sql_stm,"BEGIN ");
     otl_strcat(sql_stm,full_name);
     otl_strcat(sql_stm,"(");
     otl_strcat(sql_stm,temp_buf2);
     otl_strcat(sql_stm,"); END;");
     stm_type=otl_inout_stream_type;
     refcur_placeholder[0]=0;
     return;
    }
    }else if(desc.get_ptr()[0].get_ptr()->position==0){
      if(strcmp(desc.get_ptr()[0].get_ptr()->data_type,"REF CURSOR")==0){
     // refcur function without any parameters
      otl_strcat(sql_stm,"BEGIN ");
      otl_strcat(sql_stm,desc.get_ptr()[0].get_ptr()->bind_var);
      otl_strcat(sql_stm," ");
      otl_strcat(sql_stm,full_name);
      otl_strcat(sql_stm,"; END;");
      stm_type=otl_refcur_stream_type;
      otl_strcpy(OTL_RCAST(unsigned char*,refcur_placeholder),
                 OTL_RCAST(const unsigned char*,desc.get_ptr()[0].get_ptr()->arg_name));
      return;
    }else{
     // scalar function without any parameters
     convert_bind_var_datatype
       (temp_buf,sizeof(temp_buf),desc.get_ptr()[0].get_ptr()->data_type,
        varchar_size,all_num2type,refcur_buf_size);
     if(temp_buf[0]==0){
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
       sprintf_s(temp_buf,sizeof(temp_buf),
                 "procedure %s, parameter %s",
                 full_name,
                 desc.get_ptr()[0].get_ptr()->arg_name);
#else
      sprintf(temp_buf,
              "procedure %s, parameter %s",
              full_name,
              desc.get_ptr()[0].get_ptr()->arg_name);
#endif
#else
      sprintf(temp_buf,
              "procedure %s, parameter %s",
              full_name,
              desc.get_ptr()[0].get_ptr()->arg_name);
#endif
      throw otl_exception
       (otl_error_msg_14,
        otl_error_code_14,
        temp_buf,nullptr);
     }
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
     sprintf_s(temp_buf2,sizeof(temp_buf2),desc.get_ptr()[0].get_ptr()->bind_var,temp_buf);
#else
     sprintf(temp_buf2,desc.get_ptr()[0].get_ptr()->bind_var,temp_buf);
#endif
#else
     sprintf(temp_buf2,desc.get_ptr()[0].get_ptr()->bind_var,temp_buf);
#endif
     otl_strcat(sql_stm,"BEGIN ");
     otl_strcat(sql_stm,temp_buf2);
     otl_strcat(sql_stm," ");
     otl_strcat(sql_stm,full_name);
     otl_strcat(sql_stm,"; END;");
     stm_type=otl_inout_stream_type;
     refcur_placeholder[0]=0;
     return;
    }
   }
  }
  
  // Checking if the procedure is of the "refcur" type
  bool refcur_flag=false;
  bool refcur_outpar=false;
  int refcur_count=0;
  bool inpar_only=true;
  for(i=0;i<desc_len;++i){
   if(inpar_only && 
      strcmp(desc.get_ptr()[i].get_ptr()->in_out,"IN")!=0 &&
      strcmp(desc.get_ptr()[i].get_ptr()->data_type,"REF CURSOR")!=0)
    inpar_only=false;
   if(strcmp(desc.get_ptr()[i].get_ptr()->data_type,"REF CURSOR")==0){
    ++refcur_count;
    refcur_flag=true;
    if(refcur_count>1){
      if(refcur_outpar)
        refcur_outpar=strcmp(desc.get_ptr()[i].get_ptr()->in_out,"IN")!=0;
    }else
      refcur_outpar=strcmp(desc.get_ptr()[i].get_ptr()->in_out,"IN")!=0;
   }
  }
  if(refcur_flag){
   if(!refcur_outpar){
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
    sprintf_s(temp_buf,sizeof(temp_buf),"procedure %s",full_name);
#else
    sprintf(temp_buf,"procedure %s",full_name);
#endif
#else
    sprintf(temp_buf,"procedure %s",full_name);
#endif
    throw otl_exception
     (otl_error_msg_15,
      otl_error_code_15,
      temp_buf,nullptr);
   }
   stm_type=otl_refcur_stream_type;
   if(!inpar_only||refcur_count>1)
     stm_type=otl_mixed_refcur_stream_type;
   refcur_placeholder[0]=0;
   sql_stm[0]=0;
   bool full_name_printed=false;
   for(i=0;i<desc_len;++i){

    if(i==0)otl_strcat(sql_stm,"BEGIN ");
    if(stm_type==otl_refcur_stream_type){
      // otl_refcur_stream_type
      if(strcmp(desc.get_ptr()[i].get_ptr()->data_type,"REF CURSOR")==0)
        otl_strcpy(OTL_RCAST(unsigned char*,refcur_placeholder),
                   OTL_RCAST(const unsigned char*,desc.get_ptr()[i].get_ptr()->arg_name));
    }

    // in case of a function, function's return code
    if(desc.get_ptr()[i].get_ptr()->position==0){
     convert_bind_var_datatype
       (temp_buf,sizeof(temp_buf),desc.get_ptr()[i].get_ptr()->data_type,
        varchar_size,all_num2type,refcur_buf_size);
     if(temp_buf[0]==0&&
        strcmp(desc.get_ptr()[i].get_ptr()->data_type,"REF CURSOR")!=0){
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
      sprintf_s(temp_buf,sizeof(temp_buf),"procedure %s, parameter %s",
                full_name,desc.get_ptr()[i].get_ptr()->arg_name);
#else
      sprintf(temp_buf,"procedure %s, parameter %s",
              full_name,desc.get_ptr()[i].get_ptr()->arg_name);
#endif
#else
      sprintf(temp_buf,"procedure %s, parameter %s",
              full_name,desc.get_ptr()[i].get_ptr()->arg_name);
#endif
      throw otl_exception
       (otl_error_msg_14,
        otl_error_code_14,
        temp_buf,nullptr);
     }
     if(strcmp(desc.get_ptr()[i].get_ptr()->data_type,"REF CURSOR")==0&&
        stm_type==otl_refcur_stream_type)
         OTL_STRCPY_S(temp_buf2,sizeof(temp_buf2),desc.get_ptr()[i].get_ptr()->bind_var);
     else if(strcmp(desc.get_ptr()[i].get_ptr()->data_type,"REF CURSOR")==0&&
             stm_type==otl_mixed_refcur_stream_type){
       desc.get_ptr()[i].get_ptr()->bind_var[strlen(desc.get_ptr()[i].get_ptr()->bind_var)-5]=0;
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
       sprintf_s(temp_buf2,sizeof(temp_buf2),
                 "%s<%s> := ",
                 desc.get_ptr()[i].get_ptr()->bind_var,
                 temp_buf);
#else
       sprintf(temp_buf2,
               "%s<%s> := ",
               desc.get_ptr()[i].get_ptr()->bind_var,
               temp_buf);
#endif
#else
       sprintf(temp_buf2,
               "%s<%s> := ",
               desc.get_ptr()[i].get_ptr()->bind_var,
               temp_buf);
#endif
     }else
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
       sprintf_s(temp_buf2,sizeof(temp_buf2),desc.get_ptr()[i].get_ptr()->bind_var,temp_buf);
#else
     sprintf(temp_buf2,desc.get_ptr()[i].get_ptr()->bind_var,temp_buf);
#endif
#else
     sprintf(temp_buf2,desc.get_ptr()[i].get_ptr()->bind_var,temp_buf);
#endif
     otl_strcat(sql_stm,temp_buf2);
    }
    
    // procedure/function's name
    if(!full_name_printed){
     otl_strcat(sql_stm,full_name);
     otl_strcat(sql_stm,"(");
     full_name_printed=true;
    }

    if(desc.get_ptr()[i].get_ptr()->position!=0){
     // normal parameters
     convert_bind_var_datatype
       (temp_buf,sizeof(temp_buf),desc.get_ptr()[i].get_ptr()->data_type,
        varchar_size,all_num2type,refcur_buf_size);
     if(temp_buf[0]==0&&
        strcmp(desc.get_ptr()[i].get_ptr()->data_type,"REF CURSOR")!=0){
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
       sprintf_s(temp_buf,sizeof(temp_buf),"procedure %s, parameter %s",
                 full_name,desc.get_ptr()[i].get_ptr()->arg_name);
#else
      sprintf(temp_buf,"procedure %s, parameter %s",
              full_name,desc.get_ptr()[i].get_ptr()->arg_name);
#endif
#else
      sprintf(temp_buf,"procedure %s, parameter %s",
              full_name,desc.get_ptr()[i].get_ptr()->arg_name);
#endif
      throw otl_exception
       (otl_error_msg_14,
        otl_error_code_14,
        temp_buf,nullptr);
     }
     if(strcmp(desc.get_ptr()[i].get_ptr()->data_type,"REF CURSOR")==0&&
        stm_type==otl_refcur_stream_type)
       OTL_STRCPY_S(temp_buf2,sizeof(temp_buf2),desc.get_ptr()[i].get_ptr()->bind_var);
     else if(strcmp(desc.get_ptr()[i].get_ptr()->data_type,"REF CURSOR")==0&&
             stm_type==otl_mixed_refcur_stream_type){
       desc.get_ptr()[i].get_ptr()->bind_var[strlen(desc.get_ptr()[i].get_ptr()->bind_var)-2]=0;
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
       sprintf_s(temp_buf2,sizeof(temp_buf2),
                 "%s<%s>",
                 desc.get_ptr()[i].get_ptr()->bind_var,
                 temp_buf);
#else
       sprintf(temp_buf2,
               "%s<%s>",
               desc.get_ptr()[i].get_ptr()->bind_var,
               temp_buf);
#endif
#else
       sprintf(temp_buf2,
               "%s<%s>",
               desc.get_ptr()[i].get_ptr()->bind_var,
               temp_buf);
#endif
     }else
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
       sprintf_s(temp_buf2,sizeof(temp_buf2),desc.get_ptr()[i].get_ptr()->bind_var,temp_buf);
#else
     sprintf(temp_buf2,desc.get_ptr()[i].get_ptr()->bind_var,temp_buf);
#endif
#else
     sprintf(temp_buf2,desc.get_ptr()[i].get_ptr()->bind_var,temp_buf);
#endif
     otl_strcat(sql_stm,temp_buf2);
    }

    if(i<desc_len-1 && desc.get_ptr()[i].get_ptr()->position!=0)
     otl_strcat(sql_stm,",");
    else if(i==desc_len-1)
     otl_strcat(sql_stm,"); ");

   }
   otl_strcat(sql_stm," END;");
   return;
  }

  // The procedure is of the "general" type
  stm_type=otl_inout_stream_type;
  refcur_placeholder[0]=0;
  sql_stm[0]=0;
  bool full_name_printed=false;
  for(i=0;i<desc_len;++i){
    if(i==0)otl_strcat(sql_stm,"BEGIN ");
    // in case of a function, function's return code
    if(desc.get_ptr()[i].get_ptr()->position==0){
      convert_bind_var_datatype
        (temp_buf,sizeof(temp_buf),desc.get_ptr()[i].get_ptr()->data_type,
         varchar_size,all_num2type,refcur_buf_size);
      if(temp_buf[0]==0){
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
        sprintf_s(temp_buf,sizeof(temp_buf),"procedure %s, parameter %s",
                  full_name,desc.get_ptr()[i].get_ptr()->arg_name);
#else
        sprintf(temp_buf,"procedure %s, parameter %s",
                full_name,desc.get_ptr()[i].get_ptr()->arg_name);
#endif
#else
        sprintf(temp_buf,"procedure %s, parameter %s",
                full_name,desc.get_ptr()[i].get_ptr()->arg_name);
#endif
        throw otl_exception
          (otl_error_msg_14,
           otl_error_code_14,
           temp_buf,nullptr);
      }
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
      sprintf_s(temp_buf2,sizeof(temp_buf2),desc.get_ptr()[i].get_ptr()->bind_var,temp_buf);
#else
      sprintf(temp_buf2,desc.get_ptr()[i].get_ptr()->bind_var,temp_buf);
#endif
#else
      sprintf(temp_buf2,desc.get_ptr()[i].get_ptr()->bind_var,temp_buf);
#endif
      otl_strcat(sql_stm,temp_buf2);
    }
    // procedure/function's name
    if(!full_name_printed){
      otl_strcat(sql_stm,full_name);
      otl_strcat(sql_stm,"(");
      full_name_printed=true;
    }
    if(desc.get_ptr()[i].get_ptr()->position!=0){
      // normal parameters
      convert_bind_var_datatype
        (temp_buf,sizeof(temp_buf),desc.get_ptr()[i].get_ptr()->data_type,
         varchar_size,all_num2type,refcur_buf_size);
      if(temp_buf[0]==0){
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
        sprintf_s(temp_buf,sizeof(temp_buf),"procedure %s, parameter %s",
                  full_name,desc.get_ptr()[i].get_ptr()->arg_name);
#else
        sprintf(temp_buf,"procedure %s, parameter %s",
                full_name,desc.get_ptr()[i].get_ptr()->arg_name);
#endif
#else
        sprintf(temp_buf,"procedure %s, parameter %s",
                full_name,desc.get_ptr()[i].get_ptr()->arg_name);
#endif
        throw otl_exception
          (otl_error_msg_14,
           otl_error_code_14,
           temp_buf,nullptr);
      }
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
      sprintf_s(temp_buf2,sizeof(temp_buf2),desc.get_ptr()[i].get_ptr()->bind_var,temp_buf);
#else
      sprintf(temp_buf2,desc.get_ptr()[i].get_ptr()->bind_var,temp_buf);
#endif
#else
      sprintf(temp_buf2,desc.get_ptr()[i].get_ptr()->bind_var,temp_buf);
#endif
      otl_strcat(sql_stm,temp_buf2);
    }
    if(i<desc_len-1&&desc.get_ptr()[i].get_ptr()->position!=0)
      otl_strcat(sql_stm,",");
    else if(i==desc_len-1)
      otl_strcat(sql_stm,"); ");
  }
  otl_strcat(sql_stm," END;");
  
 }
#endif

 int get_stream_type(void) OTL_NO_THROW
 {
  if(shell==nullptr)
    return otl_no_stream_type;
  else
    return shell->stream_type;
 }

 void set_column_type(const int column_ndx,
                      const int col_type,
                      const int col_size=0) OTL_NO_THROW
 {
   if(shell==nullptr){
     init_stream();
     shell->flush_flag=true;
   }
   override_->add_override(column_ndx,col_type,col_size);
 }

 
  void set_all_column_types(const unsigned mask=0)
    OTL_NO_THROW
  {
    if(shell==nullptr){
      init_stream();
      shell->flush_flag=true;
    }
    override_->set_all_column_types(mask);
 }

 void set_flush(const bool flush_flag=true)
   OTL_NO_THROW
 {
   if(shell==nullptr)init_stream();
   shell->flush_flag=flush_flag;
 }

  void set_lob_stream_mode(const bool lob_stream_flag=false)
    OTL_NO_THROW
  {
    if(shell==nullptr)return;
    shell->lob_stream_flag=lob_stream_flag;
  }
 
 otl_var_desc* describe_in_vars(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if(shell==nullptr)return nullptr;
  if(shell->iov==nullptr)return nullptr;
  desc_len=shell->iov_len;
  return shell->iov;
 }

 otl_var_desc* describe_out_vars(int& desc_len)
   OTL_NO_THROW
 {
  desc_len=0;
  if(shell==nullptr)return nullptr;
  if(shell->ov==nullptr)return nullptr;
  desc_len=shell->ov_len;
  return shell->ov;
 }

 otl_var_desc* describe_next_in_var(void)
   OTL_NO_THROW
 {
  if(shell==nullptr)return nullptr;
  if(shell->iov==nullptr)return nullptr;
  return &(shell->iov[shell->next_iov_ndx]);
 }

 otl_var_desc* describe_next_out_var(void)
   OTL_NO_THROW
 {
  if(shell==nullptr)return nullptr;
  if(shell->ov==nullptr)return nullptr;
  return &(shell->ov[shell->next_ov_ndx]);
 }
 
  const char* get_stm_text(void)
  {
    const char* no_stm_text=OTL_NO_STM_TEXT;
    switch(shell->stream_type){
    case otl_no_stream_type:
      return no_stm_text;
    case otl_inout_stream_type:
      return (*io)->get_stm_label()?(*io)->get_stm_label():(*io)->get_stm_text();
    case otl_select_stream_type:
      return (*ss)->get_stm_label()?(*ss)->get_stm_label():(*ss)->get_stm_text();
    case otl_refcur_stream_type:
      return (*ref_ss)->get_stm_label()?
             (*ref_ss)->get_stm_label():(*ref_ss)->get_stm_text();
    default:
      return no_stm_text;
    }
  }

 long get_rpc() OTL_NO_THROW
 {
   switch(shell->stream_type){
   case otl_no_stream_type:
     return 0;
   case otl_inout_stream_type:
     (*adb)->reset_throw_count();
     return (*io)->get_rpc();
   case otl_select_stream_type:
     (*adb)->reset_throw_count();
     return (*ss)->get_rfc();
   case otl_refcur_stream_type:
     (*adb)->reset_throw_count();
     return (*ref_ss)->get_rfc();
   default:
     return 0;
   }
 }
 
 void create_var_desc(void)
 {int i;
  delete[] (*iov);
  delete[] (*ov);
  (*iov)=nullptr; (*iov_len)=0;
  (*ov)=nullptr; (*ov_len)=0;
  if((*ss)){
    if((*ss)->get_vl_len()>0){
      (*iov)=new otl_var_desc[(*ss)->get_vl_len()];
      (*iov_len)=(*ss)->get_vl_len();
      for(i=0;i<(*ss)->get_vl_len();++i)
        (*ss)->get_vl()[i]->copy_var_desc((*iov)[i]);
    }
    if((*ss)->get_sl_len()>0){
      (*ov)=new otl_var_desc[(*ss)->get_sl_len()];
      (*ov_len)=(*ss)->get_sl_len();
      for(i=0;i<(*ss)->get_sl_len();++i){
        (*ss)->get_sl()[i].copy_var_desc((*ov)[i]);
        (*ov)[i].copy_name((*ss)->get_sl_desc()[i].name);
      }
    }
  }else if((*io)){
    if((*io)->get_vl_len()>0){
      (*iov)=new otl_var_desc[(*io)->get_vl_len()];
      (*iov_len)=(*io)->get_vl_len();
      for(i=0;i<(*io)->get_vl_len();++i)
        (*io)->get_vl()[i]->copy_var_desc((*iov)[i]);
   }
    if((*io)->get_iv_len()>0){
      (*ov)=new otl_var_desc[(*io)->get_iv_len()];
      (*ov_len)=(*io)->get_iv_len();
      for(i=0;i<(*io)->get_iv_len();++i)
        (*io)->get_in_vl()[i]->copy_var_desc((*ov)[i]);
   }
  }else if((*ref_ss)){
    if((*ref_ss)->get_vl_len()>0){
      (*iov)=new otl_var_desc[(*ref_ss)->get_vl_len()];
      (*iov_len)=(*ref_ss)->get_vl_len();
      for(i=0;i<(*ref_ss)->get_vl_len();++i)
        (*ref_ss)->get_vl()[i]->copy_var_desc((*iov)[i]);
   }
    if((*ref_ss)->get_sl_len()>0){
      int temp_sl_len=(*ref_ss)->get_sl_len();
      (*ov)=new otl_var_desc[temp_sl_len];
      (*ov_len)=temp_sl_len;
      for(i=0;i<temp_sl_len;++i){
        (*ref_ss)->get_sl()[i].copy_var_desc((*ov)[i]);
        (*ov)[i].copy_name((*ref_ss)->get_sl_desc()[i].name);
    }
   }
  }
 }

 void init_stream(void)
 {
   buf_size_=1;
   last_oper_was_read_op=false;
   shell=nullptr;
   shell=new otl_stream_shell(0);
   shell_pt.assign(&shell);
   connected=0;
   
   ref_ss=&(shell->ref_ss);
   ss=&(shell->ss);
   io=&(shell->io);
   adb=&(shell->adb);
   auto_commit_flag=&(shell->auto_commit_flag);
   iov=&(shell->iov);
   iov_len=&(shell->iov_len);
   next_iov_ndx=&(shell->next_iov_ndx);
   ov=&(shell->ov);
   ov_len=&(shell->ov_len);
   next_ov_ndx=&(shell->next_ov_ndx);
   override_=&(shell->override_);
   
   (*ref_ss)=nullptr;
   (*io)=nullptr;
   (*ss)=nullptr;
   (*adb)=nullptr;
   (*ov)=nullptr; 
   (*ov_len)=0;
   (*next_iov_ndx)=0;
   (*next_ov_ndx)=0;
   (*auto_commit_flag)=1;
   (*iov)=nullptr; 
   (*iov_len)=0;

 }

 otl_stream
 (const otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  otl_connect& db,
  const char* ref_cur_placeholder=nullptr,
  const char* sqlstm_label=nullptr)
   OTL_THROWS_OTL_EXCEPTION:
 #if defined(OTL_ORA_DECLARE_COMMON_READ_STREAM_INTERFACE)
   otl_read_stream_interface(),
#endif
   shell(nullptr),
   shell_pt(),
   connected(0),
   ref_ss(nullptr),
   ss(nullptr),
   io(nullptr),
   adb(nullptr),
   auto_commit_flag(nullptr),
   iov(nullptr),
   iov_len(nullptr),
   next_iov_ndx(nullptr),
   ov(nullptr),
   ov_len(nullptr),
   next_ov_ndx(nullptr),
   end_marker(0),
   oper_int_called(0),
   last_eof_rc(0),
   last_oper_was_read_op(false),
   override_(nullptr),
   buf_size_(0)
 {
  init_stream();

  (*io)=nullptr; (*ss)=nullptr; (*ref_ss)=nullptr;
  (*iov)=nullptr; (*iov_len)=0;
  (*ov)=nullptr; (*ov_len)=0;
  (*auto_commit_flag)=1;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  (*adb)=&db;
  shell->flush_flag=true;
  open(arr_size,sqlstm,db,ref_cur_placeholder,sqlstm_label);
 }
 
 otl_stream() OTL_NO_THROW:
 #if defined(OTL_ORA_DECLARE_COMMON_READ_STREAM_INTERFACE)
   otl_read_stream_interface(),
#endif
   shell(nullptr),
   shell_pt(),
   connected(0),
   ref_ss(nullptr),
   ss(nullptr),
   io(nullptr),
   adb(nullptr),
   auto_commit_flag(nullptr),
   iov(nullptr),
   iov_len(nullptr),
   next_iov_ndx(nullptr),
   ov(nullptr),
   ov_len(nullptr),
   next_ov_ndx(nullptr),
   end_marker(0),
   oper_int_called(0),
   last_eof_rc(0),
   last_oper_was_read_op(false),
   override_(nullptr),
   buf_size_(0)
 {
  init_stream();
  shell->flush_flag=true;
 }
 
 virtual ~otl_stream() 
 {
  if(!connected)return;
  try{
   if((*io)!=nullptr&&shell->flush_flag==false)
     (*io)->set_flush_flag2(false);
   close();
   if(shell!=nullptr){
    if((*io)!=nullptr)
      (*io)->set_flush_flag2(true);
   }
  }catch(OTL_CONST_EXCEPTION otl_exception&){
    if(shell!=nullptr){
      if((*io)!=nullptr)
        (*io)->set_flush_flag2(true);
   }
#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
   clean(1);
   if(shell!=nullptr)
     shell->set_should_delete(1);
   shell_pt.destroy();
#else
   shell_pt.destroy();
#endif
#if !defined(OTL_DESTRUCTORS_DO_NOT_THROW)
   throw;
#endif
  }
#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
  if((adb && (*adb) && (*adb)->get_throw_count()>0)
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     || otl_uncaught_exception()
#elif defined(OTL_UNCAUGHT_EXCEPTION_ON)
     || otl_uncaught_exception()
#endif
     ){
   //
  }
#else
   shell_pt.destroy();
#endif
 }

 int eof(void) 
#if defined(OTL_SELECT_STREAM_ALTERNATE_FETCH)
   OTL_THROWS_OTL_EXCEPTION
#else
   OTL_NO_THROW
#endif
 {
  if((*io)){
   (*adb)->reset_throw_count();
   return (*io)->eof();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   return (*ss)->eof();
  }else if((*ref_ss)){
   (*adb)->reset_throw_count();
   return (*ref_ss)->eof();
  }else
   return 1;
 }

  void skip_to_end_of_row() OTL_NO_THROW
  {
    if(next_ov_ndx==nullptr)
      return;
    if((*ov_len)==0)return;
    last_oper_was_read_op=true;
    switch(shell->stream_type){
    case otl_no_stream_type:
      break;
    case otl_inout_stream_type:
      last_eof_rc=(*io)->eof();
      (*io)->skip_to_end_of_row();
      break;
    case otl_select_stream_type:
      last_eof_rc=(*ss)->eof();
      (*ss)->skip_to_end_of_row();
      break;
    case otl_refcur_stream_type:
      last_eof_rc=(*ref_ss)->eof();
      (*ref_ss)->skip_to_end_of_row();
      break;
    }
    *next_ov_ndx=0;
  }


  void reset_to_last_valid_row()
  {
    if((*io)){
      (*adb)->reset_throw_count();
      (*io)->reset_to_last_valid_row();
    }
  }
 
 void flush(const int rowoff=0,const bool force_flush=false)
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->flush(rowoff,force_flush);
  }
 }
 
 bool get_error_state(void) const
 {
   if((*adb)->get_throw_count()>0)
     return true;
   else if((*io))
     return (*io)->get_error_state();
   else
    return false;
 }

 void clean(const int clean_up_error_flag=0)
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->clean(clean_up_error_flag);
  }else if(*ss){
   (*adb)->reset_throw_count();
   (*ss)->clean();
  }else if(*ref_ss){
   (*adb)->reset_throw_count();
   (*ref_ss)->clean();
  }
 }

 void rewind(void)
   OTL_THROWS_OTL_EXCEPTION
 {
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->rewind();
  }else if((*ss)){
   (*adb)->reset_throw_count();
   (*ss)->rewind();
  }else if((*ref_ss)){
   (*adb)->reset_throw_count();
   (*ref_ss)->rewind();
  }
 }
 
 int is_null(void) OTL_NO_THROW
 {
  if((*io))
   return (*io)->is_null();
  else if((*ss))
   return (*ss)->is_null();
  else if((*ref_ss))
   return (*ref_ss)->is_null();
  else
   return 0;
 }

 void set_commit(int auto_commit=0) OTL_NO_THROW
 {
  (*auto_commit_flag)=auto_commit;
  if((*io)){
   (*adb)->reset_throw_count();
   (*io)->set_commit(auto_commit);
  }
 }
 
 void open
 (const otl_stream_buffer_size_type arr_size,
  const char* sqlstm,
  otl_connect& db,
  const char* ref_cur_placeholder=nullptr,
  const char* sqlstm_label=nullptr)
   OTL_THROWS_OTL_EXCEPTION
 {
   if(arr_size<=0){
     throw otl_exception
       (otl_error_msg_40,
        otl_error_code_40,
        sqlstm);
   }
#if defined(OTL_STREAM_THROWS_NOT_CONNECTED_TO_DATABASE_EXCEPTION)
   if(!db.connected){
     throw otl_exception
       (otl_error_msg_35,
        otl_error_code_35,
        sqlstm);
   }
#endif
   reset_end_marker();
   if(this->good()){
     const char* temp_stm_text=nullptr;
     switch(shell->stream_type){
     case otl_no_stream_type:
       temp_stm_text=OTL_NO_STM_TEXT;
       break;
     case otl_inout_stream_type:
       temp_stm_text=(*io)->get_stm_label()?(*io)->get_stm_label():(*io)->get_stm_text();
       break;
     case otl_select_stream_type:
       temp_stm_text=(*ss)->get_stm_label()?(*ss)->get_stm_label():(*ss)->get_stm_text();
       break;
     case otl_refcur_stream_type:
       temp_stm_text=(*ref_ss)->get_stm_label()?
                     (*ref_ss)->get_stm_label():(*ref_ss)->get_stm_text();
       break;
     default:
       temp_stm_text=OTL_NO_STM_TEXT;
       break;
     }
     throw otl_exception
       (otl_error_msg_29,
        otl_error_code_29,
        temp_stm_text);
   }
   if(shell==nullptr)
    init_stream();
   buf_size_=arr_size;
   OTL_TRACE_STREAM_OPEN2
#if (defined(OTL_STL)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
    if(*adb==nullptr)*adb=&db;
    if((*adb) && (**adb).get_stream_pool_enabled_flag()){
      char temp_buf[128];
      otl_itoa(arr_size,temp_buf);
      const char delimiter=';';
#if defined(OTL_STREAM_POOL_USES_STREAM_LABEL_AS_KEY)
      const char* temp_label=sqlstm_label?sqlstm_label:sqlstm;
#if defined(OTL_UNICODE_STRING_TYPE)
      std::string sql_stm(temp_label);
      sql_stm+=delimiter;
      sql_stm+=std::string(temp_buf);
#else
      OTL_STRING_CONTAINER sql_stm(temp_label);
      sql_stm+=delimiter;
      sql_stm+=OTL_STRING_CONTAINER(temp_buf);
#endif
#else
#if defined(OTL_UNICODE_STRING_TYPE)
      std::string sql_stm(sqlstm);
#else
      OTL_STRING_CONTAINER sql_stm(sqlstm);
#endif
      sql_stm+=delimiter;
#if defined(OTL_UNICODE_STRING_TYPE)
      sql_stm+=std::string(temp_buf);
#else
      sql_stm+=OTL_STRING_CONTAINER(temp_buf);
#endif
#endif
      if(shell!=nullptr){
        otl_select_struct_override& temp_override=shell->override_;
        for(int i=0;i<temp_override.getLen();++i){
          otl_itoa(OTL_SCAST(int,temp_override.get_col_type(i)),temp_buf);
          sql_stm+=delimiter;
#if defined(OTL_UNICODE_STRING_TYPE)
          sql_stm+=std::string(temp_buf);
#else
          sql_stm+=OTL_STRING_CONTAINER(temp_buf);
#endif
        }    
      }
      otl_stream_shell_generic* temp_shell=db.get_sc().find(sql_stm);
      if(temp_shell){
        if(shell!=nullptr)
          shell_pt.destroy();
        shell=OTL_RCAST(otl_stream_shell*,temp_shell);
        ref_ss=&(shell->ref_ss);
        ss=&(shell->ss);
        io=&(shell->io);
        if((*io)!=nullptr)(*io)->set_flush_flag2(true);
        adb=&(shell->adb);
        auto_commit_flag=&(shell->auto_commit_flag);
        iov=&(shell->iov);
        iov_len=&(shell->iov_len);
        next_iov_ndx=&(shell->next_iov_ndx);
        ov=&(shell->ov);
        ov_len=&(shell->ov_len);
        next_ov_ndx=&(shell->next_ov_ndx);
        override_=&(shell->override_);
        override_->set_master_stream_ptr(OTL_RCAST(void*,this));
        try{
          if((*iov_len)==0)this->rewind();
        }catch(OTL_CONST_EXCEPTION otl_exception&){
          if((*adb))
            (*adb)->get_sc().remove(shell,shell->orig_sql_stm);
          intern_cleanup();
          shell_pt.destroy();
          connected=0;
          throw;     
        }
        connected=1;
        return;
      }
      shell->orig_sql_stm=sql_stm;
    }
#endif

  delete[] (*iov);
  delete[] (*ov);

  (*iov)=nullptr; (*iov_len)=0;
  (*ov)=nullptr; (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;

  char tmp[7];
  char* c=OTL_CCAST(char*,sqlstm);

  while(otl_isspace(*c)||(*c)=='(')++c;
  OTL_STRNCPY_S(tmp,sizeof(tmp),c,6);
  tmp[6]=0;
  c=tmp;
  while(*c){
   *c=OTL_SCAST(char,otl_to_upper(*c));
   ++c;
  }
  if(adb==nullptr)adb=&(shell->adb);
  (*adb)=&db;
  (*adb)->reset_throw_count();
  try{
    if((strncmp(tmp,"SELECT",6)==0||
        strncmp(tmp,"WITH",4)==0)&&
       ref_cur_placeholder==nullptr){
      override_->set_master_stream_ptr(OTL_RCAST(void*,this));
      (*ss)=new otl_select_stream(override_,arr_size,
                                  sqlstm,db,otl_explicit_select,
                                  sqlstm_label);
      shell->stream_type=otl_select_stream_type;
   }else if(ref_cur_placeholder!=nullptr){
      override_->set_master_stream_ptr(OTL_RCAST(void*,this));
      (*ref_ss)=new otl_ref_select_stream
        (override_,arr_size,sqlstm,ref_cur_placeholder,
         db,sqlstm_label);
      shell->stream_type=otl_refcur_stream_type;
   }else{
    (*io)=new otl_inout_stream
      (arr_size,
       sqlstm,
       db,
       OTL_RCAST(void*,this),
       false,sqlstm_label);
    (*io)->set_flush_flag(shell->flush_flag);
    shell->stream_type=otl_inout_stream_type;
   }
  }catch(OTL_CONST_EXCEPTION otl_exception&){
   shell_pt.destroy();
   throw;
  }
  if((*io))(*io)->set_commit((*auto_commit_flag));
  create_var_desc();
  connected=1;
 }

 void intern_cleanup(void)
 {
  delete[] (*iov);
  delete[] (*ov);

  (*iov)=nullptr; (*iov_len)=0;
  (*ov)=nullptr; (*ov_len)=0;
  (*next_iov_ndx)=0;
  (*next_ov_ndx)=0;
  override_->setLen(0);
  switch(shell->stream_type){
  case otl_no_stream_type:
    break;
  case otl_inout_stream_type:
    try{
      (*io)->flush();
      (*io)->close();
    }catch(OTL_CONST_EXCEPTION otl_exception&){
      clean(1);
      (*io)->close();
      delete (*io);
      (*io)=nullptr;
      shell->stream_type=otl_no_stream_type;
      throw;
    }
    delete (*io);
    (*io)=nullptr;
    shell->stream_type=otl_no_stream_type;
    break;
  case otl_select_stream_type:
    try{
      (*ss)->close();
    }catch(OTL_CONST_EXCEPTION otl_exception&){
      delete (*ss);
      (*ss)=nullptr;
      shell->stream_type=otl_no_stream_type;
      throw;
    }
    delete (*ss);
    (*ss)=nullptr;
    shell->stream_type=otl_no_stream_type;
    break;
  case otl_refcur_stream_type:
    try{
      (*ref_ss)->close();
    }catch(OTL_CONST_EXCEPTION otl_exception&){
      delete (*ref_ss);
      (*ref_ss)=nullptr;
      shell->stream_type=otl_no_stream_type;
      throw;
    }
    delete (*ref_ss);
    (*ref_ss)=nullptr;
    shell->stream_type=otl_no_stream_type;
    break;
  }
  (*ss)=nullptr; (*io)=nullptr; (*ref_ss)=nullptr;
  if(adb!=nullptr)(*adb)=nullptr; 
  adb=nullptr;
 }

#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
 void close(const bool save_in_stream_pool=true)
#else
 void close(void)
#endif
 {
  if(shell==nullptr)return;
  OTL_TRACE_FUNC(0x4,"otl_stream","close","")
#if (defined(OTL_STL)||defined(OTL_ACE)||defined(OTL_UNICODE_STRING_TYPE)) && defined(OTL_STREAM_POOLING_ON)
  if(save_in_stream_pool && (*adb) && (**adb).get_stream_pool_enabled_flag() &&
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
     !otl_uncaught_exception()&&
#elif defined(OTL_UNCAUGHT_EXCEPTION_ON)
     !otl_uncaught_exception()&&
#endif
     (*adb)->get_throw_count()==0){
   try{
    this->flush();
    this->clean(1);
   }catch(OTL_CONST_EXCEPTION otl_exception&){
    this->clean(1);
    throw;
   }
   if((*adb) && (*adb)->get_throw_count()>0){
     (*adb)->get_sc().remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return;
   }
#if defined(OTL_STL) && defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(otl_uncaught_exception()){
    if((*adb))
      (*adb)->get_sc().remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return; 
   }
#elif defined(OTL_UNCAUGHT_EXCEPTION_ON)
   if(otl_uncaught_exception()){
    if((*adb))
      (*adb)->get_sc().remove(shell,shell->orig_sql_stm);
    intern_cleanup();
    shell_pt.destroy();
    connected=0;
    return; 
   }
#endif
   (*adb)->get_sc().add(shell,shell->orig_sql_stm.c_str());
   shell_pt.disconnect();
   connected=0;
  }else{
   if((*adb))
     (*adb)->get_sc().remove(shell,shell->orig_sql_stm);
   intern_cleanup();
   shell_pt.destroy();
   connected=0;
  }
#else
  intern_cleanup();
  connected=0;
#endif
 }

 otl_column_desc* describe_select(int& desc_len)
   OTL_NO_THROW
 {
   desc_len=0;
   switch(shell->stream_type){
   case otl_no_stream_type:
     return nullptr;
   case otl_inout_stream_type:
     return nullptr;
   case otl_select_stream_type:
     (*adb)->reset_throw_count();
     desc_len=(*ss)->get_sl_len();
     return (*ss)->get_sl_desc();
   case otl_refcur_stream_type:
     (*adb)->reset_throw_count();
     desc_len=(*ref_ss)->get_sl_len();
     return (*ref_ss)->get_sl_desc();
   default:
     return nullptr;
   }
 }

 int good(void) OTL_NO_THROW
 {
  if(!connected)return 0;
  if((*io)||(*ss)||(*ref_ss)){
   (*adb)->reset_throw_count();
   return 1;
  }else
   return 0;
 }

  otl_stream& operator>>(otl_stream& (*pf) (otl_stream&))
  {
    (*pf)(*this);
    return *this;
  }

  otl_stream& operator<<(otl_stream& (*pf) (otl_stream&))
  {
    (*pf)(*this);
    return *this;
  }

 otl_stream& operator>>(otl_pl_tab_generic& tab)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=true;
  if((*io)){
   last_eof_rc=(*io)->eof();
   (*io)->operator>>(tab);
   OTL_TRACE_WRITE(", tab len="<<tab.len(),"operator >>","PL/SQL Tab&")
   inc_next_ov();
  }
  return *this;
 }

 otl_stream& operator>>(otl_refcur_stream& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=true;
  if((*io)){
   last_eof_rc=(*io)->eof();
   (*io)->operator>>(s);
   OTL_TRACE_WRITE(" ref.cur.stream","operator >>","otl_refcur_stream&")
   inc_next_ov();
  }
  return *this;
 }

 otl_stream& operator<<(otl_pl_tab_generic& tab)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=false;
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(tab);
   OTL_TRACE_READ(", tab len="<<tab.len(),"operator <<","PL/SQL Tab&")
   inc_next_iov();
  }
  return *this;
 }

#if defined(OTL_PL_TAB) && defined(OTL_STL)

 otl_stream& operator>>(otl_pl_vec_generic& vec)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=true;
  if((*io)){
   last_eof_rc=(*io)->eof();
   (*io)->operator>>(vec);
   OTL_TRACE_WRITE(", tab len="<<vec.len(),"operator >>","PL/SQL Tab&")
   inc_next_ov();
  }
  return *this;
 }

 otl_stream& operator<<(otl_pl_vec_generic& vec)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=false;
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(vec);
   OTL_TRACE_READ(", tab len="<<vec.len(),"operator <<","PL/SQL Tab&")
   inc_next_iov();
  }
  return *this;
 }

#endif

 otl_stream& operator<<(otl_lob_stream& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=false;
   reset_end_marker();
  if((*io)){
   (*io)->operator<<(s);
   OTL_TRACE_READ(", lob stream","operator <<","PL/otl_lob_stream&")
   inc_next_iov();
  }
  return *this;
 }

 otl_stream& operator>>(otl_time0& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     {
       last_eof_rc=(*io)->eof();
       (*io)->operator>>(s);
#if defined(OTL_ORA_TIMESTAMP)
       OTL_TRACE_WRITE
         (OTL_TRACE_FORMAT_DATETIME(s),
          "operator >>",
          "otl_datetime&");
#endif
       break;
     }
   case otl_select_stream_type:
     {
       last_eof_rc=(*ss)->eof();
       (*ss)->operator>>(s);
#if defined(OTL_ORA_TIMESTAMP)
       OTL_TRACE_WRITE
         (OTL_TRACE_FORMAT_DATETIME(s),
          "operator >>",
          "otl_datetime&");
#endif
       break;
     }
   case otl_refcur_stream_type:
     {
       last_eof_rc=(*ref_ss)->eof();
       (*ref_ss)->operator>>(s);
#if defined(OTL_ORA_TIMESTAMP)
       OTL_TRACE_WRITE
         (OTL_TRACE_FORMAT_DATETIME(s),
          "operator >>",
          "otl_datetime&");
#endif
       break;
     }
   }
#if defined(OTL_ORA_TIMESTAMP)
   inc_next_ov();
#endif
   return *this;
 }

 otl_stream& operator<<(const otl_time0& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     {
       (*io)->operator<<(n);
#if defined(OTL_ORA_TIMESTAMP)
       OTL_TRACE_READ(OTL_TRACE_FORMAT_DATETIME(n),
                      "operator <<",
                      "otl_datetime&");
#endif
       break;
     }
   case otl_select_stream_type:
     {
       (*ss)->operator<<(n);
#if defined(OTL_ORA_TIMESTAMP)
       OTL_TRACE_READ(OTL_TRACE_FORMAT_DATETIME(n),
                      "operator <<",
                      "otl_datetime&");
#endif
       break;
     }
   case otl_refcur_stream_type:
     {
       (*ref_ss)->operator<<(n);
       if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
#if defined(OTL_ORA_TIMESTAMP)
       OTL_TRACE_READ(OTL_TRACE_FORMAT_DATETIME(n),
                      "operator <<",
                      "otl_datetime&");
#endif
       break;
     }
   }
#if defined(OTL_ORA_TIMESTAMP)
   inc_next_iov();
#endif
  return *this;
 }

#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  // already declared 
#else
 OTL_ORA_COMMON_READ_STREAM& operator>>(otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;

#if defined(OTL_ORA7_STRING_TO_TIMESTAMP)
  otl_var_desc* temp_next_var=describe_next_out_var();
  if(temp_next_var!=nullptr && temp_next_var->ftype==otl_var_char){
#if defined(OTL_UNICODE)
      OTL_CHAR tmp_str[100];
#elif defined(OTL_UNICODE) && defined(OTL_UNICODE_CHAR_TYPE)
      OTL_UNICODE_CHAR_TYPE tmp_str[100];
#else
      char tmp_str[100];
#endif
    (*this)>>tmp_str;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
    if((*this).is_null())
      s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
    else
      OTL_ORA7_STRING_TO_TIMESTAMP(tmp_str,s);
#else
    OTL_ORA7_STRING_TO_TIMESTAMP(tmp_str,s);
#endif
    OTL_TRACE_WRITE
      (OTL_TRACE_FORMAT_DATETIME(s),
       "operator >>",
       "otl_datetime&");
    return *this;
  }else{
    otl_time0 tmp;
    (*this)>>tmp;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
    if((*this).is_null())
      s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
    else{
      s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
      s.month=tmp.month;
      s.day=tmp.day;
      s.hour=tmp.hour-1;
      s.minute=tmp.minute-1;
      s.second=tmp.second-1;
    }
#else
    s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
    s.month=tmp.month;
    s.day=tmp.day;
    s.hour=tmp.hour-1;
    s.minute=tmp.minute-1;
    s.second=tmp.second-1;
#endif
    OTL_TRACE_WRITE
      (OTL_TRACE_FORMAT_DATETIME(s),
       "operator >>",
       "otl_datetime&")
      inc_next_ov();
    return *this;
  }

#else

   otl_time0 tmp;
   (*this)>>tmp;
#if defined(OTL_DEFAULT_DATETIME_NULL_TO_VAL)
  if((*this).is_null())
   s=OTL_DEFAULT_DATETIME_NULL_TO_VAL;
  else{
   s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
   s.month=tmp.month;
   s.day=tmp.day;
   s.hour=tmp.hour-1;
   s.minute=tmp.minute-1;
   s.second=tmp.second-1;
  }
#else
  s.year=(OTL_SCAST(int,tmp.century)-100)*100+(OTL_SCAST(int,tmp.year)-100);
  s.month=tmp.month;
  s.day=tmp.day;
  s.hour=tmp.hour-1;
  s.minute=tmp.minute-1;
  s.second=tmp.second-1;
#endif
  OTL_TRACE_WRITE
    (OTL_TRACE_FORMAT_DATETIME(s),
     "operator >>",
     "otl_datetime&");
  inc_next_ov();
  return *this;

#endif

 }
#endif

#if (defined(OTL_ORA8I)||defined(OTL_ORA9I))&&defined(OTL_ORA_TIMESTAMP)
  // already declared
#else
 otl_stream& operator<<(const otl_datetime& s)
   OTL_THROWS_OTL_EXCEPTION
 {
  last_oper_was_read_op=false;
  reset_end_marker();
#if defined(OTL_ORA7_TIMESTAMP_TO_STRING)
    otl_var_desc* temp_next_var=describe_next_in_var();
    if(temp_next_var!=nullptr && temp_next_var->ftype==otl_var_char){
#if defined(OTL_UNICODE)
      OTL_CHAR tmp_str[100];
#elif defined(OTL_UNICODE) && defined(OTL_UNICODE_CHAR_TYPE)
      OTL_UNICODE_CHAR_TYPE tmp_str[100];
#else
      char tmp_str[100];
#endif
      OTL_ORA7_TIMESTAMP_TO_STRING(s,tmp_str);
      OTL_TRACE_READ
       (OTL_TRACE_FORMAT_DATETIME(s),
        "operator <<",
        "otl_datetime&");
     (*this)<<tmp_str;
     return *this;
   }else{
     otl_time0 tmp;
     tmp.year=OTL_SCAST(unsigned char, ((s.year%100)+100));
     tmp.century=OTL_SCAST(unsigned char, ((s.year/100)+100));
     tmp.month=OTL_SCAST(unsigned char, s.month);
     tmp.day=OTL_SCAST(unsigned char, s.day);
     tmp.hour=OTL_SCAST(unsigned char, (s.hour+1));
     tmp.minute=OTL_SCAST(unsigned char, (s.minute+1));
     tmp.second=OTL_SCAST(unsigned char, (s.second+1));
     OTL_TRACE_READ
       (OTL_TRACE_FORMAT_DATETIME(s),
        "operator <<",
        "otl_datetime&");
     (*this)<<tmp;
     inc_next_iov();
     return *this;
   }
#else
  otl_time0 tmp;
  tmp.year=OTL_SCAST(unsigned char, ((s.year%100)+100));
  tmp.century=OTL_SCAST(unsigned char, ((s.year/100)+100));
  tmp.month=OTL_SCAST(unsigned char, s.month);
  tmp.day=OTL_SCAST(unsigned char, s.day);
  tmp.hour=OTL_SCAST(unsigned char, (s.hour+1));
  tmp.minute=OTL_SCAST(unsigned char, (s.minute+1));
  tmp.second=OTL_SCAST(unsigned char, (s.second+1));
  OTL_TRACE_READ
    (OTL_TRACE_FORMAT_DATETIME(s),
     "operator <<",
     "otl_datetime&");
  (*this)<<tmp;
  inc_next_iov();
  return *this;
#endif
 }
#endif

#if !defined(OTL_UNICODE)
 OTL_ORA_COMMON_READ_STREAM& operator>>(char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(c);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(c);
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(c);
     break;
   }
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   OTL_TRACE_WRITE("'"<<c<<"'","operator >>","char&")
   inc_next_ov();
   return *this;
 }
#endif

 OTL_ORA_COMMON_READ_STREAM& operator>>(unsigned char& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(c);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(c);
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(c);
     break;
   }
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   OTL_TRACE_WRITE("'"<<c<<"'","operator >>","unsigned char&")
   inc_next_ov();
   return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 OTL_ORA_COMMON_READ_STREAM& operator>>(OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
     break;
   }

#if defined(OTL_USER_DEFINED_STRING_CLASS_DEFAULT_NULL_TO_VAL)
   if((*this).is_null()){
     OTL_USER_DEFINED_STRING_CLASS_DEFAULT_NULL_TO_VAL(s);
   }
#elif defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     s=OTL_DEFAULT_STRING_NULL_TO_VAL;
#endif

   OTL_TRACE_WRITE("\""<<s<<"\"","operator >>","OTL_STRING_CONTAINER&")
   inc_next_ov();
   return *this;
 }
#endif

#if !defined(OTL_UNICODE)
 OTL_ORA_COMMON_READ_STREAM& operator>>(char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
     break;
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     otl_strcpy(OTL_RCAST(unsigned char*,s),
                OTL_RCAST(const unsigned char*,OTL_DEFAULT_STRING_NULL_TO_VAL));
#endif
   OTL_TRACE_WRITE("\""<<s<<"\"","operator >>","char*")
   inc_next_ov();
   return *this;
 }
#endif

#if defined(OTL_UNICODE_STRING_TYPE)
 OTL_ORA_COMMON_READ_STREAM& operator>>(OTL_UNICODE_STRING_TYPE& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
     break;
   }
#if defined(OTL_USER_DEFINED_STRING_CLASS_DEFAULT_NULL_TO_VAL)
   if((*this).is_null()){
     OTL_USER_DEFINED_STRING_CLASS_DEFAULT_NULL_TO_VAL(s);
   }
#elif defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     s=OTL_RCAST(const OTL_UNICODE_CHAR_TYPE*,
                 OTL_DEFAULT_STRING_NULL_TO_VAL);
#endif

   OTL_TRACE_WRITE
     ("\""<<s.c_str()<<"\"",
      "operator >>",
      "OTL_UNICODE_STRING_TYPE&");
   inc_next_ov();
   return *this;
 }

 otl_stream& operator<<(const OTL_UNICODE_STRING_TYPE& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("\""<<s.c_str()<<"\"","operator <<","OTL_UNICODE_STRING_TYPE&");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(s);
     break;
   case otl_select_stream_type:
     (*ss)->operator<<(s);
     break;
   case otl_refcur_stream_type:
     (*ref_ss)->operator<<(s);
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

#endif

 OTL_ORA_COMMON_READ_STREAM& operator>>(unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
     break;
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     otl_strcpy(OTL_RCAST(unsigned char*,s),
                OTL_RCAST(const unsigned char*,OTL_DEFAULT_STRING_NULL_TO_VAL)
               );
#endif

#if defined(OTL_UNICODE)
   OTL_TRACE_WRITE
     ("\""<<OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,s)<<"\"",
      "operator >>",
      OTL_UNICODE_CHAR_TYPE_TRACE_NAME "*")
#else
   OTL_TRACE_WRITE("\""<<s<<"\"","operator >>","unsigned char*")
#endif
   inc_next_ov();
   return *this;
 }

#if defined(OTL_UNICODE)

 OTL_ORA_COMMON_READ_STREAM& operator>>(OTL_UNICODE_CHAR_TYPE* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(OTL_RCAST(unsigned char*,s));
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(OTL_RCAST(unsigned char*,s));
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(OTL_RCAST(unsigned char*,s));
     break;
   }
#if defined(OTL_DEFAULT_STRING_NULL_TO_VAL)
   if((*this).is_null())
     otl_strcpy(OTL_RCAST(unsigned char*,s),
                OTL_RCAST(const unsigned char*,OTL_DEFAULT_STRING_NULL_TO_VAL));
#endif
   OTL_TRACE_WRITE(OTL_RCAST(OTL_UNICODE_CHAR_TYPE*,s),
                   "operator >>",
                   OTL_UNICODE_CHAR_TYPE_TRACE_NAME "*")
   inc_next_ov();
   return *this;
 }

 OTL_ORA_COMMON_READ_STREAM& operator>>(OTL_UNICODE_CHAR_TYPE& c)
   OTL_THROWS_OTL_EXCEPTION
 {
   OTL_UNICODE_CHAR_TYPE s[1024];
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(OTL_RCAST(unsigned char*,s));
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(OTL_RCAST(unsigned char*,s));
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(OTL_RCAST(unsigned char*,s));
     break;
   }
#if defined(_MSC_VER) && (_MSC_VER==1700)
#pragma warning(push)
#pragma warning (disable:6001)
#endif
   c=s[0];
#if defined(_MSC_VER) && (_MSC_VER==1700)
#pragma warning(pop)
#endif
#if defined(OTL_DEFAULT_CHAR_NULL_TO_VAL)
   if((*this).is_null())
     c=OTL_DEFAULT_CHAR_NULL_TO_VAL;
#endif
   OTL_TRACE_WRITE(c,"operator >>",
                   OTL_UNICODE_CHAR_TYPE_TRACE_NAME "")
   inc_next_ov();
   return *this;
 }

#endif

#if defined(OTL_BIGINT) && (defined(OTL_ORA11G_R2)&&!defined(OTL_STR_TO_BIGINT)&&\
    !defined(OTL_BIGINT_TO_STR))
 OTL_ORA_COMMON_READ_STREAM& operator>>(OTL_BIGINT& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(n);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(n);
#else
     (*ss)->operator>><OTL_BIGINT,otl_var_bigint>(n);
#endif
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(n);
#else
     (*ref_ss)->operator>><OTL_BIGINT,otl_var_bigint>(n);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(n,"operator >>","OTL_BIGINT&")
   inc_next_ov();
   return *this;
 }

#endif

#if defined(OTL_UBIGINT) && defined(OTL_ORA11G_R2)
 OTL_ORA_COMMON_READ_STREAM& operator>>(OTL_UBIGINT& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(n);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(n);
#else
     (*ss)->operator>><OTL_UBIGINT,otl_var_ubigint>(n);
#endif
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(n);
#else
     (*ref_ss)->operator>><OTL_UBIGINT,otl_var_ubigint>(n);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(n,"operator >>","OTL_UBIGINT&")
   inc_next_ov();
   return *this;
 }

#endif

#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)

otl_stream& operator>>(OTL_NUMERIC_TYPE_1& n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_numeric_type_1_str_size];
#if defined(OTL_UNICODE)
  OTL_CHAR unitemp_val[otl_numeric_type_1_str_size];
  (*this)>>OTL_RCAST(unsigned char*,unitemp_val);
  OTL_CHAR* uc=unitemp_val;
  char* c=temp_val;
  while(*uc){
    *c=OTL_SCAST(char,*uc);
    ++uc; ++c;
  }
  *c=0;
#else
  (*this)>>temp_val;
#endif
  if(this->is_null()){
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if(this->is_null())
     n=OTL_SCAST(OTL_NUMERIC_TYPE_1,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
    return *this;
  }
  OTL_STR_TO_NUMERIC_TYPE_1(temp_val,n)
  return *this;
}

  otl_stream& operator<<(const OTL_NUMERIC_TYPE_1& n)
    OTL_THROWS_OTL_EXCEPTION
  {
#if defined(OTL_UNICODE)
    char temp_val[otl_numeric_type_1_str_size];
    OTL_NUMERIC_TYPE_1_TO_STR(n,temp_val)
      OTL_CHAR unitemp_val[otl_numeric_type_1_str_size];
    OTL_CHAR* uc=unitemp_val;
    char* c=temp_val;
    while(*c){
      *uc=OTL_SCAST(OTL_CHAR,*c);
      ++uc; ++c;
    }
    *uc=0;
    (*this)<<OTL_RCAST(unsigned char*,unitemp_val);
#else
    char temp_val[otl_numeric_type_1_str_size];
    OTL_NUMERIC_TYPE_1_TO_STR(n,temp_val)
      (*this)<<temp_val;
#endif
    return *this;
  }
#endif

#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)

otl_stream& operator>>(OTL_NUMERIC_TYPE_2& n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_numeric_type_2_str_size];
#if defined(OTL_UNICODE)
  OTL_CHAR unitemp_val[otl_numeric_type_2_str_size];
  (*this)>>OTL_RCAST(unsigned char*,unitemp_val);
  OTL_CHAR* uc=unitemp_val;
  char* c=temp_val;
  while(*uc){
    *c=OTL_SCAST(char,*uc);
    ++uc; ++c;
  }
  *c=0;
#else
  (*this)>>temp_val;
#endif
  if(this->is_null()){
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if(this->is_null())
     n=OTL_SCAST(OTL_NUMERIC_TYPE_2,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
    return *this;
  }
  OTL_STR_TO_NUMERIC_TYPE_2(temp_val,n)
  return *this;
}

  otl_stream& operator<<(const OTL_NUMERIC_TYPE_2& n)
    OTL_THROWS_OTL_EXCEPTION
  {
#if defined(OTL_UNICODE)
    char temp_val[otl_numeric_type_2_str_size];
    OTL_NUMERIC_TYPE_2_TO_STR(n,temp_val)
      OTL_CHAR unitemp_val[otl_numeric_type_2_str_size];
    OTL_CHAR* uc=unitemp_val;
    char* c=temp_val;
    while(*c){
      *uc=OTL_SCAST(OTL_CHAR,*c);
      ++uc; ++c;
    }
    *uc=0;
    (*this)<<OTL_RCAST(unsigned char*,unitemp_val);
#else
    char temp_val[otl_numeric_type_2_str_size];
    OTL_NUMERIC_TYPE_2_TO_STR(n,temp_val)
      (*this)<<temp_val;
#endif
    return *this;
  }
#endif

#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)

otl_stream& operator>>(OTL_NUMERIC_TYPE_3& n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_numeric_type_3_str_size];
#if defined(OTL_UNICODE)
  OTL_CHAR unitemp_val[otl_numeric_type_3_str_size];
  (*this)>>OTL_RCAST(unsigned char*,unitemp_val);
  OTL_CHAR* uc=unitemp_val;
  char* c=temp_val;
  while(*uc){
    *c=OTL_SCAST(char,*uc);
    ++uc; ++c;
  }
  *c=0;
#else
  (*this)>>temp_val;
#endif
  if(this->is_null()){
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if(this->is_null())
     n=OTL_SCAST(OTL_NUMERIC_TYPE_3,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
    return *this;
  }
  OTL_STR_TO_NUMERIC_TYPE_3(temp_val,n)
  return *this;
}

  otl_stream& operator<<(const OTL_NUMERIC_TYPE_3& n)
    OTL_THROWS_OTL_EXCEPTION
  {
#if defined(OTL_UNICODE)
    char temp_val[otl_numeric_type_3_str_size];
    OTL_NUMERIC_TYPE_3_TO_STR(n,temp_val)
      OTL_CHAR unitemp_val[otl_numeric_type_3_str_size];
    OTL_CHAR* uc=unitemp_val;
    char* c=temp_val;
    while(*c){
      *uc=OTL_SCAST(OTL_CHAR,*c);
      ++uc; ++c;
    }
    *uc=0;
    (*this)<<OTL_RCAST(unsigned char*,unitemp_val);
#else
    char temp_val[otl_numeric_type_3_str_size];
    OTL_NUMERIC_TYPE_3_TO_STR(n,temp_val)
      (*this)<<temp_val;
#endif
    return *this;
  }
#endif

#if defined(OTL_BIGINT) && defined(OTL_STR_TO_BIGINT) && \
    defined(OTL_BIGINT_TO_STR)

otl_stream& operator>>(OTL_BIGINT& n)
  OTL_THROWS_OTL_EXCEPTION
{
  char temp_val[otl_bigint_str_size];
#if defined(OTL_UNICODE)
  OTL_CHAR unitemp_val[otl_bigint_str_size];
  (*this)>>OTL_RCAST(unsigned char*,unitemp_val);
  OTL_CHAR* uc=unitemp_val;
  char* c=temp_val;
  while(*uc){
    *c=OTL_SCAST(char,*uc);
    ++uc; ++c;
  }
  *c=0;
#else
  (*this)>>temp_val;
#endif
  if(this->is_null()){
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if(this->is_null())
     n=OTL_SCAST(OTL_BIGINT,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
    return *this;
  }
  OTL_STR_TO_BIGINT(temp_val,n)
  return *this;
}

  otl_stream& operator<<(const OTL_BIGINT n)
    OTL_THROWS_OTL_EXCEPTION
  {
#if defined(OTL_UNICODE)
    char temp_val[otl_bigint_str_size];
    OTL_BIGINT_TO_STR(n,temp_val)
      OTL_CHAR unitemp_val[otl_bigint_str_size];
    OTL_CHAR* uc=unitemp_val;
    char* c=temp_val;
    while(*c){
      *uc=OTL_SCAST(OTL_CHAR,*c);
      ++uc; ++c;
    }
    *uc=0;
    (*this)<<OTL_RCAST(unsigned char*,unitemp_val);
#else
    char temp_val[otl_bigint_str_size];
    OTL_BIGINT_TO_STR(n,temp_val)
      (*this)<<temp_val;
#endif
    return *this;
  }

#elif defined(OTL_BIGINT) && defined(OTL_ORA_MAP_BIGINT_TO_LONG)

  otl_stream& operator>>(OTL_BIGINT& n)
    OTL_THROWS_OTL_EXCEPTION
  {
    long temp_val;
    (*this)>>temp_val;
    if(this->is_null()){
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
      if(this->is_null())
        n=OTL_SCAST(OTL_BIGINT,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
      return *this;
    }
    n=OTL_SCAST(OTL_BIGINT,temp_val);
    return *this;
  }

  otl_stream& operator<<(const OTL_BIGINT n)
    OTL_THROWS_OTL_EXCEPTION
  {
    long temp_val=OTL_SCAST(long,n);
    (*this)<<temp_val;
    return *this;
  }

#endif

 OTL_ORA_COMMON_READ_STREAM& operator>>(int& n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(n);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(n);
#else
     (*ss)->operator>><int,otl_var_int>(n);
#endif
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(n);
#else
     (*ref_ss)->operator>><int,otl_var_int>(n);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     n=OTL_SCAST(int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(n,"operator >>","int&")
   inc_next_ov();
   return *this;
 }

 OTL_ORA_COMMON_READ_STREAM& operator>>(unsigned& u)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(u);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(u);
#else
     (*ss)->operator>><unsigned,otl_var_unsigned_int>(u);
#endif
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(u);
#else
     (*ref_ss)->operator>><unsigned,otl_var_unsigned_int>(u);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     u=OTL_SCAST(unsigned int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(u,"operator >>","unsigned&")
   inc_next_ov();
   return *this;
 }

 OTL_ORA_COMMON_READ_STREAM& operator>>(short& sh)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(sh);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(sh);
#else
     (*ss)->operator>><short,otl_var_short>(sh);
#endif
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(sh);
#else
     (*ref_ss)->operator>><short,otl_var_short>(sh);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     sh=OTL_SCAST(short int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(sh,"operator >>","short int&")
   inc_next_ov();
   return *this;
 }

 OTL_ORA_COMMON_READ_STREAM& operator>>(long int& l)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(l);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(l);
#else
     (*ss)->operator>><long,otl_var_long_int>(l);
#endif
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(l);
#else
     (*ref_ss)->operator>><long,otl_var_long_int>(l);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     l=OTL_SCAST(long int,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(l,"operator >>","long int&")
   inc_next_ov();
   return *this;
 }

 OTL_ORA_COMMON_READ_STREAM& operator>>(float& f)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(f);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(f);
#else
     (*ss)->operator>><float,otl_var_float>(f);
#endif
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(f);
#else
     (*ref_ss)->operator>><float,otl_var_float>(f);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
     f=OTL_SCAST(float,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(f,"operator >>","float&")
   inc_next_ov();
   return *this;
 }

 OTL_ORA_COMMON_READ_STREAM& operator>>(double& d)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(d);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator>>(d);
#else
     (*ss)->operator>><double,otl_var_double>(d);
#endif
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator>>(d);
#else
     (*ref_ss)->operator>><double,otl_var_double>(d);
#endif
     break;
   }
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
   if((*this).is_null())
   d=OTL_SCAST(double,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
#endif
   OTL_TRACE_WRITE(d,"operator >>","double&")
   inc_next_ov();
   return *this;
 }

 OTL_ORA_COMMON_READ_STREAM& operator>>(otl_long_string& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator>>(s);
     break;
   case otl_select_stream_type:
     (*ss)->operator>>(s);
     break;
   case otl_refcur_stream_type:
     (*ref_ss)->operator>>(s);
     break;
   }
   OTL_TRACE_WRITE(" len="<<s.len(),"operator >>","otl_long_string&")
   inc_next_ov();
   return *this;
 }

 OTL_ORA_COMMON_READ_STREAM& operator>>(otl_lob_stream& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=true;
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     last_eof_rc=(*io)->eof();
     (*io)->operator>>(s);
     break;
   case otl_select_stream_type:
     last_eof_rc=(*ss)->eof();
     (*ss)->operator>>(s);
     break;
   case otl_refcur_stream_type:
     last_eof_rc=(*ref_ss)->eof();
     (*ref_ss)->operator>>(s);
     break;
   }
   shell->lob_stream_flag=true;
   OTL_TRACE_WRITE(" lob stream","operator >>","otl_lob_stream&")
   inc_next_ov();
   return *this;
 }

#if !defined(OTL_UNICODE)
 otl_stream& operator<<(const char c)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("'"<<c<<"'","operator <<","char");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(c);
     break;
   case otl_select_stream_type:
     (*ss)->operator<<(c);
     break;
   case otl_refcur_stream_type:
     (*ref_ss)->operator<<(c);
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

 otl_stream& operator<<(const unsigned char c)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("'"<<c<<"'","operator <<","unsigned char");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(c);
     break;
   case otl_select_stream_type:
     (*ss)->operator<<(c);
     break;
   case otl_refcur_stream_type:
     (*ref_ss)->operator<<(c);
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
 otl_stream& operator<<(const OTL_STRING_CONTAINER& s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("\""<<s<<"\"","operator <<","OTL_STRING_CONTAINER&");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(s);
     break;
   case otl_select_stream_type:
     (*ss)->operator<<(s);
     break;
   case otl_refcur_stream_type:
     (*ref_ss)->operator<<(s);
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

#if !defined(OTL_UNICODE)
 otl_stream& operator<<(const char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("\""<<s<<"\"","operator <<","char*");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(s);
     break;
   case otl_select_stream_type:
     (*ss)->operator<<(s);
     break;
   case otl_refcur_stream_type:
     (*ref_ss)->operator<<(s);
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

 otl_stream& operator<<(const unsigned char* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
#if defined(OTL_UNICODE)
   OTL_TRACE_READ
     ("\""<<OTL_RCAST(const OTL_UNICODE_CHAR_TYPE*,s)<<"\"",
      "operator <<",
      OTL_UNICODE_CHAR_TYPE_TRACE_NAME "*");
#else
   OTL_TRACE_READ("\""<<s<<"\"","operator <<","unsigned char*");
#endif
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(s);
     break;
   case otl_select_stream_type:
     (*ss)->operator<<(s);
     break;
   case otl_refcur_stream_type:
     (*ref_ss)->operator<<(s);
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

#if defined(OTL_UNICODE)

 otl_stream& operator<<(const OTL_UNICODE_CHAR_TYPE* s)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ
     ("\""<<OTL_RCAST(const OTL_UNICODE_CHAR_TYPE*,s)<<"\"",
      "operator <<",
      OTL_UNICODE_CHAR_TYPE_TRACE_NAME "*");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(OTL_RCAST(const unsigned char*,s));
     break;
   case otl_select_stream_type:
     (*ss)->operator<<(OTL_RCAST(const unsigned char*,s));
     break;
   case otl_refcur_stream_type:
     (*ref_ss)->operator<<(OTL_RCAST(const unsigned char*,s));
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const OTL_UNICODE_CHAR_TYPE c)
   OTL_THROWS_OTL_EXCEPTION
 {
   OTL_UNICODE_CHAR_TYPE s[2];
   s[0]=c;
   s[1]=0;
   (*this)<<s;
   return *this;
 }

#endif

#if defined(OTL_BIGINT) && (defined(OTL_ORA11G_R2)&&!defined(OTL_STR_TO_BIGINT)&&\
    !defined(OTL_BIGINT_TO_STR))
 otl_stream& operator<<(const OTL_BIGINT n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(n,"operator <<","OTL_BIGINT");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(n);
     break;
   case otl_select_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(n);
#else
     (*ss)->operator<<<OTL_BIGINT,otl_var_bigint>(n);
#endif
     break;
   case otl_refcur_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
    (*ref_ss)->operator<<(n);
#else
     (*ref_ss)->operator<<<OTL_BIGINT,otl_var_bigint>(n);
#endif
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

#if defined(OTL_UBIGINT) && defined(OTL_ORA11G_R2)
 otl_stream& operator<<(const OTL_UBIGINT n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(n,"operator <<","OTL_UBIGINT");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(n);
     break;
   case otl_select_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(n);
#else
     (*ss)->operator<<<OTL_UBIGINT,otl_var_ubigint>(n);
#endif
     break;
   case otl_refcur_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
    (*ref_ss)->operator<<(n);
#else
     (*ref_ss)->operator<<<OTL_UBIGINT,otl_var_ubigint>(n);
#endif
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }
#endif

 otl_stream& operator<<(const int n)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(n,"operator <<","int");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(n);
     break;
   case otl_select_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(n);
#else
     (*ss)->operator<<<int,otl_var_int>(n);
#endif
     break;
   case otl_refcur_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator<<(n);
#else
     (*ref_ss)->operator<<<int,otl_var_int>(n);
#endif
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const unsigned u)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(u,"operator <<","unsigned");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(u);
     break;
   case otl_select_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(u);
#else
     (*ss)->operator<<<unsigned,otl_var_unsigned_int>(u);
#endif
     break;
   case otl_refcur_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator<<(u);
#else
     (*ref_ss)->operator<<<unsigned,otl_var_unsigned_int>(u);
#endif
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const short sh)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(sh,"operator <<","short int");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(sh);
     break;
   case otl_select_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(sh);
#else
     (*ss)->operator<<<short,otl_var_short>(sh);
#endif
     break;
   case otl_refcur_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator<<(sh);
#else
     (*ref_ss)->operator<<<short,otl_var_short>(sh);
#endif
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const long int l)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(l,"operator <<","long int");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(l);
     break;
   case otl_select_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(l);
#else
     (*ss)->operator<<<long,otl_var_long_int>(l);
#endif
     break;
   case otl_refcur_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator<<(l);
#else
     (*ref_ss)->operator<<<long,otl_var_long_int>(l);
#endif
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const float f)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(f,"operator <<","float");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(f);
     break;
   case otl_select_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(f);
#else
     (*ss)->operator<<<float,otl_var_float>(f);
#endif
     break;
   case otl_refcur_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator<<(f);
#else
     (*ref_ss)->operator<<<float,otl_var_float>(f);
#endif
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
  inc_next_iov();
  return *this;
 }

 otl_stream& operator<<(const double d)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(d,"operator <<","double");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(d);
     break;
   case otl_select_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ss)->operator<<(d);
#else
     (*ss)->operator<<<double,otl_var_double>(d);
#endif
     break;
   case otl_refcur_stream_type:
#if defined(OTL_NO_TMPL_MEMBER_FUNC_SUPPORT)
     (*ref_ss)->operator<<(d);
#else
     (*ref_ss)->operator<<<double,otl_var_double>(d);
#endif
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

  otl_stream& operator<<(const otl_null& n)
    OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ("NULL","operator <<","otl_null&");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(n);
     break;
   case otl_select_stream_type:
     (*ss)->operator<<(n);
     break;
   case otl_refcur_stream_type:
     (*ref_ss)->operator<<(n);
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

 otl_stream& operator<<(const otl_long_string& d)
   OTL_THROWS_OTL_EXCEPTION
 {
   last_oper_was_read_op=false;
   reset_end_marker();
   OTL_TRACE_READ(" len="<<d.len(),"operator <<","otl_long_string&");
   switch(shell->stream_type){
   case otl_no_stream_type:
     break;
   case otl_inout_stream_type:
     (*io)->operator<<(d);
     break;
   case otl_select_stream_type:
     (*ss)->operator<<(d);
     break;
   case otl_refcur_stream_type:
     (*ref_ss)->operator<<(d);
     if(!(*ov)&&(*ref_ss)->get_sl()) create_var_desc();
     break;
   }
   inc_next_iov();
   return *this;
 }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_stream& operator=(const otl_stream&) = delete;
  otl_stream(const otl_stream&) = delete;

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_stream& operator=(otl_stream&&) = delete;
  otl_stream(otl_stream&&) = delete;
#endif

#if !defined(OTL_STREAM_NO_PRIVATE_BOOL_OPERATORS)
  otl_stream& operator>>(bool&) = delete;
  otl_stream& operator<<(const bool) = delete;
#endif

#if !defined(OTL_STREAM_NO_PRIVATE_UNSIGNED_LONG_OPERATORS)
  otl_stream& operator>>(unsigned long int&) = delete;
  otl_stream& operator<<(const unsigned long int) = delete;
#endif
private:
#else
  otl_stream& operator=(const otl_stream&)
  {
    return *this;
  }

  otl_stream(const otl_stream&):
 #if defined(OTL_ORA_DECLARE_COMMON_READ_STREAM_INTERFACE)
   otl_read_stream_interface(),
#endif
   shell(nullptr),
   shell_pt(),
   connected(0),
   ref_ss(nullptr),
   ss(nullptr),
   io(nullptr),
   adb(nullptr),
   auto_commit_flag(nullptr),
   iov(nullptr),
   iov_len(nullptr),
   next_iov_ndx(nullptr),
   ov(nullptr),
   ov_len(nullptr),
   next_ov_ndx(nullptr),
   end_marker(0),
   oper_int_called(0),
   last_eof_rc(0),
   last_oper_was_read_op(false),
   override_(nullptr),
   buf_size_(0)
  {
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_stream& operator=(otl_stream&&)
  {
    return *this;
  }

  otl_stream(otl_stream&&):
 #if defined(OTL_ORA_DECLARE_COMMON_READ_STREAM_INTERFACE)
   otl_read_stream_interface(),
#endif
   shell(nullptr),
   shell_pt(),
   connected(0),
   ref_ss(nullptr),
   ss(nullptr),
   io(nullptr),
   adb(nullptr),
   auto_commit_flag(nullptr),
   iov(nullptr),
   iov_len(nullptr),
   next_iov_ndx(nullptr),
   ov(nullptr),
   ov_len(nullptr),
   next_ov_ndx(nullptr),
   end_marker(0),
   oper_int_called(0),
   last_eof_rc(0),
   last_oper_was_read_op(false),
   override_(nullptr),
   buf_size_(0)
  {
  }
#endif

#if !defined(OTL_STREAM_NO_PRIVATE_BOOL_OPERATORS)
  otl_stream& operator>>(bool&)
    OTL_NO_THROW
  {
   return *this;
  }

  otl_stream& operator<<(const bool)
    OTL_NO_THROW
  {
   return *this;
  }

#endif

#if !defined(OTL_STREAM_NO_PRIVATE_UNSIGNED_LONG_OPERATORS)
  otl_stream& operator>>(unsigned long int&)
    OTL_NO_THROW
  {
   return *this;
  }

  otl_stream& operator<<(const unsigned long int)
    OTL_NO_THROW
  {
   return *this;
  }
#endif
#endif

};

#if defined(OTL_ORA_SUBSCRIBE)
#if !defined(OTL_ORA_OCI_ENV_CREATE)
#error OTL_ORA_SUBSCRIBE requires #define OTL_ORA_OCI_ENV_CREATE to be enabled
#endif
#if !defined(OTL_ORA_OCI_ENV_CREATE_MODE)
#error OTL_ORA_SUBSCRIBE requires #define OTL_ORA_OCI_ENV_CREATE_MODE to be \
enabled and to have OCI_THREADED|OCI_OBJECT|OCI_EVENTS
#endif

class otl_subscriber{
public:

  otl_subscriber(otl_connect* adb=nullptr):
    db(adb),
    subscrhp(nullptr)
  {
  }

  virtual ~otl_subscriber(void)
  {
    unsubscribe();
  }

  void subscribe(const char *name=nullptr,int port=0,int timeout=1800)
  {
    if(subscrhp) unsubscribe();
    if(!db||(db&&!db->connected)) 
      throw otl_exception
        (otl_error_msg_32,
         otl_error_code_32);
    
    OCIEnv *envhp=db->get_connect_struct().get_envhp();
    OCIError *errhp=db->get_connect_struct().get_errhp();
    OCISvcCtx *svchp=db->get_connect_struct().get_svchp();
    
    if(port)
      check(OCIAttrSet(OTL_RCAST(dvoid*,envhp), 
                       OTL_SCAST(ub4,OCI_HTYPE_ENV), 
                       OTL_RCAST(dvoid*,&port),
                       0, 
                       OCI_ATTR_SUBSCR_PORTNO, 
                       errhp));

    OCISubscription** temp_subscrhp=&subscrhp;
    check(OCIHandleAlloc(OTL_RCAST(dvoid*,envhp), 
                         OTL_RCAST(dvoid**,temp_subscrhp),
                         OCI_HTYPE_SUBSCRIPTION, 
                         OTL_SCAST(size_t,0),
                         nullptr));
    
    if(name && *name)
      check(OCIAttrSet(subscrhp, 
                       OCI_HTYPE_SUBSCRIPTION, 
                       OTL_RCAST(void*,OTL_CCAST(char*,name)), 
                       OTL_SCAST(ub4,strlen(name)), 
                       OCI_ATTR_SUBSCR_NAME, 
                       errhp));
    
    ub4 nspace = OCI_SUBSCR_NAMESPACE_DBCHANGE;
    check(OCIAttrSet(subscrhp, 
                     OCI_HTYPE_SUBSCRIPTION, 
                     OTL_RCAST(dvoid*,&nspace), 
                     sizeof(ub4), 
                     OCI_ATTR_SUBSCR_NAMESPACE, 
                     errhp));
    
    check(OCIAttrSet(subscrhp, 
                     OCI_HTYPE_SUBSCRIPTION, 
#if defined(__GNUC__) && (__GNUC__<4)
                     (void*)common_notification_callback,
#else
                     OTL_RCAST(void*,common_notification_callback),
#endif
                     0, 
                     OCI_ATTR_SUBSCR_CALLBACK, 
                     errhp));
    
    int rowids_needed=1;
    check(OCIAttrSet(subscrhp, 
                     OCI_HTYPE_SUBSCRIPTION, 
                     OTL_RCAST(dvoid*,&rowids_needed),
                     sizeof(ub4), 
                     OCI_ATTR_CHNF_ROWIDS, 
                     errhp));
    
    check(OCIAttrSet(subscrhp, 
                     OTL_SCAST(ub4,OCI_HTYPE_SUBSCRIPTION),
                     OTL_RCAST(dvoid*,this),
                     0, 
                     OCI_ATTR_SUBSCR_CTX, 
                     errhp));
    
    if(timeout)
      check(OCIAttrSet(subscrhp, 
                       OCI_HTYPE_SUBSCRIPTION, 
                       OTL_RCAST(dvoid*,&timeout),
                       0, 
                       OCI_ATTR_SUBSCR_TIMEOUT, 
                       errhp));
    
    check(OCISubscriptionRegister(svchp,&subscrhp,1,errhp,OCI_DEFAULT));
    
  }

  void unsubscribe(void)
  {
    if(!subscrhp) return;
    if(!db||(db&&!db->connected)) 
      throw otl_exception
        (otl_error_msg_32,
         otl_error_code_32);
    OCIError *errhp=db->get_connect_struct().get_errhp();
    OCISvcCtx *svchp=db->get_connect_struct().get_svchp();
    OCISubscriptionUnRegister(svchp,subscrhp,errhp,OCI_DEFAULT);
    OCIHandleFree(OTL_RCAST(dvoid*,subscrhp),OCI_HTYPE_SUBSCRIPTION);
    subscrhp=nullptr;
  }

  void associate_table(const char *table_name)
  {
    if(!db||(db&&!db->connected)) 
      throw otl_exception
        (otl_error_msg_32,
         otl_error_code_32);
    char sql_stmt[1024];
    OTL_STRCPY_S(sql_stmt,sizeof(sql_stmt),"select :i<int> from ");
    OTL_STRCAT_S(sql_stmt,sizeof(sql_stmt),table_name);
    int arg=0;
    otl_stream s(1,sql_stmt,*db);
    if(!s.get_shell() || !s.get_shell()->ss)
      throw otl_exception(db->get_connect_struct(),sql_stmt);
    OCIError *errhp=db->get_connect_struct().get_errhp();
    OCIStmt *stmthp=s.get_shell()->ss->get_cursor_struct().cda;
    check(OCIAttrSet(stmthp, 
                     OCI_HTYPE_STMT, 
                     subscrhp, 
                     0, 
                     OCI_ATTR_CHNF_REGHANDLE, 
                   errhp));
    s<<arg;
  }

  void associate_query(const char *stmt)
  {
    if(!db||(db&&!db->connected)) 
      throw otl_exception
        (otl_error_msg_32,
         otl_error_code_32);
    otl_stream s(1,stmt,*db);
    if(!s.get_shell() || !s.get_shell()->ss)
      throw otl_exception(db->get_connect_struct(),stmt);
    OCIError *errhp=db->get_connect_struct().get_errhp();
    OCIStmt *stmthp=s.get_shell()->ss->get_cursor_struct().cda;
    check(OCIAttrSet(stmthp, 
                     OCI_HTYPE_STMT, 
                     subscrhp, 
                     0, 
                     OCI_ATTR_CHNF_REGHANDLE, 
                   errhp));
    s<<0;
  }

protected:

  void check(ub4 ret_code)
  {
    if(ret_code!=OCI_SUCCESS) 
      throw otl_exception(db->get_connect_struct());
  }

  virtual void OnException(OTL_CONST_EXCEPTION otl_exception& e) = 0;
  virtual void OnDeRegistration(void) = 0;

  //--- DB events:
  virtual void OnStartup(void) = 0;
  virtual void OnInstanceShutdown(void) = 0;
  virtual void OnAnyInstanceShutdown(void) = 0;

  //--- Table events:
  virtual void OnTableInvalidate(text *table_name) = 0;
  virtual void OnTableAlter(text *table_name, bool all_rows=false) = 0;
  virtual void OnTableDrop(text *table_name, bool all_rows=false) = 0;
  virtual void OnTableChange(text *table_name, bool all_rows=false) = 0;

  //--- Row events:
  virtual void OnRowInsert( text *table_name, text *row_id ) = 0;
  virtual void OnRowUpdate( text *table_name, text *row_id ) = 0;
  virtual void OnRowDelete( text *table_name, text *row_id ) = 0;

protected:  
  otl_connect* db;

private:

  OCISubscription *subscrhp;

  void notification_callback
  (dvoid* /*payload*/, ub4 /*paylen*/, dvoid *desc, ub4 /*mode*/)
  {
    if(!db||(db&&!db->connected)) 
      return;
    ub4 num_rows = 0;
    OCIColl *row_changes=nullptr;
    dvoid *row_desc, **row_descp;
    dvoid*** temp_row_descp=&row_descp;
    text *row_id;
    ub4 rowid_size;
    unsigned int j;
    try{
      OCIEnv *envhp=db->get_connect_struct().get_envhp();
      OCIError *errhp=db->get_connect_struct().get_errhp();
      
      //----------------
      ub4 notify_type;
      check(OCIAttrGet(desc, 
                       OCI_DTYPE_CHDES, 
                       &notify_type, 
                       nullptr,
                       OCI_ATTR_CHDES_NFYTYPE, 
                       errhp));
      
      switch(notify_type){
      case OCI_EVENT_STARTUP: 
        OnStartup(); 
        return;
      case OCI_EVENT_SHUTDOWN: 
        OnInstanceShutdown(); 
        return;
      case OCI_EVENT_SHUTDOWN_ANY: 
        OnAnyInstanceShutdown(); 
        return;
      case OCI_EVENT_DEREG: 
        OnDeRegistration(); 
        return;
      case OCI_EVENT_OBJCHANGE: 
        break;
      default: 
        return;
      }

      OCIColl *table_changes=nullptr;
      check(OCIAttrGet(desc, 
                       OCI_DTYPE_CHDES, 
                       &table_changes, 
                       nullptr, 
                       OCI_ATTR_CHDES_TABLE_CHANGES, 
                       errhp));
      if(!table_changes)return;
      
      ub4 num_tables=0;
      check(OCICollSize(envhp, 
                        errhp, 
                        OTL_RCAST(CONST OCIColl*,table_changes),
                        OTL_RCAST(sb4*,&num_tables)));
      if(!num_tables)return;
      
      for(unsigned int i=0;i<num_tables;i++){
        int exist;
        dvoid *elemind=nullptr, *table_desc, **table_descp;
        dvoid*** temp_table_descp=&table_descp;
        check(OCICollGetElem(envhp, 
                             errhp, 
                             table_changes, 
                             i, 
                             &exist, 
                             OTL_RCAST(dvoid**,temp_table_descp),
                             &elemind));
        table_desc=*table_descp;
        text *table_name;
        check(OCIAttrGet(table_desc, 
                         OCI_DTYPE_TABLE_CHDES, 
                         &table_name, 
                         nullptr, 
                         OCI_ATTR_CHDES_TABLE_NAME, 
                         errhp));
        
        ub4 table_op;
        check(OCIAttrGet(table_desc, 
                         OCI_DTYPE_TABLE_CHDES, 
                         OTL_RCAST(dvoid*,&table_op),
                         nullptr, 
                         OCI_ATTR_CHDES_TABLE_OPFLAGS, 
                         errhp));
        bool all_rows=table_op & OCI_OPCODE_ALLROWS;
        switch(table_op){
        case OCI_OPCODE_ALLROWS:
          OnTableInvalidate(table_name);
          continue;
        case OCI_OPCODE_ALTER:
        case (OCI_OPCODE_ALTER+OCI_OPCODE_ALLROWS):
          OnTableAlter(table_name,all_rows); 
          continue;
        case OCI_OPCODE_DROP: 
        case (OCI_OPCODE_DROP+OCI_OPCODE_ALLROWS): 
          OnTableDrop(table_name,all_rows);
          continue;
        case (OCI_OPCODE_INSERT+OCI_OPCODE_ALLROWS):
        case (OCI_OPCODE_UPDATE+OCI_OPCODE_ALLROWS):
        case (OCI_OPCODE_DELETE+OCI_OPCODE_ALLROWS): 
        case (OCI_OPCODE_INSERT+OCI_OPCODE_UPDATE+OCI_OPCODE_ALLROWS):
        case (OCI_OPCODE_INSERT+OCI_OPCODE_DELETE+OCI_OPCODE_ALLROWS):
        case (OCI_OPCODE_UPDATE+OCI_OPCODE_DELETE+OCI_OPCODE_ALLROWS):
        case (OCI_OPCODE_INSERT+OCI_OPCODE_UPDATE+
              OCI_OPCODE_DELETE+OCI_OPCODE_ALLROWS):
        case (OCI_OPCODE_UNKNOWN+OCI_OPCODE_ALLROWS):
          OnTableChange(table_name,all_rows);
          continue;
        case OCI_OPCODE_INSERT:
        case OCI_OPCODE_UPDATE:
        case OCI_OPCODE_DELETE: 
          OnTableChange(table_name,all_rows); 
          break;
        }
        
        row_changes=nullptr;
        check(OCIAttrGet(table_desc,
                         OCI_DTYPE_TABLE_CHDES, 
                         &row_changes, 
                         nullptr, 
                         OCI_ATTR_CHDES_TABLE_ROW_CHANGES, 
                         errhp));
        if(!row_changes)continue;
        num_rows=0;
        check(OCICollSize(envhp,errhp,row_changes,OTL_RCAST(sb4*,&num_rows)));
        for(j=0;j<num_rows;j++){
          elemind=nullptr;
          check(OCICollGetElem(envhp, 
                               errhp, 
                               row_changes, 
                               j, 
                               &exist, 
                               OTL_RCAST(dvoid**,temp_row_descp), 
                               &elemind));
          row_desc=*row_descp;
          
          check(OCIAttrGet(row_desc, 
                           OCI_DTYPE_ROW_CHDES, 
                           OTL_RCAST(dvoid*,&row_id),
                           &rowid_size,
                           OCI_ATTR_CHDES_ROW_ROWID, 
                           errhp));
          if(table_op&OCI_OPCODE_INSERT)OnRowInsert(table_name,row_id); 
          if(table_op&OCI_OPCODE_DELETE)OnRowDelete(table_name,row_id); 
          if(table_op&OCI_OPCODE_UPDATE)OnRowUpdate(table_name,row_id);
        }
      }
    }catch(OTL_CONST_EXCEPTION otl_exception &e){
      OnException(e);
    }
  }

  static void common_notification_callback
  (dvoid *ctx, 
   OCISubscription* /*subscrhp*/, 
   dvoid *payload, 
   ub4 paylen, 
   dvoid *desc, 
   ub4 mode)
  {
    if(!ctx) return;
    (OTL_RCAST(otl_subscriber*,ctx))->notification_callback(payload,paylen,desc,mode);
  }

public:
  bool is_online(void){ return subscrhp!=nullptr; }

private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:
  otl_subscriber(const otl_subscriber&) = delete;
  otl_subscriber& operator=(const otl_subscriber&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_subscriber(otl_subscriber&&) = delete;
  otl_subscriber& operator=(otl_subscriber&&) = delete;
#endif
private:
#else
  otl_subscriber(const otl_subscriber&):
    db(nullptr),
    subscrhp(nullptr)
  {
  }

  otl_subscriber& operator=(const otl_subscriber&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_subscriber(otl_subscriber&&):
    db(nullptr),
    subscrhp(nullptr)
  {
  }

  otl_subscriber& operator=(otl_subscriber&&)
  {
    return *this;
  }
#endif
#endif

};

#endif

inline otl_connect& operator>>(otl_connect& connect, otl_stream& s)
{
  const char* cmd=connect.getCmd();
  const char* invalid_cmd="*** INVALID COMMAND ***";
  if(!cmd)
    cmd=invalid_cmd;
  s.open(s.getBufSize(),cmd,connect);
  return connect;
}

#if (defined(OTL_STL)||defined(OTL_VALUE_TEMPLATE_ON)) \
    && defined(OTL_VALUE_TEMPLATE)
template <OTL_TYPE_NAME TData>
otl_stream& operator<<(otl_stream& s, const otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
 if(var.ind)
  s<<otl_null();
 else
  s<<var.v;
 return s;
}

template <OTL_TYPE_NAME TData>
otl_stream& operator>>(otl_stream& s, otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
  s>>var.v;
  if(s.is_null())
    var.ind=true;
  else
    var.ind=false;
  return s;
}

template <OTL_TYPE_NAME TData>
otl_refcur_stream& operator>>(otl_refcur_stream& s, otl_value<TData>& var)
  OTL_THROWS_OTL_EXCEPTION
{
  s>>var.v;
  if(s.is_null())
    var.ind=true;
  else
    var.ind=false;
  return s;
}

#endif

typedef otl_tmpl_nocommit_stream
<otl_stream,
 otl_connect,
 otl_exception> otl_nocommit_stream;

inline otl_stream& endr(otl_stream& s)
{
  s.check_end_of_row();
  return s;
}

OTL_ORA8_NAMESPACE_END
#ifndef __STDC__DEFINED
#undef __STDC__
#endif
#endif

#if defined(OTL_STL) && !defined(OTL_STLPORT)

#define STL_INPUT_ITERATOR_TO_DERIVE_FROM 

#if defined(_MSC_VER) && (_MSC_VER >= 1600) && \
    defined(_SECURE_SCL) && (_SECURE_SCL == 1) && \
   !defined(OTL_STLPORT)

#define OTL_VC10_STL_OUTPUT_ITERATOR_HELPER(namespace_name)             \
_STD_BEGIN                                                              \
template<OTL_TYPE_NAME T>                                               \
struct _Is_checked_helper<namespace_name otl_output_iterator<T> >       \
: public _STD tr1::true_type{};                                         \
_STD_END

#define STL_OUTPUT_ITERATOR_TO_DERIVE_FROM : public _STD _Outit

#else
#define STL_OUTPUT_ITERATOR_TO_DERIVE_FROM
#define OTL_VC10_STL_OUTPUT_ITERATOR_HELPER(namespace_name)

#endif

#elif defined(OTL_STLPORT)
#define OTL_VC10_STL_OUTPUT_ITERATOR_HELPER(namespace_name)

#define STL_INPUT_ITERATOR_TO_DERIVE_FROM       \
  : public STD_NAMESPACE_PREFIX iterator        \
    <STD_NAMESPACE_PREFIX input_iterator_tag,   \
     T,Distance,T*,T&>

#define STL_OUTPUT_ITERATOR_TO_DERIVE_FROM              \
  : public STD_NAMESPACE_PREFIX iterator                \
         <STD_NAMESPACE_PREFIX output_iterator_tag,     \
           T,void,void,void>

#endif

#if defined(OTL_STL) || defined(OTL_STLPORT)

#if defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__*100+__GNUC_MINOR__>=406)
#define OTL_ITER_DISTANCE Distance=STD_NAMESPACE_PREFIX ptrdiff_t
#else
#define OTL_ITER_DISTANCE Distance=ptrdiff_t
#endif

#define OTL_ITERATORS                                                   \
template <OTL_TYPE_NAME T, OTL_TYPE_NAME OTL_ITER_DISTANCE>             \
class otl_input_iterator STL_INPUT_ITERATOR_TO_DERIVE_FROM {            \
public:                                                                 \
                                                                        \
  typedef STD_NAMESPACE_PREFIX input_iterator_tag iterator_category;    \
  typedef T                  value_type;                                \
  typedef Distance           difference_type;                           \
  typedef const T*           pointer;                                   \
  typedef const T&           reference;                                 \
                                                                        \
 otl_stream* stream;                                                    \
 T value;                                                               \
 int end_marker;                                                        \
                                                                        \
 void read()                                                            \
 {                                                                      \
  if(!stream){                                                          \
   end_marker=-1;                                                       \
   return;                                                              \
  }                                                                     \
  if(stream->eof()){                                                    \
   end_marker=-1;                                                       \
   return;                                                              \
  }                                                                     \
  end_marker=stream->eof();                                             \
  if(!end_marker)*stream>>value;                                        \
  if(stream->eof())end_marker=1;                                        \
 }                                                                      \
                                                                        \
 otl_input_iterator() : stream(nullptr), value(), end_marker(-1){}            \
 otl_input_iterator(otl_stream& s) : stream(&s), value(),end_marker(0){read();} \
                                                                        \
 const T& operator*() const { return value; }                           \
                                                                        \
 otl_input_iterator<T, Distance>& operator++(){read(); return *this;}   \
                                                                        \
 otl_input_iterator<T, Distance> operator++(int)                        \
 {                                                                      \
  otl_input_iterator<T, Distance> tmp = *this;                          \
  read();                                                               \
  return tmp;                                                           \
 }                                                                      \
                                                                        \
   otl_input_iterator(const otl_input_iterator& src):                   \
   stream(src.stream),value(src.value),end_marker(src.end_marker){}     \
                                                                        \
  otl_input_iterator& operator=(const otl_input_iterator& src)          \
  {                                                                     \
    stream=src.stream;                                                  \
    value=src.value;                                                    \
    end_marker=src.end_marker;                                          \
    return *this;                                                       \
  }                                                                     \
};                                                                      \
                                                                        \
template <OTL_TYPE_NAME T, OTL_TYPE_NAME Distance>                      \
inline STD_NAMESPACE_PREFIX input_iterator_tag iterator_category(       \
  const otl_input_iterator<T, Distance>&                                \
)                                                                       \
{                                                                       \
  return STD_NAMESPACE_PREFIX input_iterator_tag();                     \
}                                                                       \
                                                                        \
template <OTL_TYPE_NAME T, OTL_TYPE_NAME Distance>                      \
inline T* value_type(const otl_input_iterator<T, Distance>&)            \
{                                                                       \
 return nullptr;                                                        \
}                                                                       \
                                                                        \
template <OTL_TYPE_NAME T, OTL_TYPE_NAME Distance>                      \
inline Distance* distance_type(const otl_input_iterator<T, Distance>&)  \
{                                                                       \
 return nullptr;                                                        \
}                                                                       \
                                                                        \
template <OTL_TYPE_NAME T, OTL_TYPE_NAME Distance>                      \
bool operator==(const otl_input_iterator<T, Distance>& x,               \
                const otl_input_iterator<T, Distance>& y)               \
{                                                                       \
  return (x.stream == y.stream && x.end_marker == y.end_marker) ||      \
    (x.end_marker == -1 && y.end_marker == -1);                         \
}                                                                       \
                                                                        \
template <OTL_TYPE_NAME T, OTL_TYPE_NAME Distance>                      \
bool operator!=(const otl_input_iterator<T, Distance>& x,               \
                const otl_input_iterator<T, Distance>& y)               \
{                                                                       \
 return !(x==y);                                                        \
}                                                                       \
                                                                        \
template <OTL_TYPE_NAME T>                                              \
class otl_output_iterator STL_OUTPUT_ITERATOR_TO_DERIVE_FROM {          \
protected:                                                              \
 otl_stream* stream;                                                    \
public:                                                                 \
                                                                        \
  typedef STD_NAMESPACE_PREFIX output_iterator_tag iterator_category;   \
  typedef void       value_type;                                        \
  typedef void       difference_type;                                   \
  typedef void       pointer;                                           \
  typedef void       reference;                                         \
                                                                        \
 otl_output_iterator(otl_stream& s) : stream(&s){}                      \
 otl_output_iterator<T>& operator=(const T& value)                      \
 {                                                                      \
  *stream << value;                                                     \
  return *this;                                                         \
 }                                                                      \
 otl_output_iterator<T>& operator*() { return *this; }                  \
 otl_output_iterator<T>& operator++() { return *this; }                 \
 otl_output_iterator<T> operator++(int) { return *this; }               \
                                                                        \
};

#define OTL_ITERATOR_TAG(namespace_name)                                \
template <OTL_TYPE_NAME T>                                              \
inline STD_NAMESPACE_PREFIX output_iterator_tag                         \
iterator_category(const namespace_name otl_output_iterator<T>&) {       \
  return STD_NAMESPACE_PREFIX output_iterator_tag();                    \
}                                                                      
                                                                        
#if defined(OTL_ORA7)
OTL_ORA7_NAMESPACE_BEGIN
OTL_ITERATORS
OTL_ORA7_NAMESPACE_END
OTL_VC10_STL_OUTPUT_ITERATOR_HELPER(OTL_ORA7_NAMESPACE_PREFIX)
OTL_ITERATOR_TAG(OTL_ORA7_NAMESPACE_PREFIX)
#endif

#if defined(OTL_ORA8)
OTL_ORA8_NAMESPACE_BEGIN
OTL_ITERATORS
OTL_ORA8_NAMESPACE_END
OTL_VC10_STL_OUTPUT_ITERATOR_HELPER(OTL_ORA8_NAMESPACE_PREFIX)                                    
OTL_ITERATOR_TAG(OTL_ORA8_NAMESPACE_PREFIX)
#endif

#if defined(OTL_ODBC)
OTL_ODBC_NAMESPACE_BEGIN
OTL_ITERATORS
OTL_ODBC_NAMESPACE_END
OTL_VC10_STL_OUTPUT_ITERATOR_HELPER(OTL_ODBC_NAMESPACE_PREFIX)
OTL_ITERATOR_TAG(OTL_ODBC_NAMESPACE_PREFIX)
#endif

#endif

#if defined(OTL_STREAM_READ_ITERATOR_ON)

#if defined(OTL_UNICODE)
#error UNICODE is not supported when #define OTL_STREAM_READ_ITERATOR_ON is enabled
#endif

#if defined(OTL_STL)
#include <map>
#endif

#if defined(OTL_ACE)
#include <ace/SString.h>
#include <ace/Array.h>
#include <ace/Functor.h>
#include <ace/RB_Tree.h>
#include <ace/Null_Mutex.h>
#endif

#if defined(OTL_STL)||defined(OTL_ACE)
class otl_ltcharstar{
public:
#if defined(OTL_STL)
 bool 
#else
 int
#endif
 operator()(const char* s1, const char* s2) const
 {
#if defined(__BORLANDC__) || defined(_MSC_VER)
   return stricmp(s1,s2)<0;
#else
#if defined(__STRICT_ANSI__)
   while(otl_to_upper(*s1)==otl_to_upper(*s2)&&*s1){
     ++s1; 
     ++s2;
   }
   return *s1 < *s2;
#else
  return strcasecmp(s1,s2)<0;
#endif
#endif
 }
};
#endif
template<OTL_TYPE_NAME OTLStream,
         OTL_TYPE_NAME OTLException
#if !defined(OTL_ORA7)
         ,OTL_TYPE_NAME OTLLobStream
#endif
         >
class otl_stream_read_iterator{
public:

  otl_stream_read_iterator(OTLStream& s)
  {
    set();
    attach(s);
  }

  otl_stream_read_iterator():
    out_vars_(nullptr),
    out_vars_len_(0),
    str_(nullptr),
    out_vars_arr_(nullptr),
    out_vars_null_arr_(nullptr),
    out_vars_constructed_(nullptr),
    lob_stream_mode_flag_(false)
#if defined(OTL_STL)
    ,var_name2pos_map_()
#endif
#if defined(OTL_ACE)
    ,var_name2pos_map_()
#endif
  {
    set();
  }

  ~otl_stream_read_iterator()
  {
    reset();
  }

  void attach(OTLStream& s)
  {
    reset();
    str_=&s;
    if(!str_->good()){
      str_=nullptr;
      throw OTLException(otl_error_msg_19,otl_error_code_19);
    }
    out_vars_=str_->describe_out_vars(out_vars_len_);
    if(!out_vars_){
      throw OTLException(otl_error_msg_21,otl_error_code_21);
    }
    lob_stream_mode_flag_=str_->get_lob_stream_flag();
    allocate_arrays();
  }

  void reattach()
  {
    if(!str_->good()){
      reset();
      throw OTLException(otl_error_msg_19,otl_error_code_19);
    }
    out_vars_=str_->describe_out_vars(out_vars_len_);
    if(!out_vars_){
      reset();
      throw OTLException(otl_error_msg_21,otl_error_code_21);
    }else{
#if defined(OTL_STL)
      var_name2pos_map_.clear();
      for(int i=0;i<out_vars_len_;++i){
        const otl_var_desc& curr_var=out_vars_[i];
        var_name2pos_map_[curr_var.name]=i;
      }
#endif
#if defined(OTL_ACE)
      var_name2pos_map_.close();
      for(int i=0;i<out_vars_len_;++i){
        const otl_var_desc& curr_var=out_vars_[i];
        var_name2pos_map_.bind(curr_var.name,i);
      }
#endif
    }
    lob_stream_mode_flag_=str_->get_lob_stream_flag();
  }


  void detach(void)
  {
    reset();
  }

  const otl_var_desc* describe(int& var_desc_len)
  {
    var_desc_len=out_vars_len_;
    return out_vars_;
  }

  bool next_row(void)
  {
    if(str_->eof())return false;
    for(int i=0;i<out_vars_len_;++i){
      otl_var_desc& curr_var=out_vars_[i];
      unsigned char* curr_ptr=out_vars_arr_[i];
      switch(curr_var.ftype){
      case otl_var_char:
        (*str_)>>OTL_RCAST(char*,curr_ptr);
        break;
      case otl_var_double:
        (*str_)>>*OTL_RCAST(double*,curr_ptr);
        break;
      case otl_var_float:
        (*str_)>>*OTL_RCAST(float*,curr_ptr);
        break;
      case otl_var_int:
        (*str_)>>*OTL_RCAST(int*,curr_ptr);
        break;
      case otl_var_unsigned_int:
        (*str_)>>*OTL_RCAST(unsigned*,curr_ptr);
        break;
      case otl_var_short:
        (*str_)>>*OTL_RCAST(short int*,curr_ptr);
        break;
      case otl_var_long_int:
        (*str_)>>*OTL_RCAST(long int*,curr_ptr);
        break;
      case otl_var_raw:
        (*str_)>>*OTL_RCAST(otl_long_string*,curr_ptr);
        break;
      case otl_var_timestamp:
      case otl_var_db2time:
      case otl_var_db2date:
      case otl_var_tz_timestamp:
      case otl_var_ltz_timestamp:
        (*str_)>>*OTL_RCAST(otl_datetime*,curr_ptr);
        break;
      case otl_var_varchar_long:
      case otl_var_raw_long:
      case otl_var_clob:
      case otl_var_blob:
#if !defined(OTL_ORA7)
        if(lob_stream_mode_flag_)
          (*str_)>>*OTL_RCAST(OTLLobStream*,curr_ptr);
        else
#endif
          (*str_)>>*OTL_RCAST(otl_long_string*,curr_ptr);
        break;
#if defined(OTL_BIGINT)
      case otl_var_bigint:
        (*str_)>>*OTL_RCAST(OTL_BIGINT*,curr_ptr);
        break;
#endif
#if defined(OTL_UBIGINT)
      case otl_var_ubigint:
        (*str_)>>*OTL_RCAST(OTL_UBIGINT*,curr_ptr);
        break;
#endif
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
      case otl_var_numeric_type_1:
        (*str_)>>*OTL_RCAST(OTL_NUMERIC_TYPE_1*,curr_ptr);
        break;
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
      case otl_var_numeric_type_2:
        (*str_)>>*OTL_RCAST(OTL_NUMERIC_TYPE_2*,curr_ptr);
        break;
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
      case otl_var_numeric_type_3:
        (*str_)>>*OTL_RCAST(OTL_NUMERIC_TYPE_3*,curr_ptr);
        break;
#endif
      }
      out_vars_null_arr_[i]=str_->is_null()==1;
    }
    return true;
  }

#if !defined(OTL_ORA7)
  void get(const int pos, OTLLobStream*& s)
  {
    check_pos(pos);
    check_type(pos,otl_var_long_string,true);
    if(!lob_stream_mode_flag_){
      char var_info[255];
      otl_var_info_var3
        (out_vars_[pos-1].name,
         out_vars_[pos-1].ftype,
         otl_var_lob_stream,
         var_info,
         sizeof(var_info));
      throw OTLException
        (otl_error_msg_25,
         otl_error_code_25,
         str_->get_stm_text(),
         var_info);
    }
    unsigned char* curr_ptr=out_vars_arr_[pos-1];
    s=OTL_RCAST(OTLLobStream*,curr_ptr);
  }
#endif

#if defined(OTL_STL) && !defined(OTL_ORA7)
  void get(const char* var_name,OTLLobStream*& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE) && !defined(OTL_ORA7)
  void get(const char* var_name,OTLLobStream*& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

  void get(const int pos, char& c)
  {
    check_pos(pos);
    check_type(pos,otl_var_char);
    unsigned char* curr_ptr=out_vars_arr_[pos-1];
    c=OTL_SCAST(char,*curr_ptr);
  }

#if defined(OTL_STL)
  void get(const char* var_name, char& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, char& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

  void get(const int pos, unsigned char& c)
  {
    check_pos(pos);
    check_type(pos,otl_var_char);
    unsigned char* curr_ptr=out_vars_arr_[pos-1];
    c=*curr_ptr;
  }

#if defined(OTL_STL)
  void get(const char* var_name, unsigned char& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, unsigned char& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

  void get(const int pos, char* s)
  {
    check_pos(pos);
    check_type(pos,otl_var_char);
    unsigned char* curr_ptr=out_vars_arr_[pos-1];
    otl_strcpy(OTL_RCAST(unsigned char*,s),
               OTL_RCAST(const unsigned char*,curr_ptr));
  }

#if defined(OTL_STL)
  void get(const char* var_name, char* n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, char* n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

  void get(const int pos, unsigned char* s)
  {
    check_pos(pos);
    check_type(pos,otl_var_char);
    unsigned char* curr_ptr=out_vars_arr_[pos-1];
    otl_strcpy(OTL_RCAST(unsigned char*,s),OTL_RCAST(const unsigned char*,curr_ptr));
  }

#if defined(OTL_STL)
  void get(const char* var_name, unsigned char* n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, unsigned char* n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif


  void get(const int pos, unsigned int& n)
  {
    check_pos(pos);
    void* curr_ptr=out_vars_arr_[pos-1];
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T<unsigned,otl_var_unsigned_int>
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#else
    int match_found=otl_numeric_convert_T
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#endif
    if(match_found)return;
    check_type(pos,otl_var_unsigned_int);
  }

#if defined(OTL_STL)
  void get(const char* var_name, unsigned int& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, unsigned int& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

  void get(const int pos, int& n)
  {
    check_pos(pos);
    void* curr_ptr=out_vars_arr_[pos-1];
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T<int,otl_var_int>
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#else
    int match_found=otl_numeric_convert_T
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#endif
    if(match_found)return;
    check_type(pos,otl_var_unsigned_int);
  }

#if defined(OTL_STL)
  void get(const char* var_name, int& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, int& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

  void get(const int pos, short int& n)
  {
    check_pos(pos);
    void* curr_ptr=out_vars_arr_[pos-1];
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T<short,otl_var_short>
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#else
    int match_found=otl_numeric_convert_T
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#endif
    if(match_found)return;
    check_type(pos,otl_var_short);
  }

#if defined(OTL_STL)
  void get(const char* var_name, short int& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, short int& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

  void get(const int pos, long int& n)
  {
    check_pos(pos);
    void* curr_ptr=out_vars_arr_[pos-1];
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T<long int,otl_var_long_int>
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#else
    int match_found=otl_numeric_convert_T
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#endif
    if(match_found)return;
    check_type(pos,otl_var_long_int);
  }

#if defined(OTL_STL)
  void get(const char* var_name, long int& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, long int& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

  void get(const int pos, float& n)
  {
    check_pos(pos);
    void* curr_ptr=out_vars_arr_[pos-1];
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T<float,otl_var_float>
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#else
    int match_found=otl_numeric_convert_T
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#endif
    if(match_found)return;
    check_type(pos,otl_var_double);
  }

#if defined(OTL_STL)
  void get(const char* var_name, float& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, float& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

  void get(const int pos, double& n)
  {
    check_pos(pos);
    void* curr_ptr=out_vars_arr_[pos-1];
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T<double,otl_var_double>
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#else
    int match_found=otl_numeric_convert_T
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#endif
    if(match_found)return;
    check_type(pos,otl_var_double);
  }

#if defined(OTL_STL)
  void get(const char* var_name, double& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, double& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif


#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
  void get(const int pos, OTL_NUMERIC_TYPE_1& n)
  {
    check_pos(pos);
    void* curr_ptr=out_vars_arr_[pos-1];
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T<OTL_NUMERIC_TYPE_1,otl_var_numeric_type_1>
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#else
    int match_found=otl_numeric_convert_T
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#endif
    if(match_found)return;
#if defined(OTL_STR_TO_NUMERIC_TYPE_1) && defined(OTL_NUMERIC_TYPE_1_TO_STR)
    if(out_vars_[pos-1].ftype==otl_var_char){
      char* temp_val=OTL_RCAST(char*,curr_ptr);
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
      if(is_null(pos)){
        n=OTL_SCAST(OTL_NUMERIC_TYPE_1,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
        return;
      }
#endif
      OTL_STR_TO_NUMERIC_TYPE_1(temp_val,n);
      return;
    }
    check_type(pos,otl_var_numeric_type_1);
#else
    check_type(pos,otl_var_numeric_type_1);
#endif
  }

#if defined(OTL_STL)
  void get(const char* var_name, OTL_NUMERIC_TYPE_1& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, OTL_NUMERIC_TYPE_1& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

#endif

#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
  void get(const int pos, OTL_NUMERIC_TYPE_2& n)
  {
    check_pos(pos);
    void* curr_ptr=out_vars_arr_[pos-1];
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T<OTL_NUMERIC_TYPE_2,otl_var_numeric_type_2>
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#else
    int match_found=otl_numeric_convert_T
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#endif
    if(match_found)return;
#if defined(OTL_STR_TO_NUMERIC_TYPE_2) && defined(OTL_NUMERIC_TYPE_2_TO_STR)
    if(out_vars_[pos-1].ftype==otl_var_char){
      char* temp_val=OTL_RCAST(char*,curr_ptr);
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
      if(is_null(pos)){
        n=OTL_SCAST(OTL_NUMERIC_TYPE_2,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
        return;
      }
#endif
      OTL_STR_TO_NUMERIC_TYPE_2(temp_val,n);
      return;
    }
    check_type(pos,otl_var_numeric_type_2);
#else
    check_type(pos,otl_var_numeric_type_2);
#endif
  }

#if defined(OTL_STL)
  void get(const char* var_name, OTL_NUMERIC_TYPE_2& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, OTL_NUMERIC_TYPE_2& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

#endif

#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
  void get(const int pos, OTL_NUMERIC_TYPE_3& n)
  {
    check_pos(pos);
    void* curr_ptr=out_vars_arr_[pos-1];
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T<OTL_NUMERIC_TYPE_3,otl_var_numeric_type_3>
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#else
    int match_found=otl_numeric_convert_T
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#endif
    if(match_found)return;
#if defined(OTL_STR_TO_NUMERIC_TYPE_3) && defined(OTL_NUMERIC_TYPE_3_TO_STR)
    if(out_vars_[pos-1].ftype==otl_var_char){
      char* temp_val=OTL_RCAST(char*,curr_ptr);
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
      if(is_null(pos)){
        n=OTL_SCAST(OTL_NUMERIC_TYPE_3,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
        return;
      }
#endif
      OTL_STR_TO_NUMERIC_TYPE_3(temp_val,n);
      return;
    }
    check_type(pos,otl_var_numeric_type_3);
#else
    check_type(pos,otl_var_numeric_type_3);
#endif
  }

#if defined(OTL_STL)
  void get(const char* var_name, OTL_NUMERIC_TYPE_3& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, OTL_NUMERIC_TYPE_3& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

#endif

#if defined(OTL_BIGINT)
  void get(const int pos, OTL_BIGINT& n)
  {
    check_pos(pos);
    void* curr_ptr=out_vars_arr_[pos-1];
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T<OTL_BIGINT,otl_var_bigint>
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#else
    int match_found=otl_numeric_convert_T
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#endif
    if(match_found)return;
#if defined(OTL_STR_TO_BIGINT) && defined(OTL_BIGINT_TO_STR)
    if(out_vars_[pos-1].ftype==otl_var_char){
      char* temp_val=OTL_RCAST(char*,curr_ptr);
#if defined(OTL_DEFAULT_NUMERIC_NULL_TO_VAL)
      if(is_null(pos)){
        n=OTL_SCAST(OTL_BIGINT,OTL_DEFAULT_NUMERIC_NULL_TO_VAL);
        return;
      }
#endif
      OTL_STR_TO_BIGINT(temp_val,n);
      return;
    }
    check_type(pos,otl_var_bigint);
#else
    check_type(pos,otl_var_bigint);
#endif
  }


#if defined(OTL_STL)
  void get(const char* var_name, OTL_BIGINT& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, OTL_BIGINT& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

#endif

#if defined(OTL_UBIGINT)
  void get(const int pos, OTL_UBIGINT& n)
  {
    check_pos(pos);
    void* curr_ptr=out_vars_arr_[pos-1];
#if defined(OTL_STRICT_NUMERIC_TYPE_CHECK_ON_SELECT)
    int match_found=otl_numeric_convert_T<OTL_UBIGINT,otl_var_ubigint>
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#else
    int match_found=otl_numeric_convert_T
      (out_vars_[pos-1].ftype,
       curr_ptr,
       n);
#endif
    if(match_found)return;
    check_type(pos,otl_var_ubigint);
  }


#if defined(OTL_STL)
  void get(const char* var_name, OTL_UBIGINT& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, OTL_UBIGINT& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

#endif

  bool is_null(const int pos)
  {
    check_pos(pos);
    return out_vars_null_arr_[pos-1];
  }

#if defined(OTL_STL)
  bool is_null(const char* var_name)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    return is_null((*it).second+1);
  }
#endif

#if defined(OTL_ACE)
  bool is_null(const char* var_name)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    return is_null(it->item()+1);
  }
#endif

#if defined(OTL_STL) || defined(USER_DEFINED_STRING_CLASS)
  void get(const int pos, OTL_STRING_CONTAINER& s)
  {
    check_pos(pos);
    otl_var_desc& curr_var=out_vars_[pos-1];
    unsigned char* curr_ptr=out_vars_arr_[pos-1];
    switch(curr_var.ftype){
    case otl_var_varchar_long:
    case otl_var_raw_long:
    case otl_var_clob:
    case otl_var_blob:
      {
        otl_long_string* ls=OTL_RCAST(otl_long_string*,curr_ptr);
        int len=ls->len();
        s.assign(OTL_RCAST(char*,ls->v),len);
      }
      break;
    case otl_var_char:
      s=OTL_RCAST(char*,curr_ptr);
      break;
    default:
      {
        char var_info[255];
        otl_var_info_var3
          (out_vars_[pos-1].name,
           out_vars_[pos-1].ftype,
           otl_var_char,
           var_info,
           sizeof(var_info));
        throw OTLException
          (otl_error_msg_23,
           otl_error_code_23,
           str_->get_stm_text(),
           var_info);
      }
    }
  }
#endif

#if defined(OTL_STL)
  void get(const char* var_name, OTL_STRING_CONTAINER& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, OTL_STRING_CONTAINER& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif


  void get(const int pos, otl_long_string& s)
  {
    check_pos(pos);
    check_type(pos,otl_var_long_string);
    unsigned char* curr_ptr=out_vars_arr_[pos-1];
    s=*OTL_RCAST(otl_long_string*,curr_ptr);
  }

#if defined(OTL_STL)
  void get(const char* var_name, otl_long_string& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, otl_long_string& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

  void get(const int pos, otl_long_string*& s)
  {
    check_pos(pos);
    check_type(pos,otl_var_long_string);
    unsigned char* curr_ptr=out_vars_arr_[pos-1];
    s=OTL_RCAST(otl_long_string*,curr_ptr);
  }

#if defined(OTL_STL)
  void get(const char* var_name, otl_long_string*& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, otl_long_string*& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

  void get(const int pos, otl_datetime& s)
  {
    check_pos(pos);
    check_type(pos,otl_var_timestamp);
    unsigned char* curr_ptr=out_vars_arr_[pos-1];
    s=*OTL_RCAST(otl_datetime*,curr_ptr);
  }

#if defined(OTL_STL)
  void get(const char* var_name, otl_datetime& n)
  {
    var_name2pos_map_type::iterator it=var_name2pos_map_.find(var_name);
    check_name(it,var_name);
    get((*it).second+1,n);
  }
#endif

#if defined(OTL_ACE)
  void get(const char* var_name, otl_datetime& n)
  {
    var_name2pos_map_type::ENTRY* it=nullptr;
    var_name2pos_map_.find(var_name,it);
    check_name(it,var_name);
    get(it->item()+1,n);
  }
#endif

protected:

  otl_var_desc* out_vars_;
  int out_vars_len_;
  OTLStream* str_;
  unsigned char** out_vars_arr_;
  bool* out_vars_null_arr_;
  bool out_vars_constructed_;
  bool lob_stream_mode_flag_;

#if defined(OTL_STL)
  typedef STD_NAMESPACE_PREFIX 
    map<const char*,int,otl_ltcharstar> var_name2pos_map_type;
  var_name2pos_map_type var_name2pos_map_;
#endif

#if defined(OTL_ACE)
  typedef
  ACE_RB_Tree<const char*,
              int,
              otl_ltcharstar,
              ACE_Null_Mutex> var_name2pos_map_type;
  var_name2pos_map_type var_name2pos_map_;
#endif

  void check_pos(const int pos)
  {
    int actual_pos=pos-1;
    if(actual_pos<0 || actual_pos>out_vars_len_-1){
      throw OTLException
        (otl_error_msg_22,
         otl_error_code_22,
         str_->get_stm_text());
    }
  }

#if defined(OTL_STL)
  void check_name(var_name2pos_map_type::iterator& it, const char* var_name)
  {
    if(it==var_name2pos_map_.end())
      throw OTLException
        (otl_error_msg_26,
         otl_error_code_26,
         str_->get_stm_text(), 
         var_name);
  }
#endif

#if defined(OTL_ACE)
  void check_name(var_name2pos_map_type::ENTRY* it,const char* var_name)
  {
    if(!it){
      throw OTLException
        (otl_error_msg_26,
         otl_error_code_26,
         str_->get_stm_text(), 
         var_name);
    }
  }
#endif

  void check_type(const int pos, 
                  const int type_code, 
                  const bool lob_stream_arg=false)
  {
    switch(out_vars_[pos-1].ftype){
    case otl_var_timestamp:
    case otl_var_tz_timestamp:
    case otl_var_ltz_timestamp:
      if(type_code==otl_var_timestamp)
        return;
      break;
    case otl_var_varchar_long:
    case otl_var_raw_long:
    case otl_var_clob:
    case otl_var_blob:
      if(type_code==otl_var_long_string)
        return;
      break;
    case otl_var_raw:
      if(type_code==otl_var_long_string && 
         lob_stream_mode_flag_ &&
         lob_stream_arg){
        char var_info1[255];
        otl_var_info_var4
          (out_vars_[pos-1].name,
           out_vars_[pos-1].ftype,
           otl_var_lob_stream,
           var_info1,
           sizeof(var_info1));
        throw OTLException
          (otl_error_msg_28,
           otl_error_code_28,
           str_->get_stm_text(),
           var_info1);
      }else
        return;
    default:
      if(out_vars_[pos-1].ftype==type_code)
        return;
      break;
    }
    char var_info2[255];
    otl_var_info_var3
      (out_vars_[pos-1].name,
       out_vars_[pos-1].ftype,
       type_code,
       var_info2,
       sizeof(var_info2));
    throw OTLException
      (otl_error_msg_23,
       otl_error_code_23,
       str_->get_stm_text(),
       var_info2);
  }

  void set(void)
  {
    out_vars_=nullptr;
    out_vars_len_=0;
    str_=nullptr;
    out_vars_arr_=nullptr;
    out_vars_null_arr_=nullptr;
    out_vars_constructed_=false;
    lob_stream_mode_flag_=false;
  }

  void reset(void)
  {
    if(out_vars_constructed_){
      for(int i=0;i<out_vars_len_;++i){
        switch(out_vars_[i].ftype){
        case otl_var_char:
          delete[] OTL_RCAST(char*,out_vars_arr_[i]);
          break;
        case otl_var_double:
          delete OTL_RCAST(double*,out_vars_arr_[i]);
          break;
        case otl_var_float:
          delete OTL_RCAST(float*,out_vars_arr_[i]);
          break;
        case otl_var_int:
          delete OTL_RCAST(int*,out_vars_arr_[i]);
          break;
        case otl_var_unsigned_int:
          delete OTL_RCAST(unsigned*,out_vars_arr_[i]);
          break;
        case otl_var_short:
          delete OTL_RCAST(short int*,out_vars_arr_[i]);
          break;
        case otl_var_long_int:
          delete OTL_RCAST(long int*,out_vars_arr_[i]);
          break;
#if defined(OTL_BIGINT)
        case otl_var_bigint:
          delete OTL_RCAST(OTL_BIGINT*,out_vars_arr_[i]);
          break;
#endif
#if defined(OTL_UBIGINT)
        case otl_var_ubigint:
          delete OTL_RCAST(OTL_UBIGINT*,out_vars_arr_[i]);
          break;
#endif
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
        case otl_var_numeric_type_1:
          delete OTL_RCAST(OTL_NUMERIC_TYPE_1*,out_vars_arr_[i]);
          break;
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
        case otl_var_numeric_type_2:
          delete OTL_RCAST(OTL_NUMERIC_TYPE_2*,out_vars_arr_[i]);
          break;
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
        case otl_var_numeric_type_3:
          delete OTL_RCAST(OTL_NUMERIC_TYPE_3*,out_vars_arr_[i]);
          break;
#endif
        case otl_var_raw:
          delete OTL_RCAST(otl_long_string*,out_vars_arr_[i]);
          break;
        case otl_var_varchar_long:
        case otl_var_raw_long:
        case otl_var_clob:
        case otl_var_blob:
#if !defined(OTL_ORA7)
          if(lob_stream_mode_flag_)
            delete OTL_RCAST(OTLLobStream*,out_vars_arr_[i]);
          else
#endif
            delete OTL_RCAST(otl_long_string*,out_vars_arr_[i]);
          break;
        case otl_var_timestamp:
          delete OTL_RCAST(otl_datetime*,out_vars_arr_[i]);
          break;
        default:
          break;
        }
        out_vars_arr_[i]=nullptr;
      }
      out_vars_constructed_=false;
    }
    delete[] out_vars_arr_;
    delete[] out_vars_null_arr_;
#if defined(OTL_STL)||defined(OTL_ACE)
    var_name2pos_map_.clear();
#endif
    set();
  }

  int calculate_buffer_size
  (const otl_var_desc* var,
   const int vars_len)
  {
    for(int i=0;i<vars_len;++i){
      const otl_var_desc& curr_var=var[i];
      if(curr_var.ftype==otl_var_refcur||curr_var.pl_tab_flag)
        throw OTLException(otl_error_msg_20,otl_error_code_20);
    }
    return vars_len;
  }
  
  void allocate_arrays(void)
  {
    if(out_vars_){
      out_vars_null_arr_=new bool[out_vars_len_];
      int buf_size=calculate_buffer_size(out_vars_,out_vars_len_);
      out_vars_arr_=new unsigned char*[buf_size];
      construct_elements();
    }
  }

  void construct_elements(void)
  {
    for(int i=0;i<out_vars_len_;++i){
      out_vars_null_arr_[i]=true;
      const otl_var_desc& curr_var=out_vars_[i];
      switch(curr_var.ftype){
      case otl_var_char:
        {
          char* ptr=new char[curr_var.elem_size];
          *ptr=0;
          out_vars_arr_[i]=OTL_RCAST(unsigned char*,ptr);
        }
        break;
      case otl_var_raw:
        out_vars_arr_[i]=
          OTL_RCAST(unsigned char*,new otl_long_string(curr_var.elem_size));
        break;
      case otl_var_double:
        out_vars_arr_[i]=OTL_RCAST(unsigned char*,new double(0));
        break;
      case otl_var_float:
        out_vars_arr_[i]=OTL_RCAST(unsigned char*,new float(0));
        break;
      case otl_var_int:
        out_vars_arr_[i]=OTL_RCAST(unsigned char*,new int(0));
        break;
      case otl_var_unsigned_int:
        out_vars_arr_[i]=OTL_RCAST(unsigned char*,new unsigned(0));
        break;
      case otl_var_short:
        out_vars_arr_[i]=OTL_RCAST(unsigned char*,new short(0));
        break;
      case otl_var_long_int:
        out_vars_arr_[i]=OTL_RCAST(unsigned char*,new long(0));
        break;
      case otl_var_timestamp:
      case otl_var_db2time:
      case otl_var_db2date:
      case otl_var_tz_timestamp:
      case otl_var_ltz_timestamp:
        out_vars_arr_[i]=OTL_RCAST(unsigned char*,new otl_datetime);
        break;
      case otl_var_varchar_long:
      case otl_var_raw_long:
      case otl_var_clob:
      case otl_var_blob:
#if !defined(OTL_ORA7)
        if(lob_stream_mode_flag_)
          out_vars_arr_[i]=OTL_RCAST(unsigned char*,new OTLLobStream());
        else
#endif
          out_vars_arr_[i]=
            OTL_RCAST(unsigned char*,
                      new otl_long_string
                      (str_->get_adb_max_long_size()));
        break;
#if defined(OTL_BIGINT)
      case otl_var_bigint:
        out_vars_arr_[i]=OTL_RCAST(unsigned char*,new OTL_BIGINT(0));
        break;
#endif
#if defined(OTL_UBIGINT)
      case otl_var_ubigint:
        out_vars_arr_[i]=OTL_RCAST(unsigned char*,new OTL_UBIGINT(0));
        break;
#endif
#if defined(OTL_NUMERIC_TYPE_1) && defined(OTL_STR_TO_NUMERIC_TYPE_1) && \
    defined(OTL_NUMERIC_TYPE_1_TO_STR) && defined(OTL_NUMERIC_TYPE_1_ID)
      case otl_var_numeric_type_1:
        out_vars_arr_[i]=OTL_RCAST(unsigned char*,new OTL_NUMERIC_TYPE_1(0));
        break;
#endif
#if defined(OTL_NUMERIC_TYPE_2) && defined(OTL_STR_TO_NUMERIC_TYPE_2) && \
    defined(OTL_NUMERIC_TYPE_2_TO_STR) && defined(OTL_NUMERIC_TYPE_2_ID)
      case otl_var_numeric_type_2:
        out_vars_arr_[i]=OTL_RCAST(unsigned char*,new OTL_NUMERIC_TYPE_2(0));
        break;
#endif
#if defined(OTL_NUMERIC_TYPE_3) && defined(OTL_STR_TO_NUMERIC_TYPE_3) && \
    defined(OTL_NUMERIC_TYPE_3_TO_STR) && defined(OTL_NUMERIC_TYPE_3_ID)
      case otl_var_numeric_type_3:
        out_vars_arr_[i]=OTL_RCAST(unsigned char*,new OTL_NUMERIC_TYPE_3(0));
        break;
#endif
      }
#if defined(OTL_STL)
      var_name2pos_map_[curr_var.name]=i;
#endif
#if defined(OTL_ACE)
      var_name2pos_map_.bind(curr_var.name,i);
#endif
    }
    out_vars_constructed_=true;
  }
  
private:

#if defined(OTL_ANSI_CPP_11_DELETE_SPEC_SUPPORT)
public:

  otl_stream_read_iterator(const otl_stream_read_iterator&) = delete;
  otl_stream_read_iterator& operator=(const otl_stream_read_iterator&) = delete;
#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_stream_read_iterator(otl_stream_read_iterator&&) = delete;
  otl_stream_read_iterator& operator=(otl_stream_read_iterator&&) = delete;
#endif
private:
#else

  otl_stream_read_iterator(const otl_stream_read_iterator&):
    out_vars_(nullptr),
    out_vars_len_(0),
    str_(nullptr),
    out_vars_arr_(nullptr),
    out_vars_null_arr_(nullptr),
    out_vars_constructed_(nullptr),
    lob_stream_mode_flag_(false)
#if defined(OTL_STL)
    ,var_name2pos_map_()
#endif
#if defined(OTL_ACE)
    ,var_name2pos_map_()
#endif
  {
  }

  otl_stream_read_iterator& operator=(const otl_stream_read_iterator&)
  {
    return *this;
  }

#if defined OTL_ANSI_CPP_11_RVAL_REF_SUPPORT
  otl_stream_read_iterator(otl_stream_read_iterator&&):
    out_vars_(nullptr),
    out_vars_len_(0),
    str_(nullptr),
    out_vars_arr_(nullptr),
    out_vars_null_arr_(nullptr),
    out_vars_constructed_(nullptr),
    lob_stream_mode_flag_(false)
#if defined(OTL_STL)
    ,var_name2pos_map_()
#endif
#if defined(OTL_ACE)
    ,var_name2pos_map_()
#endif
  {
  }

  otl_stream_read_iterator& operator=(otl_stream_read_iterator&&)
  {
    return *this;
  }
#endif
#endif
 
};

#endif

#if defined(OTL_ORA_TEXT_ON)&&defined(text)
#undef text
#endif

#endif
