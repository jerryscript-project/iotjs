## Module
``` module ``` implements ``` require ``` function.
Since ``` require ``` function is implicitly passed to user code, user do not need to import it explicitly.

### Methods

#### require(id)
* ``` id : String ``` - module name to be loaded

Loads the module named 'id'


### Module Loading by ``` require ``` function

#### ``` require ``` search paths
``` require ``` function finds modules in the order of

1. current working directory
2. node_modules directory under current working directory
3. $HOME/node_modules
4. $NODE_PATH/node_modules


#### ``` require('id') ``` works as follows:
For each directory in search paths above,

0. If native module named 'id' exists, load it and return.
1. If id.js file exists, load it and return.
2. If a directory id exists, load it as an package and return.

#### ``` require('id') ``` as a package
For a given id, if a directory id exists, module system consider the directory as a package.

0. if 'id/node_modules/package.json' contains 'main' property, load the file named main property.
1. if either id/node_modules/package.json or main property exists, load index.js
