--TEST--
leveldb - basic
--SKIPIF--
<?php if (!extension_loaded('leveldb')) { die('skip leveldb not loaded'); } ?>
--FILE--
<?php

$path = '/tmp/leveldb.test';
$db = new LevelDb($path);

echo "* setting (foo=bar): \n";
var_dump($db->set('foo', 'bar'));

echo "* getting (foo): \n";
var_dump($db->get('foo'));

echo "* delete (foo): \n";
var_dump($db->delete('foo'));

echo "* getting (foo): \n";
var_dump($db->get('foo'));

?>
--EXPECTF--
* setting (foo=bar): 
bool(true)
* getting (foo): 
string(3) "bar"
* delete (foo): 
bool(true)
* getting (foo): 
bool(false)
