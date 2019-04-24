'use strict';
var assert = require('assert');
var common = require('common.js');
var test = require('./build/' + common.buildTypePath +
                   '/test_napi_string.node');

var empty = '';
assert.strictEqual(test.TestUtf8(empty), empty);
assert.strictEqual(test.Utf8Length(empty), 0);

var str1 = 'hello world';
assert.strictEqual(test.TestUtf8(str1), str1);
assert.strictEqual(test.Utf8Length(str1), 11);

var str2 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
assert.strictEqual(test.TestUtf8(str2), str2);
assert.strictEqual(test.Utf8Length(str2), 62);

var str3 = '?!@#$%^&*()_+-=[]{}/.,<>\'"\\';
assert.strictEqual(test.TestUtf8(str3), str3);
assert.strictEqual(test.Utf8Length(str3), 27);

var str4 = '¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿';
assert.strictEqual(test.TestUtf8(str4), str4);
assert.strictEqual(test.Utf8Length(str4), 62);

var str5 ='ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞ'
  + 'ßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþ';
assert.strictEqual(test.TestUtf8(str5), str5);
assert.strictEqual(test.Utf8Length(str5), 126);

// TODO: jerry-script doesn't support copy string value to insufficient buf
