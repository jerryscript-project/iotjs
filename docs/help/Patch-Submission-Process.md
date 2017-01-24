The following guidelines on the submission process are provided to help you be more effective when submitting code to the IoT.js project.

When development is complete, a patch set should be submitted via Github pull requests. A review of the patch set will take place. When accepted, the patch set will be integrated into the master branch, verified, and tested. It is then the responsibility of the authoring developer to maintain the code throughout its lifecycle.

Please submit all patches in public by opening a pull request. Patches sent privately to Maintainers and Committers will not be considered. Because the IoT.js Project is an Open Source project, be prepared for feedback and criticism-it happens to everyone-. If asked to rework your code, be persistent and resubmit after making changes.

#### 1. Scope the patch

Smaller patches are generally easier to understand and test, so please submit changes in the smallest increments possible, within reason. Smaller patches are less likely to have unintended consequences, and if they do, getting to root cause is much easier for you and the Maintainers and Committers. Additionally, smaller patches are much more likely to be accepted.

#### 2. Sign your work with the [IoT.js DCO](IoT.js-Developer's-Certificate-of-Origin-1.0.md)

The sign-off is a simple line at the end of the explanation for the patch, which certifies that you wrote it or otherwise have the right to pass it on as an Open Source patch. The  sign-off is required for a patch to be accepted.

#### 3. Open [a Github pull request](https://github.com/Samsung/iotjs/pulls)

#### 4. What if my patch is rejected?

It happens all the time, for many reasons, and not necessarily because the code is bad. Take the feedback, adapt your code, and try again. Remember, the ultimate goal is to preserve the quality of the code and maintain the focus of the Project through intensive review.

Maintainers and Committers typically have to process a lot of submissions, and the time for any individual response is generally limited. If the reason for rejection is unclear, please ask for more information to the Maintainers and Committers.
If you have a solid technical reason to disagree with feedback and you feel that reason has been overlooked, take the time to thoroughly explain it in your response.

#### 5. Code review

Code review can be performed by all the members of the Project (not just Maintainers and Committers). Members can review code changes and share their opinion by comments with the following principles:
* Discuss code; never discuss the code's author.
* Respect and acknowledge contributions, suggestions, and comments.
* Listen and be open to all different opinions.
* Help each other.

Changes are submitted via pull requests and only the Maintainers and Committers should approve or reject the pull request.
Changes should be reviewed in reasonable amount of time. Maintainers and Committers should leave changes open for some time (at least 1 full business day) so others can offer feedback. Review times increase with the complexity of the review.

### Tips on GitHub Pull Requests
* Fork the GitHub repository(https://guides.github.com/activities/forking/) and clone it locally.
Connect your local repository to the original upstream repository by adding it as a remote.
Pull in upstream changes often to stay up-to-date so that when you submit your pull request, merge conflicts will be less likely.
* For more details, see [GitHub fork synching guidelines](https://help.github.com/articles/syncing-a-fork/).
[Create a branch](https://guides.github.com/introduction/flow/) for your edits.