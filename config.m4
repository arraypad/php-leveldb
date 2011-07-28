dnl config.m4 for extension leveldb

PHP_ARG_WITH(leveldb, for leveldb support,
[  --with-leveldb             Include leveldb support])

if test "$PHP_LEVELDB" != "no"; then
  SEARCH_PATH="$PHP_LEVELDB /usr/local /usr"
  SEARCH_INC="include/leveldb/db.h"
  SEARCH_LIB="libleveldb.a"
  AC_MSG_CHECKING([for leveldb files])
  for i in $SEARCH_PATH ; do
    if test -r $i/$SEARCH_INC && test -r $i/$SEARCH_LIB; then
      LEVELDB_DIR=$i
      AC_MSG_RESULT(found in $i)
    fi
  done
  if test -z "$LEVELDB_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please ensure the leveldb headers and static library are installed])
  fi

  PHP_ADD_INCLUDE($LEVELDB_DIR/include)

  PHP_REQUIRE_CXX()

  PHP_ADD_LIBRARY(stdc++, 1, LEVELDB_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(leveldb, $LEVELDB_DIR, LEVELDB_SHARED_LIBADD)
  PHP_SUBST(LEVELDB_SHARED_LIBADD)

  PHP_NEW_EXTENSION(leveldb, leveldb.cpp, $ext_shared,,,1)
  PHP_ADD_EXTENSION_DEP(leveldb, spl)
fi
