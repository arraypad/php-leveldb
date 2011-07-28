/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Arpad Ray <arpad@php.net>                                    |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_LEVELDB_H
#define PHP_LEVELDB_H

#define PHP_LEVELDB_EXTVER "0.0.1-dev"

#ifdef __cplusplus
extern "C" {
#endif

extern zend_module_entry leveldb_module_entry;
#define phpext_leveldb_ptr &leveldb_module_entry

#ifdef PHP_WIN32
#	define PHP_LEVELDB_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_LEVELDB_API __attribute__ ((visibility("default")))
#else
#	define PHP_LEVELDB_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_METHOD(LevelDb, __construct);
PHP_METHOD(LevelDb, set);
PHP_METHOD(LevelDb, get);
PHP_METHOD(LevelDb, delete);

PHP_METHOD(LevelDbOptions, __construct);

PHP_METHOD(LevelDbReadOptions, __construct);

PHP_METHOD(LevelDbWriteOptions, __construct);

PHP_MINIT_FUNCTION(leveldb);
PHP_MSHUTDOWN_FUNCTION(leveldb);
PHP_RINIT_FUNCTION(leveldb);
PHP_RSHUTDOWN_FUNCTION(leveldb);
PHP_MINFO_FUNCTION(leveldb);

#ifdef __cplusplus
}
#endif

#ifdef ZTS
#define LEVELDB_G(v) TSRMG(leveldb_globals_id, zend_leveldb_globals *, v)
#else
#define LEVELDB_G(v) (leveldb_globals.v)
#endif

#endif	/* PHP_LEVELDB_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
