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
	PHP_ME(LevelDb, set, arginfo_leveldb_set, ZEND_ACC_PUBLIC)
	PHP_ME(LevelDb, get, arginfo_leveldb_get, ZEND_ACC_PUBLIC)
	PHP_ME(LevelDb, delete, arginfo_leveldb_delete, ZEND_ACC_PUBLIC)
	/*
	PHP_ME(LevelDb, __destruct, arginfo_leveldb_void, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(LevelDb, write, arginfo_leveldb_write, ZEND_ACC_PUBLIC)
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
	PHP_ME(LevelDbOptions, __construct, arginfo_leveldb_void, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	{NULL, NULL, NULL}
};
/*	}}} */

/*	{{{ LevelDbReadOptions */
zend_class_entry *php_leveldb_read_options_class_entry;
#define PHP_LEVELDB_READ_OPTIONS_CLASS_NAME "LevelDbReadOptions"

static zend_function_entry leveldb_read_options_class_functions[] = {
	PHP_ME(LevelDbReadOptions, __construct, arginfo_leveldb_void, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	{NULL, NULL, NULL}
};
/*	}}} */

/*	{{{ LevelDbWriteOptions */
zend_class_entry *php_leveldb_write_options_class_entry;
#define PHP_LEVELDB_WRITE_OPTIONS_CLASS_NAME "LevelDbWriteOptions"

static zend_function_entry leveldb_write_options_class_functions[] = {
	PHP_ME(LevelDbWriteOptions, __construct, arginfo_leveldb_void, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
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

/*	{{{ LevelDbOptions */
zend_object_handlers leveldb_options_object_handlers;

struct leveldb_options_object {
	zend_object std;
	Options options;
};

void leveldb_options_free_storage(void *object TSRMLS_DC)
{
	leveldb_options_object *obj = (leveldb_options_object *)object;
	zend_objects_free_object_storage((zend_object *)object TSRMLS_CC);
}

zend_object_value leveldb_options_object_new_ex(zend_class_entry *type, leveldb_options_object **ptr TSRMLS_DC)
{
	zval *tmp;
	zend_object_value retval;
	leveldb_options_object *obj;

	obj = (leveldb_options_object *)emalloc(sizeof(leveldb_options_object));
	memset(obj, 0, sizeof(leveldb_options_object));

	if (ptr) {
		*ptr = obj;
	}

	zend_object_std_init(&obj->std, type TSRMLS_CC);
#if ZEND_MODULE_API_NO >= 20100409
	object_properties_init(&obj->std, type);
#else
	zend_hash_copy(obj->std.properties, &type->default_properties, (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
#endif

	retval.handle = zend_objects_store_put(obj, NULL, leveldb_options_free_storage, NULL TSRMLS_CC);
	retval.handlers = &leveldb_options_object_handlers;
	return retval;
}

zend_object_value leveldb_options_object_new(zend_class_entry *type TSRMLS_DC)
{
	return leveldb_options_object_new_ex(type, NULL TSRMLS_CC);
}
/*	}}} */

/*	{{{ LevelDbReadOptions */
zend_object_handlers leveldb_read_options_object_handlers;

struct leveldb_read_options_object {
	zend_object std;
	ReadOptions options;
};

void leveldb_read_options_free_storage(void *object TSRMLS_DC)
{
	leveldb_read_options_object *obj = (leveldb_read_options_object *)object;
	zend_objects_free_object_storage((zend_object *)object TSRMLS_CC);
}

zend_object_value leveldb_read_options_object_new_ex(zend_class_entry *type, leveldb_read_options_object **ptr TSRMLS_DC)
{
	zval *tmp;
	zend_object_value retval;
	leveldb_read_options_object *obj;

	obj = (leveldb_read_options_object *)emalloc(sizeof(leveldb_read_options_object));
	memset(obj, 0, sizeof(leveldb_read_options_object));

	if (ptr) {
		*ptr = obj;
	}

	zend_object_std_init(&obj->std, type TSRMLS_CC);
#if ZEND_MODULE_API_NO >= 20100409
	object_properties_init(&obj->std, type);
#else
	zend_hash_copy(obj->std.properties, &type->default_properties, (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
#endif

	retval.handle = zend_objects_store_put(obj, NULL, leveldb_read_options_free_storage, NULL TSRMLS_CC);
	retval.handlers = &leveldb_read_options_object_handlers;
	return retval;
}

zend_object_value leveldb_read_options_object_new(zend_class_entry *type TSRMLS_DC)
{
	return leveldb_read_options_object_new_ex(type, NULL TSRMLS_CC);
}
/*	}}} */

/*	{{{ LevelDbWriteOptions */
zend_object_handlers leveldb_write_options_object_handlers;

struct leveldb_write_options_object {
	zend_object std;
	WriteOptions options;
};

void leveldb_write_options_free_storage(void *object TSRMLS_DC)
{
	leveldb_write_options_object *obj = (leveldb_write_options_object *)object;
	zend_objects_free_object_storage((zend_object *)object TSRMLS_CC);
}

zend_object_value leveldb_write_options_object_new_ex(zend_class_entry *type, leveldb_write_options_object **ptr TSRMLS_DC)
{
	zval *tmp;
	zend_object_value retval;
	leveldb_write_options_object *obj;

	obj = (leveldb_write_options_object *)emalloc(sizeof(leveldb_write_options_object));
	memset(obj, 0, sizeof(leveldb_write_options_object));

	if (ptr) {
		*ptr = obj;
	}

	zend_object_std_init(&obj->std, type TSRMLS_CC);
#if ZEND_MODULE_API_NO >= 20100409
	object_properties_init(&obj->std, type);
#else
	zend_hash_copy(obj->std.properties, &type->default_properties, (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
#endif

	retval.handle = zend_objects_store_put(obj, NULL, leveldb_write_options_free_storage, NULL TSRMLS_CC);
	retval.handlers = &leveldb_write_options_object_handlers;
	return retval;
}

zend_object_value leveldb_write_options_object_new(zend_class_entry *type TSRMLS_DC)
{
	return leveldb_write_options_object_new_ex(type, NULL TSRMLS_CC);
}
/*	}}} */

/* }}} */

/* {{{ Methods */

/*	{{{ static inline bool _create_object(zval *obj, zend_class_entry *ce) */
static inline bool _create_object(zval **obj, zend_class_entry *ce)
{
	zval *ctor, unused;

	MAKE_STD_ZVAL(*obj);
	Z_TYPE_PP(obj) = IS_OBJECT;
	object_init_ex(*obj, ce);

	MAKE_STD_ZVAL(ctor);
	array_init_size(ctor, 2);
	Z_ADDREF_PP(obj);
	add_next_index_zval(ctor, *obj);
	add_next_index_string(ctor, "__construct", 1);
	if (call_user_function(&ce->function_table, obj, ctor, &unused, 0, NULL TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Unable to construct %s", ce->name);
		zval_add_ref(obj);
		zval_ptr_dtor(obj);
		return false;
	}

	zval_ptr_dtor(&ctor);
	Z_DELREF_PP(obj);
	return true;
}
/*	}}} */

/*	{{{ proto LevelDb LevelDb::__construct(string $name [, LevelDbOptions $options [, LevelDbReadOptions $read_options [, LevelDbWriteOptions $write_options]]])
	Construct a new LevelDb object. */
PHP_METHOD(LevelDb, __construct)
{
	char *name;
	int name_len;
	std::string name_str;
	zval *options_zv = NULL, *read_options_zv = NULL, *write_options_zv = NULL, *this_zv = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|OOO",
			&name, &name_len,
			&options_zv, php_leveldb_options_class_entry,
			&read_options_zv, php_leveldb_read_options_class_entry,
			&write_options_zv, php_leveldb_write_options_class_entry) == FAILURE) {
		RETURN_NULL();
	}

	/* db options */
	if (!options_zv && !_create_object(&options_zv, php_leveldb_options_class_entry)) {
		RETURN_NULL();
	}
	zend_update_property(php_leveldb_class_entry, this_zv, "options", strlen("options"), options_zv TSRMLS_CC);

	/* open db */
	DB *obj_db;
	Status status;
	leveldb_options_object *options_obj = (leveldb_options_object *)zend_object_store_get_object(options_zv TSRMLS_CC);
	status = DB::Open(options_obj->options, std::string(name, name_len), &obj_db);

	if (status.ok()) {
		leveldb_object *obj = (leveldb_object *)zend_object_store_get_object(this_zv TSRMLS_CC);
		obj->db = obj_db;
	} else {
		const char *error = status.ToString().c_str();
		zend_throw_exception(spl_ce_InvalidArgumentException, (char *)error, 0 TSRMLS_CC);
	}

	/* read options */
	if (!read_options_zv && !_create_object(&read_options_zv, php_leveldb_read_options_class_entry)) {
		RETURN_NULL();
	}
	zend_update_property(php_leveldb_class_entry, this_zv, "readoptions", strlen("readoptions"), read_options_zv TSRMLS_CC);
	
	/* write options */
	if (!write_options_zv && !_create_object(&write_options_zv, php_leveldb_write_options_class_entry)) {
		RETURN_NULL();
	}
	zend_update_property(php_leveldb_class_entry, this_zv, "writeoptions", strlen("writeoptions"), write_options_zv TSRMLS_CC);
}
/*	}}} */

/*	{{{ proto bool LevelDb::set(string $key, string $value [, LevelDbWriteOptions $write_options])
	Sets the value for the given key. */
PHP_METHOD(LevelDb, set)
{
	char *key, *value;
	int key_len, value_len;
	zval *write_options = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|O",
			&key, &key_len,
			&value, &value_len,
			&write_options, php_leveldb_write_options_class_entry) == FAILURE) {
		RETURN_FALSE;
	}

	{
		WriteOptions options;
		leveldb_object *obj = (leveldb_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
		Status status = obj->db->Put(options, key, value);
		RETVAL_BOOL(status.ok());
	}
}
/*	}}} */

/*	{{{ proto bool LevelDb::get(string $key, [, LevelDbReadOptions $read_options])
	Returns the value for a given key. */
PHP_METHOD(LevelDb, get)
{
	char *key;
	int key_len;
	zval *read_options = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|O",
			&key, &key_len,
			&read_options, php_leveldb_read_options_class_entry) == FAILURE) {
		RETURN_FALSE;
	}

	{
		std::string value;
		ReadOptions options;
		leveldb_object *obj = (leveldb_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
		Status status = obj->db->Get(options, key, &value);
		
		if (status.ok()) {
			RETURN_STRINGL(value.c_str(), value.length(), 1);
		}

		RETURN_FALSE;
	}
}
/*	}}} */

/*	{{{ proto bool LevelDb::delete(string $key, [, LevelDbWriteOptions $write_options])
	Deletes given key. */
PHP_METHOD(LevelDb, delete)
{
	char *key;
	int key_len;
	zval *write_options = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|O",
			&key, &key_len,
			&write_options, php_leveldb_write_options_class_entry) == FAILURE) {
		RETURN_FALSE;
	}

	{
		WriteOptions options;
		leveldb_object *obj = (leveldb_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
		Status status = obj->db->Delete(options, key);
		RETURN_BOOL(status.ok());
	}
}
/*	}}} */

/*	{{{ proto LevelDbOptions LevelDbOptions::__construct()
	Construct a new LevelDbOptions object. */
PHP_METHOD(LevelDbOptions, __construct)
{
	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_NULL();
	}

	leveldb_options_object *options_obj = (leveldb_options_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	options_obj->options = Options();
	options_obj->options.create_if_missing = true;
	options_obj->options.error_if_exists = false;
}
/*	}}} */

/*	{{{ proto LevelDbReadOptions LevelDbReadOptions::__construct()
	Construct a new LevelDbReadOptions object. */
PHP_METHOD(LevelDbReadOptions, __construct)
{
	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_NULL();
	}

	leveldb_read_options_object *options_obj = (leveldb_read_options_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	options_obj->options = ReadOptions();
}
/*	}}} */

/*	{{{ proto LevelDbWriteOptions LevelDbWriteOptions::__construct()
	Construct a new LevelDbWriteOptions object. */
PHP_METHOD(LevelDbWriteOptions, __construct)
{
	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_NULL();
	}

	leveldb_write_options_object *options_obj = (leveldb_write_options_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	options_obj->options = WriteOptions();
}
/*	}}} */

/* }}} */

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
	zend_class_entry ce;

	/* register LevelDb class */
	INIT_CLASS_ENTRY(ce, PHP_LEVELDB_CLASS_NAME, leveldb_class_functions);
	memcpy(&leveldb_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_leveldb_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
	php_leveldb_class_entry->create_object = leveldb_object_new;
	zend_declare_property_null(php_leveldb_class_entry, "options", strlen("options"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(php_leveldb_class_entry, "readoptions", strlen("readoptions"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(php_leveldb_class_entry, "writeoptions", strlen("writeoptions"), ZEND_ACC_PROTECTED TSRMLS_CC);

	/* register LevelDbOptions class */
	INIT_CLASS_ENTRY(ce, PHP_LEVELDB_OPTIONS_CLASS_NAME, leveldb_options_class_functions);
	memcpy(&leveldb_options_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_leveldb_options_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
	php_leveldb_options_class_entry->create_object = leveldb_options_object_new;

	/* register LevelDbReadOptions class */
	INIT_CLASS_ENTRY(ce, PHP_LEVELDB_READ_OPTIONS_CLASS_NAME, leveldb_read_options_class_functions);
	memcpy(&leveldb_read_options_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_leveldb_read_options_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
	php_leveldb_read_options_class_entry->create_object = leveldb_read_options_object_new;

	/* register LevelDbWriteOptions class */
	INIT_CLASS_ENTRY(ce, PHP_LEVELDB_WRITE_OPTIONS_CLASS_NAME, leveldb_write_options_class_functions);
	memcpy(&leveldb_write_options_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_leveldb_write_options_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
	php_leveldb_write_options_class_entry->create_object = leveldb_write_options_object_new;

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
