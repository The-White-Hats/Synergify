# Conventional Commit Messages

See how a [minor change](#examples) to your commit message style can make a difference. 

> [!TIP]
> Have a look at **[ conventionalcommits.org](https://www.conventionalcommits.org/)** for more documentations

## Commit message convention

```
<type>(optional scope): <description>
<BLANK LINE>
[optional body]
<BLANK LINE>
[optional footer(s)]
```

### Types
* `feat` Commits, that adds or remove a new feature
* `fix` Commits, that fixes a bug  
* `refactor` Commits, that rewrite/restructure your code, however **does not change** any API behaviour
* `perf` Commits are special `refactor` commits, that improve performance
* `style` Commits, that do not affect the meaning (white-space, formatting, missing semi-colons, etc)
* `build` Commits, that affect build components like build tool, ci pipeline, dependencies, project version, ...
* `docs` Commits, that affect documentation only
* `test` Commits, that add missing tests or correcting existing tests
* `ops` Commits, that affect operational components like infrastructure, deployment, backup, recovery, ...
* `chore` Miscellaneous commits e.g. modifying `.gitignore`

### Scopes
The `scope` provides additional contextual information.
* Is an **optional** part of the format
* Allowed Scopes depends on the specific project
* Don't use issue identifiers as scopes

### Breaking Changes Indicator
Breaking changes should be indicated by an `!` before the `:` in the subject line e.g. `feat(api)!: remove status endpoint`
* Is an **optional** part of the format

### Description
The `description` contains a concise description of the change.
* Is a **mandatory** part of the format
* Use the imperative, present tense: "change" not "changed" nor "changes"
  * Think of `This commit will...` or `This commit should...`
* Don't capitalize the first letter
* No period (`.`) at the end

### Body
The `body` should include the motivation for the change and contrast this with previous behavior.
* Is an **optional** part of the format
* Use the imperative, present tense: "change" not "changed" nor "changes"
* This is the place to mention issue identifiers and their relations

### Footer
The `footer` should contain any information about **Breaking Changes** and is also the place to **reference Issues** that this commit refers to.
* Is an **optional** part of the format
* **optionally** reference an issue by its id.
* **Breaking Changes** should start with the word `BREAKING CHANGES:` followed by space or two newlines. The rest of the commit message is then used for this.


### Examples
* ```
  feat: add email notifications on new direct messages
  ```
* ```
  feat(shopping cart): add the amazing button
  ```
* ```
  feat: New comment faces

  Show recent commenters in the postcard in the feed. 

  Fixes issue #223
  ```
* ```
  feat!: remove ticket list endpoint

  refers to JIRA-1337

  BREAKING CHANGES: ticket enpoints no longer supports list all entites.
  ```
* ```
  fix(api): handle empty message in request body
  ```
* ```
  fix(api): fix wrong calculation of request body checksum
  ```
* ```
  fix: add missing parameter to service call

  The error occurred because of <reasons>.
  ```
* ```
  perf: decrease memory footprint for determine uniqe visitors by using HyperLogLog
  ```
* ```
  build: update dependencies
  ```
* ```
  build(release): `bump version to 1.0.0
  ```
* ```
  refactor: implement fibonacci number calculation as recursion
  ```
* ```
  style: remove empty line
  ```  


### Structure
A full long commit message will look like this:

```console
fix(middleware): ensure Range headers adhere more closely to RFC 2616

Add one new dependency, use `range-parser` (Express dependency) to compute
range. It is more well-tested in the wild.

Fixes #2310
```
Most tools only show the first line of the commit message, which is usually enough to know what it is about.
```console
* 8decde6 - (1 months ago) docs(server): update file uploading api
* bee23f9 - (2 months ago) feat(client): add Apple login button
* a8ecd9d - (3 months ago) fix(client): broken header in safari
* 7a0f17e - (4 months ago) feat(server): add crud apis
* af61aec - (5 months ago) docs(changelog): update changelog
```