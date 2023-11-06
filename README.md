# XIOS Mirroring and Experimentation

XIOS is an XML configured Input Output Server implementation for scientific computing file I/O interactions

IPSL develop and maintain the code base, which is open source and freely available. Code management is administered via https://forge.ipsl.jussieu.fr/ioserver

This repository aims to mirror XIOS version 3, including the trunk and release branches, whilst providing experimentation code management and published continuous integration testing.

## Repository management

The SubVersion repository at IPSL is the source of truth!

* No code shall be merged onto svn repository branches from within this repository
* branches, including `trunk` & `xios-3.0-beta` shall only be updated from the `SVN` source
* further branches, providing CI testing, notes, experiments etc. are permitted

Mirrors within github provide lightweight working copies of the main source tree, one step removed from the IPSL SubVersion

  ```mermaid
graph
    SubVersion -- sync --> Mirror
    Mirror -- sync ----> MyMirror
    MyMirror -- branch --> SubVersion
    MyMirror -- fetch --> Local
    Local -- push --> MyMirror
    Mirror -- fetch --> Local
```

### Local & MyMirror

To set up a Local, create your own Mirror within github (a github fork), then clone this to a Local git.

This local can be easily configured to communicate with the shared Mirror (an upstream).
The local can also be configured to read-only communicate with the IPSL subversion

A useful and common local work flow at a git terminal is to ensure Local is up to date from Upstream, then create a new branch to work on,
using `upstream/main` as the initialisation point.

```
git fetch upstream
git checkout -b {branchNameForWork} upstream/main
... doWork ...
git commit -am '{message}'
git push origin {branchNameForWork}
```

Pull Requests and Continuous Integration can be used within the Github environment, for communication and testing.
Branches within `Local` and `MyMirror` git repositories are easy to create and experiment with, only loosely coupled to the IPSL subversion.

But, care is needed, as the aim is to maintain synchronisation with the IPSL subversion, so no code should be merged onto `Mirror` branches.
Instead, changes identified, reviewed and tested should be propsed to IPSL via subversion and IPSL's development work flows.

The SVN synchronisation should then be used to bring those changes into Mirror and MyMirrors using the subversion change tracking as the source of truth.

### Local & MyMirror to manage SVN Synchronisation 

If you want to be involved in maintaining the subversion synchronisation, then your `Local` needs to be configured to use `git svn` and to communicate with the IPSL code management.

#### Initial Creation

When creating the `Local`, first create `MyMirror` from `Mirror` (this) on Github.
```
git clone {MyMirror}

cd {MyMirrorDir}

git remote add upstream {Mirror}

git svn init -s --prefix=ipsl/ https://forge.ipsl.jussieu.fr/ioserver/svn/XIOS3/

git svn fetch

```

This will configure `Local` to use the new MyMirror, treat Mirror as `upstream` and communicate (read-only) with the IPSL Subversion via `git svn`

(
Note: there are some history fixes implemented with respect to IPSL trunk.  Specifically, there are added then deleted large build artifacts in the svn history, which github objects to;
in preparing this repository, the BFG git reconfiguration tool was used to trim out artifacts larger than 80MB and the svn revision mapping rebuilt to match the updated commits

```
java -jar bfg.jar --strip-blobs-bigger-than 80M .git
find . -name '.rev_map*'
find . -name '.rev_map*' -exec rm {} \;
git svn fetch
```
this is noted for interest / full rebuild, but is not required for any normal interactions with these mirrors!
)

#### Working with Branches

Subversion synchronisation involves working in a `Local` git repository.

Updates from IPSL Subversion are obtained, then any `Mirror` branches that are being managed are rebased using the svn content.

```
git svn fetch
git checkout trunk
git rebase ipsl/trunk

git checkout build+TestXios-3.0-beta
git rebase ipsl/xios-3.0-beta
```

The rebase procedure inserts the commits from Subversion in a continuous stream, then applies any branch commits at the top (such as CI testing changes)

This ensures that the subversion change stream is maintain in sync, and any `Mirror` changes are only applied as addendums to the managed and synchronised code.

A bit of care is needed here, to ensure the 'source of truth' status of IPSL Subversion is maintained, and that any potential future changes being explored can be properly applied through SVN and re-synchronised.

