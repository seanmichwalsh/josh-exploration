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

### Importing Third-Party Projects

### Repo vs Project Level Branch Management

### Workspaces and Shared Dependencies

### Cross-Project Breaking Changes

### Interfacing with Upstream
