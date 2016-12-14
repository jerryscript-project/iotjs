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
var assert = require('assert');

var filePath = "../resources/tobeornottobe.txt";

var data = fs.readFileSync(filePath);

var result =
  "To be, or not to be, that is the Question:\n" +
  "Whether ’tis Nobler in the mind to ſuffer\n" +
  "The Slings and Arrows of outragious Fortune,\n" +
  "Or to take Armes against a Sea of troubles,\n" +
  "And by opposing end them: to dye, to ſleepe\n" +
  "No more; and by a sleep, to say we end\n" +
  "The Heart-ake, and the thouſand Naturall ſhockes\n" +
  'That Flesh is there too? "Tis a consummation\n' +
  "Deuoutly to be wiſh'd. To dye to sleepe,\n" +
  "To sleep, perchance to Dream; I, there's the rub,\n" +
  "For in that sleep of death, what dreams may come,\n" +
  "When we haue ſhufflel’d off this mortall coile,\n" +
  "Muſt giue us pause. There's the respect\n" +
  "That makes Calamity of long life:\n" +
  "For who would beare the Whips and Scornes of time,\n" +
  "The Oppreſſors wrong, the poore mans Contumely,\n" +
  "The pangs of diſpriz’d Loue, the Lawes delay,\n" +
  "The inſolence of Office, and the Spurnes\n" +
  "That patient merit of the vnworthy takes,\n" +
  "When he himſelfe might his Quietus make\n" +
  "With a bare Bodkin? Who would theſe Fardles beare\n" +
  "To grunt and ſweat vnder a weary life,\n" +
  "But that the dread of ſomething after death,\n" +
  "The vndiſcouered Countrey, from whoſe Borne\n" +
  "No Traueller returnes, Puzels the will,\n" +
  "And makes vs rather beare those illes we haue,\n" +
  "Then flye to others that we know not of.\n" +
  "Thus Conſcience does make Cowards of vs all,\n" +
  "And thus the Natiue hew of Resolution\n" +
  "Is ſicklied o’re, with the pale caſt of Thought,\n" +
  "And enterprizes of great pith and moment,\n" +
  "With this regard their Currants turne away,\n" +
  "And looſe the name of Action. Soft you now,\n" +
  "The faire Ophelia? Nimph, in thy Orizons\n" +
  "Be all my ſinnes remembred."
assert.equal(data, result);
