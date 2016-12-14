/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


var fs = require('fs');

try{ // fs.statSync throw ex for non-existing file
  var statbuf1 = fs.statSync('../test');
  console.log("../test dev : " + statbuf1.dev);
  console.log("../test mode : " + statbuf1.mode);
  console.log("../test size : " + statbuf1.size);

  if(statbuf1.isDirectory()){
    console.log("../test is a directory");
  }
  else {
    console.log("../test is not a directory");
  }
}
catch(ex){}


try{ // fs.statSync throw ex for non-existing file
  var statbuf2 = fs.statSync('test_stat.js');

  console.log("test_stat.js dev : " + statbuf2.dev);
  console.log("test_stat.js mode : " + statbuf2.mode);
  console.log("test_stat.js size : " + statbuf2.size);

  if(statbuf2.isDirectory()){
    console.log("test_stat.js is a directory");
  }
  else {
    console.log("test_stat.js is not a directory");
  }
}
catch(ex){}

try{ // fs.statSync throw ex for non-existing file
  console.log("non-existing file(non_existing.js) statSync tried.");
  var statbuf3 = fs.statSync('non_existing.js');
}
catch(ex){
  console.log("but, failed.");
}

function statcallback(err, statbuf) {
  console.log("statcallback called");
  if(err){
    console.log("stat async call error");
    return;
  }

  if(statbuf.isDirectory()){
    console.log("async stat for test_stat.js is a directory");
  }
  else {
    console.log("async stat for test_stat.js is not a directory");
  }

}

try {
  fs.stat('test_stat.js', statcallback);
}
catch(ex) {
  console.log("stat async call error");
}
