/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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

var es6 = {
  'generator-star-spacing': [2, 'after'],
  'no-var': 2,
  'prefer-rest-params': 2,
  'prefer-spread': 2,
  'rest-spread-spacing': 2,
  'yield-star-spacing': [2, 'after'],
}

var eslintRecommended = {
  'no-console': 0,
  'no-empty': 0, // TODO: remove this feature
  'no-constant-condition': [2, { 'checkLoops': false }]
}

var style = {
  'no-multi-spaces': 2,
  'no-multi-str': 2,
  'array-bracket-spacing': [2, 'never'],
  'block-spacing': [2, 'never'],
  'brace-style': 2,
  'comma-dangle': [2, 'always-multiline'],
  'comma-spacing': 2,
  'comma-style': 2,
  'computed-property-spacing': 2,
  'eol-last': 2,
  'func-call-spacing': 2,
  'key-spacing': 2,
  'keyword-spacing': 2,
  'linebreak-style': 2,
  'no-multiple-empty-lines': [2, {max: 2}],
  'no-tabs': 2,
  'no-trailing-spaces': 2,
  'semi-spacing': 2,
  'space-before-blocks': 2,
  'space-before-function-paren': [2, {
    anonymous: 'never',
    named: 'never',
  }],
  'spaced-comment': [2, 'always'],
  'switch-colon-spacing': 2,
  'quotes': [2, 'single'],
}

var syntax = {
  'no-plusplus': 0,
  'guard-for-in': 2,
  'no-caller': 2,
  'no-extend-native': 2,
  'no-new-wrappers': 2,
  'new-cap': [2, { 'capIsNew': false, 'newIsCapExceptions': ['native'] }],
  'no-array-constructor': 2,
  'no-new-object': 2,
  'semi': 2,
}

module.exports = {
  'extends': 'eslint:recommended',
  'env': {
    'node': true,
    'es6': false,
  },
  'globals': {
    'native': true,
  },
  'rules': Object.assign(
    eslintRecommended,
    style,
    syntax,
    {
      // Optional rules
      'max-len': [2, {
        code: 80,
        tabWidth: 2,
        ignoreUrls: true,
        ignoreTemplateLiterals: true,
        ignoreRegExpLiterals: true
      }],
  }),
}
