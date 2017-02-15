/* global module */

module.exports = {
  'rules': {
    'indent': [
      2,
      2,
      { 'VariableDeclarator': 2 }
    ],
    'quotes': [
      2,
      'single'
    ],
    'linebreak-style': [
      2,
      'unix'
    ],
    'semi': [
      2,
      'always'
    ],
    'no-trailing-spaces': [
      2
    ],
    'camelcase': [
      2,
      { 'properties': 'always' }
    ],
    'no-with': 'error',
    'brace-style': [
      'error',
      '1tbs',
      { 'allowSingleLine': true }
    ],
    'curly': 'error',
    'keyword-spacing': 'error',
    'newline-before-return': 'error',
    'no-multiple-empty-lines': 'error'
  },
  'env': {
    'node': true,
    'browser': true
  },
  'extends': 'eslint:recommended'
};
