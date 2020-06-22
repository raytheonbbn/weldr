---
name: Bug report
about: Report something wrong with weldr.
title: ''
labels: ''
assignees: ''

---

**BEFORE YOU BEGIN**
1. (Coming soon!) Please run `weldr/env_info.sh` and redirect the output to a file.  This captures your operating system version, python version, and pip package versions.
2. If possible, please rerun the failing weldr job, and use the `--log-file` option to produce a log file.
3. If the bug includes a compiler or linker error, copy the full error trace and failed command to a file (These may not get captured by weldr's logger.)
4. Please attach these files to this issue.

**Describe the bug**
A clear and concise description of what the bug is.

**To Reproduce**
Steps to reproduce the behavior.  Please include the specific command line flags needed to trip the error.

**Expected behavior**
A clear and concise description of what you expected to happen.

**Additional context**
Add any other context about the problem here.

**Example Project**
If possible and applicable, please include an example weldr project for which the bug triggers.
Please package project directories into archive files, and attach them to this issue (`.tar` and `.tgz` files are preferred).

Make sure that you include the system definition file you used to run weldr,
as well as references to any external dependencies not included in the project itself.
