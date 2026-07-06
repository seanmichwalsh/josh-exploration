# Josh Exploration

## Purpose
Monorepos are an ideal way of managing projects that have highly coupled interfaces, allowing developers to make breaking changes across projects in an atomic manner. The alternative to this having a series of polyrepos that ultimately feed into a downstream integration repo, whose job is to validate and provide a stable release for the coupled upstream projects (likely using pinning). This flow is much more painful for integration: breaking changes must be coordinated across multiple discrete repos, and the "release" versions of these projects provided by the integration repo will inevitably lag behind their respective top-of-tree sources.

However, monorepos have their own pain points:
- **Third party projects:** A monorepo's monolithic history will conflict with the history of third-party projects (e.g. LLVM) when it comes time to interact with upstream. The project either needs to be vendored within the monorepo and special tooling used to handle integration of the divergent histories, or the project must be kept external from the monorepo, eliminating the benefits it would otherwise provide.
- **Performance bottlenecks:** Developers likely only want to interface locally with a subset of the monorepo. Once it grows to a certain size it can become slow to interact with it via tools like Git. Most monorepos handle this by using version control tools that allow for partial clones, like Perforce.
- **Dependency over-coupling:** When all company code exists in the same repo, it becomes trivial to create dependencies between any two targets (e.g. copy / pasting a build script). This can quickly lead to a spaghetti dependency graph of unnecessary dependencies and overly slow builds. This can be combatted with better development practices, but some level of over-coupling is inevitable.
- **Tooling complexity:** A given team's meta tools like CI/CD benefit from having all target code in one place, but also incur a complexity penalty from having to coexist with other teams' tooling that cares only about a different subset of the repo. Extra care must be taken into ensuring that teams don't conflict on tooling flows.
- **Permission granularity:** If the organization's method for specifying developer permissions operates at repo-level granularity, a monorepo makes it difficult to limit access control at the project level. Special tooling would need to be introduced to accomplish this.

[Josh](https://josh-project.dev/docs/intro.html) is a Git-compatible version control tool that aims to address some of the above issues with monorepos. It implements a method of Git history filtering that claims to be much faster than existing tools like `git filter-repo`, etc. due to clever use of caching. Faster history filtering allows it to take a monorepo and rewrite its history on the fly in order do things like importing / exporting histories for specific sub-projects, performing partial clones of sub-projects with history local only to said sub-project, etc. This repository serves as an exploration into whether Josh is an appropriate tool to use in a real world production scenario.

## Josh Workflow Walkthrough

### Structuring the Monorepo
First, let's define the layout of a toy monorepo to be managed using Josh. Let's say this repo will be shared between multiple hardware and software teams: hardware engineers will implement and validate RTL, while software engineers will reflect these designs in simulation and write libraries to be run on said simulators.

Let's divide the monorepo into the below hierarchy of subprojects based on project classification:
```
.
├── hardware
│   ├── csrs
│   └── rtl
└── software
    ├── external
    └── internal
```

We'll create a top-level split between hardware vs software projects. The `hardware` directory will be further subdivided into both an `rtl` subdirectory (containing subprojects that implement HW designs via System Verilog) and a `csr` subdirectory (containing subprojects that package SW-consumable definitions of CSR layouts derived from System Verilog files in the `rtl` directory).

The `software` directory will be subdivded into `internal` and `external` subdirectories, containing projects that are either original to the organization or vendored from an upstream source, respectively.

Note that Josh is not involved in this setup process: we create and upload the initial setup commits for the monorepo entirely with Git.

### Repo vs Project Level Development
Let's look at how Josh enables developers to perform partial clones on subprojects within the repo.

Imagine we have two developers: Patrick and Adam. Adam wants to work with the full monorepo, while Patrick only cares about developing within the `//hardware` family of subprojects. Josh should allow both developers to perform checkouts scoped only to the project granularity that matters to them.

Adam can clone the full repo:
```
josh clone git@github.com:seanmichwalsh/josh-exploration.git :/ ./josh-mono
```

While Patrick can clone only the `//hardware` subdirectory:
```
josh clone git@github.com:seanmichwalsh/josh-exploration.git :/hardware ./josh-hw
```

In both cases Josh requires the caller to specify the desired directories to clone and the name of the resulting clone as parameters, there are no defaults.

---
**Pain Point: Cloning with SSH**
Attempting to clone the repo via Josh using SSH initially fails. The issue is that Josh cannot parse the shortened SSH URI provided by Github, it needs the properly formatted URI:
- `git@github.com:seanmichwalsh/josh-exploration.git` will fail, instead use `ssh://git@github.com/seanmichwalsh/josh-exploration`.
- Note the use of the `ssh://` URI prefix, and the change of the `:` separating the forge URL and project slug to a `/`.
- If you attempt a clone and fail, a partial output directory will still be created. You MUST delete this directory before attempting another clone, as otherwise it will conflict and fail the subsequent clone with a cryptic error.
- See discussion in issue [#1983](https://github.com/josh-project/josh/issues/1983).
---

Let's say Patrick wants to define a new RTL unit `toy-core`, and generate the associate CSR header for it. His checkout contains only the `//hardware` directory, so he can do so without concerning himself with the history / contents of the rest of the monorepo. When he's done, he pushes his work with Josh as shown below:
```
josh push
```

Once he's done, Adam can pull Patrick's work with Josh. For every filter used to define a partial clone, there is a reverse filter that can be used to map said partial clone back to its place within the larger repo. So when Adam pulls with Josh, the resulting changes are placed within the proper `//hardware` directory:
```
josh fetch
josh pull
```

---
**Pain Point: Slow Pushes / Pulls**
Despite advertising a faster Git filtering implementation than competing tools, cloning, pushing, and pulling work with Josh is still significantly slower than doing so with standalone Git. This is because every hanshake with the remote involves querying and filtering the entire repo history, even for partial checkouts. Incremental checkouts are advertised as being faster due to filter caching, and Josh has explicit [filter commands](https://josh-project.dev/docs/reference/cli.html#josh-cache), but we'll evaluate this later on...

---

---
**Pain Point: Conflicting Git SHAs**
Since Josh rewrites history for partial checkouts, logically identical commits will have different Git SHAs between clones that differ in checkout scope. Two checkouts with the same scope (e.g. both scoped to `:/hardware`) will have identical SHAs for logically identical commits.

This has the potential to break tooling, and means developers using differently scoped checkout won't have compatible Git SHAs when comparing commits. To account for this, Josh recommends using [Change IDs](https://josh-project.dev/docs/guide/stacked-changes.html#change-ids) as the source-of-truth identifier for a given commit, and even depends on use of Change IDs for use of some of its commit stacking features. Regardless, it requires the repo implement extra tooling for identifying logically identical commits.

Conflicting Git history / SHAs is shown below for the example with Patrick and Adam:

**Adam's 'josh-mono' History**
```
commit 8c2bbb8b914e620ecaaea60165abc45f2c0e26f9 (HEAD -> main, origin/main, origin/HEAD)
Author: Sean Walsh <seanmichwalsh@protonmail.ch>
Date:   Sun Jul 5 17:13:30 2026 -0700

    CSRS: Generate header for toy-core CSRs

commit da4d31014765c67bab9ab30b10aa310e5c5a93a2
Author: Sean Walsh <seanmichwalsh@protonmail.ch>
Date:   Sun Jul 5 16:30:36 2026 -0700

    RTL: Create a stubbed SV file defining a toy core

commit 79a2c88b6f75ae49ec20c86d262e89a8be515378
Author: Sean Walsh <seanmichwalsh@protonmail.ch>
Date:   Sun Jul 5 15:30:07 2026 -0700

    DOC: Document repo initialization steps

commit e96ef5a6db16eed9c5fe1f0b3e03a23cf279988a
Author: Sean Walsh <seanmichwalsh@protonmail.ch>
Date:   Sun Jul 5 14:55:08 2026 -0700

    REPO: Add monorepo directory structure

commit 8e033016615719ee14639fb5f03caec0e60c3748
Author: Sean Walsh <seanmichwalsh@protonmail.ch>
Date:   Sun Jul 5 14:33:02 2026 -0700

    DOC: Add Josh workflow walkthrough outline

commit a0997c7fbfb53aa22e9df5527ae3b333fd946b46
Author: Sean Walsh <seanmichwalsh@protonmail.ch>
Date:   Sun Jul 5 14:04:00 2026 -0700

    DOC: Define purpose of Josh exploration repo
```

**Patrick's 'josh-hw' History**
```
commit 2070c71d1c4e7f2ccce3e50d4363520ad27d0391 (HEAD -> main, origin/main, origin/HEAD)
Author: Sean Walsh <seanmichwalsh@protonmail.ch>
Date:   Sun Jul 5 17:13:30 2026 -0700

    CSRS: Generate header for toy-core CSRs

commit eadc5fbf61c5d5d38dda7fd8d85f0cb7943af52b
Author: Sean Walsh <seanmichwalsh@protonmail.ch>
Date:   Sun Jul 5 16:30:36 2026 -0700

    RTL: Create a stubbed SV file defining a toy core

commit 05a5d3f03711cc4fcb44cdd2dc412ac43397df17
Author: Sean Walsh <seanmichwalsh@protonmail.ch>
Date:   Sun Jul 5 14:55:08 2026 -0700

    REPO: Add monorepo directory structure
```

---

### Importing Third-Party Projects

### Repo vs Project Level Branch Management

### Workspaces and Shared Dependencies

### Cross-Project Breaking Changes

### Interfacing with Upstream
