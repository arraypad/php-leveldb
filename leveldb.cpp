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

extern "C" {
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "Zend/zend_exceptions.h"
#include "ext/spl/spl_exceptions.h"
}

#include "php_leveldb.h"

#include <leveldb/db.h>
#include <string>

using namespace leveldb;

/* {{{ PHP 5.2 compat */
#if ZEND_MODULE_API_NO < 20090626
# define array_init_size(arg, size) array_init(arg)
# define Z_ADDREF_P(arg) ZVAL_ADDREF(arg)
# define Z_ADDREF_PP(arg) ZVAL_ADDREF(*(arg))
# define Z_DELREF_P(arg) ZVAL_DELREF(arg)
# define Z_DELREF_PP(arg) ZVAL_DELREF(*(arg))
#endif
/* }}} */

/* {{{ leveldb_functions[]
 */
static zend_function_entry leveldb_functions[] = {
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ Class declarations */

/*	{{{ LevelDb */
zend_class_entry *php_leveldb_class_entry;
#define PHP_LEVELDB_CLASS_NAME "LevelDb"

/*		{{{ arg info */
ZEND_BEGIN_ARG_INFO_EX(arginfo_leveldb_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_leveldb_construct, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_leveldb_set, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, write_options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_leveldb_delete, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, write_options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_leveldb_write, 0, 0, 1)
	ZEND_ARG_INFO(0, batch)
	ZEND_ARG_INFO(0, write_options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_leveldb_get, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, read_options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_leveldb_get_iterator, 0, 0, 0)
	ZEND_ARG_INFO(0, read_options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_leveldb_release_snapshot, 0, 0, 1)
	ZEND_ARG_INFO(0, snapshot)
ZEND_END_ARG_INFO()
/*		}}} */

static zend_function_entry leveldb_class_functions[] = {
	PHP_ME(LevelDb, __construct, arginfo_leveldb_construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	/*
	PHP_ME(LevelDb, __destruct, arginfo_leveldb_void, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(LevelDb, set, arginfo_leveldb_put, ZEND_ACC_PUBLIC)
	PHP_ME(LevelDb, delete, arginfo_leveldb_delete, ZEND_ACC_PUBLIC)
	PHP_ME(LevelDb, write, arginfo_leveldb_write, ZEND_ACC_PUBLIC)
	PHP_ME(LevelDb, get, arginfo_leveldb_get, ZEND_ACC_PUBLIC)
	PHP_ME(LevelDb, getIterator, arginfo_leveldb_get_iterator, ZEND_ACC_PUBLIC)
	PHP_ME(LevelDb, getSnapshot, arginfo_leveldb_void, ZEND_ACC_PUBLIC)
	PHP_ME(LevelDb, releaseSnapshot, arginfo_leveldb_release_snapshot, ZEND_ACC_PUBLIC)
	*/
	{NULL, NULL, NULL}
};
/*	}}} */

/*	{{{ LevelDbIterator */
zend_class_entry *php_leveldb_iterator_class_entry;
#define PHP_LEVELDB_ITERATOR_CLASS_NAME "LevelDbIterator"

static zend_function_entry leveldb_iterator_class_functions[] = {
	{NULL, NULL, NULL}
};
/*	}}} */

/*	{{{ LevelDbSnapshot */
zend_class_entry *php_leveldb_snapshot_class_entry;
#define PHP_LEVELDB_SNAPSHOT_CLASS_NAME "LevelDbSnapshot"

static zend_function_entry leveldb_snapshot_class_functions[] = {
	{NULL, NULL, NULL}
};
/*	}}} */

/*	{{{ LevelDbWriteBatch */
zend_class_entry *php_leveldb_write_batch_class_entry;
#define PHP_LEVELDB_WRITE_BATCH_CLASS_NAME "LevelDbWriteBatch"

static zend_function_entry leveldb_write_batch_class_functions[] = {
	/*
	PHP_ME(LevelDbWriteBatch, __construct, arginfo_leveldb_void, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	*/
	{NULL, NULL, NULL}
};
/*	}}} */

/*	{{{ LevelDbOptions */
zend_class_entry *php_leveldb_options_class_entry;
#define PHP_LEVELDB_OPTIONS_CLASS_NAME "LevelDbOptions"

static zend_function_entry leveldb_options_class_functions[] = {
	/*
	PHP_ME(LevelDbOptions, __construct, arginfo_leveldb_void, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	*/
	{NULL, NULL, NULL}
};
/*	}}} */

/*	{{{ LevelDbReadOptions */
zend_class_entry *php_leveldb_read_options_class_entry;
#define PHP_LEVELDB_READ_OPTIONS_CLASS_NAME "LevelDbReadOptions"

static zend_function_entry leveldb_read_options_class_functions[] = {
	/*
	PHP_ME(LevelDbReadOptions, __construct, arginfo_leveldb_void, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	*/
	{NULL, NULL, NULL}
};
/*	}}} */

/*	{{{ LevelDbWriteOptions */
zend_class_entry *php_leveldb_write_options_class_entry;
#define PHP_LEVELDB_WRITE_OPTIONS_CLASS_NAME "LevelDbWriteOptions"

static zend_function_entry leveldb_write_options_class_functions[] = {
	/*
	PHP_ME(LevelDbWriteOptions, __construct, arginfo_leveldb_void, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	*/
	{NULL, NULL, NULL}
};
/*	}}} */

/* }}} */

/* {{{ Object handlers */

/*	{{{ LevelDb */
zend_object_handlers leveldb_object_handlers;

struct leveldb_object {
	zend_object std;
	DB *db;
};

void leveldb_free_storage(void *object TSRMLS_DC)
{
	leveldb_object *obj = (leveldb_object *)object;
	delete obj->db;

	zend_objects_free_object_storage((zend_object *)object TSRMLS_CC);
}

zend_object_value leveldb_object_new_ex(zend_class_entry *type, leveldb_object **ptr TSRMLS_DC)
{
	zval *tmp;
	zend_object_value retval;
	leveldb_object *obj;

	obj = (leveldb_object *)emalloc(sizeof(leveldb_object));
	memset(obj, 0, sizeof(leveldb_object));

	if (ptr) {
		*ptr = obj;
	}

	zend_object_std_init(&obj->std, type TSRMLS_CC);
#if ZEND_MODULE_API_NO >= 20100409
	object_properties_init(&obj->std, type);
#else
	zend_hash_copy(obj->std.properties, &type->default_properties, (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
#endif

	retval.handle = zend_objects_store_put(obj, NULL, leveldb_free_storage, NULL TSRMLS_CC);
	retval.handlers = &leveldb_object_handlers;
	return retval;
}

zend_object_value leveldb_object_new(zend_class_entry *type TSRMLS_DC)
{
	return leveldb_object_new_ex(type, NULL TSRMLS_CC);
}

/*	}}} */

/* }}} */

/*	{{{ proto LevelDb LevelDb::__construct(string $name [, LevelDbOptions $options [, LevelDbReadOptions $read_options [, LevelDbWriteOptions $write_options]]])
	Construct a new LevelDb object. */
PHP_METHOD(LevelDb, __construct)
{
	char *name;
	int name_len;
	std::string name_str;
	zval *options = NULL, *read_options = NULL, *write_options = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|OOO",
			&name, &name_len,
			&options, php_leveldb_options_class_entry,
			&read_options, php_leveldb_read_options_class_entry,
			&write_options, php_leveldb_write_options_class_entry) == FAILURE) {
		RETURN_NULL();
	}

	/* todo: handle options obj from php */

	/* todo: set default read and write options prop */

	/* create DB object */
	{
		DB *obj_db;
		Options options;
		Status status = DB::Open(options, std::string(name, name_len), &obj_db);

		if (status.ok()) {
			leveldb_object *obj = (leveldb_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
			obj->db = obj_db;
		} else {
			const char *error = status.ToString().c_str();
			zend_throw_exception(spl_ce_InvalidArgumentException, (char *)error, 0 TSRMLS_CC);
		}
	}
}
/*	}}} */

/* {{{ leveldb_module_entry
 */
zend_module_entry leveldb_module_entry = {
	STANDARD_MODULE_HEADER,
	"leveldb",
	leveldb_functions,
	PHP_MINIT(leveldb),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(leveldb),
	PHP_LEVELDB_EXTVER,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_LEVELDB
extern "C" {
ZEND_GET_MODULE(leveldb)
}
#endif

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(leveldb)
{
	/* register LevelDb class */
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, PHP_LEVELDB_CLASS_NAME, leveldb_class_functions);
	memcpy(&leveldb_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_leveldb_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
	php_leveldb_class_entry->create_object = leveldb_object_new;
	zend_declare_property_null(php_leveldb_class_entry, "options", strlen("options"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(php_leveldb_class_entry, "readoptions", strlen("readoptions"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(php_leveldb_class_entry, "writeoptions", strlen("writeoptions"), ZEND_ACC_PROTECTED TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(leveldb)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "leveldb support", "enabled");
	php_info_print_table_row(2, "leveldb version", PHP_LEVELDB_EXTVER);
	php_info_print_table_end();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
