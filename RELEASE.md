Release Documentation
=====================

The following steps document the process for producing a new PropWare release:

1. Using `git flow`, create a new release branch. If the version to be released is 1.2.3, the command would be
   `git flow release start 1.2.3`
2. Update `<root>/version.txt` with the new version number.
3. Commit changes.
4. Using `git flow`, merge the release back into master: `git flow release finish 1.2.3`
